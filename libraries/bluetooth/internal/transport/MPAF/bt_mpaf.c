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
#include "bt_firmware.h"
#include "bt_firmware_image.h"
#include "bt_mpaf.h"
#include "bt_mpaf_interface.h"
#include "bt_packet_internal.h"
#include "bt_transport_driver.h"
#include "bt_transport_thread.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_SERVICE_NAME_LEN        21
#define MAX_ALLOCATE_PACKET_ATTEMPT 50

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

extern wiced_result_t bt_transport_driver_bus_read_handler        ( bt_packet_t** packet );
static wiced_result_t bt_transport_driver_event_handler           ( bt_transport_driver_event_t event );
static wiced_result_t bt_transport_thread_received_packet_handler ( bt_packet_t* packet );
static wiced_result_t bt_mpaf_send_command_packet_internal        ( bt_packet_t* packet, mpaf_command_packet_t* command_packet, uint32_t params_size );
static wiced_result_t bt_mpaf_application_callback                ( void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const mpaf_command_header_t mpaf_command_header_template =
{
    .header        =
    {
        .endpoint  = 0,
        .dir       = 0,
        .port      = 3,
        .reserved  = 0,
        .length    = 0
     },
    .control_class = 0,
    .opcode        = 0,
};

static const mpaf_data_header_t mpaf_data_header_template =
{
    .header        =
    {
        .endpoint  = 1,
        .dir       = 0,
        .port      = 3,
        .reserved  = 0,
        .length    = 0
    },
};

static mpaf_rx_data_packet_cb_t mpaf_rx_data_packet_callback = NULL;
static mpaf_event_cb_t          mpaf_system_event_callback   = NULL;
static mpaf_event_cb_t          mpaf_rfcomm_event_callback   = NULL;
static mpaf_event_cb_t          mpaf_sdp_event_callback      = NULL;
static wiced_bool_t             mpaf_initialised             = WICED_FALSE;
static bt_packet_pool_t         mpaf_data_packet_pool;
static bt_packet_pool_t         mpaf_command_packet_pool;
static bt_packet_pool_t         mpaf_event_packet_pool;
static wiced_mutex_t            mpaf_command_mutex;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_mpaf_init( mpaf_event_cb_t system_event_cb )
{
    wiced_result_t result;

    if ( mpaf_initialised == WICED_TRUE )
    {
        return WICED_BT_SUCCESS;
    }

    mpaf_initialised = WICED_TRUE;

    /* Initialise packet pools */
    result = bt_packet_pool_init( &mpaf_command_packet_pool, MPAF_COMMAND_PACKET_COUNT, MPAF_COMMAND_HEADER_SIZE, MPAF_COMMAND_PARAMS_SIZE );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error initialising MPAF command packet pool\n" ) );
        goto error;
    }

    result = bt_packet_pool_init( &mpaf_event_packet_pool, MPAF_EVENT_PACKET_COUNT, MPAF_EVENT_HEADER_SIZE, MPAF_EVENT_PARAMS_SIZE );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error initialising MPAF event packet pool\n" ) );
        goto error;
    }

    result = bt_packet_pool_init( &mpaf_data_packet_pool, MPAF_DATA_PACKET_COUNT, MPAF_DATA_HEADER_SIZE, MPAF_DATA_MTU_SIZE );
    if ( result != WICED_BT_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error initialising MPAF data packet pool\n" ) );
        goto error;
    }

    /* Download firmware */
    result = bt_firmware_download( bt_mpaf_firmware_image, bt_mpaf_firmware_size, bt_mpaf_firmware_version );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error download MPAF firmware\n" ) );
        goto error;
    }

    /* Switch to MPAF mode. Initialise MPAF transport driver */
    result = bt_transport_driver_init( bt_transport_driver_event_handler, bt_transport_driver_bus_read_handler );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error initialising MPAF driver\n" ) );
        goto error;
    }

    /* Initialise globals */
    mpaf_system_event_callback = system_event_cb;

    /* Create MPAF command mutex */
    result = wiced_rtos_init_mutex( &mpaf_command_mutex );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error creating MPAF command mutex\n" ) );
        goto error;
    }

    /* Initialise BT transport thread */
    result = bt_transport_thread_init( bt_transport_thread_received_packet_handler );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_ERROR( ( "Error initialising BT transport thread\n" ) );
        goto error;
    }

    return WICED_BT_SUCCESS;

    error:
    bt_mpaf_deinit();
    return result;
}

