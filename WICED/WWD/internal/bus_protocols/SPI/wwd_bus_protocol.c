/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <string.h>  /* For memcpy */
#include "wwd_assert.h"
#include "platform/wwd_platform_interface.h"
#include "platform/wwd_spi_interface.h"
#include "network/wwd_network_constants.h"
#include "network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"
#include "internal/wwd_internal.h"
#include "internal/wwd_bcmendian.h"
#include "internal/wwd_sdpcm.h"
#include "wwd_bus_protocol.h"
#include "chip_constants.h"

/******************************************************
 *             Constants
 ******************************************************/

#define F2_READY_TIMEOUT_MS    (1000)
#define F2_READY_TIMEOUT_LOOPS (1000)
#define F1_READY_TIMEOUT_LOOPS (1000)
#define FEADBEAD_TIMEOUT_MS    (5000)
#define ALP_AVAIL_TIMEOUT_MS   (100)
#define HT_AVAIL_TIMEOUT_MS    (1000)
/* Taken from FALCON_5_90_195_26 dhd/sys/dhd_sdio.c. For 43362, MUST be >= 8 and word-aligned otherwise dongle fw crashes */
#define SPI_F2_WATERMARK       (32)

#define GSPI_PACKET_AVAILABLE  (1 << 8)
#define GSPI_UNDERFLOW         (1 << 1)

#define WLAN_BUS_UP_ATTEMPTS   (1000)

#define VERIFY_RESULT( x )     { wwd_result_t verify_result; verify_result = ( x ); if ( verify_result != WWD_SUCCESS ) return verify_result; }

#define SWAP32_16BIT_PARTS(val) ((uint32_t)(( ((uint32_t)(val)) >> 16) + ((((uint32_t)(val)) & 0xffff)<<16)))

#ifdef GSPI_USING_DSTATUS
#define WWD_BUS_GSPI_PACKET_OVERHEAD    ( sizeof( wwd_buffer_header_t ) + sizeof( uint32 ) )
#else
#define WWD_BUS_GSPI_PACKET_OVERHEAD    ( sizeof( wwd_buffer_header_t ) )
#endif

#define MAX_GSPI_TRANSFER_LEN  2048

#define H32TO16LE(x)           ( ( uint32_t ) ( ( ( ( uint32_t ) ( x ) & ( uint32_t ) 0x000000ffU ) << 8 ) | \
                                                ( ( ( uint32_t ) ( x ) & ( uint32_t ) 0x0000ff00U ) >> 8 ) | \
                                                ( ( ( uint32_t ) ( x ) & ( uint32_t ) 0x00ff0000U ) << 8 ) | \
                                                ( ( ( uint32_t ) ( x ) & ( uint32_t ) 0xff000000U ) >> 8 ) ) )

typedef enum
{
   GSPI_INCREMENT_ADDRESS = 1,
   GSPI_FIXED_ADDRESS     = 0
} gspi_transfer_access_t;

/******************************************************
 *             Structures
 ******************************************************/

#pragma pack(1)

typedef struct
{
    wwd_bus_gspi_header_t      header;
    uint8_t            response_delay[4];
} gspi_backplane_f1_read_header_t;

#pragma pack()

typedef struct
{
    gspi_backplane_f1_read_header_t  gspi_header;
    uint32_t                         data[1];
} gspi_backplane_f1_read_packet_t;

/******************************************************
 *             Static variables
 ******************************************************/

static wiced_bool_t  wwd_bus_gspi_32bit = WICED_FALSE;
static const uint8_t wwd_bus_gspi_command_mapping[] =
{
    0,
    1
};

static wiced_bool_t bus_is_up               = WICED_FALSE;
static wiced_bool_t wwd_bus_flow_controlled = WICED_FALSE;


/******************************************************
 *             Static Function Declarations
 ******************************************************/

static wwd_result_t wwd_download_firmware   ( void );
static wwd_result_t wwd_read_register_value ( wwd_bus_function_t function, uint32_t address, uint8_t value_length, /*@out@*/ uint8_t* value );
static wwd_result_t wwd_bus_transfer_buffer   ( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, wiced_buffer_t buffer );

/******************************************************
 *             SPI Logging
 * Enable this section for logging of SPI transfers
 * by changing "if 0" to "if 1"
 ******************************************************/
#if 0

