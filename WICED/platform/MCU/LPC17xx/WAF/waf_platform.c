/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include <string.h>
#include <stdlib.h>
#include "spi_flash.h"
#include "platform_config.h"
#include "platform_peripheral.h"
#include "wwd_assert.h"
#include "wiced_framework.h"

#define PLATFORM_APP_START_SECTOR      ( 0  )
#define PLATFORM_APP_END_SECTOR        ( 29 )
#define APP_CODE_START_ADDR   ((uint32_t)&app_code_start_addr_loc)
#define DCT1_START_ADDR  ((uint32_t)&dct1_start_addr_loc)
#define DCT1_SIZE        ((uint32_t)&dct1_size_loc)
#define DCT2_START_ADDR  ((uint32_t)&dct2_start_addr_loc)
#define DCT2_SIZE        ((uint32_t)&dct2_size_loc)
#define SRAM_START_ADDR  ((uint32_t)&sram_start_addr_loc)
#define SRAM_SIZE        ((uint32_t)&sram_size_loc)

#define PLATFORM_DCT_COPY1_START_SECTOR      ( 2  )
#define PLATFORM_DCT_COPY1_START_ADDRESS     ( DCT1_START_ADDR )
#define PLATFORM_DCT_COPY1_END_SECTOR        ( 4 )
#define PLATFORM_DCT_COPY1_END_ADDRESS       ( DCT1_START_ADDR + DCT1_SIZE )
#define PLATFORM_DCT_COPY2_START_SECTOR      ( 4  )
#define PLATFORM_DCT_COPY2_START_ADDRESS     ( DCT2_START_ADDR )
#define PLATFORM_DCT_COPY2_END_SECTOR        ( 6 )
#define PLATFORM_DCT_COPY2_END_ADDRESS       ( DCT1_START_ADDR + DCT1_SIZE )


#define ERASE_DCT_1()              platform_erase_flash(PLATFORM_DCT_COPY1_START_SECTOR, PLATFORM_DCT_COPY1_END_SECTOR)
#define ERASE_DCT_2()              platform_erase_flash(PLATFORM_DCT_COPY2_START_SECTOR, PLATFORM_DCT_COPY2_END_SECTOR)


/* These come from the linker script */
extern void* dct1_start_addr_loc;
extern void* dct1_size_loc;
extern void* dct2_start_addr_loc;
extern void* dct2_size_loc;
extern void* app_code_start_addr_loc;
extern void* sram_start_addr_loc;
extern void* sram_size_loc;

extern void  platform_read_dct ( uint16_t offset, void* buffer, uint16_t buffer_length );
//extern int   platform_write_dct( uint16_t data_start_offset, const void* data, uint16_t data_length, int8_t app_valid, void (*func)(void) );
//extern int   platform_erase_flash( uint16_t start_sector, uint16_t end_sector );
//extern int   platform_write_flash_chunk( uint32_t address, const uint8_t* data, uint32_t size );
extern void platform_restore_factory_app( void );
extern void platform_load_ota_app( void );

