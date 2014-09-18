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

/******************************************************
 *                    Constants
 ******************************************************/

#define SAM4S_FLASH_START                    ( IFLASH0_ADDR )
#define SAM4S_LOCK_REGION_SIZE               ( IFLASH0_LOCK_REGION_SIZE )
#define SAM4S_PAGE_SIZE                      ( IFLASH0_PAGE_SIZE )
#define SAM4S_PAGES_PER_LOCK_REGION          ( SAM4S_LOCK_REGION_SIZE / SAM4S_PAGE_SIZE )

/* These come from the linker script */
extern void* dct1_start_addr_loc;
extern void* dct1_size_loc;
extern void* dct2_start_addr_loc;
extern void* dct2_size_loc;
extern void* app_hdr_start_addr_loc;
extern void* app_code_start_addr_loc;
extern void* sram_start_addr_loc;
extern void* sram_size_loc;


#define APP_HDR_START_ADDR                   ((uint32_t)&app_hdr_start_addr_loc)
#define APP_CODE_START_ADDR                  ((uint32_t)&app_code_start_addr_loc)
#define DCT1_START_ADDR                      ((uint32_t)&dct1_start_addr_loc)
#define DCT1_SIZE                            ((uint32_t)&dct1_size_loc)
#define DCT2_START_ADDR                      ((uint32_t)&dct2_start_addr_loc)
#define DCT2_SIZE                            ((uint32_t)&dct2_size_loc)
#define SRAM_START_ADDR                      ((uint32_t)&sram_start_addr_loc)
#define SRAM_SIZE                            ((uint32_t)&sram_size_loc)

/*
 * Memory Layout
 * +------------------------------+
 * |                              | Lock Region 0 (8KB)
 * +--------- Bootloader ---------+
 * |                              | Lock Region 1 (8KB)
 * +------------------------------+
 * |                              | Lock Region 2 (8KB)
 * +------------ DCT1 ------------+
 * |                              | Lock Region 3 (8KB)
 * +------------------------------+
 * |                              | Lock Region 4 (8KB)
 * +------------ DCT2 ------------+
 * |                              | Lock Region 5 (8KB)
 * +------------------------------+
 *
 * Section Definition
 * ==================
 * A sector in here refers to SAM4S LOCK REGION (8KB). This design decision is
 * made to align sector size with DCT block size and bootloader size.
 *
 * Page Definition
 * ===============
 * A page in here refers to SAM4S page (512B). SAM4S only supports page write.
 */

#define PLATFORM_DCT_COPY1_START_SECTOR      ( 2 )                           /* Lock Region 2 */
#define PLATFORM_DCT_COPY1_START_ADDRESS     ( DCT1_START_ADDR )             /*               */
#define PLATFORM_DCT_COPY1_END_SECTOR        ( 3 )                           /* Lock Region 3 */
#define PLATFORM_DCT_COPY1_END_ADDRESS       ( DCT1_START_ADDR + DCT1_SIZE ) /*               */
#define PLATFORM_DCT_COPY2_START_SECTOR      ( 4  )                          /* Lock Region 4 */
#define PLATFORM_DCT_COPY2_START_ADDRESS     ( DCT2_START_ADDR )             /*               */
#define PLATFORM_DCT_COPY2_END_SECTOR        ( 5 )                           /* Lock Region 5 */
#define PLATFORM_DCT_COPY2_END_ADDRESS       ( DCT1_START_ADDR + DCT1_SIZE ) /*               */

#define ERASE_DCT_1()                         platform_erase_flash(PLATFORM_DCT_COPY1_START_SECTOR, PLATFORM_DCT_COPY1_END_SECTOR)
#define ERASE_DCT_2()                         platform_erase_flash(PLATFORM_DCT_COPY2_START_SECTOR, PLATFORM_DCT_COPY2_END_SECTOR)


#define SAM4S_GET_LOCK_REGION_ADDR( region ) ( (region) * SAM4S_LOCK_REGION_SIZE + SAM4S_FLASH_START )
#define SAM4S_GET_PAGE_ADDR( page )          ( (page) * SAM4S_PAGE_SIZE + SAM4S_FLASH_START )
#define SAM4S_GET_PAGE_FROM_ADDR( addr )     ( (uint32_t)( ( ( addr ) - SAM4S_FLASH_START ) / SAM4S_PAGE_SIZE ) )


