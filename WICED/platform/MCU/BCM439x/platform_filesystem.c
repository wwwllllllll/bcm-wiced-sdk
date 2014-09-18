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
 * Defines BCM439x filesystem
 */
#include "stdint.h"
#include "string.h"
#include "platform_init.h"
#include "platform_peripheral.h"
#include "platform_mcu_peripheral.h"
#include "platform_stdio.h"
#include "platform_sleep.h"
#include "platform_config.h"
#include "platform_sflash_dct.h"
#include "wwd_constants.h"
#include "wwd_rtos.h"
#include "wwd_assert.h"
#include "RTOS/wwd_rtos_interface.h"
#include "spi_flash.h"
#include "wicedfs.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

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

extern uint32_t        sfi_read      ( uint32_t addr, uint32_t len, const uint8_t *buffer )   __attribute__((long_call));
extern uint32_t        sfi_write     ( uint32_t addr, uint32_t len, const uint8_t *buffer )   __attribute__((long_call));
extern uint32_t        sfi_size      ( void )                                                 __attribute__((long_call));
extern void            sfi_erase     ( uint32_t addr, uint32_t len )                          __attribute__((long_call));
static wicedfs_usize_t read_callback ( void* user_param, void* buf, wicedfs_usize_t size, wicedfs_usize_t pos );

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern char bcm439x_platform_inited;

host_semaphore_type_t sflash_mutex;
sflash_handle_t       wicedfs_sflash_handle;
wiced_filesystem_t    resource_fs_handle;

/******************************************************
 *               Function Definitions
 ******************************************************/

void platform_filesystem_init( void )
{
    int result;

    host_rtos_init_semaphore( &sflash_mutex );
    host_rtos_set_semaphore ( &sflash_mutex, WICED_FALSE );

    init_sflash( &wicedfs_sflash_handle, 0, SFLASH_WRITE_NOT_ALLOWED );

    result = wicedfs_init( FILESYSTEM_BASE_ADDR, read_callback, &resource_fs_handle, &wicedfs_sflash_handle );
    wiced_assert( "wicedfs init fail", result == 0 );
    REFERENCE_DEBUG_ONLY_VARIABLE( result );
}

static wicedfs_usize_t read_callback( void* user_param, void* buf, wicedfs_usize_t size, wicedfs_usize_t pos )
{
    int retval;
    retval = sflash_read( (const sflash_handle_t*) user_param, ( pos ), ( buf ), ( size ) );

    return ( ( 0 == retval ) ? size : 0 );
}

#ifndef SPI_DRIVER_SFLASH
int init_sflash( sflash_handle_t* const handle, void* peripheral_id, sflash_write_allowed_t write_allowed )
{
    UNUSED_PARAMETER( handle );
    UNUSED_PARAMETER( peripheral_id );
    UNUSED_PARAMETER( write_allowed );

    platform_ptu2_enable_serial_flash();

    if ( sfi_size( ) )
    {
        platform_ptu2_disable_serial_flash();
        return 0;
    }

    platform_ptu2_disable_serial_flash();
    return -1;
}

int sflash_read( const sflash_handle_t* const handle, unsigned long device_address, void* const data_addr, unsigned int size )
{
    uint32_t num_read;

    UNUSED_PARAMETER( handle );

    if ( bcm439x_platform_inited )
    {
        host_rtos_get_semaphore( &sflash_mutex, NEVER_TIMEOUT, WICED_FALSE );
    }

    platform_ptu2_enable_serial_flash();
    num_read = sfi_read( device_address, size, data_addr );
    platform_ptu2_disable_serial_flash();

    if ( bcm439x_platform_inited )
    {
        host_rtos_set_semaphore( &sflash_mutex, WICED_FALSE );
    }

    return ( num_read == size ) ? 0 : -1;
}

int sflash_write( const sflash_handle_t* const handle, unsigned long device_address, const void* const data_addr, unsigned int size )
{
    uint32_t num_written;

    UNUSED_PARAMETER( handle );

    if ( bcm439x_platform_inited )
    {
        host_rtos_get_semaphore( &sflash_mutex, NEVER_TIMEOUT, WICED_FALSE );
    }

    platform_ptu2_enable_serial_flash();
    num_written = sfi_write( device_address, size, data_addr );
    platform_ptu2_disable_serial_flash();

    if ( bcm439x_platform_inited )
    {
        host_rtos_set_semaphore( &sflash_mutex, WICED_FALSE );
    }
    return ( num_written == size ) ? 0 : -1;
}

int sflash_chip_erase( const sflash_handle_t* const handle )
{
    UNUSED_PARAMETER( handle );

    if ( bcm439x_platform_inited )
    {
        host_rtos_get_semaphore( &sflash_mutex, NEVER_TIMEOUT, WICED_FALSE );
    }

    platform_ptu2_enable_serial_flash();
    sfi_erase( 0, sfi_size( ) );
    platform_ptu2_disable_serial_flash();

    if ( bcm439x_platform_inited )
    {
        host_rtos_set_semaphore( &sflash_mutex, WICED_FALSE );
    }
    return 0;
}

int sflash_sector_erase( const sflash_handle_t* const handle, unsigned long device_address )
{
    UNUSED_PARAMETER( handle );

    if ( bcm439x_platform_inited )
    {
        host_rtos_get_semaphore( &sflash_mutex, NEVER_TIMEOUT, WICED_FALSE );
    }

    platform_ptu2_enable_serial_flash();
    sfi_erase( device_address, 4096 );
    platform_ptu2_disable_serial_flash();

    if ( bcm439x_platform_inited )
    {
        host_rtos_set_semaphore( &sflash_mutex, WICED_FALSE );
    }
    return 0;
}

int sflash_get_size( const sflash_handle_t* const handle, unsigned long* size )
{
    UNUSED_PARAMETER( handle );

//    *size = sfi_size();
    *size = 2 * 1024 * 1024;
    return 0;
}
#endif /* ifndef SPI_DRIVER_SFLASH */

platform_result_t platform_get_sflash_dct_loc( sflash_handle_t* sflash_handle, uint32_t* loc )
{
    UNUSED_PARAMETER( sflash_handle );

    *loc = 0;
    return PLATFORM_SUCCESS;
}
