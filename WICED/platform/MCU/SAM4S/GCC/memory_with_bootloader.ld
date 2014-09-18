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
    BTLDR_VECTORS    (rx)  : ORIGIN = 0x00400000, LENGTH = 512
    SHARED_API       (rx)  : ORIGIN = 0x00400200, LENGTH = 512
    BTLDR_CODE       (rx)  : ORIGIN = 0x00400400, LENGTH = 15K
    DCT1_FLASH       (rx)  : ORIGIN = 0x00404000, LENGTH = 16K
    DCT2_FLASH       (rx)  : ORIGIN = 0x00408000, LENGTH = 16K
    APP_CODE         (rx)  : ORIGIN = 0x0040C000, LENGTH = 0xF4000  /* 976K */
    SRAM             (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}
