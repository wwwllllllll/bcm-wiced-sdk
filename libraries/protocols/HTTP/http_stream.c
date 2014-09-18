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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wiced.h"
#include "http_stream.h"
#include "base64.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* maximum number of the sessions that can be opened at the same time */
#define NUM_HTTP_SESSIONS               ( 5 )
#define TCP_CONNECTION_TIMEOUT          ( 5000 )

#define CONNECTION_KEEP_ALIVE_STR       "Connection: Keep-Alive\r\n"
#define CONNECTION_CLOSE_STR            "Connection: Close\r\n"
#define KEEP_ALIVE_STR                  "Keep-Alive"
#define HTTP_1_1_STR                    " HTTP/1.1\r\n"
#define AUTHORIZATION_STR               "Authorization"
#define BASIC_STR                       "Basic "

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
 *               Variable Definitions
 ******************************************************/

static const char* http_methods[] =
{
    [HTTP_GET]   = "GET ",
    [HTTP_POST]  = "POST ",
    [HTTP_HEAD]  = "HEAD ",
};

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t http_deinit_stream( http_stream_t* session )
{
    wiced_assert("Bad args", session != NULL);

    session->is_connected = 0;

    wiced_tcp_stream_deinit(&session->tcp_stream);

    wiced_tcp_delete_socket( &session->socket );

    return WICED_SUCCESS;
}

wiced_result_t http_init_stream( http_stream_t* session, const char* host_name, wiced_ip_address_t* ip_address, uint16_t port )
{
    uint8_t        count  = 0;
    wiced_result_t result = WICED_ERROR;

    wiced_assert("Bad args", session != NULL);

    /* Clear the session object */
    memset(session, 0, sizeof(http_stream_t));

    /* check whether the hostname size is supported */
    if ( host_name != NULL )
    {
        if ( strlen( host_name ) > ( MAX_HTTP_HOST_NAME_SIZE - 1 ) )
        {
            return WICED_BADARG;
        }

        while ( wiced_hostname_lookup( host_name, &session->host_ip, 5000 ) != WICED_SUCCESS )
        {
            count++;
            if ( count >= 2 )
            {
                WPRINT_LIB_DEBUG(( "Unable to find a host\n\r" ));
                goto return_error;
            }
        }

        /* copy hostname locally */
        strcpy( session->hostname, host_name );
    }
    else
    {
        /* otherwise assign IP address */
        SET_IPV4_ADDRESS( session->host_ip, GET_IPV4_ADDRESS((*ip_address)) );
    }

    /* Create a TCP socket */
    if ( wiced_tcp_create_socket( &session->socket, WICED_STA_INTERFACE ) != WICED_SUCCESS )
    {
        goto return_error;
    }

    /* Open connection to the HTTP server */
    if ( wiced_tcp_connect( &session->socket, &session->host_ip, port, TCP_CONNECTION_TIMEOUT ) != WICED_SUCCESS )
    {
        goto return_error_with_socket;
    }

    /* Init the TCP stream */
    if ( wiced_tcp_stream_init( &session->tcp_stream, &session->socket ) != WICED_SUCCESS )
    {
        goto return_error_with_socket;
    }

    session->is_connected = 1;

    return WICED_SUCCESS;

return_error_with_socket:
    wiced_tcp_delete_socket( &session->socket );

return_error:

    return result;
}


#if 0 /* unreferenced */
wiced_result_t http_stream_send_request( http_stream_t* session, http_request_t method, const char* url, const http_header_t* headers, uint16_t number_of_headers,  uint8_t* body, uint16_t body_length, wiced_bool_t keep_alive )
{
    wiced_result_t result;

    /* start request with specifying the http method and the url */
    http_stream_start_headers(session, method, url );

    /* add all required headers */
    result = http_stream_add_headers( session, headers, number_of_headers );\
    if( result != WICED_SUCCESS )
    {
        return result;
    }
    /* finish with headers and set the last header keep_alive */
    http_stream_end_headers(session, keep_alive);

    /* add the body to the http request */
    http_stream_write( session, body, body_length );
    http_stream_flush( session );

    /* return with no error */
    return WICED_SUCCESS;
}
#endif /* if 0 unreferenced */

