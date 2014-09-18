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
 */

#include "sockets.h"
#include "nx_api.h"
#include "wiced_tcpip.h"
#include "wwd_wifi.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define hton16(val) \
    ((uint16_t)((((uint16_t)(val) & (uint16_t)0x00ffU) << 8) | \
          (((uint16_t)(val) & (uint16_t)0xff00U) >> 8)))

#define SOCK4_ADDRESS(x)        (((struct sockaddr_in*)(x))->sin_addr.s_addr)
#define SOCK4_PORT(x)           (((struct sockaddr_in*)(x))->sin_port)

//#define MIN(a,b)  ((a) < (b) ? (a) : (b))

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef MAX_BSD_SOCKETS
#define MAX_BSD_SOCKETS      (5)
#endif

#define BSD_SUCCESS   (0)
#define BSD_ERROR     (-1)

#ifndef TCP_WINDOW_SIZE
#define TCP_WINDOW_SIZE   (8*1024)
#endif

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    BSD_SOCKET_UDP, BSD_SOCKET_TCP,
} bsd_socket_type_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    bsd_socket_type_t type;
    wiced_bool_t      available;
    uint32_t          local_bind_port; /* Only valid if bind has been called successfully */
    wiced_bool_t      reuse;
    uint32_t          select_timeout;
    struct sockaddr   last_received_peer;
    union
    {
        NX_UDP_SOCKET udp;
        NX_TCP_SOCKET tcp;
    } socket;
} bsd_socket_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

extern NX_IP          wiced_ip_handle[2]; /* 0=STA, 1=AP */
extern NX_PACKET_POOL wiced_packet_pools[2]; /* 0=TX, 1=RX */
static bsd_socket_t   sockets[MAX_BSD_SOCKETS];
static wiced_bool_t   socket_layer_inited = WICED_FALSE;
static wiced_interface_t interface;

/******************************************************
 *               Function Definitions
 ******************************************************/

void sockets_layer_init( void )
{
    int a;
    for ( a = 0; a < MAX_BSD_SOCKETS; ++a )
    {
        sockets[a].available = WICED_TRUE;
        sockets[a].select_timeout = NX_WAIT_FOREVER;
    }

    socket_layer_inited = WICED_TRUE;
}

INT socket( INT protocolFamily, INT type, INT protocol )
{
    int a = 0;
    bsd_socket_t* free_socket;

    if ( wwd_wifi_is_ready_to_transceive( WWD_STA_INTERFACE ) == WWD_SUCCESS )
    {
        interface = WICED_STA_INTERFACE;
    }
    else if ( wwd_wifi_is_ready_to_transceive( WWD_AP_INTERFACE ) == WWD_SUCCESS )
    {
        interface = WICED_AP_INTERFACE;
    }
    else
    {
        return BSD_ERROR;
    }

    if ( socket_layer_inited == WICED_FALSE )
    {
        sockets_layer_init( );
    }

    // Find a free socket
    for ( ; a < MAX_BSD_SOCKETS; ++a )
    {
        if ( sockets[a].available == WICED_TRUE )
        {
            free_socket = &sockets[a];
            free_socket->available = WICED_FALSE;
            free_socket->select_timeout = NX_WAIT_FOREVER;
            free_socket->local_bind_port = 0;
            free_socket->reuse = WICED_FALSE;

            switch ( type )
            {
                case SOCK_DGRAM:
                    free_socket->type = BSD_SOCKET_UDP;
                    /* Set option to maximum throughput delivery */
                    if ( nx_udp_socket_create(&wiced_ip_handle[interface], &free_socket->socket.udp, "udp_socket", NX_IP_MAX_DATA, NX_FRAGMENT_OKAY, 255, 5) != NX_SUCCESS )
                    {
                        return BSD_ERROR;
                    }
                    break;
                case SOCK_STREAM:
                    free_socket->type = BSD_SOCKET_TCP;
                    /* Set option to maximum throughput delivery */
                    if ( nx_tcp_socket_create( &wiced_ip_handle[interface], &free_socket->socket.tcp, "tcp_socket", NX_IP_MAX_DATA, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, TCP_WINDOW_SIZE, NX_NULL, NX_NULL ) != NX_SUCCESS )
                    {
                        return BSD_ERROR;
                    }
                    break;
            }

            return a;
        }
    }

    return BSD_ERROR;
}

