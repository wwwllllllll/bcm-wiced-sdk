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
 * Canned UDP Send App
 *
 * Features demonstrated
 *  - Direct WWD driver access (bypasses WICED API)
 *  - No RTOS / No Network Stack
 *
 * This application demonstrates how to use the Broadcom Wi-Fi device
 * to send pre-constructed UDP packets without the use of an RTOS or
 * Network Stack.
 *
 * The application is designed to have a minimal memory footprint
 *
 * Application Instructions
 *   1. Modify the AP_SSID/AP_PASS directives in the application code below
 *      to match your access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 *   The app connects to the AP and sends a UDP packet once per second to
 *   the IP address specified by the PKT_TARGET_IP address
 *
 * *** IMPORTANT NOTE ***
 *   In release builds all UART printing is TURNED OFF to remove
 *   printf and malloc dependency. This reduces memory usage
 *   dramatically!!
 *
 */

#include "platform/wwd_platform_interface.h"
#include "wwd_management.h"
#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_interface.h"
#include "network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_wifi.h"
#include "wwd_poll.h"
#include <string.h>  /* for NULL */
#include "NoOS_canned_send.h"
#include "wwd_debug.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define AP_SSID             "YOUR_AP_SSID"
#define AP_PASS             "YOUR_AP_PASSPHRASE"
#define AP_SEC              WICED_SECURITY_WPA2_MIXED_PSK
#define COUNTRY             WICED_COUNTRY_AUSTRALIA
#define JOIN_TIMEOUT        (10000)                                  /* timeout for joining the wireless network in milliseconds  = 10 seconds */
#define IP_ADDR             MAKE_IPV4_ADDRESS( 192, 168, 1,  95 )
#define LOCAL_UDP_PORT      (50007)
#define PKT_TARGET_IP       MAKE_IPV4_ADDRESS( 255, 255, 255, 255 )  /* For unicast, change address e.g. MAKE_IPV4_ADDRESS( 192, 168, 1,   5 ) */
#define PKT_TARGET_UDP_PORT (50007)
#define MAX_PAYLOAD         (20) /* bytes */
#define PAYLOAD             "Hello!"

/* In release builds all UART printing is TURNED OFF to remove printf and malloc dependency which reduces memory usage dramatically */
#ifndef DEBUG
#undef  WPRINT_APP_INFO
#define WPRINT_APP_INFO(args)
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
/** @cond */
static void  resolve_dest_mac   ( uint32_t dest_ip_addr, wiced_mac_t * MAC_buffer );
static void  send_canned_packet ( char* pkt, uint16_t payload_len );
static char* setup_canned_packet( char *        pkt,
                                  uint16_t      pkt_len,
                                  uint32_t      my_ip_addr,
                                  wiced_mac_t * my_MAC,
                                  uint32_t      dest_ip_addr,
                                  wiced_mac_t * dest_MAC,
                                  uint16_t      src_udp_port,
                                  uint16_t      dest_udp_port );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_mac_t   my_mac             = { {   0,   0,   0,   0,   0,   0} };
static wiced_mac_t   broadcast_mac      = { {0xff,0xff,0xff,0xff,0xff,0xff} };
static uint8_t       arp_complete       = 0;
static wiced_mac_t*  arp_mac_buffer_ptr = 0;
static uint32_t      arp_dest_ip_addr   = 0;
static uint32_t      my_ip_addr         = IP_ADDR;
static char          pkt_buffer[ ((MAX( PACKET_SIZE, MIN_IOCTL_BUFFER_SIZE )+63)&(~63))+4 ];
static const wiced_ssid_t ap_ssid =
{
    .length = sizeof(AP_SSID)-1,
    .value  = AP_SSID,
};

/** @endcond */


/******************************************************
 *               Function Definitions
 ******************************************************/

/**
 * Main function of canned UDP packet send application
 *
 * This main function initializes Wiced, joins a network,
 * requests the destination address via an ARP, then repeatedly
 * sends a UDP packet.
 */

