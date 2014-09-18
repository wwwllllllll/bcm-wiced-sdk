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
 * Network Keep Alive Application
 *
 * This application demonstrates how to make the WLAN chip automatically
 * send an 802.11 Null Function data frame and/or arbitrary IP packet at
 * a regular interval.
 *
 * This feature enables the WLAN chip to automatically maintain connectivity
 * with the Wi-Fi AP and/or a remote network application without needing
 * help from an application running on the host MCU.
 *
 * Features demonstrated
 *  - Adding a keep alive packet
 *  - Retrieving an existing keep alive packet configuration
 *  - Disabling keep alive packets
 *
 * Application Instructions
 *   1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *      in the wifi_config_dct.h header file to match your Wi-Fi access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 *   Two keep alive packets are configured for this example
 *     1. A Null Function data frame that is sent at intervals of 1 second
 *     2. An ARP frame that is sent at intervals of 3 seconds
 *
 *   Once the keep alive frames have been added, the app retrieves
 *   and prints the configuration from the WLAN chip. After 30 seconds,
 *   both keep alive packets are disabled and the app halts
 *
 * Keep Alive Usage Notes
 *  - A maximum of 4 keep alive packets can exist concurrently
 *  - Keep alive packet functionality only works with client (STA) mode
 *  - If the keep alive packet length is set to 0, a Null-Function Data
 *    frame is automatically used as the keep alive
 *  - Any IP packet can be sent as a keep alive packet
 *
 * References
 *    http://en.wikipedia.org/wiki/EtherType
 *    http://en.wikipedia.org/wiki/Address_Resolution_Protocol
 *
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Null function Data Frame keep alive parameters */
#define KEEP_ALIVE_ID_NFD          0
#define KEEP_ALIVE_PERIOD_NFD_MSEC 1000

/* ARP Frame keep alive parameters */
#define KEEP_ALIVE_ID_ARP          1
#define KEEP_ALIVE_PERIOD_ARP_MSEC 3000

#define MAX_KEEP_ALIVE_PACKET_SIZE 512

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

void print_keep_alive_info( wiced_keep_alive_packet_t* packet_info );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static char arp_packet[] =
    "\xff\xff\xff\xff\xff\xff"  /*  0 : Ethernet Destination Address : Broadcast for Gratuitous ARP */
    "\x00\x00\x00\x00\x00\x00"  /*  6 : Ethernet Source Address : WLAN MAC address copied in below  */
    "\x08\x06"                  /* 12 : EtherType : ARP                                             */
    "\x00\x06"                  /* 14 : Hardware Type : IEEE 802.11 network device                  */
    "\x08\x00"                  /* 16 : Protocol Type : IPv4                                        */
    "\x06"                      /* 18 : Hardware Address Length                                     */
    "\x04"                      /* 19 : Protocol Address Length                                     */
    "\x00\x01"                  /* 20 : Operation : Request                                         */
    "\x00\x00\x00\x00\x00\x00"  /* 22 : Sender hardware address : WLAN MAC address copied in below  */
    "\x00\x00\x00\x00"          /* 28 : Sender Protocol Address : IP address copied in below        */
    "\xff\xff\xff\xff\xff\xff"  /* 32 : Target Hardware Address : Broadcast for Gratuitous ARP      */
    "\x00\x00\x00\x00";         /* 38 : Target Protocol Address : IP address copied in below        */

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    int i;
    wiced_result_t            status;
    wiced_mac_t               my_mac_address;
    uint32_t                  my_ip_address;
    wiced_ip_address_t        ipv4_address;
    wiced_keep_alive_packet_t keep_alive_packet_info;
    uint8_t                   keep_alive_packet_buffer[MAX_KEEP_ALIVE_PACKET_SIZE];

    /* Initialize the device */
    wiced_init( );

    /* Bring up the network on the STA interface */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Setup the ARP keep alive packet to copy in my MAC & IP address */
    wiced_wifi_get_mac_address( &my_mac_address );
    wiced_ip_get_ipv4_address( WICED_STA_INTERFACE, &ipv4_address );
    my_ip_address = htonl( GET_IPV4_ADDRESS(ipv4_address) );
    memcpy( &arp_packet[ 6 ], &my_mac_address, 6 );
    memcpy( &arp_packet[ 22 ], &my_mac_address, 6 );
    memcpy( &arp_packet[ 28 ], &my_ip_address, 4 );
    memcpy( &arp_packet[ 38 ], &my_ip_address, 4 );

    /* Turn off print buffers, so print output occurs immediately */
    setvbuf( stdout, NULL, _IONBF, 0 );


    /* Setup a Null function data frame keep alive */
    keep_alive_packet_info.keep_alive_id = KEEP_ALIVE_ID_NFD,
    keep_alive_packet_info.period_msec   = KEEP_ALIVE_PERIOD_NFD_MSEC,
    keep_alive_packet_info.packet_length = 0;

    status = wiced_wifi_add_keep_alive( &keep_alive_packet_info );
    switch ( status )
    {
        case WICED_SUCCESS:
        {
            WPRINT_APP_INFO( ( "\r\nAdded:\n") );
            WPRINT_APP_INFO( ( "  - Null Function Data frame with repeat period of %d milliseconds \n", KEEP_ALIVE_PERIOD_NFD_MSEC ) );
            break;
        }
        case WICED_TIMEOUT:
        {
            WPRINT_APP_INFO( ( "Timeout: Adding Null Function Data frame keep alive packet\n" ) );
            break;
        }
        default:
            WPRINT_APP_INFO( ( "Error[%d]: Adding Null Function Data frame keep alive packet\n", status ) );
            break;
    }


    /* Setup an ARP packet keep alive */
    keep_alive_packet_info.keep_alive_id = KEEP_ALIVE_ID_ARP,
    keep_alive_packet_info.period_msec   = KEEP_ALIVE_PERIOD_ARP_MSEC,
    keep_alive_packet_info.packet_length = sizeof(arp_packet)-1;
    keep_alive_packet_info.packet = (uint8_t*)arp_packet;

    status = wiced_wifi_add_keep_alive( &keep_alive_packet_info );
    switch ( status )
    {
        case WICED_SUCCESS:
        {
            WPRINT_APP_INFO( ( "  - ARP packet with repeat period of %d milliseconds\n\n", KEEP_ALIVE_PERIOD_ARP_MSEC ) );
            break;
        }
        case WICED_TIMEOUT:
        {
            WPRINT_APP_INFO( ( "Timeout: Adding ARP packet\n\n" ) );
            break;
        }
        default:
            WPRINT_APP_INFO( ( "Error[%d]: Adding ARP packet\n\n", status ) );
            break;
    }


