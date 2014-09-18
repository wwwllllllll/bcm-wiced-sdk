#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Ping_webserver

$(NAME)_SOURCES    := ping_webserver.c

$(NAME)_COMPONENTS := daemons/HTTP_server \
                      daemons/Gedday

$(NAME)_RESOURCES  := apps/ping_webserver/top.html \
                      apps/ping_webserver/table.html \
                      images/brcmlogo.png \
                      images/brcmlogo_line.png \
                      images/favicon.ico \
                      scripts/general_ajax_script.js \
                      scripts/wpad.dat

APPLICATION_DCT    := ping_webserver_dct.c