#define GSPI_LOG_SIZE        (110)
#define SDIO_LOG_HEADER_SIZE (0)   /*(0x30) */

typedef enum
{
    UNUSED,
    LOG_TX,
    LOG_RX
}gspi_log_direction_t;

typedef struct gSPI_log_entry_struct
{
    gspi_log_direction_t direction;
    wwd_bus_function_t       function;
    uint32_t             address;
    unsigned long        time;
    unsigned long        length;
#if ( SDIO_LOG_HEADER_SIZE != 0 )
    unsigned char        header[GSPI_LOG_HEADER_SIZE];
#endif /* if ( SDIO_LOG_HEADER_SIZE != 0 ) */
}gspi_log_entry_t;

static int               next_gspi_log_pos = 0;
static gspi_log_entry_t  gspi_log_data[GSPI_LOG_SIZE];

static void add_log_entry( gspi_log_direction_t dir, wwd_bus_function_t function, uint32_t address, unsigned long length, char* gspi_data )
{
    gspi_log_data[next_gspi_log_pos].direction = dir;
    gspi_log_data[next_gspi_log_pos].function = function;
    gspi_log_data[next_gspi_log_pos].address = address;
    gspi_log_data[next_gspi_log_pos].time = host_rtos_get_time();
    gspi_log_data[next_gspi_log_pos].length = length;
#if ( SDIO_LOG_HEADER_SIZE != 0 )
    memcpy( gspi_log_data[next_gspi_log_pos].header, gspi_data, (length>=GSPI_LOG_HEADER_SIZE)?GSPI_LOG_HEADER_SIZE:length );
#endif /* if ( SDIO_LOG_HEADER_SIZE != 0 ) */
    next_gspi_log_pos++;
    if (next_gspi_log_pos >= GSPI_LOG_SIZE)
    {
        next_gspi_log_pos = 0;
    }
}
#else
#define add_log_entry( dir, function, address, length, gspi_data )
#endif

/******************************************************
 *             Global Function definitions
 ******************************************************/

wwd_result_t wwd_bus_send_buffer( wiced_buffer_t buffer )
{
    wwd_result_t result = wwd_bus_transfer_buffer( BUS_WRITE, WLAN_FUNCTION, 0, buffer );
    host_buffer_release( buffer, WWD_NETWORK_TX );
    return result;
}

/*
 * Perform a transfer on the gSPI bus
 * Prerequisites: length < MAX_GSPI_TRANSFER_LEN
 */
static wwd_result_t wwd_bus_transfer_buffer( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, wiced_buffer_t buffer )
{
    uint32_t* temp;
    wwd_result_t result;
    uint16_t newsize;
    wwd_buffer_header_t* header = (wwd_buffer_header_t*) host_buffer_get_current_piece_data_pointer( buffer );
    wwd_transfer_bytes_packet_t* gspipacket = (wwd_transfer_bytes_packet_t*) &header->bus_header;

    uint16_t size = (uint16_t) ( host_buffer_get_current_piece_size( buffer ) - sizeof( wwd_buffer_header_t ) );

#ifdef SUPPORT_BUFFER_CHAINING
#error BUFFER CHAINING NOT IMPLEMENTED IN GSPI YET
#endif /* ifdef SUPPORT_BUFFER_CHAINING */
    /* Form the gSPI header */
    gspipacket->bus_header.gspi_header = (wwd_bus_gspi_header_t) ( (uint32_t) ( ( wwd_bus_gspi_command_mapping[(int)direction] & 0x1     ) << 31 ) |
                                                           (uint32_t) ( ( GSPI_INCREMENT_ADDRESS               & 0x1     ) << 30 ) |
                                                           (uint32_t) ( ( function                             & 0x3     ) << 28 ) |
                                                           (uint32_t) ( ( address                              & 0x1FFFF ) << 11 ) |
                                                           (uint32_t) ( ( size                                 & 0x7FF   ) << 0 ) );

    /* Reshuffle the bits if we're not in 32 bit mode */
    if ( wwd_bus_gspi_32bit == WICED_FALSE )
    {
        /* Note: This typecast should always be valid if the buffer containing the GSpi packet has been correctly declared as 32-bit aligned */
        temp = (uint32_t*) &gspipacket->bus_header.gspi_header;
        *temp = H32TO16LE(*temp);
    }

    /* Round size up to 32-bit alignment */
    newsize = (uint16_t) ROUND_UP(size, 4);

    /* Send the data */
    if ( direction == BUS_WRITE )
    {
        /* Wait for FIFO to be ready to accept data */
        if ( function == WLAN_FUNCTION )
        {
            uint32_t wwd_bus_gspi_status;
            uint32_t loop_count = 0;
            while ( ( ( result = wwd_read_register_value( BUS_FUNCTION, SPI_STATUS_REGISTER, (uint8_t) 4, (uint8_t*) &wwd_bus_gspi_status ) ) == WWD_SUCCESS ) &&
                    ( ( wwd_bus_gspi_status & ( 1 << 5 ) ) == 0 ) &&
                    ( loop_count < (uint32_t) F2_READY_TIMEOUT_LOOPS ) )
            {
                loop_count++;
            }
            if ( result != WWD_SUCCESS )
            {
                return result;
            }
            if ( loop_count >= (uint32_t) F2_READY_TIMEOUT_LOOPS )
            {
                WPRINT_WWD_ERROR(("Timeout waiting for data FIFO to be ready\n"));
                return WICED_TIMEOUT;
            }
        }

        add_log_entry( LOG_TX, function, address, size, (char*)&gspipacket->data );
    }
    result = host_platform_spi_transfer( direction, (uint8_t*) gspipacket, (uint16_t) ( newsize + sizeof(wwd_bus_gspi_header_t) ) );
    if ( direction == BUS_READ )
    {
        add_log_entry( LOG_RX, function, address, size, (char*)&gspipacket->data );
    }

    return result;
}

