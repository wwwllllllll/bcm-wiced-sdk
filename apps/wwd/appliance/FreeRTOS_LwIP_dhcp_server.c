/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 * A small DHCP server.
 * Allows multiple clients.
 *
 * Original source obtained on 8th April 2011 from : Public Domain source - Written by Richard Bronson - http://sourceforge.net/projects/sedhcp/
 * Heavily modified
 *
 */


#include "lwip/sockets.h"  /* equivalent of <sys/socket.h> */
#include <string.h>
#include <stdint.h>
#include "appliance.h"
#include "network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define DHCP_STACK_SIZE               (800)

/* BOOTP operations */
#define BOOTP_OP_REQUEST                (1)
#define BOOTP_OP_REPLY                  (2)

/* DHCP commands */
#define DHCPDISCOVER                    (1)
#define DHCPOFFER                       (2)
#define DHCPREQUEST                     (3)
#define DHCPDECLINE                     (4)
#define DHCPACK                         (5)
#define DHCPNAK                         (6)
#define DHCPRELEASE                     (7)
#define DHCPINFORM                      (8)

/* UDP port numbers for DHCP server and client */
#define IPPORT_DHCPS                   (67)
#define IPPORT_DHCPC                   (68)

/* DHCP socket timeout value in milliseconds. Modify this to make thread exiting more responsive */
#define DHCP_SOCKET_TIMEOUT     500



/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/* DHCP data structure */
typedef struct
{
    uint8_t  opcode;                     /* packet opcode type */
    uint8_t  hardware_type;              /* hardware addr type */
    uint8_t  hardware_addr_len;          /* hardware addr length */
    uint8_t  hops;                       /* gateway hops */
    uint32_t transaction_id;             /* transaction ID */
    uint16_t second_elapsed;             /* seconds since boot began */
    uint16_t flags;
    uint8_t  client_ip_addr[4];          /* client IP address */
    uint8_t  your_ip_addr[4];            /* 'your' IP address */
    uint8_t  server_ip_addr[4];          /* server IP address */
    uint8_t  gateway_ip_addr[4];         /* gateway IP address */
    uint8_t  client_hardware_addr[16];   /* client hardware address */
    uint8_t  legacy[192];
    uint8_t  magic[4];
    uint8_t  options[275];               /* options area */
    /* as of RFC2131 it is variable length */
} dhcp_header_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static unsigned char * find_option( dhcp_header_t* request, unsigned char option_num );
static void dhcp_thread( void * thread_input );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static char             new_ip_addr[4]                = { 192, 168, 0, 0 };
static uint16_t         next_available_ip_addr        = ( 1 << 8 ) + 100;
static char             subnet_option_buff[]          = { 1, 4, 255, 255, 0, 0 };
static char             server_ip_addr_option_buff[]  = { 54, 4, 192, 168, 1, 1 };
static char             mtu_option_buff[]             = { 26, 2, WICED_PAYLOAD_MTU>>8, WICED_PAYLOAD_MTU&0xff };
static char             dhcp_offer_option_buff[]      = { 53, 1, DHCPOFFER };
static char             dhcp_ack_option_buff[]        = { 53, 1, DHCPACK };
static char             dhcp_nak_option_buff[]        = { 53, 1, DHCPNAK };
static char             lease_time_option_buff[]      = { 51, 4, 0x00, 0x01, 0x51, 0x80 }; /* 1 day lease */
static char             dhcp_magic_cookie[]           = { 0x63, 0x82, 0x53, 0x63 };
static volatile char    dhcp_quit_flag = 0;
static int              dhcp_socket_handle            = -1;
static xTaskHandle      dhcp_thread_handle;
static dhcp_header_t    dhcp_header_buff;

/******************************************************
 *               Function Definitions
 ******************************************************/

void start_dhcp_server( uint32_t local_addr )
{
    xTaskCreate( dhcp_thread, (signed char*)"DHCP thread", DHCP_STACK_SIZE/sizeof( portSTACK_TYPE ), (void*)local_addr, DEFAULT_THREAD_PRIO, &dhcp_thread_handle);
}

void quit_dhcp_server( void )
{
    dhcp_quit_flag = 1;
}

/**
 *  Implements a very simple DHCP server.
 *
 *  Server will always offer next available address to a DISCOVER command
 *  Server will NAK any REQUEST command which is not requesting the next available address
 *  Server will ACK any REQUEST command which is for the next available address, and then increment the next available address
 *
 * @param my_addr : local IP address for binding of server port.
 */

