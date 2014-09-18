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
 * Defines BCM439x WICED application framework functions
 */
#include <string.h>
#include <stdlib.h>
#include "wiced_framework.h"
#include "wiced_utilities.h"
#include "platform_peripheral.h"
#include "spi_flash.h"
#include "wwd_assert.h"
#include "wicedfs.h"
#include "platform_cmsis.h"


#define PLATFORM_DCT_COPY1_START_SECTOR      ( 0  )
#define PLATFORM_DCT_COPY1_START_ADDRESS     ( 0 )
#define PLATFORM_DCT_COPY1_END_SECTOR        ( 4 )
#define PLATFORM_DCT_COPY1_SIZE              ( 16 * 1024 )
#define PLATFORM_DCT_COPY1_END_ADDRESS       ( PLATFORM_DCT_COPY1_START_ADDRESS + PLATFORM_DCT_COPY1_SIZE )
#define PLATFORM_DCT_COPY2_START_SECTOR      ( PLATFORM_DCT_COPY1_END_SECTOR  )
#define PLATFORM_DCT_COPY2_START_ADDRESS     ( PLATFORM_DCT_COPY1_SIZE )
#define PLATFORM_DCT_COPY2_END_SECTOR        ( PLATFORM_DCT_COPY1_END_SECTOR * 2  )
#define PLATFORM_DCT_COPY2_SIZE              ( PLATFORM_DCT_COPY1_SIZE )
#define PLATFORM_DCT_COPY2_END_ADDRESS       ( PLATFORM_DCT_COPY2_START_ADDRESS + PLATFORM_DCT_COPY2_SIZE )


const uint32_t DCT_section_offsets[] =
    {
        [DCT_APP_SECTION]         = sizeof( platform_dct_data_t ),
        [DCT_SECURITY_SECTION]    = OFFSETOF( platform_dct_data_t, security_credentials ),
        [DCT_MFG_INFO_SECTION]    = OFFSETOF( platform_dct_data_t, mfg_info ),
        [DCT_WIFI_CONFIG_SECTION] = OFFSETOF( platform_dct_data_t, wifi_config ),
        [DCT_INTERNAL_SECTION]    = 0,
    };

#define PLATFORM_SFLASH_PERIPHERAL_ID (0)



extern void           platform_restore_factory_app   ( void );
extern void           platform_load_ota_app          ( void );
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



static void           platform_perform_factory_reset ( void );
static void           platform_start_ota_upgrade     ( void );
static wiced_result_t platform_dct_write             ( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
static void           platform_reboot                ( void );
static wiced_result_t platform_erase_app             ( void );
static wiced_result_t platform_write_app_chunk       ( uint32_t offset, const uint8_t* data, uint32_t size );
static wiced_result_t platform_set_app_valid_bit     ( app_valid_t val );
static wiced_result_t platform_read_wifi_firmware    ( uint32_t offset, void* buffer, uint32_t requested_size, uint32_t* read_size );
static void           platform_start_app             ( uint32_t vector_table_address );
static wiced_result_t platform_copy_external_to_internal( void* destination, platform_copy_src_t* copy_src, uint32_t size );

static void*          platform_get_current_dct_address( dct_section_t section );
static int            platform_write_dct             ( uint32_t data_start_offset, const void* data, uint32_t data_length, int8_t app_valid, void (*func)(void) );

const shared_waf_api_t shared_api_addr_loc =
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
    platform_write_dct( 0, NULL, 0, -1, platform_restore_factory_app );
    platform_reboot();
}

static void platform_start_ota_upgrade( void )
{
    platform_write_dct( 0, NULL, 0, -1, platform_load_ota_app );
    platform_reboot();
}

static void platform_reboot( void )
{
    /* Reset request */
    NVIC_SystemReset( );
}

static wiced_result_t platform_erase_app( void )
{
   return WICED_SUCCESS;
}

static wiced_result_t platform_write_app_chunk( uint32_t offset, const uint8_t* data, uint32_t size )
{
    UNUSED_PARAMETER( offset );
    UNUSED_PARAMETER( data );
    UNUSED_PARAMETER( size );

    return WICED_SUCCESS;
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

    wiced_assert( "unimplemented", 0 == 1 );
    return WICED_SUCCESS;
}



