/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
sfi_write = ( DEFINED( sfi_write ) ? sfi_write : 0x0005da81 );
sfi_read = ( DEFINED( sfi_read ) ? sfi_read : 0x0005dbe3 );
sfi_size = ( DEFINED( sfi_size ) ? sfi_size : 0x0005dc77 );
sfi_erase = ( DEFINED( sfi_erase ) ? sfi_erase : 0x0005dc93 );
is_sflash_allowed = ( DEFINED( is_sflash_allowed ) ? is_sflash_allowed : 0x00035201 );
boot_init_config_globals = ( DEFINED( boot_init_config_globals ) ? boot_init_config_globals : 0x00004231 );
boot_init_globals = ( DEFINED( boot_init_globals ) ? boot_init_globals : 0x000042d3 );
boot_config_deconfigureClocks = ( DEFINED( boot_config_deconfigureClocks ) ? boot_config_deconfigureClocks : 0x00003e63 );
boot_config_init_util_clkPhaseCounting = ( DEFINED( boot_config_init_util_clkPhaseCounting ) ? boot_config_init_util_clkPhaseCounting : 0x000042cb );
boot_init_hardware_on_por = ( DEFINED( boot_init_hardware_on_por ) ? boot_init_hardware_on_por : 0x00003f17 );
boot_complete_FLL_programming = ( DEFINED( boot_complete_FLL_programming ) ? boot_complete_FLL_programming : 0x000041e9 );
GPIO_WriteReg = ( DEFINED( GPIO_WriteReg ) ? GPIO_WriteReg : 0x0004a895 );
GPIO_set = ( DEFINED( GPIO_set ) ? GPIO_set : 0x0004a8df );
spiffy_soft_reset_check = ( DEFINED( spiffy_soft_reset_check ) ? spiffy_soft_reset_check : 0x00051a5d );
spiffy_txrxBlock = ( DEFINED( spiffy_txrxBlock ) ? spiffy_txrxBlock : 0x00051b97 );
