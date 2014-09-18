/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_WWD_RESOURCE_INTERFACE_H_
#define INCLUDED_WWD_RESOURCE_INTERFACE_H_

#include "wwd_constants.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *             Function declarations
 ******************************************************/

typedef enum
{
    WWD_RESOURCE_WLAN_FIRMWARE,
    WWD_RESOURCE_WLAN_NVRAM,
} wwd_resource_t;

extern wwd_result_t host_platform_resource_size( wwd_resource_t resource, uint32_t* size_out );

#if defined( WWD_DIRECT_RESOURCES )
extern wwd_result_t host_platform_resource_read_direct( wwd_resource_t resource, const void** ptr_out );
#else /* ! defined( WWD_DIRECT_RESOURCES ) */
extern wwd_result_t host_platform_resource_read_indirect( wwd_resource_t resource, uint32_t offset, void* buffer, uint32_t buffer_size, uint32_t* size_out );
#endif /* if defined( WWD_DIRECT_RESOURCES ) */

#ifdef __cplusplus
}
#endif

#endif /* ifndef INCLUDED_WWD_RESOURCE_INTERFACE_H_ */
