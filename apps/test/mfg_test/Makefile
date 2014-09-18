#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

#
# GNUmakefile for wl/exe
#


UNAME = $(shell uname)

CC:=gcc


CHIPS := 43362A2 \
         4390A1 \
         43341B0

all: wl wlm

wl:  $(addprefix wl,$(CHIPS))

wlm: $(addprefix wlm,$(CHIPS))



ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
HOST:=WIN32
CFLAGS += -I/include
ARCH_EXE_SFX:=.exe
ARCH_DLL_SFX:=.dll
SO_LIB_FLAG = $(LIB_WLM_SO)
else # UNAME
ifeq ($(findstring MINGW,$(UNAME)),MINGW)
CFLAGS += -I/mingw/include -mno-ms-bitfields
HOST:=WIN32
ARCH_EXE_SFX:=.exe
ARCH_DLL_SFX:=.dll
SO_LIB_FLAG = $(LIB_WLM_SO)
else # UNAME
HOST:=LINUX
ARCH_EXE_SFX:=
ARCH_DLL_SFX:=.so
SO_LIB_FLAG = -l$(LIB_WLM_SO_RAW)
endif
endif

# Use VERBOSE=1 to get full output
ifneq ($(VERBOSE),1)
QUIET:=@
else
QUIET:=
endif



CFLAGS += -D BUILDING_WL_APP
CFLAGS += -D$(HOST)
CFLAGS += -DBAND5G
CFLAGS += -DBAND2G
CFLAGS += -DBCMINTERNAL
CFLAGS += -DBCMDBG
CFLAGS += -DBCMWAPI_WPI
CFLAGS += -DBCMWPA2
CFLAGS += -DWLCNT
CFLAGS += -DWLTEST
CFLAGS += -DRWL_SERIAL
CFLAGS += -DWICED_PAYLOAD_MTU=8292
CFLAGS += -DBWL_SMALL_WLU_DUMP_BUF
CFLAGS += -DWLPFN
CFLAGS += -DWLPFN_AUTO_CONNECT
#CFLAGS += -DWLBTAMP
#CFLAGS += -DWLP2P
#CFLAGS += -DBCMCCX
#CFLAGS += -DDEBUG_SERIAL
CFLAGS += -g -O0
#CFLAGS += -Wextra $(CUSTOM_FLAGS)

CFLAGS += -I./dummy



###########################
# Macro to create compilation targets
# $(1) = C source file
# $(2) = Suffix for OBJ variables
# $(3) = extra CFLAGS
define BUILD_C_RULE

#$(OBJ_DIR_$(2))$(notdir $(1:.c=.d)): $(1)
#	$(QUIET)mkdir -p $$(@D)
#	$(QUIET)$(CC) $(CFLAGS) $(3) -M  $$<  > $$(@:.o=.d)

$$(eval -include $(OBJ_DIR_$(2))$(notdir $(1:.c=.d)))

$(OBJ_DIR_$(2))$(notdir $(1:.c=.o)): $(1) Makefile
	$(QUIET)echo Compiling $(1)
	$(QUIET)mkdir -p $$(@D)
	$(QUIET)$(CC) -c $(CFLAGS) $(3) -o $$@ $$<

$$(eval OBJS_$(2) += $(OBJ_DIR_$(2))$(notdir $(1:.c=.o)))

endef




###################
# Macro to common setup variables for each chip
# $(1) = chip
define SET_COMMON_CHIP_FLAGS

$(1)_CFLAGS += -DCHIP$(1)
$(1)_CFLAGS += -I$(1)/include
$(1)_CFLAGS += -I$(1)/common/include
$(1)_CFLAGS += -I$(1)/shared/bcmwifi/include
$(1)_CFLAGS += -I$(1)/wl/exe
$(1)_CFLAGS += -I$(1)/wl/ppr/include


ifeq ($(findstring WIN32,$(HOST)),WIN32)
$(1)_SOURCE_WL += ./wlu_serial_only.c \
                  $(1)/wl/exe/wlu_pipe_win32.c
else
$(1)_SOURCE_WL += $(1)/wl/exe/wlu_linux.c \
                     $(1)/wl/exe/wlu_pipe_linux.c
endif

