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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_FEED_ID_LEN    (10) /* Feed ID length ranges from 5 to 10 digits */
#define API_KEY_LEN        (48) /* API key length                            */
#define MAX_CHANNEL_ID_LEN (50) /* Maximum Channel ID length                 */

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
    char     xively_feed_id[MAX_FEED_ID_LEN + 1];       /* With terminating null */
    char     xively_api_key[API_KEY_LEN + 1];           /* With terminating null */
    char     xively_channel_id[MAX_CHANNEL_ID_LEN + 1]; /* With terminating null */
    uint32_t sample_interval;
} user_dct_data_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
