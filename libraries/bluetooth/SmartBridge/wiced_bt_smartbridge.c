/**
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
#include "bt_stack.h"
#include "wiced.h"
#include "wiced_bt_smartbridge.h"
#include "bt_smart_att.h"
#include "bt_smart_gap.h"
#include "bt_smart_gatt.h"
#include "bt_transport_thread.h"
#include "bt_smartbridge_socket_manager.h"
#include "bt_smartbridge_att_cache_manager.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define SOCKET_STATE_DISCONNECTED              ( 0 )
#define SOCKET_STATE_LINK_CONNECTED            ( 1 )
#define SOCKET_STATE_LINK_ENCRYPTED            ( 2 )

#define SOCKET_ACTION_HOST_CONNECT             ( 1 << 0 )
#define SOCKET_ACTION_HOST_DISCONNECT          ( 1 << 2 )
#define SOCKET_ACTION_INITIATE_PAIRING         ( 1 << 3 )
#define SOCKET_ACTION_ENCRYPT_USING_BOND_INFO  ( 1 << 4 )

#define SOCKET_INVALID_CONNECTION_HANDLE       ( 0xFFFF )

#define MAX_CONNECTION_TIMEOUT                 ( 10000 )

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

static wiced_result_t smartbridge_app_notification_handler             ( void* arg );
static wiced_result_t smartbridge_app_disconnection_handler            ( void* arg );
static wiced_result_t smartbridge_app_pairing_handler                  ( void* arg );
static wiced_result_t smartbridge_gatt_indication_notification_handler ( uint16_t connection_handle, uint16_t attribute_handle, uint8_t* data, uint16_t length );
static wiced_result_t smartbridge_gap_connection_handler               ( bt_smart_gap_connection_event_t event, wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, uint16_t connection_handle );
static wiced_result_t smartbridge_gap_bonding_handler                  ( uint16_t connection_handle, const wiced_bt_smart_bond_info_t* bond_info );
wiced_bool_t          smartbridge_socket_check_actions_enabled         ( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits );
wiced_bool_t          smartbridge_socket_check_actions_disabled        ( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits );
void                  smartbridge_socket_set_actions                   ( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits );
void                  smartbridge_socket_clear_actions                 ( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_bt_smartbridge_socket_t* connecting_socket = NULL;
static wiced_bool_t                   initialised       = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_bt_smartbridge_init( void )
{
    wiced_result_t result;

    if ( initialised == WICED_TRUE )
    {
        return WICED_BT_SUCCESS;
    }

    WPRINT_LIB_INFO( ( "Initialising WICED SmartBridge ...\n" ) );

    /* Check if BT Smart stack is already initialised. BT Smart stack is initialised in wiced_bt_init() */
    if ( bt_stack_is_initialised() == WICED_FALSE )
    {
        WPRINT_LIB_INFO( ( "Bluetooth Smart stack is not initialised\n" ) );
        return WICED_BT_STACK_UNINITIALISED;
    }

    /* Initialise Generic Attribute Profile (GATT) module */
    result = bt_smart_gatt_init();
    if ( result  != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error initialising GATT\n" ) );
        return result;
    }

    /* Register notification/indication callback with GATT */
    bt_smart_gatt_register_notification_indication_handler( smartbridge_gatt_indication_notification_handler );

    /* Initialise Generic Access Profile (GAP) module */
    result = bt_smart_gap_init();
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error initialising GAP\n" ) );
        return result;
    }

    /* Initialise SmartBridge Socket Manager */
    result = bt_smartbridge_socket_manager_init();
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_INFO( ( "Error initialising SmartBridge Socket Manager\n" ) );
        return result;
    }

    initialised = WICED_TRUE;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_deinit( void )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    /* Disable Attribute Cache (The function checks if it's enabled) */
    bt_smartbridge_att_cache_disable();
    /* Deinitialise socket manager */
    bt_smartbridge_socket_manager_deinit();

    /* Deinitialise GATT */
    bt_smart_gatt_deinit();

    bt_smart_gatt_register_notification_indication_handler( NULL );

    /* Deinitialise GAP */
    bt_smart_gap_deinit();

    initialised = WICED_FALSE;

    return WICED_BT_SUCCESS;
}

wiced_bool_t   wiced_bt_smartbridge_is_scanning( void )
{
    return ( initialised == WICED_TRUE ) ? bt_smart_gap_is_scanning( ) : WICED_FALSE;
}

wiced_result_t wiced_bt_smartbridge_set_max_concurrent_connections( uint8_t count )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Set socket manager's connections limit */
    bt_smartbridge_socket_manager_set_max_concurrent_connections( count );

    /* Set GAP connection limit and prepares connections workspace */
    return bt_smart_gap_set_max_simultaneous_connections( count );
}

wiced_bool_t   wiced_bt_smartbridge_is_ready_to_connect( void )
{
    return ( initialised == WICED_FALSE || connecting_socket != NULL ) ? WICED_FALSE : WICED_TRUE;
}

