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
 * WICED Configuration Mode Application
 *
 * This application demonstrates how to use WICED Configuration Mode
 * to automatically configure application parameters and Wi-Fi settings
 * via a softAP and webserver
 *
 * Features demonstrated
 *  - WICED Configuration Mode
 *
 * Application Instructions
 *   1. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. After the download completes, the terminal displays WICED startup
 *      information and starts WICED configuration mode.
 *
 * In configuration mode, application and Wi-Fi configuration information
 * is entered via webpages using a Wi-Fi client (eg. your computer)
 *
 * Use your computer to step through device configuration using WICED Config Mode
 *   - Connect the computer using Wi-Fi to the config softAP "WICED Config"
 *     The config AP name & passphrase is defined in the file <WICED-SDK>/include/default_wifi_config_dct.h
 *     The AP name/passphrase is : Wiced Config / 12345678
 *   - Open a web browser and type wiced.com in the URL
 *     (or enter 192.168.0.1 which is the IP address of the softAP interface)
 *   - The Application configuration webpage appears. This page enables
 *     users to enter application specific information such as contact
 *     name and address details for device registration
 *   - Change one of more of the fields in the form and then click 'Save settings'
 *   - Click the Wi-Fi Setup button
 *   - The Wi-Fi configuration page appears. This page provides several options
 *     for configuring the device to connect to a Wi-Fi network.
 *   - Click 'Scan and select network'. The device scans for Wi-Fi networks in
 *     range and provides a webpage with a list.
 *   - Enter the password for your Wi-Fi AP in the Password box (top left)
 *   - Find your Wi-Fi AP in the list, and click the 'Join' button next to it
 *
 * Configuration mode is complete. The device stops the softAP and webserver,
 * and attempts to join the Wi-Fi AP specified during configuration. Once the
 * device completes association, application configuration information is
 * printed to the terminal
 *
 * The wiced.com URL reference in the above text is configured in the DNS
 * redirect server. To change the URL, edit the list in
 * <WICED-SDK>/Library/daemons/dns_redirect.c
 * URLs currently configured are:
 *      # http://www.broadcom.com , http://broadcom.com ,
 *      # http://www.facebook.com , http://facebook.com ,
 *      # http://www.google.com   , http://google.com   ,
 *      # http://www.bing.com     , http://bing.com     ,
 *      # http://www.apple.com    , http://apple.com    ,
 *      # http://www.wiced.com    , http://wiced.com    ,
 *
 *  *** IMPORTANT NOTE ***
 *   The config mode API will be integrated into Wi-Fi Easy Setup when
 *   WICED-SDK-3.0.0 is released.
 *
 */

#include "wiced.h"
#include "config_mode_dct.h"

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
static const configuration_entry_t app_config[] =
{
    {"First Name", DCT_OFFSET(config_mode_app_dct_t, fname),   30, CONFIG_STRING_DATA },
    {"Last Name",  DCT_OFFSET(config_mode_app_dct_t, lname),   30, CONFIG_STRING_DATA },
    {"Address 1",  DCT_OFFSET(config_mode_app_dct_t, addr1),   30, CONFIG_STRING_DATA },
    {"Address 2",  DCT_OFFSET(config_mode_app_dct_t, addr2),   30, CONFIG_STRING_DATA },
    {"Suburb",     DCT_OFFSET(config_mode_app_dct_t, suburb),  30, CONFIG_STRING_DATA },
    {"Country",    DCT_OFFSET(config_mode_app_dct_t, country), 30, CONFIG_STRING_DATA },
    {"Zipcode",    DCT_OFFSET(config_mode_app_dct_t, zip),      4, CONFIG_UINT32_DATA },
    {"Phone",      DCT_OFFSET(config_mode_app_dct_t, phone),   20, CONFIG_STRING_DATA },
    {0,0,0,0}
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    config_mode_app_dct_t* dct;

    /* Initialise the device */
    wiced_init( );

    /* Configure the device */
    wiced_configure_device( app_config );

    /* Bring network up in STA mode */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    wiced_dct_read_lock( (void**) &dct, WICED_FALSE, DCT_APP_SECTION, 0, sizeof(config_mode_app_dct_t));

    /* Print DCT that was configured */
    WPRINT_APP_INFO( ( "\n" ) );
    WPRINT_APP_INFO( ( "Application DCT Configuration:\n" ) );
    WPRINT_APP_INFO( ( "First Name = %s\n",               dct->fname   ) );
    WPRINT_APP_INFO( ( "Last Name  = %s\n",               dct->lname   ) );
    WPRINT_APP_INFO( ( "Address 1  = %s\n",               dct->addr1   ) );
    WPRINT_APP_INFO( ( "Address 2  = %s\n",               dct->addr2   ) );
    WPRINT_APP_INFO( ( "Suburb     = %s\n",               dct->suburb  ) );
    WPRINT_APP_INFO( ( "Country    = %s\n",               dct->country ) );
    WPRINT_APP_INFO( ( "Zipcode    = %d\n", (unsigned int)dct->zip     ) );
    WPRINT_APP_INFO( ( "Phone      = %s\n",               dct->phone   ) );

    wiced_dct_read_unlock( dct, WICED_FALSE );

    /* Deinitialise WICED system */
    wiced_deinit();

}

