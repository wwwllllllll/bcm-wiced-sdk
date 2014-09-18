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
 * Service Discovery Application
 *
 * Features demonstrated
 *  - Gedday service discovery library
 *
 * Application Instructions
 *   1. Build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. Connect a Wi-Fi client (PC, phone, tablet) to the
 *      "WICED Service Discovery App" softAP
 *   3. Run an mDNS (or Bonjour) browser on the Wi-Fi client
 *   4. A WICED TCP service should appear in the mDNS browser
 */

#include "wiced.h"
#include "gedday.h"

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

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192, 168, 0,   1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255, 255, 255, 0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192, 168, 0,   1) ),
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_init( );

    wiced_network_up(WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &device_init_ip_settings);

    gedday_init(WICED_AP_INTERFACE, "WICED_Gedday_Example");
    gedday_add_service("Gedday_instance", "_http._tcp.local", 80, 6, "Broadcom WICED = Wi-Fi for MCUs!");
}