wiced_result_t bt_mpaf_deinit( void )
{
    if ( mpaf_initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    /* Initialise BT transport thread */
    bt_transport_thread_deinit( );

    /* Delete MPAF command mutex */
    wiced_rtos_deinit_mutex( &mpaf_command_mutex );

    /* Deinitialise MPAF transport driver */
    bt_transport_driver_deinit( );

    /* Deinitialise packet pools */
    bt_packet_pool_deinit( &mpaf_command_packet_pool );
    bt_packet_pool_deinit( &mpaf_event_packet_pool );
    bt_packet_pool_deinit( &mpaf_data_packet_pool );

    /* Reset globals */
    mpaf_rx_data_packet_callback = NULL;
    mpaf_system_event_callback   = NULL;
    mpaf_rfcomm_event_callback   = NULL;
    mpaf_sdp_event_callback      = NULL;
    mpaf_initialised             = WICED_FALSE;

    return WICED_BT_SUCCESS;
}

wiced_bool_t bt_mpaf_is_initialised( void )
{
    return mpaf_initialised;
}

wiced_result_t bt_mpaf_register_rfcomm_callback( mpaf_event_cb_t event_cb, mpaf_rx_data_packet_cb_t rx_data_packet_cb )
{
    mpaf_rfcomm_event_callback = event_cb;
    mpaf_rx_data_packet_callback = rx_data_packet_cb;
    return WICED_BT_SUCCESS;
}

wiced_result_t bt_mpaf_register_sdp_callback( mpaf_event_cb_t event_cb )
{
    mpaf_sdp_event_callback = event_cb;
    return WICED_BT_SUCCESS;
}

wiced_result_t bt_mpaf_open_transport_channel( mpaf_channel_direction_t direction )
{
    bt_packet_t* packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, MPAF_OPEN_TRANSPORT_CHANNEL_PARAMS_SIZE );
    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_OPEN_TRANSPORT_CHANNEL;
        command_packet->params[0]     = direction;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, MPAF_OPEN_TRANSPORT_CHANNEL_PARAMS_SIZE );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_close_transport_channel( uint8_t endpoint, mpaf_channel_direction_t direction )
{
    bt_packet_t*   packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, MPAF_CLOSE_TRANSPORT_CHANNEL_PARAMS_SIZE );
    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_CLOSE_TRANSPORT_CHANNEL;
        command_packet->params[0] = direction;
        command_packet->params[1] = endpoint;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, MPAF_CLOSE_TRANSPORT_CHANNEL_PARAMS_SIZE );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_get_controller_status( void )
{
    bt_packet_t*   packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, MPAF_GET_CONTROLLER_STATUS_PARAMS_SIZE );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;
        wiced_result_t result;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_GET_CONTROLLER_STATUS;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, MPAF_GET_CONTROLLER_STATUS_PARAMS_SIZE );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_rfcomm_create_connection( const void* uuid, uuid_size_t uuid_size, const char* service_name, mpaf_rfcomm_mode_t mode, uint8_t feature_mask )
{
    bt_packet_t*   packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, MPAF_RFCOMM_CREATE_CONNECTION_PARAMS_SIZE );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;
        uint32_t i = 0;
        uint8_t service_name_length = strnlen( service_name, MAX_SERVICE_NAME_LEN );

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_RFCOMM_CREATE_CONNECTION;

        /* UUID size: 1 byte */
        command_packet->params[i++] = uuid_size;

        /* Copy UUID content */
        memcpy( &command_packet->params[i], uuid, uuid_size );
        i += uuid_size;

        /* Service name length: 1 byte */
        command_packet->params[i++] = service_name_length;

        /* Copy service name */
        memcpy( &command_packet->params[i], service_name, service_name_length );
        i += service_name_length;

        /* Server mode: 1 byte */
        command_packet->params[i++] = mode;

        /* Feature mask: 1 byte */
        command_packet->params[i++] = feature_mask;

        /* Reserved: 3 bytes. Reset to 0x000000 and increment parameter index */
        command_packet->params[i++] = 0;
        command_packet->params[i++] = 0;
        command_packet->params[i++] = 0;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, i );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_rfcomm_remove_connection( uint8_t endpoint )
{
    bt_packet_t*   packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, MPAF_RFCOMM_REMOVE_CONNECTION_PARAMS_SIZE );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_RFCOMM_REMOVE_CONNECTION;

        /* Endpoint: 1 byte */
        command_packet->params[0] = endpoint;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, MPAF_RFCOMM_REMOVE_CONNECTION_PARAMS_SIZE );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_rfcomm_create_connection_cancel( void )
{
    bt_packet_t*   packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, MPAF_RFCOMM_CREATE_CONNECTION_CANCEL_PARAMS_SIZE );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_RFCOMM_CREATE_CONNECTION_CANCEL;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, MPAF_RFCOMM_CREATE_CONNECTION_CANCEL_PARAMS_SIZE );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_rfcomm_create_data_packet( bt_packet_t** packet, uint32_t data_size, uint8_t** data )
{
    wiced_result_t result = bt_mpaf_create_packet( packet, MPAF_DATA_PACKET, data_size );

    if ( result == WICED_BT_SUCCESS )
    {
        *data = ( *packet )->data_start;
    }

    return result;
}

