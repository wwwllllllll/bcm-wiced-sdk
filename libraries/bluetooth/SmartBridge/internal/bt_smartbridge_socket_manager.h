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

#include "wiced_utilities.h"
#include "wiced_bt_smartbridge.h"
#include "bt_linked_list.h"

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

wiced_result_t bt_smartbridge_socket_manager_init( void );

wiced_result_t bt_smartbridge_socket_manager_deinit( void );

wiced_result_t bt_smartbridge_socket_manager_set_max_concurrent_connections( uint8_t count );

wiced_bool_t   bt_smartbridge_socket_manager_is_full( void );

wiced_result_t bt_smartbridge_socket_manager_insert_socket( wiced_bt_smartbridge_socket_t* socket );

wiced_result_t bt_smartbridge_socket_manager_remove_socket( uint16_t connection_handle, wiced_bt_smartbridge_socket_t** socket );

wiced_result_t bt_smartbridge_socket_manager_find_socket_by_handle( uint16_t connection_handle, wiced_bt_smartbridge_socket_t** socket );

wiced_result_t bt_smartbridge_socket_manager_find_socket_by_address( const wiced_bt_device_address_t* address, wiced_bt_smartbridge_socket_t** socket );

#ifdef __cplusplus
} /* extern "C" */
#endif
