/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "lwip/opt.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "ipv4/lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "wiced_network.h"
#include "wiced_management.h"
#include "wwd_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
//#include "Platform/wiced_platform_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "../console.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#include "wiced_time.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define PING_RCV_TIMEO       (1000)    /* ping receive timeout - in milliseconds */
#define PING_ID              (0xAFAF)
#define PING_MAX_PAYLOAD_SIZE ( 10000 ) /* ping max size */

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
static void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len );
static err_t ping_recv( int socket_hnd );


/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint16_t      ping_seq_num;


/******************************************************
 *               Function Definitions
 ******************************************************/

/*!
 ******************************************************************************
 * Sends an ICMP ping to the indicated host or IP address
 *
 * @return  0 for success, otherwise error
 */

int ping( int argc, char *argv[] )
{
    struct hostent * host;
    struct sockaddr_in host_addr;
    int socket_hnd;
    int recv_timeout = PING_RCV_TIMEO;

    if ( argc == 1 )
    {
        return ERR_INSUFFICENT_ARGS;
    }

    int i        = 0;
    int len      = 100;
    int num      = 1;
    int interval = 1000;
    wiced_bool_t continuous = WICED_FALSE;

    host = gethostbyname( argv[1] );

    if ( host == NULL )
    {
        WPRINT_APP_INFO(( "Could not find host %s\n", argv[1] ));
        return ERR_UNKNOWN;
    }

    host_addr.sin_addr.s_addr = *((uint32_t*) host->h_addr_list[0]);
    host_addr.sin_len = sizeof( host_addr );
    host_addr.sin_family = AF_INET;

    /* Open a local socket for pinging */
    if ( ( socket_hnd = lwip_socket( AF_INET, SOCK_RAW, IP_PROTO_ICMP ) ) < 0 )
    {
        WPRINT_APP_INFO(( "unable to create socket for Ping\r\n" ));
        return ERR_UNKNOWN;
    }

    /* Set the receive timeout on local socket so pings will time out. */
    lwip_setsockopt( socket_hnd, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof( recv_timeout ) );

    WPRINT_APP_INFO (("PING %u.%u.%u.%u\r\n", (unsigned char) ( ( htonl( host_addr.sin_addr.s_addr ) >> 24 ) & 0xff ),
                                            (unsigned char) ( ( htonl( host_addr.sin_addr.s_addr ) >> 16 ) & 0xff ),
                                            (unsigned char) ( ( htonl( host_addr.sin_addr.s_addr ) >>  8 ) & 0xff ),
                                            (unsigned char) ( ( htonl( host_addr.sin_addr.s_addr ) >>  0 ) & 0xff ) ));

    for (i = 2; i < argc; i++ )
    {
        switch (argv[i][1])
        {
            case 'i':
                interval = atoi(argv[i+1]);
                if ( interval < 0 )
                {
                    WPRINT_APP_INFO(("min interval 0\n\r"));
                    return ERR_CMD_OK;
                }
                WPRINT_APP_INFO(("interval: %d milliseconds\n\r", interval));
                i++;
                break;

            case 'l':
                len = atoi(argv[i+1]);
                if ( ( len > PING_MAX_PAYLOAD_SIZE ) || ( len < 0 ) )
                {
                    WPRINT_APP_INFO(("max ping length: %d, min: 0\n\r", PING_MAX_PAYLOAD_SIZE));
                    return ERR_CMD_OK;
                }
                WPRINT_APP_INFO(("length: %d\n\r", len));
                i++;
                break;

            case 'n':
                num = atoi(argv[i+1]);
                if ( num < 1 )
                {
                    WPRINT_APP_INFO(("min number of packets 1\n\r"));
                    return ERR_CMD_OK;
                }
                WPRINT_APP_INFO(("number : %d\n\r", num));
                i++;
                break;

            case 't':
                continuous = WICED_TRUE;
                WPRINT_APP_INFO(("continuous...\n\r"));
                break;

            default:
                WPRINT_APP_INFO(("Not supported, ignoring: %s\n\r", argv[i]));
            break;
        }
    }

    struct icmp_echo_hdr *iecho;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + len;

    /* Allocate memory for packet */
    if ( !( iecho = mem_malloc( ping_size ) ) )
    {
        return ERR_MEM;
    }

    /* Construct ping request */
    ping_prepare_echo( iecho, ping_size );

    wiced_time_t send_time;
    wiced_time_t reply_time;

    while (( num > 0 ) || ( continuous == WICED_TRUE ) )
    {
        /* Send ping and wait for reply */
        send_time = host_rtos_get_time( );

        if ( lwip_sendto( socket_hnd, iecho, ping_size, 0, (struct sockaddr*) &host_addr, host_addr.sin_len ) > 0 )
        {
            /* Wait for ping reply */
            err_t result = ping_recv( socket_hnd );
            reply_time = host_rtos_get_time( );
            if ( ERR_OK == result )
            {
                WPRINT_APP_INFO( ("Ping Reply %dms\r\n", (int)( reply_time - send_time ) ) );
            }
            else
            {
                WPRINT_APP_INFO( ("Ping timeout\r\n") );
            }
        }
        else
        {
            WPRINT_APP_INFO(("Ping error\r\n"));
        }

        num--;
        if ( ( num > 0 ) || ( continuous == WICED_TRUE ) )
        {
            wiced_rtos_delay_milliseconds( interval ); // This is simple and should probably wait for a residual period
        }
    }

    // Free the packet
    mem_free( iecho );

    if ( 0 != lwip_close( socket_hnd ) )
    {
        WPRINT_APP_INFO( ("Could not close ping socket\r\n") );
        return ERR_UNKNOWN;
    }

    return ERR_CMD_OK;
}



