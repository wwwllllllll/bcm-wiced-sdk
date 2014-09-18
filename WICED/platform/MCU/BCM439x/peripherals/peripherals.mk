#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := BCM439x_Peripheral_Drivers

ifneq ($(wildcard $(CURDIR)BCM439x_Peripheral_Drivers.$(HOST_ARCH).$(BUILD_TYPE).a),)
$(NAME)_PREBUILT_LIBRARY := BCM439x_Peripheral_Drivers.$(HOST_ARCH).$(BUILD_TYPE).a
else
include $(CURDIR)peripherals_src.mk
endif
