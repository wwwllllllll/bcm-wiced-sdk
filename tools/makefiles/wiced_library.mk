#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# Some possible inputs into this file
#RTOS        : FreeRTOS, ThreadX
#NETWORK     : LwIP, NetX_Duo, NetX
#HOST_ARCH   : ARM_CM3, ARM_CM4
#NAME        : BESL, Gedday, uSSL, ThreadX, NetX_Duo
#SOURCE_ROOT : ../../../    (needs trailing slash)
#DEBUG       : 1

all: stripped_lib

$(info $(filter wipe_source_for_test,$(MAKECMDGOALS)))
ifeq ($(filter wipe_source_for_test,$(MAKECMDGOALS)),)

LIBRARY_NAME :=$(NAME)

$(info Building Prebuilt Library: $(NAME) )

ifdef RTOS
LIBRARY_NAME :=$(LIBRARY_NAME).$(RTOS)
endif
ifdef NETWORK
LIBRARY_NAME:=$(LIBRARY_NAME).$(NETWORK)
endif

ifndef HOST_ARCH
$(error HOST_ARCH not defined - needed to include correct toolchain)
endif

LIBRARY_NAME:=$(LIBRARY_NAME).$(HOST_ARCH)

CC :=

include $(SOURCE_ROOT)tools/makefiles/wiced_toolchain_common.mk
include $(SOURCE_ROOT)tools/makefiles/wiced_toolchain_ARM_GNU.mk

ifndef CC
$(error No matching toolchain found for architecture $(HOST_ARCH))
endif

$(SOURCE_ROOT)build/$(NAME)/%.o: %.c Makefile
	$(QUIET)$(call MKDIR, $(dir $@))
	$(QUIET)$(ECHO) Compiling $<
	$(QUIET)$(CC) $(CFLAGS) -o $@ $<

$(SOURCE_ROOT)build/$(NAME)/%.o: %.cpp Makefile
	$(QUIET)$(call MKDIR, $(dir $@))
	$(QUIET)$(ECHO) Compiling $<
	$(QUIET)$(CC) $(CFLAGS) -o $@ $<

$(SOURCE_ROOT)build/$(NAME)/%.o: %.S Makefile
	$(QUIET)$(call MKDIR, $(dir $@))
	$(QUIET)$(ECHO) Assembling $<
	$(QUIET)$(AS) $(ASMFLAGS) -o $@ $<

# Standard library defines
CFLAGS += -c -MD -ggdb $(CPU_CFLAGS) $(ENDIAN_CFLAGS_LITTLE) -Wall -fsigned-char -ffunction-sections -Werror -fdata-sections -fno-common

# WICED pre-built library defines
CFLAGS += -DWICED_PREBUILT_LIBS

ifdef DEBUG
CFLAGS += -DDEBUG
ifndef ALWAYS_OPTIMISE
CFLAGS += -O0
else
CFLAGS += -O3
endif
LIBRARY_NAME :=$(LIBRARY_NAME).debug
else
CFLAGS += -O3 -DNDEBUG
LIBRARY_NAME :=$(LIBRARY_NAME).release
endif

ASMFLAGS += $(CPU_ASMFLAGS)

# BESL include
CFLAGS += -I$(SOURCE_ROOT)WICED/security/BESL/host/WICED

#platform_assert
CFLAGS += -I$(SOURCE_ROOT)WICED/platform/$(HOST_ARCH)

# LwIP
ifeq ($(NETWORK),LwIP)
CFLAGS += -I$(SOURCE_ROOT)WICED/network/$(NETWORK)/ver1.4.0.rc1/src/include
CFLAGS += -I$(SOURCE_ROOT)WICED/network/$(NETWORK)/WWD/FreeRTOS
CFLAGS += -I$(SOURCE_ROOT)WICED/network/$(NETWORK)/ver1.4.0.rc1/src/include/ipv4
endif

# NetX
ifeq ($(NETWORK),NetX)
CFLAGS += -I$(SOURCE_ROOT)WICED/network/$(NETWORK)/ver5.5_sp1
CFLAGS  += -DNX_INCLUDE_USER_DEFINE_FILE
CFLAGS += -DTX_INCLUDE_USER_DEFINE_FILE
endif

# NetX_Duo
ifeq ($(NETWORK),NetX_Duo)
CFLAGS += -I$(SOURCE_ROOT)WICED/network/$(NETWORK)/ver5.7_sp1
CFLAGS  += -DNX_INCLUDE_USER_DEFINE_FILE
CFLAGS += -DTX_INCLUDE_USER_DEFINE_FILE
endif

