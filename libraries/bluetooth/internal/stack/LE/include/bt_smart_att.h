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

#include "wiced_utilities.h"
#include "bt_packet_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define ATT_DEFAULT_MTU           (23)
#define ATT_STANDARD_VALUE_LENGTH (ATT_DEFAULT_MTU - 3)
#define ATT_STANDARD_TIMEOUT      (500)

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    BT_SMART_ATT_ERR_NO_ERROR                    = 0x00,
    BT_SMART_ATT_ERR_INVALID_HANDLE              = 0x01,
    BT_SMART_ATT_ERR_READ_NOT_PERMITTED          = 0x02,
    BT_SMART_ATT_ERR_WRITE_NOT_PERMITTED         = 0x03,
    BT_SMART_ATT_ERR_INVALID_PDU                 = 0x04,
    BT_SMART_ATT_ERR_INSUFFICIENT_AUTHENTICATION = 0x05,
    BT_SMART_ATT_ERR_REQ_NOT_SUPPORTED           = 0x06,
    BT_SMART_ATT_ERR_INVALID_OFFSET              = 0x07,
    BT_SMART_ATT_ERR_INSUFFICIENT_AUTHORIZATION  = 0x08,
    BT_SMART_ATT_ERR_PREPARE_QUEUE_FULL          = 0x09,
    BT_SMART_ATT_ERR_ATTRIBUTE_NOT_FOUND         = 0x0a,
    BT_SMART_ATT_ERR_ATTRIBUTE_NOT_LONG          = 0x0b,
    BT_SMART_ATT_ERR_INSUFFICIENT_ENC_KEY_SIZE   = 0x0c,
    BT_SMART_ATT_ERR_INVALID_ATTR_VALUE_LENGTH   = 0x0d,
    BT_SMART_ATT_ERR_UNLIKELY_ERROR              = 0x0e,
    BT_SMART_ATT_ERR_INSUFFICIENT_ENCRYPTION     = 0x0f,
    BT_SMART_ATT_ERR_UNSUPPORTED_GROUP_TYPE      = 0x10,
    BT_SMART_ATT_ERR_INSUFFICIENT_RESOURCES      = 0x11,
    BT_SMART_ATT_ERR_APPLICATION_ERROR           = 0xff
} bt_smart_att_error_t;

typedef enum
{
    BT_SMART_ATT_CANCEL_ALL_PREPARE_WRITES         = 0x00,
    BT_SMART_ATT_WRITE_ALL_PENDING_PREPARED_VALUES = 0x01
} bt_smart_att_execute_write_flags_t;

typedef enum
{
    BT_SMART_ATT_UUID_FORMAT_16BIT  = 0x01,
    BT_SMART_ATT_UUID_FORMAT_128BIT = 0x02
} bt_smart_att_uuid_format_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct bt_smart_att_pdu bt_smart_att_pdu_t;
typedef wiced_result_t (*bt_smart_att_response_handler_t)( bt_packet_t* packet, uint16_t pdu_length, const bt_smart_att_pdu_t* pdu );
typedef wiced_result_t (*bt_smart_att_timeout_handler_t)( void );

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)
typedef struct
{
    uint8_t signature[12];
} bt_smart_att_signed_write_signature_t;

struct bt_smart_att_pdu
{
    uint8_t code;
};
#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_smart_att_init( void );

wiced_result_t bt_smart_att_deinit( void );

wiced_result_t bt_smart_att_set_timeout( uint32_t timeout_ms );

wiced_result_t bt_smart_att_register_timeout_handler( bt_smart_att_timeout_handler_t handler );

wiced_result_t bt_smart_att_register_response_handler( bt_smart_att_response_handler_t handler );

wiced_result_t bt_smart_att_error_response( uint16_t connection_handle, uint16_t handle, bt_smart_att_error_t error_code, uint8_t request_opcode );

wiced_result_t bt_smart_att_exchange_mtu_request( uint16_t connection_handle, uint16_t client_mtu );

wiced_result_t bt_smart_att_find_information_request( uint16_t connection_handle, uint16_t starting_handle, uint16_t ending_handle );

wiced_result_t bt_smart_att_find_by_type_value_request( uint16_t connection_handle, uint16_t starting_handle, uint16_t ending_handle, uint16_t uuid, const uint8_t* value, uint32_t value_size );

wiced_result_t bt_smart_att_read_by_type_request( uint16_t connection_handle, uint16_t starting_handle, uint16_t ending_handle, const wiced_bt_uuid_t* uuid );

wiced_result_t bt_smart_att_read_request( uint16_t connection_handle, uint16_t handle );

wiced_result_t bt_smart_att_read_blob_request( uint16_t connection_handle, uint16_t handle, uint16_t offset );

wiced_result_t bt_smart_att_read_multiple_request( uint16_t connection_handle, const uint16_t* handle_list, uint32_t count );

wiced_result_t bt_smart_att_read_by_group_type_request( uint16_t connection_handle, uint16_t starting_handle, uint16_t ending_handle, const wiced_bt_uuid_t* uuid );

wiced_result_t bt_smart_att_write_request( uint16_t connection_handle, uint16_t handle, const uint8_t* value, uint32_t value_size );

wiced_result_t bt_smart_att_prepare_write_request( uint16_t connection_handle, uint16_t handle, uint16_t offset, const uint8_t* value, uint32_t value_size );

wiced_result_t bt_smart_att_execute_write_request( uint16_t connection_handle, bt_smart_att_execute_write_flags_t flags );

wiced_result_t bt_smart_att_write_command( uint16_t connection_handle, uint16_t handle, const uint8_t* value, uint32_t value_size );

wiced_result_t bt_smart_att_signed_write_command( uint16_t connection_handle, uint16_t handle, const uint8_t* value, uint32_t value_size, const bt_smart_att_signed_write_signature_t* signature );

wiced_result_t bt_smart_att_handle_value_confirmation( uint16_t connection_handle );

#ifdef __cplusplus
} /* extern "C" */
#endif
