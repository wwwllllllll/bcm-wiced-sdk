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
 * UDP Transmit Application
 *
 * This application snippet demonstrates how to send a UDP packet
 * to a network client (and optionally receive a UDP response).
 *
 * Features demonstrated
 *  - Wi-Fi softAP mode
 *  - DHCP server
 *  - UDP transmit
 *
 * Application Instructions
 *   1. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. Ensure Python 2.7.x (*NOT* 3.x) is installed on your computer
 *   3. Connect your computer using Wi-Fi to "WICED UDP Transmit App"
 *        - SoftAP credentials are defined in wifi_config_dct.h
 *   4. Open a command shell
 *   5. Run the python UDP echo server as follows from the udp_transmit dir
 *      c:\<WICED-SDK>\Apps\snip\udp_transmit> c:\path\to\Python27\python.exe udp_echo_server.py
 *        - Ensure your firewall allows UDP for Python on port 50007
 *
 *   The WICED application starts a softAP, and regularly sends a broadcast
 *   UDP packet containing a sequence number. The WICED app prints the
 *   sequence number of the transmitted packet to the UART.
 *
 *   The computer running the UDP echo server receives the
 *   packet and echoes it back to the WICED application. If the
 *   #define GET_UDP_RESPONSE is enabled the WICED app prints the
 *   sequence number of any received packet to the UART.
 *
 *   When the Wi-Fi client (computer) joins the WICED softAP,
 *   it receives an IP address such as 192.168.0.2. To force
 *   the app to send UDP packets directly to the computer (rather than
 *   to a broadcast address), comment out the #define UDP_TARGET_IS_BROADCAST
 *   and change the target IP address to the IP address of your computer.
 *
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define UDP_MAX_DATA_LENGTH         30
#define UDP_RX_TIMEOUT              1
#define UDP_TX_INTERVAL             1
#define UDP_RX_INTERVAL             1
#define UDP_TARGET_PORT             50007
#define UDP_TARGET_IS_BROADCAST
#define GET_UDP_RESPONSE

#ifdef UDP_TARGET_IS_BROADCAST
#define UDP_TARGET_IP MAKE_IPV4_ADDRESS(192,168,0,255)
#else
#define UDP_TARGET_IP MAKE_IPV4_ADDRESS(192,168,0,2)
#endif

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

static wiced_result_t tx_udp_packet();
static wiced_result_t rx_udp_packet();

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255,255,255,  0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
};

static wiced_udp_socket_t  udp_socket;
static wiced_timed_event_t udp_tx_event;

static uint32_t tx_count   = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start(void)
{
    /* Initialise the device and WICED framework */
    wiced_init( );

    /* Bring up the softAP and network interface */
    wiced_network_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &device_init_ip_settings );

    /* Create UDP socket */
    if ( wiced_udp_create_socket( &udp_socket, UDP_TARGET_PORT, WICED_AP_INTERFACE ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("UDP socket creation failed\n") );
    }

    /* Register a function to send UDP packets */
    wiced_rtos_register_timed_event( &udp_tx_event, WICED_NETWORKING_WORKER_THREAD, &tx_udp_packet, UDP_TX_INTERVAL * SECONDS, 0 );

    WPRINT_APP_INFO( ("Sending a UDP packet every %d seconds ...\n", UDP_TX_INTERVAL) );

#ifdef GET_UDP_RESPONSE
    while ( 1 )
    {
        /* Try to receive a UDP response */
        rx_udp_packet( NEVER_TIMEOUT );
    }
#endif
}


/*
 * Sends a UDP packet
 */
wiced_result_t tx_udp_packet()
{
    wiced_packet_t*          packet;
    char*                    udp_data;
    uint16_t                 available_data_length;
    const wiced_ip_address_t INITIALISER_IPV4_ADDRESS( target_ip_addr, UDP_TARGET_IP );

    /* Create the UDP packet */
    if ( wiced_packet_create_udp( &udp_socket, UDP_MAX_DATA_LENGTH, &packet, (uint8_t**) &udp_data, &available_data_length ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("UDP tx packet creation failed\n") );
        return WICED_ERROR;
    }

    /* Write packet number into the UDP packet data */
    sprintf( udp_data, "%d", (int) tx_count++ );

    /* Set the end of the data portion */
    wiced_packet_set_data_end( packet, (uint8_t*) udp_data + UDP_MAX_DATA_LENGTH );

    /* Send the UDP packet */
    if ( wiced_udp_send( &udp_socket, &target_ip_addr, UDP_TARGET_PORT, packet ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("UDP packet send failed\n") );
        wiced_packet_delete( packet ); /* Delete packet, since the send failed */
        return WICED_ERROR;
    }

    /*
     * NOTE : It is not necessary to delete the packet created above, the packet
     *        will be automatically deleted *AFTER* it has been successfully sent
     */

    WPRINT_APP_INFO( ("sent: %d\n", (int)tx_count) );

    return WICED_SUCCESS;
}


/*
 * Attempts to receive a UDP packet
 */
wiced_result_t rx_udp_packet(uint32_t timeout)
{
    wiced_packet_t* packet;
    char*           udp_data;
    uint16_t        data_length;
    uint16_t        available_data_length;

    /* Wait for UDP packet */
    wiced_result_t result = wiced_udp_receive( &udp_socket, &packet, timeout );

    if ( ( result == WICED_ERROR ) || ( result == WICED_TIMEOUT ) )
    {
        return result;
    }

    wiced_packet_get_data( packet, 0, (uint8_t**) &udp_data, &data_length, &available_data_length );

    /* Null terminate the received string */
    udp_data[ data_length ] = '\x0';

    WPRINT_APP_INFO( ("%s\n\n", udp_data) );

    /* Delete packet as it is no longer needed */
    wiced_packet_delete( packet );

    return WICED_SUCCESS;
}
