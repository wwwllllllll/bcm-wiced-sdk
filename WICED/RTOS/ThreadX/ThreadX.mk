#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := ThreadX

THREADX_VERSION := 5.6

$(NAME)_COMPONENTS := WICED/RTOS/ThreadX/WWD
ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))
$(NAME)_COMPONENTS += WICED/RTOS/ThreadX/WICED
endif

# Define some macros to allow for some network-specific checks
GLOBAL_DEFINES += RTOS_$(NAME)=1
GLOBAL_DEFINES += $(NAME)_VERSION=$$(SLASH_QUOTE_START)v$(THREADX_VERSION)$$(SLASH_QUOTE_END)
GLOBAL_INCLUDES := ver$(THREADX_VERSION)

GLOBAL_DEFINES += TX_INCLUDE_USER_DEFINE_FILE

ifneq ($(filter $(HOST_ARCH), ARM_CM3 ARM_CM4),)
THREADX_ARCH:=Cortex_M3_M4
GLOBAL_INCLUDES += ver$(THREADX_VERSION)/Cortex_M3_M4/GCC
endif

ifdef WICED_ENABLE_TRACEX
$(info TRACEX)
THREADX_LIBRARY_NAME :=ThreadX.TraceX.$(HOST_ARCH).$(BUILD_TYPE).a
else
THREADX_LIBRARY_NAME :=ThreadX.$(HOST_ARCH).$(BUILD_TYPE).a
endif

ifneq ($(wildcard $(CURDIR)$(THREADX_LIBRARY_NAME)),)
$(NAME)_PREBUILT_LIBRARY := $(THREADX_LIBRARY_NAME)
else
# Build from source (Broadcom internal)
include $(CURDIR)ThreadX_src.mk
endif # ifneq ($(wildcard $(CURDIR)ThreadX.$(HOST_ARCH).$(BUILD_TYPE).a),)