wiced_result_t http_stream_start_headers( http_stream_t* session, http_request_t method, const char* url )
{
    wiced_assert("Bad args", (session != NULL) && (url != NULL));

    /* add method and version, host and url to the tcp stream */
    WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, http_methods[method], strlen( http_methods[method] ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, url, strlen( url ) ) );
    WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, HTTP_1_1_STR, sizeof(HTTP_1_1_STR) - 1 ) );

    /* Write host name required by http/1.0 version */
    if ( strlen( session->hostname ) != 0 )
    {
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, "Host: ", strlen("Host: ") ) );
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, session->hostname, strlen( session->hostname ) ) );
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, HTTP_NEW_LINE, strlen( HTTP_NEW_LINE ) ) );
    }

    return WICED_SUCCESS;
}

/* add a list of headers to the request */
wiced_result_t http_stream_add_headers( http_stream_t* session, const http_header_t* headers, uint16_t number_of_headers )
{
    uint16_t i = 0;

    wiced_assert("Bad args", (session != NULL) && (headers != NULL));

    for ( i = 0; i < number_of_headers; ++i, ++headers )
    {
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, headers->name, strlen( headers->name ) ) );
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, ": ", strlen( ": " ) ) );
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, headers->value, strlen( headers->value ) ) );
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, HTTP_NEW_LINE, strlen( HTTP_NEW_LINE ) ) );
    }

    return WICED_SUCCESS;
}

wiced_result_t http_stream_end_headers( http_stream_t* session, wiced_bool_t keep_alive)
{
    if ( keep_alive == WICED_TRUE )
    {
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, CONNECTION_KEEP_ALIVE_STR, sizeof(CONNECTION_KEEP_ALIVE_STR) - 1 ) );
    }
    else
    {
        WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, CONNECTION_CLOSE_STR, sizeof(CONNECTION_CLOSE_STR) - 1 ) );
    }

    WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, HTTP_NEW_LINE, strlen( HTTP_NEW_LINE ) ) );

    return WICED_SUCCESS;
}

/* this function will add a body to the http request */
wiced_result_t http_stream_write( http_stream_t* session, uint8_t* data, uint16_t length )
{
    wiced_assert("Bad args", (session != NULL) && (data != NULL));

    WICED_VERIFY( wiced_tcp_stream_write( &session->tcp_stream, data, length ) );

    return WICED_SUCCESS;
}

wiced_result_t http_stream_flush( http_stream_t* session )
{
    wiced_assert("Bad args", (session != NULL));

    WICED_VERIFY( wiced_tcp_stream_flush( &session->tcp_stream ) );

    return WICED_SUCCESS;
}

wiced_result_t http_send_basic_authorization( http_stream_t* session, char* username_password )
{
    wiced_result_t result;
    int            base64_datalen;
    unsigned int   value_size;
    http_header_t  basic_auth = { AUTHORIZATION_STR, NULL };

    wiced_assert("Bad args", (username_password != NULL));

    value_size = sizeof( BASIC_STR ) + DIV_ROUND_UP( strlen(username_password) * 4, 3 ) + 1;

    basic_auth.value = malloc( value_size );
    if ( basic_auth.value == NULL )
    {
        return WICED_OUT_OF_HEAP_SPACE;
    }

    /* Add Authorisation basic field */
    strcpy( basic_auth.value, BASIC_STR );

    base64_datalen = base64_encode( (unsigned char*) username_password, strlen( username_password ), (unsigned char*)&basic_auth.value[sizeof( BASIC_STR ) - 1], value_size - ( sizeof( BASIC_STR ) - 1 ), BASE64_ENC_STANDARD );
    if ( base64_datalen < 0 )
    {
        free( basic_auth.value );
        return WICED_ERROR;
    }

    result = http_stream_add_headers( session, &basic_auth, 1 );

    free( basic_auth.value );

    return result;

}