wiced_result_t bt_mpaf_rfcomm_send_data_packet( uint8_t endpoint, bt_packet_t* packet )
{
    mpaf_data_packet_t* data_packet = (mpaf_data_packet_t*)packet->packet_start;

    data_packet->header.header.endpoint = endpoint;
    data_packet->header.header.length = (uint32_t)( packet->data_end - data_packet->data_start );

    /* Send packet to MPAF worker thread for processing */
    return bt_transport_thread_send_packet( packet );
}

wiced_result_t bt_mpaf_sdp_create_record( void )
{
    bt_packet_t* packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, 0 );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_SDP_CREATE_RECORD;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, 0 );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_sdp_delete_record( uint8_t record_handle )
{
    bt_packet_t* packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, 1 );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_SDP_DELETE_RECORD;
        command_packet->params[0] = record_handle;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, 1 );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_sdp_add_protocol_descriptor_list( uint8_t record_handle, const mpaf_protocol_element_param_t* element_list, uint8_t list_size )
{

    bt_packet_t* packet;
    wiced_result_t result;
    uint32_t packet_length = sizeof( record_handle ) + sizeof( list_size );
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t k = 0;

    /* Calculate packet length */
    for ( j = 0; j < list_size; j++ )
    {
        packet_length += sizeof( element_list[j].protocol_uuid );
        packet_length += sizeof( element_list[j].number_of_params );
        packet_length += element_list[j].number_of_params * sizeof( element_list[j].params[0] );
    }

    result = bt_mpaf_create_dynamic_packet( &packet, MPAF_COMMAND_PACKET, packet_length );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_SDP_ADD_PROTOCOL_DESCRIPTOR_LIST;

        /* Record handle: 1 byte */
        command_packet->params[i++] = record_handle;

        /* Number of elements: 1 byte */
        command_packet->params[i++] = list_size;

        for ( j = 0; j < list_size; j++ )
        {
            /* Protocol UUID: 2 bytes */
            memcpy( &command_packet->params[i], &element_list[j].protocol_uuid, sizeof( element_list[j].protocol_uuid ) );
            i += sizeof( element_list[j].protocol_uuid );

            /* Number of params: 1 byte */
            command_packet->params[i++] = element_list[j].number_of_params;

            /* Parameters: 2 byte each. Max 2 parameters */
            for ( k = 0; k < element_list[j].number_of_params; k++ )
            {
                memcpy( &command_packet->params[i], &element_list[j].params[k], sizeof( element_list[j].params[k] ) );
                i += sizeof( element_list[j].params[k] );
            }
        }

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, i );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_sdp_add_attributes( uint8_t record_handle, uint16_t attribute_id, mpaf_sdp_attribute_type_t attribute_type, uint8_t attribute_length, const void* attribute_value )
{
    uint32_t packet_length = sizeof( record_handle ) + sizeof( attribute_id ) + sizeof( attribute_type ) + sizeof( attribute_length ) + attribute_length;
    bt_packet_t* packet;
    wiced_result_t result;

    result = bt_mpaf_create_packet( &packet, MPAF_COMMAND_PACKET, packet_length );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;
        uint32_t i = 0;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_SDP_ADD_ATTRIBUTES;
        command_packet->params[i++] = record_handle;

        memcpy( &command_packet->params[i], &attribute_id, sizeof( attribute_id ) );
        i += sizeof( attribute_id );

        command_packet->params[i++] = attribute_type;
        command_packet->params[i++] = attribute_length;

        memcpy( &command_packet->params[i], attribute_value, attribute_length );
        i += attribute_length;

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, i );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

