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

typedef struct bt_list_node bt_list_node_t;

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)
struct bt_list_node
{
    void*           data;
    bt_list_node_t* next;
    bt_list_node_t* prev;
};

typedef struct
{
    uint32_t        count;
    bt_list_node_t* front;
    bt_list_node_t* rear;
} bt_linked_list_t;
#pragma pack()

typedef wiced_bool_t (*bt_linked_list_compare_callback_t)( bt_list_node_t* node_to_compare, void* user_data );

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t bt_linked_list_init( bt_linked_list_t* list );

wiced_result_t bt_linked_list_deinit( bt_linked_list_t* list );

wiced_result_t bt_linked_list_get_count( bt_linked_list_t* list, uint32_t* count );

wiced_result_t bt_linked_list_set_node_data( bt_list_node_t* node, const void* data );

wiced_result_t bt_linked_list_get_front( bt_linked_list_t* list, bt_list_node_t** front_node );

wiced_result_t bt_linked_list_get_rear( bt_linked_list_t* list, bt_list_node_t** rear_node );

wiced_result_t bt_linked_list_find( bt_linked_list_t* list, bt_linked_list_compare_callback_t callback, void* user_data, bt_list_node_t** node_found );

wiced_result_t bt_linked_list_insert_at_front( bt_linked_list_t* list, bt_list_node_t* node );

wiced_result_t bt_linked_list_insert_at_rear( bt_linked_list_t* list, bt_list_node_t* node );

wiced_result_t bt_linked_list_insert_before( bt_linked_list_t* list, bt_list_node_t* reference_node, bt_list_node_t* node_to_insert );

wiced_result_t bt_linked_list_insert_after( bt_linked_list_t* list, bt_list_node_t* reference_node, bt_list_node_t* node_to_insert );

wiced_result_t bt_linked_list_remove_from_front( bt_linked_list_t* list, bt_list_node_t** removed_node );

wiced_result_t bt_linked_list_remove_from_rear( bt_linked_list_t* list, bt_list_node_t** removed_node );

wiced_result_t bt_linked_list_remove( bt_linked_list_t* list, bt_list_node_t* node );

#ifdef __cplusplus
} /* extern "C" */
#endif