static void           platform_perform_factory_reset ( void );
static void           platform_start_ota_upgrade     ( void );
static wiced_result_t platform_dct_read_with_copy    ( void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
static wiced_result_t platform_dct_write             ( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
static void           platform_reboot                ( void );
static wiced_result_t platform_erase_app             ( void );
static wiced_result_t platform_write_app_chunk       ( uint32_t offset, const uint8_t* data, uint32_t size );
static wiced_result_t platform_set_app_valid_bit     ( app_valid_t val );
static wiced_result_t platform_read_wifi_firmware    ( uint32_t offset, void* buffer, uint32_t requested_size, uint32_t* read_size );
static void           platform_start_app             ( uint32_t vector_table_address );
static wiced_result_t platform_copy_external_to_internal( void* destination, platform_copy_src_t* copy_src, uint32_t size );

static void* platform_get_current_dct_address(  dct_section_t section );
static int platform_write_dct( uint32_t data_start_offset, const void* data, uint32_t data_length, int8_t app_valid, void (*func)(void) );
static int platform_erase_flash( uint16_t start_sector, uint16_t end_sector );
static int platform_write_flash_chunk( uint32_t address, const uint8_t* data, uint32_t size );

 const shared_waf_api_t shared_waf_api_val =
{
    .platform_perform_factory_reset     = platform_perform_factory_reset,
    .platform_start_ota_upgrade         = platform_start_ota_upgrade,
    .platform_dct_read_with_copy        = platform_dct_read_with_copy,
    .platform_get_current_dct_address   = platform_get_current_dct_address,
    .platform_dct_write                 = platform_dct_write,
    .platform_reboot                    = platform_reboot,
    .platform_erase_app                 = platform_erase_app,
    .platform_write_app_chunk           = platform_write_app_chunk,
    .platform_set_app_valid_bit         = platform_set_app_valid_bit,
    .platform_read_wifi_firmware        = platform_read_wifi_firmware,
    .platform_start_app                 = platform_start_app,
    .platform_copy_external_to_internal = platform_copy_external_to_internal
};

const uint32_t DCT_section_offsets[] =
    {
        [DCT_APP_SECTION]         = sizeof( platform_dct_data_t ),
        [DCT_SECURITY_SECTION]    = OFFSETOF( platform_dct_data_t, security_credentials ),
        [DCT_MFG_INFO_SECTION]    = OFFSETOF( platform_dct_data_t, mfg_info ),
        [DCT_WIFI_CONFIG_SECTION] = OFFSETOF( platform_dct_data_t, wifi_config ),
        [DCT_INTERNAL_SECTION]    = 0,
    };


//const shared_waf_api_t* shared_waf_api = &shared_waf_api_val;



#if defined ( __ICCARM__ )

static inline void __jump_to( uint32_t addr )
{
    __asm( "MOV R1, #0x00000001" );
    __asm( "ORR R0, R1, #0" );  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
    __asm( "BX R0" );
}

#elif defined ( __GNUC__ )

__attribute__( ( always_inline ) ) static __INLINE void __jump_to( uint32_t addr )
{
    addr |= 0x00000001;  /* Last bit of jump address indicates whether destination is Thumb or ARM code */
  __ASM volatile ("BX %0" : : "r" (addr) );
}

#endif


static void platform_perform_factory_reset( void )
{
    platform_write_dct( 0, NULL, 0, -1, NULL ); //platform_restore_factory_app );
    platform_reboot();
}

static void platform_start_ota_upgrade( void )
{
    platform_write_dct( 0, NULL, 0, -1, NULL ); //platform_load_ota_app );
    platform_reboot();
}

static void platform_reboot( void )
{
    /* Reset request */
    NVIC_SystemReset( );
}

static wiced_result_t platform_erase_app( void )
{
   return platform_erase_flash( PLATFORM_APP_START_SECTOR, PLATFORM_APP_END_SECTOR );
}

static wiced_result_t platform_write_app_chunk( uint32_t offset, const uint8_t* data, uint32_t size )
{
    return platform_write_flash_chunk( APP_CODE_START_ADDR + offset, data, size );
}

static wiced_result_t platform_set_app_valid_bit( app_valid_t val )
{
    return platform_write_dct( 0, NULL, 0, (int8_t) val, NULL );
}


static wiced_result_t platform_read_wifi_firmware( uint32_t offset, void* buffer, uint32_t requested_size, uint32_t* read_size )
{
    (void) offset;
    (void) buffer;
    (void) requested_size;
    (void) read_size;

#if 0
    int status;
    sflash_handle_t sflash_handle;
    bootloader_app_header_t image_header;

    *read_size = 0;

    /* Initialise the serial flash */
    if ( 0 != ( status = init_sflash( &sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_NOT_ALLOWED ) ) )
    {
        return status;
    }

    /* Read the image size from the serial flash */
    if ( 0 != ( status = sflash_read( &sflash_handle, APP_IMAGE_LOCATION_IN_SFLASH, &image_header, sizeof( image_header ) ) ) )
    {
        return status;
    }

    if ( offset > image_header.size_of_wlan_firmware )
    {
        return -1;
    }

    if ( offset + requested_size > image_header.size_of_wlan_firmware )
    {
        requested_size = image_header.size_of_wlan_firmware - offset;
    }

    offset += image_header.address_of_wlan_firmware - APP_CODE_START_ADDR;

    if ( 0 != ( status = sflash_read( &sflash_handle, offset, buffer, requested_size ) ) )
    {
        return status;
    }

    *read_size = requested_size;
#endif
    return WICED_SUCCESS;
}



static void platform_start_app( uint32_t entry_point )
{

    /* Simulate a reset for the app: */
    /*   Switch to Thread Mode, and the Main Stack Pointer */
    /*   Change the vector table offset address to point to the app vector table */
    /*   Set other registers to reset values (esp LR) */
    /*   Jump to the reset vector */


    if ( entry_point == 0 )
    {
        uint32_t* vector_table =  (uint32_t*) APP_CODE_START_ADDR;
        entry_point = vector_table[1];
    }


    __asm( "MOV LR,        #0xFFFFFFFF" );
    __asm( "MOV R1,        #0x01000000" );
    __asm( "MSR APSR_nzcvq,     R1" );
    __asm( "MOV R1,        #0x00000000" );
    __asm( "MSR PRIMASK,   R1" );
    __asm( "MSR FAULTMASK, R1" );
    __asm( "MSR BASEPRI,   R1" );
    __asm( "MSR CONTROL,   R1" );

/*  Now rely on the app crt0 to load VTOR / Stack pointer

    SCB->VTOR = vector_table_address; - Change the vector table to point to app vector table
    __set_MSP( *stack_ptr ); */

    __jump_to( entry_point );

}





/******************************************************
 *                 DCT Functions
 ******************************************************/
static wiced_result_t platform_dct_read_with_copy( void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size )
{
    char* curr_dct  = (char*) platform_get_current_dct_address( section );

    memcpy( info_ptr, &curr_dct[ offset ], size );

    return WICED_SUCCESS;
}

static void* platform_get_current_dct_address( dct_section_t section )
{
    const platform_dct_header_t hdr =
    {
        .write_incomplete    = 0,
        .is_current_dct      = 1,
        .app_valid           = 1,
        .mfg_info_programmed = 0,
        .magic_number        = BOOTLOADER_MAGIC_NUMBER,
        .load_app_func       = NULL
    };

#ifdef EXTERNAL_DCT
    platform_dct_header_t dct1_val;
    platform_dct_header_t dct2_val;
    platform_dct_header_t* dct1 = &dct1_val;
    platform_dct_header_t* dct2 = &dct2_val;
    sflash_handle_t sflash_handle;
    init_sflash( sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_NOT_ALLOWED );
    sflash_read( sflash_handle, PLATFORM_DCT_COPY1_START_ADDRESS, dct1_val, sizeof(platform_dct_header_t) );
    sflash_read( sflash_handle, PLATFORM_DCT_COPY2_START_ADDRESS, dct2_val, sizeof(platform_dct_header_t) );
#else
    platform_dct_header_t* dct1 = ((platform_dct_header_t*) PLATFORM_DCT_COPY1_START_ADDRESS);
    platform_dct_header_t* dct2 = ((platform_dct_header_t*) PLATFORM_DCT_COPY2_START_ADDRESS);
#endif /* ifdef EXTERNAL_DCT */

    if ( ( dct1->is_current_dct == 1 ) &&
         ( dct1->write_incomplete == 0 ) &&
         ( dct1->magic_number == BOOTLOADER_MAGIC_NUMBER ) )
    {
        return &((char*)dct1)[ DCT_section_offsets[section] ];
    }

    if ( ( dct2->is_current_dct == 1 ) &&
         ( dct2->write_incomplete == 0 ) &&
         ( dct2->magic_number == BOOTLOADER_MAGIC_NUMBER ) )
    {
        return &((char*)dct2)[ DCT_section_offsets[section] ];
    }

    /* No valid DCT! */
    /* Erase the first DCT and init it. */

    wiced_assert("BOTH DCTs ARE INVALID!", 0 != 0 );

    ERASE_DCT_1();

#ifdef EXTERNAL_DCT
    sflash_sector_erase ( sflash_handle, PLATFORM_DCT_COPY1_START_SECTOR )

    sflash_read( sflash_handle, PLATFORM_DCT_COPY1_START_ADDRESS, dct1_val, sizeof(platform_dct_header_t) );
#define ERASE_DCT_1()              platform_erase_flash(PLATFORM_DCT_COPY1_START_SECTOR, PLATFORM_DCT_COPY1_END_SECTOR)
#define ERASE_DCT_2()              platform_erase_flash(PLATFORM_DCT_COPY2_START_SECTOR, PLATFORM_DCT_COPY2_END_SECTOR)
#else
    platform_erase_flash(PLATFORM_DCT_COPY1_START_SECTOR, PLATFORM_DCT_COPY1_END_SECTOR);
#endif /* ifdef EXTERNAL_DCT */

//    platform_bootloader_erase_dct( 1 );
#ifdef EXTERNAL_DCT

#endif /* ifdef EXTERNAL_DCT */

    platform_write_flash_chunk( PLATFORM_DCT_COPY1_START_ADDRESS, (uint8_t*) &hdr, sizeof(hdr) );

    return &((char*)dct1)[ DCT_section_offsets[section] ];
}


/* TODO: Disable interrupts during function */
/* Note Function allocates a chunk of memory for the bootloader data on the stack - ensure the stack is big enough */
static int platform_write_dct( uint32_t data_start_offset, const void* data, uint32_t data_length, int8_t app_valid, void (*func)(void) )
{
    platform_dct_header_t* new_dct;
    uint32_t               bytes_after_data;
    uint8_t*               new_app_data_addr;
    uint8_t*               curr_app_data_addr;
    platform_dct_header_t* curr_dct  = &((platform_dct_data_t*)platform_get_current_dct_address( DCT_INTERNAL_SECTION ))->dct_header;
    char                   zero_byte = 0;
    platform_dct_header_t  hdr =
    {
        .write_incomplete = 1,
        .is_current_dct   = 1,
        .magic_number     = BOOTLOADER_MAGIC_NUMBER
    };

    /* Check if the data is too big to write */
    if ( data_length + data_start_offset > ( PLATFORM_DCT_COPY1_END_ADDRESS - PLATFORM_DCT_COPY1_START_ADDRESS ) )
    {
        return -1;
    }

    /* Erase the non-current DCT */
    if ( curr_dct == ((platform_dct_header_t*)PLATFORM_DCT_COPY1_START_ADDRESS) )
    {
        new_dct = (platform_dct_header_t*)PLATFORM_DCT_COPY2_START_ADDRESS;
        ERASE_DCT_2();
    }
    else
    {
        new_dct = (platform_dct_header_t*)PLATFORM_DCT_COPY1_START_ADDRESS;
        ERASE_DCT_1();
    }

    data_start_offset -= sizeof( platform_dct_header_t );
    /* Write the mfg data and initial part of app data before provided data */
    platform_write_flash_chunk( ((uint32_t) &new_dct[1]), (uint8_t*) &curr_dct[1], data_start_offset );

    /* Verify the mfg data */
    if ( memcmp( &new_dct[1], &curr_dct[1], data_start_offset ) != 0 )
    {
        return -2;
    }

    /* Write the app data */
    new_app_data_addr  =  (uint8_t*) &new_dct[1]  + data_start_offset;
    curr_app_data_addr =  (uint8_t*) &curr_dct[1] + data_start_offset;

    platform_write_flash_chunk( (uint32_t)new_app_data_addr, data, data_length );

    /* Verify the app data */
    if ( memcmp( new_app_data_addr, data, data_length ) != 0 )
    {
        /* Error writing app data */
        return -3;
    }

    bytes_after_data = ( PLATFORM_DCT_COPY1_END_ADDRESS - PLATFORM_DCT_COPY1_START_ADDRESS ) - (sizeof(platform_dct_header_t) + data_start_offset + data_length );

    if ( bytes_after_data != 0 )
    {
        new_app_data_addr += data_length;
        curr_app_data_addr += data_length;

        platform_write_flash_chunk( (uint32_t)new_app_data_addr, curr_app_data_addr, bytes_after_data );
        /* Verify the app data */
        if ( 0 != memcmp( new_app_data_addr, curr_app_data_addr, bytes_after_data ) )
        {
            /* Error writing app data */
            return -4;
        }
    }

    hdr.app_valid           = (char) (( app_valid == -1 )? curr_dct->app_valid : app_valid);
    hdr.load_app_func       = func;
    hdr.mfg_info_programmed = curr_dct->mfg_info_programmed;

    /* Write the header data */
    platform_write_flash_chunk( (uint32_t)new_dct, (uint8_t*) &hdr, sizeof(hdr) );

    /* Verify the header data */
    if ( memcmp( new_dct, &hdr, sizeof(hdr) ) != 0 )
    {
        /* Error writing header data */
        return -5;
    }

    /* Mark new DCT as complete and current */
    platform_write_flash_chunk( (uint32_t)&new_dct->write_incomplete, (uint8_t*) &zero_byte, sizeof(zero_byte) );
    platform_write_flash_chunk( (uint32_t)&curr_dct->is_current_dct, (uint8_t*) &zero_byte, sizeof(zero_byte) );

    return 0;
}

static int platform_erase_flash( uint16_t start_sector, uint16_t end_sector )
{

    UNUSED_PARAMETER(start_sector);
    UNUSED_PARAMETER(end_sector);
    return 0;
}

static int platform_write_flash_chunk( uint32_t address, const uint8_t* data, uint32_t size )
{

    UNUSED_PARAMETER(address);
    UNUSED_PARAMETER(data);
    UNUSED_PARAMETER(size);

    return 0;
}




/* TODO: Disable interrupts during function */
/* Note Function allocates a chunk of memory for the bootloader data on the stack - ensure the stack is big enough */
static wiced_result_t platform_dct_write( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size )
{
    int retval;

    retval = platform_write_dct( DCT_section_offsets[section] + offset, info_ptr, size, 1, NULL );
    (void) info_ptr;
    (void) section;
    (void) offset;
    (void) size;
    return (retval == 0)? WICED_SUCCESS : WICED_ERROR;
}

static wiced_result_t platform_copy_external_to_internal( void* destination, platform_copy_src_t* copy_src, uint32_t size )
{
    (void) destination;
    (void) copy_src;
    (void) size;
    return WICED_SUCCESS;
}



void platform_restore_factory_app( void )
{
#if 0
    sflash_handle_t sflash_handle;
    bootloader_app_header_t image_header;
    static uint8_t  rx_buffer[4096]; /* API Function will not return, so it is safe to declare static big buffer */
    uint32_t write_address = APP_CODE_START_ADDR;
    platform_dct_header_t dct_header;
    uint32_t progress;

    platform_set_bootloader_led( 1 );

    /* Mark the App image as invalid to guard against power failure during writing */
    platform_set_app_valid_bit( APP_INVALID );


    platform_watchdog_kick( );

    /* Erase the internal flash memory */
    platform_erase_app( );


    platform_watchdog_kick( );


    /* Init the external SPI flash */
    init_sflash( &sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_NOT_ALLOWED );

    platform_watchdog_kick( );

    /* Read the image header */
    sflash_read( &sflash_handle, APP_IMAGE_LOCATION_IN_SFLASH, &image_header, sizeof( image_header ) );

    platform_watchdog_kick( );

    /* Quick image validation */

    /* Flash the factory app */
    progress = 0;
    while ( progress < image_header.size_of_app )
    {
        /* Read a chunk of image data */
        uint32_t read_size = MIN(image_header.size_of_app - progress, sizeof(rx_buffer));
        sflash_read( &sflash_handle, APP_IMAGE_LOCATION_IN_SFLASH + progress, &rx_buffer, read_size );

        /* Write it to the flash */
        platform_write_flash_chunk( write_address, rx_buffer, read_size );

        write_address += read_size;
        progress      += read_size;
        platform_watchdog_kick( );
    }

    /* Read the DCT header (starts immediately after app */
    sflash_read( &sflash_handle, image_header.size_of_app, &dct_header, sizeof( platform_dct_header_t ) );

    /* Erase the DCT */
    platform_erase_dct( );

    /* Flash the factory DCT */
    write_address = DCT1_START_ADDR;
    progress = 0;
    while ( progress < dct_header.used_size )
    {
        /* Read a chunk of dct image data */
        uint32_t read_size = MIN(dct_header.used_size - progress, sizeof(rx_buffer));
        sflash_read( &sflash_handle, image_header.size_of_app + progress, &rx_buffer, read_size );

        /* Write it to the flash */
        platform_write_flash_chunk( write_address, rx_buffer, read_size );

        write_address += read_size;
        progress      += read_size;
        platform_watchdog_kick( );
    }

    platform_set_app_valid_bit( APP_VALID );
    platform_start_app( 0 );
#endif
}

void platform_load_ota_app( void )
{
#if 0
    sflash_handle_t sflash_handle;
    bootloader_app_header_t image_header;
    uint32_t start_of_ota_image;
    uint32_t start_of_ota_app;
    platform_dct_header_t dct_header;

    /* Move the stack so that it does not get overwritten when the OTA upgrade app is copied into memory */
    platform_set_load_stack( );

    /* Init the external SPI flash */
    init_sflash( &sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_NOT_ALLOWED );

    platform_watchdog_kick( );

    /* Read the factory app image header */
    sflash_read( &sflash_handle, APP_IMAGE_LOCATION_IN_SFLASH, &image_header, sizeof( image_header ) );

    platform_watchdog_kick( );

    /* Read the DCT header (starts immediately after app */
    sflash_read( &sflash_handle, image_header.size_of_app, &dct_header, sizeof( platform_dct_header_t ) );

    platform_watchdog_kick( );

    /* Read the image header of the OTA application which starts at the end of the dct image */
    start_of_ota_image = image_header.size_of_app + dct_header.full_size;
    sflash_read( &sflash_handle, APP_IMAGE_LOCATION_IN_SFLASH + start_of_ota_image, &image_header, sizeof( image_header ) );
    start_of_ota_app = start_of_ota_image + image_header.offset_to_vector_table;

    platform_watchdog_kick( );

    /* Write the OTA app */
    sflash_read( &sflash_handle, start_of_ota_app, (void*)SRAM_START_ADDR, image_header.size_of_app );

    platform_watchdog_kick( );

    platform_start_app( SRAM_START_ADDR );
#endif
}
