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

#include "wiced_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MPAF_OPEN_TRANSPORT_CHANNEL_PARAMS_SIZE          1
#define MPAF_CLOSE_TRANSPORT_CHANNEL_PARAMS_SIZE         2
#define MPAF_GET_CONTROLLER_STATUS_PARAMS_SIZE           0
#define MPAF_RFCOMM_CREATE_CONNECTION_PARAMS_SIZE       44
#define MPAF_RFCOMM_REMOVE_CONNECTION_PARAMS_SIZE        1
#define MPAF_RFCOMM_CREATE_CONNECTION_CANCEL_PARAMS_SIZE 0

/* Controller Status */
#define MPAF_CONTROLLER_STATUS_RADIO_ON               0x01
#define MPAF_CONTROLLER_STATUS_CONNECTABLE            0x02
#define MPAF_CONTROLLER_STATUS_DISCOVERABLE           0x04
#define MPAF_CONTROLLER_STATUS_INQUIRING              0x08
#define MPAF_CONTROLLER_STATUS_CONNECTED              0x10
#define MPAF_CONTROLLER_STATUS_FATAL_ERROR            0x20
#define MPAF_CONTROLLER_STATUS_SHUTTING_DOWN          0x40

/* Feature Mask */
#define MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER_MASK       0x80
#define MPAF_DEFAULT_FEATURE_MASK                     0x00

#ifndef MPAF_COMMAND_PACKET_COUNT
#define MPAF_COMMAND_PACKET_COUNT (4)
#endif /* MPAF_COMMAND_PACKET_COUNT */

#ifndef MPAF_EVENT_PACKET_COUNT
#define MPAF_EVENT_PACKET_COUNT   (4)
#endif /* MPAF_EVENT_PACKET_COUNT */

#ifndef MPAF_DATA_PACKET_COUNT
#define MPAF_DATA_PACKET_COUNT    (8)
#endif /* MPAF_DATA_PACKET_COUNT */

#define MPAF_COMMAND_PARAMS_SIZE  ( MPAF_RFCOMM_CREATE_CONNECTION_PARAMS_SIZE ) /* Theoretical max size is 65KB. Set to longer known params length */
#define MPAF_EVENT_PARAMS_SIZE    ( sizeof(mpaf_event_params_t) )               /* Theoretical max size is 65KB. Set to longer known params length */

#ifndef MPAF_DATA_MTU_SIZE
#define MPAF_DATA_MTU_SIZE        (300) /* Theoretical max size is 65KB. Set to longer known application usage */
#endif /* HCI_ACL_DATA_SIZE */

#define MPAF_COMMAND_HEADER_SIZE  ( sizeof( mpaf_command_header_t ) )
#define MPAF_EVENT_HEADER_SIZE    ( sizeof( mpaf_event_header_t) )
#define MPAF_DATA_HEADER_SIZE     ( sizeof( mpaf_data_header_t ) )

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    MPAF_COMMAND_PACKET,
    MPAF_EVENT_PACKET,
    MPAF_DATA_PACKET,
} bt_mpaf_packet_type_t;

typedef enum
{
    MPAF_OPEN_TRANSPORT_CHANNEL           = 0x00,
    MPAF_CLOSE_TRANSPORT_CHANNEL          = 0x01,
    MPAF_GET_CONTROLLER_STATUS            = 0x02,
    MPAF_SET_MODE                         = 0x03,
    MPAF_RFCOMM_CREATE_CONNECTION         = 0x80,
    MPAF_RFCOMM_REMOVE_CONNECTION         = 0x81,
    MPAF_RFCOMM_CREATE_CONNECTION_CANCEL  = 0x82,
    MPAF_SDP_CREATE_RECORD                = 0xA0,
    MPAF_SDP_ADD_PROTOCOL_DESCRIPTOR_LIST = 0xA1,
    MPAF_SDP_ADD_ATTRIBUTES               = 0xA2,
    MPAF_SDP_ADD_UUID_SEQUENCE            = 0xA3,
    MPAF_SDP_DELETE_RECORD                = 0xA4,
} mpaf_command_opcode_t;

