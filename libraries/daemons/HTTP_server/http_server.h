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

#include "wiced_defaults.h"
#include "wiced_tcpip.h"
#include "wiced_rtos.h"
#include "wiced_resource.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define START_OF_HTTP_PAGE_DATABASE(name) \
    const wiced_http_page_t name[] = {

#define ROOT_HTTP_PAGE_REDIRECT(url) \
    { "/", "text/html", WICED_RAW_STATIC_URL_CONTENT, .url_content.static_data  = {url, sizeof(url)-1 } }

#define END_OF_HTTP_PAGE_DATABASE() \
    {0,0,0, .url_content.static_data  = {NULL, 0 } } \
    }

#define EXPAND_AS_ENUMERATION(a,b)   a,
#define EXPAND_AS_MIME_TABLE(a,b)    b,

/******************************************************
 *                    Constants
 ******************************************************/

#define HTTP_404 \
    "HTTP/1.1 404 Not Found\r\n" \
    "Content-Type: text/html\r\n\r\n" \
    "<!doctype html>\n" \
    "<html><head><title>404 - WICED Web Server</title></head><body>\n" \
    "<h1>Address not found on WICED Web Server</h1>\n" \
    "<p><a href=\"/\">Return to home page</a></p>\n" \
    "</body>\n</html>\n"

#define MIME_TABLE( ENTRY ) \
    ENTRY( MIME_TYPE_TLV = 0 ,                "application/x-tlv8"               ) \
    ENTRY( MIME_TYPE_APPLE_BINARY_PLIST,      "application/x-apple-binary-plist" ) \
    ENTRY( MIME_TYPE_APPLE_PROXY_AUTOCONFIG,  "application/x-ns-proxy-autoconfig") \
    ENTRY( MIME_TYPE_BINARY_DATA,             "application/octet-stream"         ) \
    ENTRY( MIME_TYPE_JAVASCRIPT,              "application/javascript"           ) \
    ENTRY( MIME_TYPE_HAP_JSON,                "application/hap+json"             ) \
    ENTRY( MIME_TYPE_HAP_PAIRING,             "application/pairing+tlv8"         ) \
    ENTRY( MIME_TYPE_HAP_VERIFY,              "application/hap+verify"           ) \
    ENTRY( MIME_TYPE_TEXT_HTML,               "text/html"                        ) \
    ENTRY( MIME_TYPE_TEXT_PLAIN,              "text/plain"                       ) \
    ENTRY( MIME_TYPE_TEXT_CSS,                "text/css"                         ) \
    ENTRY( MIME_TYPE_IMAGE_PNG,               "image/png"                        ) \
    ENTRY( MIME_TYPE_IMAGE_GIF,               "image/gif"                        ) \
    ENTRY( MIME_TYPE_IMAGE_MICROSOFT,         "image/vnd.microsoft.icon"         ) \
    ENTRY( MIME_TYPE_ALL,                     "*/*"                              ) /* This must always be the last mimne*/

#define IOS_CAPTIVE_PORTAL_ADDRESS    "/library/test/success.html"
#define CHUNKED_TRANSFER_TRUE         WICED_TRUE
#define CHUNKED_TRANSFER_FALSE        WICED_FALSE

#define ALLOW_CACHE                   WICED_FALSE
#define REQUEST_NO_CACHE              WICED_TRUE

#define NO_CONTENT_LENGTH 0

#define HTTP_HEADER_200                "HTTP/1.1 200 OK"
#define HTTP_HEADER_204                "HTTP/1.1 204 No Content"
#define HTTP_HEADER_207                "HTTP/1.1 207 Multi Status"
#define HTTP_HEADER_301                "HTTP/1.1 301"
#define HTTP_HEADER_400                "HTTP/1.1 400 Bad Request"
#define HTTP_HEADER_403                "HTTP/1.1 403"
#define HTTP_HEADER_404                "HTTP/1.1 404 Not Found"
#define HTTP_HEADER_405                "HTTP/1.1 405 Method Not Allowed"
#define HTTP_HEADER_406                "HTTP/1.1 406"
#define HTTP_HEADER_429                "HTTP/1.1 429 Too Many Requests"
#define HTTP_HEADER_444                "HTTP/1.1 444"
#define HTTP_HEADER_470                "HTTP/1.1 470 Connection Authorization Required"
#define HTTP_HEADER_500                "HTTP/1.1 500 Internal Server Error"
#define HTTP_HEADER_CONTENT_LENGTH     "Content-Length: "
#define HTTP_HEADER_CONTENT_TYPE       "Content-Type: "
#define HTTP_HEADER_CHUNKED            "Transfer-Encoding: chunked"
#define HTTP_HEADER_LOCATION           "Location: "
#define HTTP_HEADER_ACCEPT             "Accept: "
#define HTTP_HEADER_KEEP_ALIVE         "Connection: Keep-Alive"
#define HTTP_HEADER_CLOSE              "Connection: close"
#define NO_CACHE_HEADER                "Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n"\
                                       "Pragma: no-cache"

