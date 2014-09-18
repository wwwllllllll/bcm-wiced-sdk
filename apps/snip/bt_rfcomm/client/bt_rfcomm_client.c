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
 * Bluetooth RFCOMM Client Application
 *
 * Features Demonstrated
 *   - Bluetooth RFCOMM API acting as a client
 *
 * This application demonstrates how to implement an RFCOMM
 * client using the Bluetooth RFCOMM API. The application
 * works in conjunction with the Bluetooth RFCOMM Server
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
 *   The app initialises the Bluetooth RFCOMM protocol and
 *   attempts to initiate a Bluetooth connection with the specified
 *   RFCOMM server. If a connection is established, the app
 *   sends a test packet and disconnects.
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

/* Interval between connections in milliseconds */
#define CONNECTION_INTERVAL_MS 10000

/* Change this to your unique friendly name */
#define CLIENT_FRIENDLY_NAME   "WICED+ Client"

/* Packet content */
#define PACKET_CONTENT_STR     "Hello World from "

/* RFCOMM Server to connect to */
#define SERVER_NAME            "WICED+ Server"

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

static wiced_bool_t             connection_established = WICED_FALSE;
static wiced_semaphore_t        connection_semaphore;
static wiced_bt_rfcomm_socket_t client_socket;

/* UUID is in little endian. This has to match with the RFCOMM server UUID */
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
    wiced_bt_init( WICED_BT_MPAF_MODE, "WICED+ Client" );

    /* Initialise Bluetooth RFCOMM protocol */
    wiced_bt_rfcomm_init( );

    memset( &client_socket, 0, sizeof( client_socket ) );

    /* Create an RFCOMM socket */
    wiced_bt_rfcomm_init_socket( &client_socket, socket_event_callback, NULL );

    while ( 1 )
    {
        WPRINT_APP_INFO( ( "Connecting to server ...\n" ) );

        /* Attempt to connect with a WICED+ server */
        if ( wiced_bt_rfcomm_connect( &client_socket, SERVER_NAME, &service_uuid ) == WICED_SUCCESS )
        {
            wiced_bt_packet_t* packet;
            uint8_t*           data;

            /* Wait in semaphore until connection is complete.
             * socket_event_callback will set the semaphore once the RFCOMM connection is established
             */
            wiced_rtos_get_semaphore( &connection_semaphore, WICED_WAIT_FOREVER );

            if ( connection_established == WICED_TRUE )
            {
                wiced_bt_device_address_t address;
                uint32_t data_length;
                char* data_ptr;

                wiced_bt_device_get_address( &address );

                WPRINT_APP_INFO( ( "Client connected. Start sending packets\n" ) );

                /* Create packet */
                wiced_bt_rfcomm_create_packet( &packet, strlen( PACKET_CONTENT_STR ) + sizeof( address ) + sizeof(char), &data );

                data_ptr = (char*)data;

                /* Append client's device address to the packet */
                data_length = sprintf( data_ptr,
                                       "%s%02X:%02X:%02X:%02X:%02X:%02X",
                                       PACKET_CONTENT_STR,
                                       address.address[5],
                                       address.address[4],
                                       address.address[3],
                                       address.address[2],
                                       address.address[1],
                                       address.address[0] );

                /* Ensure string is nul-terminated */
                data_ptr   += data_length;
                *data_ptr++ = '\0';

                /* Mark the end of the packet data section */
                wiced_bt_packet_set_data_end( packet, (uint8_t*)data_ptr );

                /* Send packet */
                if ( wiced_bt_rfcomm_send_packet( &client_socket, packet ) == WICED_SUCCESS )
                {
                    WPRINT_APP_INFO( ( "Packet sent\n" ) );
                }
                else
                {
                    WPRINT_APP_INFO( ( "Error sending packet!\n" ) );
                }

                /* Disconnect and clean up socket */
                wiced_bt_rfcomm_disconnect( &client_socket );

                WPRINT_APP_INFO( ( "Client disconnected\n" ) );

            }
            else
            {
                WPRINT_APP_INFO( ( "Connection error!\n" ) );
            }

            WPRINT_APP_INFO( ( "Waiting %d seconds before connecting ...\n", CONNECTION_INTERVAL_MS / SECONDS ) );

            wiced_rtos_delay_milliseconds( CONNECTION_INTERVAL_MS );
        }
    }
}

static void socket_event_callback( wiced_bt_rfcomm_socket_t* socket, wiced_bt_rfcomm_event_t event, void* arg )
{
    switch ( event )
    {
        case RFCOMM_EVENT_CLIENT_CONNECTED:
        {
            connection_established = WICED_TRUE;

            /* Notify application thread waiting on the semaphore */
            wiced_rtos_set_semaphore( &connection_semaphore );
            break;
        }
        case RFCOMM_EVENT_CONNECTION_ERROR:
        {
            connection_established = WICED_FALSE;

            /* Notify application thread waiting on the semaphore */
            wiced_rtos_set_semaphore( &connection_semaphore );
            break;
        }
        case RFCOMM_EVENT_CONNECTION_ACCEPTED:
        {
            /* Unused in this application */
            break;
        }
        case RFCOMM_EVENT_CONNECTION_DISCONNECTED:
        {
            /* Unused in this application */
            break;
        }
        case RFCOMM_EVENT_INCOMING_PACKET_READY:
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
