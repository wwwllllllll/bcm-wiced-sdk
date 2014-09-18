#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Apsta

$(NAME)_SOURCES := apsta.c

$(NAME)_RESOURCES  := apps/apsta/ap_top.html \
                      images/brcmlogo.png \
                      images/brcmlogo_line.png \
                      images/favicon.ico

GLOBAL_DEFINES :=

WIFI_CONFIG_DCT_H := wifi_config_dct.h