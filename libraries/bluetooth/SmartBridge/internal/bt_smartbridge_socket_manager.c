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
#include "bt_linked_list.h"
#include "bt_smartbridge_socket_manager.h"

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

static wiced_bool_t smartbridge_socket_manager_find_socket_by_handle_callback  ( bt_list_node_t* node_to_compare, void* user_data );
static wiced_bool_t smartbridge_socket_manager_find_socket_by_address_callback ( bt_list_node_t* node_to_compare, void* user_data );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* Socket Management Globals */
static bt_linked_list_t connected_socket_list;
static wiced_mutex_t    connected_socket_list_mutex;
static uint8_t          max_number_of_connections = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_smartbridge_socket_manager_init( void )
{
    wiced_result_t result;

    result = bt_linked_list_init( &connected_socket_list );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error creating linked list\n" ) );
        return result;
    }

    result = wiced_rtos_init_mutex( &connected_socket_list_mutex );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error creating mutex\n" ) );
        return result;
    }

    max_number_of_connections = 1;

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_smartbridge_socket_manager_deinit( void )
{
    wiced_rtos_deinit_mutex( &connected_socket_list_mutex );
    bt_linked_list_deinit( &connected_socket_list );
    max_number_of_connections = 0;
    return WICED_BT_SUCCESS;
}

wiced_result_t bt_smartbridge_socket_manager_set_max_concurrent_connections( uint8_t count )
{
    max_number_of_connections = count;
    return WICED_BT_SUCCESS;
}

wiced_bool_t   bt_smartbridge_socket_manager_is_full( void )
{
    uint32_t active_connection_count;

    bt_linked_list_get_count( &connected_socket_list, &active_connection_count );

    return ( active_connection_count == max_number_of_connections ) ? WICED_TRUE : WICED_FALSE;
}

wiced_result_t bt_smartbridge_socket_manager_insert_socket( wiced_bt_smartbridge_socket_t* socket )
{
    wiced_result_t result;
    uint32_t       count;

    bt_linked_list_get_count( &connected_socket_list, &count );

    if ( count == max_number_of_connections )
    {
        return WICED_BT_MAX_CONNECTIONS_REACHED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = bt_linked_list_insert_at_rear( &connected_socket_list, &socket->node );

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

wiced_result_t bt_smartbridge_socket_manager_remove_socket( uint16_t connection_handle, wiced_bt_smartbridge_socket_t** socket )
{
    wiced_result_t  result;
    uint32_t        count;
    bt_list_node_t* node_found;
    uint32_t        user_data = connection_handle;

    bt_linked_list_get_count( &connected_socket_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = bt_linked_list_find( &connected_socket_list, smartbridge_socket_manager_find_socket_by_handle_callback, (void*)user_data, &node_found );
    if ( result == WICED_BT_SUCCESS )
    {
        result = bt_linked_list_remove( &connected_socket_list, node_found );

        if ( result == WICED_BT_SUCCESS )
        {
            *socket = (wiced_bt_smartbridge_socket_t*)node_found->data;
        }
    }

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

wiced_result_t bt_smartbridge_socket_manager_find_socket_by_handle( uint16_t connection_handle, wiced_bt_smartbridge_socket_t** socket )
{
    wiced_result_t  result;
    uint32_t        count;
    bt_list_node_t* node_found;
    uint32_t        user_data = connection_handle;

    bt_linked_list_get_count( &connected_socket_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = bt_linked_list_find( &connected_socket_list, smartbridge_socket_manager_find_socket_by_handle_callback, (void*)user_data, &node_found );

    if ( result == WICED_BT_SUCCESS )
    {
        *socket = (wiced_bt_smartbridge_socket_t*)node_found->data;
    }

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

wiced_result_t bt_smartbridge_socket_manager_find_socket_by_address( const wiced_bt_device_address_t* address, wiced_bt_smartbridge_socket_t** socket )
{
    wiced_result_t  result;
    uint32_t        count;
    bt_list_node_t* node_found;

    bt_linked_list_get_count( &connected_socket_list, &count );

    if ( count == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    /* Lock protection */
    wiced_rtos_lock_mutex( &connected_socket_list_mutex );

    result = bt_linked_list_find( &connected_socket_list, smartbridge_socket_manager_find_socket_by_address_callback, (void*)address, &node_found );

    if ( result == WICED_SUCCESS )
    {
        *socket = (wiced_bt_smartbridge_socket_t*)node_found->data;
    }

    /* Unlock protection */
    wiced_rtos_unlock_mutex( &connected_socket_list_mutex );

    return result;
}

static wiced_bool_t smartbridge_socket_manager_find_socket_by_handle_callback( bt_list_node_t* node_to_compare, void* user_data )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)node_to_compare->data;
    uint32_t connection_handle = (uint32_t)user_data;

    return ( socket->connection_handle == connection_handle ) ? WICED_TRUE : WICED_FALSE;
}

static wiced_bool_t smartbridge_socket_manager_find_socket_by_address_callback( bt_list_node_t* node_to_compare, void* user_data )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)node_to_compare->data;
    wiced_bt_device_address_t*    address = (wiced_bt_device_address_t*)user_data;

    return ( memcmp( socket->remote_device.address.address, address->address, sizeof( address->address ) ) == 0 ) ? WICED_TRUE : WICED_FALSE;
}
