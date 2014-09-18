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

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef wiced_tcp_stream_t   xively_tcp_stream_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    const char*                id;
    const char*                api_key;
    wiced_tcp_socket_t         socket;
    wiced_tls_simple_context_t tls_context;
} xively_feed_t;

typedef struct
{
    const char*        channel_name;
    wiced_tcp_stream_t tcp_stream;
    uint32_t           data_size;
    uint32_t           number_of_datapoints;
} xively_datastream_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

extern wiced_result_t xively_open_feed         ( xively_feed_t* feed );
extern wiced_result_t xively_close_feed        ( xively_feed_t* feed );
extern wiced_result_t xively_create_datastream ( xively_feed_t* feed, xively_datastream_t* stream, const char* channel_name, uint32_t data_size, uint32_t number_of_datapoints );
extern wiced_result_t xively_write_datapoint   ( xively_datastream_t* stream, const uint8_t* data, const wiced_iso8601_time_t* timestamp );
extern wiced_result_t xively_flush_datastream  ( xively_datastream_t* stream );
extern void           xively_u16toa            ( uint16_t value, char* output, uint8_t min_length );

#ifdef __cplusplus
} /* extern "C" */
#endif
