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
 * UART API Application
 *
 * This application demonstrates how to use the generic
 * WICED UART API to send and receive characters on the UART
 *
 * Features demonstrated
 *  - UART API
 *  - Ring buffer library
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   Follow the prompts printed on the terminal
 *
 * NOTE

 *   Standard I/O definition (STDIO_UART) can be found in
 *   <WICED-SDK>/platforms/<Platform>/platform.h.
 *   To access STDIO_UART using the generic UART API,
 *   it is necessary to add a global define WICED_DISABLE_STDIO
 *   in the uart.mk application makefile
 *
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define RX_BUFFER_SIZE    64
#define TEST_STR          "\r\nType something! Keystrokes are echoed to the terminal ...\r\n> "

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

wiced_uart_config_t uart_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

wiced_ring_buffer_t rx_buffer;
uint8_t             rx_data[RX_BUFFER_SIZE];

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    char c;

    /* Initialise the WICED device */
    wiced_init();

    /* Initialise ring buffer */
    ring_buffer_init(&rx_buffer, rx_data, RX_BUFFER_SIZE );

    /* Initialise UART. A ring buffer is used to hold received characters */
    wiced_uart_init( STDIO_UART, &uart_config, &rx_buffer );

    /* Send a test string to the terminal */
    wiced_uart_transmit_bytes( STDIO_UART, TEST_STR, sizeof( TEST_STR ) - 1 );

    /* Wait for user input. If received, echo it back to the terminal */
    while ( wiced_uart_receive_bytes( STDIO_UART, &c, 1, WICED_NEVER_TIMEOUT ) == WICED_SUCCESS )
    {
        wiced_uart_transmit_bytes( STDIO_UART, &c, 1 );
    }
}
