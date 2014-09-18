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
 * LwIP TCP/IP library
 */

#include "wiced.h"
#include "wiced_network.h"
#include "wiced_ping.h"
#include "lwip/api.h"
#include "lwip/netbuf.h"
#include "ipv4/lwip/igmp.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_tcpip.h"
#include "wwd_assert.h"
#include "wwd_crypto.h"
#include "lwipopts.h"
#include "queue.h"
#include "semphr.h"
#include "dns.h"
#include "wiced_tls.h"
#include "tls_host_api.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define WICED_LINK_CHECK( interface )     do { if ( netif_is_up( &IP_HANDLE(interface) ) != 1){ return WICED_NOTUP; }} while(0)
#define CONVERT_TO_LWIP_TIMEOUT(timeout)   do { if ( timeout == 0 ){ timeout = 1; }} while(0)

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS
#define WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS    (5)
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
 *               Function Declarations
 ******************************************************/

static wiced_result_t internal_tcp_receive  ( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout );
static wiced_result_t internal_udp_send     ( struct netconn* handler, wiced_packet_t* packet, wiced_interface_t interface );
static wiced_result_t internal_packet_create( wiced_packet_t** packet, uint16_t content_length, uint8_t** data, uint16_t* available_space );
static void internal_async_socket_callback  ( struct netconn* conn, enum netconn_evt event, u16_t length );
static wiced_tcp_socket_t* internal_netconn_to_wiced_async_socket( struct netconn *conn );

/* TLS helper function to do TCP without involving TLS context */
wiced_result_t network_tcp_receive    ( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout );
wiced_result_t network_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t*  packet );


/******************************************************
 *               Variables Definitions
 ******************************************************/

extern xSemaphoreHandle send_interface_mutex;
static wiced_tcp_socket_t* tcp_sockets_with_callbacks[WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS];

static wiced_result_t lwip_to_wiced_err[] =
{
[-(ERR_OK        )] = WICED_SUCCESS,
[-(ERR_MEM       )] = WICED_OUT_OF_HEAP_SPACE,
[-(ERR_BUF       )] = WICED_PACKET_BUFFER_CORRUPT,
[-(ERR_TIMEOUT   )] = WICED_TIMEOUT,
[-(ERR_RTE       )] = WICED_ROUTING_ERROR,
[-(ERR_INPROGRESS)] = WICED_PENDING,
[-(ERR_VAL       )] = WICED_BADVALUE,
[-(ERR_WOULDBLOCK)] = WICED_WOULD_BLOCK,
[-(ERR_ABRT      )] = WICED_ABORTED,
[-(ERR_RST       )] = WICED_CONNECTION_RESET,
[-(ERR_CLSD      )] = WICED_CONNECTION_CLOSED,
[-(ERR_CONN      )] = WICED_NOT_CONNECTED,
[-(ERR_ARG       )] = WICED_BADARG,
[-(ERR_USE       )] = WICED_ADDRESS_IN_USE,
[-(ERR_IF        )] = WICED_NETWORK_INTERFACE_ERROR,
[-(ERR_ISCONN    )] = WICED_ALREADY_CONNECTED,

};

#define LWIP_TO_WICED_ERR( lwip_err )  ((lwip_err >= ERR_ISCONN)? lwip_to_wiced_err[ -lwip_err ] : WICED_UNKNOWN_NETWORK_STACK_ERROR )

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_ping( wiced_interface_t interface, const wiced_ip_address_t* address, uint32_t timeout_ms, uint32_t* elapsed_ms )
{
    WICED_LINK_CHECK( interface );

    return wiced_ping_send( interface, address, timeout_ms, elapsed_ms );
}

wiced_result_t wiced_multicast_join( wiced_interface_t interface, const wiced_ip_address_t* address )
{
    uint32_t temp;
    err_t res;

    WICED_LINK_CHECK( interface );

    temp = htonl(address->ip.v4);
    res = igmp_joingroup( &IP_HANDLE(interface).ip_addr, (ip_addr_t*) &temp );
    return LWIP_TO_WICED_ERR( res );
}

wiced_result_t wiced_multicast_leave( wiced_interface_t interface, const wiced_ip_address_t* address )
{
    uint32_t temp;
    err_t res;

    WICED_LINK_CHECK( interface );

    /* call lwip stack to leave a multicast group */
    temp = htonl(address->ip.v4);
    res = igmp_leavegroup( &IP_HANDLE( interface ).ip_addr, (ip_addr_t*) &temp );
    return LWIP_TO_WICED_ERR( res );
}

