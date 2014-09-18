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

/* Many of the callbacks of higher layer Bluetooth protocols run on WICED_NETWORK_WORKER_PRIORITY
 * context. Bluetooth transport thread priority is to 1 higher than that of
 * WICED_NETWORK_WORKER_PRIORITY to let it preempt WICED_NETWORK_WORKER_PRIORITY.
 */
#define BT_TRANSPORT_THREAD_PRIORITY WICED_NETWORK_WORKER_PRIORITY - 1

/* ~4K of stack space is for printf and stack check.
 */
#define BT_TRANSPORT_STACK_SIZE      4096


#define BT_TRANSPORT_QUEUE_SIZE      10

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef wiced_result_t (*bt_transport_thread_received_packet_handler_t)( bt_packet_t* packet );
typedef wiced_result_t (*bt_transport_thread_callback_handler_t)( void* arg );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_transport_thread_init( bt_transport_thread_received_packet_handler_t handler );

wiced_result_t bt_transport_thread_deinit( void );

wiced_result_t bt_transport_thread_send_packet( bt_packet_t* packet );

wiced_result_t bt_transport_thread_notify_packet_received( void );

wiced_result_t bt_transport_thread_execute_callback( bt_transport_thread_callback_handler_t callback_handler, void* arg );

wiced_result_t bt_transport_thread_enable_packet_dump( void );

wiced_result_t bt_transport_thread_disable_packet_dump( void );

#ifdef __cplusplus
} /* extern "C" */
#endif
