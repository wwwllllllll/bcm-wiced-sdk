#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_BT_SmartBridge_Manufacturing_Test

# Enable Bluetooth Manufacturing Test Support
BT_MODE            := MFGTEST

$(NAME)_SOURCES    := bt_smartbridge_mfg_test.c

$(NAME)_COMPONENTS += bluetooth

GLOBAL_DEFINES     += WICED_DISABLE_STDIO     # Disable default STDIO. Manufacturing Test uses raw UART
GLOBAL_DEFINES     += BT_BUS_RX_FIFO_SIZE=512 # Set Bluetooth UART RX FIFO size to large enough number

VALID_PLATFORMS    := BCM9WCDPLUS114 \
                      BCM943341WCD1 \
                      BCM9WCD1AUDIO
