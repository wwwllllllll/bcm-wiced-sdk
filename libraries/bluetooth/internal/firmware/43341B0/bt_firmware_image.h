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
extern "C" {
#endif

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
 *                 Global Variables
 ******************************************************/

#if defined ( BT_MPAF_MODE )
extern const char* const   bt_mpaf_firmware_version;
extern const unsigned char bt_mpaf_firmware_image[];
extern const unsigned long bt_mpaf_firmware_size;
#endif

#if defined ( BT_HCI_MODE ) || defined ( BT_MFGTEST_MODE )
extern const char* const   bt_hci_firmware_version;
extern const unsigned char bt_hci_firmware_image[];
extern const unsigned long bt_hci_firmware_size;
#endif

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif
