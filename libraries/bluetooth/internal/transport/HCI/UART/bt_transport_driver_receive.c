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
#include "wiced_result.h"
#include "wiced_bt_constants.h"
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

wiced_result_t bt_hci_transport_driver_bus_read_handler( bt_packet_t** packet )
{
    hci_packet_type_t packet_type;
    wiced_result_t    result;

    /* Get the packet type */
    result = bt_bus_receive( (uint8_t*) &packet_type, 1, WICED_NEVER_TIMEOUT );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    /* Read the header and determine the   */
    switch ( packet_type )
    {
        case HCI_ACL_DATA_PACKET:
        {
            hci_acl_packet_header_t header;

            /* Get the packet type */
            result = bt_bus_receive( (uint8_t*) &header.hci_handle, sizeof( header ) - sizeof( packet_type ), WICED_NEVER_TIMEOUT );
            if ( result != WICED_BT_SUCCESS )
            {
                return result;
            }

            header.packet_type = packet_type;

            /* Allocate buffer for the incoming packet */
            result = bt_hci_create_packet( packet_type, packet, header.content_length );
            if ( result != WICED_BT_SUCCESS )
            {
                return result;
            }

            /* Copy header to the packet */
            memcpy( ( *packet )->packet_start, &header, sizeof( header ) );
            ( *packet )->data_end  = ( *packet )->data_start + header.content_length;
            break;
        }

        case HCI_SCO_DATA_PACKET:
        {
            hci_sco_packet_header_t header;

            /* Get the packet type */
            result = bt_bus_receive( (uint8_t*)&header.hci_handle, sizeof( header ) - sizeof( packet_type ), WICED_NEVER_TIMEOUT );
            if ( result != WICED_BT_SUCCESS )
            {
                return result;
            }

            header.packet_type = packet_type;

            /* Allocate buffer for the incoming packet */
            result = bt_hci_create_packet( packet_type, packet, header.content_length );
            if ( result != WICED_BT_SUCCESS )
            {
                return result;
            }

            /* Copy header to the packet */
            memcpy( ( *packet )->packet_start, &header, sizeof( header ) );
            ( *packet )->data_end  = ( *packet )->data_start + header.content_length;
            break;
        }

        case HCI_EVENT_PACKET:
        {
            hci_event_header_t header;

            /* Get the packet type */
            result = bt_bus_receive( (uint8_t*)&header.event_code, sizeof( header ) - sizeof( packet_type ), WICED_NEVER_TIMEOUT );
            if ( result != WICED_BT_SUCCESS )
            {
                return result;
            }

            header.packet_type = packet_type;

            /* Allocate buffer for the incoming packet */
            result = bt_hci_create_packet( packet_type, packet, header.content_length );
            if ( result != WICED_SUCCESS )
            {
                return result;
            }

            /* Copy header to the packet */
            memcpy( ( *packet )->packet_start, &header, sizeof( header ) );
            ( *packet )->data_end  = ( *packet )->data_start + header.content_length;
            break;
        }

        case HCI_COMMAND_PACKET: /* Fall-through */
        default:
            wiced_assert( "Unexpected packet", 0!=0 );
            return WICED_BT_UNKNOWN_PACKET;
    }

    /* Receive the remainder of the packet */
    result = bt_bus_receive( (uint8_t*)( ( *packet )->data_start ), (uint32_t)( ( *packet )->data_end - ( *packet )->data_start ), WICED_NEVER_TIMEOUT );
    if ( result != WICED_SUCCESS )
    {
        bt_packet_pool_free_packet( *packet );
        return result;
    }

    return WICED_BT_SUCCESS;
}
