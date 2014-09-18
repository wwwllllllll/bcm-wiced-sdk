#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_Scan_$(RTOS)_$(NETWORK)

$(NAME)_SOURCES := $(RTOS)_$(NETWORK)_Scan.c

LWIP_NUM_PACKET_BUFFERS_IN_POOL := 4

FreeRTOS_START_STACK := 600
ThreadX_START_STACK  := 600

# Disable watchdog for all WWD apps
GLOBAL_DEFINES += WICED_DISABLE_WATCHDOG
