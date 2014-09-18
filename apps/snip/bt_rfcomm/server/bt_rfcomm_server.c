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
 * Bluetooth RFCOMM Server Application
 *
 * Features Demonstrated
 *   - Bluetooth RFCOMM API acting as a server
 *
 * This application demonstrates how to implement an RFCOMM
 * server using the Bluetooth RFCOMM API. The application
 * works in conjunction with the Bluetooth RFCOMM Client
 * application.
 *
 * The application only works with a WICED eval board that
 * supports Bluetooth such as BCM9WCDPLUS114
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   The app initialises the Bluetooth RFCOMM protocol and listens for
 *   an incoming connection. If a connection is established, the app
 *   waits for incoming packets until the connection is disconnected
 *   by the client.
 *
 *   This app DOES NOT use Wi-Fi.
 *
 */

#include "wiced.h"
#include "wiced_bt.h"
#include "wiced_bt_rfcomm.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* RFCOMM Server to connect to */
#define SERVER_NAME "WICED+ Server"

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

static void socket_event_callback( wiced_bt_rfcomm_socket_t* socket, wiced_bt_rfcomm_event_t event, void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static volatile wiced_bool_t    connection_established = WICED_FALSE;
static volatile wiced_bool_t    connection_error       = WICED_FALSE;
static wiced_semaphore_t        connection_semaphore;
static wiced_bt_rfcomm_socket_t server_socket;

/* UUID is in little endian */
static const wiced_bt_uuid_t service_uuid =
{
    .size  = UUID_128BIT,
    .value =
    {
        .value_128_bit = { 0xCAFF, 0xDECA, 0xDEAF, 0xDECA, 0xFADE, 0xDECA, 0x0000, 0x0000 },
    },
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    /* Initialise WICED */
    wiced_init( );

    /* Initialise semaphore to indicate that connection with a client is established */
    wiced_rtos_init_semaphore( &connection_semaphore );

    /* Initialise WICED Bluetooth device */
    wiced_bt_init( WICED_BT_MPAF_MODE, "WICED+ Server" );

    /* Initialise Bluetooth RFCOMM protocol */
    wiced_bt_rfcomm_init( );

    /* Create an RFCOMM socket */
    wiced_bt_rfcomm_init_socket( &server_socket, socket_event_callback, NULL );

    while ( 1 )
    {
        /* Place RFCOMM into listening for incoming connection from a RFCOMM client */
        if ( wiced_bt_rfcomm_listen( &server_socket, WICED_TRUE, SERVER_NAME, &service_uuid ) == WICED_SUCCESS )
        {
            WPRINT_APP_INFO( ( "Waiting for incoming connection ...\n" ) );

            /* Wait in semaphore until connection is complete.
             * socket_event_callback will set the semaphore once the RFCOMM connection is established
             */
            wiced_rtos_get_semaphore( &connection_semaphore, WICED_WAIT_FOREVER );

            if ( connection_error == WICED_FALSE )
            {
                WPRINT_APP_INFO( ( "Connected. Start receiving packets\n" ) );

                /* Continue receiving data from the client until connection is disconnected */
                while ( connection_established == WICED_TRUE )
                {
                    wiced_bt_packet_t* packet;
                    uint8_t*           data;
                    uint32_t           data_size;

                    if ( wiced_bt_rfcomm_receive_packet( &server_socket, &packet, &data, &data_size, WICED_NEVER_TIMEOUT ) == WICED_SUCCESS )
                    {
                        /* Output string content received from RFCOMM client */
                        WPRINT_APP_INFO( ("Packet received: %s\n", data ) );

                        /* Delete packet */
                        wiced_bt_packet_delete( packet );
                    }
                }

                WPRINT_APP_INFO( ( "Disconnected by client\n" ) );
            }
            else
            {
                WPRINT_APP_INFO( ( "Connection error!\n" ) );
            }

            /* Reset flags */
            connection_error       = WICED_FALSE;
            connection_established = WICED_FALSE;
        }
        else
        {
            WPRINT_APP_INFO( ( "Error listening for incoming connection. Retrying ...\n" ) );
        }
    }
}

static void socket_event_callback( wiced_bt_rfcomm_socket_t* socket, wiced_bt_rfcomm_event_t event, void* arg )
{
    switch ( event )
    {
        case RFCOMM_EVENT_CONNECTION_ERROR:
        {
            connection_error       = WICED_TRUE;
            connection_established = WICED_FALSE;

            /* Notify application thread waiting on the semaphore */
            wiced_rtos_set_semaphore( &connection_semaphore );
            break;
        }
        case RFCOMM_EVENT_CONNECTION_ACCEPTED:
        {
            connection_error       = WICED_FALSE;
            connection_established = WICED_TRUE;

            /* Notify application thread waiting on the semaphore */
            wiced_rtos_set_semaphore( &connection_semaphore );
            break;
        }
        case RFCOMM_EVENT_CONNECTION_DISCONNECTED:
        {
            connection_established = WICED_FALSE;
            break;
        }
        case RFCOMM_EVENT_INCOMING_PACKET_READY:
        {
            /* Unused in this application */
            break;
        }
        case RFCOMM_EVENT_CLIENT_CONNECTED:
        {
            /* Unused in this application */
            break;
        }
        default:
        {
            break;
        }
    }
}
