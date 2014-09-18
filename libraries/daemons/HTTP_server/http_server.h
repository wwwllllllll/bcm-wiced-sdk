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

#define HTTP_REDIRECT(url) "HTTP/1.0 301\r\nLocation: " url "\r\n\r\n"

#define HTTP_404 \
    "HTTP/1.0 404 Not Found\r\n" \
    "Content-Type: text/html\r\n\r\n" \
    "<!doctype html>\n" \
    "<html><head><title>404 - WICED Web Server</title></head><body>\n" \
    "<h1>Address not found on WICED Web Server</h1>\n" \
    "<p><a href=\"/\">Return to home page</a></p>\n" \
    "</body>\n</html>\n"

#define START_OF_HTTP_PAGE_DATABASE(name) \
    const wiced_http_page_t name[] = {

#define ROOT_HTTP_PAGE_REDIRECT(url) \
    { "/", "", WICED_RAW_STATIC_URL_CONTENT, .url_content.static_data  = {HTTP_REDIRECT(url), sizeof(HTTP_REDIRECT(url))-1 } }

#define END_OF_HTTP_PAGE_DATABASE() \
    {0,0,0, .url_content.static_data  = {NULL, 0 } } \
    }

/******************************************************
 *                    Constants
 ******************************************************/

#define IOS_CAPTIVE_PORTAL_ADDRESS    "/library/test/success.html"

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef int (*url_processor_t)( const char* url, wiced_tcp_stream_t* stream, void* arg );

/******************************************************
 *                    Structures
 ******************************************************/

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

typedef struct
{
    wiced_tcp_socket_t       socket;
    volatile wiced_bool_t    quit;
    wiced_thread_t           thread;
    const wiced_http_page_t* page_database;
} wiced_http_server_t;

typedef struct
{
    wiced_tcp_socket_t         socket;
    volatile wiced_bool_t      quit;
    wiced_thread_t             thread;
    const wiced_http_page_t*   page_database;
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

wiced_result_t wiced_http_write_reply_header ( wiced_tcp_stream_t* stream, const char* mime_type, wiced_bool_t nocache );

#ifdef __cplusplus
} /* extern "C" */
#endif
