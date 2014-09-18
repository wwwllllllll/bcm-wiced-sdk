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
#include "wiced.h"
#include "wiced_rtos.h"
#include "wiced_utilities.h"
#include "wiced_bt.h"
#include "wiced_bt_rfcomm.h"
#include "wiced_bt_rfcomm_socket.h"
#include "bt_mpaf.h"
#include "bt_management_mpaf.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define SEND_COMMAND( func ) \
    do \
    { \
        wiced_result_t retval = func; \
        if ( retval != WICED_BT_SUCCESS ) \
        { \
            WPRINT_LIB_ERROR( ( "Device ID Profile: error sending command\n" ) ); \
            return retval; \
        } \
    } \
    while ( 0 )

#define RECEIVE_EVENT( packet ) \
    do \
    { \
        wiced_result_t retval; \
        if ( packet != NULL ) \
        { \
            bt_packet_pool_free_packet( packet ); \
            packet = NULL; \
        } \
        retval = bt_management_mpaf_wait_for_event( &packet ); \
        if ( retval != WICED_BT_SUCCESS ) \
        { \
            WPRINT_LIB_ERROR( ( "Device ID Profile: Error receiving event\n" ) ); \
            return retval; \
        } \
    } \
    while ( 0 )

/******************************************************
 *                    Constants
 ******************************************************/

