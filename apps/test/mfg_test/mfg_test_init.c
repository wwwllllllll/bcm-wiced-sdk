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
 * This is the main file for the manufacturing test app
 *
 * To use the manufacturing test application, please
 * read the instructions in the WICED Manufacturing
 * Test User Guide provided in the <WICED-SDK>/Doc
 * directory: WICED-MFG2xx-R.pdf
 *
 */

#include <stdio.h>
#include "wiced.h"

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
 *               Function Declarations
 ******************************************************/

extern int remote_server_exec(int argc, char **argv, void *ifr);

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
#include "internal/wwd_internal.h"
extern wwd_wlan_status_t wwd_wlan_status;
void application_start( )
{
   wiced_init( );

   /* Run the main application function
    * turn off buffers, so IO occurs immediately
    */
   setvbuf(stdin, NULL, _IONBF, 0);
   setvbuf(stdout, NULL, _IONBF, 0);
   setvbuf(stderr, NULL, _IONBF, 0);

   ++wwd_wlan_status.keep_wlan_awake;
   int argc = 2;
   char *argv[] = { "", "" };
   /* Main server process for all transport types */
   remote_server_exec(argc, argv, NULL);

}

