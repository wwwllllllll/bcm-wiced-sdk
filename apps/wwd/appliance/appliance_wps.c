/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 * This file performs the WPS negotiation for the Appliance App
 *
 */


#include "wiced_debug.h"
#include "wps_host.h"
#include "string.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_CREDENTIALS     5

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
static const wiced_wps_device_detail_t details =
{
    .device_name     = PLATFORM,
    .manufacturer    = "Broadcom",
    .model_name      = PLATFORM,
    .model_number    = "1.0",
    .serial_number   = "1408248",
    .device_category = WICED_WPS_DEVICE_COMPUTER,
    .sub_category    = 7,
    .config_methods  = WPS_CONFIG_LABEL | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN
};

/******************************************************
 *               Function Definitions
 ******************************************************/
void do_wps( wiced_wps_mode_t wps_mode, char* pin )
{
    WINFO_APP(("Starting WPS enrollee. Press WPS button on access point\n"));
    wiced_wps_credential_t credential[MAX_CREDENTIALS];
    memset(credential, 0, MAX_CREDENTIALS*sizeof(wiced_wps_credential_t));
    besl_wps_init();
    besl_wps_enrollee( wps_mode, &details, pin, credential, MAX_CREDENTIALS, NULL);
    besl_wps_deinit();

    /* Check if we got valid credentials */
    if (credential[0].SSID.length == 0)
    {
        /* WPS failed. Abort */
        WINFO_APP(("No access point found. Halting\n"));
        while(1);
    }

    /* Attempt to join the Wi-Fi network. Try all the credentials in a round robin fashion */
    int a = 0;
    wiced_result_t ret;
    wiced_wps_credential_t* cred;
    do
    {
        cred = &credential[a];
        WINFO_APP(("Joining : %s\n", cred->SSID.value));
        ret = wiced_wifi_join( (char*)cred->SSID.value, cred->security, (uint8_t*) cred->passphrase, cred->passphrase_length, NULL );
        if (ret != WICED_SUCCESS)
        {
            WINFO_APP(("Failed to join  : %s   .. retrying\n", cred->SSID.value));
            ++a;
            if (credential[a].SSID.length == 0)
            {
                a = 0;
            }
        }
    }
    while (ret != WICED_SUCCESS);
    WINFO_APP(("Successfully joined : %s\n", cred->SSID.value));

}
