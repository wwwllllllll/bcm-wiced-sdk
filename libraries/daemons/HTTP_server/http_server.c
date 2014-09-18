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
 *  Implements both HTTP and HTTPS servers
 *
 */

#include <string.h>
#include "http_server.h"
#include "wwd_assert.h"
#include "wiced.h"
#include "wiced_resource.h"
#include "strings.h"
#include "platform_resource.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#if (defined(WPRINT_ENABLE_WEBSERVER) && (defined(WPRINT_ENABLE_DEBUG) || defined(WPRINT_ENABLE_ERROR)))
#define HTTP_SERVER_STACK_SIZE      (5000) /* printf requires 4K of stack */
#else
#define HTTP_SERVER_STACK_SIZE      (10000)
#endif

#define HTTP_SERVER_THREAD_PRIORITY     (WICED_DEFAULT_LIBRARY_PRIORITY)
#define SSL_LISTEN_PORT                 (443)
#define HTTP_LISTEN_PORT                (80)
#define HTTP_SERVER_RECEIVE_TIMEOUT     (2000)

/* HTTP Tokens */
#define GET_TOKEN                      "GET "
#define POST_TOKEN                     "POST "
#define PUT_TOKEN                      "PUT "

#define HTTP_1_1_TOKEN                 " HTTP/1.1"
#define FINAL_CHUNKED_PACKET            "0\r\n\r\n"

/*
 * Request-Line =   Method    SP        Request-URI           SP       HTTP-Version      CRLFCRLF
 *              = <-3 char->  <-1 char->   <-1 char->      <-1 char->  <--8 char-->    <-4char->
 *              = 18
 */
#define MINIMUM_REQUEST_LINE_LENGTH   (18)
#define EVENT_QUEUE_DEPTH             (10)
#define COMPARE_MATCH                 (0)
#define MAX_URL_LENGTH                (100)

#ifdef USE_SELF_SIGNED_TLS_CERT
static const char brcm_server_certificate[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDnzCCAwigAwIBAgIJANJ7KTgzT4vwMA0GCSqGSIb3DQEBBQUAMIGSMQswCQYD\r\n"
    "VQQGEwJBVTEMMAoGA1UECBMDTlNXMQ8wDQYDVQQHEwZTeWRuZXkxHTAbBgNVBAoT\r\n"
    "FEJyb2FkY29tIENvcnBvcmF0aW9uMQ0wCwYDVQQLEwRXTEFOMREwDwYDVQQDEwhC\r\n"
    "cm9hZGNvbTEjMCEGCSqGSIb3DQEJARYUc3VwcG9ydEBicm9hZGNvbS5jb20wHhcN\r\n"
    "MTIwMzEyMjE0ODIyWhcNMTIwNDExMjE0ODIyWjCBkjELMAkGA1UEBhMCQVUxDDAK\r\n"
    "BgNVBAgTA05TVzEPMA0GA1UEBxMGU3lkbmV5MR0wGwYDVQQKExRCcm9hZGNvbSBD\r\n"
    "b3Jwb3JhdGlvbjENMAsGA1UECxMEV0xBTjERMA8GA1UEAxMIQnJvYWRjb20xIzAh\r\n"
    "BgkqhkiG9w0BCQEWFHN1cHBvcnRAYnJvYWRjb20uY29tMIGfMA0GCSqGSIb3DQEB\r\n"
    "AQUAA4GNADCBiQKBgQDVyKV/R5LCQvLliHFU1303w8nv63THLzmUMGqpf1/N/nZP\r\n"
    "pPsgihr/ZZDFxb4zNKpUqU1yoPl0QpB9TdjFDHzHMzGdo6ZcKGk86xTJSg6Ed6uT\r\n"
    "O/qXDXIvURiDhAdQXmBNDs9xGoIxGH8FwvNHPfLENCfnuHKX9KDye6MvOv5kHwID\r\n"
    "AQABo4H6MIH3MB0GA1UdDgQWBBRDEgx5s41uyudha03Bv3YaGy6NdzCBxwYDVR0j\r\n"
    "BIG/MIG8gBRDEgx5s41uyudha03Bv3YaGy6Nd6GBmKSBlTCBkjELMAkGA1UEBhMC\r\n"
    "QVUxDDAKBgNVBAgTA05TVzEPMA0GA1UEBxMGU3lkbmV5MR0wGwYDVQQKExRCcm9h\r\n"
    "ZGNvbSBDb3Jwb3JhdGlvbjENMAsGA1UECxMEV0xBTjERMA8GA1UEAxMIQnJvYWRj\r\n"
    "b20xIzAhBgkqhkiG9w0BCQEWFHN1cHBvcnRAYnJvYWRjb20uY29tggkA0nspODNP\r\n"
    "i/AwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQCrKwmGU1Zd4ZSdr3zy\r\n"
    "qhKXb3PZwr01k1CXgYHgBv7VomIeMqRjE2S/oAM5tWmjWPuvfKjROJNoFestWM3w\r\n"
    "Pad3TtmGZSnHK13LXVdoGlDb/zYzlQhoAYYxvQrt0VD7sNhyvT9Ec1/trmfMR+jK\r\n"
    "RljZgDdQdIhenl+zlV33enbx2Q==\r\n"
    "-----END CERTIFICATE-----\r\n";

static const char brcm_server_rsa_key[] =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIICWwIBAAKBgQDVyKV/R5LCQvLliHFU1303w8nv63THLzmUMGqpf1/N/nZPpPsg\r\n"
    "ihr/ZZDFxb4zNKpUqU1yoPl0QpB9TdjFDHzHMzGdo6ZcKGk86xTJSg6Ed6uTO/qX\r\n"
    "DXIvURiDhAdQXmBNDs9xGoIxGH8FwvNHPfLENCfnuHKX9KDye6MvOv5kHwIDAQAB\r\n"
    "AoGAEiw7PUWVSSQtx6tAjwi+YTYofVeTlrcB+wHen0fvmfAumHiazFpRDzLQCq/T\r\n"
    "ikDI1eeKaNscOXDLHYu3iJCWLqTkmhMBweOuorYMIQIBYbIqUg6OsBoY7oRrJbwg\r\n"
    "9jJEFI2vPEtIWLn3cRRgL9fs6zF3unF0qXMkr3N34BuyEYECQQD5yfF7cWgEKQvS\r\n"
    "P4yaZcGQZdcU3G3OEh+SpgFQmN5fYol2VTKZzd1iR9GQSUjoTrkMEVNiwQtVvYiu\r\n"
    "a9Pt4xffAkEA2xmCW8XkUFc94uRx2MTCjn3so1am79SRc3f4SRbA4rfJfQH883a0\r\n"
    "YYSV2tZICWq8VKpdsGYCFWSajMazgNc7wQJAUq9UbmZl5iqoLRq4MkvIvUHY5qDp\r\n"
    "ADPjm6mz+bgAtFZr5m3haCRLSkM3zalUpwGYI7SAg8ofNGyfGA29g5uOxQJARAUF\r\n"
    "XWxwVyjeg6QcXAmpxQb/Ai6SoP5DMa/bGwW/WCNqoC6P0x3VHjlFNK01rAbA9R/2\r\n"
    "+h6RIwcam/3MGIG5gQJANq3n4PlU8nkn661XfWpEXKrwHwqZqMjtKaLRBwmE1ojU\r\n"
    "B3zc3RMJ5qy0nBYkFdQkbDiomqRjnnT/j4kDJF8Xfw==\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";
#endif

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    SOCKET_MESSAGE_EVENT,
    SOCKET_CONNECT_EVENT,
    SOCKET_DISCONNECT_EVENT,
} http_server_event_t;

