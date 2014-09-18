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
#include "wiced_bt_smart_interface.h"

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

/* GAP connection event
 */
typedef enum
{
    GAP_CONNECTED,          /* Connection Successful Event                   */
    GAP_DISCONNECTED,       /* Disconnection Successful Event                */
    GAP_CONNECTION_TIMEOUT, /* Connection Timeout Event                      */
    GAP_ENCRYPTION_ENABLED, /* Encryption Enabled (Pairing Successful) Event */
    GAP_ENCRYPTION_FAILED,  /* Encryption Failed (Pairing Failed) Event      */
    GAP_ENCRYPTION_TIMEOUT, /* Encryption Timeout (Pairing Timeout) Event    */
} bt_smart_gap_connection_event_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/* GAP connection event callback
 */
typedef wiced_result_t (*bt_smart_gap_connection_handler_t)( bt_smart_gap_connection_event_t event, wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, uint16_t connection_handle );

/* GAP bonding successful callback
 */
typedef wiced_result_t (*bt_smart_gap_bonding_handler_t)   ( uint16_t connection_handle, const wiced_bt_smart_bond_info_t* bond_info );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/* Initialise Generic Access Profile Module
 */
wiced_result_t bt_smart_gap_init( void );

/* Deinitialise Generic Access Profile Module
 */
wiced_result_t bt_smart_gap_deinit( void );

/* Set maximum number of simultaneous connections supported by the stack
 */
wiced_result_t bt_smart_gap_set_max_simultaneous_connections( uint8_t count );

/* Check if currently scanning for BLE devices
 */
wiced_bool_t   bt_smart_gap_is_scanning( void );

/* Start scanning for BLE devices
 */
wiced_result_t bt_smart_gap_start_scan( const wiced_bt_smart_scan_settings_t* settings, wiced_bt_smart_scan_complete_callback_t complete_callback, wiced_bt_smart_advertising_report_callback_t advertising_report_callback );

/* Stop scanning for BLE devices
 */
wiced_result_t bt_smart_gap_stop_scan( void );

/* Get scan result list
 */
wiced_result_t bt_smart_gap_get_scan_results( wiced_bt_smart_scan_result_t** results, uint32_t* count );

/* Add a Bluetooth Smart device to the whitelist
 */
wiced_result_t bt_smart_gap_add_device_to_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type );

/* Remove a Bluetooth Smart device from the whitelist
 */
wiced_result_t bt_smart_gap_remove_device_from_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type );

/* Get whitelist size
 */
wiced_result_t bt_smart_gap_get_whitelist_size( uint32_t* size );

/* Clear the scan whitelist
 */
wiced_result_t bt_smart_gap_clear_whitelist( void );

/* Initiate a connection to a BLE device
 */
wiced_result_t bt_smart_gap_connect( const wiced_bt_smart_connection_settings_t* settings, const wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, bt_smart_gap_connection_handler_t handler );

/* Cancel the most recent connection attempt
 */
wiced_result_t bt_smart_gap_cancel_last_connect( void );

/* Set transmit power during connection
 */
wiced_result_t bt_smart_gap_set_connection_tx_power( uint16_t connection_handle, int8_t tx_power_dbm );

/* Disconnect an existing connection with the given handle
 */
wiced_result_t bt_smart_gap_disconnect( uint16_t connection_handle );

/* Register bonding result callback
 */
wiced_result_t bt_smart_gap_register_bonding_callback( bt_smart_gap_bonding_handler_t bonding_callback );

/* Enable pairing request on the next connection attempt
 */
wiced_result_t bt_smart_gap_enable_pairing( const wiced_bt_smart_security_settings_t* settings, const char* passkey );

/* Disable pairing request on the next connection attempt
 */
wiced_result_t bt_smart_gap_disable_pairing( void );

/* Load bond info for the next connection attempt
 */
wiced_result_t bt_smart_gap_set_bond_info( const wiced_bt_smart_security_settings_t* settings, const wiced_bt_smart_bond_info_t* bond_info );

/* Clear bond info for the next connection attempt
 */
wiced_result_t bt_smart_gap_clear_bond_info( void );

/* Check if bond info for the next connection attempt is set
 */
wiced_bool_t   bt_smart_gap_is_bond_info_set( void );

#ifdef __cplusplus
} /* extern "C" */
#endif
