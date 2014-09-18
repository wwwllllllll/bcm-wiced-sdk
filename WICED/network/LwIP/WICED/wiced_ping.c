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
#include "ipv4/lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"
#include "lwip/inet.h"
#include "netif/etharp.h"
#include "ipv4/lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "wiced.h"
#include "wiced_ping.h"
#include "wiced_network.h"
#include "RTOS/wwd_rtos_interface.h"

#define PING_ID              (0xAFAF)
#define PING_DATA_SIZE       (32)      /** ping additional data size to include in the packet */



int socket_for_ping;


struct icmp_packet
{
    struct icmp_echo_hdr hdr;
    uint8_t data[PING_DATA_SIZE];

};

static struct icmp_packet ping_packet;
static uint16_t     ping_seq_num;


static void  ping_prepare_echo( struct icmp_packet *iecho, uint16_t len );
static err_t ping_send( int socket_hnd, const wiced_ip_address_t* address );
static err_t ping_recv( int socket_hnd );

wiced_result_t wiced_ping_send(wiced_interface_t interface, const wiced_ip_address_t* address, uint32_t timeout_ms, uint32_t* elapsed_ms)
{
    wiced_time_t send_time;
    err_t result;
    UNUSED_PARAMETER(interface);

    /* Open a local socket for pinging */
    socket_for_ping = lwip_socket( AF_INET, SOCK_RAW, IP_PROTO_ICMP );
    if ( socket_for_ping < 0 )
    {
        return WICED_ERROR;
    }

    /* Set the receive timeout on local socket so pings will time out. */
    lwip_setsockopt( socket_for_ping, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof( timeout_ms ) );

    /* Send a ping */
    if ( ping_send( socket_for_ping, address ) != ERR_OK )
    {
        /* close a socket */
        lwip_close( socket_for_ping );
        return WICED_ERROR;
    }
    /* Record time ping was sent */
    send_time = host_rtos_get_time( );

    /* Wait for ping reply */
    result = ping_recv( socket_for_ping );
    if ( ERR_OK == result )
    {
        /* return elapsed time since a ping request was initiated */
        *elapsed_ms = (uint32_t)( host_rtos_get_time( ) - send_time );
        /* close a socket */
        lwip_close( socket_for_ping );
        return WICED_SUCCESS;
    }
    else
    {
        /* close a socket */
        lwip_close( socket_for_ping );
        return WICED_ERROR;
    }
}


static void ping_prepare_echo( struct icmp_packet *iecho, uint16_t len )
{
    int i;
    ICMPH_TYPE_SET(&iecho->hdr, ICMP_ECHO);
    ICMPH_CODE_SET(&iecho->hdr, 0);
    iecho->hdr.chksum = 0;
    iecho->hdr.id = PING_ID;
    iecho->hdr.seqno = htons( ++ping_seq_num );

    /* fill the additional data buffer with some data */
    for ( i = 0; i < (int)sizeof(iecho->data); i++ )
    {
        iecho->data[i] = (uint8_t)i;
    }

    iecho->hdr.chksum = inet_chksum( iecho, len );
}


static err_t ping_send( int socket_hnd, const wiced_ip_address_t* address )
{
    int                err;
    struct sockaddr_in to;
    struct icmp_packet *iecho = &ping_packet;

    /* Construct ping request */
    ping_prepare_echo( iecho, sizeof(ping_packet) );

    /* Send the ping request */
    to.sin_len         = sizeof( to );
    to.sin_family      = AF_INET;
    to.sin_addr.s_addr = htonl(address->ip.v4);

    err = lwip_sendto( socket_hnd, iecho, sizeof(ping_packet), 0, (struct sockaddr*) &to, sizeof( to ) );

    return ( err ? ERR_OK : ERR_VAL );
}


static err_t ping_recv( int socket_hnd )
{
    char                  buf[64];
    int                   fromlen;
    int                   len;
    struct sockaddr_in    from;
    struct ip_hdr*        iphdr;
    struct icmp_echo_hdr* iecho;

    do
    {
        len = lwip_recvfrom( socket_hnd, buf, sizeof( buf ), 0, (struct sockaddr*) &from, (socklen_t*) &fromlen );

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
    } while (len > 0);

    return ERR_TIMEOUT; /* No valid echo reply received before timeout */
}
