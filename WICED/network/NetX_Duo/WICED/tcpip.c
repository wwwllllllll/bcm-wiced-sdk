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
 * NetX_Duo TCP/IP library
 */

#include "wiced.h"
#include "wiced_network.h"
#include "nx_api.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_tcpip.h"
#include "wwd_assert.h"
#include "wwd_crypto.h"
#include "dns.h"
#ifndef WICED_DISABLE_TLS
#include "wiced_tls.h"
#include "tls_host_api.h"
#endif /* ifndef WICED_DISABLE_TLS */

/******************************************************
 *                      Macros
 ******************************************************/

#define MIN(x,y)  ((x) < (y) ? (x) : (y))

#define NX_TIMEOUT(timeout_ms)   ((timeout_ms != WICED_NEVER_TIMEOUT) ? ((ULONG)(timeout_ms * SYSTICK_FREQUENCY / 1000)) : NX_WAIT_FOREVER )

#define WICED_LINK_CHECK( ip_handle )  { if ( (ip_handle)->nx_ip_driver_link_up == 0 ){ return WICED_NOTUP; }}

/******************************************************
 *                    Constants
 ******************************************************/

#ifndef WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS
#define WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS    (5)
#endif

#ifndef WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS
#define WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS (WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS)
#endif

#ifndef WICED_TCP_TX_RETRIES
#define WICED_TCP_TX_RETRIES       WICED_DEFAULT_TCP_TX_RETRIES
#endif

#ifndef WICED_TCP_TX_DEPTH_QUEUE
#define WICED_TCP_TX_DEPTH_QUEUE    WICED_DEFAULT_TCP_TX_DEPTH_QUEUE
#endif

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef VOID (*tcp_listen_callback_t)(NX_TCP_SOCKET *socket_ptr, UINT port);

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void           internal_tcp_socket_disconnect_callback      ( struct NX_TCP_SOCKET_STRUCT *socket_ptr );
static void           internal_tcp_socket_receive_callback         ( struct NX_TCP_SOCKET_STRUCT *socket_ptr );
static void           internal_udp_socket_receive_callback         ( struct NX_UDP_SOCKET_STRUCT *socket_ptr );
static void           internal_tcp_listen_callback                 ( NX_TCP_SOCKET *socket_ptr, UINT port );
static wiced_result_t internal_wiced_tcp_server_listen             ( wiced_tcp_server_t* tcp_server);
static void           internal_common_tcp_callback                 ( struct NX_TCP_SOCKET_STRUCT *socket_ptr, wiced_tcp_callback_index_t callback );
static uint32_t       str_to_ipv4                                  ( const char* arg );

/* TLS helper function to do TCP without involving TLS context */
wiced_result_t network_tcp_receive      ( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout );
wiced_result_t network_tcp_send_packet  ( wiced_tcp_socket_t* socket, wiced_packet_t*  packet );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_tcp_socket_t* tcp_sockets_with_callbacks[WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS];
static wiced_udp_socket_t* udp_sockets_with_callbacks[WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS];

