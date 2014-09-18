#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_Bluetooth

################################################################################
# Build Variants
#
# Define the following macro(s) in application makefile to overwrite the default
# settings.
#
# BT_MODE          : MPAF, HCI, DUAL, MFGTEST
# BT_TRANSPORT_BUS : UART
# BT_STACK         : NoStack, LE
################################################################################

################################################################################
# Supported variants                                                           #
################################################################################

SUPPORTED_BT_CHIPS           := 20702B0 43341B0
SUPPORTED_BT_TRANSPORT_BUSES := UART
SUPPORTED_BT_STACKS          := LE
SUPPORTED_BT_MODES           := DUAL HCI MPAF MFGTEST

################################################################################
# Default settings                                                             #
################################################################################

ifndef BT_TRANSPORT_BUS
BT_TRANSPORT_BUS := UART
endif

ifndef BT_STACK
BT_STACK := LE
endif

COMPLETE_BT_CHIP_NAME := $(BT_CHIP)$(BT_CHIP_REVISION)

################################################################################
# Add defines for specific variants and check variants for validity            #
################################################################################

ifndef BT_CHIP
$(error ERROR: BT_CHIP is undefined!)
endif

ifndef BT_CHIP_REVISION
$(error ERROR: BT_CHIP_REVISION is undefined!)
endif

ifeq ($(COMPLETE_BT_CHIP_NAME),$(filter $(COMPLETE_BT_CHIP_NAME), $(SUPPORTED_BT_CHIPS)))
else
$(error ERROR: Selected BT_CHIP and BT_CHIP_REVISION is unsupported!)
endif

ifndef BT_MODE
$(error ERROR: BT_MODE is undefined!)
endif

ifeq ($(BT_MODE),$(filter $(BT_MODE), $(SUPPORTED_BT_MODES)))
else
$(error ERROR: Selected BT_MODE is unsupported!)
endif

ifndef BT_TRANSPORT_BUS
$(error ERROR: BT_TRANSPORT_BUS is undefined!)
endif

ifeq ($(BT_TRANSPORT_BUS),$(filter $(BT_TRANSPORT_BUS),$(SUPPORTED_BT_TRANSPORT_BUSES)))
else
$(error ERROR: Selected BT_TRANSPORT_BUS is unsupported!)
endif

ifndef BT_STACK
$(error ERROR: BT_STACK is undefined!)
endif

ifeq ($(BT_STACK),$(filter $(BT_STACK), $(SUPPORTED_BT_STACKS)))
else
$(error ERROR: Selected BT_STACK is unsupported!)
endif

################################################################################
# Specify global include directories                                           #
################################################################################

GLOBAL_INCLUDES  := include \
                    internal/bus \
                    internal/firmware \
                    internal/firmware/$(COMPLETE_BT_CHIP_NAME) \
                    internal/framework/management \
                    internal/framework/packet \
                    internal/framework/utilities \
                    internal/framework/utilities/linked_list \
                    internal/stack \
                    internal/transport/driver \
                    internal/transport/thread

################################################################################
# Specify local include directories and source files                           #
################################################################################

ifeq ($(BT_MODE),$(filter $(BT_MODE),MPAF DUAL))
$(NAME)_DEFINES  += BT_MPAF_MODE

GLOBAL_INCLUDES  += internal/framework/management/MPAF \
                    internal/transport/MPAF \
                    internal/transport/HCI

$(NAME)_SOURCES  += internal/framework/management/MPAF/bt_management_mpaf.c \
                    internal/transport/MPAF/bt_mpaf.c \
                    internal/transport/MPAF/$(BT_TRANSPORT_BUS)/bt_transport_driver_receive.c \
                    internal/firmware/$(COMPLETE_BT_CHIP_NAME)/bt_mpaf_firmware_image.c

endif # ifeq ($(BT_MODE),$(filter $(BT_MODE),MPAF DUAL))

ifeq ($(BT_MODE),$(filter $(BT_MODE),HCI DUAL))
$(NAME)_DEFINES  += BT_HCI_MODE

GLOBAL_INCLUDES  += internal/framework/management/HCI \
                    internal/transport/HCI

$(NAME)_SOURCES  += internal/framework/management/HCI/bt_management_hci.c \
                    internal/transport/HCI/bt_hci.c \
                    internal/transport/HCI/$(BT_TRANSPORT_BUS)/bt_transport_driver_receive.c \
                    internal/firmware/$(COMPLETE_BT_CHIP_NAME)/bt_hci_firmware_image.c

endif # ifeq ($(BT_MODE),$(filter $(BT_MODE),HCI DUAL))

ifeq ($(BT_MODE),MFGTEST)
$(NAME)_DEFINES  += BT_MFGTEST_MODE

GLOBAL_INCLUDES  += internal/transport/MFGTEST \
                    internal/transport/HCI

$(NAME)_SOURCES  += internal/transport/MFGTEST/bt_mfgtest.c \
                    internal/transport/MFGTEST/$(BT_TRANSPORT_BUS)/bt_transport_driver_receive.c \
                    internal/firmware/$(COMPLETE_BT_CHIP_NAME)/bt_hci_firmware_image.c
endif

ifeq ($(BT_STACK),$(filter $(BT_STACK),LE))
# 23 bytes of ATT MTU size + 4 bytes of L2CAP header size
$(NAME)_DEFINES  += HCI_ACL_DATA_SIZE=27

$(NAME)_COMPONENTS := bluetooth/internal/stack/$(BT_STACK)

$(NAME)_SOURCES  += internal/framework/utilities/attribute/bt_smart_attribute.c
endif # ifeq ($(BT_STACK),$(filter $(BT_STACK),LE))

$(NAME)_SOURCES  += internal/bus/$(BT_TRANSPORT_BUS)/bt_bus.c \
                    internal/firmware/bt_firmware.c \
                    internal/transport/driver/$(BT_TRANSPORT_BUS)/bt_transport_driver.c \
                    internal/transport/thread/bt_transport_thread.c \
                    internal/framework/packet/bt_packet.c \
                    internal/framework/management/bt_management.c \
                    internal/framework/utilities/linked_list/bt_linked_list.c
