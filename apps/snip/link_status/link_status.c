/*
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
 * Link Status Application
 *
 * This application demonstrates how to asynchronously handle
 * changes in the Wi-Fi link connection status.
 *
 * Features demonstrated
 *  - Wi-Fi client mode
 *  - Wi-Fi link status callback API
 *
 * Application Instructions
 *   1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *      in the wifi_config_dct.h header file to match your Wi-Fi access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 * After download, the app joins the configured AP as a client.
 * To demonstrate the link up/link down API:
 *   - Power OFF the AP
 *   - Wait for a link down message to appear on the terminal
 *   - Power ON the AP
 *   - Wait for a link up message to appear on the terminal
 *
 */

#include "wiced.h"

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

static void link_up  ( void );
static void link_down( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_semaphore_t link_up_semaphore;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start(void)
{
    /* Initialise the device */
    wiced_init();

    /* Register callbacks */
    wiced_network_register_link_callback( link_up, link_down );

    /* Bring up the STA (client) interface ------------------------------------------------------- */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Initialise semaphore to notify when the network comes up */
    wiced_rtos_init_semaphore( &link_up_semaphore );

    WPRINT_APP_INFO( ("\nSwitch your AP off\n\n") );

    /* The link_up() function sets a semaphore when the link is back up. Wait here until the semaphore is set */
    wiced_rtos_get_semaphore( &link_up_semaphore, WICED_NEVER_TIMEOUT );

    /* Clean up and halt */
    wiced_rtos_deinit_semaphore( &link_up_semaphore );
    wiced_network_deregister_link_callback( link_up, link_down );
    wiced_deinit();
}


static void link_up( void )
{
    /* Set a semaphore to indicate the link is back up */
    wiced_rtos_set_semaphore( &link_up_semaphore );
    WPRINT_APP_INFO( ("And we're connected again!\n") );
}


static void link_down( void )
{
    WPRINT_APP_INFO( ("Network connection is down.\n") );
    WPRINT_APP_INFO( ("Switch on your AP and wait until the connection is up.\n") );
}
