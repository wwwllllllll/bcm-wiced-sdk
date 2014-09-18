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
    BTLDR_API               (rx)    : ORIGIN = 0x0,         LENGTH = 512
    CODE_READ_PROTECTION    (rx)    : ORIGIN = 0x2FC,       LENGTH = 4
    SRAM_32K_LOCAL          (rwx)   : ORIGIN = 0x10000000,  LENGTH = 32K
    SRAM_32K_AHB            (rwx)   : ORIGIN = 0x2007C000,  LENGTH = 32K
    OTA_HDR                 (rx)    : ORIGIN = 0x1FFFFE00,  LENGTH = 512  /* App Header for OTA only goes in SFLASH, hence does not need a valid address */
}
