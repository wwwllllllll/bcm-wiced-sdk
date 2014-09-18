/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "wiced_framework.h"
#include "temp_control_dct.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Put your Xively feed ID, API key, and channel ID here */
#define XIVELY_FEED_ID    "xxxxxxxxxx"
#define XIVELY_API_KEY    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define XIVELY_CHANNEL_ID "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

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
 *                 Global Variables
 ******************************************************/

DEFINE_APP_DCT(user_dct_data_t)
{
    .xively_details_valid = 0,
    .xively_feed_id       = XIVELY_FEED_ID,
    .xively_api_key       = XIVELY_API_KEY,
    .xively_channel_id    = XIVELY_CHANNEL_ID,
    .sample_interval      = 1000,
};

/******************************************************
 *               Function Declarations
 ******************************************************/
