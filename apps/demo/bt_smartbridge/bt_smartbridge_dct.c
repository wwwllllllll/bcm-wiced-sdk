/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "wiced_bt_smart_interface.h"
#include "wiced_framework.h"
#include "bt_smartbridge_dct.h"


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
 *               Variable Definitions
 ******************************************************/

DEFINE_APP_DCT(bt_smartbridge_bond_info_dct_t)
{
    .bond_info[0] =
    {
        .peer_address = { .address = { 0 } },
        .address_type =   0,
        .irk          = { 0 },
        .csrk         = { 0 },
        .ltk          = { 0 },
        .rand         = { 0 },
        .ediv         =   0,
    },
    .bond_info[1] =
    {
        .peer_address = { .address = { 0 } },
        .address_type =   0,
        .irk          = { 0 },
        .csrk         = { 0 },
        .ltk          = { 0 },
        .rand         = { 0 },
        .ediv         =   0,
    },
    .bond_info[2] =
    {
        .peer_address = { .address = { 0 } },
        .address_type =   0,
        .irk          = { 0 },
        .csrk         = { 0 },
        .ltk          = { 0 },
        .rand         = { 0 },
        .ediv         =   0,
    },
    .bond_info[3] =
    {
        .peer_address = { .address = { 0 } },
        .address_type =   0,
        .irk          = { 0 },
        .csrk         = { 0 },
        .ltk          = { 0 },
        .rand         = { 0 },
        .ediv         =   0,
    },
    .bond_info[4] =
    {
        .peer_address = { .address = { 0 } },
        .address_type =   0,
        .irk          = { 0 },
        .csrk         = { 0 },
        .ltk          = { 0 },
        .rand         = { 0 },
        .ediv         =   0,
    },
    .current_index = 0,
};

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
