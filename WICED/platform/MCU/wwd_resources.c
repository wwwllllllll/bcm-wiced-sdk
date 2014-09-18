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
 * Defines WWD resource functions for BCM943341WCD1 platform
 */
#include "resources.h"
#include "wifi_nvram_image.h"
#include "platform/wwd_resource_interface.h"
#include "wiced_resource.h"

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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wwd_result_t host_platform_resource_size( wwd_resource_t resource, uint32_t* size_out )
{
    if ( resource == WWD_RESOURCE_WLAN_FIRMWARE )
    {
        *size_out = (uint32_t) resource_get_size( &wifi_firmware_image );
    }
    else
    {
        *size_out = sizeof( wifi_nvram_image );
    }
    return WWD_SUCCESS;
}

#if defined( WWD_DIRECT_RESOURCES )
wwd_result_t host_platform_resource_read_direct( wwd_resource_t resource, const void** ptr_out )
{
    if ( resource == WWD_RESOURCE_WLAN_FIRMWARE )
    {
        *ptr_out = wifi_firmware_image.val.mem.data;
    }
    else
    {
        *ptr_out = wifi_nvram_image;
    }
    return WWD_SUCCESS;
}
#else /* ! defined( WWD_DIRECT_RESOURCES ) */
wwd_result_t host_platform_resource_read_indirect( wwd_resource_t resource, uint32_t offset, void* buffer, uint32_t buffer_size, uint32_t* size_out )
{
    if ( resource == WWD_RESOURCE_WLAN_FIRMWARE )
    {
        return resource_read( &wifi_firmware_image, offset, buffer_size, size_out, buffer );
    }
    else
    {
        *size_out = MIN( buffer_size, sizeof( wifi_nvram_image ) - offset );
        memcpy( buffer, &wifi_nvram_image[ offset ], *size_out );
        return WWD_SUCCESS;
    }
}
#endif /* if defined( WWD_DIRECT_RESOURCES ) */
