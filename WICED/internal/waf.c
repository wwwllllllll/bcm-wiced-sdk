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
#include "platform_config.h" /* Needed for EXTERNAL_DCT */
#include "wiced_framework.h"
#include "wiced_utilities.h"

/** @file
 *
 */

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_dct_read_lock( void** info_ptr, wiced_bool_t ptr_is_writable, dct_section_t section, uint32_t offset, uint32_t size )
{
#ifdef EXTERNAL_DCT

    UNUSED_PARAMETER( ptr_is_writable );

    *info_ptr = malloc_named( "DCT", size );
    return shared_waf_api->platform_dct_read_with_copy( *info_ptr, section, offset, size);
#else /* ifdef EXTERNAL_DCT */
    if ( ptr_is_writable == WICED_TRUE )
    {
        *info_ptr = (void*)malloc_named( "DCT", size );
        if ( *info_ptr == NULL )
        {
            return WICED_ERROR;
        }
        shared_waf_api->platform_dct_read_with_copy( *info_ptr, section, offset, size );
    }
    else
    {
        *info_ptr = (char*)shared_waf_api->platform_get_current_dct_address( section ) + offset;
    }
    return WICED_SUCCESS;
#endif /* ifdef EXTERNAL_DCT */
}

wiced_result_t wiced_dct_read_unlock( void* info_ptr, wiced_bool_t ptr_is_writable )
{
#ifdef EXTERNAL_DCT

    UNUSED_PARAMETER( ptr_is_writable );

    free( info_ptr );
#else
    if ( ptr_is_writable == WICED_TRUE )
    {
        free( info_ptr );
    }
#endif /* ifdef EXTERNAL_DCT */
    return WICED_SUCCESS;
}

#ifndef EXTERNAL_DCT

void const* wiced_dct_get_app_section( void )
{
    void* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( &info_ptr, WICED_FALSE, DCT_APP_SECTION, 0, 1 ) )
    {
        return NULL;
    }
    return info_ptr;
}

platform_dct_mfg_info_t const* wiced_dct_get_mfg_info_section( void )
{
    platform_dct_mfg_info_t* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( (void**) &info_ptr, WICED_FALSE, DCT_MFG_INFO_SECTION, 0, 1 ) )
    {
        return NULL;
    }
    return info_ptr;
}

platform_dct_security_t const* wiced_dct_get_security_section( void )
{
    platform_dct_security_t* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( (void**) &info_ptr, WICED_FALSE, DCT_SECURITY_SECTION, 0, 1 ) )
    {
        return NULL;
    }
    return info_ptr;
}

platform_dct_wifi_config_t const* wiced_dct_get_wifi_config_section( void )
{
    platform_dct_wifi_config_t* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( (void**) &info_ptr, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, 0, 1 ) )
    {
        return NULL;
    }
    return info_ptr;
}

wiced_result_t wiced_dct_read_security_section( platform_dct_security_t* security_dct )
{
    platform_dct_security_t* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( (void**) &info_ptr, WICED_FALSE, DCT_SECURITY_SECTION, 0, 1 ) )
    {
        return WICED_ERROR;
    }
    memcpy( security_dct, info_ptr, sizeof( *security_dct ) );
    return WICED_SUCCESS;
}

wiced_result_t wiced_dct_write_security_section( const platform_dct_security_t* security_dct )
{
    return wiced_dct_write( security_dct, DCT_SECURITY_SECTION, 0, sizeof( *security_dct ) );
}

wiced_result_t wiced_dct_read_wifi_config_section( platform_dct_wifi_config_t* wifi_config_dct )
{
    platform_dct_wifi_config_t* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( (void**) &info_ptr, WICED_FALSE, DCT_WIFI_CONFIG_SECTION, 0, 1 ) )
    {
        return WICED_ERROR;
    }
    memcpy( wifi_config_dct, info_ptr, sizeof( *wifi_config_dct ) );
    return WICED_SUCCESS;
}

wiced_result_t wiced_dct_write_wifi_config_section( const platform_dct_wifi_config_t* wifi_config_dct )
{
    return wiced_dct_write( wifi_config_dct, DCT_SECURITY_SECTION, 0, sizeof( *wifi_config_dct ) );
}

wiced_result_t wiced_dct_read_app_section( void* app_dct, uint32_t size )
{
    void* info_ptr = NULL;
    if ( WICED_SUCCESS != wiced_dct_read_lock( &info_ptr, WICED_FALSE, DCT_APP_SECTION, 0, size ) )
    {
        return WICED_ERROR;
    }
    memcpy( app_dct, info_ptr, size );
    return WICED_SUCCESS;
}

wiced_result_t wiced_dct_write_app_section( const void* app_dct, uint32_t size )
{
    return wiced_dct_write( app_dct, DCT_APP_SECTION, 0, size );
}

#endif /* ifndef EXTERNAL_DCT */