#define FEATURE_MASK MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER_MASK

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_bool_t initialised;
    wiced_bool_t sdp_record_added; /* Flag to indicate whether SDP record has been added */

    struct
    {
        void                      (*lock)  ( void );
        void                      (*unlock)( void );
        wiced_mutex_t             mutex;                                      /* Mutex to protect shared data */
        wiced_bt_rfcomm_socket_t* connected_socket_list[RFCOMM_MAX_ENDPOINT]; /* Connected socket list. Use [Endpoint - 1] as array index */
        wiced_bt_rfcomm_socket_t* server_connecting_socket;                   /* Server socket waiting for connection */
    }shared;

} rfcomm_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static wiced_result_t rfcomm_add_sdp_record       ( wiced_bt_rfcomm_socket_t* socket, const char* service_name, const wiced_bt_uuid_t* uuid );
//static wiced_result_t rfcomm_delete_sdp_record    ( wiced_bt_rfcomm_socket_t* socket );
static void           rfcomm_mpaf_rx_data_handler ( uint8_t endpoint, bt_packet_t* packet, uint8_t* data_start, uint32_t data_size );
static void           rfcomm_mpaf_event_handler   ( mpaf_event_opcode_t event, bt_packet_t* packet, mpaf_event_params_t* params );
static void           rfcomm_lock_shared_data     ( void );
static void           rfcomm_unlock_shared_data   ( void );
static void           rfcomm_lock_socket          ( wiced_bt_rfcomm_socket_t* socket );
static void           rfcomm_unlock_socket        ( wiced_bt_rfcomm_socket_t* socket );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* RFCOMM instance */
static rfcomm_t rfcomm_instance = { .initialised = WICED_FALSE };

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_bt_rfcomm_init( void )
{
    wiced_result_t result;

    if ( rfcomm_instance.initialised == WICED_TRUE )
    {
        return WICED_BT_SUCCESS;
    }

    /* Memset shared data */
    memset( &rfcomm_instance, 0, sizeof( rfcomm_instance ) );

    rfcomm_instance.shared.lock   = rfcomm_lock_shared_data;
    rfcomm_instance.shared.unlock = rfcomm_unlock_shared_data;

    /* Initialise data mutex */
    result = wiced_rtos_init_mutex( &rfcomm_instance.shared.mutex );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error initialising RFCOMM data mutex\n") );
        return result;
    }

    /* Initialise MPAF client */
    result = bt_mpaf_register_rfcomm_callback( rfcomm_mpaf_event_handler, rfcomm_mpaf_rx_data_handler );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error registering MPAF RFCOMM callbacks\n") );
        return result;
    }

    rfcomm_instance.initialised      = WICED_TRUE;
    rfcomm_instance.sdp_record_added = WICED_FALSE;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_rfcomm_deinit( void )
{
    if ( rfcomm_instance.initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    /* Deinit MPAF */
    bt_mpaf_register_rfcomm_callback( NULL, NULL );

    /* Deinit access mutex */
    wiced_rtos_deinit_mutex( &rfcomm_instance.shared.mutex );

    /* Reset globals */
    memset( &rfcomm_instance, 0, sizeof( rfcomm_instance ) );
    rfcomm_instance.initialised = WICED_FALSE;
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_rfcomm_listen( wiced_bt_rfcomm_socket_t* socket, wiced_bool_t auto_accept, const char* service_name, const wiced_bt_uuid_t* uuid )
{
    wiced_result_t       result = WICED_BT_ERROR;
    wiced_bt_packet_t*   packet;
    mpaf_event_params_t* params;

    if ( auto_accept == WICED_FALSE )
    {
        return WICED_BT_UNSUPPORTED;
    }

    if ( rfcomm_instance.initialised == WICED_FALSE )
    {
        return WICED_BT_RFCOMM_UNINITIALISED;
    }

    rfcomm_instance.shared.lock( );

    if ( rfcomm_instance.shared.server_connecting_socket != NULL )
    {
        rfcomm_instance.shared.unlock( );
        return WICED_BT_CONNECT_IN_PROGRESS;
    }

    rfcomm_instance.shared.server_connecting_socket = socket;
    rfcomm_instance.shared.unlock( );

    socket->shared.lock( socket );
    socket->shared.status |= RFCOMM_SOCKET_LISTENING;
    socket->shared.unlock( socket );

    /* Create RFCOMM connection. This is an asynchronous function. The result will returned via rfcomm_mpaf_event_handler */
    result = bt_mpaf_rfcomm_create_connection( (const void*)&uuid->value, (uuid_size_t)uuid->size, wiced_bt_device_get_name(), MPAF_RFCOMM_SERVER_MODE, FEATURE_MASK );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error creating RFCOMM connection\n") );
        goto error;
    }

    /* Wait on event queue for MPAF_COMMAND_STATUS event */
    result = bt_management_mpaf_wait_for_event( &packet );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error waiting for MPAF_COMMAND_STATUS event\n") );
        goto error;
    }

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_status.status == MPAF_SUCCESS )
    {
        bt_packet_pool_free_packet( packet );

        if ( FEATURE_MASK == MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER_MASK )
        {
            if ( rfcomm_instance.sdp_record_added == WICED_FALSE )
            {
                result = rfcomm_add_sdp_record( socket, service_name, uuid );
                if ( result != WICED_BT_SUCCESS )
                {
                    goto error;
                }

                rfcomm_instance.sdp_record_added = WICED_TRUE;
            }
        }

        return WICED_BT_SUCCESS;
    }
    else
    {
        bt_packet_pool_free_packet( packet );
        goto error;
    }

    error:

    rfcomm_instance.shared.lock( );
    rfcomm_instance.shared.server_connecting_socket = NULL;
    rfcomm_instance.shared.unlock( );

    socket->shared.lock( socket );
    socket->shared.status &= ~(uint8_t)RFCOMM_SOCKET_LISTENING;
    socket->shared.unlock( socket );

    return result;

}