typedef enum
{
    MPAF_COMMAND_COMPLETE                 = 0x00,
    MPAF_COMMAND_STATUS                   = 0x01,
    MPAF_RFCOMM_CONNECTION_COMPLETE       = 0x80,
    MPAF_RFCOMM_DISCONNECTION_COMPLETE    = 0x81,
    MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER    = 0x83,
} mpaf_event_opcode_t;

typedef enum
{
    MPAF_SUCCESS                          = 0x00,
    MPAF_PENDING                          = 0x01,
    MPAF_BUSY                             = 0x02,
    MPAF_FAILED                           = 0x04,
    MPAF_INVALID_PARAMETER                = 0x05,
    MPAF_UNKNOWN_COMMAND                  = 0x06,
    MPAF_INTERNAL_FAILURE                 = 0x07,
    MPAF_PAGE_TIMEOUT                     = 0x08,
    MPAF_AUTHENTICATION_FAILURE           = 0x09,
    MPAF_CONNECTION_TIMEOUT               = 0x0A,
    MPAF_MEMORY_CAPACITY_EXCEEDED         = 0x0B,
    MPAF_COMMAND_DISALLOWED               = 0x0C,
} mpaf_result_t;

typedef enum
{
    MPAF_CHANNEL_DIRECTION_IN  = 0x1, /* Direction Host -> Bluetooth Chip */
    MPAF_CHANNEL_DIRECTION_OUT = 0x2, /* Direction Bluetooth Chop -> Host */
    MPAF_CHANNEL_BIDIRECTIONAL = MPAF_CHANNEL_DIRECTION_IN | MPAF_CHANNEL_DIRECTION_OUT,
} mpaf_channel_direction_t;

typedef enum
{
    UUID_SIZE_16BIT  = 0x02,
    UUID_SIZE_32BIT  = 0x04,
    UUID_SIZE_128BIT = 0x10,
} uuid_size_t;

typedef enum
{
    MPAF_RFCOMM_CLIENT_MODE = 0,
    MPAF_RFCOMM_SERVER_MODE = 1
} mpaf_rfcomm_mode_t;

typedef enum
{
    MPAF_SDP_ATTRIBUTE_NULL                      = 0x0,
    MPAF_SDP_ATTRIBUTE_UNSIGNED_INTEGER          = 0x1,
    MPAF_SDP_ATTRIBUTE_SIGNED_2S_COMPLEMENT      = 0x2,
    MPAF_SDP_ATTRIBUTE_UUID                      = 0x3,
    MPAF_SDP_ATTRIBUTE_STRING                    = 0x4,
    MPAF_SDP_ATTRIBUTE_BOOLEAN                   = 0x5,
    MPAF_SDP_ATTRIBUTE_SEQUENCE_DATA             = 0x6,
    MPAF_SDP_ATTRIBUTE_SEQUENCE_DATA_ALTERNATIVE = 0x7,
    MPAF_SDP_ATTRIBUTE_URL                       = 0x8,
} mpaf_sdp_attribute_type_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)

/* MPAF header */
typedef struct
{
    uint32_t endpoint : 7;
    uint32_t dir      : 1;
    uint32_t port     : 4;
    uint32_t reserved : 4;
    uint32_t length   : 16;
} mpaf_header_t;

/* MPAF common header. This is the common format between command, event, and data headers */
typedef struct
{
    uint8_t               packet_type; /* transport layer packet type. Not transmitted for USB */
    mpaf_header_t         header;
} mpaf_common_header_t;

/* MPAF common packet */
typedef struct
{
    mpaf_common_header_t  header;
    uint8_t               common_data_start[1];
} mpaf_common_packet_t;

/* MPAF data header */
typedef struct
{
    uint8_t               packet_type; /* transport layer packet type. Not transmitted for USB */
    mpaf_header_t         header;
} mpaf_data_header_t;

/* MPAF data packet */
typedef struct
{
    mpaf_data_header_t    header;
    uint8_t               data_start[1];
} mpaf_data_packet_t;

/* MPAF command header */
typedef struct
{
    uint8_t               packet_type; /* transport layer packet type. Not transmitted for USB */
    mpaf_header_t         header;
    uint8_t               control_class;
    mpaf_command_opcode_t opcode;
}mpaf_command_header_t;

/* MPAF command packet */
typedef struct
{
    mpaf_command_header_t header;
    uint8_t               params[1];
} mpaf_command_packet_t;