wwd_result_t wwd_bus_poke_wlan( void )
{
    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_set_flow_control( uint8_t value )
{
    if ( value != 0 )
    {
        wwd_bus_flow_controlled = WICED_TRUE;
    }
    else
    {
        wwd_bus_flow_controlled = WICED_FALSE;
    }
    return WWD_SUCCESS;
}

wiced_bool_t wwd_bus_is_flow_controlled( void )
{
    return wwd_bus_flow_controlled;
}

wwd_result_t wwd_bus_ack_interrupt(uint32_t intstatus)
{
    return wwd_bus_write_register_value( BUS_FUNCTION, SPI_INTERRUPT_REGISTER, (uint8_t) 2, intstatus );
}

uint32_t wwd_bus_packet_available_to_read(void)
{
    uint16_t interrupt_register;

    /* Read the interrupt register */
    if (wwd_read_register_value( BUS_FUNCTION, SPI_INTERRUPT_REGISTER, (uint8_t) 2, (uint8_t*) &interrupt_register ) != WWD_SUCCESS)
    {
        goto return_with_error;
    }

    if ( ( interrupt_register & 0x0086 ) != 0 ) /* This should be 0x87, but occasional "data not available" errors are flagged seemingly for no reason */
    {
        /* Error condition detected */
        WPRINT_WWD_DEBUG(("Bus error condition detected\n"));
    }

    /* Clear interrupt register */
    if ( interrupt_register != 0 )
    {
        if (wwd_bus_write_register_value( BUS_FUNCTION, SPI_INTERRUPT_REGISTER, (uint8_t) 2, interrupt_register ) != WWD_SUCCESS)
        {
            goto return_with_error;
        }
    }

    return (uint32_t)((interrupt_register) & (F2_PACKET_AVAILABLE));

return_with_error:
    wiced_assert("Error accessing backplane", 0 != 0);
    return 0;
}

/*@only@*/ /*@null@*/ wwd_result_t wwd_bus_read_frame( wiced_buffer_t* buffer )
{
    uint32_t wwd_bus_gspi_status;
    wwd_result_t result;
    uint32_t wiced_gspi_bytes_pending;

    result = wwd_read_register_value( BUS_FUNCTION, SPI_STATUS_REGISTER, (uint8_t) 4, (uint8_t*) &wwd_bus_gspi_status );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

#ifdef DEBUG
    if ((wwd_bus_gspi_status & GSPI_PACKET_AVAILABLE)&&(( (wwd_bus_gspi_status >> 9) & 0x7FF )== 0 )) { WPRINT_WWD_DEBUG(("gSPI packet of size zero available\n")); }

    if ((wwd_bus_gspi_status & GSPI_PACKET_AVAILABLE)&&(( (wwd_bus_gspi_status >> 9) & 0x7FF ) > WICED_LINK_MTU - WWD_BUS_GSPI_PACKET_OVERHEAD )) { WPRINT_WWD_DEBUG(("gSPI packet size available is too big for buffers\n")); }

    if ((wwd_bus_gspi_status & GSPI_PACKET_AVAILABLE)&&( wwd_bus_gspi_status & GSPI_UNDERFLOW) ) { WPRINT_WWD_ERROR(("gSPI underflow - packet size will be wrong\n")); }
#endif /* ifdef DEBUG */

    wiced_gspi_bytes_pending = 0;

    if ( ( wwd_bus_gspi_status & GSPI_PACKET_AVAILABLE ) != 0 )
    {
        wiced_gspi_bytes_pending = ( ( wwd_bus_gspi_status >> 9 ) & 0x7FF );
    }

    if ( wiced_gspi_bytes_pending == 0 )
    {
        return WWD_NO_PACKET_TO_RECEIVE;
    }

    /* Allocate a suitable buffer */
    result = host_buffer_get( buffer, WWD_NETWORK_RX, (unsigned short)( wiced_gspi_bytes_pending + WWD_BUS_GSPI_PACKET_OVERHEAD), WICED_FALSE );

    if ( result != WWD_SUCCESS )
    {
        /* Read out the first 12 bytes to get the bus credit information */
        uint8_t temp_buffer[12 + sizeof(wwd_bus_header_t)];
        wwd_bus_transfer_bytes( BUS_READ, WLAN_FUNCTION, 0, 12, (wwd_transfer_bytes_packet_t*) temp_buffer );

        /* Abort the transfer to force the packet to be dropped */
        if ( wiced_gspi_bytes_pending > 12 )
        {
            wwd_bus_write_register_value( BACKPLANE_FUNCTION, SPI_FRAME_CONTROL, 1, ( 1 << 0 ) );
        }

        /* Process bus data credit information */
        wwd_sdpcm_update_credit( (uint8_t*) ( temp_buffer + sizeof(wwd_bus_header_t) ) );
        return result;
    }

    result = wwd_bus_transfer_buffer( BUS_READ, WLAN_FUNCTION, 0, *buffer );
    if ( result != WWD_SUCCESS)
    {
        host_buffer_release( *buffer, WWD_NETWORK_RX );
        return result;
    }

    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_init( void )
{
    uint32_t data = 0;
    uint32_t wwd_bus_gspi_status;
    uint16_t data16 = 0;
    uint32_t loop_count;
    wwd_result_t result;
    uint8_t init_data[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    wwd_bus_gspi_32bit = WICED_FALSE;

    wwd_bus_init_backplane_window( );

    host_platform_power_wifi( WICED_TRUE );
    host_platform_reset_wifi( WICED_TRUE );
    host_rtos_delay_milliseconds( (uint32_t) 1 );
    host_platform_reset_wifi( WICED_FALSE );

    /* Due to an chip issue, the first transfer will be corrupted.
     * This means a repeated safe read of a known value register is required until
     * the correct value is returned - signalling the bus is running.
     * This known value register must be accessed using fixed (non-incrementing) address
     * mode, hence a custom packet header must be constructed
     * Due to the chip issue, the data received could be left shifted by one bit.
     */
    loop_count = 0;
    do
    {
        wwd_bus_gspi_header_t* gspi_header = (wwd_bus_gspi_header_t*) init_data;

        *gspi_header = (wwd_bus_gspi_header_t) SWAP32_16BIT_PARTS( BCMSWAP32( (uint32_t)( ( wwd_bus_gspi_command_mapping[(int)BUS_READ] & 0x1      ) << 31) |
                                                                      (uint32_t)( ( GSPI_FIXED_ADDRESS                  & 0x1      ) << 30) |
                                                                      (uint32_t)( ( BUS_FUNCTION                        & 0x3      ) << 28) |
                                                                      (uint32_t)( ( SPI_READ_TEST_REGISTER              & 0x1FFFFu ) << 11) |
                                                                      (uint32_t)( ( 4u /*size*/                         & 0x7FFu   ) <<  0 ) ) );
        VERIFY_RESULT( host_platform_spi_transfer(BUS_READ, init_data, (uint16_t) 12 ) );
        loop_count++;
    } while ( ( NULL == memchr( &init_data[4], SPI_READ_TEST_REG_LSB,      (size_t) 8 ) ) &&
              ( NULL == memchr( &init_data[4], SPI_READ_TEST_REG_LSB_SFT1, (size_t) 8 ) ) &&
              ( NULL == memchr( &init_data[4], SPI_READ_TEST_REG_LSB_SFT2, (size_t) 8 ) ) &&
              ( NULL == memchr( &init_data[4], SPI_READ_TEST_REG_LSB_SFT3, (size_t) 8 ) ) &&
              ( loop_count < (uint32_t) FEADBEAD_TIMEOUT_MS ) &&
              ( host_rtos_delay_milliseconds( (uint32_t) 1 ), ( 1 == 1 ) ) );

    /* Keep/reset defaults for registers 0x0-0x4 except for, 0x0: Change word length to 32bit, set endianness, enable wakeup. 0x2: enable interrupt with status. */
#if defined(IL_BIGENDIAN)
    VERIFY_RESULT( wwd_bus_write_register_value(BUS_FUNCTION, SPI_BUS_CONTROL, (uint8_t) 4, (uint32_t) ( WORD_LENGTH_32 | (0 & ENDIAN_BIG) | INTR_POLARITY_HIGH | WAKE_UP | (0x4 << (8*SPI_RESPONSE_DELAY)) | ((0 & STATUS_ENABLE) << (8*SPI_STATUS_ENABLE)) | ( INTR_WITH_STATUS << (8*SPI_STATUS_ENABLE)) ) ) );
#else
    VERIFY_RESULT( wwd_bus_write_register_value(BUS_FUNCTION, SPI_BUS_CONTROL, (uint8_t) 4, (uint32_t) ( WORD_LENGTH_32 | ENDIAN_BIG       | INTR_POLARITY_HIGH | WAKE_UP | (0x4 << (8*SPI_RESPONSE_DELAY)) | ((0 & STATUS_ENABLE) << (8*SPI_STATUS_ENABLE)) | ( INTR_WITH_STATUS << (8*SPI_STATUS_ENABLE)) ) ) );
#endif
    wwd_bus_gspi_32bit = WICED_TRUE;
    VERIFY_RESULT( wwd_read_register_value(BUS_FUNCTION, SPI_BUS_CONTROL, (uint8_t) 4, (uint8_t*)&data ) );

    /* Check feedbead can be read - i.e. the device is alive */
    data = 0;
    VERIFY_RESULT( wwd_read_register_value( BUS_FUNCTION, SPI_READ_TEST_REGISTER, (uint8_t) 4, (uint8_t*) &data ) );

    if ( data != SPI_READ_TEST_REGISTER_VALUE )
    {
        WPRINT_WWD_ERROR(("Read %x, instead of 0xFEEDBEAD from the WLAN chip\n", (unsigned int)data ));
        return WWD_SPI_ID_READ_FAIL;
    }

    /* Make sure error interrupt bits are clear */
    VERIFY_RESULT( wwd_bus_write_register_value(BUS_FUNCTION, SPI_INTERRUPT_REGISTER, (uint8_t) 1, (uint32_t) ( DATA_UNAVAILABLE | COMMAND_ERROR | DATA_ERROR | F1_OVERFLOW ) ) );

    /* Enable a selection of interrupts */
    VERIFY_RESULT( wwd_bus_write_register_value(BUS_FUNCTION, SPI_INTERRUPT_ENABLE_REGISTER, (uint8_t) 2, (uint32_t) ( F2_F3_FIFO_RD_UNDERFLOW | F2_F3_FIFO_WR_OVERFLOW | COMMAND_ERROR | DATA_ERROR | F2_PACKET_AVAILABLE | F1_OVERFLOW ) ) );

    /* Request ALP */
    VERIFY_RESULT( wwd_bus_write_register_value(BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, SBSDIO_ALP_AVAIL_REQ ) );

    /* Wait until ALP is available */
    loop_count = 0;
    while ( ( ( result = wwd_read_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 2, (uint8_t*) &data16 ) ) == WWD_SUCCESS ) &&
            ( ( data16 & SBSDIO_ALP_AVAIL ) == 0 ) &&
            ( loop_count < (uint32_t) ALP_AVAIL_TIMEOUT_MS ) )
    {
        host_rtos_delay_milliseconds( (uint32_t) 1 );
        loop_count++;
    }
    if ( loop_count >= (uint32_t) ALP_AVAIL_TIMEOUT_MS )
    {
        return WICED_TIMEOUT;
    }
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Clear request for ALP */
    VERIFY_RESULT( wwd_bus_write_register_value(BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, (uint32_t) 0) );

    /* Download the firmware */
    result = wwd_download_firmware( );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_WWD_ERROR(("Could not download firmware\n"));
        return result;
    }

    /* Wait for F2 to be ready */
    loop_count = 0;
    while ( ( ( result = wwd_read_register_value( BUS_FUNCTION, SPI_STATUS_REGISTER, (uint8_t) 4, (uint8_t*) &wwd_bus_gspi_status ) ) == WWD_SUCCESS ) &&
            ( ( wwd_bus_gspi_status & ( 1 << 5 ) ) == 0 ) &&
            ( loop_count < (uint32_t) F2_READY_TIMEOUT_MS ) )
    {
        host_rtos_delay_milliseconds( (uint32_t) 1 );
        loop_count++;
    }
    if ( loop_count >= (uint32_t) F2_READY_TIMEOUT_MS )
    {
        /* If your system fails here, it could be due to incorrect NVRAM variables.
         * Check which 'wifi_nvram_image.h' file your platform is using, and
         * check that it matches the WLAN device on your platform, including the
         * crystal frequency.
         */
        WPRINT_WWD_ERROR(("Timeout while waiting for function 2 to be ready\n"));
        return WICED_TIMEOUT;
    }

    bus_is_up = WICED_TRUE;

    return result;
}

wwd_result_t wwd_bus_deinit( void )
{
    /* put device in reset. */
    host_platform_reset_wifi( WICED_TRUE );

    bus_is_up = WICED_FALSE;

    return WWD_SUCCESS;
}

/******************************************************
 *     Function definitions for Protocol Common
 ******************************************************/

/*
 * Write a value to a register NOT on the backplane
 * Prerequisites: value_length <= 4
 */
wwd_result_t wwd_bus_write_register_value( wwd_bus_function_t function, uint32_t address, uint8_t value_length, uint32_t value )
{
    char gspi_internal_buffer[sizeof(wwd_bus_header_t) + sizeof(uint32_t) + sizeof(uint32_t)];
    wwd_transfer_bytes_packet_t* internal_gspi_packet = (wwd_transfer_bytes_packet_t*) gspi_internal_buffer;

    /* Flip the bytes if we're not in 32 bit mode */
    if ( wwd_bus_gspi_32bit == WICED_FALSE )
    {
        value = H32TO16LE(value);
    }
    /* Write the value and value_length into the packet */
    internal_gspi_packet->data[0] = value;

    /* Send it off */
    return wwd_bus_transfer_bytes( BUS_WRITE, function, address, value_length, internal_gspi_packet );
}

/*
 * Write a value to a register on the backplane
 * Prerequisites: value_length <= 4
 */
wwd_result_t wwd_bus_write_backplane_value( uint32_t address, uint8_t register_length, uint32_t value )
{
    VERIFY_RESULT( wwd_bus_set_backplane_window(address) );
    return wwd_bus_write_register_value( BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, register_length, value );
}

/*
 * Read the value of a register on the backplane
 * Prerequisites: value_length <= 4
 */
wwd_result_t wwd_bus_read_backplane_value( uint32_t address, uint8_t register_length, /*@out@*/ uint8_t* value )
{
    *value = 0;
    VERIFY_RESULT( wwd_bus_set_backplane_window(address) );
    return wwd_read_register_value( BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, register_length, value );
}

wwd_result_t wwd_bus_transfer_bytes( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint16_t size, /*@in@*/ /*@out@*/ wwd_transfer_bytes_packet_t* data )
{
    uint32_t* temp;
    wwd_result_t result;
    uint16_t newsize;

    data->bus_header.gspi_header = (wwd_bus_gspi_header_t) ( (uint32_t) ( ( wwd_bus_gspi_command_mapping[(int)direction] & 0x1     ) << 31 ) |
                                                     (uint32_t) ( ( GSPI_INCREMENT_ADDRESS               & 0x1     ) << 30 ) |
                                                     (uint32_t) ( ( function                             & 0x3     ) << 28 ) |
                                                     (uint32_t) ( ( address                              & 0x1FFFF ) << 11 ) |
                                                     (uint32_t) ( ( size                                 & 0x7FF   ) <<  0 ) );

    /* Reshuffle the bits if we're not in 32 bit mode */
    if ( wwd_bus_gspi_32bit == WICED_FALSE )
    {
        /* Note: This typecast should always be valid if the buffer containing the GSpi packet has been correctly declared as 32-bit aligned */
        temp  = (uint32_t*) data;
        *temp = H32TO16LE(*temp);
    }

    /* Round size up to 32-bit alignment */
    newsize = (uint16_t) ROUND_UP(size, 4);

    /* Ensure we are clear to write */
    if ( ( direction == BUS_WRITE ) && ( function == WLAN_FUNCTION ) )
    {
        uint32_t wwd_bus_gspi_status;
        uint32_t loop_count = 0;

        /* Verify the SDPCM size and stated size match */
        uint16_t* frametag_ptr = (uint16_t*) &data->data;
        if ( size != *frametag_ptr )
        {
            WPRINT_WWD_DEBUG(("Error - gSPI size does not match SDPCM size!\n"));
            return WWD_SPI_SIZE_MISMATCH;
        }

        /* Wait for WLAN FIFO to be ready to accept data */
        while ( ( ( result = wwd_read_register_value( BUS_FUNCTION, SPI_STATUS_REGISTER, (uint8_t) 4, (uint8_t*) &wwd_bus_gspi_status ) ) == WWD_SUCCESS ) &&
                ( ( wwd_bus_gspi_status & ( 1 << 5 ) ) == 0 ) &&
                ( loop_count < (uint32_t) F2_READY_TIMEOUT_LOOPS ) )
        {
            ++loop_count;
        }

        if ( result != WWD_SUCCESS )
        {
            return result;
        }

        if ( loop_count >= (uint32_t) F2_READY_TIMEOUT_LOOPS )
        {
            WPRINT_WWD_DEBUG(("Timeout waiting for data FIFO to be ready\n"));
            return WICED_TIMEOUT;
        }

        add_log_entry( LOG_TX, function, address, (unsigned long)size, (char*)&data->data );
    }

    /* Send the data */
    result = host_platform_spi_transfer( direction, (uint8_t*) data, (uint16_t) ( newsize + sizeof(wwd_bus_gspi_header_t) ) );

    if ( direction == BUS_READ )
    {
        add_log_entry( LOG_RX, function, address, (unsigned long)((function == BACKPLANE_FUNCTION)?size-4:size), (function == BACKPLANE_FUNCTION)?((char*)&data->data)+4:(char*)&data->data );
    }

    return result;
}

wwd_result_t wwd_bus_ensure_is_up( void )
{
    uint32_t attempts = (uint32_t) WLAN_BUS_UP_ATTEMPTS;
    uint32_t spi_bus_reg_value;
    uint8_t  csr_reg_value;

    if ( bus_is_up == WICED_TRUE )
    {
        return WICED_SUCCESS;
    }

    /* Wake up WLAN SPI interface module */
    VERIFY_RESULT( wwd_read_register_value( BUS_FUNCTION, SPI_BUS_CONTROL, sizeof(uint32_t), (uint8_t*)&spi_bus_reg_value ) );
    spi_bus_reg_value |= (uint32_t)( WAKE_UP );
    VERIFY_RESULT( wwd_bus_write_register_value( BUS_FUNCTION, SPI_BUS_CONTROL, sizeof(uint32_t), spi_bus_reg_value ) );

    /* Ensure HT clock is up */
    VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, 1, SBSDIO_HT_AVAIL_REQ ) );

    do
    {
        VERIFY_RESULT( wwd_read_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, 1, &csr_reg_value ) );
        --attempts;
    }
    while ( ( ( csr_reg_value & SBSDIO_HT_AVAIL_REQ ) == 0 ) && ( attempts != 0 ) && ( host_rtos_delay_milliseconds( 1 ), 1 == 1 ) );

    if ( attempts == 0 )
    {
        return WICED_ERROR;
    }
    else
    {
        bus_is_up = WICED_TRUE;
        return WWD_SUCCESS;
    }
}

