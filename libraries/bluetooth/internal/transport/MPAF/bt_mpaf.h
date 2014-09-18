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

#include "bt_mpaf_interface.h"
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

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef void (*mpaf_rx_data_packet_cb_t) ( uint8_t endpoint, bt_packet_t* packet, uint8_t* data_start, uint32_t data_size );
typedef void (*mpaf_event_cb_t)          ( mpaf_event_opcode_t event, bt_packet_t* packet, mpaf_event_params_t* params );

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    uuid_size_t uuid_size;
    uint8_t     uuid[UUID_SIZE_128BIT];
} mpaf_uuid_param_t;

typedef struct
{
    uint16_t    protocol_uuid;
    uint8_t     number_of_params; /* 2 is maximum */
    uint16_t    params[2];
} mpaf_protocol_element_param_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_mpaf_init( mpaf_event_cb_t event_cb );

wiced_result_t bt_mpaf_deinit( void );

wiced_bool_t   bt_mpaf_is_initialised( void );

wiced_result_t bt_mpaf_register_rfcomm_callback( mpaf_event_cb_t event_cb, mpaf_rx_data_packet_cb_t rx_data_packet_cb );

wiced_result_t bt_mpaf_register_sdp_callback( mpaf_event_cb_t event_cb );

wiced_result_t bt_mpaf_open_transport_channel( mpaf_channel_direction_t direction );

wiced_result_t bt_mpaf_close_transport_channel( uint8_t endpoint, mpaf_channel_direction_t direction );

wiced_result_t bt_mpaf_get_controller_status( void );

wiced_result_t bt_mpaf_rfcomm_create_connection( const void* uuid, uuid_size_t uuid_size, const char* service_name, mpaf_rfcomm_mode_t mode, uint8_t feature_mask );

wiced_result_t bt_mpaf_rfcomm_remove_connection( uint8_t endpoint );

wiced_result_t bt_mpaf_rfcomm_create_connection_cancel( void );

wiced_result_t bt_mpaf_create_dynamic_packet( bt_packet_t** packet, bt_mpaf_packet_type_t packet_type, uint32_t data_size );

wiced_result_t bt_mpaf_create_packet( bt_packet_t** packet, bt_mpaf_packet_type_t packet_type, uint32_t data_size );

wiced_result_t bt_mpaf_rfcomm_create_data_packet( bt_packet_t** packet, uint32_t data_size, uint8_t** data );

wiced_result_t bt_mpaf_rfcomm_send_data_packet( uint8_t endpoint, bt_packet_t* packet );

wiced_result_t bt_mpaf_sdp_create_record( void );

wiced_result_t bt_mpaf_sdp_delete_record( uint8_t record_handle );

wiced_result_t bt_mpaf_sdp_add_protocol_descriptor_list( uint8_t record_handle, const mpaf_protocol_element_param_t* element_list, uint8_t list_size );

wiced_result_t bt_mpaf_sdp_add_attributes( uint8_t record_handle, uint16_t attribute_id, mpaf_sdp_attribute_type_t attribute_type, uint8_t attribute_length, const void* attribute_value );

wiced_result_t bt_mpaf_sdp_add_uuid_sequence( uint8_t record_handle, uint16_t attribute_id, const mpaf_uuid_param_t* uuid_list, uint8_t list_size );

#ifdef __cplusplus
} /* extern "C" */
#endif