typedef enum
{
    HTTP_HEADER_AND_DATA_FRAME_STATE,
    HTTP_DATA_ONLY_FRAME_STATE,
} wiced_http_packet_state_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_tcp_socket_t*  socket;
    http_server_event_t  event_type;
} server_event_message_t;

typedef struct
{
    wiced_tcp_socket_t*        socket;
    wiced_http_packet_state_t  http_packet_state;
    char                       url[MAX_URL_LENGTH];
    uint16_t                   url_length;
    wiced_http_message_body_t  message_body_descriptor;
} wiced_http_message_context_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

wiced_result_t wiced_http_server_connect_callback    ( void *socket );
wiced_result_t wiced_http_server_disconnect_callback ( void *socket );
wiced_result_t wiced_http_server_receive_callback    ( void *socket );

static wiced_result_t            http_server_parse_packet                               (const wiced_http_page_t* page_database, wiced_tcp_socket_t* socket, wiced_packet_t* packet, wiced_bool_t* close_request_received);
static wiced_result_t            process_url_request                                    ( wiced_tcp_socket_t* socket, const wiced_http_page_t* server_url_list, char * url, int url_len, wiced_http_message_body_t* http_message_body );
static uint16_t                  escaped_string_copy                                    ( char* output, uint16_t output_length, const char* input, uint16_t input_length );
static wiced_packet_mime_type_t  wiced_get_mime_type                                    ( const char* request_data );
static void                      http_server_thread_main                                ( uint32_t arg );
static wiced_result_t            wiced_get_http_request_type_and_url                    ( char* request, wiced_http_request_type_t* type, char** url_start, uint16_t* url_length );
static wiced_result_t            wiced_http_chunk_response                              ( void* stream, const void* data, uint32_t data_length );
static wiced_result_t            wiced_http_server_get_packet_data                      ( wiced_packet_t* packet, char** data, uint16_t* request_length );
static wiced_result_t            wiced_http_server_set_http_message_body_type_for_socket( wiced_tcp_socket_t* socket, char* url, uint16_t url_length, wiced_http_message_body_t* message_body_descriptor );
static wiced_http_packet_state_t wiced_get_http_packet_state_and_socket_context         ( wiced_tcp_socket_t* socket, wiced_http_message_context_t* socket_http_context );
static wiced_result_t            wiced_reset_current_message_body_context_for_socket    ( wiced_tcp_socket_t* socket );

static wiced_result_t            wiced_http_reset_socket_close_request                  ( void );
static wiced_bool_t              wiced_http_get_socket_close_request                    ( void );