wiced_result_t wiced_bt_rfcomm_connect( wiced_bt_rfcomm_socket_t* socket, const char* service_name, const wiced_bt_uuid_t* uuid )
{
    wiced_result_t       result = WICED_BT_ERROR;
    wiced_bt_packet_t*   packet;
    mpaf_event_params_t* params;

    if ( rfcomm_instance.initialised == WICED_FALSE )
    {
        return WICED_BT_RFCOMM_UNINITIALISED;
    }

    rfcomm_instance.shared.lock( );

    if ( rfcomm_instance.shared.server_connecting_socket != NULL )
    {
        rfcomm_instance.shared.unlock( );
        return WICED_BT_CONNECT_IN_PROGRESS;
    }

    rfcomm_instance.shared.server_connecting_socket = socket;
    rfcomm_instance.shared.unlock( );

    socket->shared.lock( socket );
    socket->shared.status |= RFCOMM_SOCKET_CONNECTING;
    socket->shared.unlock( socket );

    /* Create RFCOMM connection. This is an asynchronous function. The result will returned via rfcomm_mpaf_event_handler */
    result = bt_mpaf_rfcomm_create_connection( (const void*)&uuid->value, (uuid_size_t)uuid->size, wiced_bt_device_get_name(), MPAF_RFCOMM_CLIENT_MODE, 0 );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error creating RFCOMM connection\n") );
        goto error;
    }

    /* Wait on event queue for MPAF_COMMAND_STATUS event */
    result = bt_management_mpaf_wait_for_event( &packet );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error waiting for MPAF_COMMAND_STATUS event\n") );
        goto error;
    }

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_status.status == MPAF_SUCCESS )
    {

        bt_packet_pool_free_packet( packet );
        return WICED_BT_SUCCESS;
    }
    else
    {
        bt_packet_pool_free_packet( packet );
        goto error;
    }

    error:

    rfcomm_instance.shared.lock( );
    rfcomm_instance.shared.server_connecting_socket = NULL;
    rfcomm_instance.shared.unlock( );

    socket->shared.lock( socket );
    socket->shared.status &= ~(uint8_t)RFCOMM_SOCKET_CONNECTING;
    socket->shared.unlock( socket );

    return result;

}

wiced_result_t wiced_bt_rfcomm_disconnect( wiced_bt_rfcomm_socket_t* socket )
{
    wiced_result_t       result = WICED_BT_ERROR;
    wiced_bt_packet_t*   packet;
    mpaf_event_params_t* params;
    uint8_t              status;
    uint8_t              endpoint;

    if ( rfcomm_instance.initialised == WICED_FALSE )
    {
        return WICED_BT_RFCOMM_UNINITIALISED;
    }

    socket->shared.lock( socket );
    status   = socket->shared.status;
    endpoint = socket->shared.endpoint;
    socket->shared.unlock( socket );

    if ( ( status & RFCOMM_SOCKET_DISCONNECTING ) != 0 )
    {
        return WICED_BT_DISCONNECT_IN_PROGRESS;
    }

    socket->shared.lock( socket );
    socket->shared.status |= RFCOMM_SOCKET_DISCONNECTING; /* Change socket status to DISCONNECTING */
    socket->shared.unlock( socket );

    if ( ( status & RFCOMM_SOCKET_LISTENING ) != 0 || ( status & RFCOMM_SOCKET_CONNECTING ) != 0 )
    {
        /* Disconnect RFCOMM This is an asynchronous function. The result will returned via rfcomm_mpaf_event_handler */
        result = bt_mpaf_rfcomm_create_connection_cancel();
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error cancel RFCOMM connection\n") );
            goto error;
        }

        /* Wait on event queue for MPAF_COMMAND_STATUS event */
        result = bt_management_mpaf_wait_for_event( &packet );
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error waiting for MPAF_COMMAND_COMPLETE event\n") );
            goto error;
        }

        params = (mpaf_event_params_t*)packet->data_start;

        if ( params->command_complete.return_params.rfcomm_create_connection_cancel.status == MPAF_SUCCESS )
        {
            bt_packet_pool_free_packet( packet );
            return WICED_BT_SUCCESS;
        }
        else
        {
            bt_packet_pool_free_packet( packet );
            goto error;
        }
    }
    else if ( ( status & RFCOMM_SOCKET_CONNECTED ) != 0 )
    {
        /* Disconnect RFCOMM This is an asynchronous function. The result will returned via rfcomm_mpaf_event_handler */
        result = bt_mpaf_rfcomm_remove_connection( endpoint );
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error removing RFCOMM connection\n") );
            goto error;
        }

        /* Wait on event queue for MPAF_COMMAND_STATUS event */
        result = bt_management_mpaf_wait_for_event( &packet );
        if ( result != WICED_BT_SUCCESS )
        {
            WPRINT_LIB_ERROR( ("Error waiting on RFCOMM event\n") );
            goto error;
        }

        params = (mpaf_event_params_t*)packet->data_start;

        if ( params->command_status.status == MPAF_SUCCESS )
        {
            bt_packet_pool_free_packet( packet );

            /* Wait on event queue for MPAF_RFCOMM_DISCONNECTION_COMPLETE event */
            result = bt_management_mpaf_wait_for_event( &packet );
            if ( result != WICED_BT_SUCCESS )
            {
                WPRINT_LIB_ERROR( ("Error waiting on RFCOMM event\n") );
                goto error;
            }

            params = (mpaf_event_params_t*)packet->data_start;

            if ( params->rfcomm_disconnection_complete.status == MPAF_SUCCESS )
            {
                bt_packet_pool_free_packet( packet );
                return WICED_BT_SUCCESS;
            }
            else
            {
                bt_packet_pool_free_packet( packet );
                goto error;
            }
        }
        else
        {
            bt_packet_pool_free_packet( packet );
            goto error;
        }
    }

    error:

    /* Reset DISCONNECTING bit */
    socket->shared.lock( socket );
    socket->shared.status &= ~(uint8_t)RFCOMM_SOCKET_DISCONNECTING;
    socket->shared.unlock( socket );

    return result;
}