/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    SAM4S_FLASH_ERASE_4_PAGES  = 0x04,
    SAM4S_FLASH_ERASE_8_PAGES  = 0x08,
    SAM4S_FLASH_ERASE_16_PAGES = 0x10,
    SAM4S_FLASH_ERASE_32_PAGES = 0x20,
} sam4s_flash_erase_page_amount_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void           platform_perform_factory_reset    ( void );
static void           platform_start_ota_upgrade        ( void );
static wiced_result_t platform_dct_read_with_copy       ( void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
static wiced_result_t platform_dct_write                ( const void* info_ptr, dct_section_t section, uint32_t offset, uint32_t size );
static void           platform_reboot                   ( void );
static wiced_result_t platform_erase_app                ( void );
static wiced_result_t platform_write_app_chunk          ( uint32_t offset, const uint8_t* data, uint32_t size );
static wiced_result_t platform_set_app_valid_bit        ( app_valid_t val );
static wiced_result_t platform_read_wifi_firmware       ( uint32_t offset, void* buffer, uint32_t requested_size, uint32_t* read_size );
static void           platform_start_app                ( uint32_t vector_table_address );
static wiced_result_t platform_copy_external_to_internal( void* destination, platform_copy_src_t* copy_src, uint32_t size );
static void*          platform_get_current_dct_address  (  dct_section_t section );
static int            platform_write_dct                ( uint32_t data_start_offset, const void* data, uint32_t data_length, int8_t app_valid, void (*func)(void) );
static int            platform_erase_flash              ( uint16_t start_sector, uint16_t end_sector );
static int            platform_write_flash_chunk        ( uint32_t address, const uint8_t* data, uint32_t size );
static wiced_result_t platform_write_flash              ( uint32_t start_address, const uint8_t* data, uint32_t data_size );
static wiced_result_t platform_unlock_flash             ( uint32_t start_address, uint32_t end_address );
static wiced_result_t platform_lock_flash               ( uint32_t start_address, uint32_t end_address );


/******************************************************
 *               Variable Definitions
 ******************************************************/
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

/******************************************************
 *               Function Definitions
 ******************************************************/

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
    /* Must be implmented */
//   return platform_erase_flash( PLATFORM_APP_START_SECTOR, PLATFORM_APP_END_SECTOR );
    return WICED_SUCCESS;
}

static wiced_result_t platform_set_app_valid_bit( app_valid_t val )
{
    return platform_write_dct( 0, NULL, 0, (int8_t) val, NULL );
}

