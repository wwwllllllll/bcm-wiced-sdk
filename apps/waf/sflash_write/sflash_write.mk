#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_SFlash_write

$(NAME)_SOURCES  := sflash_write.c

ifeq ($(TOOLCHAIN_NAME),IAR)
NoOS_START_STACK := 10000
else
NoOS_START_STACK := 5024
endif

$(NAME)_INCLUDES += ../common/spi_flash .

# This uses cflags instead of the normal includes to avoid being
# relative to the directory of this module
#$(NAME)_CFLAGS += -I$(SPI_FLASH_IMAGE_DIR)

#$(NAME)_DEFINES += FACTORY_RESET_AFTER_SFLASH

# blocking printf so breakpoint calls still print information
#GLOBAL_DEFINES += PRINTF_BLOCKING

#NoOS_START_STACK := 6000

#GLOBAL_LINK_SCRIPT := mfg_spi_flash_link.ld

APP_WWD_ONLY   := 1
GLOBAL_DEFINES := WICED_NO_WIFI
NODCT          := 1

ifeq ($(WIPE),1)
GLOBAL_DEFINES += WIPE_SFLASH
endif
