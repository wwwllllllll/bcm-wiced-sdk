/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "wiced.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_HTTP_HOST_NAME_SIZE         ( 40 )

#define HTTP_NEW_LINE                   "\r\n"
#define HTTP_HEADERS_BODY_SEPARATOR     "\r\n\r\n"

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    HTTP_GET  = 0,
    HTTP_POST = 1,
    HTTP_HEAD = 2,
    HTTP_END  = 3,
} http_request_t;

typedef enum
{
    HTTP_CONTINUE                        = 100,
    HTTP_SWITCHING_PROTOCOLS             = 101,
    HTTP_RESPONSE_OK                     = 200,
    HTTP_CREATED                         = 201,
    HTTP_ACCEPTED                        = 202,
    HTTP_NONAUTHORITATIVE                = 203,
    HTTP_NO_CONTENT                      = 204,
    HTTP_RESET_CONTENT                   = 205,
    HTTP_PARTIAL_CONTENT                 = 206,
    HTTP_MULTIPLE_CHOICES                = 300,
    HTTP_MOVED_PERMANENTLY               = 301,
    HTTP_FOUND                           = 302,
    HTTP_SEE_OTHER                       = 303,
    HTTP_NOT_MODIFIED                    = 304,
    HTTP_USEPROXY                        = 305,
    HTTP_TEMPORARY_REDIRECT              = 307,
    HTTP_BAD_REQUEST                     = 400,
    HTTP_UNAUTHORIZED                    = 401,
    HTTP_PAYMENT_REQUIRED                = 402,
    HTTP_FORBIDDEN                       = 403,
    HTTP_NOT_FOUND                       = 404,
    HTTP_METHOD_NOT_ALLOWED              = 405,
    HTTP_NOT_ACCEPTABLE                  = 406,
    HTTP_PROXY_AUTHENTICATION_REQUIRED   = 407,
    HTTP_REQUEST_TIMEOUT                 = 408,
    HTTP_CONFLICT                        = 409,
    HTTP_GONE                            = 410,
    HTTP_LENGTH_REQUIRED                 = 411,
    HTTP_PRECONDITION_FAILED             = 412,
    HTTP_REQUESTENTITYTOOLARGE           = 413,
    HTTP_REQUESTURITOOLONG               = 414,
    HTTP_UNSUPPORTEDMEDIATYPE            = 415,
    HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    HTTP_EXPECTATION_FAILED              = 417,
    HTTP_INTERNAL_SERVER_ERROR           = 500,
    HTTP_NOT_IMPLEMENTED                 = 501,
    HTTP_BAD_GATEWAY                     = 502,
    HTTP_SERVICE_UNAVAILABLE             = 503,
    HTTP_GATEWAY_TIMEOUT                 = 504,
    HTTP_VERSION_NOT_SUPPORTED           = 505,
} http_status_code_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_bool_t       is_connected;
    wiced_ip_address_t host_ip;
    wiced_tcp_socket_t socket;
    wiced_tcp_stream_t tcp_stream;
    wiced_packet_t*    packet;
    char               hostname[MAX_HTTP_HOST_NAME_SIZE];
} http_stream_t;

typedef struct
{
    const char* name;
    char* value;
} http_header_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/* HTTP stream management functions */
wiced_result_t http_init_stream  ( http_stream_t* session, const char* host_name, wiced_ip_address_t* ip_address, uint16_t port );
wiced_result_t http_deinit_stream( http_stream_t* session );

/* HTTP stream header functions */
wiced_result_t http_stream_start_headers    ( http_stream_t* session, http_request_t method, const char* url );
wiced_result_t http_stream_add_headers      ( http_stream_t* session, const http_header_t* headers, uint16_t number_of_headers );
wiced_result_t http_send_basic_authorization( http_stream_t* session, char* username_password );
wiced_result_t http_stream_end_headers      ( http_stream_t* session, wiced_bool_t keep_alive);

/* HTTP stream writing functions */
wiced_result_t http_stream_write( http_stream_t* session, uint8_t* data, uint16_t length );
wiced_result_t http_stream_flush( http_stream_t* session );

/* HTTP stream reading and processing functions */
wiced_result_t http_stream_receive  ( http_stream_t* session, wiced_packet_t** packet, uint32_t timeout);
wiced_result_t http_extract_headers ( wiced_packet_t* packet, http_header_t* headers, uint16_t number_of_headers );
wiced_result_t http_get_body        ( wiced_packet_t* packet, uint8_t** body, uint32_t* body_length );
wiced_result_t http_process_response( wiced_packet_t* packet, http_status_code_t* response_code );

#ifdef __cplusplus
} /* extern "C" */
#endif
