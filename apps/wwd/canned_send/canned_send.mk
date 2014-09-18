#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Canned_Send_$(RTOS)

$(NAME)_SOURCES   := $(RTOS)_canned_send.c
$(NAME)_INCLUDES  :=
$(NAME)_DEFINES   :=

# Disable watchdog for all WWD apps
GLOBAL_DEFINES    += WICED_DISABLE_WATCHDOG
GLOBAL_DEFINES    += WICED_DISABLE_MCU_POWERSAVE

#To enable printing compile as a debug image. Note that printing requires a much larger stack.
ifneq ($(BUILD_TYPE),debug)
NO_STDIO := 1
GLOBAL_DEFINES    += WICED_DISABLE_STDIO
NoOS_START_STACK  := 700
else
#Large stack needed for printf in debug mode
NoOS_START_STACK  := 4000
endif

VALID_OSNS_COMBOS := NoOS-NoNS
