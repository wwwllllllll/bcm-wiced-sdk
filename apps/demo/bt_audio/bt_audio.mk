#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Bluetooth_Audio

$(NAME)_SOURCES    := bt_audio.c

$(NAME)_COMPONENTS := libraries/inputs/button \
                      libraries/bt_audio

GLOBAL_DEFINES := BUILDCFG \
                  WICED_USE_AUDIO

GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=1
GLOBAL_DEFINES += RX_PACKET_POOL_SIZE=1


VALID_OSNS_COMBOS  := ThreadX-NetX_Duo
VALID_PLATFORMS    := BCM9WCD1AUDIO
