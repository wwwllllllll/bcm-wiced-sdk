#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := NetX_Duo

NETX_DUO_VERSION := 5.7_sp1

VALID_RTOS_LIST:= ThreadX

$(NAME)_COMPONENTS += WICED/network/NetX_Duo/WWD

ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))
$(NAME)_COMPONENTS += WICED/network/NetX_Duo/WICED
endif

# Define some macros to allow for some network-specific checks
GLOBAL_DEFINES += NETWORK_$(NAME)=1
GLOBAL_DEFINES += $(NAME)_VERSION=$$(SLASH_QUOTE_START)v$(NETX_DUO_VERSION)$$(SLASH_QUOTE_END)
# prevent sending reset for non-blocking disconnect
GLOBAL_DEFINES += NX_INCLUDE_USER_DEFINE_FILE
GLOBAL_DEFINES += __fd_set_defined
GLOBAL_DEFINES += SYS_TIME_H_AVAILABLE

GLOBAL_INCLUDES := ver$(NETX_DUO_VERSION) \
                   WICED


ifdef WICED_ENABLE_TRACEX
# Precompiled library with TraceX
NETX_DUO_LIBRARY_NAME :=NetX_Duo.TraceX.$(HOST_ARCH).$(BUILD_TYPE).a
else
# Precompiled library
NETX_DUO_LIBRARY_NAME :=NetX_Duo.$(HOST_ARCH).$(BUILD_TYPE).a
endif

ifneq ($(wildcard $(CURDIR)$(NETX_DUO_LIBRARY_NAME)),)
# Using a precompiled Library
$(NAME)_PREBUILT_LIBRARY := $(NETX_DUO_LIBRARY_NAME)
else
# Build from source (Broadcom internal)
include $(CURDIR)NetX_Duo_src.mk
endif #ifneq ($(wildcard $(CURDIR)NetX_Duo.$(HOST_ARCH).$(BUILD_TYPE).a),)
