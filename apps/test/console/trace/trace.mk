#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# Make sure to include this trace_macros.h first (to ensure the trace macros are defined accordingly)
GLOBAL_CFLAGS   += -include Apps/console/trace/clocktimer/ARM_CM3_DWT.h
GLOBAL_CXXFLAGS += -include Apps/console/trace/clocktimer/ARM_CM3_DWT.h
GLOBAL_CFLAGS   += -include Apps/console/trace/$(RTOS)_trace.h
GLOBAL_CXXFLAGS += -include Apps/console/trace/$(RTOS)_trace.h

ifeq (FreeRTOS, $(RTOS))
# Needed to store useful tracing information
GLOBAL_DEFINES  += configUSE_TRACE_FACILITY=1
endif

$(NAME)_SOURCES += trace/trace.c
$(NAME)_SOURCES += trace/trace_hook.c

#==============================================================================
# Configuration
#==============================================================================
TRACE_BUFFERED       := 1
TRACE_BUFFERED_PRINT := 1
TRACE_GPIO			 := 1


#==============================================================================
# Defines and sources
#==============================================================================
ifeq (1, $(TRACE_BUFFERED))
$(NAME)_DEFINES += TRACE_ENABLE_BUFFERED
$(NAME)_SOURCES += trace/buffered/buffered_trace.c
$(NAME)_SOURCES += trace/clocktimer/ARM_CM3_DWT.c

ifeq (1, $(TRACE_BUFFERED_PRINT))
$(NAME)_DEFINES += TRACE_ENABLE_BUFFERED_PRINT
$(NAME)_SOURCES += trace/buffered/print/buffered_trace_print.c
endif

ifeq (1, $(TRACE_GPIO))
$(NAME)_DEFINES += TRACE_ENABLE_GPIO
$(NAME)_SOURCES += trace/gpio/gpio_trace.c
# Note that we need to use delayed variable expansion here.
# See: http://www.bell-labs.com/project/nmake/newsletters/issue025.html
$(NAME)_SOURCES += trace/gpio/$$(HOST_MICRO)_gpio_trace.c
endif
endif