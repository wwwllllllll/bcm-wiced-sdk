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
 * Main header file for the Appliance App, containing structures used by it etc.
 */

#ifndef INCLUDED_SENSOR_H
#define INCLUDED_SENSOR_H

#include "wwd_wifi.h"
#include "web_server.h"

#ifdef APPLIANCE_ENABLE_WPS
#include "wps_host.h"
#endif /* ifdef APPLIANCE_ENABLE_WPS */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
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
    enum
    {
        CONFIG_NONE = 0,
        CONFIG_WPS_PBC,
        CONFIG_WPS_PIN,
        CONFIG_SCANJOIN,
    } config_type;
    union
    {
        struct
        {
            char                pin[9]; /* extra byte for terminating null */
        } wps_pin;
        struct
        {
            wiced_scan_result_t scanresult;
            char                passphrase[WSEC_MAX_PSK_LEN];
            unsigned char       passphrase_len;
        } scanjoin;
    } vals;
} appliance_config_t;

/******************************************************
 *                 Global Variables
 ******************************************************/
extern appliance_config_t       appliance_config;
extern const url_list_elem_t    config_STA_url_list[];
extern const url_list_elem_t    config_AP_url_list[];

/******************************************************
 *               Function Declarations
 ******************************************************/
void start_dns_server( uint32_t local_addr );
void quit_dns_server( void );
void start_dhcp_server( uint32_t local_addr );
void quit_dhcp_server( void );

#ifdef APPLIANCE_ENABLE_WPS
void do_wps( wiced_wps_mode_t wps_mode, char* pin );
#endif /* ifdef APPLIANCE_ENABLE_WPS */


#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_SENSOR_H */
