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
 * This provides prototypes for images for the web pages of the example web
 * server apps.
 *
 */

#ifndef INCLUDED_BRCMLOGOS_H
#define INCLUDED_BRCMLOGOS_H

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
extern const char          favicon[];
extern const char          brcmlogo[];

extern const unsigned long favicon_size;
extern const unsigned long brcmlogo_size;

/******************************************************
 *               Function Declarations
 ******************************************************/

int process_favicon ( void* socket, char * params, int params_len );
int process_brcmlogo( void* socket, char * params, int params_len );

#ifdef __cplusplus
} /*extern "C" */
#endif

#endif /*ifndef INCLUDED_BRCMLOGOS_H */