wiced_result_t wiced_bt_rfcomm_init_socket( wiced_bt_rfcomm_socket_t* socket, wiced_bt_rfcomm_event_handler_t handler, void* arg )
{
    wiced_result_t result;

    if ( socket->id == RFCOMM_SOCKET_ID )
    {
        return WICED_BT_SOCKET_IN_USE;
    }

    socket->id              = RFCOMM_SOCKET_ID;
    socket->handler         = (void*)handler;
    socket->arg             = arg;
    socket->shared.status   = RFCOMM_SOCKET_INITIALISED;
    socket->shared.channel  = 0;
    socket->shared.endpoint = 0;
    socket->shared.lock     = rfcomm_lock_socket;
    socket->shared.unlock   = rfcomm_unlock_socket;

    result = wiced_rtos_init_semaphore( &socket->semaphore );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error initting RFCOMM socket semaphore\n") );
        return result;
    }

    result = wiced_rtos_init_mutex( &socket->shared.mutex );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error initting RFCOMM socket mutex\n") );
        return result;
    }

    result = bt_linked_list_init( &socket->shared.rx_packet_list );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error initting RX packet list\n") );
        return result;
    }

    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_rfcomm_deinit_socket( wiced_bt_rfcomm_socket_t* socket )
{
    if ( socket->id != RFCOMM_SOCKET_ID )
    {
        return WICED_BT_SUCCESS;
    }

//        if ( FEATURE_MASK == MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER_MASK )
//        {
//            uint8_t status;
//
//            socket->shared.lock( socket );
//            status = socket->shared.status;
//            socket->shared.unlock( socket );
//
//            if ( ( status & RFCOMM_SOCKET_TERMINATED ) != 0 )
//            {
//                if ( rfcomm_delete_sdp_record( socket ) != WICED_BT_SUCCESS )
//                {
//                    return WICED_ERROR;
//                }
//            }
//        }

    wiced_rtos_deinit_semaphore( &socket->semaphore );
    wiced_rtos_deinit_mutex( &socket->shared.mutex );
    bt_linked_list_deinit( &socket->shared.rx_packet_list );
    memset( socket, 0, sizeof( *socket ) );
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_rfcomm_send_buffer( wiced_bt_rfcomm_socket_t* socket, const uint8_t* buffer, uint32_t buffer_size )
{
    wiced_bt_packet_t* packet;
    wiced_result_t     result;
    uint8_t*           append_data_ptr;
    uint8_t            status;
    uint8_t            endpoint;

    if ( buffer == NULL || buffer_size == 0 )
    {
        return WICED_BT_BADARG;
    }

    socket->shared.lock( socket );
    status   = socket->shared.status;
    endpoint = socket->shared.endpoint;
    socket->shared.unlock( socket );

    if ( ( status & RFCOMM_SOCKET_CONNECTED ) == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    result = bt_mpaf_rfcomm_create_data_packet( &packet, buffer_size, &append_data_ptr );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error creating RFCOMM packet\n") );
        return result;
    }

    /* Copy buffer content to the packet and update the packet's data length */
    memcpy( append_data_ptr, buffer, buffer_size );

    /* Update packet length */
    append_data_ptr += buffer_size;
    wiced_bt_packet_set_data_end( packet, (const uint8_t*)( append_data_ptr ) );

    /* Send packet via MPAF */
    result = bt_mpaf_rfcomm_send_data_packet( endpoint, packet );
    if ( result != WICED_BT_SUCCESS )
    {
        bt_packet_pool_free_packet( packet );
    }

    return result;
}

wiced_result_t wiced_bt_rfcomm_create_packet( wiced_bt_packet_t** packet, uint32_t data_size, uint8_t** data )
{
    wiced_result_t result = bt_mpaf_rfcomm_create_data_packet( packet, data_size, data );

    if ( result == WICED_BT_SUCCESS )
    {
        (*packet)->owner = BT_PACKET_OWNER_APP;
    }

    return result;
}

wiced_result_t wiced_bt_rfcomm_send_packet(  wiced_bt_rfcomm_socket_t* socket, wiced_bt_packet_t* packet )
{
    uint8_t status;
    uint8_t endpoint;

    if ( packet == NULL )
    {
        return WICED_BT_BADARG;
    }

    socket->shared.lock( socket );
    status   = socket->shared.status;
    endpoint = socket->shared.endpoint;
    socket->shared.unlock( socket );

    if ( ( status & RFCOMM_SOCKET_CONNECTED ) != 0 )
    {
        return bt_mpaf_rfcomm_send_data_packet( endpoint, packet );
    }
    else
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }
}

