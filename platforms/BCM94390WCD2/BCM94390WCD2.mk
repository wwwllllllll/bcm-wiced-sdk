#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Platform_BCM94390WCD2

WLAN_CHIP            := 4390
WLAN_CHIP_REVISION   := A1
HOST_MCU_FAMILY      := BCM439x
HOST_MCU_VARIANT     := BCM4390
HOST_MCU_PART_NUMBER := BCM4390DKFFBG
HOST_OPENOCD         := BCM439x

# BCM94390WCD2-specific make targets
PLATFORM_TARGETS_MAKEFILE :=$(SOURCE_ROOT)platforms/BCM94390WCD2/BCM94390WCD2_targets.mk

# Add valid MCU-WLAN bus interface(s)
VALID_BUSES :=SoC.4390

ifndef BUS
BUS:=SoC/4390
endif

ifeq ($(BUS),SoC/4390)
WIFI_IMAGE_DOWNLOAD := buffered
else
$(error This platform only supports SoC/4390 bus protocol)
endif

# Global includes
GLOBAL_INCLUDES := .

# Global defines
GLOBAL_DEFINES += $$(if $$(NO_CRLF_STDIO_REPLACEMENT),,CRLF_STDIO_REPLACEMENT) \
                  BCM4390_ENABLE_FULL_PDS_MODE

# Source files
$(NAME)_SOURCES := platform.c

ifneq ($(APP),bootloader)
ifneq ($(MAIN_COMPONENT_PROCESSING),1)
$(info ******************************************************************************** )
$(info The target Wi-Fi MAC address is defined in <WICED-SDK>/generated_mac_address.txt )
$(info Ensure each target device has a unique address!                                  )
$(info ******************************************************************************** )
endif
endif

