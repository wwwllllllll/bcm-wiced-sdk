#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Bluetooth_RFCOMM_Server

$(NAME)_SOURCES    := bt_rfcomm_server.c

$(NAME)_COMPONENTS := bluetooth \
                      bluetooth/protocols/RFCOMM

VALID_PLATFORMS    := BCM9WCDPLUS114 \
                      BCM9WCD1AUDIO