static const wiced_result_t netx_duo_returns[] =
{
    [NX_SUCCESS             ] = WICED_TCPIP_SUCCESS,
    [NX_NO_PACKET           ] = WICED_TCPIP_TIMEOUT,
    [NX_UNDERFLOW           ] = WICED_TCPIP_ERROR,
    [NX_OVERFLOW            ] = WICED_TCPIP_ERROR,
    [NX_NO_MAPPING          ] = WICED_TCPIP_ERROR,
    [NX_DELETED             ] = WICED_TCPIP_ERROR,
    [NX_POOL_ERROR          ] = WICED_TCPIP_ERROR,
    [NX_PTR_ERROR           ] = WICED_TCPIP_ERROR,
    [NX_WAIT_ERROR          ] = WICED_TCPIP_ERROR,
    [NX_SIZE_ERROR          ] = WICED_TCPIP_ERROR,
    [NX_OPTION_ERROR        ] = WICED_TCPIP_ERROR,
    [NX_DELETE_ERROR        ] = WICED_TCPIP_ERROR,
    [NX_CALLER_ERROR        ] = WICED_TCPIP_ERROR,
    [NX_INVALID_PACKET      ] = WICED_TCPIP_INVALID_PACKET,
    [NX_INVALID_SOCKET      ] = WICED_TCPIP_INVALID_SOCKET,
    [NX_NOT_ENABLED         ] = WICED_TCPIP_ERROR,
    [NX_ALREADY_ENABLED     ] = WICED_TCPIP_ERROR,
    [NX_ENTRY_NOT_FOUND     ] = WICED_TCPIP_ERROR,
    [NX_NO_MORE_ENTRIES     ] = WICED_TCPIP_ERROR,
    [NX_ARP_TIMER_ERROR     ] = WICED_TCPIP_ERROR,
    [NX_RESERVED_CODE0      ] = WICED_TCPIP_ERROR,
    [NX_WAIT_ABORTED        ] = WICED_TCPIP_WAIT_ABORTED,
    [NX_IP_INTERNAL_ERROR   ] = WICED_TCPIP_ERROR,
    [NX_IP_ADDRESS_ERROR    ] = WICED_TCPIP_ERROR,
    [NX_ALREADY_BOUND       ] = WICED_TCPIP_ERROR,
    [NX_PORT_UNAVAILABLE    ] = WICED_TCPIP_PORT_UNAVAILABLE,
    [NX_NOT_BOUND           ] = WICED_TCPIP_ERROR,
    [NX_RESERVED_CODE1      ] = WICED_TCPIP_ERROR,
    [NX_SOCKET_UNBOUND      ] = WICED_TCPIP_ERROR,
    [NX_NOT_CREATED         ] = WICED_TCPIP_ERROR,
    [NX_SOCKETS_BOUND       ] = WICED_TCPIP_ERROR,
    [NX_NO_RESPONSE         ] = WICED_TCPIP_ERROR,
    [NX_POOL_DELETED        ] = WICED_TCPIP_ERROR,
    [NX_ALREADY_RELEASED    ] = WICED_TCPIP_ERROR,
    [NX_RESERVED_CODE2      ] = WICED_TCPIP_ERROR,
    [NX_MAX_LISTEN          ] = WICED_TCPIP_ERROR,
    [NX_DUPLICATE_LISTEN    ] = WICED_TCPIP_ERROR,
    [NX_NOT_CLOSED          ] = WICED_TCPIP_ERROR,
    [NX_NOT_LISTEN_STATE    ] = WICED_TCPIP_ERROR,
    [NX_IN_PROGRESS         ] = WICED_TCPIP_IN_PROGRESS,
    [NX_NOT_CONNECTED       ] = WICED_TCPIP_ERROR,
    [NX_WINDOW_OVERFLOW     ] = WICED_TCPIP_ERROR,
    [NX_ALREADY_SUSPENDED   ] = WICED_TCPIP_ERROR,
    [NX_DISCONNECT_FAILED   ] = WICED_TCPIP_ERROR,
    [NX_STILL_BOUND         ] = WICED_TCPIP_ERROR,
    [NX_NOT_SUCCESSFUL      ] = WICED_TCPIP_ERROR,
    [NX_UNHANDLED_COMMAND   ] = WICED_TCPIP_ERROR,
    [NX_NO_FREE_PORTS       ] = WICED_TCPIP_ERROR,
    [NX_INVALID_PORT        ] = WICED_TCPIP_ERROR,
    [NX_INVALID_RELISTEN    ] = WICED_TCPIP_ERROR,
    [NX_CONNECTION_PENDING  ] = WICED_TCPIP_IN_PROGRESS,
    [NX_TX_QUEUE_DEPTH      ] = WICED_TCPIP_ERROR,
    [NX_NOT_IMPLEMENTED     ] = WICED_TCPIP_ERROR,
    [NX_NOT_SUPPORTED       ] = WICED_TCPIP_ERROR,
    [NX_INVALID_INTERFACE   ] = WICED_TCPIP_ERROR,
    [NX_INVALID_PARAMETERS  ] = WICED_TCPIP_ERROR,
    [NX_NOT_FOUND           ] = WICED_TCPIP_ERROR,
    [NX_CANNOT_START        ] = WICED_TCPIP_ERROR,
    [NX_NO_INTERFACE_ADDRESS] = WICED_TCPIP_ERROR,
    [NX_INVALID_MTU_DATA    ] = WICED_TCPIP_ERROR,
    [NX_DUPLICATED_ENTRY    ] = WICED_TCPIP_ERROR,
    [NX_PACKET_OFFSET_ERROR ] = WICED_TCPIP_ERROR,
    [NX_OPTION_HEADER_ERROR ] = WICED_TCPIP_ERROR,
    [NX_PARAMETER_ERROR     ] = WICED_TCPIP_BADARG,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_tcp_server_start( wiced_tcp_server_t* tcp_server, wiced_interface_t interface, uint16_t port, wiced_socket_callback_t connect_callback, wiced_socket_callback_t receive_callback, wiced_socket_callback_t disconnect_callback)
{
    int            socket_index;
    wiced_result_t status;

    tcp_server->interface = interface;
    tcp_server->port      = port;

    for (socket_index=0; socket_index < WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS; socket_index++ )
    {
        WICED_VERIFY ( wiced_tcp_create_socket ( &tcp_server->socket[socket_index], interface ) );

        WICED_VERIFY ( wiced_tcp_register_callbacks( &tcp_server->socket[socket_index], connect_callback, receive_callback, disconnect_callback ) );
    }

    /* start server listen */
    status = nx_tcp_server_socket_listen( tcp_server->socket[0].socket.nx_tcp_socket_ip_ptr, tcp_server->port, &tcp_server->socket[0].socket, WICED_DEFAULT_TCP_LISTEN_QUEUE_SIZE, internal_tcp_listen_callback );

    return status;
}

wiced_result_t wiced_tcp_server_accept( wiced_tcp_server_t* tcp_server, wiced_tcp_socket_t* socket )
{
    wiced_result_t result;

    result = wiced_tcp_accept( socket );

    internal_wiced_tcp_server_listen( tcp_server );

    return result;
}

wiced_result_t wiced_tcp_server_disconnect_socket( wiced_tcp_server_t* tcp_server, wiced_tcp_socket_t* socket)
{
    UINT result;

    if ( socket->socket.nx_tcp_socket_state <= NX_TCP_CLOSE_WAIT )
    {
        nx_tcp_socket_disconnect( &socket->socket, NX_NO_WAIT );
    }

    nx_tcp_server_socket_unaccept( &socket->socket );

    result = internal_wiced_tcp_server_listen( tcp_server );

    return result;
}

wiced_result_t wiced_tcp_server_stop( wiced_tcp_server_t* tcp_server )
{
    int i;

    for ( i = 0; i < WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS; i++ )
    {
        wiced_tcp_delete_socket( &tcp_server->socket[ i ] );
        tcp_sockets_with_callbacks[ i ] = NULL;
    }
    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_create_socket( wiced_tcp_socket_t* socket, wiced_interface_t interface )
{
    UINT result;

    memset( socket, 0, sizeof(wiced_tcp_socket_t) );
    result = nx_tcp_socket_create( &IP_HANDLE(interface), &socket->socket, (CHAR*)"", NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, WICED_TCP_WINDOW_SIZE, NX_NULL, NX_NULL );
    wiced_assert("Error creating socket", result == NX_SUCCESS);
    nx_tcp_socket_transmit_configure(&socket->socket, WICED_TCP_TX_DEPTH_QUEUE, WICED_TCP_SEND_TIMEOUT/WICED_TCP_TX_RETRIES, WICED_TCP_TX_RETRIES, 0);

    return netx_duo_returns[result];
}

wiced_result_t wiced_tcp_accept( wiced_tcp_socket_t* socket )
{
    UINT result;

    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    if ( socket->callbacks[WICED_TCP_CONNECT_CALLBACK_INDEX] == NULL )
    {
        if ( socket->socket.nx_tcp_socket_state != NX_TCP_LISTEN_STATE )
        {
            nx_tcp_socket_disconnect( &socket->socket, NX_TIMEOUT(WICED_TCP_DISCONNECT_TIMEOUT) );
        }

        nx_tcp_server_socket_unaccept( &socket->socket );
        nx_tcp_server_socket_relisten( socket->socket.nx_tcp_socket_ip_ptr, socket->socket.nx_tcp_socket_port, &socket->socket );

#ifndef WICED_DISABLE_TLS
        if ( socket->tls_context != NULL )
        {
            wiced_tls_reset_context( socket->tls_context );
        }
#endif /* ifndef WICED_DISABLE_TLS */

        result = nx_tcp_server_socket_accept( &socket->socket, NX_TIMEOUT(WICED_TCP_ACCEPT_TIMEOUT) );
        if ( ( result == NX_NOT_CONNECTED ) || ( result == NX_WAIT_ABORTED ) )
        {
            return netx_duo_returns[result];
        }
        else if ( result != NX_SUCCESS )
        {
            WPRINT_LIB_INFO( ( "Error accepting connection\n" ) );
            return netx_duo_returns[result];
        }

#ifndef WICED_DISABLE_TLS
        if ( socket->tls_context != NULL )
        {
            result = wiced_tcp_start_tls( socket, WICED_TLS_AS_SERVER, WICED_TLS_DEFAULT_VERIFICATION );
            if ( result != WICED_SUCCESS )
            {
                WPRINT_LIB_INFO( ( "Error starting TLS connection\n" ) );
                return netx_duo_returns[result];
            }
        }
#endif /* ifndef WICED_DISABLE_TLS */
    }
    else
    {
        if ( ( socket->socket.nx_tcp_socket_state == NX_TCP_LISTEN_STATE ) ||( socket->socket.nx_tcp_socket_state == NX_TCP_SYN_RECEIVED ) )
        {
        result = nx_tcp_server_socket_accept( &socket->socket, NX_NO_WAIT );
        }
        else
        {
            return WICED_ERROR;
        }
        return netx_duo_returns[result];
    }

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_connect( wiced_tcp_socket_t* socket, const wiced_ip_address_t* address, uint16_t port, uint32_t timeout_ms )
{
    UINT result;
    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    /* Check if socket is not yet bound to a local port */
    if ( !socket->socket.nx_tcp_socket_bound_next )
    {
        result = nx_tcp_client_socket_bind( &socket->socket, NX_ANY_PORT, NX_TIMEOUT(WICED_TCP_BIND_TIMEOUT) );
        if ( result != NX_SUCCESS )
        {
            wiced_assert("Error binding to port", 0 != 0 );
            return netx_duo_returns[result];
        }
    }

    result = nxd_tcp_client_socket_connect( &socket->socket, (NXD_ADDRESS*) address, port, NX_TIMEOUT(timeout_ms) );
    if ( result != NX_SUCCESS )
    {
        wiced_assert("Error connecting to TCP destination", 0 != 0 );
        return netx_duo_returns[result];
    }

#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        result = wiced_tcp_start_tls(socket, WICED_TLS_AS_CLIENT, WICED_TLS_DEFAULT_VERIFICATION);
        if ( result != WICED_SUCCESS)
        {
            nx_tcp_socket_disconnect( &socket->socket, NX_TIMEOUT(WICED_TCP_DISCONNECT_TIMEOUT) );
            return netx_duo_returns[result];
        }
    }
#endif /* ifndef WICED_DISABLE_TLS */

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_bind( wiced_tcp_socket_t* socket, uint16_t port )
{
    UINT result;
    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    result = nx_tcp_client_socket_bind( &socket->socket, port, NX_TIMEOUT(WICED_TCP_BIND_TIMEOUT) );
    return netx_duo_returns[result];
}

wiced_result_t wiced_tcp_register_callbacks( wiced_tcp_socket_t* socket, wiced_socket_callback_t connect_callback, wiced_socket_callback_t receive_callback, wiced_socket_callback_t disconnect_callback )
{
    int a;

    /* Add the socket to the list of sockets with callbacks */
    for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
    {
        if ( tcp_sockets_with_callbacks[a] == socket || tcp_sockets_with_callbacks[a] == NULL )
        {
            tcp_sockets_with_callbacks[a] = socket;
            break;
        }
    }

    if ( a == WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS )
    {
        return WICED_ERROR;
    }

    if ( disconnect_callback != NULL )
    {
        socket->callbacks[WICED_TCP_DISCONNECT_CALLBACK_INDEX] = disconnect_callback;
        socket->socket.nx_tcp_disconnect_callback =  internal_tcp_socket_disconnect_callback;
    }

    if ( receive_callback != NULL )
    {
        socket->callbacks[WICED_TCP_RECEIVE_CALLBACK_INDEX] = receive_callback;
        nx_tcp_socket_receive_notify( &socket->socket, internal_tcp_socket_receive_callback );
    }

    if ( connect_callback != NULL )
    {
        socket->callbacks[WICED_TCP_CONNECT_CALLBACK_INDEX] = connect_callback;
        /* Note: Link to NetX_Duo callback mechanism is configured in wiced_tcp_listen() */
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_tcp_unregister_callbacks( wiced_tcp_socket_t* socket )
{
    int a;
    for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
    {
        if ( tcp_sockets_with_callbacks[ a ] == socket )
        {
            tcp_sockets_with_callbacks[ a ] = NULL;
        }
    }
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_delete_socket( wiced_tcp_socket_t* socket )
{
#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        wiced_tls_close_notify( socket );

        wiced_tls_deinit_context( socket->tls_context );

        if ( socket->context_malloced == WICED_TRUE )
        {
            free( socket->tls_context );
            socket->tls_context = NULL;
            socket->context_malloced = WICED_FALSE;
        }
    }
#endif /* ifndef WICED_DISABLE_TLS */

    nx_tcp_socket_disconnect     ( &socket->socket, NX_TIMEOUT(WICED_TCP_DISCONNECT_TIMEOUT));
    nx_tcp_server_socket_unaccept( &socket->socket );
    nx_tcp_server_socket_unlisten( socket->socket.nx_tcp_socket_ip_ptr, socket->socket.nx_tcp_socket_port );
    nx_tcp_client_socket_unbind  ( &socket->socket );
    nx_tcp_socket_delete         ( &socket->socket );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_listen( wiced_tcp_socket_t* socket, uint16_t port )
{
    UINT result;
    tcp_listen_callback_t listen_callback = NULL;
    struct NX_TCP_LISTEN_STRUCT* listen_ptr;
    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    /* Check if there is already another socket listening on the port */
    listen_ptr = socket->socket.nx_tcp_socket_ip_ptr->nx_ip_tcp_active_listen_requests;
    if ( listen_ptr != NULL )
    {
        /* Search the active listen requests for this port. */
        do
        {
            /* Determine if there is another listen request for the same port. */
            if ( listen_ptr->nx_tcp_listen_port == port )
            {
                /* Do a re-listen instead of a listen */
                result = nx_tcp_server_socket_relisten( socket->socket.nx_tcp_socket_ip_ptr, port, &socket->socket );
                return netx_duo_returns[result];
            }
            listen_ptr = listen_ptr->nx_tcp_listen_next;
        } while ( listen_ptr != socket->socket.nx_tcp_socket_ip_ptr->nx_ip_tcp_active_listen_requests);
    }

    /* Check if this socket has an asynchronous connect callback */
    if (socket->callbacks[WICED_TCP_CONNECT_CALLBACK_INDEX] != NULL)
    {
        listen_callback = internal_tcp_listen_callback;
    }

    if (socket->socket.nx_tcp_socket_state != NX_TCP_CLOSED)
    {
        nx_tcp_server_socket_unaccept( &socket->socket );
        result = nx_tcp_server_socket_relisten( socket->socket.nx_tcp_socket_ip_ptr, socket->socket.nx_tcp_socket_port, &socket->socket );
    }
    else
    {
        result = nx_tcp_server_socket_listen( socket->socket.nx_tcp_socket_ip_ptr, port, &socket->socket, WICED_DEFAULT_TCP_LISTEN_QUEUE_SIZE, listen_callback );
    }
    return netx_duo_returns[result];
}

wiced_result_t wiced_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t* packet )
{
#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        wiced_tls_encrypt_packet( &socket->tls_context->context, packet );
    }
#endif /* ifndef WICED_DISABLE_TLS */

    return network_tcp_send_packet( socket, packet );
}

wiced_result_t network_tcp_send_packet( wiced_tcp_socket_t* socket, wiced_packet_t* packet )
{
    UINT result;

    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    result = nx_tcp_socket_send(&socket->socket, packet, NX_TIMEOUT(WICED_TCP_SEND_TIMEOUT) );
    return netx_duo_returns[result];
}

wiced_result_t wiced_tcp_send_buffer( wiced_tcp_socket_t* socket, const void* buffer, uint16_t buffer_length )
{
    wiced_result_t result;
    NX_PACKET* packet = NULL;
    uint8_t* data_ptr = (uint8_t*)buffer;
    uint8_t* packet_data_ptr;
    uint16_t available_space;

    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    while ( buffer_length != 0 )
    {
        uint16_t data_to_write;

        result = wiced_packet_create_tcp( socket, buffer_length, &packet, &packet_data_ptr, &available_space );
        if ( result != WICED_TCPIP_SUCCESS )
        {
            return result;
        }

        /* Write data */
        data_to_write = MIN(buffer_length, available_space);
        packet_data_ptr = MEMCAT(packet_data_ptr, data_ptr, data_to_write);

        wiced_packet_set_data_end( packet, packet_data_ptr );

        /* Update variables */
        data_ptr       += data_to_write;
        buffer_length   = (uint16_t)(buffer_length - data_to_write);
        available_space = (uint16_t)(available_space - data_to_write);

        result = wiced_tcp_send_packet( socket, packet );
        if ( result != WICED_TCPIP_SUCCESS )
        {
            wiced_packet_delete( packet );
            return result;
        }
    }

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        return wiced_tls_receive_packet( socket, packet, timeout );
    }
    else
#endif /* ifndef WICED_DISABLE_TLS */
    {
        return network_tcp_receive( socket, packet, timeout );
    }
}

wiced_result_t network_tcp_receive( wiced_tcp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    UINT result;

    result = nx_tcp_socket_receive( &socket->socket, packet, NX_TIMEOUT(timeout) );

    return netx_duo_returns[result];
}

wiced_result_t wiced_tcp_disconnect( wiced_tcp_socket_t* socket )
{
    UINT result;
    WICED_LINK_CHECK( socket->socket.nx_tcp_socket_ip_ptr );

    if ( socket->tls_context != NULL )
    {
        wiced_tls_close_notify( socket );
    }

    nx_tcp_socket_disconnect( &socket->socket, NX_TIMEOUT(WICED_TCP_DISCONNECT_TIMEOUT ) );
    if ( socket->socket.nx_tcp_socket_client_type == NX_TRUE)
    {
    /* Un-bind the socket, so the TCP port becomes available for other sockets which are suspended on bind requests. This will also flush the receive queue of the socket */
    /* We ignore the return of the unbind as there isn't much we can do */
        result = nx_tcp_client_socket_unbind( &socket->socket );
    }
    else
    {
        result = nx_tcp_server_socket_unaccept( &socket->socket );
    }

    return netx_duo_returns[result];
}

wiced_result_t wiced_packet_create_tcp( wiced_tcp_socket_t* socket, uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    UINT     result;
    uint16_t maximum_segment_size = (uint16_t)MIN(socket->socket.nx_tcp_socket_mss, socket->socket.nx_tcp_socket_connect_mss);

    UNUSED_PARAMETER( content_length );
    result = nx_packet_allocate( &wiced_packet_pools[0], packet, NX_TCP_PACKET, NX_TIMEOUT(WICED_ALLOCATE_PACKET_TIMEOUT) );
    if ( result != NX_SUCCESS )
    {
        *packet = NULL;
        *data   = NULL;
        *available_space = 0;
        wiced_assert( "Packet allocation error", result != NX_NO_PACKET );
        return netx_duo_returns[result];;
    }

#ifndef WICED_DISABLE_TLS
    if ( socket->tls_context != NULL )
    {
        uint16_t header_space;
        uint16_t footer_pad_space;

        wiced_tls_calculate_overhead( &socket->tls_context->context, content_length, &header_space, &footer_pad_space );

        ( *packet )->nx_packet_prepend_ptr += header_space;

        *available_space  = (uint16_t)(MIN((*packet)->nx_packet_data_end - (*packet)->nx_packet_prepend_ptr, maximum_segment_size) - footer_pad_space);
    }
    else
#endif /* ifndef WICED_DISABLE_TLS */
    {
        *available_space = (uint16_t) MIN((*packet)->nx_packet_data_end - (*packet)->nx_packet_prepend_ptr, maximum_segment_size);
    }

    ( *packet )->nx_packet_append_ptr = ( *packet )->nx_packet_prepend_ptr;
    *data = ( *packet )->nx_packet_prepend_ptr;
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_init( wiced_tcp_stream_t* tcp_stream, wiced_tcp_socket_t* socket )
{
    tcp_stream->tx_packet                 = NULL;
    tcp_stream->rx_packet                 = NULL;
    tcp_stream->socket                    = socket;
    tcp_stream->use_custom_tcp_stream     = WICED_FALSE;
    tcp_stream->tcp_stream_write_callback = NULL;
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_deinit( wiced_tcp_stream_t* tcp_stream )
{
    /* Flush the TX */
    wiced_tcp_stream_flush( tcp_stream );

    /* Flush the RX */
    if ( tcp_stream->rx_packet != NULL )
    {
        wiced_packet_delete( tcp_stream->rx_packet );
        tcp_stream->rx_packet = NULL;
    }
    tcp_stream->tx_packet = NULL;
    tcp_stream->rx_packet = NULL;
    tcp_stream->socket    = NULL;
    tcp_stream->use_custom_tcp_stream = WICED_FALSE;
    tcp_stream->tcp_stream_write_callback = NULL;
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_write( wiced_tcp_stream_t* tcp_stream, const void* data, uint32_t data_length )
{
    wiced_result_t result;
    wiced_assert("Bad args", tcp_stream != NULL);

    WICED_LINK_CHECK( tcp_stream->socket->socket.nx_tcp_socket_ip_ptr );

    if( tcp_stream->use_custom_tcp_stream )
    {
        tcp_stream->tcp_stream_write_callback( tcp_stream, data, data_length );
    }
    else
    {
        while ( data_length != 0 )
        {
            uint16_t amount_to_write;
            /* Check if we don't have a packet */
            if ( tcp_stream->tx_packet == NULL )
            {
                result = wiced_packet_create_tcp( tcp_stream->socket, (uint16_t) MIN( data_length, 0xffff ), &tcp_stream->tx_packet, &tcp_stream->tx_packet_data , &tcp_stream->tx_packet_space_available );
                if ( result != WICED_TCPIP_SUCCESS )
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
                /* Send the packet */
                wiced_packet_set_data_end( tcp_stream->tx_packet, (uint8_t*)tcp_stream->tx_packet_data );
                result = wiced_tcp_send_packet( tcp_stream->socket, tcp_stream->tx_packet );

                tcp_stream->tx_packet_data            = NULL;
                tcp_stream->tx_packet_space_available = 0;

                if ( result != WICED_TCPIP_SUCCESS )
                {
                    wiced_packet_delete( tcp_stream->tx_packet );
                    tcp_stream->tx_packet = NULL;
                    return result;
                }

                tcp_stream->tx_packet = NULL;
            }
        }
    }
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_read( wiced_tcp_stream_t* tcp_stream, void* buffer, uint16_t buffer_length, uint32_t timeout )
{
    WICED_LINK_CHECK( tcp_stream->socket->socket.nx_tcp_socket_ip_ptr );

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
            if ( result != WICED_TCPIP_SUCCESS)
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
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_stream_flush( wiced_tcp_stream_t* tcp_stream )
{
    wiced_result_t result = WICED_TCPIP_SUCCESS;

    wiced_assert("Bad args", tcp_stream != NULL);

    WICED_LINK_CHECK( tcp_stream->socket->socket.nx_tcp_socket_ip_ptr );

    /* Check if there is a packet to send */
    if ( tcp_stream->tx_packet != NULL )
    {
        wiced_packet_set_data_end(tcp_stream->tx_packet, tcp_stream->tx_packet_data);
        result = wiced_tcp_send_packet( tcp_stream->socket, tcp_stream->tx_packet );

        tcp_stream->tx_packet_data            = NULL;
        tcp_stream->tx_packet_space_available = 0;
        if ( result != WICED_TCPIP_SUCCESS )
        {
            wiced_packet_delete( tcp_stream->tx_packet );
        }

        tcp_stream->tx_packet = NULL;
    }
    return result;
}

wiced_result_t wiced_tcp_enable_keepalive( wiced_tcp_socket_t* socket, uint16_t interval, uint16_t probes, uint16_t time )
{
    UNUSED_PARAMETER( socket );
    UNUSED_PARAMETER( interval );
    UNUSED_PARAMETER( probes );
    UNUSED_PARAMETER( time );

    return WICED_TCPIP_UNSUPPORTED;
}

wiced_result_t wiced_packet_create_udp( wiced_udp_socket_t* socket, uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    UINT result;
    UNUSED_PARAMETER( content_length );
    UNUSED_PARAMETER( socket );

    result = nx_packet_allocate( &wiced_packet_pools[0], packet, NX_UDP_PACKET, NX_TIMEOUT(WICED_ALLOCATE_PACKET_TIMEOUT) );
    if ( result != NX_SUCCESS )
    {
        *packet = NULL;
        *data   = NULL;
        *available_space = 0;
        wiced_assert( "Packet allocation error", result == NX_NO_PACKET );
        return netx_duo_returns[result];
    }

    *data = ( *packet )->nx_packet_prepend_ptr;
    *available_space = (uint16_t)((*packet)->nx_packet_data_end - (*packet)->nx_packet_prepend_ptr);
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_create( uint16_t content_length, wiced_packet_t** packet, uint8_t** data, uint16_t* available_space )
{
    UINT result;
    UNUSED_PARAMETER( content_length );

    result = nx_packet_allocate( &wiced_packet_pools[0], packet, NX_IP_PACKET, NX_TIMEOUT(WICED_ALLOCATE_PACKET_TIMEOUT) );
    if ( result != NX_SUCCESS )
    {
        *packet = NULL;
        *data   = NULL;
        *available_space = 0;
        wiced_assert( "Packet allocation error", result == NX_NO_PACKET );
        return netx_duo_returns[result];
    }

    *data = ( *packet )->nx_packet_prepend_ptr;
    *available_space = (uint16_t)((*packet)->nx_packet_data_end - (*packet)->nx_packet_prepend_ptr);
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_delete( wiced_packet_t* packet )
{
    nx_packet_release( packet );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_set_data_end( wiced_packet_t* packet, uint8_t* data_end )
{
    wiced_assert("Bad packet end\n", (data_end >= packet->nx_packet_prepend_ptr) && (data_end <= packet->nx_packet_data_end));
    packet->nx_packet_append_ptr = data_end;
    packet->nx_packet_length     = (ULONG)(packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr);
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_packet_set_data_start( wiced_packet_t* packet, uint8_t* data_start )
{
    wiced_assert("Bad packet end\n", (data_start >= packet->nx_packet_data_start) && (data_start <= packet->nx_packet_data_end));
    packet->nx_packet_prepend_ptr = data_start;
    packet->nx_packet_length      = (ULONG)(packet->nx_packet_append_ptr - packet->nx_packet_prepend_ptr);
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_udp_packet_get_info( wiced_packet_t* packet, wiced_ip_address_t* address, uint16_t* port )
{
    UINT result;
    UINT p;

    result = nxd_udp_source_extract( packet, (NXD_ADDRESS*) address, &p );
    if ( result == NX_SUCCESS )
    {
        *port = (uint16_t)p;
    }
    return netx_duo_returns[result];
}

wiced_result_t wiced_packet_get_data( wiced_packet_t* packet, uint16_t offset, uint8_t** data, uint16_t* fragment_available_data_length, uint16_t *total_available_data_length )
{
    NX_PACKET* iter = packet;

    wiced_assert( "Bad args", (packet != NULL) && (data != NULL) && (fragment_available_data_length != NULL) && (total_available_data_length != NULL) );

    while ( iter != NULL )
    {
        if ( iter->nx_packet_length == 0 )
        {
            *data                           = NULL;
            *fragment_available_data_length = 0;
            *total_available_data_length    = 0;
            return WICED_TCPIP_SUCCESS;
        }
        else if ( offset < iter->nx_packet_length )
        {
            *data = iter->nx_packet_prepend_ptr + offset;
            *fragment_available_data_length = (uint16_t) ( iter->nx_packet_append_ptr - *data );
            *total_available_data_length    = (uint16_t) ( packet->nx_packet_length - offset );
            return WICED_TCPIP_SUCCESS;
        }
        else
        {
            offset = (uint16_t) ( offset - iter->nx_packet_length );
        }
        iter = iter->nx_packet_next;
    }

    return WICED_TCPIP_ERROR;
}

wiced_result_t wiced_ip_get_ipv4_address( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    ULONG temp;
    ipv4_address->version = WICED_IPV4;
    nx_ip_address_get( &IP_HANDLE(interface), (ULONG*)&ipv4_address->ip.v4, &temp );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_ip_get_ipv6_address( wiced_interface_t interface, wiced_ip_address_t* ipv6_address, wiced_ipv6_address_type_t address_type )
{

    ULONG    ipv6_prefix;
    UINT     ipv6_interface_index;
    uint32_t i;
    UINT     result;

    for ( i = 0; i < NX_MAX_IPV6_ADDRESSES; i++ )
    {
        if ( IP_HANDLE(interface).nx_ipv6_address[i].nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID )
        {
            NXD_ADDRESS ip_address;

            result = nxd_ipv6_address_get( &IP_HANDLE(interface), i, &ip_address, &ipv6_prefix, &ipv6_interface_index );
            if ( result != NX_SUCCESS )
            {
                return netx_duo_returns[result];
            }

            if ( address_type == IPv6_GLOBAL_ADDRESS )
            {
                if ( ( ip_address.nxd_ip_address.v6[0] & 0xf0000000 ) == 0x30000000 || ( ip_address.nxd_ip_address.v6[0] & 0xf0000000 ) == 0x20000000 )
                {
                    memcpy(ipv6_address, &ip_address, sizeof(NXD_ADDRESS));
                    return WICED_TCPIP_SUCCESS;
                }
            }
            else if ( address_type == IPv6_LINK_LOCAL_ADDRESS )
            {
                if ( ( ip_address.nxd_ip_address.v6[0] & 0xffff0000 ) == 0xfe800000 )
                {
                    memcpy(ipv6_address, &ip_address, sizeof(NXD_ADDRESS));
                    return WICED_TCPIP_SUCCESS;
                }
            }
            else
            {
                wiced_assert("Wrong address type", 0!=0 );
                return WICED_TCPIP_ERROR;
            }
        }
    }

    return WICED_TCPIP_ERROR;
}

wiced_result_t wiced_udp_create_socket( wiced_udp_socket_t* socket, uint16_t port, wiced_interface_t interface )
{
    UINT result;

    WICED_LINK_CHECK( &IP_HANDLE(interface) );

    result = nx_udp_socket_create(&IP_HANDLE(interface), &socket->socket, (char*)"WICEDsock", 0, NX_DONT_FRAGMENT, 255, 5);
    if ( result != NX_SUCCESS )
    {
        wiced_assert( "Socket creation error", 0 != 0 );
        return netx_duo_returns[result];
    }

    result = nx_udp_socket_bind( &socket->socket, ( port == WICED_ANY_PORT )? NX_ANY_PORT : (UINT) port, NX_TIMEOUT(WICED_UDP_BIND_TIMEOUT) );
    if ( result != NX_SUCCESS )
    {
        wiced_assert( "Socket bind error", 0 != 0 );
        nx_udp_socket_delete( &socket->socket );
        return netx_duo_returns[result];
    }
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_udp_send( wiced_udp_socket_t* socket, const wiced_ip_address_t* address, uint16_t port, wiced_packet_t* packet )
{
    UINT result;

    WICED_LINK_CHECK( socket->socket.nx_udp_socket_ip_ptr );

    result = nxd_udp_socket_send( &socket->socket, packet, (NXD_ADDRESS*) address, port );
    wiced_assert( "UDP send error", result == NX_SUCCESS );
    return netx_duo_returns[result];
}

wiced_result_t wiced_udp_reply( wiced_udp_socket_t* socket, wiced_packet_t* in_packet, wiced_packet_t* out_packet )
{
    wiced_ip_address_t source_ip;
    UINT               source_port;

    WICED_LINK_CHECK( socket->socket.nx_udp_socket_ip_ptr );

    nx_udp_source_extract( (NX_PACKET*) in_packet, (ULONG*) &source_ip.ip.v4, &source_port );
    return wiced_udp_send( socket, &source_ip, (uint16_t) source_port, out_packet );
}

wiced_result_t wiced_udp_receive( wiced_udp_socket_t* socket, wiced_packet_t** packet, uint32_t timeout )
{
    UINT result;

    WICED_LINK_CHECK( socket->socket.nx_udp_socket_ip_ptr );

    result = nx_udp_socket_receive( &socket->socket, packet, NX_TIMEOUT(timeout) );
    wiced_assert("UDP receive error", result == NX_SUCCESS || result == NX_NO_PACKET || result == NX_WAIT_ABORTED );
    return netx_duo_returns[result];
}

wiced_result_t wiced_udp_delete_socket( wiced_udp_socket_t* socket )
{
    UINT result;

    /* Check if socket is still bound */
    if ( socket->socket.nx_udp_socket_bound_next )
    {
        nx_udp_socket_unbind( &socket->socket );
    }

    result = nx_udp_socket_delete( &socket->socket );
    return netx_duo_returns[result];
}

wiced_result_t wiced_udp_register_callbacks( wiced_udp_socket_t* socket, wiced_socket_callback_t receive_callback )
{
    int a;

    /* Add the socket to the list of sockets with callbacks */
    for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
    {
        if ( udp_sockets_with_callbacks[a] == socket || udp_sockets_with_callbacks[a] == NULL )
        {
            udp_sockets_with_callbacks[a] = socket;
            break;
        }
    }

    if ( receive_callback != NULL )
    {
        socket->receive_callback = receive_callback;
        nx_udp_socket_receive_notify( &socket->socket, internal_udp_socket_receive_callback );
    }

    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_udp_unregister_callbacks( wiced_udp_socket_t* socket )
{
   int a;
   for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
   {
       if ( udp_sockets_with_callbacks[a] == socket )
       {
           udp_sockets_with_callbacks[a] = NULL;
       }
   }
   return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_multicast_join( wiced_interface_t interface, const wiced_ip_address_t* address )
{
    UINT result;
    WICED_LINK_CHECK( &IP_HANDLE(interface) );

    result = nx_igmp_multicast_join( &IP_HANDLE( interface ), address->ip.v4 );
    return netx_duo_returns[result];
}

wiced_result_t wiced_multicast_leave( wiced_interface_t interface, const wiced_ip_address_t* address )
{
    WICED_LINK_CHECK( &IP_HANDLE(interface) );

    nx_igmp_multicast_leave( &IP_HANDLE( interface ), address->ip.v4 );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_hostname_lookup( const char* hostname, wiced_ip_address_t* address, uint32_t timeout_ms )
{
    WICED_LINK_CHECK( &IP_HANDLE( WICED_STA_INTERFACE ) );

    /* Check if address is a string representation of a IPv4 address i.e. xxx.xxx.xxx.xxx */
    address->ip.v4 = str_to_ipv4( hostname );
    if ( address->ip.v4 != 0 )
    {
        address->version = WICED_IPV4;
        /* yes this is a string representation of a IPv4 address */
        return WICED_TCPIP_SUCCESS;
    }

    return dns_client_hostname_lookup( hostname, address, timeout_ms );
}

wiced_result_t wiced_ping( wiced_interface_t interface, const wiced_ip_address_t* address, uint32_t timeout_ms, uint32_t* elapsed_ms )
{
    UINT         result;
    wiced_time_t send_time;
    wiced_time_t reply_time;
    NX_PACKET*   response_ptr;

    WICED_LINK_CHECK( &IP_HANDLE(interface) );

    if ( address->version == WICED_IPV4 )
    {
        send_time     = host_rtos_get_time( );
        result        = nxd_icmp_ping( &IP_HANDLE(interface), (NXD_ADDRESS*)address, NULL, 0, &response_ptr, NX_TIMEOUT(timeout_ms) );
        reply_time    = host_rtos_get_time( );
        if ( result == NX_SUCCESS )
        {
            *elapsed_ms = (reply_time - send_time)*1000/SYSTICK_FREQUENCY;
            nx_packet_release( response_ptr );

            return WICED_TCPIP_SUCCESS;
        }
        else if ( result == NX_NO_RESPONSE )
        {
            return WICED_TCPIP_TIMEOUT;
        }
        else
        {
            return WICED_TCPIP_ERROR;
        }
    }
    else
    {
        wiced_assert("ping for ipv6 not implemented yet", 0!=0);
        return WICED_TCPIP_UNSUPPORTED;
    }
}

wiced_result_t wiced_ip_get_gateway_address( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    SET_IPV4_ADDRESS( *ipv4_address, IP_HANDLE(interface).nx_ip_gateway_address );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_ip_get_netmask( wiced_interface_t interface, wiced_ip_address_t* ipv4_address )
{
    SET_IPV4_ADDRESS( *ipv4_address, IP_HANDLE(interface).nx_ip_network_mask );
    return WICED_TCPIP_SUCCESS;
}

wiced_result_t wiced_tcp_server_peer( wiced_tcp_socket_t* socket, wiced_ip_address_t* address, uint16_t* port )
{
    UINT result;
    ULONG peer_port;
    ULONG peer_address;

    if (socket->socket.nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
        address->version = WICED_IPV6;
        memcpy(address->ip.v6, socket->socket.nx_tcp_socket_connect_ip.nxd_ip_address.v6, 16);
        *port = (uint16_t)socket->socket.nx_tcp_socket_connect_port;
        return WICED_TCPIP_SUCCESS;
    }
    else
    {
        result = nx_tcp_socket_peer_info_get( &socket->socket, &peer_address, &peer_port );
        if ( result == NX_SUCCESS )
        {
            *port = (uint16_t)peer_port;
            SET_IPV4_ADDRESS( *address, peer_address );
            return WICED_TCPIP_SUCCESS;
        }
    }

    return netx_duo_returns[result];
}

wiced_result_t wiced_tcp_server_add_tls( wiced_tcp_server_t* tcp_server, wiced_tls_advanced_context_t* context, const char* server_cert, const char* server_key )
{
    int i;

    wiced_tls_init_advanced_context( context, server_cert, server_key );

    for ( i = 0; i < WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS; i++ )
    {
        wiced_tcp_enable_tls( &tcp_server->socket[ i ], context );
    }

    return WICED_SUCCESS;
}

/******************************************************
 *            Static Function Definitions
 ******************************************************/

/*
 ******************************************************************************
 * Convert an ipv4 string to a uint32_t.
 *
 * @param     arg  The string containing the value.
 *
 * @return    The value represented by the string.
 */
static uint32_t str_to_ipv4( const char* arg )
{
    uint32_t addr = 0;
    uint8_t num = 0;

    arg--;

    do
    {
        addr = addr << 8;
        addr += (uint32_t) atoi( ++arg );
        while ( ( *arg != '\x00' ) && ( *arg != '.' ) )
        {
            arg++;
        }
        num++;
    } while ( ( num < 4 ) && ( *arg != '\x00' ) );
    if ( num < 4 )
    {
        return 0;
    }
    return addr;
}

static void internal_tcp_socket_disconnect_callback( struct NX_TCP_SOCKET_STRUCT *socket_ptr )
{
    internal_common_tcp_callback( socket_ptr, WICED_TCP_DISCONNECT_CALLBACK_INDEX );
}

static void internal_tcp_socket_receive_callback( struct NX_TCP_SOCKET_STRUCT *socket_ptr )
{
    internal_common_tcp_callback( socket_ptr, WICED_TCP_RECEIVE_CALLBACK_INDEX );
}

static void internal_common_tcp_callback( struct NX_TCP_SOCKET_STRUCT *socket_ptr, wiced_tcp_callback_index_t callback )
{
    int a;

    /* Find the registered socket and tell the Network Worker thread to run the callback */
    for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
    {
        if ( &tcp_sockets_with_callbacks[a]->socket == socket_ptr )
        {
            wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, tcp_sockets_with_callbacks[a]->callbacks[callback], tcp_sockets_with_callbacks[a] );
            break;
        }
    }
}

static VOID internal_tcp_listen_callback(NX_TCP_SOCKET *socket_ptr, UINT port)
{
    UNUSED_PARAMETER(port);
    internal_common_tcp_callback( socket_ptr, WICED_TCP_CONNECT_CALLBACK_INDEX );
}

static void internal_udp_socket_receive_callback( struct NX_UDP_SOCKET_STRUCT *socket_ptr )
{
    int a;

    /* Find the registered socket and tell the Network Worker thread to run the callback */
    for ( a = 0; a < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; ++a )
    {
        if ( &udp_sockets_with_callbacks[a]->socket == socket_ptr )
        {
            wiced_rtos_send_asynchronous_event( WICED_NETWORKING_WORKER_THREAD, udp_sockets_with_callbacks[a]->receive_callback, udp_sockets_with_callbacks[a] );
            break;
        }
    }
}

static wiced_result_t internal_wiced_tcp_server_listen( wiced_tcp_server_t* tcp_server )
{
    int            i;
    wiced_result_t status;
    int            free_socket_index = -1;

    /* Find a socket that is either listening or free to listen */
    for ( i = 0; i < WICED_MAXIMUM_NUMBER_OF_SERVER_SOCKETS; i++ )
    {
        if ( tcp_server->socket[ i ].socket.nx_tcp_socket_state == NX_TCP_LISTEN_STATE )
        {
            return WICED_SUCCESS;
        }
        else if ( tcp_server->socket[ i ].socket.nx_tcp_socket_state == NX_TCP_CLOSED )
        {
            free_socket_index = i;
        }
        else if ( tcp_server->socket[ i ].socket.nx_tcp_socket_state == NX_TCP_CLOSE_WAIT )
        {
            nx_tcp_socket_disconnect( &tcp_server->socket[ i ].socket, NX_NO_WAIT );
        }
    }

    /* Check if we didn't find a free socket */
    if ( free_socket_index == -1 )
    {
        return WICED_ERROR;
    }

    status = nx_tcp_server_socket_relisten( tcp_server->socket[ free_socket_index ].socket.nx_tcp_socket_ip_ptr, tcp_server->port, &tcp_server->socket[ free_socket_index ].socket );

    if ( status == NX_ALREADY_BOUND )
    {
        nx_tcp_server_socket_unaccept( &tcp_server->socket[ free_socket_index ].socket );
        status = nx_tcp_server_socket_relisten( tcp_server->socket[ free_socket_index ].socket.nx_tcp_socket_ip_ptr, tcp_server->port, &tcp_server->socket[ free_socket_index ].socket );
    }
    return netx_duo_returns[ status ];
}
