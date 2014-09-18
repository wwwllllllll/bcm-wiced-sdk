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
#include "wiced_rtos.h"
#include "wiced_utilities.h"
#include "wiced_bt.h"
#include "wiced_bt_did.h"
#include "bt_mpaf.h"
#include "bt_management_mpaf.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define SEND_COMMAND( func ) \
    do \
    { \
        wiced_result_t retval = func; \
        if ( retval != WICED_BT_SUCCESS ) \
        { \
            WPRINT_LIB_ERROR( ( "Device ID Profile: error sending command\n" ) ); \
            return retval; \
        } \
    } \
    while ( 0 )

#define RECEIVE_EVENT( packet ) \
    do \
    { \
        wiced_result_t retval; \
        if ( packet != NULL ) \
        { \
            bt_packet_pool_free_packet( packet ); \
            packet = NULL; \
        } \
        retval = bt_management_mpaf_wait_for_event( &packet ); \
        if ( retval != WICED_BT_SUCCESS ) \
        { \
            WPRINT_LIB_ERROR( ( "Device ID Profile: Error receiving event\n" ) ); \
            return retval; \
        } \
    } \
    while ( 0 )

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

static const mpaf_uuid_param_t did_uuid =
{
    .uuid_size = UUID_SIZE_16BIT,
    .uuid      = { 0x00, 0x12 }
};

static uint8_t did_record_handle = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_bt_did_enable( const wiced_bt_did_mandatory_attributes_t* mandatory_attributes, const wiced_bt_did_optional_attributes_t* optional_attributes )
{
    bt_packet_t*         packet = NULL;
    mpaf_event_params_t* params = NULL;
    wiced_result_t       result;

    UNUSED_PARAMETER( optional_attributes );

    if ( mandatory_attributes == NULL )
    {
        return WICED_BT_BADARG;
    }

    if ( bt_mpaf_is_initialised() != WICED_TRUE )
    {
        WPRINT_LIB_ERROR( ( "Error initialising Device ID Profile. MPAF uninitialised!\n" ) );
        return WICED_BT_MPAF_UNINITIALISED;
    }

    /* Create SDP record for Device ID Profile */
    SEND_COMMAND( bt_mpaf_sdp_create_record() );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( ((mpaf_event_params_t*)(packet->data_start))->command_complete.return_params.sdp_create_record.status != MPAF_SUCCESS )
    {
        bt_packet_pool_free_packet( packet );
        return WICED_BT_ERROR;
    }

    /* SDP record successfully created. Retrieve record handle */
    did_record_handle = params->command_complete.return_params.sdp_create_record.record_handle;

    /* Add PnP UUID */
    SEND_COMMAND( bt_mpaf_sdp_add_uuid_sequence( did_record_handle, 0x0001, &did_uuid, 1 ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_uuid_sequence.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add SpecificationID Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( did_record_handle,
                                              0x0200,
                                              MPAF_SDP_ATTRIBUTE_UNSIGNED_INTEGER,
                                              sizeof( mandatory_attributes->specification_id ),
                                              (const void*)&mandatory_attributes->specification_id ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add VendorID Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( did_record_handle,
                                              0x0201,
                                              MPAF_SDP_ATTRIBUTE_UNSIGNED_INTEGER,
                                              sizeof( mandatory_attributes->vendor_id ),
                                              (const void*)&mandatory_attributes->vendor_id ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add ProductID Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( did_record_handle,
                                              0x0202,
                                              MPAF_SDP_ATTRIBUTE_UNSIGNED_INTEGER,
                                              sizeof( mandatory_attributes->product_id ),
                                              (const void*)&mandatory_attributes->product_id ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add Version Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( did_record_handle,
                                              0x0203,
                                              MPAF_SDP_ATTRIBUTE_UNSIGNED_INTEGER,
                                              sizeof( mandatory_attributes->version ),
                                              (const void*)&mandatory_attributes->version ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add PrimaryRecord Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( did_record_handle,
                                              0x0204,
                                              MPAF_SDP_ATTRIBUTE_BOOLEAN,
                                              sizeof( mandatory_attributes->primary_record ),
                                              (const void*)&mandatory_attributes->primary_record ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    /* Add VendorIDSource Attribute */
    SEND_COMMAND( bt_mpaf_sdp_add_attributes( did_record_handle,
                                              0x0205,
                                              MPAF_SDP_ATTRIBUTE_UNSIGNED_INTEGER,
                                              sizeof( mandatory_attributes->vendor_id_source ),
                                              (const void*)&mandatory_attributes->vendor_id_source ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_add_attributes.status != MPAF_SUCCESS )
    {
        goto delete_record;
    }

    return WICED_BT_SUCCESS;

    delete_record:

    SEND_COMMAND( bt_mpaf_sdp_delete_record( did_record_handle ) );
    RECEIVE_EVENT( packet );
    bt_packet_pool_free_packet( packet );
    return WICED_BT_ERROR;
}

wiced_result_t wiced_bt_did_disable( void )
{
    bt_packet_t*         packet;
    mpaf_event_params_t* params;

    SEND_COMMAND( bt_mpaf_sdp_delete_record( did_record_handle ) );
    RECEIVE_EVENT( packet );

    params = (mpaf_event_params_t*)packet->data_start;

    if ( params->command_complete.return_params.sdp_delete_record.status == MPAF_SUCCESS )
    {
        bt_packet_pool_free_packet( packet );
        did_record_handle = 0;
        return WICED_BT_SUCCESS;
    }
    else
    {
        bt_packet_pool_free_packet( packet );
        return WICED_BT_ERROR;
    }
}