INT send( INT sockID, const CHAR *msg, INT msgLength, INT flags )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            NX_PACKET* packet;
            INT progress = 0;
            while ( progress != msgLength )
            {
                nx_packet_allocate( &wiced_packet_pools[0], &packet, NX_IPv4_TCP_PACKET, NX_WAIT_FOREVER );
                INT data_length = MIN( msgLength, sockets[sockID].socket.tcp.nx_tcp_socket_mss );
                nx_packet_data_append( packet, (VOID*) msg, data_length, &wiced_packet_pools[0], NX_WAIT_FOREVER );
                if ( nx_tcp_socket_send(&sockets[sockID].socket.tcp, packet, NX_WAIT_FOREVER) != NX_SUCCESS )
                {
                    nx_packet_release( packet );
                    return BSD_ERROR;
                }
                else
                {
                    progress += data_length;
                    msg += data_length;
                }
            }
            return progress;
            break;
        }
        case BSD_SOCKET_UDP:
        {
            NX_PACKET* packet;
            nx_packet_allocate( &wiced_packet_pools[0], &packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER );
            nx_packet_data_append( packet, (VOID*) msg, msgLength, &wiced_packet_pools[0], NX_WAIT_FOREVER );
            if ( nx_udp_socket_send(&sockets[sockID].socket.udp, packet, htonl(SOCK4_ADDRESS(&sockets[sockID].last_received_peer)), hton16(SOCK4_PORT(&sockets[sockID].last_received_peer))) != NX_SUCCESS )
            {
                nx_packet_release( packet );
            }
            else
            {
                return BSD_SUCCESS;
            }
            break;
        }
    }

    return BSD_ERROR;
}

INT sendto( INT sockID, CHAR *msg, INT msgLength, INT flags, struct sockaddr *destAddr, INT destAddrLen )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            NX_PACKET* packet;
            nx_packet_allocate( &wiced_packet_pools[0], &packet, NX_IPv4_TCP_PACKET, NX_WAIT_FOREVER );
            INT data_length = MIN( msgLength, sockets[sockID].socket.tcp.nx_tcp_socket_mss );
            nx_packet_data_append( packet, (VOID*) msg, data_length, &wiced_packet_pools[0], NX_WAIT_FOREVER );
            if ( nx_tcp_socket_send(&sockets[sockID].socket.tcp, packet, NX_WAIT_FOREVER) != NX_SUCCESS )
            {
                nx_packet_release( packet );
            }
            else
            {
                return data_length;
            }
            break;
        }
        case BSD_SOCKET_UDP:
        {
            NX_PACKET* packet;

            /* Note that it's important to specify the packet as NX_IPv4_UDP_PACKET otherwise NetX Duo allocates another 20 bytes for IPv6 header and iperf won't work properly */
            nx_packet_allocate( &wiced_packet_pools[0], &packet, NX_IPv4_UDP_PACKET, NX_WAIT_FOREVER );
            nx_packet_data_append( packet, msg, msgLength, &wiced_packet_pools[0], NX_WAIT_FOREVER );
            if ( nx_udp_socket_send(&sockets[sockID].socket.udp, packet, htonl(SOCK4_ADDRESS(destAddr)), hton16(SOCK4_PORT(destAddr))) != NX_SUCCESS )
            {
                nx_packet_release( packet );
            }
            else
            {
                return msgLength;
            }
            break;
        }
    }

    return BSD_ERROR;
}

INT getpeername( INT sockID, struct sockaddr *remoteAddress, INT *addressLength )
{
    return BSD_ERROR;
}

INT getsockname( INT sockID, struct sockaddr *localAddress, INT *addressLength )
{
    return BSD_ERROR;
}

INT recvfrom( INT sockID, CHAR *buffer, INT buffersize, INT flags, struct sockaddr *fromAddr, INT *fromAddrLen )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            NX_PACKET* packet;
            if ( nx_tcp_socket_receive( &sockets[sockID].socket.tcp, &packet, NX_WAIT_FOREVER ) == NX_SUCCESS )
            {
                INT size = MIN( packet->nx_packet_length, buffersize );
                memcpy( buffer, packet->nx_packet_prepend_ptr, size );
                nx_packet_release( packet );
                return size;
            }
            break;
        }
        case BSD_SOCKET_UDP:
        {
            NX_PACKET* packet;
            if ( nx_udp_socket_receive( &sockets[sockID].socket.udp, &packet, NX_WAIT_FOREVER ) == NX_SUCCESS )
            {
                INT size = MIN( packet->nx_packet_length, buffersize );
                memcpy( buffer, packet->nx_packet_prepend_ptr, size );
                UINT port;
                ULONG address;
                nx_udp_source_extract( packet, &address, &port );
                SOCK4_ADDRESS(fromAddr) = htonl( address );
                SOCK4_PORT(fromAddr) = hton16(port);
                nx_packet_release( packet );
                return size;
            }
            break;
        }
    }

    return BSD_ERROR;
}