static void platform_start_app( uint32_t entry_point )
{

    /* Simulate a reset for the app: */
    /*   Switch to Thread Mode, and the Main Stack Pointer */
    /*   Set other registers to reset values (esp LR) */
    /*   Jump to the entry point */
    /*   The App will change the vector table offset address to point to its vector table */

    __asm( "MOV LR,        #0xFFFFFFFF" );
    __asm( "MOV R1,        #0x01000000" );
    __asm( "MSR APSR_nzcvq,     R1" );
    __asm( "MOV R1,        #0x00000000" );
    __asm( "MSR PRIMASK,   R1" );
    __asm( "MSR FAULTMASK, R1" );
    __asm( "MSR BASEPRI,   R1" );
    __asm( "MSR CONTROL,   R1" );


    __jump_to( entry_point );

}





/******************************************************
 *                 DCT Functions
 ******************************************************/
static wiced_result_t platform_dct_read_with_copy    ( void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size )
{
    uint32_t curr_dct  = (uint32_t)platform_get_current_dct_address( section );

    sflash_handle_t sflash_handle;
    init_sflash( &sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_NOT_ALLOWED );

    sflash_read( &sflash_handle, curr_dct + offset, info_ptr, size );

    return WICED_SUCCESS;
}

static void erase_dct_copy( const sflash_handle_t* const sflash_handle, char copy_num )
{
    uint32_t sector_num;
    uint32_t start = (copy_num==1)? PLATFORM_DCT_COPY1_START_SECTOR : PLATFORM_DCT_COPY2_START_SECTOR;
    uint32_t end   = (copy_num==1)? PLATFORM_DCT_COPY1_END_SECTOR   : PLATFORM_DCT_COPY2_END_SECTOR;

    for ( sector_num = start; sector_num < end; sector_num++)
    {
        sflash_sector_erase ( sflash_handle, sector_num * 4096 );
    }
}


static void* platform_get_current_dct_address( dct_section_t section )
{
    uint32_t sector_num;

    const platform_dct_header_t hdr =
    {
        .write_incomplete    = 0,
        .is_current_dct      = 1,
        .app_valid           = 1,
        .mfg_info_programmed = 0,
        .magic_number        = BOOTLOADER_MAGIC_NUMBER,
        .load_app_func       = NULL
    };

    platform_dct_header_t dct1_val;
    platform_dct_header_t dct2_val;
    platform_dct_header_t* dct1 = &dct1_val;
    platform_dct_header_t* dct2 = &dct2_val;
    sflash_handle_t sflash_handle;
    init_sflash( &sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_NOT_ALLOWED );
    sflash_read( &sflash_handle, PLATFORM_DCT_COPY1_START_ADDRESS, dct1, sizeof(platform_dct_header_t) );
    sflash_read( &sflash_handle, PLATFORM_DCT_COPY2_START_ADDRESS, dct2, sizeof(platform_dct_header_t) );

    if ( ( dct1->is_current_dct == 1 ) &&
         ( dct1->write_incomplete == 0 ) &&
         ( dct1->magic_number == BOOTLOADER_MAGIC_NUMBER ) )
    {
        return (void*)( PLATFORM_DCT_COPY1_START_ADDRESS +  DCT_section_offsets[section] );
    }

    if ( ( dct2->is_current_dct == 1 ) &&
         ( dct2->write_incomplete == 0 ) &&
         ( dct2->magic_number == BOOTLOADER_MAGIC_NUMBER ) )
    {
        return (void*)( PLATFORM_DCT_COPY2_START_ADDRESS + DCT_section_offsets[section] );
    }

    /* No valid DCT! */
    /* Erase the first DCT and init it. */

    wiced_assert("BOTH DCTs ARE INVALID!", 0 != 0 );

    for ( sector_num = PLATFORM_DCT_COPY1_START_SECTOR; sector_num < PLATFORM_DCT_COPY1_END_SECTOR; sector_num++)
    {
        sflash_sector_erase ( &sflash_handle, sector_num );
    }

    sflash_write( &sflash_handle, PLATFORM_DCT_COPY1_START_ADDRESS, (uint8_t*) &hdr, sizeof(hdr) );

    return (void*)( PLATFORM_DCT_COPY1_START_ADDRESS + DCT_section_offsets[section] );
}


