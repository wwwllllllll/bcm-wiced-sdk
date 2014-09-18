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
#include "websocket.h"
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************
 *                      Macros
 ******************************************************/
#define SHA_LENGTH              20
#define WEBSOCKET_KEY_LENGTH    16
/******************************************************
 * Base 64 encoding adds 1 byte for every 3 bytes being encoded
 * Also add 2 bytes for padding and 1 byte for null character
 *********************************************************/
#define CLIENT_WEBSOCKET_BASE64_KEY_LENGTH (WEBSOCKET_KEY_LENGTH+10)
/*The server websocket key involves concatenation of GUID, hence the longer length*/
#define SERVER_WEBSOCKET_BASE64_SHA1_KEY_LENGTH (SHA_LENGTH + 12)
/******************************************************
 *                    Constants
 ******************************************************/

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/*
 * @brief                               generate opening handshake header, send handshake and verify server response
 *
 * @param websocket                     websocket to send handshake on
 * @param websocket_header_fields       application populated header fields
 *
 * @return                              WICED_SUCCESS if successful, or WICED_ERROR.
 *
 */
wiced_result_t wiced_establish_websocket_handshake( wiced_websocket_t* websocket, wiced_websocket_handshake_fields_t* websocket_header_fields );

/*
 * @brief                               get the subprotocol negotiated with server (assuming it was requested)
 *
 * @param subprotocol                   subprotocol used
 *
 * @return                              WICED_SUCCESS if successful, or WICED_ERROR.
 *
 */
wiced_result_t wiced_get_websocket_subprotocol( const char* subprotocol );
#ifdef __cplusplus
} /* extern "C" */
#endif
