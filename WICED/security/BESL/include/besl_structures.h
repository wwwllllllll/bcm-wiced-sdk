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

#include <stdint.h>

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

#ifndef RESULT_ENUM
#define RESULT_ENUM( prefix, name, value )  prefix ## name = (value)
#endif /* ifndef RESULT_ENUM */

/*
 * Values: 3000 - 3999
 */
#define WPS_BESL_RESULT_LIST( prefix ) \
    RESULT_ENUM( prefix, SUCCESS,                                    0 ),   /**< Success */          \
    RESULT_ENUM( prefix, TIMEOUT,                                    2 ),   /**< Timeout */          \
    RESULT_ENUM( prefix, UNPROCESSED,                             3001 ),   /**<  */                 \
    RESULT_ENUM( prefix, IN_PROGRESS,                             3002 ),   /**< In progress */      \
    RESULT_ENUM( prefix, COMPLETE,                                3003 ),   /**<   */ \
    RESULT_ENUM( prefix, PBC_OVERLAP,                             3004 ),   /**<   */ \
    RESULT_ENUM( prefix, ABORTED,                                 3005 ),   /**<   */ \
    RESULT_ENUM( prefix, NOT_STARTED,                             3006 ),   /**<   */ \
    RESULT_ENUM( prefix, REGISTRAR_NOT_READY,                     3007 ),   /**<   */ \
    RESULT_ENUM( prefix, ATTEMPTED_EXTERNAL_REGISTRAR_DISCOVERY,  3008 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_JOIN_FAILED,                       3009 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_ENCRYPTED_TLV_HMAC_FAIL,           3010 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_SECRET_NONCE_MISMATCH,             3011 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_MESSAGE_HMAC_FAIL,                 3012 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_MESSAGE_TLV_MASK_MISMATCH,         3013 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_ENCRYPTION_TYPE_ERROR,             3014 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_AUTHENTICATION_TYPE_ERROR,         3015 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_REGISTRAR_NONCE_MISMATCH,          3016 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_ENROLLEE_NONCE_MISMATCH,           3017 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_VERSION_MISMATCH,                  3018 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_CRYPTO_ERROR,                      3019 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_MESSAGE_MISSING_TLV,               3020 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_INCORRECT_MESSAGE,                 3021 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_NO_RESPONSE,                       3022 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_RECEIVED_EAP_FAIL,                 3023 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_RECEIVED_WEP_CREDENTIALS,          3024 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_OUT_OF_MEMORY,                     3025 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_QUEUE_PUSH,                        3026 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_SCAN_START_FAIL,                   3027 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_WPS_STACK_MALLOC_FAIL,             3028 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_CREATING_EAPOL_PACKET,             3029 ),   /**<   */ \
    RESULT_ENUM( prefix, UNKNOWN_EVENT,                           3033 ),   /**<   */ \
    RESULT_ENUM( prefix, OUT_OF_HEAP_SPACE,                       3034 ),   /**<   */ \
    RESULT_ENUM( prefix, CERT_PARSE_FAIL,                         3035 ),   /**<   */ \
    RESULT_ENUM( prefix, KEY_PARSE_FAIL,                          3036 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_DEVICE_LIST_FIND,                  3037 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_NO_P2P_TLV,                        3038 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_ALREADY_STARTED,                   3039 ),   /**<   */




#define TLS_RESULT_LIST( prefix ) \
    RESULT_ENUM( prefix, SUCCESS,                                0 ),   /**<   */ \
    RESULT_ENUM( prefix, TIMEOUT,                                2 ),   /**<   */ \
    RESULT_ENUM( prefix, RECEIVE_FAILED,                      4001 ),   /**<   */ \
    RESULT_ENUM( prefix, ALERT_NO_CERTIFICATE,                4002 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_OUT_OF_MEMORY,                 4003 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_FEATURE_UNAVAILABLE,           4004 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_INPUT_DATA,                4005 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_INVALID_MAC,                   4006 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_INVALID_RECORD,                4007 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_INVALID_MODULUS_SIZE,          4008 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_UNKNOWN_CIPHER,                4009 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_NO_CIPHER_CHOSEN,              4010 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_NO_SESSION_FOUND,              4011 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_NO_CLIENT_CERTIFICATE,         4012 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_CERTIFICATE_TOO_LARGE,         4013 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_CERTIFICATE_REQUIRED,          4014 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_PRIVATE_KEY_REQUIRED,          4015 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_CA_CHAIN_REQUIRED,             4016 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_UNEXPECTED_MESSAGE,            4017 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_FATAL_ALERT_MESSAGE,           4018 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_PEER_VERIFY_FAILED,            4019 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_PEER_CLOSE_NOTIFY,             4020 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_CLIENT_HELLO,           4021 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_SERVER_HELLO,           4022 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_CERTIFICATE,            4023 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_CERTIFICATE_REQUEST,    4024 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_SERVER_KEY_EXCHANGE,    4025 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_SERVER_HELLO_DONE,      4026 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_CLIENT_KEY_EXCHANGE,    4027 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_CERTIFICATE_VERIFY,     4028 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_CHANGE_CIPHER_SPEC,     4029 ),   /**<   */ \
    RESULT_ENUM( prefix, ERROR_BAD_HS_FINISHED,               4030 ),   /**<   */ \
    RESULT_ENUM( prefix, HANDSHAKE_TIMEOUT,                   4031 ),   /**<   */ \
    RESULT_ENUM( prefix, HANDSHAKE_ERROR,                     4032 ),   /**<   */ \
    RESULT_ENUM( prefix, INIT_FAIL,                           4033 ),   /**<   */ \
    RESULT_ENUM( prefix, BAD_MESSAGE,                         4034 ),   /**<   */



typedef enum
{
    WPS_BESL_RESULT_LIST( BESL_ )
    TLS_RESULT_LIST( BESL_TLS )

    BESL_RESULT_FORCE_32_BIT = 0x7FFFFFFF
} besl_result_t;


/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef uint8_t   besl_bool_t;

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)

typedef struct
{
    uint8_t octet[6];
} besl_mac_t;

typedef struct
{
    uint8_t* data;
    uint32_t length;
    uint32_t packet_mask;
} besl_ie_t;

#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