/* Extract a header value from the HTTP response received from the server */
wiced_result_t http_extract_headers( wiced_packet_t* packet, http_header_t* headers, uint16_t number_of_headers )
{
    uint8_t* data;
    uint16_t data_length;
    uint16_t available_data_length;
    char*    header_location;
    int a;
    wiced_bool_t finished = WICED_FALSE;
    uint16_t matched_headers = 0;

    wiced_packet_get_data( packet, 0, &data, &data_length, &available_data_length );

    header_location = (char*)data;
    do
    {
        /* Skip to the next header entry */
        header_location = strchr( (const char*) header_location, '\n' );
        if ( header_location == 0 )
        {
            return WICED_ERROR;
        }
        ++header_location;

        /* Check if we haven't reached the end of the headers */
        if (((uint8_t*)header_location - data) < data_length &&
            !(header_location[0] == '\r' && header_location[1] == '\n'))
        {
            /* Try find a matching header */
            for ( a = 0; a < number_of_headers; ++a )
            {
                unsigned int string_length = strlen( headers[a].name );
                if ( ( memcmp( header_location, headers[a].name, string_length ) == 0 ) &&
                     ( *( header_location + string_length ) == ':' ) )
                {
                    headers[a].value = header_location + string_length + 1;
                    ++matched_headers;
                    break;
                }
            }
        }
        else
        {
            finished = WICED_TRUE;
        }
    } while ( ( finished == WICED_FALSE ) && ( matched_headers != number_of_headers ) );

    return WICED_SUCCESS;
}

/* get body of the response received from the server */
wiced_result_t http_get_body( wiced_packet_t* packet, uint8_t** body, uint32_t* body_length )
{
    uint8_t* packet_data;
    uint16_t packet_data_length;
    uint16_t available_data_length;
    char*    start_of_body;

    wiced_assert("Bad args", (packet != NULL) && (body != NULL) && (body_length != NULL));

    WICED_VERIFY( wiced_packet_get_data( packet, 0, &packet_data, &packet_data_length, &available_data_length ) );

    /* Find the first occurrence of "\r\n\r\n", this signifies the end of the headers */
    start_of_body = strstr( (const char*) packet_data, HTTP_HEADERS_BODY_SEPARATOR );
    if ( start_of_body == 0 )
    {
        return WICED_NOT_FOUND;
    }
    /* Skip the "\r\n\r\n" */
    start_of_body += 4;

    *body_length = (uint32_t)( packet_data_length - ( ( start_of_body - (char*) packet_data ) ) );
    *body        = (uint8_t*) start_of_body;

    return WICED_SUCCESS;
}

wiced_result_t http_stream_receive( http_stream_t* session, wiced_packet_t** packet, uint32_t timeout)
{
    wiced_assert("Bad args", (session != NULL) && (packet != NULL) );

    /* Receive a response from the tcp, connection is considered to be established */
    WICED_VERIFY( wiced_tcp_receive( &session->socket, packet, timeout ) );

    return WICED_SUCCESS;
}

wiced_result_t http_process_response( wiced_packet_t* packet, http_status_code_t* response_code )
{
    char*          response_status;
    uint8_t*       response;
    uint16_t       response_length;
    uint16_t       available_data_length;

    /* Get pointer to data from a packet */
    if ( wiced_packet_get_data( packet, 0, &response, &response_length, &available_data_length ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    /* Check we have enough data to identify the response number */
    if (response_length < 12)
    {
        return WICED_ERROR;
    }

    /* Find the HTTP/x.x part*/
    response_status = strstr( (const char*) response, "HTTP/" );
    if (response_status == 0)
    {
        return WICED_ERROR;
    }
    /* Skip the "HTTP/" and the version "x.x"*/
    response_status += 5 + 3;

    /* Verify next character is a space*/
    if ( *response_status != ' ' )
    {
        return WICED_ERROR;
    }
    /* Skip the space */
    ++response_status;

    /* Verify response is 3 characters followed by a space */
    if ( *(response_status + 3) != ' ' )
    {
        return WICED_ERROR;
    }
    *response_code = atoi( response_status );

    return WICED_SUCCESS;
}
