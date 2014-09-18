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

#include "xively.h"
#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define HOST_NAME               "api.xively.com"
#define DNS_TIMEOUT             3000
#define SOCKET_CONNECT_TIMEOUT  5000
#define HTTPS_PORT              443

#define PUT_START               "PUT /v2/feeds/"
#define PUT_END                 ".csv HTTP/1.1\r\n"
#define HOST_TEMPLATE           "Host: api.xively.com\r\n"
#define API_KEY_HEADER          "X-ApiKey: "
#define COMMA                   ","
#define NEWLINE                 "\r\n"
#define CONTENT_LENGTH          "Content-Length: "

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

/******************************************************
 *               Variables Declarations
 ******************************************************/

static wiced_ip_address_t xively_server_ip_address;
static wiced_bool_t       is_ip_address_resolved = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t xively_open_feed( xively_feed_t* feed )
{
    wiced_result_t result;
    if ( is_ip_address_resolved == WICED_FALSE )
    {
        WICED_VERIFY( wiced_hostname_lookup( HOST_NAME, &xively_server_ip_address, DNS_TIMEOUT ) );

        is_ip_address_resolved = WICED_TRUE;
    }

    wiced_tls_init_simple_context( &feed->tls_context, NULL );

    WICED_VERIFY( wiced_tcp_create_socket( &feed->socket, WICED_STA_INTERFACE ) );

    result = wiced_tcp_bind( &feed->socket, WICED_ANY_PORT );
    if ( result != WICED_SUCCESS )
    {
        wiced_tcp_delete_socket( &feed->socket );
        return result;
    }

    result = wiced_tcp_enable_tls( &feed->socket, &feed->tls_context );
    if ( result != WICED_SUCCESS )
    {
        wiced_tls_deinit_context( &feed->tls_context );
        wiced_tcp_delete_socket( &feed->socket );
        return result;
    }

    result = wiced_tcp_connect( &feed->socket, &xively_server_ip_address, HTTPS_PORT, SOCKET_CONNECT_TIMEOUT );
    if ( result != WICED_SUCCESS )
    {
        wiced_tcp_delete_socket( &feed->socket );
        return result;
    }

    return WICED_SUCCESS;
}

wiced_result_t xively_close_feed( xively_feed_t* feed )
{
    wiced_tcp_disconnect( &feed->socket );
    return wiced_tcp_delete_socket( &feed->socket );
}

void xively_u16toa( uint16_t value, char* output, uint8_t min_length )
{
    char   buffer[ 5 ] = "00000";
    int8_t digits_left = 5;

    while ( value != 0 )
    {
        --digits_left;
        buffer[ digits_left ] = (char)(( value % 10 ) + '0');
        value = value / 10;
    }

    if ( min_length <= 5 )
    {
        digits_left = (int8_t) MIN(5-min_length, digits_left);
    }

    memcpy( output, &buffer[ digits_left ], (size_t)( 5 - digits_left ) );
}

wiced_result_t xively_create_datastream( xively_feed_t* feed, xively_datastream_t* stream, const char* channel_name, uint32_t data_size, uint32_t number_of_datapoints )
{
    char     content_length_str[ 5 ];
    uint16_t content_length = 0;

    if ( wiced_tcp_stream_init( &stream->tcp_stream, &feed->socket ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    stream->channel_name         = channel_name;
    stream->data_size            = data_size;
    stream->number_of_datapoints = number_of_datapoints;

    /* Write METHOD and API Endpoint */
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)PUT_START, sizeof( PUT_START ) - 1 ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)feed->id, strlen( feed->id ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)PUT_END, sizeof( PUT_END ) - 1 ) );

    /* Write Host */
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)HOST_TEMPLATE, sizeof( HOST_TEMPLATE ) - 1 ) );

    /* Write API Key */
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)API_KEY_HEADER, sizeof( API_KEY_HEADER ) - 1 ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)feed->api_key, strlen( feed->api_key ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)NEWLINE, sizeof( NEWLINE ) - 1 ) );

    /* Write data stream content length */
    content_length = (uint16_t)( number_of_datapoints * ( strlen( channel_name ) + ( sizeof( COMMA ) - 1 ) + sizeof(wiced_iso8601_time_t) + ( sizeof( COMMA ) - 1 ) + data_size + ( sizeof( NEWLINE ) - 1 ) ) );
    memset( content_length_str, 0, sizeof( content_length_str ) );
    xively_u16toa( content_length, content_length_str, sizeof( content_length_str ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)CONTENT_LENGTH, sizeof( CONTENT_LENGTH ) - 1 ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)content_length_str, sizeof( content_length_str ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)NEWLINE, sizeof( NEWLINE ) - 1 ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)NEWLINE, sizeof( NEWLINE ) - 1 ) );

    return WICED_SUCCESS;
}

wiced_result_t xively_write_datapoint( xively_datastream_t* stream, const uint8_t* data, const wiced_iso8601_time_t* timestamp )
{
    /* Write datapoint */
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)stream->channel_name, strlen( stream->channel_name ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)COMMA, sizeof( COMMA ) - 1 ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)timestamp, sizeof( *timestamp ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)COMMA, sizeof( COMMA ) - 1 ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)data, stream->data_size ) );
    WICED_VERIFY( wiced_tcp_stream_write( &stream->tcp_stream, (void*)NEWLINE, sizeof( NEWLINE ) - 1 ) );

    return WICED_SUCCESS;
}

wiced_result_t xively_flush_datastream( xively_datastream_t* stream )
{
    wiced_packet_t* response_packet;

    WICED_VERIFY( wiced_tcp_stream_flush( &stream->tcp_stream ) );

    /* Receive and free response packet */
    if ( wiced_tcp_receive( stream->tcp_stream.socket, &response_packet, 3000 ) == WICED_SUCCESS )
    {
        uint8_t* data;
        uint16_t data_length;
        uint16_t available_data_length;
        char* is_ok_found;

        /* Parse HTTP response.  */
        wiced_packet_get_data( response_packet, 0, &data, &data_length, &available_data_length );

        /* Ensure that packet payload is NUL-terminated */
        *( data + data_length - 1 ) = 0;

        /* Find OK. Returns NULL of not found */
        is_ok_found = strstr( (char*) data, (char*) "OK" );

        wiced_packet_delete( response_packet );

        /* OK found. */
        if ( is_ok_found != NULL )
        {
            return WICED_SUCCESS;
        }

        /* If Not OK, assert flag to perform DNS query in the next data sending */
        is_ip_address_resolved = WICED_FALSE;
    }

    return WICED_ERROR;
}