wiced_result_t wiced_bt_smartbridge_start_scan( const wiced_bt_smart_scan_settings_t* settings, wiced_bt_smart_scan_complete_callback_t complete_callback, wiced_bt_smart_advertising_report_callback_t advertising_report_callback )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_start_scan( settings, complete_callback, advertising_report_callback );
}

wiced_result_t wiced_bt_smartbridge_stop_scan( void )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_stop_scan( );
}

wiced_result_t wiced_bt_smartbridge_get_scan_result_list( wiced_bt_smart_scan_result_t** result_list, uint32_t* count )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_get_scan_results( result_list, count );
}

wiced_result_t wiced_bt_smartbridge_add_device_to_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_add_device_to_whitelist( device_address, address_type );
}

wiced_result_t wiced_bt_smartbridge_remove_device_from_whitelist( const wiced_bt_device_address_t* device_address, wiced_bt_smart_address_type_t address_type )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_remove_device_from_whitelist( device_address, address_type );
}

wiced_result_t wiced_bt_smartbridge_get_whitelist_size( uint32_t* size )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_get_whitelist_size( size );
}

wiced_result_t wiced_bt_smartbridge_clear_whitelist( void )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_clear_whitelist( );
}

wiced_result_t wiced_bt_smartbridge_create_socket( wiced_bt_smartbridge_socket_t* socket )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Reset socket fields */
    memset( socket, 0, sizeof( *socket ) );
    socket->connection_handle = SOCKET_INVALID_CONNECTION_HANDLE;

    /* Point node data to socket */
    socket->node.data = (void*)socket;

    /* Initialise socket semaphore */
    return wiced_rtos_init_semaphore( &socket->semaphore );
}