#define CRLF                            "\r\n"
#define CRLF_CRLF                       "\r\n\r\n"

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    MIME_TABLE( EXPAND_AS_ENUMERATION )
    MIME_UNSUPPORTED
} wiced_packet_mime_type_t;

typedef enum
{
    HTTP_200_TYPE,
    HTTP_204_TYPE,
    HTTP_207_TYPE,
    HTTP_301_TYPE,
    HTTP_400_TYPE,
    HTTP_403_TYPE,
    HTTP_404_TYPE,
    HTTP_405_TYPE,
    HTTP_406_TYPE,
    HTTP_429_TYPE,
    HTTP_444_TYPE,
    HTTP_470_TYPE,
    HTTP_500_TYPE,
} http_status_codes_t;

typedef enum
{
    WICED_HTTP_GET_REQUEST,
    WICED_HTTP_POST_REQUEST,
    WICED_HTTP_PUT_REQUEST,
    REQUEST_UNDEFINED
} wiced_http_request_type_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    const uint8_t*            data;                      /* packet data in message body      */
    uint16_t                  message_data_length;       /* data length in current packet    */
    uint16_t                  total_message_data_remaining; /* data yet to be consumed       */
    wiced_bool_t              chunked_transfer;          /* chunked data format              */
    wiced_packet_mime_type_t  mime_type;                 /* mime type                        */
    wiced_http_request_type_t request_type;              /*GET, POST or PUT request          */
} wiced_http_message_body_t;

typedef int32_t (*url_processor_t)(  const char* url, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );

typedef struct
{
    const char* const url;
    const char* const mime_type;
    enum
    {
        WICED_STATIC_URL_CONTENT,
        WICED_DYNAMIC_URL_CONTENT,
        WICED_RESOURCE_URL_CONTENT,
        WICED_RAW_STATIC_URL_CONTENT,
        WICED_RAW_DYNAMIC_URL_CONTENT,
        WICED_RAW_RESOURCE_URL_CONTENT
    } url_content_type;
    union
    {
        struct
        {
            const url_processor_t generator;
            void*                 arg;
        } dynamic_data;
        struct
        {
            const void* ptr;
            uint32_t length;
        } static_data;
        const resource_hnd_t* resource_data;
    } url_content;
} wiced_http_page_t;

typedef wiced_result_t (*http_server_packet_process_callback_t)( uint8_t** data, uint16_t* data_length );

typedef struct
{
    wiced_tcp_server_t       tcp_server;
    wiced_thread_t           thread;
    volatile wiced_bool_t    quit;
    const wiced_http_page_t* page_database;
} wiced_http_server_t;

typedef struct
{
    wiced_tcp_server_t           tcp_server;
    volatile wiced_bool_t        quit;
    wiced_thread_t               thread;
    const wiced_http_page_t*     page_database;
    wiced_tls_advanced_context_t tls_context;
} wiced_https_server_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t wiced_http_server_start       ( wiced_http_server_t* server, uint16_t port, const wiced_http_page_t* page_database, wiced_interface_t interface );
wiced_result_t wiced_http_server_stop        ( wiced_http_server_t* server );

wiced_result_t wiced_https_server_start      ( wiced_https_server_t* server, uint16_t port, const wiced_http_page_t* page_database, const char* server_cert, const char* server_key, wiced_interface_t interface );
wiced_result_t wiced_https_server_stop       ( wiced_https_server_t* server );

wiced_result_t wiced_http_write_reply_header ( wiced_tcp_stream_t* stream, http_status_codes_t status_code, wiced_bool_t chunked, uint32_t content_length, wiced_bool_t cached, wiced_packet_mime_type_t mime_type );

wiced_result_t wiced_http_turn_on_chunking   ( wiced_tcp_stream_t* stream );

wiced_result_t wiced_http_turn_off_chunking  ( wiced_tcp_stream_t* stream );
wiced_result_t wiced_http_current_socket_close_request                      ( void );

wiced_result_t wiced_http_server_register_packet_post_processing_function   ( http_server_packet_process_callback_t custom_receive_callback );

wiced_result_t wiced_http_server_deregister_packet_post_processing_function ( void );

#ifdef __cplusplus
} /* extern "C" */
#endif