/* Get Null Function Data Frame keep alive packet info */
keep_alive_packet_info.keep_alive_id = KEEP_ALIVE_ID_NFD;
keep_alive_packet_info.packet_length = MAX_KEEP_ALIVE_PACKET_SIZE;
keep_alive_packet_info.packet        = &keep_alive_packet_buffer[0];

    status = wiced_wifi_get_keep_alive( &keep_alive_packet_info );
    if ( status == WICED_SUCCESS )
    {
        print_keep_alive_info( &keep_alive_packet_info );
    }
    else
    {
        WPRINT_APP_INFO( ( "ERROR[%d]: Get keep alive packet failed for ID:%d\n", status, KEEP_ALIVE_ID_NFD) );
    }


    /* Get ARP keep alive packet info */
    keep_alive_packet_info.keep_alive_id = KEEP_ALIVE_ID_ARP;
    keep_alive_packet_info.packet_length = MAX_KEEP_ALIVE_PACKET_SIZE;
    keep_alive_packet_info.packet        = &keep_alive_packet_buffer[0];

    status = wiced_wifi_get_keep_alive( &keep_alive_packet_info );
    if ( status == WICED_SUCCESS )
    {
        print_keep_alive_info( &keep_alive_packet_info );
    }
    else
    {
        WPRINT_APP_INFO( ( "ERROR[%d]: Get keep alive packet failed for ID:%d\n", status, KEEP_ALIVE_ID_ARP) );
    }


    /* Wait 30 seconds, then disable all keep alive packets */
    WPRINT_APP_INFO( ( "Sending keep alive packets " ) );
    for ( i = 0; i < 30; i++ )
    {
        WPRINT_APP_INFO( ( "." ) );
        wiced_rtos_delay_milliseconds( 1000 );
    }
    WPRINT_APP_INFO( ( " done\n\n" ) );


    /* Disable Null Function Data Frame keep alive packet*/
    status = wiced_wifi_disable_keep_alive( KEEP_ALIVE_ID_NFD );
    if ( status == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Null Function data frame keep alive packet disabled\n" ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "ERROR[%d]: Failed to disable NFD keep alive packet\n", status) );
    }


    /* Disable ARP keep alive packet*/
    status = wiced_wifi_disable_keep_alive( KEEP_ALIVE_ID_ARP );
    if ( status == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "ARP keep alive packet disabled\n" ) );
    }
    else
    {
        WPRINT_APP_INFO( ( "ERROR[%d]: Failed to disable ARP keep alive packet\n", status) );
    }

    WPRINT_APP_INFO( ( "\r\nStop.\n") );
    while ( 1 )
    {
    }
}


void print_keep_alive_info( wiced_keep_alive_packet_t* packet_info )
{
    int i;
    WPRINT_APP_INFO( ( "Keep alive ID: %d\n", packet_info->keep_alive_id ) );
    WPRINT_APP_INFO( ( "Repeat period: %lu ms\n", packet_info->period_msec ) );
    WPRINT_APP_INFO( ( "Packet length: %d bytes\n", packet_info->packet_length ) );
    WPRINT_APP_INFO( ( "Packet bytes : ") );
    for ( i = 1; i <= packet_info->packet_length; i++ )
    {
        WPRINT_APP_INFO( ( "%02X", packet_info->packet[i] ) );
        if ( i % 4 == 0 )
        {
            WPRINT_APP_INFO( ( ":" ) );
        }
    }
    WPRINT_APP_INFO( ( "\n\n" ) );
}
