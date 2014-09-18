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
 * Only client websockets are implemented currently
 ******************************************************/

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define SUB_PROTOCOL_STRING_LENGTH 10

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    WEBSOCKET_CONNECTING = 0, /* The connection has not yet been established */
    WEBSOCKET_OPEN          , /* The WebSocket connection is established and communication is possible */
    WEBSOCKET_CLOSING       , /* The connection is going through the closing handshake */
    WEBSOCKET_CLOSED        , /* The connection has been closed or could not be opened */
    WEBSOCKET_NOT_REGISTERED
} wiced_websocket_state_t;

typedef enum
{
    WEBSOCKET_CONTINUATION_FRAME = 0,
    WEBSOCKET_TEXT_FRAME            ,
    WEBSOCKET_BINARY_FRAME          ,
    WEBSOCKET_RESERVED_3            ,
    WEBSOCKET_RESERVED_4            ,
    WEBSOCKET_RESERVED_5            ,
    WEBSOCKET_RESERVED_6            ,
    WEBSOCKET_RESERVED_7            ,
    WEBSOCKET_CONNECTION_CLOSE      ,
    WEBSOCKET_PING                  ,
    WEBSOCKET_PONG                  ,
    WEBSOCKET_RESERVED_B            ,
    WEBSOCKET_RESERVED_C            ,
    WEBSOCKET_RESERVED_D            ,
    WEBSOCKET_RESERVED_E            ,
    WEBSOCKET_RESERVED_F
} wiced_websocket_payload_type_t;

typedef enum
{
    WEBSOCKET_NO_ERROR=0,                           /* Error: 0 */
    WEBSOCKET_CLIENT_CONNECT_ERROR,                 /* Error: 1 */
    WEBSOCKET_NO_AVAILABLE_SOCKET,                  /* Error: 2 */
    WEBSOCKET_SERVER_HANDSHAKE_RESPONSE_INVALID,    /* Error: 3 */
    WEBSOCKET_CREATE_SOCKET_ERROR,                  /* Error: 4 */
    WEBSOCKET_FRAME_SEND_ERROR,                     /* Error: 5 */
    WEBSOCKET_HANDSHAKE_SEND_ERROR,                 /* Error: 6 */
    WEBSOCKET_PONG_SEND_ERROR,                      /* Error: 7 */
    WEBSOCKET_RECEIVE_ERROR,                        /* Error: 8 */
    WEBSOCKET_DNS_RESOLVE_ERROR,                    /* Error: 9 */
    WEBSOCKET_SUBPROTOCOL_NOT_SUPPORTED             /* Error: 10 */
} wiced_websocket_error_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    wiced_tcp_socket_t        socket;
    wiced_socket_callback_t   on_open;
    wiced_socket_callback_t   on_error;
    wiced_socket_callback_t   on_close;
    wiced_socket_callback_t   on_message;
    wiced_websocket_error_t   error_type;
    wiced_websocket_state_t   state;
    const char                subprotocol[SUB_PROTOCOL_STRING_LENGTH];
} wiced_websocket_t;

typedef struct
{
    wiced_bool_t                    final_frame;
    wiced_websocket_payload_type_t  payload_type;
    uint16_t                        payload_length;
    void*                           payload;
} wiced_websocket_frame_t;

typedef struct
{
    char* request_uri;
    char* host;
    char* origin;
    char* sec_websocket_protocol;
} wiced_websocket_handshake_fields_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/*
 * @brief                          perform opening handshake on port 80 with server and establish a connection
 *
 * @param websocket                websocket identifier
 * @param websocket_header         http header information to be used in handshake
 *
 * @return                         WICED_SUCCESS if successful, or WICED_ERROR.
 *
 * @note                           For additional error information, check the wiced_websocket_error_t field
 *                                 of the  wiced_websocket_t structure
 */
wiced_result_t wiced_ws_connect( wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header );

/*
 * @brief                          perform opening handshake on port 443 with server and establish a connection
 *
 * @param websocket                websocket identifier
 * @param websocket_header         http header information to be used in handshake
 *
 * @return                         WICED_SUCCESS if successful, or WICED_ERROR.
 *
 * @note                           For additional error information, check the wiced_websocket_error_t field
 *                                 of the  wiced_websocket_t structure
 */
wiced_result_t wiced_wss_connect( wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header );

/*
 * @brief                           send data to websocket server
 *
 * @param websocket                 websocket to send on
 * @param tx_frame                  tx_frame to send
 *
 * @return                          WICED_SUCCESS if successful, or WICED_ERROR.
 *
 */
wiced_result_t wiced_websocket_send ( wiced_websocket_t* websocket, wiced_websocket_frame_t* tx_frame );

/*
 * @brief                           receive data from websocket server
 *
 * @param websocket                 websocket to receive on
 * @param tx_frame                  tx_frame to send
 *
 * @return                          WICED_SUCCESS if successful, or WICED_ERROR.
 *
 */
wiced_result_t wiced_websocket_receive ( wiced_websocket_t* websocket, wiced_websocket_frame_t* websocket_frame );

/*
 * @brief                           close and delete websocket, and send close message to websocket server
 *
 * @param websocket                 websocket to close
 * @param optional_close_message    optional closing message to send server.
 *
 * @return                          WICED_SUCCESS if successful, or WICED_ERROR.
 *
 */
wiced_result_t wiced_websocket_close ( wiced_websocket_t* websocket, const char* optional_close_message) ;

/*
 * @brief                           get websocket state
 * *
 * @param websocket                 request state for this websocket
 *
 * @return wiced_websocket_state_t  is socket open/close/connecting.
 *
 * @note                            none
 */

#ifdef __cplusplus
} /* extern "C" */
#endif






