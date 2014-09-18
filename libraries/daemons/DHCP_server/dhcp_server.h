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

#include "wiced_rtos.h"
#include "wiced_tcpip.h"

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct
{
    wiced_thread_t         thread;
    wiced_udp_socket_t     socket;
    volatile wiced_bool_t  quit;
    wiced_interface_t      interface;
} wiced_dhcp_server_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

extern wiced_result_t wiced_start_dhcp_server( wiced_dhcp_server_t* server, wiced_interface_t interface );
extern wiced_result_t wiced_stop_dhcp_server ( wiced_dhcp_server_t* server );

#ifdef __cplusplus
} /* extern "C" */
#endif
