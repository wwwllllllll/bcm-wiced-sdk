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
 * Bluetooth SmartBridge Manufacturing Test Application
 *
 * This application enables users to access the local Bluetooth
 * device using a PC manufacturing test application such as BlueTool.
 *
 * Application Instructions
 *   1. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. Run BlueTool on your computer
 *   3. Select Transport | HCI Control | UART
 *   4. Enter the corresponding COM port and set the baud rate to 115200.
 *      Ensure that 'CTS flow control and 'SLIP/Three-wire' are unticked and then
 *      click 'OK'. The HCI command window appears
 *   5. Select View | Log Window. The log window appears
 *   6. Double-click on your desired HCI command to send
 */

#include "wiced.h"
#include "wiced_bt.h"

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

static const wiced_uart_config_t const stdio_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

/* Application entry point
 */
void application_start( )
{
    /* Initialise WICED */
    wiced_init( );

    /* Enter BT manufacturing test mode */
    wiced_bt_start_mfgtest_mode( &stdio_config );
}