$(1)_SOURCE_WL += $(1)/wl/exe/wlu.c \
                  $(1)/wl/exe/wlu_cmd.c \
                  $(1)/wl/exe/wlu_iov.c \
                  $(1)/wl/exe/wlu_client_shared.c \
                  $(1)/wl/exe/wlu_pipe.c



$(1)_SOURCE_SHARED += $(1)/shared/miniopt.c \
                      $(1)/shared/bcmutils.c \
                      $(1)/shared/bcm_app_utils.c
$(1)_SOURCE_WLM    += $(1)/wl/exe/wlm.c

# Prefix for produced .obj files
OBJ_DIR_$(1)_WL         :=objects/$(1)/wl/
OBJ_DIR_$(1)_WLM        :=objects/$(1)/wlm/
OBJ_DIR_$(1)_SHARED     :=objects/$(1)/wl_shared/
OBJ_DIR_$(1)_WLM_EX_APP :=objects/$(1)/wlm_example_app/

# Final exe names
$(1)_EXE_WL          := wl$(1)$(ARCH_EXE_SFX)
$(1)_LIB_WLM_SO_RAW  := wiced_wlm_$(1)
$(1)_LIB_WLM_SO      := $$($(1)_LIB_WLM_SO_RAW)$(ARCH_DLL_SFX)
$(1)_EXE_WLM_EXAMPLE := wiced_wlm_example_app_$(1)$(ARCH_EXE_SFX)

CHIP:=$(1)
include $(1)/$(1).mk

$$(foreach src, $$($(1)_SOURCE_WL),         $$(eval $$(call BUILD_C_RULE,$$(src),$(1)_WL,         $$($(1)_CFLAGS) )))
$$(foreach src, $$($(1)_SOURCE_SHARED),     $$(eval $$(call BUILD_C_RULE,$$(src),$(1)_SHARED,     $$($(1)_CFLAGS) )))
$$(foreach src, $$($(1)_SOURCE_WLM),        $$(eval $$(call BUILD_C_RULE,$$(src),$(1)_WLM,        $$($(1)_CFLAGS) -DWLMSO )))
$$(foreach src, $$($(1)_SOURCE_WLM_EX_APP), $$(eval $$(call BUILD_C_RULE,$$(src),$(1)_WLM_EX_APP, $$($(1)_CFLAGS) )))




# Final link targets
wl$(1): $$($(1)_EXE_WL)
wlm$(1): $$($(1)_LIB_WLM_SO)

$$($(1)_EXE_WL): $$(OBJS_$(1)_WL) $$(OBJS_$(1)_SHARED)
	$(QUIET)echo Linking $$@
	$(QUIET)$(CC) $$($(1)_LDFLAGS) -o $$@ $$^
	$(QUIET)echo Finished $$@

$$($(1)_LIB_WLM_SO): $$(OBJS_$(1)_WLM) $$(OBJS_$(1)_WL) $$(OBJS_$(1)_SHARED)
	$(QUIET)echo Linking $$@
	$(QUIET)mkdir -p $$(@D)
	$(QUIET)$(CC) $$($(1)_LDFLAGS) -shared -o $$@ $$^

$$($(1)_EXE_WLM_EXAMPLE): $$($(1)_LIB_WLM_SO) $$(OBJS_$(1)_WLM_EX_APP)
	$(QUIET)echo Linking $$@
	$(QUIET)mkdir -p $$(@D)
	$(QUIET)$(CC) $$($(1)_LDFLAGS) -o $$@ $(SO_LIB_FLAG) $$(OBJS_$(1)_WLM_EX_APP)




endef



#ifeq (43362a2,$(CHIP))
#SOURCE_WL         += $(CHIP)/shared/bcmwifi.c
#endif

#SOURCE_WLM_EX_APP := ./wlm_example_app.c


$(foreach chip, $(CHIPS),$(eval $(call SET_COMMON_CHIP_FLAGS,$(chip))))



# $(EXE_WLM_EXAMPLE)


# Compilation targets






.PHONY: all clean wlm

clean:
	 rm -rfv $(foreach chip,$(CHIPS), $($(chip)_EXE_WL) $($(chip)_LIB_WLM_SO) $($(chip)_EXE_WLM_EXAMPLE) $(OBJ_DIR_$(chip)_WL) $(OBJ_DIR_$(chip)_WLM) $(OBJ_DIR_$(chip)_SHARED) $(OBJ_DIR_$(chip)_WLM_EX_APP) )

