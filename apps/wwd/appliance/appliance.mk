#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_$(RTOS)_$(NETWORK)_$(PLATFORM)_appliance

$(NAME)_SOURCES := $(RTOS)_$(NETWORK)_appliance.c  \
                   AP_content.c \
                   STA_content.c \
                   web_server_common.c \
                   $(RTOS)_$(NETWORK)_web_server.c \
                   $(RTOS)_$(NETWORK)_dhcp_server.c \
                   $(RTOS)_$(NETWORK)_dns_server.c \
                   brcmlogos.c \
                   icons.c

$(NAME)_DEFINES := WEB_SERVER_NO_PRINT

# Disable watchdog for all WWD apps
GLOBAL_DEFINES := WICED_DISABLE_WATCHDOG

LWIP_NUM_PACKET_BUFFERS_IN_POOL := 8

FreeRTOS_START_STACK := 600
ThreadX_START_STACK  := 600


# WPS inclusion
#$(NAME)_COMPONENTS += BESL/wps
#$(NAME)_DEFINES += APPLIANCE_ENABLE_WPS
#$(NAME)_SOURCES += appliance_wps.c

VALID_OSNS_COMBOS := FreeRTOS-LwIP