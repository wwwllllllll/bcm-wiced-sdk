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

#ifdef __cplusplus
extern "C" {
#endif

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

extern appliance_config_t       appliance_config;

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_SENSOR_H */