INT recv( INT sockID, VOID *rcvBuffer, INT bufferLength, INT flags )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            NX_PACKET* packet;
            if ( nx_tcp_socket_receive( &sockets[sockID].socket.tcp, &packet, sockets[sockID].select_timeout ) == NX_SUCCESS )
            {
                INT size = MIN( packet->nx_packet_length, bufferLength );
                memcpy( rcvBuffer, packet->nx_packet_prepend_ptr, size );
                nx_packet_release( packet );
                return size;
            }
            break;
        }
        case BSD_SOCKET_UDP:
        {
            NX_PACKET* packet;
            if ( nx_udp_socket_receive( &sockets[sockID].socket.udp, &packet, sockets[sockID].select_timeout ) == NX_SUCCESS )
            {
                INT size = MIN( packet->nx_packet_length, bufferLength );
                memcpy( rcvBuffer, packet->nx_packet_prepend_ptr, size );
                UINT port;
                ULONG address;
                nx_udp_source_extract( packet, &address, &port );
                SOCK4_ADDRESS(&sockets[sockID].last_received_peer) = htonl( address );
                SOCK4_PORT(&sockets[sockID].last_received_peer) = hton16(port);
                nx_packet_release( packet );
                return size;
            }
            break;
        }
    }

    return BSD_ERROR;
}

INT accept( INT sockID, struct sockaddr *ClientAddress, INT *addressLength )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            if ( sockets[sockID].socket.tcp.nx_tcp_socket_state != NX_TCP_LISTEN_STATE )
            {
                nx_tcp_socket_disconnect( &sockets[sockID].socket.tcp, NX_WAIT_FOREVER );
            }

            nx_tcp_server_socket_unaccept( &sockets[sockID].socket.tcp );
            nx_tcp_server_socket_relisten( sockets[sockID].socket.tcp.nx_tcp_socket_ip_ptr, sockets[sockID].socket.tcp.nx_tcp_socket_port, &sockets[sockID].socket.tcp );

            if ( nx_tcp_server_socket_accept( &sockets[sockID].socket.tcp, NX_WAIT_FOREVER ) == NX_SUCCESS )
            {
                return BSD_SUCCESS;
            }
            break;
        }
        case BSD_SOCKET_UDP:
            return BSD_ERROR;
            break;
    }

    return BSD_ERROR;
}

INT listen( INT sockID, INT backlog )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            if ( nx_tcp_server_socket_listen( &wiced_ip_handle[interface], sockets[sockID].local_bind_port, &sockets[sockID].socket.tcp, backlog, NULL ) == NX_SUCCESS )
            {
                return BSD_SUCCESS;
            }
            break;
        }
        case BSD_SOCKET_UDP:
            return BSD_ERROR;
            break;
    }

    return BSD_ERROR;
}

INT bind( INT sockID, struct sockaddr *localAddress, INT addressLength )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            sockets[sockID].local_bind_port = hton16(SOCK4_PORT(localAddress));
            return BSD_SUCCESS;
        }
        case BSD_SOCKET_UDP:
        {
            if ( nx_udp_socket_bind( &sockets[sockID].socket.udp, hton16(SOCK4_PORT(localAddress)), NX_WAIT_FOREVER ) == NX_SUCCESS )
            {
                return BSD_SUCCESS;
            }
            break;
        }
    }

    return BSD_ERROR;
}

INT connect( INT sockID, struct sockaddr *remoteAddress, INT addressLength )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
        {
            if ( !sockets[sockID].socket.tcp.nx_tcp_socket_bound_next )
            {
                nx_tcp_client_socket_bind( &sockets[sockID].socket.tcp, sockets[sockID].local_bind_port, NX_WAIT_FOREVER );
            }
            if ( remoteAddress->sa_family == AF_INET )
            {
                if ( nx_tcp_client_socket_connect( &sockets[sockID].socket.tcp, htonl( SOCK4_ADDRESS(remoteAddress) ), hton16(SOCK4_PORT(remoteAddress)), NX_WAIT_FOREVER ) == NX_SUCCESS )
                {
                    return BSD_SUCCESS;
                }
            }
            break;
        }
        case BSD_SOCKET_UDP:
            return BSD_ERROR;
            break;
    }

    return BSD_ERROR;
}

