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

#include "wiced.h"
#include "bt_packet_internal.h"

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

typedef enum
{
    TRANSPORT_DRIVER_INCOMING_PACKET_READY,
} bt_transport_driver_event_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef wiced_result_t (*bt_transport_driver_event_handler_t)   ( bt_transport_driver_event_t event );
typedef wiced_result_t (*bt_transport_driver_bus_read_handler_t)( bt_packet_t** packet );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_transport_driver_init( bt_transport_driver_event_handler_t event_handler,  bt_transport_driver_bus_read_handler_t bus_read_handler );

wiced_result_t bt_transport_driver_deinit( void );

wiced_result_t bt_transport_driver_send_packet( bt_packet_t* packet );

wiced_result_t bt_transport_driver_receive_packet( bt_packet_t** packet );

#ifdef __cplusplus
} /* extern "C" */
#endif
