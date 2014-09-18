/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "wiced_management.h"
#include "wiced_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

#define WICED_TO_WWD_INTERFACE( interface )    ((interface==WICED_STA_INTERFACE)? WWD_STA_INTERFACE:WWD_AP_INTERFACE)

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
    uint16_t          received_byte_count;
    uint8_t           received_cooee_data[512];
    wiced_semaphore_t sniff_complete;
    wiced_mac_t         initiator_mac;
    wiced_mac_t         ap_bssid;
    wiced_bool_t      wiced_cooee_complete;
    uint16_t          size_of_zero_data_packet;
    uint32_t          received_segment_bitmap[32];
    wiced_bool_t      scan_complete;
    uint8_t*          user_processed_data;
} wiced_cooee_workspace_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/* WICED <-> Platform API */
extern wiced_result_t wiced_platform_init( void );

/* WICED <-> RTOS API */
extern wiced_result_t wiced_rtos_init  ( void );
extern wiced_result_t wiced_rtos_deinit( void );

/* WICED <-> Network API */
extern wiced_result_t wiced_network_init  ( void );
extern wiced_result_t wiced_network_deinit( void );
extern wiced_result_t wiced_join_ap       ( void );
extern wiced_result_t wiced_leave_ap      ( void );
extern wiced_result_t wiced_start_ap      ( wiced_ssid_t* ssid, wiced_security_t security, const char* key, uint8_t channel);
extern wiced_result_t wiced_stop_ap       ( void );
extern wiced_result_t wiced_ip_up         ( wiced_interface_t interface, wiced_network_config_t config, const wiced_ip_setting_t* ip_settings );
extern wiced_result_t wiced_ip_down       ( wiced_interface_t interface );

/* WICED <-> Network Link Management API */
/* NOTE:
 * The notify link functions below are called from within the context of the WICED thread
 * The link handler functions below are called from within the context of the Network Worker thread */
extern void           wiced_network_notify_link_up    ( void );
extern void           wiced_network_notify_link_down  ( void );
extern wiced_result_t wiced_network_link_down_handler ( void* arg );
extern wiced_result_t wiced_network_link_up_handler   ( void* arg );
extern wiced_result_t wiced_network_link_renew_handler( void );

/* Wiced Cooee API*/
extern wiced_result_t wiced_wifi_cooee( wiced_cooee_workspace_t* workspace );

/* Entry point for user Application */
extern void application_start          ( void );


#ifdef __cplusplus
} /*extern "C" */
#endif
