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
#include "wiced_bt_constants.h"
#include "wiced_bt_smart_attribute.h"

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

typedef wiced_result_t (*bt_smart_gatt_notification_indication_handler_t )( uint16_t connection_handle, uint16_t attribute_handle, uint8_t* data, uint16_t length );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/* Initalisation / Deinitalisation Functions
 */
wiced_result_t bt_smart_gatt_init( void );

wiced_result_t bt_smart_gatt_deinit( void );

wiced_result_t bt_smart_gatt_set_timeout( uint32_t timeout_ms );

/* Callback Registration Function
 */
wiced_result_t bt_smart_gatt_register_notification_indication_handler( bt_smart_gatt_notification_indication_handler_t handler );


/* Server Configuration Function
 */
wiced_result_t bt_smart_gatt_exchange_mtu( uint16_t connection_handle, uint16_t client_mtu, uint16_t* server_mtu );


/* Primary Service Discovery Functions
 */
wiced_result_t bt_smart_gatt_discover_all_primary_services( uint16_t connection_handle, wiced_bt_smart_attribute_list_t* service_list );

wiced_result_t bt_smart_gatt_discover_primary_services_by_uuid( uint16_t connection_handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* service_list );


/* Relationship Discovery Functions
 */
wiced_result_t bt_smart_gatt_find_included_services( uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* include_list );


/* Characteristic Discovery Functions
 */
wiced_result_t bt_smart_gatt_discover_all_characteristics_in_a_service( uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list );

wiced_result_t bt_smart_gatt_discover_characteristics_by_uuid( uint16_t connection_handle, const wiced_bt_uuid_t* uuid, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list );


/* Characteristic Descriptor Discovery Functions
 */
wiced_result_t bt_smart_gatt_discover_all_characteristic_descriptors( uint16_t connection_handle, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* no_value_descriptor_list );


/* Characteristic Descriptor Read Functions
 */
wiced_result_t bt_smart_gatt_read_characteristic_descriptor( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor );

wiced_result_t bt_smart_gatt_read_long_characteristic_descriptor( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor );


/* Characteristic Descriptor Write Functions
 */
wiced_result_t bt_smart_gatt_write_characteristic_descriptor( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

wiced_result_t bt_smart_gatt_write_long_characteristic_descriptor( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );


/* Characteristic Value Read Functions
 */
wiced_result_t bt_smart_gatt_read_characteristic_value( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* type, wiced_bt_smart_attribute_t** characteristic_value );

wiced_result_t bt_smart_gatt_read_characteristic_values_using_uuid( uint16_t connection_handle,  const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* characteristic_value_list );

wiced_result_t bt_smart_gatt_read_long_characteristic_value( uint16_t connection_handle, uint16_t handle, const wiced_bt_uuid_t* type, wiced_bt_smart_attribute_t** characteristic_value );


/* Characteristic Value Write Functions
 */
//wiced_result_t bt_smart_gatt_write_characteristic_value_without_response( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

//wiced_result_t bt_smart_gatt_signed_write_characteristic_value_without_response( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

wiced_result_t bt_smart_gatt_write_characteristic_value( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

wiced_result_t bt_smart_gatt_write_long_characteristic_value( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

//wiced_result_t bt_smart_gatt_reliable_write_characteristic_value( uint16_t connection_handle, wiced_bt_smart_attribute_t* attribute );

#ifdef __cplusplus
} /* extern "C" */
#endif
