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
 * Bluetooth Audio AVDT Sink Application
 *
 * This application demonstrates how to use the Bluetooth Application Layer (BTA)
 *  APIs
 *
 * The application demonstrates the following features ...
 *  - Bluetooth intialization
 *  - AVDT sink
 *  - SBC Decoding
 *  - Audio playback
 *
 *
 * Application Operation
 * The app runs in a thread, the entry point is application_start()
 *
 *
 *    Usage
 *        On startup device will be discoverable and connectable,
 *        allowing a BT audio source to connect and streaming
 *        audio.
 *
 *    Notes
 *        Currently only supports 44.1kHz audio s
 *
 */
#include <stdlib.h>
#include "wiced.h"
#include "wiced_bt_audio.h"
#include "platform_audio.h"


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

/******************************************************
 *               Function Definitions
 ******************************************************/

/*******************************************************************************
 **
 ** Function         application_start
 **
 ** Description      Application entry point for Wiced framework
 **
 ** Returns          nothing
 **
 *******************************************************************************/
void application_start( )
{
    wiced_init( );
    platform_init_audio();

    /* Initialize BT host stack and controller */
    wiced_bt_audio_init( );

    /* Initialise BT AVDT Audio Sink (enable discoverability/connectability and wait for BT Audio Source to connect */
    wiced_bt_audio_start_player();
}




