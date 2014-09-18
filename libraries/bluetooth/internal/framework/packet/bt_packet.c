/**
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */

#include "wiced.h"
#include "wiced_bt.h"
#include "bt_packet_internal.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define DYNAMIC_PACKET_POOL (bt_packet_pool_t*)( 0xBEEFFEED )

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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

static uint32_t bt_dynamic_packet_created = 0;
static uint32_t bt_dynamic_packet_deleted = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_packet_pool_init( bt_packet_pool_t* pool, uint32_t packet_count, uint32_t header_size, uint32_t data_size )
{
    uint32_t       packet_size = header_size + data_size + sizeof(bt_packet_t) - 1;
    uint32_t       buffer_size = packet_count * packet_size;
    uint8_t*       packet_ptr  = NULL;
    void*          buffer      = NULL;
    wiced_result_t result;
    uint32_t       a;

    memset( pool, 0, sizeof( *pool ) );

    buffer = (void*)malloc_named( "bt_packet_pool", buffer_size );
    if ( buffer == NULL )
    {
        return WICED_BT_OUT_OF_HEAP_SPACE;
    }

    result = bt_linked_list_init( &pool->pool_list );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    pool->pool_id          = PACKET_POOL_ID;
    pool->max_packet_count = packet_count;
    pool->header_size      = header_size;
    pool->data_size        = data_size;
    pool->pool_buffer      = buffer;
    packet_ptr             = buffer;

    for ( a = 0; a < packet_count; a++ )
    {
        bt_packet_t* packet = (bt_packet_t*)packet_ptr;

        result = bt_linked_list_insert_at_front( &pool->pool_list, &packet->node );

        if ( result == WICED_BT_SUCCESS )
        {
            packet->node.data = (void*)packet;
            packet->pool      = pool;
            packet->packet_id = PACKET_ID;

        }
        else
        {
            return result;
        }

        packet_ptr += packet_size;
    }

    return wiced_rtos_init_mutex( &pool->mutex );
}

wiced_result_t bt_packet_pool_deinit( bt_packet_pool_t* pool )
{
    uint32_t a;
    uint32_t packet_count;

    if ( pool == NULL || pool->pool_id != PACKET_POOL_ID )
    {
        return WICED_BT_BADARG;
    }

    wiced_rtos_deinit_mutex( &pool->mutex );
    bt_linked_list_get_count( &pool->pool_list, &packet_count );

    for ( a = 0; a < packet_count; a++ )
    {
        bt_list_node_t* removed_node;

        if ( bt_linked_list_remove_from_rear( &pool->pool_list, &removed_node ) == WICED_BT_SUCCESS )
        {
            removed_node->data = NULL;
        }
    }

    bt_linked_list_deinit( &pool->pool_list );

    free( pool->pool_buffer );

    memset( pool, 0, sizeof( *pool ) );

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_packet_pool_allocate_packet( bt_packet_pool_t* pool, bt_packet_t** packet )
{
    bt_list_node_t* node   = NULL;
    wiced_result_t  result = WICED_BT_SUCCESS;

    if ( pool == NULL || packet == NULL || pool == DYNAMIC_PACKET_POOL )
    {
        return WICED_BT_BADARG;
    }

    wiced_rtos_lock_mutex( &pool->mutex );

    result = bt_linked_list_remove_from_rear( &pool->pool_list, &node );

    if ( result == WICED_BT_SUCCESS )
    {
        (*packet)              = (bt_packet_t*)node->data;
        (*packet)->node.data   = (void*)(*packet);
        (*packet)->packet_id   = PACKET_ID;
        (*packet)->pool        = pool;
        (*packet)->owner       = BT_PACKET_OWNER_STACK;
        (*packet)->packet_end  = (*packet)->packet_start + pool->header_size + pool->data_size;
        (*packet)->data_start  = (*packet)->packet_start + pool->header_size;
        (*packet)->data_end    = (*packet)->data_start;
        pool->packet_created++;
    }
    else if ( result == WICED_BT_LIST_EMPTY )
    {
        result = WICED_BT_PACKET_POOL_EXHAUSTED;
    }

    wiced_rtos_unlock_mutex( &pool->mutex );

    return result;
}

wiced_result_t bt_packet_pool_dynamic_allocate_packet( bt_packet_t** packet, uint32_t header_size, uint32_t data_size )
{
    if ( packet == NULL )
    {
        return WICED_BT_BADARG;
    }

    *packet = (bt_packet_t*)malloc_named( "bt_packet_pool", header_size + data_size + sizeof(bt_packet_t) - 1 );
    if ( *packet == NULL )
    {
        return WICED_BT_OUT_OF_HEAP_SPACE;
    }

    (*packet)->node.prev   = NULL;
    (*packet)->node.next   = NULL;
    (*packet)->node.data   = (void*)(*packet);
    (*packet)->pool        = DYNAMIC_PACKET_POOL;
    (*packet)->owner       = BT_PACKET_OWNER_STACK;
    (*packet)->packet_end  = (*packet)->packet_start + header_size + data_size;
    (*packet)->data_start  = (*packet)->packet_start + header_size;
    (*packet)->data_end    = (*packet)->data_start;
    bt_dynamic_packet_created++;

    return WICED_BT_SUCCESS;
}

wiced_result_t bt_packet_pool_free_packet( bt_packet_t* packet )
{
    if ( packet == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( packet->pool == DYNAMIC_PACKET_POOL )
    {
        malloc_transfer_to_curr_thread( packet );
        packet->owner     = BT_PACKET_OWNER_POOL;
        packet->node.prev = NULL;
        packet->node.next = NULL;
        packet->node.data = NULL;
        free( packet );
        bt_dynamic_packet_deleted++;
        return WICED_BT_SUCCESS;
    }
    else if ( packet->packet_id == PACKET_ID )
    {
        wiced_result_t result;

        wiced_rtos_lock_mutex( &packet->pool->mutex );

        result = bt_linked_list_insert_at_front( &packet->pool->pool_list, &packet->node );

        if ( result == WICED_SUCCESS )
        {
            packet->owner = BT_PACKET_OWNER_POOL;
            packet->pool->packet_deleted++;
        }

        wiced_rtos_unlock_mutex( &packet->pool->mutex );

        return result;
    }

    return WICED_BT_UNKNOWN_PACKET;
}

wiced_result_t wiced_bt_packet_delete( wiced_bt_packet_t* packet )
{
    return ( packet->owner == BT_PACKET_OWNER_APP ) ? bt_packet_pool_free_packet( packet ) : WICED_BT_PACKET_WRONG_OWNER;
}

wiced_result_t wiced_bt_packet_get_data( const wiced_bt_packet_t* packet, uint8_t** data, uint32_t* current_data_size, uint32_t* available_space )
{
    if ( packet == NULL || data == NULL || current_data_size == NULL || available_space == NULL )
    {
        return WICED_BT_BADARG;
    }

    *data              = packet->data_start;
    *current_data_size = (uint32_t)( packet->data_end - packet->data_start );
    *available_space   = (uint32_t)( packet->packet_end - packet->data_end );
    return WICED_BT_SUCCESS;
}

wiced_result_t wiced_bt_packet_set_data_end( wiced_bt_packet_t* packet, const uint8_t* data_end )
{
    if ( packet == NULL || data_end == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( data_end > packet->packet_end )
    {
        WPRINT_LIB_ERROR( ( "data pointer to set is beyond packet boundary\n" ) );
        return WICED_ERROR;
    }

    packet->data_end = (uint8_t*)data_end;
    return WICED_BT_SUCCESS;
}