wiced_result_t bt_mpaf_sdp_add_uuid_sequence( uint8_t record_handle, uint16_t attribute_id, const mpaf_uuid_param_t* uuid_list, uint8_t list_size )
{
    uint32_t params_length = sizeof( record_handle ) + sizeof( attribute_id );
    bt_packet_t* packet;
    wiced_result_t result;
    uint32_t i = 0;
    uint32_t j = 0;

    /* Calculate packet length */
    for ( j = 0; j < list_size; j++ )
    {
        params_length += sizeof( uuid_list[j].uuid_size );
        params_length += uuid_list[j].uuid_size;
    }

    result = bt_mpaf_create_dynamic_packet( &packet, MPAF_COMMAND_PACKET, params_length );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        mpaf_command_packet_t* command_packet = (mpaf_command_packet_t*)packet->packet_start;

        wiced_rtos_lock_mutex( &mpaf_command_mutex );

        /* Fill packet fields */
        command_packet->header.opcode = MPAF_SDP_ADD_UUID_SEQUENCE;

        /* Record handle: 1 bytes */
        command_packet->params[i++] = record_handle;

        /* Attribute ID: 2 bytes */
        memcpy( &command_packet->params[i], &attribute_id, sizeof( attribute_id ) );
        i += sizeof( attribute_id );

        /* Number of UUIDs: 1 byte */
        command_packet->params[i++] = list_size;

        for ( j = 0; j < list_size; j++ )
        {
            /* UUID size: 1 byte */
            command_packet->params[i++] = uuid_list[j].uuid_size;

            /* UUID: uuid_size bytes */
            memcpy( &command_packet->params[i], &uuid_list[j].uuid, uuid_list[j].uuid_size );
            i += uuid_list[j].uuid_size;
        }

        result = bt_mpaf_send_command_packet_internal( packet, command_packet, i );

        wiced_rtos_unlock_mutex( &mpaf_command_mutex );

        return result;
    }
    else
    {
        return result;
    }
}

static wiced_result_t bt_mpaf_send_command_packet_internal( bt_packet_t* packet, mpaf_command_packet_t* command_packet, uint32_t params_size )
{
    /* Set length in packet == number of parameters + size of control_class (1 byte) + size of opcode (1 byte) */
    command_packet->header.header.length = params_size + sizeof( command_packet->header.control_class ) + sizeof( command_packet->header.opcode );

    /* Set the end of data section (also end of packet) */
    packet->data_end = &command_packet->params[params_size];

    /* Send packet to MPAF worker thread for processing */
    return bt_transport_thread_send_packet( packet );
}

wiced_result_t bt_mpaf_create_dynamic_packet( bt_packet_t** packet, bt_mpaf_packet_type_t packet_type, uint32_t data_size )
{
    wiced_result_t        result;
    mpaf_common_header_t* header;

    switch ( packet_type )
    {
        case MPAF_COMMAND_PACKET:
        {
            result = bt_packet_pool_dynamic_allocate_packet( packet, MPAF_COMMAND_HEADER_SIZE, data_size );

            if ( result == WICED_SUCCESS )
            {
                memcpy( ( *packet )->packet_start, &mpaf_command_header_template, sizeof( mpaf_command_header_template ) );
            }
            break;
        }
        case MPAF_DATA_PACKET:
        {
            result = bt_packet_pool_dynamic_allocate_packet( packet, MPAF_DATA_HEADER_SIZE, data_size );

            if ( result == WICED_SUCCESS )
            {
                memcpy( ( *packet )->packet_start, &mpaf_data_header_template, sizeof( mpaf_data_header_template ) );
            }
            break;
        }
        default:
        {
            return WICED_BT_UNKNOWN_PACKET;
        }
    }

    if ( result == WICED_SUCCESS )
    {
        header = (mpaf_common_header_t*)( ( *packet )->packet_start );
        header->packet_type = 0xA;
    }

    return result;
}