# ThreadX
ifeq ($(RTOS),ThreadX)
CFLAGS += -I$(SOURCE_ROOT)WICED/RTOS/$(RTOS)/ver5.6
CFLAGS += -I$(SOURCE_ROOT)WICED/RTOS/$(RTOS)/ver5.6/Cortex_M3_M4/GCC
CFLAGS += -DTX_INCLUDE_USER_DEFINE_FILE
endif

# FreeRTOS
ifeq ($(RTOS),FreeRTOS)
CFLAGS += -I$(SOURCE_ROOT)WICED/RTOS/$(RTOS)/ver7.5.2/Source/include
CFLAGS += -I$(SOURCE_ROOT)WICED/RTOS/$(RTOS)/ver7.5.2/Source/portable/GCC/ARM_CM3
CFLAGS += -I$(SOURCE_ROOT)WICED/RTOS/$(RTOS)/WWD/ARM_CM3
endif

OBJS := $(addprefix $(SOURCE_ROOT)build/$(NAME)/,$(filter %.o,$(SOURCES:.cpp=.o) $(SOURCES:.c=.o) $(SOURCES:.S=.o)))

$(SOURCE_ROOT)build/$(NAME)/$(LIBRARY_NAME).a: $(OBJS)
	$(QUIET)$(RM) $@
	$(QUIET)$(ECHO) Making Library
	$(if $(wordlist 1,50,     $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 1,50,     $(OBJS)))
	$(if $(wordlist 51,100,   $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 51,100,   $(OBJS)))
	$(if $(wordlist 101,150,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 101,150,  $(OBJS)))
	$(if $(wordlist 151,200,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 151,200,  $(OBJS)))
	$(if $(wordlist 201,250,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 201,250,  $(OBJS)))
	$(if $(wordlist 251,300,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 251,300,  $(OBJS)))
	$(if $(wordlist 301,350,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 301,350,  $(OBJS)))
	$(if $(wordlist 351,400,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 351,400,  $(OBJS)))
	$(if $(wordlist 401,450,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 401,450,  $(OBJS)))
	$(if $(wordlist 451,500,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 451,500,  $(OBJS)))
	$(if $(wordlist 501,550,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 501,550,  $(OBJS)))
	$(if $(wordlist 551,600,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 551,600,  $(OBJS)))
	$(if $(wordlist 601,650,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 601,650,  $(OBJS)))
	$(if $(wordlist 651,700,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 651,700,  $(OBJS)))
	$(if $(wordlist 701,750,  $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 701,750,  $(OBJS)))
	$(if $(wordlist 751,1000, $(OBJS)),$(QUIET)$(AR) -rcs $@ $(wordlist 751,1000, $(OBJS)))


stripped_lib: $(SOURCE_ROOT)build/$(NAME)/$(LIBRARY_NAME).a Makefile
ifdef NO_STRIP_LIBS
	$(QUIET)$(CP) $(SOURCE_ROOT)/build/$(NAME)/$(LIBRARY_NAME).a $(LIBRARY_NAME).a
else
	$(QUIET)$(STRIP) -g -o $(LIBRARY_NAME).a $(SOURCE_ROOT)/build/$(NAME)/$(LIBRARY_NAME).a
endif
	$(QUIET)$(RM) -rf $(SOURCE_ROOT)/build/$(NAME)
	$(QUIET)$(ECHO) ALL DONE


else

ifeq ($(HOST_OS),Win32)
LIB_TOOLS_DIR := \pf8_3\MinGW\msys\1.0\bin\\
EXECUTABLE_SUFFIX  := .exe
endif

LIB_RM    := $(LIB_TOOLS_DIR)rm$(EXECUTABLE_SUFFIX)
LIB_RMDIR := $(LIB_TOOLS_DIR)rmdir$(EXECUTABLE_SUFFIX)
LIB_XARGS := $(LIB_TOOLS_DIR)xargs$(EXECUTABLE_SUFFIX)
LIB_FIND := $(LIB_TOOLS_DIR)find$(EXECUTABLE_SUFFIX)

wipe_source_for_test:
	cd $(KEEP_LIST_DIR) && $(LIB_FIND) . -type f -a -not \( $(foreach SPEC,$(KEEP_LIST), -path "./$(SPEC)" -o ) -false \) -print0 | $(LIB_XARGS) -0 $(LIB_RM)
	cd $(KEEP_LIST_DIR) && $(LIB_FIND) . -type d -empty -print0 | $(LIB_XARGS) -0 --no-run-if-empty $(LIB_RMDIR)

endif