static void copy_sflash( const sflash_handle_t* sflash_handle, uint32_t dest_loc, uint32_t src_loc, uint32_t size )
{
    char buff[64];

    while ( size > 0 )
    {
        uint32_t write_size = MIN( sizeof(buff), size);
        sflash_read( sflash_handle, src_loc, buff, write_size );
        sflash_write( sflash_handle, dest_loc, buff, write_size );

        src_loc += write_size;
        dest_loc += write_size;
        size -= write_size;
    }
}


/* TODO: Disable interrupts during function */
static int platform_write_dct( uint32_t data_start_offset, const void* data, uint32_t data_length, int8_t app_valid, void (*func)(void) )
{
    uint32_t               new_dct;
    platform_dct_header_t  curr_dct_header;
    uint32_t               bytes_after_data;
    uint8_t*               new_app_data_addr=NULL;
    uint8_t*               curr_app_data_addr=NULL;
    platform_dct_header_t* curr_dct  = &((platform_dct_data_t*)platform_get_current_dct_address( DCT_INTERNAL_SECTION ))->dct_header;
    char                   zero_byte = 0;
    sflash_handle_t        sflash_handle;
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

    /* initialise the serial flash */
    init_sflash( &sflash_handle, PLATFORM_SFLASH_PERIPHERAL_ID, SFLASH_WRITE_ALLOWED );

    /* Erase the non-current DCT */
    if ( curr_dct == PLATFORM_DCT_COPY1_START_ADDRESS )
    {
        new_dct = PLATFORM_DCT_COPY2_START_ADDRESS;
        erase_dct_copy( &sflash_handle, 2 );
    }
    else
    {
        new_dct = PLATFORM_DCT_COPY1_START_ADDRESS;
        erase_dct_copy( &sflash_handle, 1 );
    }

    /* Write the mfg data and initial part of app data before provided data */
    copy_sflash( &sflash_handle, new_dct + sizeof(platform_dct_header_t), (uint32_t) curr_dct + sizeof(platform_dct_header_t), (unsigned long) data_start_offset - sizeof(platform_dct_header_t) );

    /* Write the app data */
    sflash_write( &sflash_handle, new_dct +  data_start_offset, data, data_length );

    /* Calculate how many bytes need to be written after the end of the requested data write */
    bytes_after_data = ( PLATFORM_DCT_COPY1_SIZE ) - ( data_start_offset + data_length );

    if ( bytes_after_data != 0 )
    {
        /* There is data after end of requested write - copy it from old DCT to new DCT */
        new_app_data_addr += data_length;
        curr_app_data_addr += data_length;

        copy_sflash( &sflash_handle,
                     new_dct  + data_start_offset + data_length,
                     (uint32_t) curr_dct + data_start_offset + data_length,
                     bytes_after_data );
    }

    /* read the header from the old DCT */
    sflash_read( &sflash_handle, (uint32_t) curr_dct, &curr_dct_header, sizeof(curr_dct_header) );

    /* Copy values from old DCT header to new DCT header */
    hdr.full_size           = curr_dct_header.full_size;
    hdr.used_size           = curr_dct_header.used_size;
    memcpy( &hdr.boot_detail, &curr_dct_header.boot_detail, sizeof(boot_detail_t));
    hdr.app_valid           = (char) (( app_valid == -1 )? curr_dct_header.app_valid : app_valid);
    hdr.mfg_info_programmed = curr_dct_header.mfg_info_programmed;

    /* If a new bootload startup function has been requested, set it */
    if ( func )
    {
        hdr.load_app_func   = func;
    }
    else
    {
        hdr.load_app_func   = curr_dct_header.load_app_func;
    }

    /* Write the new DCT header data */
    sflash_write( &sflash_handle, new_dct, &hdr, sizeof(hdr) );

    /* Mark new DCT as complete and current */
    sflash_write( &sflash_handle, new_dct + OFFSETOF(platform_dct_header_t,write_incomplete), &zero_byte, sizeof(zero_byte) );
    sflash_write( &sflash_handle, (unsigned long) curr_dct + OFFSETOF(platform_dct_header_t,is_current_dct), &zero_byte, sizeof(zero_byte) );

    return 0;
}






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
    return (size == wicedfs_fread( destination, size, 1, copy_src->val.file_handle ) )? WICED_SUCCESS : WICED_ERROR;
}



void platform_restore_factory_app( void )
{
    wiced_assert( "unimplemented", 0 == 1 );
}

void platform_load_ota_app( void )
{
    wiced_assert( "unimplemented", 0 == 1 );
}
