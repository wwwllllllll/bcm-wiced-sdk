#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_BT_SmartBridge_Console

$(NAME)_SOURCES    := bt_smartbridge_console.c \
                      console.c \
                      reboot/reboot.c

$(NAME)_COMPONENTS += daemons/HTTP_server \
                      protocols/SNTP \
                      daemons/Gedday \
                      bluetooth \
                      bluetooth/SmartBridge

GLOBAL_DEFINES     += USE_SELF_SIGNED_TLS_CERT \
                      CONSOLE_ENABLE_REBOOT

APPLICATION_DCT    := bt_smartbridge_dct.c

WIFI_CONFIG_DCT_H  := wifi_config_dct.h

VALID_PLATFORMS    := BCM9WCDPLUS114 \
                      BCM943341WCD1 \
                      BCM9WCD1AUDIO