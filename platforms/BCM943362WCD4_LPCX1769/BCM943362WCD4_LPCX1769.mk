#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Platform_BCM943362WCD4_LPCX1769

WLAN_CHIP            := 43362
WLAN_CHIP_REVISION   := A2
HOST_MCU_FAMILY      := LPC17xx
HOST_MCU_VARIANT     := LPCxx69
HOST_MCU_PART_NUMBER := LPC1769FBD100

INTERNAL_MEMORY_RESOURCES = $(ALL_RESOURCES)
JTAG                      = BCM9WCD1EVAL1_LPC

# LPC17xx-specific make targets
PLATFORM_TARGETS_MAKEFILE :=$(SOURCE_ROOT)platforms/BCM943362WCD4_LPCX1769/BCM943362WCD4_LPCX1769_targets.mk

ifndef BUS
BUS := SPI
endif

# Only SPI is supported
VALID_BUSES := SPI

# Use buffer for downloading Wi-Fi firmware image
WIFI_IMAGE_DOWNLOAD := buffered

# Global includes
GLOBAL_INCLUDES := .

# Global defines
# crystal frequency = 12MHz
GLOBAL_DEFINES += CRYSTAL_MAIN_FREQ_IN=12000000
GLOBAL_DEFINES += $$(if $$(NO_CRLF_STDIO_REPLACEMENT),,CRLF_STDIO_REPLACEMENT)
GLOBAL_DEFINES += TX_PACKET_POOL_SIZE=3
GLOBAL_DEFINES += RX_PACKET_POOL_SIZE=3
# Components
$(NAME)_COMPONENTS += drivers/spi_flash

# Source files
$(NAME)_SOURCES := platform.c

ifneq ($(APP),bootloader)
ifneq ($(MAIN_COMPONENT_PROCESSING),1)
$(info +-----------------------------------------------------------------------------------------------------+ )
$(info | IMPORTANT NOTES                                                                                     | )
$(info +-----------------------------------------------------------------------------------------------------+ )
$(info | Wi-Fi MAC Address                                                                                   | )
$(info |    The target Wi-Fi MAC address is defined in <WICED-SDK>/generated_mac_address.txt                 | )
$(info |    Ensure each target device has a unique address.                                                  | )
$(info +-----------------------------------------------------------------------------------------------------+ )
$(info | MCU & Wi-Fi Power Save                                                                              | )
$(info |    It is *critical* that applications using WICED Powersave API functions connect an accurate 32kHz | )
$(info |    reference clock to the sleep clock input pin of the WLAN chip. Please read the WICED Powersave   | )
$(info |    Application Note located in the documentation directory if you plan to use powersave features.   | )
$(info +-----------------------------------------------------------------------------------------------------+ )
endif
endif
