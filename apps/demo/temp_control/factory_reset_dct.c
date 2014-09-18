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
 */

#include "WICED/platform/include/platform_dct.h"
#include "temp_control_dct.h"
#include <stdlib.h>

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

typedef struct
{
    platform_dct_data_t platform;
    user_dct_data_t     user;
} factory_reset_dct_t;

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

static const factory_reset_dct_t factory_reset_dct =
{
    /* DCT Header Section ____________________________________________________________*/
    .platform.dct_header.write_incomplete     = 0,
    .platform.dct_header.is_current_dct       = 1,
    .platform.dct_header.app_valid            = 1,
    .platform.dct_header.mfg_info_programmed  = 1,
    .platform.dct_header.magic_number         = BOOTLOADER_MAGIC_NUMBER,
    .platform.dct_header.load_app_func        = NULL,

    /* Manufacturing Section _________________________________________________________*/
    .platform.mfg_info = _DYNAMIC_MFG_INFO,

    /* Security Credentials for Config Section _______________________________________*/
    .platform.security_credentials.certificate = _DYNAMIC_CERTIFICATE_STORE,

    /* Wi-Fi Configuration Section ___________________________________________________*/
   .platform.wifi_config.device_configured = 1,
   .platform.wifi_config.mac_address       = _DYNAMIC_WLAN_MAC_ADDRESS,
   .platform.wifi_config.stored_ap_list[0] = _DYNAMIC_STORED_AP_INFO,
   .platform.wifi_config.stored_ap_list[1] = 0,
   .platform.wifi_config.stored_ap_list[2] = 0,
   .platform.wifi_config.stored_ap_list[3] = 0,
   .platform.wifi_config.stored_ap_list[4] = 0,
   .platform.wifi_config.soft_ap_settings  = 0,
   .platform.wifi_config.config_ap_settings.SSID          = _DYNAMIC_CONFIG_AP_SSID,
   .platform.wifi_config.config_ap_settings.security_key  = _DYNAMIC_CONFIG_AP_PASSPHRASE,
   .platform.wifi_config.config_ap_settings.security      = WICED_SECURITY_WPA2_AES_PSK,
   .platform.wifi_config.config_ap_settings.channel       = 1,
   .platform.wifi_config.config_ap_settings.details_valid = CONFIG_VALIDITY_VALUE,
   .platform.wifi_config.country_code                     = WICED_COUNTRY_AUSTRALIA,

    /* Application Data Section_______________________________________________________*/
   .user.xively_feed_id       = "",
   .user.xively_api_key       = "",
   .user.xively_channel_id    = "",
   .user.sample_interval      = 1000,
};