/******************************************************
 *                 Static Variables
 ******************************************************/

static wiced_queue_t                           server_event_queue;
static http_server_packet_process_callback_t   packet_process_callback = NULL;
static uint8_t                                 socket_http_message_context_counter = 0;
static wiced_http_message_context_t            socket_http_message_context[WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS];

static const char* http_mime_array[ MIME_UNSUPPORTED ] =
{
    MIME_TABLE( EXPAND_AS_MIME_TABLE )
};

static wiced_bool_t external_close_request_received = WICED_FALSE;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_http_server_start( wiced_http_server_t* server, uint16_t port, const wiced_http_page_t* page_database, wiced_interface_t interface )
{
    memset( server, 0, sizeof( *server ) );

    /* Store the inputs database */
    server->page_database = page_database;

    wiced_http_server_deregister_packet_post_processing_function();

    /* Initialise the socket state for all sockets */
    WICED_VERIFY( wiced_tcp_server_start( &server->tcp_server, interface, port, wiced_http_server_connect_callback, wiced_http_server_receive_callback, wiced_http_server_disconnect_callback ) );

    WICED_VERIFY( wiced_rtos_init_queue( &server_event_queue, NULL, sizeof(server_event_message_t), EVENT_QUEUE_DEPTH ) );

    /* Create server thread and return */
    return wiced_rtos_create_thread(&server->thread, HTTP_SERVER_THREAD_PRIORITY, "HTTPserver", http_server_thread_main, HTTP_SERVER_STACK_SIZE, server);
}

wiced_result_t wiced_http_server_stop (wiced_http_server_t* server)
{
    server->quit = WICED_TRUE;
    if ( wiced_rtos_is_current_thread( &server->thread ) != WICED_SUCCESS )
    {
        wiced_rtos_thread_force_awake( &server->thread );
        wiced_rtos_thread_join( &server->thread );
        wiced_rtos_delete_thread( &server->thread );
    }
    wiced_http_server_deregister_packet_post_processing_function();

    return WICED_SUCCESS;
}