INT select( INT nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout )
{
    /*
     * iperf only uses select() in two places. Both are used to check if the socket is readable.
     * So instead of waiting here, we'll record the wait timeout and use it in the recv() call that follows
     */
    sockets[*readfds].select_timeout = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    return 1;
}

INT soc_close( INT sockID )
{
    switch ( sockets[sockID].type )
    {
        case BSD_SOCKET_TCP:
            nx_tcp_socket_disconnect( &sockets[sockID].socket.tcp, NX_WAIT_FOREVER );
            nx_tcp_server_socket_unaccept( &sockets[sockID].socket.tcp );

            if ( sockets[sockID].reuse == WICED_FALSE )
            {
                nx_tcp_server_socket_unlisten( sockets[sockID].socket.tcp.nx_tcp_socket_ip_ptr, sockets[sockID].socket.tcp.nx_tcp_socket_port );
                nx_tcp_client_socket_unbind( &sockets[sockID].socket.tcp );
                if ( nx_tcp_socket_delete( &sockets[sockID].socket.tcp ) != NX_SUCCESS )
                {
                    sockets[sockID].available = WICED_FALSE;
                }
                sockets[sockID].available = WICED_TRUE;
            }
            break;

        case BSD_SOCKET_UDP:
            if ( sockets[sockID].socket.udp.nx_udp_socket_bound_next )
            {
                nx_udp_socket_unbind( &sockets[sockID].socket.udp );
            }
            if ( nx_udp_socket_delete( &sockets[sockID].socket.udp ) != NX_SUCCESS )
            {
                sockets[sockID].available = WICED_FALSE;
            }
            sockets[sockID].available = WICED_TRUE;
            break;
    }
    return BSD_SUCCESS;
}

INT setsockopt( INT sockID, INT option_level, INT option_name, const void *option_value, INT option_length )
{
    /*
     * Socket options used by iperf:
     * Listener.cpp:
     * SO_REUSEADDR
     * IP_ADD_MEMBERSHIP
     * IPV6_ADD_MEMBERSHIP
     * IP_MULTICAST_TTL
     * IPV6_MULTICAST_HOPS
     *
     * PerfSocket.cpp:
     * TCP_CONGESTION
     * IP_MULTICAST_TTL
     * IPV6_MULTICAST_HOPS
     * IP_TOS
     * TCP_NODELAY
     *
     * tcp_window_size:
     * TCP_WINSHIFT
     * TCP_RFC1323
     * SO_RCVBUF
     * SO_SNDBUF
     */
    switch(option_name)
    {
        case SO_REUSEADDR:
            sockets[sockID].reuse = WICED_TRUE;
            break;
        case IP_ADD_MEMBERSHIP:
            nx_igmp_multicast_join(&wiced_ip_handle[interface], htonl(*(uint32_t*)option_value));
            break;
        case IP_TOS:
            if (sockets[sockID].type == BSD_SOCKET_TCP)
            {
                sockets[sockID].socket.tcp.nx_tcp_socket_type_of_service = (*(uint32_t*)option_value) << 16;
            }
            else if (sockets[sockID].type == BSD_SOCKET_UDP)
            {
                sockets[sockID].socket.udp.nx_udp_socket_type_of_service = (*(uint32_t*)option_value) << 16;
            }
            break;
        default:
//            printf("Setsockopt %d\r\n", option_name);
            break;
    }
    return BSD_SUCCESS;
}

INT getsockopt( INT sockID, INT option_level, INT option_name, void *option_value, INT *option_length )
{
    /* Socket options requested by iperf:
     * sockets.c:
     * TCP_MAXSEG
     *
     * tcp_window_size.c:
     * SO_SNDBUF
     * SO_RCVBUF
     */
    switch ( option_name )
    {
        case TCP_MAXSEG:
            *(uint32_t*) option_value = 1024;
            break;
        case SO_SNDBUF:
        case SO_RCVBUF:
            *(uint32_t*) option_value = TCP_WINDOW_SIZE;
            break;
    }
    return BSD_SUCCESS;
}