wiced_result_t wiced_ip_get_gateway_address( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    ipv4_address->version = WICED_IPV4;
    ipv4_address->ip.v4 = htonl(IP_HANDLE(interface).gw.addr);
    return WICED_SUCCESS;
}

wiced_result_t wiced_ip_get_netmask( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    SET_IPV4_ADDRESS( *ipv4_address, htonl(IP_HANDLE(interface).netmask.addr));
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_create_socket( wiced_tcp_socket_t* socket, wiced_interface_t interface )
{
    /* Check whether the interface is a valid one and it was attached to an IP address */
    if ( IP_HANDLE(interface).ip_addr.addr == 0 )
    {
        return WICED_INVALID_INTERFACE;
    }

    socket->conn_handler = netconn_new( NETCONN_TCP );
    if( !socket->conn_handler )
    {
        return WICED_SOCKET_CREATE_FAIL;
    }

    /* get the local ip addr of the given network interface and keep it inside */
    /* socket structure since we will need it for future uses when we are going to perform a bind or connect */
    socket->local_ip_addr.addr = IP_HANDLE(interface).ip_addr.addr;
    socket->is_bound = WICED_FALSE;
    socket->accept_handler = ( struct netconn* )0;
    //socket->conn_handler = ( struct netconn* )0;
    socket->interface = interface;

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_enable_keepalive( wiced_tcp_socket_t* socket, uint16_t interval, uint16_t probes, uint16_t _time )
{
    struct netconn*             conn;
    wiced_assert("Bad args", socket != NULL);
    /* Just set a few options of the socket */
    if( socket->accept_handler )
    {
        /* if we are a server, then configure accept tcp pcb */
        conn = socket->accept_handler;
    }
    else
    {
        if( socket->conn_handler == NULL )
        {
            return WICED_INVALID_SOCKET;
        }
        conn = socket->conn_handler;
    }
    conn->pcb.tcp->so_options |= SOF_KEEPALIVE;
    conn->pcb.tcp->keep_idle  =  (u32_t)( 1000 * _time );
    conn->pcb.tcp->keep_intvl =  (u32_t)( 1000 * interval );
    conn->pcb.tcp->keep_cnt   =  (u32_t)( probes );

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_server_peer( wiced_tcp_socket_t* socket, wiced_ip_address_t* address, uint16_t* port )
{
    ip_addr_t peer_address;
    err_t       res;

    wiced_assert("Bad args", (socket != NULL) && (address != NULL) && (port != NULL));

    if ( ( res = netconn_peer( socket->accept_handler, (ip_addr_t*) &peer_address, port ) ) == ERR_OK )
    {
        SET_IPV4_ADDRESS( *address, htonl( peer_address.addr ) );
        return WICED_SUCCESS;
    }

    return LWIP_TO_WICED_ERR( res );
}

wiced_result_t wiced_tcp_accept( wiced_tcp_socket_t* socket )
{
    err_t status;

    wiced_assert("Bad args", socket != NULL);

    WICED_LINK_CHECK( socket->interface );

    if ( socket->tls_context != NULL )
    {
        wiced_tls_reset_context( socket->tls_context);
    }

    if ( socket->accept_handler != NULL )
    {
        /* delete previous connection and use the same connection handler to accept a new one */
        status = netconn_delete( socket->accept_handler );
        socket->accept_handler = NULL;
        if ( status != ERR_OK )
        {
            return LWIP_TO_WICED_ERR( status );
        }
    }

    if ( ( status =  netconn_accept( socket->conn_handler, &socket->accept_handler ) ) != ERR_OK )
    {
        netconn_delete( socket->accept_handler );
        socket->accept_handler = NULL;
        return LWIP_TO_WICED_ERR( status );
    }

    if ( socket->tls_context != NULL )
    {
        wiced_result_t result = wiced_tcp_start_tls( socket, WICED_TLS_AS_SERVER, WICED_TLS_DEFAULT_VERIFICATION );
        if ( result != WICED_SUCCESS )
        {
            WPRINT_LIB_INFO( ( "Error starting TLS connection\n" ) );
            return result;
        }
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_connect( wiced_tcp_socket_t* socket, const wiced_ip_address_t* address, uint16_t port, uint32_t timeout )
{
    uint32_t temp;
    err_t    lwip_error;
    UNUSED_PARAMETER( timeout );

    wiced_assert("Bad args", (socket != NULL) && (address != NULL));

    WICED_LINK_CHECK( socket->interface );

    temp = htonl(GET_IPV4_ADDRESS(*address));
    if ( socket->conn_handler == NULL )
    {
        socket->conn_handler = netconn_new( NETCONN_TCP );
        if ( socket->conn_handler == NULL )
        {
            return WICED_SOCKET_CREATE_FAIL;
        }
        socket->is_bound = WICED_TRUE;
    }
    lwip_error = netconn_connect( socket->conn_handler, (ip_addr_t*) &temp, port, (uint16_t) timeout );
    if ( lwip_error != ERR_OK )
    {
        return LWIP_TO_WICED_ERR( lwip_error );
    }

    socket->conn_handler->pcb.tcp->flags &= (uint8_t) ( ~TF_NODELAY );

    if ( socket->tls_context != NULL )
    {
        wiced_result_t result = wiced_tcp_start_tls( socket, WICED_TLS_AS_CLIENT, WICED_TLS_DEFAULT_VERIFICATION );
        if ( result != WICED_SUCCESS)
        {
            netconn_delete( socket->conn_handler );
            socket->conn_handler = NULL;
            return result;
        }
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_bind( wiced_tcp_socket_t* socket, uint16_t port )
{
    err_t lwip_error;
    wiced_assert("Bad args", socket != NULL);

    WICED_LINK_CHECK( socket->interface );

    if ( socket->conn_handler != NULL )
    {
        lwip_error = netconn_bind( socket->conn_handler, &socket->local_ip_addr, port );
        if ( lwip_error == ERR_OK)
        {
            socket->is_bound = WICED_TRUE;
            return WICED_SUCCESS;
        }
        return LWIP_TO_WICED_ERR( lwip_error );
    }

    return WICED_INVALID_SOCKET;
}

wiced_result_t wiced_tcp_delete_socket( wiced_tcp_socket_t* socket )
{
    err_t res;

    wiced_assert("Bad args", socket != NULL);

    if ( socket->tls_context != NULL )
    {
        /* Check if context is of an advanced variety. Note that the server and advanced client context are exactly the same */
        wiced_tls_deinit_context( socket->tls_context );
        if ( socket->context_malloced == WICED_TRUE )
        {
            free( socket->tls_context );
            socket->tls_context = NULL;
            socket->context_malloced = WICED_FALSE;
        }
    }

    res = netconn_delete( socket->conn_handler );
    if ( res != ERR_OK )
    {
        socket->conn_handler = NULL;
        return LWIP_TO_WICED_ERR( res );
    }

    if ( socket->accept_handler != NULL )
    {
        netconn_delete( socket->accept_handler );
    }

    socket->accept_handler = NULL;
    socket->conn_handler   = NULL;
    socket->is_bound       = WICED_FALSE;

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_listen( wiced_tcp_socket_t* socket, uint16_t port )
{
    err_t res;
    wiced_assert("Bad args", socket != NULL);

    WICED_LINK_CHECK( socket->interface );

    if ( socket->conn_handler == NULL)
    {
        return WICED_INVALID_SOCKET;
    }

    /* Check if this socket is already listening */
    if (socket->conn_handler->state == NETCONN_LISTEN)
    {
        return WICED_SUCCESS;
    }

    /* Ensure the socket is bound to a port */
    if ( socket->is_bound == WICED_FALSE )
    {
        res = netconn_bind( socket->conn_handler, &socket->local_ip_addr, port );
        if ( res != ERR_OK )
        {
            return LWIP_TO_WICED_ERR( res );
        }
    }

    res = netconn_listen_with_backlog( socket->conn_handler, WICED_DEFAULT_TCP_LISTEN_QUEUE_SIZE );
    return LWIP_TO_WICED_ERR( res );
}

wiced_result_t wiced_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t* packet )
{
    if ( socket->tls_context != NULL )
    {
        wiced_tls_encrypt_packet( &socket->tls_context->context, packet );
    }

    return network_tcp_send_packet( socket, packet );
}

wiced_result_t network_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t* packet )
{
    err_t    status;
    uint16_t length;
    void*    data;
    uint16_t available;
    wiced_result_t result;

    wiced_assert("Bad args", socket != NULL);

    WICED_LINK_CHECK( socket->interface );

    /* Get data pointer of the payload from the allocated packet */
    packet->p->len = packet->p->tot_len;
    result = wiced_packet_get_data( packet, 0, (uint8_t**) &data, &length, &available );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    if ( socket->accept_handler == NULL )
    {
        status = netconn_write( socket->conn_handler, data, length, NETCONN_COPY );
    }
    else
    {
        status = netconn_write( socket->accept_handler, data, length, NETCONN_COPY );
    }

    if ( status != ERR_OK )
    {
        return LWIP_TO_WICED_ERR( status );
    }

    /* Release the use provided packet as the contents have been copied */
    netbuf_delete( packet );

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_send_buffer( wiced_tcp_socket_t* socket, const void* buffer, uint16_t buffer_length )
{
    wiced_packet_t* packet;
    uint8_t*        packet_data_ptr;
    uint16_t        available_space;
    uint8_t*        data_ptr = (uint8_t*)buffer;

    wiced_assert("Bad args", socket != NULL);

    WICED_LINK_CHECK( socket->interface );

    /* Create a packet, copy the data and send it off */
    while ( buffer_length != 0 )
    {
        uint16_t data_to_write;
        wiced_result_t result = wiced_packet_create_tcp( socket, buffer_length, &packet, &packet_data_ptr, &available_space );
        if ( result != WICED_SUCCESS)
        {
            return result;
        }

        /* Write data */
        data_to_write   = MIN(buffer_length, available_space);
        packet_data_ptr = MEMCAT(packet_data_ptr, data_ptr, data_to_write);
        wiced_packet_set_data_end( packet, packet_data_ptr );

        /* Update variables */
        data_ptr       += data_to_write;
        buffer_length   = (uint16_t) ( buffer_length - data_to_write );
        available_space = (uint16_t) ( available_space - data_to_write );

        result = wiced_tcp_send_packet( socket, packet );
        if ( result != WICED_SUCCESS )
        {
            wiced_packet_delete( packet );
            return result;
        }
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    wiced_assert("Bad args", (socket != NULL) && (packet != NULL) && (socket->conn_handler != NULL));

    CONVERT_TO_LWIP_TIMEOUT(timeout);

    WICED_LINK_CHECK( socket->interface );

    if ( socket->tls_context != NULL )
    {
        return wiced_tls_receive_packet( socket, packet, timeout );
    }
    else
    {
        return internal_tcp_receive( socket, packet, timeout );
    }
}

wiced_result_t network_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    wiced_assert("Bad args", (socket != NULL) && (packet != NULL) && (socket->conn_handler != NULL));

    CONVERT_TO_LWIP_TIMEOUT(timeout);
    return internal_tcp_receive( socket, packet, timeout );
}

wiced_result_t wiced_tcp_disconnect( wiced_tcp_socket_t* socket )
{
    wiced_assert("Bad args", socket != NULL);

    if ( socket->accept_handler != NULL )
    {
        err_t res = netconn_delete( socket->accept_handler );
        if ( res != ERR_OK )
        {
            return LWIP_TO_WICED_ERR( res );
        }

        socket->accept_handler = NULL;
    }
    else
    {
        netconn_delete( socket->conn_handler );
        socket->conn_handler = NULL;
        socket->is_bound     = WICED_FALSE;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_register_callbacks( wiced_tcp_socket_t* socket, wiced_socket_callback_t connect_callback, wiced_socket_callback_t receive_callback, wiced_socket_callback_t disconnect_callback)
{
    int a;

    /* Add the socket to the callback list */
    for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
    {
        if ( tcp_sockets_with_callbacks[a] == socket || tcp_sockets_with_callbacks[a] == NULL )
        {
            tcp_sockets_with_callbacks[a] = socket;
            break;
        }
    }

    if ( connect_callback != NULL )
    {
        socket->callbacks[WICED_TCP_CONNECT_CALLBACK_INDEX] = connect_callback;
    }
    if ( receive_callback != NULL )
    {
        socket->callbacks[WICED_TCP_RECEIVE_CALLBACK_INDEX] = receive_callback;
    }
    if ( disconnect_callback != NULL )
    {
        socket->callbacks[WICED_TCP_DISCONNECT_CALLBACK_INDEX] = disconnect_callback;
    }

    netconn_set_async_callback( socket->conn_handler, internal_async_socket_callback );

    return WICED_SUCCESS;

}

wiced_result_t wiced_packet_create_tcp( wiced_tcp_socket_t* socket, uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    wiced_result_t result;

    UNUSED_PARAMETER(content_length);

    result = internal_packet_create( packet, MAX_TCP_PAYLOAD_SIZE, data, available_space );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }
    if ( socket->tls_context != NULL )
    {
        uint32_t temp;

        /* Reserve space for the TLS record header */
        pbuf_header((*packet)->p, -(s16_t)sizeof(tls_record_header_t));
        *data = (*packet)->p->payload;

        /* Reserve space at the end for the TLS MAC */
        temp = (*packet)->p->len;
        if ( socket->tls_context->context.ivlen != 0 )
        {
            temp -= temp % (uint32_t) socket->tls_context->context.ivlen;
        }
        temp -= (uint32_t) socket->tls_context->context.maclen + 40;
        *available_space = (uint16_t) temp;
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_packet_create_udp( wiced_udp_socket_t* socket, uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    UNUSED_PARAMETER( socket );
    UNUSED_PARAMETER( content_length );

    return internal_packet_create( packet, MAX_UDP_PAYLOAD_SIZE, data, available_space );
}


wiced_result_t wiced_packet_create( uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    UNUSED_PARAMETER( content_length );

    return internal_packet_create( packet, MAX_IP_PAYLOAD_SIZE, data, available_space );
}


wiced_result_t wiced_packet_delete( wiced_packet_t* packet )
{
    wiced_assert("Bad args", packet != NULL);
    netbuf_delete( packet );
    return WICED_SUCCESS;
}

wiced_result_t wiced_packet_set_data_end( wiced_packet_t* packet, uint8_t* data_end )
{
    packet->p->len     = (uint16_t) ( data_end - ( (uint8_t*) packet->p->payload ) );
    packet->p->tot_len = packet->p->len;

    return WICED_SUCCESS;
}

wiced_result_t wiced_packet_set_data_start( wiced_packet_t* packet, uint8_t* data_start )
{
    pbuf_header( packet->p, (s16_t) ( (uint8_t*) packet->p->payload - data_start ) );

    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_packet_get_info( wiced_packet_t* packet, wiced_ip_address_t* address, uint16_t* port )
{
    UNUSED_PARAMETER( address );

    address->version = WICED_IPV4;
    address->ip.v4   = htonl(packet->addr.addr);
    *port            = netbuf_fromport( packet );

    return WICED_SUCCESS;
}

wiced_result_t wiced_packet_get_data( wiced_packet_t* packet, uint16_t offset, uint8_t** data, uint16_t* fragment_available_data_length, uint16_t *total_available_data_length )
{
    s8_t get_next_result;

    wiced_assert( "Bad args", (packet != NULL) && (data != NULL) && (fragment_available_data_length != NULL) && (total_available_data_length != NULL) );

    netbuf_first( packet );
    *total_available_data_length = (uint16_t)( netbuf_len(packet) - offset );

    do
    {
        uint16_t frag_size = packet->ptr->len;
        *data        = packet->ptr->payload;

        if ( frag_size == 0 )
        {
            *data                           = NULL;
            *fragment_available_data_length = 0;
            *total_available_data_length    = 0;
            return WICED_SUCCESS;
        }
        else if ( offset < frag_size )
        {
            *data += offset;
            *fragment_available_data_length = (uint16_t)(frag_size - offset);
            return WICED_SUCCESS;
        }
        else
        {
            offset = (uint16_t)(offset - frag_size);
            get_next_result = netbuf_next( packet );
        }
    } while ( get_next_result != -1 );

    return WICED_CORRUPT_PACKET_BUFFER;
}

wiced_result_t wiced_ip_get_ipv4_address( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    ipv4_address->version = WICED_IPV4;
    ipv4_address->ip.v4   = htonl(IP_HANDLE(interface).ip_addr.addr);
    return WICED_SUCCESS;
}

wiced_result_t wiced_ip_get_ipv6_address( wiced_interface_t interface, wiced_ip_address_t* ipv6_address, wiced_ipv6_address_type_t address_type )
{
    UNUSED_PARAMETER( interface );
    UNUSED_PARAMETER( ipv6_address );
    UNUSED_PARAMETER( address_type );
//    wiced_assert("IPv6 unsupported in LwIP", 0!=0);
    return WICED_UNSUPPORTED;
}

wiced_result_t wiced_udp_create_socket( wiced_udp_socket_t* socket, uint16_t port, wiced_interface_t interface )
{
    err_t status;
    wiced_assert("Bad args", socket != NULL);

    /* Check link and return if there is no link */
    WICED_LINK_CHECK( interface );

    socket->conn_handler = netconn_new( NETCONN_UDP );
    if( socket->conn_handler == NULL )
    {
        return WICED_SOCKET_CREATE_FAIL;
    }

    /* Bind it to designated port and an interface */
    status = netconn_bind( socket->conn_handler, &IP_HANDLE(interface).ip_addr, port );
    if( status != ERR_OK )
    {
        netconn_delete( socket->conn_handler );
        socket->conn_handler = NULL;
        return LWIP_TO_WICED_ERR( status );
    }
    socket->is_bound  = WICED_TRUE;
    socket->interface = interface;

    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_send( wiced_udp_socket_t* socket, const wiced_ip_address_t* address, uint16_t port, wiced_packet_t* packet )
{
    uint32_t temp;
    err_t status;
    wiced_result_t result;

    wiced_assert("Bad args", (socket != NULL) && (address != NULL) && (packet != NULL));

    WICED_LINK_CHECK( socket->interface );

    /* Associate UDP socket with specific remote IP address and a port */
    temp = htonl(address->ip.v4);
    status = netconn_connect( socket->conn_handler, (ip_addr_t*) &temp, port, 0 );
    if ( status != ERR_OK )
    {
        return LWIP_TO_WICED_ERR( status );
    }

    /* Total length and a length must be equal for a packet to be valid */
    packet->p->len = packet->p->tot_len;

    /* Send the packet via UDP socket */
    result = internal_udp_send( socket->conn_handler, packet, (wiced_interface_t)socket->interface );
    if ( result != WICED_SUCCESS )
    {
        netconn_disconnect( socket->conn_handler );
        return result;
    }

    netbuf_delete( packet );

    /* Return back to disconnected state
     * Note: We are ignoring the return for this as we MUST return WICED_SUCCESS otherwise the caller may attempt to
     * free the packet a second time.
     */
    netconn_disconnect( socket->conn_handler );

    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_reply( wiced_udp_socket_t* socket, wiced_packet_t* in_packet, wiced_packet_t* out_packet )
{
    err_t status;
    UNUSED_PARAMETER(in_packet);

    wiced_assert("Bad args", (socket != NULL) && (in_packet != NULL) && (out_packet != NULL));

    WICED_LINK_CHECK( socket->interface );

    /* Verify connection is valid */
    if ( socket->conn_handler == NULL )
    {
        return WICED_INVALID_SOCKET;
    }

    status = netconn_send( socket->conn_handler, out_packet );
    if ( status != ERR_OK )
    {
        return LWIP_TO_WICED_ERR( status );
    }

    /* Remove our reference to the sent packet */
    netbuf_delete( out_packet );

    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_receive( wiced_udp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    err_t status;
    CONVERT_TO_LWIP_TIMEOUT(timeout);

    wiced_assert("Bad args", socket != NULL);

    WICED_LINK_CHECK( socket->interface );

    netconn_set_recvtimeout( socket->conn_handler, (int)timeout );

    status = netconn_recv( socket->conn_handler, packet );
    if ( status != ERR_OK )
    {
        return LWIP_TO_WICED_ERR( status );
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_udp_delete_socket( wiced_udp_socket_t* socket )
{
    wiced_assert("Bad args", socket != NULL);

    if ( socket->conn_handler == NULL )
    {
        return WICED_INVALID_SOCKET;
    }

    /* Note: No need to check return value of netconn_delete. It only ever returns ERR_OK */
    netconn_delete( socket->conn_handler );

    socket->conn_handler = NULL;
    return WICED_SUCCESS;
}

wiced_result_t wiced_hostname_lookup( const char* hostname, wiced_ip_address_t* address, uint32_t timeout_ms )
{
    wiced_assert("Bad args", (hostname != NULL) && (address != NULL));

    WICED_LINK_CHECK( WICED_STA_INTERFACE);

    return dns_client_hostname_lookup( hostname, address, timeout_ms );
}

wiced_result_t wiced_tcp_stream_init( wiced_tcp_stream_t* tcp_stream, wiced_tcp_socket_t* socket )
{
    tcp_stream->tx_packet = NULL;
    tcp_stream->rx_packet = NULL;
    tcp_stream->socket    = socket;
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_stream_deinit( wiced_tcp_stream_t* tcp_stream )
{
    /* Flush the TX */
    wiced_tcp_stream_flush( tcp_stream );

    /* Flush the RX */
    if ( tcp_stream->rx_packet != NULL )
    {
        wiced_packet_delete( tcp_stream->rx_packet );
    }
    tcp_stream->tx_packet = NULL;
    tcp_stream->rx_packet = NULL;
    tcp_stream->socket    = NULL;
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_stream_write( wiced_tcp_stream_t* tcp_stream, const void* data, uint32_t data_length )
{
    wiced_assert("Bad args", tcp_stream != NULL);

    WICED_LINK_CHECK( tcp_stream->socket->interface );

    while ( data_length != 0 )
    {
        uint16_t amount_to_write;

        /* Check if we don't have a packet */
        if ( tcp_stream->tx_packet == NULL )
        {
            wiced_result_t result;
            result = wiced_packet_create_tcp( tcp_stream->socket, (uint16_t) MIN( data_length, 0xffff ), &tcp_stream->tx_packet, &tcp_stream->tx_packet_data , &tcp_stream->tx_packet_space_available );
            if ( result != WICED_SUCCESS )
            {
                return result;
            }
        }

        /* Write data */
        amount_to_write = (uint16_t) MIN( data_length, tcp_stream->tx_packet_space_available );
        tcp_stream->tx_packet_data     = MEMCAT( tcp_stream->tx_packet_data, data, amount_to_write );

        /* Update variables */
        data_length                           = (uint16_t)(data_length - amount_to_write);
        tcp_stream->tx_packet_space_available = (uint16_t) ( tcp_stream->tx_packet_space_available - amount_to_write );
        data                                  = (void*)((uint32_t)data + amount_to_write);

        /* Check if the packet is full */
        if ( tcp_stream->tx_packet_space_available == 0 )
        {
            wiced_result_t result;

            /* Send the packet */
            wiced_packet_set_data_end( tcp_stream->tx_packet, (uint8_t*)tcp_stream->tx_packet_data );
            result = wiced_tcp_send_packet( tcp_stream->socket, tcp_stream->tx_packet );

            tcp_stream->tx_packet_data            = NULL;
            tcp_stream->tx_packet_space_available = 0;

            if ( result != WICED_SUCCESS )
            {
                wiced_packet_delete(tcp_stream->tx_packet);
                tcp_stream->tx_packet = NULL;
                return result;
            }

            tcp_stream->tx_packet = NULL;
        }
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_stream_read( wiced_tcp_stream_t* tcp_stream, void* buffer, uint16_t buffer_length, uint32_t timeout )
{
    WICED_LINK_CHECK( tcp_stream->socket->interface );

    while ( buffer_length != 0 )
    {
        uint16_t amount_to_read;
        uint16_t total_available;
        uint8_t* packet_data     = NULL;
        uint16_t space_available = 0;

        /* Check if we don't have a packet */
        if (tcp_stream->rx_packet == NULL)
        {
            wiced_result_t result;
            result = wiced_tcp_receive(tcp_stream->socket, &tcp_stream->rx_packet, timeout );
            if ( result != WICED_SUCCESS)
            {
                return result;
            }
        }

        wiced_packet_get_data(tcp_stream->rx_packet, 0, &packet_data, &space_available, &total_available);

        /* Read data */
        amount_to_read = MIN(buffer_length, space_available);
        buffer         = MEMCAT((uint8_t*)buffer, packet_data, amount_to_read);

        /* Update buffer length */
        buffer_length = (uint16_t)(buffer_length - amount_to_read);

        /* Check if we need a new packet */
        if ( amount_to_read == space_available )
        {
            wiced_packet_delete( tcp_stream->rx_packet );
            tcp_stream->rx_packet = NULL;
        }
        else
        {
            /* Otherwise update the start of the data for the next read request */
            wiced_packet_set_data_start(tcp_stream->rx_packet, packet_data + amount_to_read);
        }
    }
    return WICED_SUCCESS;
}


wiced_result_t wiced_tcp_stream_flush( wiced_tcp_stream_t* tcp_stream )
{
    wiced_result_t result = WICED_SUCCESS;

    wiced_assert("Bad args", tcp_stream != NULL);

    WICED_LINK_CHECK( tcp_stream->socket->interface );

    /* Check if there is a packet to send */
    if ( tcp_stream->tx_packet != NULL )
    {
        wiced_packet_set_data_end(tcp_stream->tx_packet, tcp_stream->tx_packet_data);
        result = wiced_tcp_send_packet( tcp_stream->socket, tcp_stream->tx_packet );

        tcp_stream->tx_packet_data            = NULL;
        tcp_stream->tx_packet_space_available = 0;
        if ( result != WICED_SUCCESS )
        {
            wiced_packet_delete( tcp_stream->tx_packet );
        }

        tcp_stream->tx_packet = NULL;
    }
    return result;
}


/******************************************************
 *            Static Function Definitions
 ******************************************************/

static wiced_result_t internal_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    err_t status;

    if ( socket->accept_handler == NULL )
    {
        netconn_set_recvtimeout( socket->conn_handler, (int)timeout );
        status = netconn_recv( socket->conn_handler, packet );
    }
    else
    {
        netconn_set_recvtimeout( socket->accept_handler, (int)timeout );
        status = netconn_recv( socket->accept_handler, packet );
    }

    return LWIP_TO_WICED_ERR( status );
}

static wiced_result_t internal_packet_create( wiced_packet_t** packet, uint16_t content_length, uint8_t** data, uint16_t* available_space )
{
    int i = 0;
    wiced_assert("Bad args", (packet != NULL) && (data != NULL) && (available_space != NULL));

    /* Try to allocate a packet, waiting up to WICED_ALLOCATE_PACKET_TIMEOUT milliseconds */
    for ( i = 0; i < WICED_ALLOCATE_PACKET_TIMEOUT; ++i )
    {
        *packet = netbuf_new( );
        if ( *packet != NULL )
        {
            *data = netbuf_alloc( *packet, content_length );
            if ( *data != NULL )
            {
                *available_space = content_length;
                return WICED_SUCCESS;
            }
            netbuf_delete( *packet );
            *packet = NULL;
            *available_space = 0;
        }

        wiced_rtos_delay_milliseconds( 1 );
    }

    *available_space = 0;
    return WICED_TIMEOUT;
}

static wiced_result_t internal_udp_send( struct netconn* handler, wiced_packet_t* packet, wiced_interface_t interface )
{
    err_t status;
    xSemaphoreTake( send_interface_mutex, portMAX_DELAY );

    /* set to default the requested interface */
    netif_set_default( &IP_HANDLE(interface));

    /* send a packet */
    packet->p->len = packet->p->tot_len;
    status = netconn_send( handler, packet );
    xSemaphoreGive( send_interface_mutex );
    return LWIP_TO_WICED_ERR( status );
}

static wiced_tcp_socket_t* internal_netconn_to_wiced_async_socket( struct netconn *conn )
{
    int i = 0;
    for ( i = 0; i < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++i )
    {
        if ( tcp_sockets_with_callbacks[i] != NULL )
        {
            if ( tcp_sockets_with_callbacks[i]->conn_handler == conn )
            {
                return tcp_sockets_with_callbacks[i];
            }
            else if ( tcp_sockets_with_callbacks[i]->conn_handler )
            {
                if ( tcp_sockets_with_callbacks[i]->conn_handler->pcb.tcp->local_port == conn->pcb.tcp->local_port )
                {
                    return tcp_sockets_with_callbacks[i];
                }
            }
        }
    }
    return NULL;
}

static void internal_async_socket_callback( struct netconn *conn, enum netconn_evt event, u16_t length )
{
    wiced_tcp_socket_t* socket;

    /* Verify that the socket is among the asynchronous callback sockets list */
    socket = internal_netconn_to_wiced_async_socket( conn );
    if (socket == NULL)
    {
        return;
    }

    switch ( event )
    {
        case NETCONN_EVT_RCVPLUS:
            if ( length > 0 )
            {
                wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, socket->callbacks[WICED_TCP_RECEIVE_CALLBACK_INDEX], socket );
            }
            else
            {
                /* Logic of events:
                 * Disconnect - if we receive length zero for an existing accepted connection
                 * Connect    - if we receive length zero for a non-existing connection */
                if ( ( socket->accept_handler != NULL ) && ( conn == socket->accept_handler ) )
                {
                    wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, socket->callbacks[WICED_TCP_DISCONNECT_CALLBACK_INDEX], socket );
                }
                else
                {
                    wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, socket->callbacks[WICED_TCP_CONNECT_CALLBACK_INDEX], socket );
                }
            }
            break;

        case NETCONN_EVT_ERROR:
            /*  Connection was reseted externally, inform about disconnect */
            wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, socket->callbacks[WICED_TCP_DISCONNECT_CALLBACK_INDEX], socket );
            break;

        case NETCONN_EVT_RCVMINUS:
        case NETCONN_EVT_SENDPLUS:
        case NETCONN_EVT_SENDMINUS:
        default:
            break;
    }
}
