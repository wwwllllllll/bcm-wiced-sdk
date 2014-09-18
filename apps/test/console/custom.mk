#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

ifeq ($(findstring custom.h, $(wildcard $(CURDIR)*.h)), custom.h)

$(NAME)_DEFINES += USE_CUSTOM_COMMANDS


#==============================================================================
# Configuration
#==============================================================================
ALL_TESTS := 1

ifeq (1, $(ALL_TESTS))
MALLINFO	   := 1
PLATFORM_CMD   := 1
THREADS        := 1
TRACE		   := 0
endif


#==============================================================================
# Defines and sources
#==============================================================================
#$(NAME)_DEFINES += TCP_ENABLE_WICED_CLIENT

#ifeq (1, $(IPERF))
#
## Make sure iperf will fit onto the specified platform in the specified build configuration
#VALID_IPERF_DEBUG_PLATFORMS   := BCM94319WICED5 BCM943362WCD2 BCM943362WCD3 BCM943362WCD4
#VALID_IPERF_RELEASE_PLATFORMS := AMPAK_E225413 BCM94319WICED2 BCM94319WICED3 BCM94319WICED5 BCM943362WCD1 BCM943362WCD2 BCM943362WCD3 BCM943362WCD4 STM3210E_Eval ISM4319_M3_L44 SAM3S_EK
#
#ifeq ($(BUILD_TYPE),debug)
#    # Debug build
#    ifeq (,$(findstring $(PLATFORM),$(VALID_IPERF_DEBUG_PLATFORMS)))
#        $(warning ************************************************************* )
#        $(warning iperf cannot be built in debug mode on platform $(PLATFORM) since it will not fit in flash. The following is a list of platforms allowed in debug mode: $(VALID_IPERF_DEBUG_PLATFORMS))
#        $(warning Console app will be built without iperf ! )
#        $(warning ************************************************************* )
#    else
#        $(NAME)_COMPONENTS += common/iperf
#        $(NAME)_DEFINES += CONSOLE_ENABLE_IPERF
#    endif
#else
#    # Release build
#    ifeq (,$(findstring $(PLATFORM),$(VALID_IPERF_RELEASE_PLATFORMS)))
#        $(warning ************************************************************* )
#        $(warning iperf cannot be built in release (non-debug) mode on platform $(PLATFORM) since it will not fit in flash. The following is a list of platforms allowed in release mode: $(VALID_IPERF_RELEASE_PLATFORMS) )
#        $(warning Console app will be built without iperf ! )
#        $(warning ************************************************************* )
#    else
#        $(NAME)_COMPONENTS += common/iperf
#        $(NAME)_DEFINES += CONSOLE_ENABLE_IPERF
#    endif
#endif # ifeq ($(BUILD_TYPE),debug)
#endif # ifeq (IPERF, 1)

ifeq (1, $(MALLINFO))
$(NAME)_DEFINES += CONSOLE_ENABLE_MALLINFO
$(NAME)_SOURCES += mallinfo/mallinfo.c
endif

ifeq (1, $(PLATFORM_CMD))
$(NAME)_DEFINES += CONSOLE_ENABLE_PLATFORM_CMD
$(NAME)_SOURCES += platform/platform.c
endif

ifeq (1, $(THREADS))
$(NAME)_DEFINES += CONSOLE_ENABLE_THREADS
$(NAME)_SOURCES += thread/thread.c
endif

ifeq (1, $(TRACE))
$(NAME)_DEFINES += CONSOLE_ENABLE_TRACE
include $(CURDIR)trace/trace.mk
endif


endif