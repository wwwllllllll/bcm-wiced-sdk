/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */

#include "wiced.h"
#include "wiced_bt_smartbridge.h"
#include "wiced_bt_smartbridge_gatt.h"
#include "bt_smart_gatt.h"

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_bt_smartbridge_gatt_discover_all_primary_services( const wiced_bt_smartbridge_socket_t* socket, wiced_bt_smart_attribute_list_t* service_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || service_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_discover_all_primary_services( socket->connection_handle, service_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_primary_services_by_uuid( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* service_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || service_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_discover_primary_services_by_uuid( socket->connection_handle, uuid, service_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_find_included_services( const wiced_bt_smartbridge_socket_t* socket, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* include_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || include_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_find_included_services( socket->connection_handle, start_handle, end_handle, include_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_all_characteristics_in_a_service( const wiced_bt_smartbridge_socket_t* socket, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || characteristic_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_discover_all_characteristics_in_a_service( socket->connection_handle, start_handle, end_handle, characteristic_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_characteristic_by_uuid( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_uuid_t* uuid, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* characteristic_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_discover_characteristics_by_uuid( socket->connection_handle, uuid, start_handle, end_handle, characteristic_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_discover_handle_and_type_of_all_characteristic_descriptors( const wiced_bt_smartbridge_socket_t* socket, uint16_t start_handle, uint16_t end_handle, wiced_bt_smart_attribute_list_t* descriptor_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || descriptor_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_discover_all_characteristic_descriptors( socket->connection_handle, start_handle, end_handle, descriptor_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_read_characteristic_descriptor( socket->connection_handle, handle, uuid, descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_long_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_read_long_characteristic_descriptor( socket->connection_handle, handle, uuid, descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_write_characteristic_descriptor( socket->connection_handle, (wiced_bt_smart_attribute_t*)descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_long_characteristic_descriptor( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* descriptor )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || descriptor == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_write_long_characteristic_descriptor( socket->connection_handle, (wiced_bt_smart_attribute_t*)descriptor );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_read_characteristic_value( socket->connection_handle, handle, uuid, characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_characteristic_values_using_uuid( const wiced_bt_smartbridge_socket_t* socket,  const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_list_t* characteristic_value_list )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_value_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_read_characteristic_values_using_uuid( socket->connection_handle, uuid, characteristic_value_list );
}

wiced_result_t wiced_bt_smartbridge_gatt_read_long_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, uint16_t handle, const wiced_bt_uuid_t* uuid, wiced_bt_smart_attribute_t** characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || uuid == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_read_long_characteristic_value( socket->connection_handle, handle, uuid, characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_characteristic_value_without_response( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    return WICED_BT_UNSUPPORTED;
}

wiced_result_t wiced_bt_smartbridge_gatt_signed_write_characteristic_value_without_response( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    return WICED_BT_UNSUPPORTED;
}

wiced_result_t wiced_bt_smartbridge_gatt_write_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_write_characteristic_value( socket->connection_handle, (wiced_bt_smart_attribute_t*)characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_write_long_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    wiced_bt_smartbridge_socket_status_t status;

    if ( socket == NULL || characteristic_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    wiced_bt_smartbridge_get_socket_status( (wiced_bt_smartbridge_socket_t*)socket, &status );
    if ( status != SMARTBRIDGE_SOCKET_CONNECTED )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    return bt_smart_gatt_write_long_characteristic_value( socket->connection_handle, (wiced_bt_smart_attribute_t*)characteristic_value );
}

wiced_result_t wiced_bt_smartbridge_gatt_reliable_write_characteristic_value( const wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* characteristic_value )
{
    return WICED_BT_UNSUPPORTED;
}
