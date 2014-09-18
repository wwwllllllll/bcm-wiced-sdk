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

#if defined( WWD_DYNAMIC_NVRAM )
#define NVRAM_SIZE             dynamic_nvram_size
#define NVRAM_IMAGE_VARIABLE   dynamic_nvram_image
#else
#define NVRAM_SIZE             sizeof( wifi_nvram_image )
#define NVRAM_IMAGE_VARIABLE   wifi_nvram_image
#endif

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

#if defined( WWD_DYNAMIC_NVRAM )
uint32_t dynamic_nvram_size  = sizeof( wifi_nvram_image );
void*    dynamic_nvram_image = &wifi_nvram_image;
#endif

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
        *size_out = NVRAM_SIZE;
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
        *ptr_out = NVRAM_IMAGE_VARIABLE;
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
        *size_out = MIN( buffer_size, NVRAM_SIZE - offset );
        memcpy( buffer, &NVRAM_IMAGE_VARIABLE[ offset ], *size_out );
        return WWD_SUCCESS;
    }
}
#endif /* if defined( WWD_DIRECT_RESOURCES ) */
