#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_console_$(RTOS)_$(NETWORK)

#==============================================================================
# Console specific files
#==============================================================================
$(NAME)_SOURCES := console.c \
                   $(RTOS)_$(NETWORK)_wiced_init.c \
                   wifi/wifi.c \
                   ping/$(RTOS)_$(NETWORK)_Ping.c


#==============================================================================
# Includes
#==============================================================================
$(NAME)_INCLUDES := .
-include $(CURDIR)custom.mk


#==============================================================================
# Configuration
#==============================================================================

WIFI_CONFIG_DCT_H := wifi_config_dct.h

FreeRTOS_START_STACK := 600
ThreadX_START_STACK  := 600


#==============================================================================
# Global defines
#==============================================================================
GLOBAL_DEFINES += UART_BUFFER_SIZE=128

ifeq ($(PLATFORM),$(filter $(PLATFORM), BCM94390WCD1 BCM94390WCD2 BCM94390WCD3))
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=14 RX_PACKET_POOL_SIZE=14
GLOBAL_DEFINES += PBUF_POOL_TX_SIZE=8 PBUF_POOL_RX_SIZE=8
else
ifneq ($(PLATFORM),BCM943362WCD2)
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=15 RX_PACKET_POOL_SIZE=15
GLOBAL_DEFINES += PBUF_POOL_TX_SIZE=8 PBUF_POOL_RX_SIZE=8
endif
endif

#==============================================================================
# WPS inclusion
#==============================================================================
CONSOLE_ENABLE_WPS := 1
$(NAME)_DEFINES += CONSOLE_ENABLE_WPS
$(NAME)_SOURCES += wps/wps.c
CONSOLE_ENABLE_P2P := 1
$(NAME)_DEFINES += CONSOLE_ENABLE_P2P
$(NAME)_SOURCES += p2p/p2p.c


#==============================================================================
# Network stack-specific inclusion
#==============================================================================
ifeq ($(NETWORK),NetX)
$(NAME)_SOURCES += NetX/netdb.c

NETX_ADD_BSD := 1
NETX_ADD_DHCP_SERVER := 1
NETX_ADD_DNS := 1

ifdef CONSOLE_ENABLE_WPS
GLOBAL_DEFINES  += ADD_NETX_EAPOL_SUPPORT
endif
endif

ifeq ($(NETWORK),NetX_Duo)
$(NAME)_SOURCES += NetX_Duo/netdb.c

NETX_ADD_BSD := 1
NETX_ADD_DHCP_SERVER := 1
NETX_ADD_DNS := 1

ifdef CONSOLE_ENABLE_WPS
GLOBAL_DEFINES  += ADD_NETX_EAPOL_SUPPORT
endif
endif


#==============================================================================
# iperf inclusion
#==============================================================================
ifndef CONSOLE_NO_IPERF
$(NAME)_COMPONENTS += test/console/iperf
$(NAME)_DEFINES    += CONSOLE_ENABLE_IPERF
endif