wiced_result_t wiced_https_server_stop (wiced_https_server_t* server)
{
    server->quit = WICED_TRUE;

    if ( wiced_rtos_is_current_thread( &server->thread ) != WICED_SUCCESS )
    {
        wiced_rtos_thread_force_awake( &server->thread );
        wiced_rtos_thread_join( &server->thread );
        wiced_rtos_delete_thread( &server->thread );
    }

    wiced_http_server_deregister_packet_post_processing_function();

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_server_connect_callback( void* socket )
{
    server_event_message_t current_event;

    current_event.event_type = SOCKET_CONNECT_EVENT;
    current_event.socket     = (wiced_tcp_socket_t*)socket;
    wiced_rtos_push_to_queue( &server_event_queue, &current_event, WICED_NO_WAIT );

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_server_disconnect_callback( void* socket )
{
    server_event_message_t current_event;

    current_event.event_type = SOCKET_DISCONNECT_EVENT;
    current_event.socket     = (wiced_tcp_socket_t*)socket;
    wiced_rtos_push_to_queue( &server_event_queue, &current_event, WICED_NO_WAIT );

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_server_receive_callback( void* socket )
{

    server_event_message_t current_event;

    current_event.event_type = SOCKET_MESSAGE_EVENT;
    current_event.socket     = (wiced_tcp_socket_t*)socket;
    wiced_rtos_push_to_queue( &server_event_queue, &current_event, WICED_NO_WAIT );

    return WICED_SUCCESS;
}

wiced_result_t wiced_https_server_start(wiced_https_server_t* server, uint16_t port, const wiced_http_page_t* page_database, const char* server_cert, const char* server_key, wiced_interface_t interface )
{
    memset( server, 0, sizeof( *server ) );

    /* store the inputs database */
    server->page_database = page_database;

    /* Initialise the socket state for all sockets */
    WICED_VERIFY( wiced_tcp_server_start( &server->tcp_server, interface, port, wiced_http_server_connect_callback, wiced_http_server_receive_callback, wiced_http_server_disconnect_callback ) );

#ifdef USE_SELF_SIGNED_TLS_CERT
    server_cert = brcm_server_certificate;
    server_key  = brcm_server_rsa_key;
#endif
    wiced_tcp_server_add_tls( &server->tcp_server, &server->tls_context, server_cert, server_key );

    WICED_VERIFY( wiced_rtos_init_queue( &server_event_queue, NULL, sizeof(server_event_message_t), EVENT_QUEUE_DEPTH ) );

    wiced_http_server_deregister_packet_post_processing_function();

    /* Create server thread and return */
    return wiced_rtos_create_thread( &server->thread, HTTP_SERVER_THREAD_PRIORITY, "HTTPSserver", http_server_thread_main, HTTP_SERVER_STACK_SIZE, server );

}

static wiced_result_t http_server_parse_packet(const wiced_http_page_t* page_database, wiced_tcp_socket_t* socket, wiced_packet_t* packet, wiced_bool_t* close_request_received)
{
    char*                         request_string;
    uint16_t                      request_length;
    uint16_t                      new_url_length;
    char*                         message_data_length_string;
    char*                         mime;
    wiced_http_packet_state_t     http_packet_state;
    char*                         start_of_url           = NULL; /* Suppress compiler warning */
    uint16_t                      url_length             = 0;    /* Suppress compiler warning */
    wiced_http_message_context_t* socket_context         = NULL;
    wiced_result_t                result                 = WICED_ERROR;
    wiced_bool_t                  processing_http_packet = WICED_TRUE;
    wiced_http_message_body_t     http_message_body =
    {
            .data                         = NULL,
            .message_data_length          = 0,
            .total_message_data_remaining = 0,
            .chunked_transfer             = WICED_FALSE,
            .mime_type                    = MIME_UNSUPPORTED,
            .request_type                 = REQUEST_UNDEFINED
    };

    wiced_http_reset_socket_close_request();

    if ( packet == NULL )
    {
        return WICED_ERROR;
    }

    WICED_VERIFY( wiced_http_server_get_packet_data( packet, &request_string, &request_length ) );

    /* Check socket context and check if this socket received all data in http message or was it split */
    http_packet_state = wiced_get_http_packet_state_and_socket_context( socket, socket_context );

    while ( processing_http_packet == WICED_TRUE )
    {
        switch( http_packet_state )
        {
            case HTTP_DATA_ONLY_FRAME_STATE:
                socket_context->message_body_descriptor.data = (uint8_t*)request_string;

                if( request_length <= socket_context->message_body_descriptor.total_message_data_remaining )
                {
                    socket_context->message_body_descriptor.message_data_length = request_length;
                }
                else
                {
                    socket_context->message_body_descriptor.message_data_length = socket_context->message_body_descriptor.total_message_data_remaining;
                }

                socket_context->message_body_descriptor.total_message_data_remaining = (uint16_t)( socket_context->message_body_descriptor.total_message_data_remaining -  http_message_body.message_data_length );

                if( socket_context->message_body_descriptor.total_message_data_remaining == 0 )
                {
                    socket_context->http_packet_state = HTTP_HEADER_AND_DATA_FRAME_STATE;
                }

                process_url_request( (wiced_tcp_socket_t*)socket, page_database, socket_context->url, socket_context->url_length , &socket_context->message_body_descriptor );

                request_length = (uint16_t)( request_length - socket_context->message_body_descriptor.message_data_length );

                http_packet_state = socket_context->http_packet_state = HTTP_HEADER_AND_DATA_FRAME_STATE;

                break;

            case HTTP_HEADER_AND_DATA_FRAME_STATE:
                /* Verify we have enough data to start processing */
                if ( request_length < MINIMUM_REQUEST_LINE_LENGTH )
                {
                    return WICED_ERROR;
                }

                /* Check if this is a close request */
                if ( strstr( request_string, HTTP_HEADER_CLOSE ) != NULL )
                {
                    *close_request_received = WICED_TRUE;
                    wiced_reset_current_message_body_context_for_socket( socket );
                }
                else
                {
                    *close_request_received = WICED_FALSE;
                }

                /* First extract the URL from the packet */
                result = wiced_get_http_request_type_and_url( request_string, &http_message_body.request_type, &start_of_url, &url_length );

                if ( result == WICED_ERROR )
                {
                    return result;
                }

                /* Remove escape strings from URL */
                new_url_length = escaped_string_copy( start_of_url, url_length, start_of_url, url_length );

                /* Now extract packet payload info such as data, data length, data type and message length */
                http_message_body.data = (uint8_t*)strstr( request_string, CRLF_CRLF );

                /* This indicates start of data/end of header was not found, so exit */
                if ( http_message_body.data  == NULL)
                {
                    return WICED_ERROR;
                }
                else
                {
                    /* Payload starts just after the header */
                    http_message_body.data += strlen( CRLF_CRLF );
                }

                mime = strstr(request_string, HTTP_HEADER_CONTENT_TYPE );
                if ( ( mime != NULL ) && ( mime < (char*)http_message_body.data ) )
                {
                    mime+=strlen(HTTP_HEADER_CONTENT_TYPE);
                    http_message_body.mime_type = wiced_get_mime_type( mime );
                }
                else
                {
                    http_message_body.mime_type = MIME_TYPE_ALL;
                }

                if ( strstr( request_string, HTTP_HEADER_CHUNKED ) )
                {
                    /* Indicate the format of this frame is chunked. Its up to the application to reassemble */
                    http_message_body.chunked_transfer  = WICED_TRUE;
                    http_message_body.message_data_length = (uint16_t)( strstr( (char*)http_message_body.data, FINAL_CHUNKED_PACKET ) - (char*)http_message_body.data );
                }
                else
                {
                    message_data_length_string = strstr( request_string, HTTP_HEADER_CONTENT_LENGTH );

                    if ( ( message_data_length_string != NULL ) &&
                         ( message_data_length_string < (char*)http_message_body.data ) )
                    {
                        http_message_body.message_data_length = (uint16_t)( (uint8_t*)( request_string + request_length ) - http_message_body.data );

                        message_data_length_string+= ( sizeof( HTTP_HEADER_CONTENT_LENGTH ) -1 );

                        http_message_body.total_message_data_remaining = (uint16_t)strtol( message_data_length_string, NULL, 10 );

                        http_message_body.total_message_data_remaining = (uint16_t)( http_message_body.total_message_data_remaining - http_message_body.message_data_length );

                        if ( http_message_body.total_message_data_remaining )
                        {
                            /* If data is split across packets, save the url and set data state for this socket*/
                            wiced_http_server_set_http_message_body_type_for_socket( socket, start_of_url, new_url_length, &http_message_body );
                        }
                    }
                    else
                    {
                        http_message_body.message_data_length = 0;
                    }

                }

                result = process_url_request( (wiced_tcp_socket_t*)socket, page_database, start_of_url, new_url_length, &http_message_body );

                *close_request_received = wiced_http_get_socket_close_request();

                processing_http_packet = WICED_FALSE;
                break;

            default:
                return WICED_ERROR;
                break;
        }
    }

    return result;
}

static wiced_result_t process_url_request( wiced_tcp_socket_t* socket, const wiced_http_page_t* server_url_list, char* url, int url_len, wiced_http_message_body_t* http_message_body )
{
    wiced_tcp_stream_t       stream;
    wiced_packet_mime_type_t server_mime;
    http_status_codes_t      status_code;
    char*                    url_query_parameters = url;
    int                      query_length         = url_len;
    int                      i                    = 0;

    url[url_len]='\x00';

    while ( ( *url_query_parameters != '?' ) &&
            ( query_length > 0 )             &&
            ( *url_query_parameters != '\0' ) )
    {
        url_query_parameters++;
        query_length--;
    }


    if ( query_length != 0 )
    {
        url_len =  url_len - query_length;
    }
    else
    {
        url_query_parameters = url;
    }

    WPRINT_WEBSERVER_DEBUG(("Processing request for: %s\n", url));

    /* Init the tcp stream */
    wiced_tcp_stream_init( &stream, socket );

    status_code = HTTP_404_TYPE;

    /* Search URL list to determine if request matches one of our pages, and break out when found */
    while ( server_url_list[i].url != NULL )
    {
        if ( ( strncasecmp( server_url_list[i].url, url, (uint32_t)url_len ) == COMPARE_MATCH ) )
        {
            server_mime = wiced_get_mime_type( server_url_list[i].mime_type );

            if( ( server_mime == http_message_body->mime_type ) || ( http_message_body->mime_type == MIME_TYPE_ALL ) )
            {
                status_code = HTTP_200_TYPE;
                break;
            }
        }
        i++;
    }

    if ( status_code == HTTP_200_TYPE )
    {

         /* Call the content handler function to write the page content into the packet and adjust the write pointers */
        switch ( server_url_list[ i ].url_content_type )
        {
             case WICED_DYNAMIC_URL_CONTENT:
                 /* message packaging into stream is done through wiced_http_write_dynamic_response_frame API called from callback */
                 *url_query_parameters = '\x00';
                 url_query_parameters++;

                 wiced_http_write_reply_header ( &stream, status_code, CHUNKED_TRANSFER_TRUE, NO_CONTENT_LENGTH, REQUEST_NO_CACHE,  server_mime );

                 wiced_http_turn_on_chunking ( &stream );

                 server_url_list[i].url_content.dynamic_data.generator( url_query_parameters, &stream, server_url_list[i].url_content.dynamic_data.arg, http_message_body );

                 wiced_http_turn_off_chunking ( &stream );
                 break;

             case WICED_RAW_DYNAMIC_URL_CONTENT:
                 server_url_list[i].url_content.dynamic_data.generator( url, &stream, server_url_list[i].url_content.dynamic_data.arg, http_message_body );
                 break;

             case WICED_STATIC_URL_CONTENT:
                 wiced_http_write_reply_header( &stream, status_code, CHUNKED_TRANSFER_FALSE, server_url_list[i].url_content.static_data.length, ALLOW_CACHE, server_mime );
                 wiced_tcp_stream_write       ( &stream, server_url_list[ i ].url_content.static_data.ptr, server_url_list[ i ].url_content.static_data.length );
                 break;

             case WICED_RAW_STATIC_URL_CONTENT: /* This is just a Location header */
                 wiced_tcp_stream_write( &stream, HTTP_HEADER_301, strlen( HTTP_HEADER_301 ) );
                 wiced_tcp_stream_write( &stream, CRLF, strlen( CRLF ) );
                 wiced_tcp_stream_write( &stream, HTTP_HEADER_LOCATION, strlen( HTTP_HEADER_LOCATION ) );
                 wiced_tcp_stream_write( &stream, server_url_list[i].url_content.static_data.ptr, server_url_list[i].url_content.static_data.length  );
                 wiced_tcp_stream_write( &stream, CRLF, strlen( CRLF ) );
                 wiced_tcp_stream_write( &stream, HTTP_HEADER_CONTENT_LENGTH, strlen( HTTP_HEADER_CONTENT_LENGTH ) );
                 wiced_tcp_stream_write( &stream, "0", 1 );
                 wiced_tcp_stream_write( &stream, CRLF_CRLF, strlen( CRLF_CRLF ) );
                 break;

             case WICED_RESOURCE_URL_CONTENT:
                 /* Fall through */
             case WICED_RAW_RESOURCE_URL_CONTENT:
                 wiced_http_write_reply_header  ( &stream,  status_code, CHUNKED_TRANSFER_TRUE, NO_CONTENT_LENGTH, WICED_FALSE, server_mime );
                 wiced_http_turn_on_chunking    ( &stream );
                 wiced_tcp_stream_write_resource( &stream, server_url_list[i].url_content.resource_data );
                 wiced_http_turn_off_chunking   ( &stream );
                 break;

             default:
                wiced_assert("Unknown entry in URL list", 0 != 0 );
                break;
         }
    }


    WICED_VERIFY( wiced_tcp_stream_flush ( &stream ) );

    if ( status_code >= HTTP_400_TYPE )
    {
        wiced_http_write_reply_header ( &stream, status_code, CHUNKED_TRANSFER_FALSE, NO_CONTENT_LENGTH, WICED_FALSE, MIME_TYPE_TEXT_HTML );
        WICED_VERIFY( wiced_tcp_stream_flush ( &stream ) );
    }

    wiced_tcp_stream_deinit( &stream );

    wiced_assert( "Page Serve finished with data still in stream", stream.tx_packet == NULL );

    return WICED_SUCCESS;
}


wiced_result_t wiced_http_write_reply_header (wiced_tcp_stream_t* stream, http_status_codes_t status_code, wiced_bool_t chunked, uint32_t content_length, wiced_bool_t cached, wiced_packet_mime_type_t mime_type )
{
    char data_length_string[15];

    memset( data_length_string, 0x0 , sizeof( data_length_string ) );

    switch ( status_code )
    {
        case HTTP_200_TYPE:
            /*HTTP/1.1 200 OK\r\n*/
            wiced_tcp_stream_write( stream, HTTP_HEADER_200, sizeof( HTTP_HEADER_200 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_204_TYPE:
            /*HTTP/1.1 204 OK\r\n*/
            wiced_tcp_stream_write( stream, HTTP_HEADER_204, sizeof( HTTP_HEADER_204 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_207_TYPE:
            /*HTTP/1.1 200 OK\r\n*/
            wiced_tcp_stream_write( stream, HTTP_HEADER_207, sizeof( HTTP_HEADER_207 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_301_TYPE:
        case HTTP_404_TYPE:
            /* HTTP/1.1 301 */
            wiced_tcp_stream_write( stream, HTTP_HEADER_301, sizeof( HTTP_HEADER_301 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );

            /* Location: url */
            wiced_tcp_stream_write( stream, HTTP_HEADER_LOCATION, sizeof( HTTP_HEADER_LOCATION )-1 );
            wiced_tcp_stream_write( stream, "/", 1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_400_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_400, sizeof( HTTP_HEADER_400 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_403_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_403, sizeof( HTTP_HEADER_403 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_405_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_405, sizeof( HTTP_HEADER_405 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_406_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_406, sizeof( HTTP_HEADER_406 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_429_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_429, sizeof( HTTP_HEADER_429 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1);
            break;

        case HTTP_444_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_444, sizeof( HTTP_HEADER_444 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_470_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_470, sizeof( HTTP_HEADER_470 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        case HTTP_500_TYPE:
            wiced_tcp_stream_write( stream, HTTP_HEADER_500, sizeof( HTTP_HEADER_500 )-1 );
            wiced_tcp_stream_write( stream, CRLF, sizeof( CRLF )-1 );
            break;

        default:
            break;
    }

    /* Content-Type: xx/yy\r\n */
    wiced_tcp_stream_write( stream, HTTP_HEADER_CONTENT_TYPE, strlen( HTTP_HEADER_CONTENT_TYPE ) );
    wiced_tcp_stream_write( stream, http_mime_array[mime_type], strlen( http_mime_array[mime_type] ) );
    wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );

    if ( cached == REQUEST_NO_CACHE )
    {
        wiced_tcp_stream_write( stream, NO_CACHE_HEADER, strlen( NO_CACHE_HEADER ) );
        wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );
    }

    if ( status_code == HTTP_444_TYPE )
    {
        /* Connection: close */
        wiced_tcp_stream_write( stream, HTTP_HEADER_CLOSE, strlen( HTTP_HEADER_CLOSE ) );
        wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );
    }
    else
    {
        wiced_tcp_stream_write( stream, HTTP_HEADER_KEEP_ALIVE, strlen( HTTP_HEADER_KEEP_ALIVE ) );
        wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );
    }

    if ( chunked == CHUNKED_TRANSFER_TRUE )
    {
        wiced_tcp_stream_write( stream, HTTP_HEADER_CHUNKED, strlen( HTTP_HEADER_CHUNKED ) );
        wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );
    }
    /* Content-Length: xx\r\n */
    else if ( content_length )
    {
        sprintf( data_length_string,"%lu", (long)content_length );
        wiced_tcp_stream_write( stream, HTTP_HEADER_CONTENT_LENGTH, strlen( HTTP_HEADER_CONTENT_LENGTH ) );
        wiced_tcp_stream_write( stream, data_length_string, strlen(data_length_string) );
        wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );
    }

    /* Closing sequence */
    wiced_tcp_stream_write( stream, CRLF, strlen( CRLF ) );

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_chunk_response (void* stream, const void* data, uint32_t data_length )
{
    char                data_length_string[5];
    wiced_tcp_stream_t* local_stream = (wiced_tcp_stream_t*)stream;

    memset( data_length_string, 0x0, sizeof(data_length_string) );

    local_stream->use_custom_tcp_stream = WICED_FALSE;

    /* Add chunked transfer coding fields */
    if ( data_length != 0 )
    {
        sprintf(data_length_string,"%lx",data_length);
        wiced_tcp_stream_write( local_stream, data_length_string, strlen( data_length_string ) );
        wiced_tcp_stream_write( local_stream, CRLF, sizeof( CRLF )-1 );
        wiced_tcp_stream_write( local_stream, data, (uint16_t) data_length );
        wiced_tcp_stream_write( local_stream, CRLF, sizeof( CRLF )-1 );
    }

    local_stream->use_custom_tcp_stream = WICED_TRUE;

    return WICED_SUCCESS;
}

static uint16_t escaped_string_copy( char* output, uint16_t output_length, const char* input, uint16_t input_length )
{
    uint16_t bytes_copied;
    int      a;

    for ( bytes_copied = 0; ( input_length > 0 ) && ( bytes_copied != output_length ); ++bytes_copied )
    {
        if ( *input == '%' )
        {
            if ( *( input + 1 ) == '%' )
            {
                ++input;
                *output = '%';
            }
            else
            {
                *output = 0;
                for ( a = 0; a < 2; ++a )
                {
                    *output = (char) ( *output << 4 );
                    ++input;
                    if (*input >= '0' && *input <= '9')
                    {
                        *output = (char) ( *output + *input - '0' );
                    }
                    else if (*input >= 'a' && *input <= 'f')
                    {
                        *output = (char) ( *output + *input - 'a' + 10 );
                    }
                    else if (*input >= 'A' && *input <= 'F')
                    {
                        *output = (char) ( *output + *input - 'A' + 10 );
                    }
                }
                input_length = (uint16_t) ( input_length - 3 );
            }
        }
        else
        {
            *output = *input;
            --input_length;
        }
        ++output;
        ++input;
    }

    return bytes_copied;
}

static wiced_packet_mime_type_t wiced_get_mime_type( const char* request_data )
{
    wiced_packet_mime_type_t mime_type = MIME_TYPE_TLV;

    if ( request_data != NULL )
    {
        while ( mime_type < MIME_TYPE_ALL )
        {
            if ( strncmp( request_data, http_mime_array[mime_type],strlen( http_mime_array[mime_type]) ) == COMPARE_MATCH )
            {
                break;
            }
            mime_type++;
        }
    }
    else
    {
        /* If MIME not specified, assumed all supported (according to rfc2616)*/
        mime_type = MIME_TYPE_ALL;
    }
    return mime_type;
}

static wiced_result_t wiced_get_http_request_type_and_url( char* request, wiced_http_request_type_t* type, char** url_start, uint16_t* url_length )
{
    char* end_of_url;

    end_of_url = strstr( request, HTTP_1_1_TOKEN);
    if ( end_of_url == NULL )
    {
        return WICED_ERROR;
    }

    if ( memcmp( request, GET_TOKEN, sizeof( GET_TOKEN ) - 1 ) == COMPARE_MATCH )
    {
        /* Get type  */
        *type = WICED_HTTP_GET_REQUEST;
        *url_start  = request + sizeof( GET_TOKEN ) - 1 ;
        *url_length =(uint16_t)( end_of_url - *url_start );
    }
    else if ( memcmp( request, POST_TOKEN, sizeof( POST_TOKEN ) - 1 ) == COMPARE_MATCH )
    {
        *type = WICED_HTTP_POST_REQUEST;
        *url_start = request + sizeof( POST_TOKEN ) - 1 ;
        *url_length =(uint16_t)( end_of_url - *url_start );
    }
    else if ( memcmp( request, PUT_TOKEN, sizeof( PUT_TOKEN ) - 1 ) == COMPARE_MATCH )
    {
        *type = WICED_HTTP_PUT_REQUEST;
        *url_start = request + sizeof( PUT_TOKEN ) - 1 ;
        *url_length =(uint16_t)( end_of_url - *url_start );
    }
    else
    {
        *type = REQUEST_UNDEFINED;
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_turn_on_chunking ( wiced_tcp_stream_t* stream )
{
    stream->use_custom_tcp_stream     = WICED_TRUE;
    stream->tcp_stream_write_callback = wiced_http_chunk_response;

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_turn_off_chunking ( wiced_tcp_stream_t* stream )
{
    stream->use_custom_tcp_stream     = WICED_FALSE;
    stream->tcp_stream_write_callback = NULL;

    /* Send final chunked frame */
    wiced_tcp_stream_write( stream, FINAL_CHUNKED_PACKET, sizeof(FINAL_CHUNKED_PACKET)-1 );

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_server_register_packet_post_processing_function ( http_server_packet_process_callback_t custom_receive_callback )
{
    packet_process_callback = custom_receive_callback;

    return WICED_SUCCESS;
}

wiced_result_t wiced_http_server_deregister_packet_post_processing_function ( void )
{
    packet_process_callback = NULL;

    return WICED_SUCCESS;
}

static wiced_result_t wiced_http_server_get_packet_data( wiced_packet_t* packet, char** data, uint16_t* request_length )
{
    uint16_t       available_data_length;
    wiced_result_t result;

    result = wiced_packet_get_data( packet, 0, (uint8_t**)data, request_length, &available_data_length );

    if ( packet_process_callback != NULL )
    {
        result = packet_process_callback( (uint8_t**)data, request_length );
    }

    return  result;
}

static wiced_result_t wiced_http_server_set_http_message_body_type_for_socket( wiced_tcp_socket_t* socket, char* url, uint16_t url_length, wiced_http_message_body_t* message_body_descriptor )
{
    if ( socket_http_message_context_counter < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS )
    {
        socket_http_message_context[socket_http_message_context_counter].socket            = socket;
        socket_http_message_context[socket_http_message_context_counter].http_packet_state = HTTP_HEADER_AND_DATA_FRAME_STATE;

        memcpy( socket_http_message_context[socket_http_message_context_counter].url, url, url_length );

        socket_http_message_context[socket_http_message_context_counter].url_length              = url_length;
        socket_http_message_context[socket_http_message_context_counter].message_body_descriptor = *message_body_descriptor;

        socket_http_message_context_counter++;

        return WICED_SUCCESS;
    }

    return WICED_ERROR;
}

static wiced_http_packet_state_t wiced_get_http_packet_state_and_socket_context( wiced_tcp_socket_t* socket, wiced_http_message_context_t* socket_http_context )
{
    uint16_t i;

    for ( i = 0; i < WICED_MAXIMUM_NUMBER_OF_SOCKETS_WITH_CALLBACKS; i++ )
    {
        if ( socket_http_message_context[i].socket == socket )
        {
            socket_http_context = &socket_http_message_context[i];
            return socket_http_context->http_packet_state;
        }
    }

    socket_http_context = NULL;

    return HTTP_HEADER_AND_DATA_FRAME_STATE;
}

static wiced_result_t wiced_reset_current_message_body_context_for_socket( wiced_tcp_socket_t* socket )
{
    wiced_http_message_context_t* http_message_context = NULL;
    wiced_result_t                result;

    result = wiced_get_http_packet_state_and_socket_context( socket, http_message_context );

    if ( ( socket_http_message_context_counter ) && ( result == WICED_SUCCESS ) )
    {
        http_message_context->http_packet_state = HTTP_HEADER_AND_DATA_FRAME_STATE;

        socket_http_message_context_counter--;

        return WICED_SUCCESS;
    }

    return WICED_ERROR;
}

wiced_result_t wiced_http_current_socket_close_request( void )
{
    external_close_request_received = WICED_TRUE;

    return WICED_SUCCESS;
}

static wiced_result_t wiced_http_reset_socket_close_request ( void )
{
    external_close_request_received = WICED_FALSE;

    return WICED_SUCCESS;
}

static wiced_bool_t wiced_http_get_socket_close_request ( void )
{
    return external_close_request_received;
}

static void http_server_thread_main(uint32_t arg)
{
    wiced_http_server_t*   server                    = (wiced_http_server_t*) arg;
    wiced_packet_t*        packet                    = NULL;
    wiced_bool_t           close_request_from_client = WICED_FALSE;
    server_event_message_t current_event;

    while ( server->quit != WICED_TRUE )
    {
        wiced_rtos_pop_from_queue( &server_event_queue, &current_event, WICED_NEVER_TIMEOUT );

          switch( current_event.event_type )
          {
              case SOCKET_DISCONNECT_EVENT:
                  wiced_tcp_server_disconnect_socket( &server->tcp_server, current_event.socket );
                  break;

              case SOCKET_CONNECT_EVENT:
                  wiced_tcp_server_accept( &server->tcp_server, current_event.socket );
                  break;

              case SOCKET_MESSAGE_EVENT:
                  wiced_tcp_receive( current_event.socket, &packet, WICED_NO_WAIT );

                  if ( packet != NULL )
                  {
                      http_server_parse_packet( server->page_database, current_event.socket , packet, &close_request_from_client );

                      wiced_packet_delete( packet );

                      if ( close_request_from_client )
                      {
                          wiced_tcp_server_disconnect_socket( &server->tcp_server, current_event.socket );
                      }
                  }

                  break;

              default:
                  break;
          }
    }

    wiced_tcp_server_stop( &server->tcp_server );
    wiced_rtos_deinit_queue( &server_event_queue );
    WICED_END_OF_CURRENT_THREAD( );
}