wwd_result_t wwd_bus_allow_wlan_bus_to_sleep( void )
{
    if ( bus_is_up == WICED_TRUE )
    {
        uint32_t spi_bus_reg_value;

        bus_is_up = WICED_FALSE;

        /* Clear HT clock request */
        VERIFY_RESULT( wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, 1, 0 ) );

        /* Put SPI interface block to sleep */
        VERIFY_RESULT( wwd_read_register_value( BUS_FUNCTION, SPI_BUS_CONTROL, sizeof(uint32_t), (uint8_t*)&spi_bus_reg_value ) );
        spi_bus_reg_value &= ~(uint32_t) ( WAKE_UP );
        return wwd_bus_write_register_value( BUS_FUNCTION, SPI_BUS_CONTROL, sizeof(uint32_t), spi_bus_reg_value );
    }
    else
    {
        return WWD_SUCCESS;
    }
}

/******************************************************
 *             Static  Function definitions
 ******************************************************/

static wwd_result_t wwd_download_firmware( void )
{
    uint8_t csr_val;
    wwd_result_t result;
    uint32_t loop_count = 0;

    VERIFY_RESULT( wwd_disable_device_core(ARM_CORE) );
    VERIFY_RESULT( wwd_disable_device_core(SOCRAM_CORE) );
    VERIFY_RESULT( wwd_reset_device_core(SOCRAM_CORE) );

#ifdef MFG_TEST_ALTERNATE_WLAN_DOWNLOAD
    VERIFY_RESULT( external_write_wifi_firmware_and_nvram_image( ) );
#else
    VERIFY_RESULT( wwd_bus_write_wifi_firmware_image( ) );
    VERIFY_RESULT( wwd_bus_write_wifi_nvram_image( ) );
#endif /* ifdef MFG_TEST_ALTERNATE_WLAN_DOWNLOAD */

    /* Take the ARM core out of reset */
    VERIFY_RESULT( wwd_reset_device_core( ARM_CORE ) );
    result = wwd_device_core_is_up( ARM_CORE );
    if ( result != WWD_SUCCESS )
    {
        WPRINT_WWD_DEBUG(("Could not bring ARM core up\n"));
        return result;
    }

    /* Wait until the HT clock is available */
    while ( ( ( result = wwd_read_register_value( BACKPLANE_FUNCTION, SDIO_CHIP_CLOCK_CSR, (uint8_t) 1, &csr_val ) ) == WWD_SUCCESS ) &&
            ( ( csr_val & SBSDIO_HT_AVAIL ) == 0 ) &&
            ( loop_count < (uint32_t) HT_AVAIL_TIMEOUT_MS ) )
    {
        host_rtos_delay_milliseconds( (uint32_t) 1 );
        loop_count++;
    }
    if ( loop_count >= (uint32_t) HT_AVAIL_TIMEOUT_MS )
    {
        return WICED_TIMEOUT;
    }
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Set up the interrupt mask and enable interrupts */
    VERIFY_RESULT( wwd_bus_write_backplane_value(SDIO_INT_HOST_MASK, (uint8_t) 4, I_HMB_SW_MASK) );

    /* Lower F2 Watermark to avoid DMA Hang in F2 when SD Clock is stopped. */
    return wwd_bus_write_backplane_value( SDIO_FUNCTION2_WATERMARK, (uint8_t) 1, (uint32_t) SPI_F2_WATERMARK );
}