/**
 *  Prepare the contents of an echo ICMP request packet
 *
 *  @param iecho  : Pointer to an icmp_echo_hdr structure in which the ICMP packet will be constructed
 *  @param len    : The length in bytes of the packet buffer passed to the iecho parameter
 *
 */

static void ping_prepare_echo( struct icmp_echo_hdr *iecho, uint16_t len )
{
    int i, payload_size;

    payload_size = len - sizeof(struct icmp_echo_hdr);

    ICMPH_TYPE_SET( iecho, ICMP_ECHO );
    ICMPH_CODE_SET( iecho, 0 );
    iecho->chksum = 0;
    iecho->id = PING_ID;
    iecho->seqno = htons( ++ping_seq_num );

    /* fill the additional data buffer with some data */
    for ( i = 0; i < payload_size; i++ )
    {
        ( (char*) iecho )[sizeof(struct icmp_echo_hdr) + i] = i;
    }

    iecho->chksum = inet_chksum( iecho, len );
}


/**
 *  Receive a Ping reply
 *
 *  Waits for a ICMP echo reply (Ping reply) to be received using the specified socket. Compares the
 *  sequence number, and ID number to the last ping sent, and if they match, returns ERR_OK, indicating
 *  a valid ping response.
 *
 *  @param socket_hnd : The handle for the local socket through which the ping reply will be received
 *
 *  @return  ERR_OK if valid reply received, ERR_TIMEOUT otherwise
 */

static err_t ping_recv( int socket_hnd )
{
    char buf[64];
    int fromlen, len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    while ( ( len = lwip_recvfrom( socket_hnd, buf, sizeof( buf ), 0, (struct sockaddr*) &from, (socklen_t*) &fromlen ) ) > 0 )
    {
        if ( len >= (int) ( sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr) ) )
        {
            iphdr = (struct ip_hdr *) buf;
            iecho = (struct icmp_echo_hdr *) ( buf + ( IPH_HL( iphdr ) * 4 ) );

            if ( ( iecho->id == PING_ID ) &&
                 ( iecho->seqno == htons( ping_seq_num ) ) &&
                 ( ICMPH_TYPE( iecho ) == ICMP_ER ) )
            {
                return ERR_OK; /* Echo reply received - return success */
            }
        }
    }

    return ERR_TIMEOUT; /* No valid echo reply received before timeout */
}

