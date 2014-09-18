#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := SPI_Flash_Library_$(PLATFORM)

$(NAME)_SOURCES := spi_flash.c spi_flash_wiced.c

$(NAME)_DEFINES += SFLASH_SUPPORT_SST_PARTS \
                   SFLASH_SUPPORT_MACRONIX_PARTS \
                   SFLASH_SUPPORT_EON_PARTS

GLOBAL_INCLUDES := .

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)