wiced_result_t bt_mpaf_create_packet( bt_packet_t** packet, bt_mpaf_packet_type_t packet_type, uint32_t data_size )
{
    mpaf_common_header_t* header;
    wiced_result_t result  = WICED_BT_ERROR;
    uint32_t       attempt = 0;

    while( result != WICED_BT_SUCCESS )
    {
        switch ( packet_type )
        {
            case MPAF_COMMAND_PACKET:
            {
                result = bt_packet_pool_allocate_packet( &mpaf_command_packet_pool, packet );

                if ( result == WICED_SUCCESS )
                {
                    memcpy( ( *packet )->packet_start, &mpaf_command_header_template, sizeof( mpaf_command_header_template ) );
                }
                break;
            }
            case MPAF_DATA_PACKET:
            {
                result = bt_packet_pool_allocate_packet( &mpaf_data_packet_pool, packet );

                if ( result == WICED_SUCCESS )
                {
                    memcpy( ( *packet )->packet_start, &mpaf_data_header_template, sizeof( mpaf_data_header_template ) );
                }
                break;
            }
            case MPAF_EVENT_PACKET:
            {
                result = bt_packet_pool_allocate_packet( &mpaf_event_packet_pool, packet );
                break;
            }
            default:
            {
                return WICED_BT_UNKNOWN_PACKET;
            }
        }

        if ( result != WICED_SUCCESS )
        {
            /* Sleep for 10 milliseconds to let other threads run and release packets */
            wiced_rtos_delay_milliseconds( 10 );
            attempt++;
            wiced_assert( "Maximum attempt reached! Check for packet leak!", attempt < MAX_ALLOCATE_PACKET_ATTEMPT );
            return WICED_BT_PACKET_POOL_FATAL_ERROR;
        }
    }



    if ( result == WICED_SUCCESS )
    {
        header = (mpaf_common_header_t*)( ( *packet )->packet_start );
        header->packet_type = 0xA;
    }

    return result;
}

static wiced_result_t bt_transport_driver_event_handler( bt_transport_driver_event_t event )
{
    if ( event == TRANSPORT_DRIVER_INCOMING_PACKET_READY )
    {
        return bt_transport_thread_notify_packet_received();
    }

    return WICED_BT_ERROR;
}

static wiced_result_t bt_transport_thread_received_packet_handler( bt_packet_t* packet )
{
    return wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, bt_mpaf_application_callback, (void*) packet );
}

static wiced_result_t bt_mpaf_application_callback( void* arg )
{
    bt_packet_t*          packet        = (bt_packet_t*)arg;
    mpaf_common_packet_t* common_packet = (mpaf_common_packet_t*)packet->packet_start;

    wiced_assert( "mpaf_sdp_event_callback isn't set",      mpaf_sdp_event_callback    != NULL );
    wiced_assert( "mpaf_rfcomm_event_callback isn't set",   mpaf_rfcomm_event_callback != NULL );
    wiced_assert( "mpaf_system_event_callback isn't set",   mpaf_system_event_callback != NULL );
    wiced_assert( "mpaf_rx_data_packet_callback isn't set", mpaf_rx_data_packet_callback != NULL );

    /* Check if incoming packet is an event or data packet */
    if ( common_packet->header.header.endpoint == 0 )
    {
        mpaf_event_packet_t* event_packet = (mpaf_event_packet_t*)common_packet;

        /* Update packet's data_start pointer and allocated_data_size to reflect an event packet */
        packet->data_start = packet->packet_start + sizeof(mpaf_event_header_t);

        /* Decode event and direct it to the appropriate callback */
        if ( event_packet->header.opcode == MPAF_COMMAND_COMPLETE )
        {
            mpaf_event_params_t* params = (mpaf_event_params_t*)event_packet->params;

            if ( params->command_complete.command_opcode >= 0xA0 )
            {
                mpaf_sdp_event_callback( event_packet->header.opcode, packet, (mpaf_event_params_t*)event_packet->params );
            }
            else if ( params->command_complete.command_opcode >= 0x80 )
            {
                mpaf_rfcomm_event_callback( event_packet->header.opcode, packet, (mpaf_event_params_t*)event_packet->params );
            }
            else
            {
                mpaf_system_event_callback( event_packet->header.opcode, packet, (mpaf_event_params_t*)event_packet->params );
            }
        }
        else
        {
            mpaf_rfcomm_event_callback( event_packet->header.opcode, packet, (mpaf_event_params_t*)event_packet->params );
        }

        /* Packet isn't destroyed because ownership has been transferred to the application */
    }
    else
    {
        /* packet fields update isn't required here as MPAF common packet is identical with MPAF data packet */
        /* Call data packet handler */
        mpaf_rx_data_packet_callback( common_packet->header.header.endpoint, packet, common_packet->common_data_start, packet->data_end - packet->data_start );
        /* Packet isn't destroyed because ownership has been transferred to the application */
    }

    return WICED_BT_SUCCESS;
}