wiced_result_t wiced_bt_rfcomm_receive_packet( wiced_bt_rfcomm_socket_t* socket, wiced_bt_packet_t** packet, uint8_t** data, uint32_t* data_size, uint32_t timeout_ms )
{
    wiced_result_t result;
    uint8_t        status;

    if ( packet == NULL || data == NULL || data_size == NULL )
    {
        return WICED_BT_BADARG;
    }

    socket->shared.lock( socket );
    status = socket->shared.status;
    socket->shared.unlock( socket );

    if ( ( status & RFCOMM_SOCKET_CONNECTED ) == 0 )
    {
        return WICED_BT_SOCKET_NOT_CONNECTED;
    }

    socket->shared.lock( socket );
    socket->shared.status |= RFCOMM_SOCKET_WAITING_FOR_PACKET;
    socket->shared.unlock( socket );

    /* Wait in a semaphore. If successful, data is available */
    result = wiced_rtos_get_semaphore( &socket->semaphore, timeout_ms );

    socket->shared.lock( socket );
    status = socket->shared.status;
    socket->shared.status &= ~(uint8_t)RFCOMM_SOCKET_WAITING_FOR_PACKET;

    if ( ( status & RFCOMM_SOCKET_CONNECTED ) == 0 )
    {
        result = WICED_BT_SOCKET_NOT_CONNECTED;
    }
    else if ( result == WICED_BT_SUCCESS )
    {
        /* Data is available. Retreive data */
        bt_list_node_t* removed_node;

        result = bt_linked_list_remove_from_front( &socket->shared.rx_packet_list, &removed_node );

        if ( result == WICED_BT_SUCCESS )
        {
            *packet          = (bt_packet_t*)removed_node->data;
            (*packet)->owner = BT_PACKET_OWNER_APP;
            *data            = (*packet)->data_start;
            *data_size       = (uint32_t)( (*packet)->data_end - (*packet)->data_start );
        }
    }

    socket->shared.unlock( socket );

    return result;
}

