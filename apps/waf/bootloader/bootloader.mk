#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_WICED_Bootloader_$(PLATFORM)

$(NAME)_SOURCES    := bootloader.c

NoOS_START_STACK   := 4000

APP_WWD_ONLY       := 1

GLOBAL_DEFINES     := WICED_NO_WIFI
GLOBAL_DEFINES     += WICED_DISABLE_STDIO
GLOBAL_DEFINES     += WICED_DISABLE_MCU_POWERSAVE

GLOBAL_INCLUDES    += .

$(NAME)_COMPONENTS := filesystems/wicedfs

VALID_OSNS_COMBOS  := NoOS-NoNS
VALID_BUILD_TYPES  := release