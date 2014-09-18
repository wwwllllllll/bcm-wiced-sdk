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
/*
    BTLDR_OR_VECTOR_FLASH      (rx)  : ORIGIN = 0x00400000, LENGTH = 16K
*/
    DCT1_FLASH                 (rx)  : ORIGIN = 0x00404000, LENGTH = 16K
    DCT2_FLASH                 (rx)  : ORIGIN = 0x00408000, LENGTH = 16K
    APP_FLASH                  (rx)  : ORIGIN = 0x0040C000, LENGTH = 976K
    SRAM                       (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

PHDRS
{
    rodata PT_LOAD ;
}

SECTIONS
{
    .rodata :
    {
        dct_location = .;
        KEEP(*(.rodata.initial_dct))
        KEEP(*(.rodata._app_dct))
        dct_end = .;

    } >DCT1_FLASH  AT> DCT1_FLASH :rodata
}

PROVIDE( dct_full_size_loc = LENGTH( DCT1_FLASH ));
PROVIDE( dct_used_size_loc = (dct_end - ORIGIN( DCT1_FLASH )));