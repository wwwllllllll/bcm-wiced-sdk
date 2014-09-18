#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := FreeRTOS

VERSION := 7.5.2

$(NAME)_COMPONENTS := WICED/RTOS/FreeRTOS/WWD
ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))
$(NAME)_COMPONENTS += WICED/RTOS/FreeRTOS/WICED
endif

# Define some macros to allow for some network-specific checks
GLOBAL_DEFINES += RTOS_$(NAME)=1
GLOBAL_DEFINES += configUSE_MUTEXES
GLOBAL_DEFINES += configUSE_RECURSIVE_MUTEXES
GLOBAL_DEFINES += $(NAME)_VERSION=$$(SLASH_QUOTE_START)v$(VERSION)$$(SLASH_QUOTE_END)

GLOBAL_INCLUDES := ver$(VERSION)/Source/include


$(NAME)_SOURCES :=  ver$(VERSION)/Source/croutine.c \
                    ver$(VERSION)/Source/list.c \
                    ver$(VERSION)/Source/queue.c \
                    ver$(VERSION)/Source/tasks.c \
                    ver$(VERSION)/Source/timers.c \
                    ver$(VERSION)/Source/portable/MemMang/heap_3.c

# Win32_x86 specific sources and includes
$(NAME)_Win32_x86_SOURCES  := ver$(VERSION)/Source/portable/MSVC-MingW/port.c
$(NAME)_Win32_x86_INCLUDES := ver$(VERSION)/Source/portable/MSVC-MingW

# ARM Cortex M3/4 specific sources and includes
ifeq ($(TOOLCHAIN_NAME),IAR)
$(NAME)_ARM_CM3_SOURCES  := ver$(VERSION)/Source/portable/IAR/ARM_CM3/port.c \
                          ver$(VERSION)/Source/portable/IAR/ARM_CM3/portasm.S
$(NAME)_ARM_CM3_INCLUDES := ver$(VERSION)/Source/portable/IAR/ARM_CM3 \
                    WWD/ARM_CM3
else
$(NAME)_ARM_CM3_SOURCES  := ver$(VERSION)/Source/portable/GCC/ARM_CM3/port.c
$(NAME)_ARM_CM3_INCLUDES := ver$(VERSION)/Source/portable/GCC/ARM_CM3
endif
$(NAME)_ARM_CM4_SOURCES  := $($(NAME)_ARM_CM3_SOURCES)
$(NAME)_ARM_CM4_INCLUDES := $($(NAME)_ARM_CM3_INCLUDES)

$(NAME)_SOURCES += $($(NAME)_$(HOST_ARCH)_SOURCES)
GLOBAL_INCLUDES += $($(NAME)_$(HOST_ARCH)_INCLUDES)


