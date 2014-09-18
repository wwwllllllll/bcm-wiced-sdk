/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *  Broadcom WLAN SDIO Protocol interface
 *
 *  Implements the WWD Bus Protocol Interface for SDIO
 *  Provides functions for initialising, de-intitialising 802.11 device,
 *  sending/receiving raw packets etc
 */

#include <string.h> /* For memcpy */
#include "wwd_assert.h"
#include "network/wwd_buffer_interface.h"
#include "internal/wwd_sdpcm.h"
#include "internal/wwd_internal.h"
#include "RTOS/wwd_rtos_interface.h"
#include "platform/wwd_platform_interface.h"
#include "internal/bus_protocols/wwd_bus_protocol_interface.h"
#include "chip_constants.h"
#include "wiced_resource.h"   /* TODO: remove include dependency */
#include "resources.h"        /* TODO: remove include dependency */
#include "wifi_nvram_image.h" /* TODO: remove include dependency */

/******************************************************
 *                      Macros
 ******************************************************/

#define ROUNDUP( x, y )     ((((x) + ((y) - 1)) / (y)) * (y))
#define VERIFY_RESULT( x )  { wwd_result_t verify_result; verify_result = (x); if ( verify_result != WWD_SUCCESS ) return verify_result; }
#define htol32( i )         ( i )

/******************************************************
 *                    Constants
 ******************************************************/

#define I_PC      ( 1 << 10 )  /* descriptor error */
#define I_PD      ( 1 << 11 )  /* data error */
#define I_DE      ( 1 << 12 )  /* Descriptor protocol Error */
#define I_RU      ( 1 << 13 )  /* Receive descriptor Underflow */
#define I_RO      ( 1 << 14 )  /* Receive fifo Overflow */
#define I_XU      ( 1 << 15 )  /* Transmit fifo Underflow */
#define I_RI      ( 1 << 16 )  /* Receive Interrupt */
#define I_XI      ( 1 << 24 )  /* Transmit Interrupt */
#define I_ERRORS  ( I_PC | I_PD | I_DE | I_RU | I_RO | I_XU )  /* DMA Errors */

#define WLAN_MEMORY_SIZE  ( 512 * 1024 )
#define WLAN_ADDR         ( 0x680000 )
#define RAM_SZ            ( 0x80000 )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

