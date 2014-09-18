#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_bt_audio

ifneq ($(wildcard $(CURDIR)bt_audio.$(RTOS).$(NETWORK).$(HOST_ARCH).release.a),)
$(NAME)_PREBUILT_LIBRARY := bt_audio.$(RTOS).$(NETWORK).$(HOST_ARCH).release.a
else
# Build from source (Broadcom internal)
include $(CURDIR)bt_audio_src.mk
endif # ifneq ($(wildcard $(CURDIR)bt_stack.$(HOST_ARCH).release.a),)

GLOBAL_INCLUDES := .

