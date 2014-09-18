/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_REBOOT_H_
#define INCLUDED_REBOOT_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/

#ifdef CONSOLE_ENABLE_REBOOT

#define REBOOT_COMMANDS \
    { "reboot",         reboot,       0, DELIMIT, NULL, NULL, "Reboot the device"},

#else /* ifdef CONSOLE_ENABLE_REBOOT */
#define REBOOT_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_REBOOT */

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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
int reboot( int argc, char* argv[] );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_REBOOT_H_ */