/* MPAF event header */
typedef struct
{
    uint8_t               packet_type; /* transport layer packet type. Not transmitted for USB */
    mpaf_header_t         header;
    uint8_t               control_class;
    mpaf_event_opcode_t   opcode;
} mpaf_event_header_t;

/* MPAF event packet */
typedef struct
{
    mpaf_event_header_t   header;
    uint8_t               params[1];
} mpaf_event_packet_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_OPEN_TRANSPORT_CHANNEL command  */
typedef struct
{
    uint8_t               endpoint;
    mpaf_result_t         status;
} return_params_open_transport_channel_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_CLOSE_TRANSPORT_CHANNEL command  */
typedef struct
{
    mpaf_result_t         status;
} return_params_close_transport_channel_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_GET_CONTROLLER_STATUS command  */
typedef struct
{
    uint8_t               controller_status;
    mpaf_result_t         status;
} return_params_get_controller_status_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_RFCOMM_CREATE_CONNECTION_CANCEL command  */
typedef struct
{
    mpaf_result_t         status;
} return_params_rfcomm_create_connection_cancel_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_SDP_CREATE_RECORD command  */
typedef struct
{
    uint8_t               record_handle;
    mpaf_result_t         status;
} return_params_sdp_create_record_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_SDP_ADD_PROTOCOL_DESCRIPTOR_LIST command  */
typedef struct
{
    mpaf_result_t         status;
} return_params_sdp_add_protocol_descriptor_list_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_SDP_ADD_ATTRIBUTES command  */
typedef struct
{
    mpaf_result_t         status;
} return_params_sdp_add_attributes_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_SDP_ADD_UUID_SEQUENCE command  */
typedef struct
{
    mpaf_result_t         status;
} return_params_sdp_add_uuid_sequence_t;

/* Parameters for MPAF_COMMAND_COMPLETE event in response to MPAF_SDP_DELETE_RECORD command  */
typedef struct
{
    mpaf_result_t         status;
} return_params_sdp_delete_record_t;

/* Parameters for MPAF_COMMAND_COMPLETE event packet  */
typedef struct
{
    mpaf_command_opcode_t command_opcode;

    union
    {
        return_params_open_transport_channel_t           open_transport_channel;
        return_params_close_transport_channel_t          close_transport_channel;
        return_params_get_controller_status_t            get_controller_status;
        return_params_rfcomm_create_connection_cancel_t  rfcomm_create_connection_cancel;
        return_params_sdp_create_record_t                sdp_create_record;
        return_params_sdp_add_protocol_descriptor_list_t sdp_add_protocol_descriptor_list;
        return_params_sdp_add_attributes_t               sdp_add_attributes;
        return_params_sdp_add_uuid_sequence_t            sdp_add_uuid_sequence;
        return_params_sdp_delete_record_t                sdp_delete_record;
    } return_params;

} event_params_command_complete_t;

/* Parameters for MPAF_COMMAND_STATUS event packet  */
typedef struct
{
    mpaf_result_t         status;
    mpaf_command_opcode_t command_opcode;
} event_params_command_status_t;

/* Parameters for MPAF_RFCOMM_CONNECTION_COMPLETE event packet  */
typedef struct
{
    uint8_t               endpoint;
    mpaf_result_t         status;
} event_params_rfcomm_connection_complete_t;

/* Parameters for MPAF_RFCOMM_DISCONNECTION_COMPLETE event packet  */
typedef struct
{
    uint8_t               endpoint;
    mpaf_result_t         status;
} event_params_rfcomm_disconnection_complete_t;

/* Parameters for MPAF_RFCOMM_SERVICE_CHANNEL_NUMBER event packet  */
typedef struct
{
    uint8_t               channel;
} event_params_rfcomm_service_channel_number_t;

typedef union
{
    event_params_command_complete_t              command_complete;
    event_params_command_status_t                command_status;
    event_params_rfcomm_connection_complete_t    rfcomm_connection_complete;
    event_params_rfcomm_disconnection_complete_t rfcomm_disconnection_complete;
    event_params_rfcomm_service_channel_number_t rfcomm_service_channel_number;
} mpaf_event_params_t;

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
