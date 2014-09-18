#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := WWD_for_$(subst /,_,$(BUS))_$(RTOS)

GLOBAL_INCLUDES := . \
                   include \
                   include/network \
                   include/RTOS \
                   internal/bus_protocols/$(BUS) \
                   internal/chips/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)


$(NAME)_SOURCES := internal/wwd_thread.c \
                   internal/wwd_sdpcm.c \
                   internal/wwd_internal.c \
                   internal/wwd_management.c \
                   internal/wwd_wifi.c \
                   internal/wwd_crypto.c \
                   internal/wwd_logging.c \
                   internal/bus_protocols/wwd_bus_common.c \
                   internal/bus_protocols/$(BUS)/wwd_bus_protocol.c

$(NAME)_CHECK_HEADERS := \
                         internal/wwd_ap.h \
                         internal/wwd_bcmendian.h \
                         internal/wwd_internal.h \
                         internal/wwd_logging.h \
                         internal/wwd_sdpcm.h \
                         internal/wwd_thread.h \
                         internal/bus_protocols/wwd_bus_protocol_interface.h \
                         internal/bus_protocols/$(BUS)/wwd_bus_protocol.h \
                         internal/chips/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/chip_constants.h \
                         include/wwd_assert.h \
                         include/wwd_constants.h \
                         include/wwd_crypto.h \
                         include/wwd_debug.h \
                         include/wwd_events.h \
                         include/wwd_management.h \
                         include/wwd_poll.h \
                         include/wwd_structures.h \
                         include/wwd_wifi.h \
                         include/wwd_wlioctl.h \
                         include/Network/wwd_buffer_interface.h \
                         include/Network/wwd_network_constants.h \
                         include/Network/wwd_network_interface.h \
                         include/platform/wwd_bus_interface.h \
                         include/platform/wwd_platform_interface.h \
                         include/platform/wwd_resource_interface.h \
                         include/platform/wwd_sdio_interface.h \
                         include/platform/wwd_spi_interface.h \
                         include/RTOS/wwd_rtos_interface.h


ifndef NO_WIFI_FIRMWARE
$(NAME)_RESOURCES += firmware/$(WLAN_CHIP)/$(WLAN_CHIP)$(WLAN_CHIP_REVISION).bin
ifeq ($(WLAN_CHIP),43362)
GLOBAL_DEFINES += FIRMWARE_WITH_PMK_CALC_SUPPORT
endif
endif

ifeq ($(WLAN_CHIP),)
$(error ERROR: WLAN_CHIP must be defined in your platform makefile)
endif

ifeq ($(WLAN_CHIP_REVISION),)
$(error ERROR: WLAN_CHIP_REVISION must be defined in your platform makefile)
endif

ifeq ($(HOST_OPENOCD),)
$(error ERROR: HOST_OPENOCD must be defined in your platform makefile)
endif

$(NAME)_SOURCES += internal/chips/$(WLAN_CHIP)$(WLAN_CHIP_REVISION)/wwd_ap.c

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)

