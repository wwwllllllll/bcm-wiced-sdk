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
#include "bt_mpaf.h"
#include "bt_mpaf_interface.h"
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

wiced_result_t bt_transport_driver_bus_read_handler( bt_packet_t** packet )
{
    mpaf_common_header_t  mpaf_common_header;
    mpaf_common_packet_t* mpaf_common_packet;
    wiced_result_t        result;

    /* Read MPAF packet_list_header to determine the data length */
    result = bt_bus_receive( (uint8_t*)&mpaf_common_header, sizeof( mpaf_common_header ), WICED_NEVER_TIMEOUT );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    if ( mpaf_common_header.header.endpoint == 0 )
    {
        /* Endpoint 0 is event packet */
        result = bt_mpaf_create_packet( packet, MPAF_EVENT_PACKET, mpaf_common_header.header.length );
    }
    else
    {
        /* Endpoint 1 is data packet */
        result = bt_mpaf_create_packet( packet, MPAF_DATA_PACKET, mpaf_common_header.header.length );
    }

    if ( result != WICED_BT_SUCCESS )
    {
        return result;
    }

    mpaf_common_packet = (mpaf_common_packet_t*)( ( *packet )->packet_start );

    /* Copy packet type and MPAF packet_list_header to the packet */
    memcpy( &mpaf_common_packet->header, &mpaf_common_header, sizeof( mpaf_common_header ) );

    /* Read the remainder of the packet */
    result = bt_bus_receive( mpaf_common_packet->common_data_start, mpaf_common_header.header.length, WICED_NEVER_TIMEOUT );
    if ( result != WICED_BT_SUCCESS )
    {
        wiced_assert("Error receiving MPAF packet\n", result == WICED_SUCCESS );
        bt_packet_pool_free_packet( *packet );
        return result;
    }

    (*packet)->data_end = mpaf_common_packet->common_data_start + mpaf_common_header.header.length;
    return result;
}
