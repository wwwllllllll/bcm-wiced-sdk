#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := NoNS

$(NAME)_COMPONENTS += WICED/network/NoNS/WWD

# Define some macros to allow for some network-specific checks
GLOBAL_DEFINES += NETWORK_$(NAME)=1

$(NAME)_INCLUDES :=
$(NAME)_SOURCES  :=
$(NAME)_DEFINES  :=

VALID_RTOS_LIST:= NoOS

NS_WWD_ONLY := TRUE