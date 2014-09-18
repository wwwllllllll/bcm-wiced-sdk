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

#ifdef __cplusplus
extern "C" {
#endif
#include "button.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/* Events for wiced_bt_audio_callback_t */
typedef enum
{
    WICED_BT_AUDIO_EVT_CONFIG, /* Player configuration                 */
    WICED_BT_AUDIO_EVT_GET_BUF, /* Request buffer for audio data        */
    WICED_BT_AUDIO_EVT_GET_PLAYER_STATS, /* Request for player statistics        */
    WICED_BT_AUDIO_EVT_DATA, /* Data to play                         */
    WICED_BT_AUDIO_AVK_EVT_OPEN,
    WICED_BT_AUDIO_AVK_EVT_START,
    WICED_BT_AUDIO_AVK_EVT_STOP,
    WICED_BT_AUDIO_AVK_EVT_SUSPEND,
    WICED_BT_AUDIO_AVK_EVT_CLOSE,
} wiced_bt_audio_event_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/* Event data structure for WICED_BT_AUDIO_EVT_GET_PLAYER_STATS */
typedef struct
{
    uint32_t curr_queue_count; /* Number of unplayed buffers in the queue */
    uint32_t player_overrun_count; /* Number of times player had nothing to play */
    uint32_t cback_count;
} wiced_bt_audio_player_stats_t;

/* Union of event data structures for tWICED_BT_AUDIO_CBACK */
typedef union
{
    void*                         p_audio_data;
    wiced_bt_audio_player_stats_t player_stats;
} wiced_bt_audio_event_data_t;

typedef struct
{
    wiced_button_event_t    evt;
    wiced_button_t*         button;
} wiced_bt_button_event_t;

typedef void (*wiced_bt_audio_callback_t)( wiced_bt_audio_event_t event, wiced_bt_audio_event_data_t* event_data );

/******************************************************
 *                 Global Variables
 ******************************************************/

extern uint8_t wiced_log_enabled;

/******************************************************
 *               Function Declarations
 ******************************************************/

wiced_result_t wiced_bt_audio_init( void );
wiced_result_t wiced_bt_audio_register_callback( wiced_bt_audio_callback_t callback );
wiced_result_t wiced_bt_audio_start_player( void );
wiced_result_t wiced_bt_audio_button_send_event( wiced_button_t* button, wiced_button_event_t evt );

#ifdef __cplusplus
} /* extern "C" */
#endif