wiced_result_t wiced_bt_smartbridge_delete_socket( wiced_bt_smartbridge_socket_t* socket )
{
    wiced_result_t result;
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    result = wiced_rtos_deinit_semaphore( &socket->semaphore );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    memset( socket, 0, sizeof( *socket ) );
    socket->connection_handle = SOCKET_INVALID_CONNECTION_HANDLE;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_get_socket_status( wiced_bt_smartbridge_socket_t* socket, wiced_bt_smartbridge_socket_status_t* status )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( socket->state == SOCKET_STATE_LINK_ENCRYPTED )
    {
        *status = SMARTBRIDGE_SOCKET_CONNECTED;
    }
    else if ( socket->state == SOCKET_STATE_LINK_CONNECTED )
    {
        /* Status is connected if socket does not have loaded bond info and does not initiate pairing */
        if ( smartbridge_socket_check_actions_disabled( socket, SOCKET_ACTION_ENCRYPT_USING_BOND_INFO | SOCKET_ACTION_INITIATE_PAIRING ) == WICED_TRUE )
        {
            *status = SMARTBRIDGE_SOCKET_CONNECTED;
        }
        else
        {
            *status = SMARTBRIDGE_SOCKET_CONNECTING;
        }
    }
    else
    {
        *status = SMARTBRIDGE_SOCKET_DISCONNECTED;
    }

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_connect( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_device_t* remote_device, const wiced_bt_smart_connection_settings_t* settings, wiced_bt_smartbridge_disconnection_callback_t disconnection_callback, wiced_bt_smartbridge_notification_callback_t notification_callback )
{
    wiced_bt_smartbridge_socket_t* found_socket;
    wiced_result_t result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( connecting_socket != NULL )
    {
        /* Only 1 connecting socket is allowed */
        return WICED_BT_CONNECT_IN_PROGRESS;
    }

    if ( bt_smartbridge_socket_manager_is_full() == WICED_TRUE )
    {
        return WICED_BT_MAX_CONNECTIONS_REACHED;
    }

    if ( bt_smartbridge_socket_manager_find_socket_by_address( &remote_device->address, &found_socket ) == WICED_BT_SUCCESS )
    {
        /* device is already connected */
        return WICED_BT_SOCKET_IN_USE;
    }

    /* Clean-up accidentally set semaphores */
    while( wiced_rtos_get_semaphore( &socket->semaphore, WICED_NO_WAIT ) == WICED_BT_SUCCESS )
    {
    }

    /* Store socket pointer in a temporary global variable so it can be referenced in smartbridge_gap_connection_handler */
    connecting_socket = socket;

    /* Store connection settings */
    memcpy( &socket->connection_settings, settings, sizeof( *settings ) );

    /* Store remote device information */
    memcpy( &socket->remote_device, remote_device, sizeof( *remote_device ) );

    /* Set callback functions */
    socket->disconnection_callback = disconnection_callback;
    socket->notification_callback  = notification_callback;

    /* Reset connection handle to invalid value */
    socket->connection_handle = SOCKET_INVALID_CONNECTION_HANDLE;

    /* Reset state */
    socket->state = SOCKET_STATE_DISCONNECTED;

    if ( smartbridge_socket_check_actions_enabled( socket, SOCKET_ACTION_INITIATE_PAIRING ) == WICED_TRUE )
    {
        /* Register callback to let GAP notify SmartBridge if bond is successfully created */
        bt_smart_gap_register_bonding_callback( smartbridge_gap_bonding_handler );

        /* Tell GAP to initiate pairing on the next connection attempt */
        bt_smart_gap_enable_pairing( &socket->security_settings, socket->passkey );
    }
    else
    {
        /* Reset GAP bonding callback */
        bt_smart_gap_register_bonding_callback( NULL );

        /* Tell GAP to not send pairing request on the next connection attempt */
        bt_smart_gap_disable_pairing();
    }

    if ( smartbridge_socket_check_actions_enabled( socket, SOCKET_ACTION_ENCRYPT_USING_BOND_INFO ) == WICED_TRUE )
    {
        /* Tell GAP to enable encryption using bond info and security settings provided */
        bt_smart_gap_set_bond_info( &socket->security_settings, &socket->bond_info );
    }
    else
    {
        /* Tell GAP to clear bond info and disable encryption */
        bt_smart_gap_clear_bond_info();
    }

    /* Set socket action to connecting */
    smartbridge_socket_set_actions( socket, SOCKET_ACTION_HOST_CONNECT );

    /* Set attribute protocol timeout */
    bt_smart_gatt_set_timeout( settings->attribute_protocol_timeout_ms );

    /* Tell GAP to send connection request */
    bt_smart_gap_connect( settings, &remote_device->address, remote_device->address_type, smartbridge_gap_connection_handler );

    /* Wait for connection */
    wiced_rtos_get_semaphore( &socket->semaphore, WICED_NEVER_TIMEOUT );

    /* Check if link is connected. Otherwise, return error */
    if ( socket->state == SOCKET_STATE_LINK_CONNECTED )
    {
        /* Check if encryption is required */
        if ( smartbridge_socket_check_actions_enabled( socket, SOCKET_ACTION_INITIATE_PAIRING ) == WICED_TRUE ||
             smartbridge_socket_check_actions_enabled( socket, SOCKET_ACTION_ENCRYPT_USING_BOND_INFO ) == WICED_TRUE )
        {
            /* Wait until link is encrypted */
            wiced_rtos_get_semaphore( &socket->semaphore, WICED_NEVER_TIMEOUT );

            if ( socket->state != SOCKET_STATE_LINK_ENCRYPTED )
            {
                result = WICED_BT_ENCRYPTION_FAILED;
                goto error;
            }
        }
    }
    else
    {
        result = WICED_BT_SOCKET_NOT_CONNECTED;
        goto error;
    }

    /* Successful */
    if ( bt_smartbridge_att_cache_is_enabled() == WICED_TRUE )
    {
        bt_smartbridge_att_cache_t* cache = NULL;

        result = bt_smartbridge_att_cache_find( remote_device, &cache );
        if ( result == WICED_BT_SUCCESS )
        {
            WPRINT_LIB_DEBUG(( "USING ATT CACHE ...\n" ));
        }
        else
        {
            WPRINT_LIB_DEBUG(( "GENERATING ATT CACHE ...\n" ));

            result = bt_smartbridge_att_cache_generate( remote_device, socket->connection_handle, &cache );
            if ( result != WICED_BT_SUCCESS )
            {
                goto error;
            }
        }

        /* Successful. Mark cache as active and store reference in socket */
        bt_smartbridge_att_cache_set_active_state( cache, WICED_TRUE );
        socket->att_cache = (void*)cache;
    }

    /* Clear connect action as it's no longer needed */
    smartbridge_socket_clear_actions( socket, SOCKET_ACTION_HOST_CONNECT );

    /* Reset connecting socket pointer */
    connecting_socket = NULL;

    /* Link is connected. Return success */
    return WICED_BT_SUCCESS;


    error:
    /* Link is not connected nor encrypted. Issue disconnection attempt to clean-up */
    wiced_bt_smartbridge_disconnect( socket );

    /* Clear connect action as it's no longer needed */
    smartbridge_socket_clear_actions( socket, SOCKET_ACTION_HOST_CONNECT );

    /* Reset connecting socket pointer */
    connecting_socket = NULL;

    /* Link is not connected. Return error */
    return result;
}

wiced_result_t wiced_bt_smartbridge_disconnect( wiced_bt_smartbridge_socket_t* socket )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Mark disconnection flag that it's coming from the host */
    smartbridge_socket_set_actions( socket, SOCKET_ACTION_HOST_DISCONNECT );

    /* Clean-up accidentally set semaphores */
    while( wiced_rtos_get_semaphore( &socket->semaphore, WICED_NO_WAIT ) == WICED_BT_SUCCESS )
    {
    }

    /* Check if either link is encrypted or connected */
    if ( socket->state >= SOCKET_STATE_LINK_CONNECTED )
    {
        /* Tell GAP to send disconnection request */
        bt_smart_gap_disconnect( socket->connection_handle );

        /* Wait for disconnection */
        wiced_rtos_get_semaphore( &socket->semaphore, socket->connection_settings.timeout_second );
    }
    else
    {
        /* Link is not yet connected. Cancel last */
        bt_smart_gap_cancel_last_connect();
    }

    /* Clear socket disconnect action */
    smartbridge_socket_set_actions( socket, SOCKET_ACTION_HOST_DISCONNECT );

    /* Proper clean-up if socket isn't properly disconnected */
    if ( socket->state != SOCKET_STATE_DISCONNECTED )
    {
        wiced_bt_smartbridge_socket_t* removed_socket;

        bt_smartbridge_socket_manager_remove_socket( socket->connection_handle, &removed_socket );

        /* Reset connection handle to invalid value */
        socket->connection_handle = SOCKET_INVALID_CONNECTION_HANDLE;

        /* Clear socket state */
        socket->state = SOCKET_STATE_DISCONNECTED;

        /* Mark att cache as inactive and reset reference to cache */
        bt_smartbridge_att_cache_set_active_state( (bt_smartbridge_att_cache_t*)socket->att_cache, WICED_FALSE );
        socket->att_cache = NULL;
    }


    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_set_transmit_power( wiced_bt_smartbridge_socket_t* socket, int8_t transmit_power_dbm )
{
    if ( initialised == WICED_FALSE || socket->state == SOCKET_STATE_DISCONNECTED )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    return bt_smart_gap_set_connection_tx_power( socket->connection_handle, transmit_power_dbm );
}

wiced_result_t wiced_bt_smartbridge_set_bond_info( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_security_settings_t* settings, const wiced_bt_smart_bond_info_t* bond_info )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Set local copies of security settings and bond info */
    memcpy( &socket->bond_info, bond_info, sizeof( *bond_info ) );
    memcpy( &socket->security_settings, settings, sizeof( *settings ) );

    /* Clear socket action to initiate pairing request */
    smartbridge_socket_clear_actions( socket, SOCKET_ACTION_INITIATE_PAIRING );

    /* Set socket action to encrypt using loaded bond info */
    smartbridge_socket_set_actions( socket, SOCKET_ACTION_ENCRYPT_USING_BOND_INFO );

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_clear_bond_info( wiced_bt_smartbridge_socket_t* socket )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Reset bond info */
    memset( &socket->bond_info, 0, sizeof( socket->bond_info ) );

    /* Clear socket action to encrypt using loaded bond info */
    smartbridge_socket_clear_actions( socket, SOCKET_ACTION_ENCRYPT_USING_BOND_INFO );

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_enable_pairing( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_security_settings_t* settings, const char* numeric_passkey, wiced_bt_smartbridge_bonding_callback_t bonding_callback )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Store security settings in local copy */
    memcpy( &socket->security_settings, settings, sizeof( *settings ) );

    /* Reset bond info */
    memset( &socket->bond_info, 0, sizeof( socket->bond_info ) );

    /* Store passkey */
    memset( socket->passkey, 0, sizeof( socket->passkey ) );
    if ( numeric_passkey != NULL )
    {
        memcpy( socket->passkey, numeric_passkey, strnlen( numeric_passkey, sizeof( socket->passkey ) - 1 ) );
    }

    /* Set pairing callback */
    socket->bonding_callback = bonding_callback;

    /* Clear socket action to encrypt using loaded bond info */
    smartbridge_socket_clear_actions( socket, SOCKET_ACTION_ENCRYPT_USING_BOND_INFO );

    /* Set socket action to iniatiate pairing request */
    smartbridge_socket_set_actions( socket, SOCKET_ACTION_INITIATE_PAIRING );

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_disable_pairing( wiced_bt_smartbridge_socket_t* socket )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Clear socket action to iniatiate pairing request */
    smartbridge_socket_clear_actions( socket, SOCKET_ACTION_INITIATE_PAIRING );

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_smartbridge_enable_attribute_cache( uint32_t cache_count )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Call internal function */
    return bt_smartbridge_att_cache_enable( cache_count );
}

