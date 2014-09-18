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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_management_mpaf_init( void );

wiced_result_t bt_management_mpaf_deinit( void );

wiced_bool_t   bt_management_mpaf_is_device_connectable( void );

wiced_bool_t   bt_management_mpaf_is_device_discoverable( void );

wiced_result_t bt_management_mpaf_wait_for_event( bt_packet_t** packet );

wiced_result_t bt_management_mpaf_notify_event_received( bt_packet_t* packet );

#ifdef __cplusplus
} /* extern "C" */
#endif
