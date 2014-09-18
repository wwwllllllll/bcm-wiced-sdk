/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "../console.h"
#include "wiced_p2p.h"
#include "wps_host.h"

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
static besl_p2p_device_detail_t device_details =
{
    .wps_device_details =
    {
        .device_name     = "Wiced",
        .manufacturer    = "Broadcom",
        .model_name      = "BCM943362",
        .model_number    = "Wiced",
        .serial_number   = "12345670",
        .device_category = WICED_WPS_DEVICE_COMPUTER,
        .sub_category    = 7,
        .config_methods  = WPS_CONFIG_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN,
    },
    .group_owner_intent = 1,
    .ap_ssid_suffix     = "wiced!",
    .device_name        = "WICED-P2P",
};

static p2p_workspace_t workspace;



/******************************************************
 *               Function Definitions
 ******************************************************/

/*!
 ******************************************************************************
 * Starts P2P
 *
 * @return  0 for success, otherwise error
 */
int start_p2p( int argc, char* argv[] )
{
    uint8_t group_owner_intent = 1;

    if (argc > 1)
    {
        group_owner_intent = atoi( argv[1] );
        if ( ( group_owner_intent >= 0 ) && ( group_owner_intent <= 15 ) )
        {
            device_details.group_owner_intent = group_owner_intent;
        }
        else
        {
            return ERR_UNKNOWN;
        }
    }

    WPRINT_APP_INFO( ("Group Owner intent %u\r\n", device_details.group_owner_intent ) );

    besl_p2p_init( &workspace, &device_details );
    besl_p2p_start( &workspace );

    return ERR_CMD_OK;
}

/*!
 ******************************************************************************
 * Stops P2P
 *
 * @return  0 for success, otherwise error
 */
int stop_p2p( int argc, char* argv[] )
{
   besl_p2p_stop( &workspace );
   return 0;
}

/*!
 ******************************************************************************
 * Prints the P2P peer list
 *
 * @return  0 for success, otherwise error
 */
int p2p_peer_list( int argc, char* argv[] )
{
    p2p_discovered_device_t* devices;
    uint8_t device_count;
    besl_p2p_get_discovered_peers(&workspace, &devices, &device_count);
    WPRINT_APP_INFO( ("P2P Peers:\r\n") );
    for(; device_count != 0; )
    {
        --device_count;
        WPRINT_APP_INFO( (" %u: '%s' on channel %u\r\n", device_count, devices[device_count].device_name, devices[device_count].channel) );
    }
    return 0;
}


int p2p_invite( int argc, char* argv[] )
{
    int id = atoi(argv[1]);
    besl_p2p_invite(&workspace, &workspace.discovered_devices[id]);
    return 0;
}
