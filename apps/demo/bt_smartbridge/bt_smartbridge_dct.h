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
 */
#pragma once

#include "wiced_bt_smart_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Maximum number of bond information stored in the DCT */
#define MAX_BOND_INFO (5)


/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)

typedef struct
{
    wiced_bt_smart_bond_info_t bond_info[MAX_BOND_INFO]; /* Bond information                      */
    uint32_t                   current_index;            /* Index of the bond info array to store */
} bt_smartbridge_bond_info_dct_t;

#pragma pack()

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Global Variables
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
