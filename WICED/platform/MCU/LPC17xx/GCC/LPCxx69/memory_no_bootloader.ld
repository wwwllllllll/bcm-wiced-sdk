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
    /* No Bootloader or DCT */
    BTLDR_VECTORS           (rx)  : ORIGIN = 0x0,         LENGTH = 512
    CODE_READ_PROTECTION    (rx)  : ORIGIN = 0x2FC,       LENGTH = 4
    APP_CODE                (rx)  : ORIGIN = 0x300,       LENGTH = 523520
    SRAM_32K_LOCAL          (rwx) : ORIGIN = 0x10000000, LENGTH = 32K
    SRAM_32K_AHB            (rwx) : ORIGIN = 0x2007C000, LENGTH = 32K
}
