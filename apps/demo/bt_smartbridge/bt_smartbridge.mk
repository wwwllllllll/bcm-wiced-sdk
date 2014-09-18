#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_BT_SmartBridge

$(NAME)_SOURCES    := bt_smartbridge.c

$(NAME)_COMPONENTS += daemons/HTTP_server \
                      protocols/SNTP \
                      daemons/Gedday \
                      bluetooth \
                      bluetooth/SmartBridge

$(NAME)_RESOURCES  += apps/bt_smartbridge/smartbridge_report.html \
                      apps/bt_smartbridge/data.html \
                      images/brcmlogo.png \
                      images/brcmlogo_line.png \
                      images/favicon.ico \
                      scripts/general_ajax_script.js \
                      scripts/wpad.dat

GLOBAL_DEFINES     := USE_SELF_SIGNED_TLS_CERT

APPLICATION_DCT    := bt_smartbridge_dct.c

VALID_PLATFORMS    := BCM9WCDPLUS114 \
                      BCM943341WCD1 \
                      BCM9WCD1AUDIO
