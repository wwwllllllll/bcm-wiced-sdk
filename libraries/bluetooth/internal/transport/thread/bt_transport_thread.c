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
#include "bt_packet_internal.h"
#include "bt_transport_driver.h"
#include "bt_transport_thread.h"

/******************************************************
 *                      Macros
 ******************************************************/

#ifdef DEBUG
#define DUMP_PACKET( direction, start, end )                        \
do                                                                  \
{                                                                   \
    if ( bt_transport_enable_packet_dump == WICED_TRUE )            \
    {                                                               \
        uint8_t* current = start;                                   \
        if ( direction == 0 )                                       \
            WPRINT_LIB_INFO(( "\n[BT Transport] TX -----\n" ));     \
        else                                                        \
            WPRINT_LIB_INFO(( "\n[BT Transport] RX -----\n" ));     \
        while ( current != end )                                    \
        {                                                           \
            WPRINT_LIB_INFO(( "%.2X ", (int)*current++ ));          \
            if ( ( current - start ) % 16 == 0 )                    \
            {                                                       \
                WPRINT_LIB_INFO(( "\n" ));                          \
            }                                                       \
        }                                                           \
        WPRINT_LIB_INFO(( "\n-----------------------\n" ));         \
    }                                                               \
}                                                                   \
while (0)
#else
#define DUMP_PACKET( direction, start, end )
#endif

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

static wiced_result_t bt_transport_thread_send_packet_handler   ( void* arg );
static wiced_result_t bt_transport_thread_receive_packet_handler( void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_worker_thread_t                         bt_transport_thread;
static wiced_bool_t                                  bt_transport_thread_initialised      = WICED_FALSE;
static bt_transport_thread_received_packet_handler_t bt_transport_received_packet_handler = NULL;
#ifdef DEBUG
static wiced_bool_t                                  bt_transport_enable_packet_dump      = WICED_FALSE;
#endif

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_transport_thread_init( bt_transport_thread_received_packet_handler_t handler )
{
    wiced_result_t result;

    if ( bt_transport_thread_initialised == WICED_TRUE )
    {
        return WICED_BT_SUCCESS;
    }

    bt_transport_received_packet_handler = handler;

    /* Create MPAF worker thread */
    result = wiced_rtos_create_worker_thread( &bt_transport_thread, BT_TRANSPORT_THREAD_PRIORITY, BT_TRANSPORT_STACK_SIZE, BT_TRANSPORT_QUEUE_SIZE );
    if ( result == WICED_SUCCESS )
    {
        return WICED_BT_SUCCESS;
    }
    else
    {
        wiced_assert( "Error creating BT transport thread\n", 0!=0 );
        bt_transport_received_packet_handler = NULL;
        return result;
    }
}

wiced_result_t bt_transport_thread_deinit( void )
{
    if ( bt_transport_thread_initialised == WICED_FALSE )
    {
        return WICED_BT_SUCCESS;
    }

    /* Deliberately not checking for result. Deinit always assumed successful */
    wiced_rtos_delete_worker_thread( &bt_transport_thread );
    bt_transport_received_packet_handler = NULL;
    bt_transport_thread_initialised      = WICED_FALSE;

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_transport_thread_send_packet( bt_packet_t* packet )
{
    if ( packet == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( wiced_rtos_is_current_thread( &bt_transport_thread.thread ) == WICED_SUCCESS )
    {
        return bt_transport_thread_send_packet_handler( (void*)packet );
    }
    else
    {
        return wiced_rtos_send_asynchronous_event( &bt_transport_thread, bt_transport_thread_send_packet_handler, (void*)packet );
    }
}

wiced_result_t bt_transport_thread_notify_packet_received( void )
{
    return wiced_rtos_send_asynchronous_event( &bt_transport_thread, bt_transport_thread_receive_packet_handler, NULL );
}

wiced_result_t bt_transport_thread_execute_callback( bt_transport_thread_callback_handler_t callback_handler, void* arg )
{
    return wiced_rtos_send_asynchronous_event( &bt_transport_thread, callback_handler, arg );
}

wiced_result_t bt_transport_thread_enable_packet_dump( void )
{
#ifdef DEBUG
    bt_transport_enable_packet_dump = WICED_TRUE;
    return WICED_BT_SUCCESS;
#else
    return WICED_BT_UNSUPPORTED;
#endif
}

wiced_result_t bt_transport_thread_disable_packet_dump( void )
{
#ifdef DEBUG
    bt_transport_enable_packet_dump = WICED_FALSE;
    return WICED_BT_SUCCESS;
#else
    return WICED_BT_UNSUPPORTED;
#endif
}

static wiced_result_t bt_transport_thread_send_packet_handler( void* arg )
{
    bt_packet_t* packet = (bt_packet_t*) arg;

    DUMP_PACKET( 0, packet->packet_start, packet->data_end );

    return bt_transport_driver_send_packet( packet );
}

static wiced_result_t bt_transport_thread_receive_packet_handler( void* arg )
{
    bt_packet_t*   packet = NULL;
    wiced_result_t result = bt_transport_driver_receive_packet( &packet );

    wiced_assert( "bt_transport_received_packet_handler isn't set", bt_transport_received_packet_handler != NULL );

    UNUSED_PARAMETER( arg );

    if ( result == WICED_BT_SUCCESS && packet != NULL )
    {
        DUMP_PACKET( 1, packet->packet_start, packet->data_end );
        result = bt_transport_received_packet_handler( packet );
    }

    return result;
}
