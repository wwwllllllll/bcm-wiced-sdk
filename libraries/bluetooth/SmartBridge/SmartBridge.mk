#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_Bluetooth_SmartBridge

GLOBAL_INCLUDES := .

$(NAME)_INCLUDES := ./internal

$(NAME)_SOURCES := wiced_bt_smartbridge.c \
                   wiced_bt_smartbridge_gatt.c \
                   internal/bt_smartbridge_socket_manager.c \
                   internal/bt_smartbridge_att_cache_manager.c
