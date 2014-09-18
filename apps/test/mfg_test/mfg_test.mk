#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := App_mfg_test

$(NAME)_SOURCES := $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe/wlu_server_shared.c \
                   wlu_server_wiced.c \
                   $(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe/wlu_pipe.c \
                   mfg_test_init.c

$(NAME)_RESOURCES += firmware/$(WLAN_CHIP)/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)-mfgtest.bin
NO_WIFI_FIRMWARE := YES

$(NAME)_INCLUDES := ./dummy \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/include \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/common/include \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wl/exe \
                    ./$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/shared/bcmwifi/include

$(NAME)_DEFINES  := RWL_SERIAL TARGET_wiced

#$(NAME)_COMPONENTS += test/malloc_debug

NO_CRLF_STDIO_REPLACEMENT := YES

GLOBAL_DEFINES += STDIO_BUFFER_SIZE=1024
GLOBAL_DEFINES += WICED_PAYLOAD_MTU=8320
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=2 RX_PACKET_POOL_SIZE=2
GLOBAL_DEFINES += PBUF_POOL_TX_SIZE=2 PBUF_POOL_RX_SIZE=2

#AVOID_GLOMMING_IOVAR AVOID_APSTA SET_COUNTRY_WITH_IOCTL_NOT_IOVAR


