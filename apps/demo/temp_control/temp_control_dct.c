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
#define XIVELY_FEED_ID    ""   /* 5 to 10 digits */
#define XIVELY_API_KEY    ""   /* 48 characters */
#define XIVELY_CHANNEL_ID ""   /* up to 50 characters*/

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
 *               Variable Definitions
 ******************************************************/

DEFINE_APP_DCT(user_dct_data_t)
{
    .xively_feed_id       = XIVELY_FEED_ID,
    .xively_api_key       = XIVELY_API_KEY,
    .xively_channel_id    = XIVELY_CHANNEL_ID,
    .sample_interval      = 1000,
};

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
