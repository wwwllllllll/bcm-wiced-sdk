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
#include "wiced_utilities.h"
#include "bt_packet_internal.h"
#include "bt_hci_interface.h"

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

typedef wiced_result_t (*bt_hci_incoming_packet_handler_t)( bt_packet_t* packet );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_hci_init( void );

wiced_result_t bt_hci_deinit( void );

wiced_result_t bt_hci_register_event_handler( bt_hci_incoming_packet_handler_t event_handler );

wiced_result_t bt_hci_register_acl_data_handler( bt_hci_incoming_packet_handler_t acl_data_handler );

wiced_result_t bt_hci_register_sco_data_handler( bt_hci_incoming_packet_handler_t sco_data_handler );

wiced_result_t bt_hci_execute_application_callback( event_handler_t application_callback, void* arg );

wiced_result_t bt_hci_create_packet( hci_packet_type_t packet_type, bt_packet_t** packet, uint32_t data_size );

wiced_result_t bt_hci_delete_packet( bt_packet_t* packet );

wiced_result_t bt_hci_send_packet( bt_packet_t* packet );

#ifdef __cplusplus
} /* extern "C" */
#endif