static wiced_result_t rfcomm_add_sdp_record( wiced_bt_rfcomm_socket_t* socket, const char* service_name, const wiced_bt_uuid_t* uuid )
{
    bt_packet_t*                  packet = NULL;
    mpaf_event_params_t*          params = NULL;
    uint8_t                       record_handle;
    uint8_t                       channel;
    mpaf_protocol_element_param_t protocol_descriptor_list[2];
    mpaf_uuid_param_t             uuid_sequence;

    /* Create SDP record for Device ID Profile */
    SEND_COMMAND( bt_mpaf_sdp_create_record() );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_create_record.status != MPAF_SUCCESS )
    {
        bt_packet_pool_free_packet( packet );
        return WICED_BT_ERROR;
    }

    /* SDP record successfully created. Retrieve record handle */
    socket->shared.lock( socket );
    socket->shared.sdp_record_handle = params->command_complete.return_params.sdp_create_record.record_handle;
    record_handle                    = socket->shared.sdp_record_handle;
    channel                          = socket->shared.channel;
    socket->shared.unlock( socket );

    protocol_descriptor_list[0].protocol_uuid    = 0x0100;
    protocol_descriptor_list[0].number_of_params = 0;
    protocol_descriptor_list[0].params[0]        = 0;
    protocol_descriptor_list[0].params[1]        = 0;
    protocol_descriptor_list[1].protocol_uuid    = 0x0003;
    protocol_descriptor_list[1].number_of_params = 1;
    protocol_descriptor_list[1].params[0]        = channel;
    protocol_descriptor_list[1].params[1]        = 0;

    /* Add Protocol descriptor list */
    SEND_COMMAND( bt_mpaf_sdp_add_protocol_descriptor_list( record_handle, protocol_descriptor_list, 2 ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_protocol_descriptor_list.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    uuid_sequence.uuid_size = uuid->size;
    memcpy( uuid_sequence.uuid, &uuid->value, sizeof( uuid_sequence.uuid ));

    /* Add Service Class UUID */
    SEND_COMMAND( bt_mpaf_sdp_add_uuid_sequence( record_handle, 0x0001, &uuid_sequence, 1 ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_uuid_sequence.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add device name Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( record_handle,
                                              0x0100,
                                              MPAF_SDP_ATTRIBUTE_STRING,
                                              strnlen( service_name, 21 ),
                                              (const void*)service_name ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    uuid_sequence.uuid_size = UUID_SIZE_16BIT;
    uuid_sequence.uuid[0]   = 0x02;
    uuid_sequence.uuid[1]   = 0x10;
    uuid_sequence.uuid[2]   = 0x00;

    /* Add Service Class UUID */
    SEND_COMMAND( bt_mpaf_sdp_add_uuid_sequence( record_handle, 0x0005, &uuid_sequence, 1 ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_uuid_sequence.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    return WICED_BT_SUCCESS;

    delete_record:

    SEND_COMMAND( bt_mpaf_sdp_delete_record( record_handle ) );
    RECEIVE_EVENT( packet );
    bt_packet_pool_free_packet( packet );
    return WICED_BT_ERROR;
}

//static wiced_result_t rfcomm_delete_sdp_record( wiced_bt_rfcomm_socket_t* socket )
//{
//    bt_packet_t*         packet = NULL;
//    mpaf_event_params_t* params = NULL;
//    uint8_t              record_handle;
//
//    return WICED_BT_SUCCESS;
//
//    socket->shared.lock( socket );
//    record_handle = socket->shared.sdp_record_handle;
//    socket->shared.unlock( socket );
//
//    SEND_COMMAND( bt_mpaf_sdp_delete_record( record_handle ) );
//    RECEIVE_EVENT( packet );
//
//    params = (mpaf_event_params_t*)packet->data_start;
//
//    if ( params->command_complete.return_params.sdp_delete_record.status == MPAF_SUCCESS )
//    {
//        bt_packet_pool_free_packet( packet );
//
//        socket->shared.lock( socket );
//        socket->shared.sdp_record_handle = 0;
//        socket->shared.unlock( socket );
//
//        return WICED_BT_SUCCESS;
//    }
//    else
//    {
//        bt_packet_pool_free_packet( packet );
//        return WICED_ERROR;
//    }
//}

static void rfcomm_mpaf_rx_data_handler( uint8_t endpoint, bt_packet_t* packet, uint8_t* data_start, uint32_t data_size )
{
    UNUSED_PARAMETER( data_start );
    UNUSED_PARAMETER( data_size );

    wiced_bt_rfcomm_socket_t* socket;

    rfcomm_instance.shared.lock();
    socket = rfcomm_instance.shared.connected_socket_list[endpoint - 1];
    rfcomm_instance.shared.unlock();

    socket->shared.lock( socket );

    bt_linked_list_insert_at_rear( &socket->shared.rx_packet_list, &packet->node );

    socket->shared.unlock( socket );

    if ( wiced_rtos_set_semaphore( &socket->semaphore ) != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ("Error setting RFCOMM socket semaphore\n") );
    }

    ((wiced_bt_rfcomm_event_handler_t)socket->handler)( socket, RFCOMM_EVENT_INCOMING_PACKET_READY, socket->arg );

}

static void rfcomm_mpaf_event_handler( mpaf_event_opcode_t event, bt_packet_t* packet, mpaf_event_params_t* params )
{
    switch ( event )
    {
        case MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER:
        {
            wiced_bt_rfcomm_socket_t* socket;

            rfcomm_instance.shared.lock();
            socket = rfcomm_instance.shared.server_connecting_socket;
            rfcomm_instance.shared.unlock();

            socket->shared.lock( socket );
            socket->shared.channel = params->rfcomm_service_channel_number.channel;
            socket->shared.unlock( socket );
            break;
        }

        case MPAF_COMMAND_COMPLETE:
        {
            switch ( params->command_complete.command_opcode )
            {
                case MPAF_RFCOMM_CREATE_CONNECTION_CANCEL:

                    /* Notify app thread that waits for the MPAF_COMMAND_COMPLETE event */
                    if ( bt_management_mpaf_notify_event_received( packet ) != WICED_BT_SUCCESS )
                    {
                        WPRINT_LIB_ERROR( ("Error pushing event packet to queue\n") );
                    }

                    return;

                default:
                    break;
            }
            break;
        }

        case MPAF_COMMAND_STATUS:
        {
            switch ( params->command_status.command_opcode )
            {
                case MPAF_RFCOMM_CREATE_CONNECTION:
                case MPAF_RFCOMM_REMOVE_CONNECTION:

                    /* Notify app thread that waits for the MPAF_COMMAND_STATUS event */
                    if ( bt_management_mpaf_notify_event_received( packet ) != WICED_BT_SUCCESS )
                    {
                        WPRINT_LIB_ERROR( ("Error pushing event packet to queue\n") );
                    }

                    return;

                default:
                    break;
            }
            break;
        }

        case MPAF_RFCOMM_CONNECTION_COMPLETE:

            if ( params->rfcomm_connection_complete.status == MPAF_SUCCESS )
            {
                wiced_bt_rfcomm_socket_t* socket;
                wiced_bool_t is_connecting;

                rfcomm_instance.shared.lock();
                socket = rfcomm_instance.shared.server_connecting_socket;
                rfcomm_instance.shared.connected_socket_list[params->rfcomm_connection_complete.endpoint - 1] = socket;
                rfcomm_instance.shared.server_connecting_socket = NULL;
                rfcomm_instance.shared.unlock();

                /* Update socket */
                socket->shared.lock( socket );

                is_connecting = ( ( socket->shared.status & RFCOMM_SOCKET_CONNECTING ) != 0 ) ? WICED_TRUE : WICED_FALSE;

                if ( is_connecting == WICED_TRUE )
                {
                    socket->shared.status &= ~(uint8_t)RFCOMM_SOCKET_CONNECTING;
                }
                else
                {
                    socket->shared.status &= ~(uint8_t)RFCOMM_SOCKET_LISTENING;
                }

                socket->shared.status  |= RFCOMM_SOCKET_CONNECTED;
                socket->shared.endpoint = params->rfcomm_connection_complete.endpoint;
                socket->shared.unlock( socket );

                /* Notify app via callback */
                if ( is_connecting == WICED_TRUE )
                {
                    ((wiced_bt_rfcomm_event_handler_t)socket->handler)( socket, RFCOMM_EVENT_CLIENT_CONNECTED, socket->arg );
                }
                else
                {
                    ((wiced_bt_rfcomm_event_handler_t)socket->handler)( socket, RFCOMM_EVENT_CONNECTION_ACCEPTED, socket->arg );
                }
            }
            else
            {
                wiced_bt_rfcomm_socket_t* socket;

                /* Update shared data */
                rfcomm_instance.shared.lock();
                socket = rfcomm_instance.shared.server_connecting_socket; /* Save socket pointer */
                rfcomm_instance.shared.server_connecting_socket = NULL;   /* Already connected. Reset server_connecting_socket */
                rfcomm_instance.shared.unlock();

                /* Update socket */
                socket->shared.lock( socket );
                socket->shared.status &= ~(uint8_t)( RFCOMM_SOCKET_LISTENING | RFCOMM_SOCKET_CONNECTING );
                socket->shared.unlock( socket );

                ((wiced_bt_rfcomm_event_handler_t)socket->handler)( socket, RFCOMM_EVENT_CONNECTION_ERROR, socket->arg );
            }

            break;

        case MPAF_RFCOMM_DISCONNECTION_COMPLETE:

            if ( params->rfcomm_connection_complete.status == MPAF_SUCCESS )
            {
                wiced_bt_rfcomm_socket_t* socket;
                wiced_bool_t disconnect_by_host;

                /* Update socket list */
                rfcomm_instance.shared.lock();
                socket = rfcomm_instance.shared.connected_socket_list[params->rfcomm_connection_complete.endpoint - 1];
                rfcomm_instance.shared.connected_socket_list[params->rfcomm_connection_complete.endpoint - 1] = NULL;
                rfcomm_instance.shared.unlock();

                /* Update socket */
                socket->shared.lock( socket );
                disconnect_by_host       = ( socket->shared.status & RFCOMM_SOCKET_DISCONNECTING ) ? WICED_TRUE : WICED_FALSE;
                socket->shared.status   &= ~(uint8_t)( RFCOMM_SOCKET_CONNECTED | RFCOMM_SOCKET_DISCONNECTING );
                socket->shared.status   |= RFCOMM_SOCKET_TERMINATED;
                socket->shared.endpoint  = 0;
                socket->shared.unlock( socket );

                /* Notify waiting thread that connection is down */
                if ( wiced_rtos_set_semaphore( &socket->semaphore ) != WICED_BT_SUCCESS )
                {
                    WPRINT_LIB_ERROR( ("Error setting RFCOMM socket semaphore\n") );
                }

                if ( disconnect_by_host == WICED_TRUE )
                {
                    /* Notify app thread that waits for the MPAF_RFCOMM_DISCONNECTION_COMPLETE event */
                    if ( bt_management_mpaf_notify_event_received( packet ) != WICED_BT_SUCCESS )
                    {
                        WPRINT_LIB_ERROR( ("Error pushing event packet to queue\n") );
                    }

                    return;
                }
                else
                {
                    ((wiced_bt_rfcomm_event_handler_t)socket->handler)( socket, RFCOMM_EVENT_CONNECTION_DISCONNECTED, socket->arg );
                }
            }
            else
            {
                WPRINT_LIB_ERROR( ("Error pushing event packet to queue\n") );
            }

            break;

        default:
            break;
    }

    /* Packet is not needed anymore. Delete! */
    bt_packet_pool_free_packet( packet );
}

static void rfcomm_lock_shared_data( void )
{
    wiced_result_t result = wiced_rtos_lock_mutex( &rfcomm_instance.shared.mutex );
    UNUSED_PARAMETER( result );
    wiced_assert( "Error locking RFCOMM socket mutex\n", result == WICED_BT_SUCCESS );
}

static void rfcomm_unlock_shared_data( void )
{
    wiced_result_t result = wiced_rtos_unlock_mutex( &rfcomm_instance.shared.mutex );
    UNUSED_PARAMETER( result );
    wiced_assert( "Error unlocking RFCOMM socket mutex\n", result == WICED_BT_SUCCESS );
}

static void rfcomm_lock_socket( wiced_bt_rfcomm_socket_t* socket )
{
    wiced_result_t result = wiced_rtos_lock_mutex( &socket->shared.mutex );
    UNUSED_PARAMETER( result );
    wiced_assert( "Error locking RFCOMM socket mutex\n", result == WICED_BT_SUCCESS );
}

static void rfcomm_unlock_socket( wiced_bt_rfcomm_socket_t* socket )
{
    wiced_result_t result = wiced_rtos_unlock_mutex( &socket->shared.mutex );
    UNUSED_PARAMETER( result );
    wiced_assert( "Error unlocking RFCOMM socket mutex\n", result == WICED_BT_SUCCESS );
}
