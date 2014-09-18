#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_Bluetooth_RFCOMM

GLOBAL_INCLUDES := . \
                   ./MPAF

$(NAME)_SOURCES := MPAF/wiced_bt_rfcomm.c

$(NAME)_INCLUDES := ../../internal/transport/MPAF \
                    ../../internal/framework/management/MPAF \
                    ../../internal/framework/packet