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
#include "bt_bus.h"
#include "bt_hci.h"
#include "bt_hci_interface.h"
#include "bt_packet_internal.h"

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

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_mfgtest_transport_driver_bus_read_handler( bt_packet_t** packet )
{
    hci_event_header_t header;
    wiced_result_t     result;

    /* Get the packet type */
    result = bt_bus_receive( (uint8_t*)&header, sizeof( header ), WICED_NEVER_TIMEOUT );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    /* Allocate buffer for the incoming packet. Always use dynamic packet pool for the  */
    result = bt_packet_pool_dynamic_allocate_packet( packet, sizeof( header ), header.content_length );
    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    /* Copy header to the packet */
    memcpy( ( *packet )->packet_start, &header, sizeof( header ) );
    ( *packet )->data_end  = ( *packet )->data_start + header.content_length;

    /* Receive the remainder of the packet */
    result = bt_bus_receive( (uint8_t*)( ( *packet )->data_start ), (uint32_t)( ( *packet )->data_end - ( *packet )->data_start ), WICED_NEVER_TIMEOUT );
    if ( result == WICED_BT_SUCCESS )
    {
        bt_packet_pool_free_packet( *packet );
        return result;
    }

    if ( header.packet_type == 0xff )
    {
        bt_packet_pool_free_packet( *packet );
        return WICED_BT_UNKNOWN_PACKET;
    }

    return WICED_BT_SUCCESS;
}
