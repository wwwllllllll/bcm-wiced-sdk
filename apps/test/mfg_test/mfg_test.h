/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#ifndef INCLUDED_MFG_TEST_H
#define INCLUDED_MFG_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                     Macros
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
 *                 Global Variables
 ******************************************************/
extern char g_rem_ifname[];
extern unsigned short defined_debug;
extern int remote_type;

/******************************************************
 *               Function Declarations
 ******************************************************/
int remote_server_exec(int argc, char **argv, void *ifr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* INCLUDED_MFG_TEST_H */
