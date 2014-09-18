/**
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

#include "wiced.h"
#include "wiced_bt.h"
#include "bt_bus.h"
#include "bt_hci.h"
#include "bt_packet_internal.h"
#include "bt_management_hci.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define BT_DEVICE_NAME_MAX_LENGTH 21

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

wiced_result_t bt_management_hci_init( void )
{
    return bt_hci_init();
}

wiced_result_t bt_management_hci_deinit( void )
{
    return bt_hci_deinit();
}

wiced_bool_t bt_management_hci_is_device_connectable( void )
{
    return WICED_FALSE;
}

wiced_bool_t bt_management_hci_is_device_discoverable( void )
{
    return WICED_FALSE;
}
