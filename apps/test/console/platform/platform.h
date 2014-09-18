/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_PLATFORM_H_
#define INCLUDED_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#ifdef CONSOLE_ENABLE_PLATFORM_CMD

#define PLATFORM_COMMANDS \
    { "reboot",         reboot,        0, DELIMIT, NULL, NULL,    "Reboot the device"}, \
    { "mcu_powersave",  mcu_powersave, 1, DELIMIT, NULL, "<0|1>", "Enable/disable MCU powersave"},
#else /* ifdef CONSOLE_ENABLE_PLATFORM_CMD */
#define PLATFORM_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_PLATFORM_CMD */

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

/******************************************************
 *               Function Declarations
 ******************************************************/

int reboot( int argc, char* argv[] );
int mcu_powersave( int argc, char *argv[] );
int platform_enable_mcu_powersave ( void );
int platform_disable_mcu_powersave( void );

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /* ifndef INCLUDED_PLATFORM_H_ */
