#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_Bluetooth_LE_Stack

GLOBAL_INCLUDES := . \
                   include

ifneq ($(wildcard $(CURDIR)Lib_Bluetooth_LE_Stack.$(RTOS).$(HOST_ARCH).$(BUILD_TYPE).a),)
$(NAME)_PREBUILT_LIBRARY := Lib_Bluetooth_LE_Stack.$(RTOS).$(HOST_ARCH).$(BUILD_TYPE).a
else
# Build from source (Broadcom internal)
include $(CURDIR)LE_src.mk
endif