static wiced_result_t platform_write_app_chunk( uint32_t offset, const uint8_t* data, uint32_t size )
{
    return platform_write_flash_chunk( APP_CODE_START_ADDR + offset, data, size );
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


static wiced_result_t platform_erase_flash_pages( uint32_t start_page, sam4s_flash_erase_page_amount_t total_pages )
{
    uint32_t erase_result = 0;
    uint32_t argument     = 0;

    platform_watchdog_kick( );

    if ( total_pages == 32 )
    {
        start_page &= ~( 32u - 1u );
        argument = ( start_page ) | 3; /* 32 pages */
    }
    else if ( total_pages == 16 )
    {
        start_page &= ~( 16u - 1u );
        argument = ( start_page ) | 2; /* 16 pages */
    }
    else if ( total_pages == 8 )
    {
        start_page &= ~( 8u - 1u );
        argument = ( start_page ) | 1; /* 8 pages */
    }
    else
    {
        start_page &= ~( 4u - 1u );
        argument = ( start_page ) | 0; /* 4 pages */
    }

    erase_result = efc_perform_command( EFC0, EFC_FCMD_EPA, argument ) ;

    platform_watchdog_kick( );

    return ( erase_result == 0 ) ? WICED_SUCCESS : WICED_ERROR;
}

int platform_erase_flash( uint16_t start_sector, uint16_t end_sector )
{
    uint32_t start_address = SAM4S_GET_PAGE_ADDR( start_sector * SAM4S_PAGES_PER_LOCK_REGION );
    uint32_t end_address   = SAM4S_GET_PAGE_ADDR( end_sector * SAM4S_PAGES_PER_LOCK_REGION );
    uint32_t i;

    if ( platform_unlock_flash( start_address, end_address ) != WICED_SUCCESS )
    {
        return -1;
    }

    for ( i = start_sector; i <= end_sector; i++ )
    {
        if ( platform_erase_flash_pages( i * SAM4S_PAGES_PER_LOCK_REGION, SAM4S_FLASH_ERASE_16_PAGES ) != WICED_SUCCESS )
        {
            return -1;
        }
    }

    if ( platform_lock_flash( start_address, end_address ) != WICED_SUCCESS )
    {
        return -1;
    }

    return 0;
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


int platform_write_flash_chunk( uint32_t address, const uint8_t* data, uint32_t size )
{
    wiced_result_t write_result = WICED_SUCCESS;

    if ( platform_unlock_flash( address, address + size ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    write_result = platform_write_flash(address , data, size );

    if ( platform_lock_flash( address, address + size ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }


    /* Successful */
    return ( write_result == WICED_SUCCESS ) ? 0 : -1;
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
    (void) destination;
    (void) copy_src;
    (void) size;
    return WICED_SUCCESS;
}


static wiced_result_t platform_lock_flash( uint32_t start_address, uint32_t end_address )
{
    uint32_t start_page = SAM4S_GET_PAGE_FROM_ADDR( start_address );
    uint32_t end_page   = SAM4S_GET_PAGE_FROM_ADDR( end_address );

    start_page -= ( start_page % SAM4S_PAGES_PER_LOCK_REGION );

    while ( start_page <= end_page )
    {
        if ( efc_perform_command( EFC0, EFC_FCMD_SLB, start_page ) != 0 )
        {
            return WICED_ERROR;
        }

        start_page += SAM4S_PAGES_PER_LOCK_REGION;
    }

    return WICED_SUCCESS;
}

static wiced_result_t platform_unlock_flash( uint32_t start_address, uint32_t end_address )
{
    uint32_t start_page = SAM4S_GET_PAGE_FROM_ADDR( start_address );
    uint32_t end_page   = SAM4S_GET_PAGE_FROM_ADDR( end_address );

    start_page -= ( start_page % SAM4S_PAGES_PER_LOCK_REGION );

    while ( start_page <= end_page )
    {
        if ( efc_perform_command( EFC0, EFC_FCMD_CLB, start_page ) != 0 )
        {
            return WICED_ERROR;
        }

        start_page += SAM4S_PAGES_PER_LOCK_REGION;
    }

    return WICED_SUCCESS;
}

static wiced_result_t platform_write_flash( uint32_t start_address, const uint8_t* data, uint32_t data_size )
{
    uint32_t  start_page = SAM4S_GET_PAGE_FROM_ADDR( start_address );
    uint32_t  end_page   = SAM4S_GET_PAGE_FROM_ADDR( ( start_address + data_size ) );
    uint32_t* dst_ptr    = (uint32_t*)SAM4S_GET_PAGE_ADDR( start_page );
    uint8_t*  src_ptr    = (uint8_t*)dst_ptr;
    uint8_t*  data_ptr   = (uint8_t*)data;
    uint8_t*  data_start = (uint8_t*)start_address;
    uint8_t*  data_end   = data_start + data_size;
    uint32_t  page;
    uint32_t  word;
    uint32_t  byte;

    for ( page = start_page; page <= end_page; page++ )
    {
        for ( word = 0; word < 128; word++ )
        {
            uint8_t   word_to_write[4];
            uint32_t* word_to_write_ptr = (uint32_t*)word_to_write;

            UNUSED_PARAMETER( word_to_write );

            for ( byte = 0; byte < 4; byte++ )
            {
                if ( (src_ptr >= data_start) && (src_ptr < data_end) )
                {
                    word_to_write[byte] = *data_ptr++;
                }
                else
                {
                    word_to_write[byte] = *src_ptr;
                }

                src_ptr++;
            }

            /* 32-bit aligned write to the flash */
            *dst_ptr++ = *word_to_write_ptr;
        }

        /* Send write page command */

        if ( efc_perform_command( EFC0, EFC_FCMD_WP, page) != 0 )
        {
            return WICED_ERROR;
        }

    }

    return WICED_SUCCESS;
}


void platform_erase_dct( void )
{
    ERASE_DCT_1();
    ERASE_DCT_2();
}
