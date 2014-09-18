/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

/**************************************************************************************************************
 * INCLUDES
 **************************************************************************************************************/

#include "wiced_tcpip.h"
#include "wiced_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************************
 * CONSTANTS
 **************************************************************************************************************/

/**************************************************************************************************************
 * STRUCTURES
 **************************************************************************************************************/

typedef struct
{
    wiced_ip_address_t ip;
    uint16_t           port;
    const char*        service_name;  /* This variable is used internally */
    char*              instance_name;
    char*              hostname;
    wiced_semaphore_t* semaphore;     /* This variable is used internally */
} gedday_service_t;

/**************************************************************************************************************
 * VARIABLES
 **************************************************************************************************************/

/**************************************************************************************************************
 * FUNCTION DECLARATIONS
 **************************************************************************************************************/

extern void           gedday_init            ( wiced_interface_t interface, const char* desired_name );
extern wiced_result_t gedday_discover_service( const char* service_query, gedday_service_t* service_result );
extern wiced_result_t gedday_add_service     ( const char* instance_name, const char* service_name, uint16_t port, uint32_t ttl, const char* txt );
extern const char*    gedday_get_hostname    ( void );
extern wiced_result_t gedday_update_ip       ( void );
extern void           gedday_deinit          ( void );

#ifdef __cplusplus
} /* extern "C" */
#endif
