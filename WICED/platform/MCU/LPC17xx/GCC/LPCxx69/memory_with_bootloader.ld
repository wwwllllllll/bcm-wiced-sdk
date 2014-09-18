/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

MEMORY
{
    BTLDR_VECTORS           (rx) : ORIGIN = 0x0,        LENGTH = 512
    CODE_READ_PROTECTION    (rx) : ORIGIN = 0x2FC,      LENGTH = 4
    SHARED_API              (rx) : ORIGIN = 0x300,      LENGTH = 512
    BTLDR_CODE              (rx) : ORIGIN = 0x500,      LENGTH = 15K
    DCT1_FLASH              (rx) : ORIGIN = 0x4100,     LENGTH = 16K
    DCT2_FLASH              (rx) : ORIGIN = 0x8100,     LENGTH = 16K
    APP_CODE                (rx) : ORIGIN = 0xC100,     LENGTH = 474880
    SRAM_32K_LOCAL          (rwx): ORIGIN = 0x10000000, LENGTH = 32K
    SRAM_32K_AHB            (rwx): ORIGIN = 0x2007C000, LENGTH = 32K
}
