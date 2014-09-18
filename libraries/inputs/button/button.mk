#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME := Lib_wiced_button

ifneq ($(wildcard $(CURDIR)bt_audio.$(RTOS).$(NETWORK).$(HOST_ARCH).release.a),)
$(NAME)_PREBUILT_LIBRARY := bt_audio.$(RTOS).$(NETWORK).$(HOST_ARCH).release.a
else
# Build from source (Broadcom internal)
$(NAME)_SOURCES := button.c
$(NAME)_INCLUDES := .
endif # ifneq ($(wildcard $(CURDIR)bt_stack.$(HOST_ARCH).release.a),)

GLOBAL_INCLUDES := .


$(info  )
$(info +-----------------------------------------------------------------------------------------------------+ )
$(info | BUTTON LIBRARY  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!                                                      | )
$(info +-----------------------------------------------------------------------------------------------------+ )
$(info | WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   WARNING   | )
$(info | This App is using the Button library which is a TEMPORARY ALPHA library.                            | )
$(info |    The Button Library will be removed in the next WICED release.                                    | )
$(info |    If possible, do not use it directly                                                              | )
$(info +-----------------------------------------------------------------------------------------------------+ )
$(info  )