int main( void )
{
    wiced_mac_t dest_mac = { {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };
    char* payload_ptr = 0;
    wwd_result_t result;

    NoOS_setup_timing( );

    WPRINT_APP_INFO(("\nPlatform " PLATFORM " initialised\n"));

    /* Initialise Wiced */
    WPRINT_APP_INFO(("Starting Wiced v" WICED_VERSION "\n"));
    nons_buffer_init_t pkt_buff_init = { pkt_buffer, sizeof( pkt_buffer ) };

    while ( WWD_SUCCESS != ( result = wwd_management_init( COUNTRY, &pkt_buff_init ) ) )
    {
        WPRINT_APP_INFO(("Error %d while starting WICED!\n", result));
    }

    /* Get MAC address - this needs to be done before joining a network, so that */
    /* we can check the address of any incoming packets against our MAC */
    wwd_wifi_get_mac_address( &my_mac, WWD_STA_INTERFACE );

    /* Attempt to join the Wi-Fi network */
    WPRINT_APP_INFO(("Joining : " AP_SSID "\n"));
    while ( wwd_wifi_join( &ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, NULL ) != WWD_SUCCESS )
    {
        WPRINT_APP_INFO(("Failed to join  : " AP_SSID "   .. retrying\n"));
    }
    WPRINT_APP_INFO(("Successfully joined : " AP_SSID "\n"));

    if ( PKT_TARGET_IP != 0xFFFFFFFF )  /* Check if the target has a broadcast address */
    {
        /* Send an ARP request to resolve the destination IP address into a MAC address */
        resolve_dest_mac( PKT_TARGET_IP, &dest_mac );
    }

    /* Loop forever, repeatedly sending the UDP packet */
    while ( 1 )
    {
        int i;

        /* Setup the packet buffer with the canned packet contents. */
        payload_ptr = setup_canned_packet( pkt_buffer, sizeof( pkt_buffer ), my_ip_addr, &my_mac, PKT_TARGET_IP, &dest_mac, LOCAL_UDP_PORT, PKT_TARGET_UDP_PORT );

        /* Copy the payload into the packet */
        memcpy( payload_ptr, PAYLOAD, sizeof( PAYLOAD ) - 1 );

        WPRINT_APP_INFO(("Sending Hello!\n"));
        send_canned_packet( pkt_buffer, sizeof( PAYLOAD ) - 1 );

        for ( i = 0; i < 10; i++ )
        {
            host_rtos_delay_milliseconds( 100 );
            /* Poll for packets to receive (which will be dropped) - 802.11 device will run out of memory if packets are not read out of it */
            while ( wwd_thread_poll_all( ) != 0 )
            { }
        }
    }
}





/**
 * Resolves an IP address using ARP
 *
 * Creates and sends a ARP request packet, then waits
 * for the response.
 *
 * @Note : Currently there is no provision for timeouts/retries
 *
 * @param dest_ip_addr : the destination IP address in network endian format (little endian)
 * @param MAC_buffer   : pointer to a MAC address structure which will receive the resolved
 *                       MAC address of the destination IP
 */
static void resolve_dest_mac( uint32_t dest_ip_addr, wiced_mac_t * MAC_buffer )
{
    /* Template for ARP request packet */
    static const arp_packet_t arp_template =
    {
        .ethernet_header =
        {
            { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } },
            { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
            SWAP16( ETHER_TYPE_ARP )
        },
        .arp_message =
        {
            SWAP16( ARP_HARDWARE_TYPE_ETHERNET ),
            SWAP16( ARP_PROTOCOL_TYPE_IPV4 ),
            6,
            4,
            SWAP16( ARP_OPERATION_REQUEST ),
            { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
            0x00000000,
            { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
            0x00000000,
        }
    };

    do
    {
        int pollcount = 0;

        WPRINT_APP_INFO(("Sending ARP request\n"));
        arp_packet_t* pkt = (arp_packet_t*)pkt_buffer;
        memcpy(pkt, &arp_template, sizeof(arp_packet_t));

        /* Fill missing fields into ARP request template */
        memcpy( &pkt->ethernet_header.source,              &my_mac, 6 );
        memcpy( &pkt->arp_message.sender_hardware_address, &my_mac, 6 );
        pkt->arp_message.sender_protocol_address = my_ip_addr;
        pkt->arp_message.target_protocol_address = dest_ip_addr;

        /* Set the current size of the packet */
        host_buffer_set_size( (wiced_buffer_t)pkt,  sizeof(arp_packet_t) - sizeof(wwd_buffer_header_t) );

        /* Queue the packet in the transmit queue */
        wwd_network_send_ethernet_data( (wiced_buffer_t) &pkt->ethernet_header, WWD_STA_INTERFACE );

        /* Setup global variables to allow the ARP response processing to compare against */
        arp_mac_buffer_ptr = MAC_buffer;
        arp_dest_ip_addr = dest_ip_addr;

        /* Send and receive packets until the ARP transaction has completed */
        while ( ( arp_complete != 1 ) && ( pollcount < 10000 ) )
        {
            wwd_thread_poll_all( );
            pollcount++;
        }
        pollcount = 0;
    } while ( arp_complete != 1 );

    /* Reset global variable so incoming ARP packets won't be processed */
    arp_mac_buffer_ptr = 0;

}


/**
 * Creates a canned UDP packet
 *
 * This function creates an empty UDP packet in the given packet buffer
 *
 * @param pkt           : the packet buffer in which the UDP packet will be created
 * @param pkt_len       : the maximum length of pkt (the packet buffer)
 * @param my_ip_addr    : the local IP address in network format (little endian)
 * @param my_MAC        : the local MAC address
 * @param dest_ip_addr  : the destination IP address in network format (little endian)
 * @param dest_MAC      : the destination MAC address
 * @param src_udp_port  : the local UDP port to use
 * @param dest_udp_port : the remote UDP port at the destination
 *
 * @return A pointer to the location where the UDP payload should be copied.
 */
static char* setup_canned_packet( char *        pkt,
                                  uint16_t      pkt_len,
                                  uint32_t      my_ip_addr,
                                  wiced_mac_t *   my_MAC,
                                  uint32_t      dest_ip_addr,
                                  wiced_mac_t *   dest_MAC,
                                  uint16_t      src_udp_port,
                                  uint16_t      dest_udp_port )
{
    udp_packet_t *      udp_pkt = (udp_packet_t*) pkt;
    ethernet_header_t * eth     = &udp_pkt->ethernet_header;
    ipv4_header_t *     iphdr   = &udp_pkt->ip_header;
    udp_header_t *      udphdr  = &udp_pkt->udp_header;

    /* Clear packet */
    memset( pkt, 0, pkt_len );

    /* Setup Ethernet header */
    memcpy( &eth->source, my_MAC, 6 );
    memcpy( &eth->destination, dest_MAC, 6 );
    eth->ether_type = SWAP16( ETHER_TYPE_IPv4 );

    /* Setup IP header
     * Cannot setup total_length or checksum at this stage
     * The following fields are left zero : differentiated_services, identification
     */
    iphdr->header_length         = 5; /* IP header is 5 x 32bits long */
    iphdr->version               = 4; /* IPv4 */
    iphdr->flags_fragment_offset = 2 << 5;
    iphdr->time_to_live          = 128;
    iphdr->protocol              = 0x11; /* UDP protocol */
    iphdr->source_address        = my_ip_addr;
    iphdr->destination_address   = dest_ip_addr;

    /* Setup UDP header
     * Cannot setup udp_lengthat this stage
     * Checksum field left zero to disable UDP checksum
     */
    udphdr->source_port = SWAP16( src_udp_port );
    udphdr->dest_port   = SWAP16( dest_udp_port );

    /* Return the start address of the UDP payload, so the caller can copy their payload in. */
    return udp_pkt->data;
}


/**
 * Send the canned UDP packet
 *
 * Adds the packet length to the IP and UDP headers, then calculates the IP header
 * checksum.
 *
 * @param pkt         : the packet buffer to be sent
 * @param payload_len : the length of the UDP payload data in bytes
 *
 */

static void send_canned_packet( char* pkt, uint16_t payload_len )
{
    udp_packet_t *  udp_pkt       = (udp_packet_t*) pkt;
    ipv4_header_t * iphdr         = &udp_pkt->ip_header;
    udp_header_t *  udphdr        = &udp_pkt->udp_header;
    uint32_t     checksum_temp = 0;
    uint8_t      i;

    /* Add packet length to IP and UDP headers */
    iphdr->total_length = SWAP16( sizeof(ipv4_header_t) + sizeof(udp_header_t) + payload_len );
    udphdr->udp_length  = SWAP16(                         sizeof(udp_header_t) + payload_len );

    /* Calculate the IP header checksum */
    iphdr->header_checksum = 0;
    for ( i = 0; i < ( sizeof(ipv4_header_t) ); i += 2 )
    {
        checksum_temp += ( ( (unsigned char*) iphdr )[i] << 8 ) | ( ( (unsigned char*) iphdr )[i + 1] );
    }
    while ( checksum_temp >> 16 )
    {
        checksum_temp = ( checksum_temp & 0xFFFF ) + ( checksum_temp >> 16 );
    }
    iphdr->header_checksum = SWAP16( (uint16_t)(~checksum_temp) );


    /* Set the packet size */
    host_buffer_set_size( pkt, sizeof(ethernet_header_t) + sizeof(ipv4_header_t) + sizeof(udp_header_t) + payload_len );

    /* Send the packet */
    wwd_network_send_ethernet_data( (wiced_buffer_t) &udp_pkt->ethernet_header, WWD_STA_INTERFACE );
    wwd_thread_send_one_packet( ); /* Send packet only - do not poll all, since that would cause receives which will overwrite packet buffer contents */
}


/**
 * Processes incoming received packets
 *
 * This function is called as a callback by Wiced when a packet has been
 * received.
 * For this example application, the only kind of packet of interest is
 * an ARP response from the destination IP.
 *
 * @param p : packet buffer containing newly received data packet
 * @param interface : The interface (AP or STA) on which the packet was received.
 */
void host_network_process_ethernet_data( /*@only@*/ wiced_buffer_t p, wwd_interface_t interface )
{
    ethernet_header_t * ether_header = (ethernet_header_t *) host_buffer_get_current_piece_data_pointer( p );

    if ( ( interface != WWD_STA_INTERFACE ) ||                                   /* Check that packet came from STA interface */
         ( ether_header->ether_type != SWAP16( ETHER_TYPE_ARP ) ) ||             /* Check ethertype first as it is less costly than memcmp - Only ARP packets need to be received - all other packets are silently ignored */
         ( ( 0 != memcmp( &ether_header->destination, &my_mac, 6 ) ) &&          /* Check if the destination MAC matches ours, or the broadcast value */
           ( 0 != memcmp( &ether_header->destination, &broadcast_mac, 6 ) ) ) )

    {
        host_buffer_release( p, WWD_NETWORK_RX );
        return;
    }

    arp_message_t * arp = (arp_message_t *) &ether_header[1];

    /* Only process ARP replys - Ignore ARP requests, since we do not want anyone to send us any data */
    if ( arp->operation != SWAP16( ARP_OPERATION_REPLY ) )
    {
        host_buffer_release( p, WWD_NETWORK_RX );
        return;
    }

    /* Check if the ARP response is from our destination IP address */
    if ( arp->sender_protocol_address != arp_dest_ip_addr )
    {
        /* Not a response containing the MAC of our destination IP */
        host_buffer_release( p, WWD_NETWORK_RX );
        return;
    }

    /* We now have a MAC for our destination */
    memcpy( arp_mac_buffer_ptr, &arp->sender_hardware_address, 6 );

    arp_complete = 1;
}
