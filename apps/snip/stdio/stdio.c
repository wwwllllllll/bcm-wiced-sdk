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
 * STDIO Application
 *
 * Features demonstrated
 *  - How to use the UART for STDIO operations
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   When the application runs, follow the prompts printed on the terminal.
 *
 */

#include "wiced.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define PROMPT1    "Enter your name (up to 20 characters) \n"
#define PROMPT2    "type any digit from 0 to 9\n"

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

void application_start( )
{
    int i =0;
    char c;
    char name[20];

    memset( name, 0, sizeof( name ) );

    wiced_init();

    /* Remove buffering from all std streams */
    setvbuf( stdin,  NULL, _IONBF, 0 );
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );


    /* Read the users name from standard input */
    printf( "\n" );
    printf( PROMPT1 );
    do
    {
       name[i] = getchar();
    }
    while( ( i < sizeof(name) ) && ( name[i++] != 0x0d ) );

    /* Set the line terminator */
    if ( i != 0 )
    {
        name[--i] = 0x00;
    }

    printf( "Hello %s, "
            "welcome to WICED.\n", name );
    printf( "Ok %s, "PROMPT2, name );
    c = getchar();
    printf( "Your number is %d\n", c - 0x30 );
    printf( "Goodbye\n" );
}