/*
 * Read the value of a register NOT on the backplane
 * Prerequisites: value_length <= 4
 */
static wwd_result_t wwd_read_register_value( wwd_bus_function_t function, uint32_t address, uint8_t value_length, /*@out@*/ uint8_t* value )
{
    uint32_t* data_ptr;
    wwd_result_t result;
    uint8_t padding = 0;

    char gspi_internal_buffer[sizeof(wwd_bus_header_t) + sizeof(uint32_t) + sizeof(uint32_t)];

    /* Clear the receiving part of memory and set the value_length */
    if ( function == BACKPLANE_FUNCTION )
    {
        gspi_backplane_f1_read_packet_t* pkt = (gspi_backplane_f1_read_packet_t*) gspi_internal_buffer;
        data_ptr = pkt->data;
        padding = 4; /* Add response delay */
    }
    else
    {
        wwd_transfer_bytes_packet_t* pkt = (wwd_transfer_bytes_packet_t*) gspi_internal_buffer;
        data_ptr = pkt->data;
    }
    *data_ptr = 0;
    result = wwd_bus_transfer_bytes( BUS_READ, function, address, (uint16_t)(value_length + padding), (wwd_transfer_bytes_packet_t*) gspi_internal_buffer );

    memcpy( value, data_ptr, value_length );

    return result;
}