wiced_result_t wiced_bt_smartbridge_disable_attribute_cache( void )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    /* Call internal function */
    return bt_smartbridge_att_cache_disable();
}

wiced_result_t wiced_bt_smartbridge_enable_attribute_cache_notification( wiced_bt_smartbridge_socket_t* socket )
{
    wiced_bt_smart_attribute_list_t* list;
    wiced_bt_smart_attribute_t*      iterator;
    bt_smartbridge_att_cache_t*      cache;
    wiced_result_t                   result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL )
    {
        return WICED_BT_BADARG;
    }

    cache = (bt_smartbridge_att_cache_t*)socket->att_cache;

    if ( bt_smartbridge_att_cache_is_discovering( cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    result = bt_smartbridge_att_cache_get_list( cache, &list );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    result = wiced_bt_smart_attribute_get_list_head( list, &iterator );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    while( iterator != NULL )
    {
        if ( iterator->type.value.value_16_bit == 0x2902 )
        {
            /* Swith on notification from server */
            bt_smartbridge_att_cache_lock( cache );

            iterator->value.client_config.config_bits = 1;

            bt_smartbridge_att_cache_unlock( cache );

            result = bt_smart_gatt_write_characteristic_descriptor( socket->connection_handle, iterator );
        }

        iterator = iterator->next;
    }

    return result;
}

wiced_result_t wiced_bt_smartbridge_disable_attribute_cache_notification( wiced_bt_smartbridge_socket_t* socket )
{
    wiced_bt_smart_attribute_list_t* list;
    wiced_bt_smart_attribute_t*      iterator;
    bt_smartbridge_att_cache_t*      cache;
    wiced_result_t                   result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL )
    {
        return WICED_BT_BADARG;
    }

    cache = (bt_smartbridge_att_cache_t*)socket->att_cache;

    if ( bt_smartbridge_att_cache_is_discovering( cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    result = bt_smartbridge_att_cache_get_list( cache, &list );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    result = wiced_bt_smart_attribute_get_list_head( list, &iterator );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    while( iterator != NULL )
    {
        if ( iterator->type.value.value_16_bit == 0x2902 )
        {
            /* Swith on notification from server */
            bt_smartbridge_att_cache_lock( cache );

            iterator->value.client_config.config_bits = 0;

            bt_smartbridge_att_cache_unlock( cache );

            result = bt_smart_gatt_write_characteristic_descriptor( socket->connection_handle, iterator );
        }

        iterator = iterator->next;
    }

    return result;
}

wiced_result_t wiced_bt_smartbridge_get_attribute_cache_list( wiced_bt_smartbridge_socket_t* socket, wiced_bt_smart_attribute_list_t** att_cache_list )
{
    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL || att_cache_list == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( bt_smartbridge_att_cache_is_discovering( (bt_smartbridge_att_cache_t*)socket->att_cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    /* Call internal function */
    return bt_smartbridge_att_cache_get_list( (bt_smartbridge_att_cache_t*)socket->att_cache, att_cache_list );
}

wiced_result_t wiced_bt_smartbridge_get_attribute_cache_by_handle( wiced_bt_smartbridge_socket_t* socket, uint16_t handle, wiced_bt_smart_attribute_t* attribute, uint16_t size )
{
    bt_smartbridge_att_cache_t*      cache          = NULL;
    wiced_bt_smart_attribute_list_t* att_cache_list = NULL;
    wiced_bt_smart_attribute_t*      att            = NULL;
    wiced_result_t                   result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL || attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    cache = (bt_smartbridge_att_cache_t*)socket->att_cache;

    if ( bt_smartbridge_att_cache_is_discovering( cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    bt_smartbridge_att_cache_get_list( cache, &att_cache_list );

    bt_smartbridge_att_cache_lock( cache );

    result = wiced_bt_smart_attribute_search_list_by_handle( att_cache_list, handle, &att );

    if ( result == WICED_BT_SUCCESS )
    {
        if ( att->value_struct_size + ATTR_COMMON_FIELDS_SIZE > size )
        {
            result = WICED_BT_ATTRIBUTE_VALUE_TOO_LONG;
        }
        else
        {
            memcpy( attribute, att, att->value_struct_size + ATTR_COMMON_FIELDS_SIZE );
        }
    }

    bt_smartbridge_att_cache_unlock( cache );
    return result;
}

wiced_result_t wiced_bt_smartbridge_get_attribute_cache_by_uuid( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_uuid_t* uuid, uint16_t starting_handle, uint16_t ending_handle, wiced_bt_smart_attribute_t* attribute, uint32_t size )
{
    bt_smartbridge_att_cache_t*      cache          = NULL;
    wiced_bt_smart_attribute_list_t* att_cache_list = NULL;
    wiced_bt_smart_attribute_t*      att            = NULL;
    wiced_result_t                   result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL || uuid == NULL || attribute == NULL )
    {
        return WICED_BT_BADARG;
    }

    cache = (bt_smartbridge_att_cache_t*)socket->att_cache;

    if ( bt_smartbridge_att_cache_is_discovering( cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    bt_smartbridge_att_cache_get_list( cache, &att_cache_list );

    bt_smartbridge_att_cache_lock( cache );

    result = wiced_bt_smart_attribute_search_list_by_uuid( att_cache_list, uuid, starting_handle, ending_handle, &att );
    if ( result == WICED_BT_SUCCESS )
    {
        if ( att->value_struct_size + ATTR_COMMON_FIELDS_SIZE > size )
        {
            result = WICED_BT_ATTRIBUTE_VALUE_TOO_LONG;
        }
        else
        {
            memcpy( attribute, att, att->value_struct_size + ATTR_COMMON_FIELDS_SIZE );
        }
    }

    bt_smartbridge_att_cache_unlock( cache );
    return result;
}

wiced_result_t wiced_bt_smartbridge_refresh_attribute_cache_characteristic_value( wiced_bt_smartbridge_socket_t* socket, uint16_t handle )
{
    bt_smartbridge_att_cache_t*      cache          = NULL;
    wiced_bt_smart_attribute_list_t* att_cache_list = NULL;
    wiced_bt_smart_attribute_t*      current_att    = NULL;
    wiced_bt_smart_attribute_t*      refreshed_att  = NULL;
    wiced_result_t                   result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL )
    {
        return WICED_BT_BADARG;
    }

    cache = (bt_smartbridge_att_cache_t*)socket->att_cache;

    if ( bt_smartbridge_att_cache_is_discovering( cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    bt_smartbridge_att_cache_get_list( cache, &att_cache_list );

    bt_smartbridge_att_cache_lock( cache );

    result = wiced_bt_smart_attribute_search_list_by_handle( att_cache_list, handle, &current_att );
    if ( result == WICED_BT_SUCCESS )
    {
        /* Check if length is longer than what read characteristic value can handle
         * If longer, use read long characteristic value
         */
        if ( current_att->value_length <= ATT_STANDARD_VALUE_LENGTH )
        {
            result = bt_smart_gatt_read_characteristic_value( socket->connection_handle, current_att->handle, &current_att->type, &refreshed_att );
        }
        else
        {
            result = bt_smart_gatt_read_long_characteristic_value( socket->connection_handle, current_att->handle, &current_att->type, &refreshed_att );
        }

        /* If read is successful, replace attribute with the refreshed one
         */
        if ( result == WICED_BT_SUCCESS )
        {
            /* This function removes and also deletes the attribute with handle specified */
            result = wiced_bt_smart_attribute_remove_from_list( att_cache_list, current_att->handle );
            if ( result == WICED_BT_SUCCESS )
            {
                result = wiced_bt_smart_attribute_add_to_list( att_cache_list, refreshed_att );
            }
        }
    }

    bt_smartbridge_att_cache_unlock( cache );
    return result;
}

wiced_result_t wiced_bt_smartbridge_write_attribute_cache_characteristic_value( wiced_bt_smartbridge_socket_t* socket, const wiced_bt_smart_attribute_t* char_value )
{
    bt_smartbridge_att_cache_t*      cache          = NULL;
    wiced_bt_smart_attribute_list_t* att_cache_list = NULL;
    wiced_bt_smart_attribute_t*      att            = NULL;
    wiced_result_t                   result;

    if ( initialised == WICED_FALSE )
    {
        return WICED_BT_SMARTBRIDGE_UNINITIALISED;
    }

    if ( bt_smartbridge_att_cache_is_enabled( ) == WICED_FALSE )
    {
        return WICED_BT_ATT_CACHE_UNINITIALISED;
    }

    if ( socket == NULL || socket->att_cache == NULL || char_value == NULL )
    {
        return WICED_BT_BADARG;
    }

    cache = (bt_smartbridge_att_cache_t*)socket->att_cache;

    if ( bt_smartbridge_att_cache_is_discovering( cache ) == WICED_TRUE )
    {
        return WICED_BT_DISCOVER_IN_PROGRESS;
    }

    bt_smartbridge_att_cache_get_list( cache, &att_cache_list );


    if ( char_value->value_length <= ATT_STANDARD_VALUE_LENGTH )
    {
        result = bt_smart_gatt_write_characteristic_value( socket->connection_handle, (wiced_bt_smart_attribute_t*)char_value );
    }
    else
    {
        result = bt_smart_gatt_write_long_characteristic_value( socket->connection_handle, (wiced_bt_smart_attribute_t*)char_value );
    }

    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    bt_smartbridge_att_cache_lock( cache );

    /* Find characteristic value in local attribute list. Add to the list if not found */

    result = wiced_bt_smart_attribute_search_list_by_handle( att_cache_list, char_value->handle, &att );

    if ( result == WICED_BT_SUCCESS )
    {
        /* Found. Compare lengths first.
         * If new length is not equal old length, replace old attribute with new one.
         * If equal, copy content directly.
         */
        if ( char_value->value_length != att->value_length )
        {
            result = wiced_bt_smart_attribute_remove_from_list( att_cache_list, att->handle );

            if ( result != WICED_BT_SUCCESS )
            {
                goto exit;
            }

            result = wiced_bt_smart_attribute_delete( att );
            att = NULL; /* Reuse attribute pointer */

            if ( result != WICED_BT_SUCCESS )
            {
                goto exit;
            }

            result = wiced_bt_smart_attribute_create( &att, WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE, char_value->value_length );

            if ( result != WICED_BT_SUCCESS )
            {
                goto exit;
            }

            att->handle            = char_value->handle;
            att->type              = char_value->type;
            att->value_length      = char_value->value_length;
            att->value_struct_size = char_value->value_struct_size;

            memcpy( att->value.value, char_value->value.value, char_value->value_length );

            result = wiced_bt_smart_attribute_add_to_list( att_cache_list, att );
        }
        else
        {
            memcpy( att->value.value, char_value->value.value, char_value->value_length );
        }
    }
    else if ( result == WICED_BT_ITEM_NOT_IN_LIST )
    {
        /* Not found. Create new one and add attribute to the list */
        result = wiced_bt_smart_attribute_create( &att, WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE, char_value->value_length );

        if ( result != WICED_BT_SUCCESS )
        {
            goto exit;
        }

        att->handle            = char_value->handle;
        att->type              = char_value->type;
        att->value_length      = char_value->value_length;
        att->value_struct_size = char_value->value_struct_size;

        memcpy( att->value.value, char_value->value.value, char_value->value_length );

        result = wiced_bt_smart_attribute_add_to_list( att_cache_list, att );
    }

    exit:
    bt_smartbridge_att_cache_unlock( cache );
    return result;
}

/******************************************************
 *               Callback Definitions
 ******************************************************/

static wiced_result_t smartbridge_gap_connection_handler( bt_smart_gap_connection_event_t event, wiced_bt_device_address_t* address, wiced_bt_smart_address_type_t type, uint16_t connection_handle )
{
    switch ( event )
    {
        case GAP_CONNECTED:
        {
            /* Update connection handle and state of the socket */
            connecting_socket->state = SOCKET_STATE_LINK_CONNECTED;
            connecting_socket->connection_handle = connection_handle;

            /* Add socket to the connected list */
            bt_smartbridge_socket_manager_insert_socket( connecting_socket );

            /* Notify app thread that link is connected */
            wiced_rtos_set_semaphore( &connecting_socket->semaphore );
            break;
        }
        case GAP_ENCRYPTION_ENABLED:
        {
            /* Update state of the socket to encrypted */
            connecting_socket->state = SOCKET_STATE_LINK_ENCRYPTED;

            /* Notify app thread that link is already encrypted */
            wiced_rtos_set_semaphore( &connecting_socket->semaphore );
            break;
        }
        case GAP_CONNECTION_TIMEOUT:
        case GAP_ENCRYPTION_FAILED:
        case GAP_ENCRYPTION_TIMEOUT:
        {
            /* Notify app thread that link is already encrypted */
            wiced_rtos_set_semaphore( &connecting_socket->semaphore );
            break;
        }
        case GAP_DISCONNECTED:
        {
            wiced_bt_smartbridge_socket_t* removed_socket = NULL;

            /* Remove socket from the connected list */
            if ( bt_smartbridge_socket_manager_remove_socket( connection_handle, &removed_socket ) == WICED_BT_SUCCESS )
            {
                /* Reset connection handle to invalid value */
                removed_socket->connection_handle = SOCKET_INVALID_CONNECTION_HANDLE;

                /* Reset socket state */
                removed_socket->state = SOCKET_STATE_DISCONNECTED;

                /* Mark att cache as inactive and reset reference to cache */
                bt_smartbridge_att_cache_set_active_state( (bt_smartbridge_att_cache_t*)removed_socket->att_cache, WICED_FALSE );
                removed_socket->att_cache = NULL;

                /* Check if disconnection is from host or remote device */

                if ( smartbridge_socket_check_actions_enabled( removed_socket, SOCKET_ACTION_HOST_DISCONNECT ) == WICED_TRUE )
                {
                    /* Disconnection is originated from the host. Notify app thread that disconnection is complete */
                    wiced_rtos_set_semaphore( &removed_socket->semaphore );
                }
                else
                {
                    /* Notify app that connection is disconnected by the remote device */
                    if ( removed_socket->disconnection_callback != NULL )
                    {
                        wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, smartbridge_app_disconnection_handler, (void*)removed_socket );
                    }

                    /* If disconnection happens when connection is still being established. Notify app */
                    if ( connecting_socket == removed_socket )
                    {
                        wiced_rtos_set_semaphore( &connecting_socket->semaphore );
                    }
                }
            }
            else
            {
                /* If disconnection happens when connection is still being established. Notify app */
                if ( connecting_socket != NULL )
                {
                    wiced_rtos_set_semaphore( &connecting_socket->semaphore );
                }
            }


            break;
        }
        default:
        {
            return WICED_BT_UNKNOWN_EVENT;
        }
    }

    return WICED_BT_SUCCESS;
}

static wiced_result_t smartbridge_gatt_indication_notification_handler( uint16_t connection_handle, uint16_t attribute_handle, uint8_t* data, uint16_t length )
{
    wiced_bt_smartbridge_socket_t* socket;

    /* Search for socket with indicated connection handle in the connected list */
    if ( bt_smartbridge_socket_manager_find_socket_by_handle( connection_handle, &socket ) == WICED_BT_SUCCESS )
    {
        if ( bt_smartbridge_att_cache_is_enabled() == WICED_TRUE && socket->att_cache != NULL )
        {

            bt_smartbridge_att_cache_t*      cache          = (bt_smartbridge_att_cache_t*)socket->att_cache;
            wiced_bt_smart_attribute_list_t* att_cache_list = NULL;
            wiced_bt_smart_attribute_t*      att            = NULL;

            bt_smartbridge_att_cache_get_list( cache, &att_cache_list );

            /* Socket found. lock mutex for protected access */
            bt_smartbridge_att_cache_lock( cache );

            /* Search for att in the socket's att list */
            if ( wiced_bt_smart_attribute_search_list_by_handle( att_cache_list, attribute_handle, &att ) == WICED_BT_SUCCESS )
            {
                wiced_bt_uuid_t uuid       = att->type;
                wiced_bool_t    is_new_att = WICED_FALSE;

                /* Check if existing att memory length is sufficient */
                if ( length > att->value_length )
                {
                    /* length isn't sufficient. Remove existing from the list */
                    wiced_bt_smart_attribute_remove_from_list( att_cache_list, attribute_handle );
                    att = NULL;

                    /* Create a new one and marked as new */
                    wiced_bt_smart_attribute_create( &att, WICED_ATTRIBUTE_TYPE_CHARACTERISTIC_VALUE, length );
                    is_new_att = WICED_TRUE;
                }

                /* Copy new value to the att */
                att->handle       = attribute_handle;
                att->type         = uuid;
                att->value_length = length;
                memcpy( att->value.value, data, length );

                if ( is_new_att == WICED_TRUE )
                {
                    /* Add newly created att to the list */
                    wiced_bt_smart_attribute_add_to_list( att_cache_list, att );
                }
            }

            /* Socket found. lock mutex for protected access */
            bt_smartbridge_att_cache_unlock( cache );
        }

        socket->last_notified_attribute_handle = attribute_handle;

        /* Notification callback is called regardless of att cache is enabled or not */
        if ( socket->notification_callback != NULL )
        {
            wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, smartbridge_app_notification_handler, (void*)socket );
        }

        return WICED_BT_SUCCESS;
    }

    return WICED_BT_ERROR;
}

static wiced_result_t smartbridge_gap_bonding_handler( uint16_t connection_handle, const wiced_bt_smart_bond_info_t* bond_info )
{
    wiced_bt_smartbridge_socket_t* socket;

    if ( bt_smartbridge_socket_manager_find_socket_by_handle( connection_handle, &socket ) == WICED_BT_SUCCESS )
    {
        /* Bonding successful. Update socket's bond info and post callback to WICED_NETWORKING_WORKER_THREAD */
        memcpy( &socket->bond_info, bond_info, sizeof( *bond_info ) );

        if ( socket != NULL && socket->bonding_callback != NULL )
        {
            wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, smartbridge_app_pairing_handler, (void*)socket );
        }

        return WICED_BT_SUCCESS;
    }

    return WICED_BT_ERROR;
}

static wiced_result_t smartbridge_app_notification_handler( void* arg )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)arg;

    if ( socket != NULL && socket->notification_callback != NULL )
    {
        socket->notification_callback( socket, socket->last_notified_attribute_handle );
        return WICED_BT_SUCCESS;
    }

    return WICED_BT_ERROR;
}

static wiced_result_t smartbridge_app_disconnection_handler( void* arg )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)arg;

    if ( socket != NULL && socket->disconnection_callback != NULL )
    {
        socket->disconnection_callback( socket );
        return WICED_BT_SUCCESS;
    }

    return WICED_BT_ERROR;
}

static wiced_result_t smartbridge_app_pairing_handler( void* arg )
{
    wiced_bt_smartbridge_socket_t* socket = (wiced_bt_smartbridge_socket_t*)arg;

    if ( socket != NULL && socket->bonding_callback != NULL )
    {
        socket->bonding_callback( socket, &socket->bond_info );
        return WICED_BT_SUCCESS;
    }

    return WICED_BT_ERROR;
}

/******************************************************
 *            Socket Action Helper Functions
 ******************************************************/

wiced_bool_t smartbridge_socket_check_actions_enabled( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    return ( ( socket->actions & action_bits ) == action_bits ) ? WICED_TRUE : WICED_FALSE;
}

wiced_bool_t smartbridge_socket_check_actions_disabled( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    return ( ( socket->actions | ~action_bits ) == ~action_bits ) ? WICED_TRUE : WICED_FALSE;
}

void smartbridge_socket_set_actions( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    socket->actions |= action_bits;
}

void smartbridge_socket_clear_actions( wiced_bt_smartbridge_socket_t* socket, uint8_t action_bits )
{
    socket->actions &= ~action_bits;
}
