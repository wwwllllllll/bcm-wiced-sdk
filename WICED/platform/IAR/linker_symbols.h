/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                     Macros
 ******************************************************/
#define ENTRY_ADDRESS (wiced_program_start)

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
/*#pragma section="CSTACK" */
extern void* link_stack_end = __section_end("CSTACK");  /* provided by link script */

/******************************************************
 *               Function Declarations
 ******************************************************/
void wiced_program_start(void);

#ifdef __cplusplus
} /*extern "C" */
#endif
