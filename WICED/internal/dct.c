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

#include "platform_config.h"
#include "generated_security_dct.h"
#include "WICED/platform/include/platform_dct.h"
#include "wiced_defaults.h"

#ifdef WIFI_CONFIG_APPLICATION_DEFINED
#include "wifi_config_dct.h"
#else/* #ifdef WIFI_CONFIG_APPLICATION_DEFINED */
#include "default_wifi_config_dct.h"
#endif /* #ifdef WIFI_CONFIG_APPLICATION_DEFINED */

#include "generated_mac_address.txt"

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

extern const void * const dct_full_size_loc; /* Defined by linker script */

#if defined(__ICCARM__)
#pragma section="initial_dct_section"
extern const void * const dct_used_size_loc; /* Defined by linker script */
#else
extern const void * const dct_used_size_loc; /* Defined by linker script */
#endif

#ifndef SOFT_AP_PASSPHRASE_LENGTH
#define SOFT_AP_PASSPHRASE_LENGTH      sizeof(SOFT_AP_PASSPHRASE)-1
#endif

#ifndef CONFIG_AP_PASSPHRASE_LENGTH
#define CONFIG_AP_PASSPHRASE_LENGTH    sizeof(CONFIG_AP_PASSPHRASE)-1
#endif

#ifndef COOEE_KEY_STRING
#define COOEE_KEY_STRING       "abcdabcdabcdabcd"
#endif

#define DEFAULT_AP_LIST  \
    { \
        [0] = \
        { \
            .details = {{sizeof(CLIENT_AP_SSID)-1, CLIENT_AP_SSID},{{0,0,0,0,0,0}}, 0, 0, CLIENT_AP_BSS_TYPE, CLIENT_AP_SECURITY, CLIENT_AP_CHANNEL, CLIENT_AP_BAND}, \
            .security_key_length = sizeof(CLIENT_AP_PASSPHRASE)-1, \
            .security_key = CLIENT_AP_PASSPHRASE\
        }, \
    }

#if defined ( __IAR_SYSTEMS_ICC__ )
#pragma section="initial_dct_section"
static const platform_dct_data_t initial_dct @ "initial_dct_section";
#endif /* if defined ( __IAR_SYSTEMS_ICC__ ) */

static const platform_dct_data_t initial_dct =
{
    .dct_header.full_size            = (unsigned long)&dct_full_size_loc,
#if defined (__ICCARM__)
    .dct_header.used_size            = (unsigned long)__section_size("initial_dct_section"),
#else
    .dct_header.used_size            = (unsigned long)&dct_used_size_loc,
#endif
    .dct_header.write_incomplete     = 0,
    .dct_header.is_current_dct       = 1,
    .dct_header.app_valid            = 1,
    .dct_header.mfg_info_programmed  = 0,
    .dct_header.magic_number         = BOOTLOADER_MAGIC_NUMBER,

#ifdef BOOTLOADER_LOAD_MAIN_APP_FROM_FILESYSTEM
    .dct_header.boot_detail.entry_point = 0,
    .dct_header.boot_detail.load_details.load_once = 0,
    .dct_header.boot_detail.load_details.valid = 1,
    .dct_header.boot_detail.load_details.source.id = EXTERNAL_FILESYSTEM_FILE,
    .dct_header.boot_detail.load_details.source.detail.filesytem_filename = "app.elf",
    .dct_header.boot_detail.load_details.destination.id = INTERNAL,
#else
    .dct_header.boot_detail.entry_point = 0,
    .dct_header.boot_detail.load_details.valid = 0,
#endif /* ifdef USES_RESOURCE_FILESYSTEM */

    .security_credentials.certificate = CERTIFICATE_STRING,
    .security_credentials.private_key = PRIVATE_KEY_STRING,
    .security_credentials.cooee_key   = COOEE_KEY_STRING,
#ifdef CUSTOM_DEFAULT_DCT
    .wifi_config.device_configured   = WICED_TRUE,
#else
    .wifi_config.device_configured   = WICED_FALSE,
#endif
    .wifi_config.stored_ap_list      = DEFAULT_AP_LIST,
    .wifi_config.soft_ap_settings    = {{sizeof(SOFT_AP_SSID)-1,   SOFT_AP_SSID},   SOFT_AP_SECURITY,   SOFT_AP_CHANNEL,   SOFT_AP_PASSPHRASE_LENGTH, SOFT_AP_PASSPHRASE},
    .wifi_config.config_ap_settings  = {{sizeof(CONFIG_AP_SSID)-1, CONFIG_AP_SSID}, CONFIG_AP_SECURITY, CONFIG_AP_CHANNEL, CONFIG_AP_PASSPHRASE_LENGTH, CONFIG_AP_PASSPHRASE, CONFIG_VALIDITY_VALUE},
#ifdef WICED_COUNTRY_CODE
    .wifi_config.country_code        = WICED_COUNTRY_CODE,
#else
    .wifi_config.country_code        = WICED_DEFAULT_COUNTRY_CODE,
#endif
    .wifi_config.mac_address.octet   = DCT_GENERATED_MAC_ADDRESS,
};

#if defined ( __IAR_SYSTEMS_ICC__ )
4__root int wiced_program_start(void)
{
    /* in iar we must reference dct structure, otherwise it may not be included in */
    /* the dct image */
    return initial_dct.dct_header.write_incomplete;
}
#endif /* if defined ( __IAR_SYSTEMS_ICC__ ) */


/******************************************************
 *               Function Definitions
 ******************************************************/
