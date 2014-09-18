/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_MALLINFO_H_
#define INCLUDED_MALLINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#ifdef CONSOLE_ENABLE_MALLINFO

#define MALLINFO_COMMANDS \
    { "malloc_info",         malloc_info_command,       0, DELIMIT, NULL, NULL, "Print memory allocation information"},

#else /* ifdef CONSOLE_ENABLE_MALLINFO */
#define MALLINFO_COMMANDS
#endif /* ifdef CONSOLE_ENABLE_MALLINFO */

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
int malloc_info_command( int argc, char* argv[] );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_MALLINFO_H_ */