static void dhcp_thread( void * thread_input )
{
    struct sockaddr_in    my_addr;
    struct sockaddr_in    source_addr;
    struct sockaddr_in    destination_addr;
    int                   status;
    char*                 option_ptr;
    socklen_t             source_addr_len;
    static dhcp_header_t* dhcp_header_ptr = &dhcp_header_buff;
    int                   recv_timeout = DHCP_SOCKET_TIMEOUT;

    /* Save local IP address to be sent in DHCP packets */
    my_addr.sin_addr.s_addr = (u32_t) thread_input;
    my_addr.sin_family = AF_INET;
    memcpy( &server_ip_addr_option_buff[2], &my_addr.sin_addr.s_addr, 4 );

    /* Add port number to IP address */
    my_addr.sin_port = htons( IPPORT_DHCPS );

    /* Create DHCP socket */
    dhcp_socket_handle = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt( dhcp_socket_handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&recv_timeout, sizeof( recv_timeout ) );

    /* Bind the socket to the local IP address */
    status = bind(dhcp_socket_handle, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));

    /* Set up the destination IP address and port number */
    destination_addr.sin_port   = htons( IPPORT_DHCPC );
    destination_addr.sin_family = AF_INET;
    memset( &destination_addr.sin_addr.s_addr, 0xff, 4 ); /* Broadcast */

    /* Loop endlessly */
    while ( dhcp_quit_flag == 0 )
    {
        /* Sleep until data is received from socket. */
        status = recvfrom( dhcp_socket_handle, (char *)dhcp_header_ptr, sizeof( dhcp_header_buff ), 0 , (struct sockaddr *) &source_addr, &source_addr_len);
        if ( status > 0 )
        {
            /* Check DHCP command */
            switch ( dhcp_header_ptr->options[2] )
            {
                case DHCPDISCOVER:
                    {
                        /* Discover command - send back OFFER response */
                        dhcp_header_ptr->opcode = BOOTP_OP_REPLY;

                        /* Clear the DHCP options list */
                        memset( &dhcp_header_ptr->options, 0, sizeof( dhcp_header_ptr->options ) );

                        /* Create the IP address for the Offer */
                        new_ip_addr[2] = next_available_ip_addr >> 8;
                        new_ip_addr[3] = next_available_ip_addr & 0xff;
                        memcpy( &dhcp_header_ptr->your_ip_addr, new_ip_addr, 4 );

                        /* Copy the magic DHCP number */
                        memcpy( dhcp_header_ptr->magic, dhcp_magic_cookie, 4 );

                        /* Add options */
                        option_ptr = (char *) &dhcp_header_ptr->options;
                        memcpy( option_ptr, dhcp_offer_option_buff, 3 );       /* DHCP message type */
                        option_ptr += 3;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* Server identifier */
                        option_ptr += 6;
                        memcpy( option_ptr, lease_time_option_buff, 6 );       /* Lease Time */
                        option_ptr += 6;
                        memcpy( option_ptr, subnet_option_buff, 6 );           /* Subnet Mask */
                        option_ptr += 6;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* Router (gateway) */
                        option_ptr[0] = 3; /* Router id */
                        option_ptr += 6;
                        memcpy( option_ptr, server_ip_addr_option_buff, 6 );   /* DNS server */
                        option_ptr[0] = 6; /* DNS server id */
                        option_ptr += 6;
                        memcpy( option_ptr, mtu_option_buff, 4 );              /* Interface MTU */
                        option_ptr += 4;
                        option_ptr[0] = 0xff; /* end options */
                        option_ptr++;

                        /* Send packet */
                        sendto( dhcp_socket_handle, (char *)dhcp_header_ptr, (int)(option_ptr - (char*)&dhcp_header_buff), 0 , (struct sockaddr *) &destination_addr, sizeof( destination_addr ));
                    }
                    break;

                case DHCPREQUEST:
                    {
                        /* REQUEST command - send back ACK or NAK */
                        unsigned char* requested_address;
                        uint32_t*      server_id_req;
                        uint32_t*      req_addr_ptr;
                        uint32_t*      newip_ptr;

                        /* Check that the REQUEST is for this server */
                        server_id_req = (uint32_t*) find_option( dhcp_header_ptr, 54 );
                        if ( ( server_id_req != NULL ) &&
                             ( my_addr.sin_addr.s_addr != *server_id_req ) )
                        {
                            break; /* Server ID does not match local IP address */
                        }

                        dhcp_header_ptr->opcode = BOOTP_OP_REPLY;

                        /* Locate the requested address in the options */
                        requested_address = find_option( dhcp_header_ptr, 50 );

                        /* Copy requested address */
                        memcpy( &dhcp_header_ptr->your_ip_addr, requested_address, 4 );

                        /* Blank options list */
                        memset( &dhcp_header_ptr->options, 0, sizeof( dhcp_header_ptr->options ) );

                        /* Copy DHCP magic number into packet */
                        memcpy( dhcp_header_ptr->magic, dhcp_magic_cookie, 4 );

                        option_ptr = (char *) &dhcp_header_ptr->options;

                        /* Check if Request if for next available IP address */
                        req_addr_ptr = (uint32_t*) dhcp_header_ptr->your_ip_addr;
                        newip_ptr = (uint32_t*) new_ip_addr;
                        if ( *req_addr_ptr != ( ( *newip_ptr & 0x0000ffff ) | ( ( next_available_ip_addr & 0xff ) << 24 ) | ( ( next_available_ip_addr & 0xff00 ) << 8 ) ) )
                        {
                            /* Request is not for next available IP - force client to take next available IP by sending NAK */
                            /* Add appropriate options */
                            memcpy( option_ptr, dhcp_nak_option_buff, 3 );  /* DHCP message type */
                            option_ptr += 3;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Server identifier */
                            option_ptr += 6;
                            memset( &dhcp_header_ptr->your_ip_addr, 0, sizeof( dhcp_header_ptr->your_ip_addr ) ); /* Clear 'your address' field */
                        }
                        else
                        {
                            /* Request is not for next available IP - force client to take next available IP by sending NAK
                             * Add appropriate options
                             */
                            memcpy( option_ptr, dhcp_ack_option_buff, 3 );       /* DHCP message type */
                            option_ptr += 3;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Server identifier */
                            option_ptr += 6;
                            memcpy( option_ptr, lease_time_option_buff, 6 );     /* Lease Time */
                            option_ptr += 6;
                            memcpy( option_ptr, subnet_option_buff, 6 );         /* Subnet Mask */
                            option_ptr += 6;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* Router (gateway) */
                            option_ptr[0] = 3; /* Router id */
                            option_ptr += 6;
                            memcpy( option_ptr, server_ip_addr_option_buff, 6 ); /* DNS server */
                            option_ptr[0] = 6; /* DNS server id */
                            option_ptr += 6;
                            memcpy( option_ptr, mtu_option_buff, 4 );            /* Interface MTU */
                            option_ptr += 4;
                            /* WPRINT_APP_INFO(("Assigned new IP address %d.%d.%d.%d\n", (uint8_t)new_ip_addr[0], (uint8_t)new_ip_addr[1], next_available_ip_addr>>8, next_available_ip_addr&0xff )); */

                            /* Increment IP address */
                            next_available_ip_addr++;
                            if ( ( next_available_ip_addr & 0xff ) == 0xff ) /* Handle low byte rollover */
                            {
                                next_available_ip_addr += 101;
                            }
                            if ( ( next_available_ip_addr >> 8 ) == 0xff ) /* Handle high byte rollover */
                            {
                                next_available_ip_addr += ( 2 << 8 );
                            }
                        }
                        option_ptr[0] = 0xff; /* end options */
                        option_ptr++;

                        /* Send packet */
                        sendto( dhcp_socket_handle, (char *)&dhcp_header_buff, (int)(option_ptr - (char*)&dhcp_header_buff), 0 , (struct sockaddr *) &destination_addr, sizeof( destination_addr ));
                    }
                    break;

                default:
                    break;
            }
        }
    }

    /* Delete DHCP socket */
    lwip_close( dhcp_socket_handle );

    /* Clean up this startup thread */
    vTaskDelete( dhcp_thread_handle );
}


/**
 *  Finds a specified DHCP option
 *
 *  Searches the given DHCP request and returns a pointer to the
 *  specified DHCP option data, or NULL if not found
 *
 * @param request :    The DHCP request structure
 * @param option_num : Which DHCP option number to find
 *
 * @return Pointer to the DHCP option data, or NULL if not found
 */

static unsigned char * find_option( dhcp_header_t* request, unsigned char option_num )
{
    unsigned char* option_ptr = (unsigned char*) request->options;
    while ( ( option_ptr[0] != 0xff ) &&
            ( option_ptr[0] != option_num ) &&
            ( option_ptr < ( (unsigned char*) request ) + sizeof( dhcp_header_t ) ) )
    {
        option_ptr += option_ptr[1] + 2;
    }
    if ( option_ptr[0] == option_num )
    {
        return &option_ptr[2];
    }
    return NULL;

}