extern uint32_t read_intstatus          ( void );
extern void     int_enab                ( void );
extern void     dma_tx_data             ( void * data, uint32_t data_size );
extern void     reset_wlan_core         ( void );
extern void     write_reset_instruction ( uint32_t reset_inst );
extern void*    read_dma_packet         ( uint16_t** hwtag );
extern void     dma_tx_reclaim          ( void );
extern void     refill_dma              ( void );
extern int      rxactive_dma            ( void );
extern void     init_wlan_uart          ( void );
extern void     setup_pre_wlan_download ( void );
extern void     setup_dma               ( void );
extern void     deinit_sddma            ( void );
static void     boot_wlan               ( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_bool_t bus_is_up;
static wiced_bool_t wwd_bus_flow_controlled;
static uint32_t fake_backplane_window_addr = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Device data transfer functions */
wwd_result_t wwd_bus_send_buffer( wiced_buffer_t buffer )
{
    host_buffer_add_remove_at_front(&buffer, sizeof(wwd_buffer_header_t));
    dma_tx_data( buffer, host_buffer_get_current_piece_size( buffer ) );
    return WWD_SUCCESS;
}

extern resource_result_t resource_read( const resource_hnd_t* resource, uint32_t offset, uint32_t maxsize, uint32_t* size, void* buffer );

wwd_result_t  wwd_bus_write_wifi_firmware_image( void )
{
    uint32_t offset = 0;
    uint32_t total_size;
    uint32_t reset_inst = 0;
    uint32_t size_read;

    total_size = (uint32_t) resource_get_size( &wifi_firmware_image );

    resource_read ( &wifi_firmware_image, 0, 4, &size_read, &reset_inst );

    while ( total_size > offset )
    {
        resource_read ( &wifi_firmware_image, 0, WLAN_MEMORY_SIZE, &size_read, (uint8_t *)(WLAN_ADDR+offset) );
        offset += size_read;
    }

    /*
     * copy reset vector to FLOPS
     * WLAN Address = {Programmable Register[31:18],
     * Current Transaction's HADDR[17:0]}
     */
    write_reset_instruction( reset_inst );

    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_write_wifi_nvram_image( void )
{
    uint32_t varsize, varaddr, ramsz, phys_size, varsizew;
    uint32_t wlan_mem_start_addr;

    /* RAM size is 512K */
    ramsz = RAM_SZ;
    wlan_mem_start_addr = 0x680000;
    varsize = ROUNDUP(sizeof(wifi_nvram_image), 4);

    varaddr = (ramsz - 4) - varsize;

    varaddr += wlan_mem_start_addr;

    /* write Vars into WLAN RAM */
    memcpy((uint8_t *)varaddr, wifi_nvram_image, varsize);

    phys_size = ramsz;

    phys_size += wlan_mem_start_addr;

    varsize = ((phys_size - 4) - varaddr);
    varsizew = varsize / 4;
    varsizew = (~varsizew << 16) | (varsizew & 0x0000FFFF);
    varsizew = htol32(varsizew);
    memcpy((uint8_t *)(phys_size - 4), (uint8_t*)&varsizew, 4);

    return WWD_SUCCESS;
}

void boot_wlan( void )
{
#ifdef MFG_TEST_ALTERNATE_WLAN_DOWNLOAD
    external_write_wifi_firmware_and_nvram_image( );
#else
    /* Load wlan firmware from sflash */
    wwd_bus_write_wifi_firmware_image();

    /* Load nvram from sflash */
    wwd_bus_write_wifi_nvram_image( );
#endif /* ifdef MFG_TEST_ALTERNATE_WLAN_DOWNLOAD */

    /* init wlan uart */
    init_wlan_uart();

    /* Reset ARM core */
    reset_wlan_core( );

    host_rtos_delay_milliseconds( 200 );
}

wwd_result_t wwd_bus_init( void )
{

    wwd_result_t result;
    result = WWD_SUCCESS;

    host_platform_power_wifi( WICED_FALSE );
    host_platform_power_wifi( WICED_TRUE );
    setup_pre_wlan_download();
    boot_wlan();

    /*
     * The enabling of SDIO internal clock is done in WLAN firmware.
     * Doing many access across AXI-bridge without proper sequencing will lead more instability
     */
    setup_dma( );
    return result;
}

wwd_result_t wwd_bus_deinit( void )
{
    deinit_sddma();
    /* put device in reset. */
    host_platform_reset_wifi( WICED_TRUE );
    return WWD_SUCCESS;
}

uint32_t wwd_bus_packet_available_to_read(void)
{
    return 1;


}

/*
 * From internal documentation: hwnbu-twiki/SdioMessageEncapsulation
 * When data is available on the device, the device will issue an interrupt:
 * - the device should signal the interrupt as a hint that one or more data frames may be available on the device for reading
 * - the host may issue reads of the 4 byte length tag at any time -- that is, whether an interupt has been issued or not
 * - if a frame is available, the tag read should return a nonzero length (>= 4) and the host can then read the remainder of the frame by issuing one or more CMD53 reads
 * - if a frame is not available, the 4byte tag read should return zero
 */

/*@only@*//*@null@*/wwd_result_t wwd_bus_read_frame( wiced_buffer_t* buffer )
{
    uint32_t intstatus;
    void *p0 = NULL;
    uint16_t *hwtag;

    intstatus = read_intstatus();

    /* Handle DMA interrupts */
    if (intstatus & I_XI) {
        dma_tx_reclaim();
    }


    if ( rxactive_dma( ) == 0 )
    {
        refill_dma( );
        if ( rxactive_dma( ) != 0 )
        {
            int_enab();
        }
    }


        /* Handle DMA errors */
    if (intstatus & I_ERRORS) {
        refill_dma( );
        WPRINT_WWD_DEBUG(("RX errors: intstatus: 0x%x\n", (unsigned int)intstatus));
    }
    /* Handle DMA receive interrupt */
    p0 = read_dma_packet( &hwtag );
    if ( p0  == NULL)
    {

        if ( rxactive_dma( ) != 0 )
        {
            int_enab();
        }

        return WWD_NO_PACKET_TO_RECEIVE;
    }

    *buffer = p0;

    host_buffer_add_remove_at_front( buffer, - (int)sizeof(wwd_buffer_header_t) );
    wwd_sdpcm_update_credit((uint8_t*)hwtag);

    refill_dma( );
    /* where are buffers from dma_rx and dma_getnextrxp created? */

    return WWD_SUCCESS;



}

wwd_result_t wwd_bus_ensure_is_up( void )
{
    /* Ensure HT clock is up */
    if (bus_is_up == WICED_TRUE)
    {
        return WWD_SUCCESS;
    }
    bus_is_up = WICED_TRUE;
    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_allow_wlan_bus_to_sleep( void )
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

wwd_result_t wwd_bus_poke_wlan( void )
{
    return WWD_SUCCESS;
}


wwd_result_t wwd_bus_set_backplane_window( uint32_t addr )
{
    /* No such thing as a backplane window on 4390 */
    fake_backplane_window_addr = addr & (~((uint32_t)BACKPLANE_ADDRESS_MASK));
    return WWD_SUCCESS;
}

wwd_result_t wwd_bus_transfer_bytes( wwd_bus_transfer_direction_t direction, wwd_bus_function_t function, uint32_t address, uint16_t size, /*@in@*/ /*@out@*/ wwd_transfer_bytes_packet_t* data )
{
    if ( function != BACKPLANE_FUNCTION )
    {
        wiced_assert( "Only backplane available on 4390", 0 != 0 );
        return WWD_DOES_NOT_EXIST;
    }

    if ( direction == BUS_WRITE )
    {
        memcpy( (uint8_t *)(WLAN_ADDR + address + fake_backplane_window_addr), data->data, size );
        if ( address == 0 )
        {
            uint32_t resetinst = *((uint32_t*)data->data);
            write_reset_instruction( resetinst );
        }
    }
    else
    {
        memcpy( data->data, (uint8_t *)(WLAN_ADDR + address + fake_backplane_window_addr), size );
    }
    return WWD_SUCCESS;
}


