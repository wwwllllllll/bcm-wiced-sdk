#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := WWD_FreeRTOS_Interface_$(PLATFORM)

GLOBAL_INCLUDES := . \
                   ./$(PLATFORM_FULL)

$(NAME)_SOURCES := wwd_rtos.c

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

$(NAME)_CHECK_HEADERS := \
                         wwd_FreeRTOS_systick.h \
                         wwd_rtos.h

$(NAME)_ARM_CM3_SOURCES  := ARM_CM3/low_level_init.c
$(NAME)_ARM_CM3_INCLUDES := ./ARM_CM3

$(NAME)_ARM_CM4_SOURCES  := $($(NAME)_ARM_CM3_SOURCES)
$(NAME)_ARM_CM4_INCLUDES := $($(NAME)_ARM_CM3_INCLUDES)

$(NAME)_SOURCES += $($(NAME)_$(HOST_ARCH)_SOURCES)
GLOBAL_INCLUDES += $($(NAME)_$(HOST_ARCH)_INCLUDES)
