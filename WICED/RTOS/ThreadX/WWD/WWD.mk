#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := WWD_ThreadX_Interface

GLOBAL_INCLUDES := .

ifeq ($(TOOLCHAIN_NAME),IAR)
$(NAME)_SOURCES  := wwd_rtos.c \
                    low_level_init.c \
                    interrupt_handlers_IAR.s

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

$(NAME)_LINK_FILES := interrupt_handlers_IAR.o

else
$(NAME)_SOURCES  := wwd_rtos.c \
                    low_level_init.c

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

endif

$(NAME)_CHECK_HEADERS := \
                         wwd_rtos.h