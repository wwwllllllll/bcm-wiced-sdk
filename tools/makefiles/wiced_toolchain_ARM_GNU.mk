#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

ARM_GNU_ARCH_LIST := ARM_CM3 \
                     ARM_CM4

ifneq ($(filter $(HOST_ARCH), $(ARM_GNU_ARCH_LIST)),)

HOST_INSTRUCTION_SET := THUMB

TOOLCHAIN_PREFIX  := arm-none-eabi-

ifeq ($(HOST_OS),Win32)
################
# Windows settings
################


TOOLCHAIN_PATH    := $(TOOLS_ROOT)/ARM_GNU/bin/Win32/
GDBINIT_STRING     = shell start /B $(OPENOCD_FULL_NAME) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD)_gdb_jtag.cfg -l $(OPENOCD_LOG_FILE)
GDB_COMMAND        = cmd /C $(call CONV_SLASHES, $(TOOLCHAIN_PATH))$(TOOLCHAIN_PREFIX)gdb$(EXECUTABLE_SUFFIX)

else  # Win32
ifeq ($(HOST_OS),Linux32)
################
# Linux 32-bit settings
################


TOOLCHAIN_PATH    := $(TOOLS_ROOT)/ARM_GNU/bin/Linux32/
GDBINIT_STRING     = 'shell $(COMMON_TOOLS_PATH)dash -c "trap \\"\\" 2;$(OPENOCD_FULL_NAME) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD)_gdb_jtag.cfg -l $(OPENOCD_LOG_FILE) &"'
GDB_COMMAND        = "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)gdb"

else # Linux32
ifeq ($(HOST_OS),Linux64)
################
# Linux 64-bit settings
################

TOOLCHAIN_PATH    := $(TOOLS_ROOT)/ARM_GNU/bin/Linux64/
GDBINIT_STRING     = 'shell $(COMMON_TOOLS_PATH)dash -c "trap \\"\\" 2;$(OPENOCD_FULL_NAME) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD)_gdb_jtag.cfg -l $(OPENOCD_LOG_FILE) &"'
GDB_COMMAND        = "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)gdb"

else # Linux64
ifeq ($(HOST_OS),OSX)
################
# OSX settings
################

TOOLCHAIN_PATH    := $(TOOLS_ROOT)/ARM_GNU/bin/OSX/
GDBINIT_STRING     = 'shell $(COMMON_TOOLS_PATH)dash -c "trap \\"\\" 2;$(OPENOCD_FULL_NAME) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD)_gdb_jtag.cfg -l $(OPENOCD_LOG_FILE) &"'
GDB_COMMAND        = "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)gdb"

else # OSX

$(error incorrect 'make' used ($(MAKE)) - please use:  (Windows) .\make.exe <target_string>    (OS X, Linux) ./make <target_string>)
endif # OSX
endif # Linux64
endif # Linux32
endif # Win32


# Set shortcuts to the compiler and other tools
ifneq ($(IAR),1)
#GCC compiler

# Notes on C++ options:
# The next two CXXFLAGS reduce the size of C++ code by removing unneeded
# features. For example, these flags reduced the size of a console app build
# (with C++ iperf) from 604716kB of flash to 577580kB of flash and 46756kB of
# RAM to 46680kB of RAM.
#
# -fno-rtti
# Disable generation of information about every class with virtual functions for
# use by the C++ runtime type identification features (dynamic_cast and typeid).
# Disabling RTTI eliminates several KB of support code from the C++ runtime
# library (assuming that you don't link with code that uses RTTI).
#
# -fno-exceptions
# Stop generating extra code needed to propagate exceptions, which can produce
# significant data size overhead. Disabling exception handling eliminates
# several KB of support code from the C++ runtime library (assuming that you
# don't link external code that uses exception handling).

CC      := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)gcc$(EXECUTABLE_SUFFIX)" -isystem $(TOOLCHAIN_PATH)../../include -isystem $(TOOLCHAIN_PATH)../../lib/include -isystem $(TOOLCHAIN_PATH)../../lib/include-fixed
CXX     := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)g++$(EXECUTABLE_SUFFIX)"
AS      := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)as$(EXECUTABLE_SUFFIX)"
AR      := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)ar$(EXECUTABLE_SUFFIX)"

ADD_COMPILER_SPECIFIC_STANDARD_CFLAGS   = $(1) -Wall -fsigned-char -ffunction-sections -fdata-sections -fno-common
ADD_COMPILER_SPECIFIC_STANDARD_CXXFLAGS = $(1) -Wall -fsigned-char -ffunction-sections -fdata-sections -fno-common -fno-rtti -fno-exceptions
ADD_COMPILER_SPECIFIC_STANDARD_ADMFLAGS = $(1)
COMPILER_SPECIFIC_PEDANTIC_CFLAGS  := $(COMPILER_SPECIFIC_STANDARD_CFLAGS) -Werror -Wstrict-prototypes  -W -Wshadow  -Wwrite-strings -pedantic -std=c99 -Wconversion -Wextra -Wdeclaration-after-statement -Wconversion -Waddress -Wlogical-op -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wmissing-field-initializers -Wdouble-promotion -Wswitch-enum -Wswitch-default -Wuninitialized -Wunknown-pragmas -Wfloat-equal  -Wundef  -Wshadow # -Wcast-qual -Wtraditional -Wtraditional-conversion
COMPILER_SPECIFIC_ARFLAGS_CREATE   := -rcs
COMPILER_SPECIFIC_ARFLAGS_ADD      := -rcs
COMPILER_SPECIFIC_ARFLAGS_VERBOSE  := -v
COMPILER_SPECIFIC_DEBUG_CFLAGS     := -DDEBUG -ggdb -O0
COMPILER_SPECIFIC_DEBUG_CXXFLAGS   := -DDEBUG -ggdb -O0
COMPILER_SPECIFIC_DEBUG_ASFLAGS    := --defsym DEBUG=1 -ggdb
COMPILER_SPECIFIC_DEBUG_LDFLAGS    := -Wl,--gc-sections -Wl,--cref
COMPILER_SPECIFIC_RELEASE_CFLAGS   := -DNDEBUG -ggdb -O3
COMPILER_SPECIFIC_RELEASE_CXXFLAGS := -DNDEBUG -ggdb -O3
COMPILER_SPECIFIC_RELEASE_ASFLAGS  := -ggdb
COMPILER_SPECIFIC_RELEASE_LDFLAGS  := -Wl,--gc-sections -Wl,-O3 -Wl,--cref
COMPILER_SPECIFIC_DEPS_FLAG        := -MD
COMPILER_SPECIFIC_COMP_ONLY_FLAG   := -c
COMPILER_SPECIFIC_LINK_MAP         =  -Wl,-Map,$(1)
COMPILER_SPECIFIC_LINK_FILES       =  -Wl,--start-group $(1) -Wl,--end-group
COMPILER_SPECIFIC_LINK_SCRIPT_DEFINE_OPTION = -Wl$(COMMA)-T
COMPILER_SPECIFIC_LINK_SCRIPT      =  $(addprefix -Wl$(COMMA)-T ,$(1))
LINKER                             := $(CXX) --static -Wl,-static -Wl,--warn-common
LINK_SCRIPT_SUFFIX                 := .ld
TOOLCHAIN_NAME := GCC
OPTIONS_IN_FILE_OPTION    := @

# Chip specific flags for GCC
ifeq ($(HOST_ARCH),ARM_CM4)
CPU_CFLAGS     := -mthumb -mcpu=cortex-m4
CPU_CXXFLAGS   := -mthumb -mcpu=cortex-m4
CPU_ASMFLAGS   := -mcpu=cortex-m4 -mfpu=softvfp
CPU_LDFLAGS    := -mthumb -mcpu=cortex-m4 -Wl,-A,thumb
endif

ifeq ($(HOST_ARCH),ARM_CM3)
CPU_CFLAGS   := -mthumb -mcpu=cortex-m3
CPU_CXXFLAGS := -mthumb -mcpu=cortex-m3
CPU_ASMFLAGS := -mcpu=cortex-m3 -mfpu=softvfp
CPU_LDFLAGS  := -mthumb -mcpu=cortex-m3 -Wl,-A,thumb
endif

ENDIAN_CFLAGS_LITTLE   := -mlittle-endian
ENDIAN_CXXFLAGS_LITTLE := -mlittle-endian
ENDIAN_ASMFLAGS_LITTLE :=
ENDIAN_LDFLAGS_LITTLE  := -mlittle-endian

# $(1) is map file, $(2) is CSV output file
COMPILER_SPECIFIC_MAPFILE_TO_CSV = $(PERL) $(MAPFILE_PARSER) -a $(1) > $(2)

MAPFILE_PARSER            :=$(TOOLS_ROOT)/mapfile_parser/map_parse_gcc.pl

# $(1) is map file, $(2) is CSV output file
COMPILER_SPECIFIC_MAPFILE_DISPLAY_SUMMARY = $(PERL) $(MAPFILE_PARSER) $(1)



else
#IAR compiler
CC := $(IAR_TOOLCHAIN_PATH)iccarm.exe
AS := $(IAR_TOOLCHAIN_PATH)iasmarm.exe
AR := $(IAR_TOOLCHAIN_PATH)iarchive.exe
LN := $(IAR_TOOLCHAIN_PATH)ilinkarm.exe


#file for compilation results, we will use this file to generate build results table
IAR_BUILD_RESULTS_FILE	   := $(OUTPUT_DIR)/iar_build_results_tmp.txt

COMPILER_SPECIFIC_PEDANTIC_CFLAGS     := --char_is_signed --strict --warnings_are_errors
IAR_IDENTIFIER_DEFINE				  := -DIAR_TOOLCHAIN
# Suppress unix line ending warning
ADD_COMPILER_SPECIFIC_STANDARD_CFLAGS := --char_is_signed -e $(IAR_IDENTIFIER_DEFINE) --diag_suppress Pa050
COMPILER_SPECIFIC_ARFLAGS_CREATE      := --create -o
COMPILER_SPECIFIC_ARFLAGS_ADD         := -r -o
COMPILER_SPECIFIC_ARFLAGS_VERBOSE     :=
COMPILER_SPECIFIC_DEPS_FLAG            = --dependencies=m +
COMPILER_SPECIFIC_DEBUG_CFLAGS        := -DDEBUG --debug -On
COMPILER_SPECIFIC_DEBUG_CXXFLAGS      := -DDEBUG --debug -On
COMPILER_SPECIFIC_DEBUG_ASFLAGS       :=
COMPILER_SPECIFIC_DEBUG_LDFLAGS       :=
COMPILER_SPECIFIC_RELEASE_CFLAGS      := -DNDEBUG --debug -Oh
COMPILER_SPECIFIC_RELEASE_CXXFLAGS    := -DNDEBUG --debug -Oh
COMPILER_SPECIFIC_RELEASE_ASFLAGS     :=
COMPILER_SPECIFIC_RELEASE_LDFLAGS     :=
COMPILER_SPECIFIC_LINK_MAP            =  --map $(1)
COMPILER_SPECIFIC_LINK_FILES          =  $(1)
COMPILER_SPECIFIC_LINK_SCRIPT_DEFINE_OPTION = --config
COMPILER_SPECIFIC_LINK_SCRIPT         =  --config $(1)
ASM_COMPILER_FLAGS					  = $(IAR_IDENTIFIER_DEFINE) -D$(RTOS)


COMPILER_SPECIFIC_STDOUT_REDIRECT     := >> $(IAR_BUILD_RESULTS_FILE)
#force not to use default libraries
LINKER 							      :=  $(LN) --entry wiced_program_start
LINK_SCRIPT_SUFFIX                    := .icf
#--no_library_search
OPTIONS_IN_FILE_OPTION                := -f
TOOLCHAIN_NAME                        := IAR

# $(1) is map file, $(2) is CSV output file
COMPILER_SPECIFIC_MAPFILE_TO_CSV =

MAPFILE_PARSER            :=$(TOOLS_ROOT)/mapfile_parser/map_parse_iar.pl

# $(1) is map file, $(2) is CSV output file
COMPILER_SPECIFIC_MAPFILE_DISPLAY_SUMMARY = $(PERL) $(MAPFILE_PARSER) $(1)

# Add to build_done $(if $(TOOLCHAIN_NAME),IAR,$(IAR_PROJECT_FILE))
$(IAR_PROJECT_FILE):
	$(QUIET)$(ECHO) Generating IAR project file
	$(QUIET)$(IAR_PROJECT_GENERATOR) $(IAR_PROJECT_DEFAULT_TEMPLATE) $(SOURCE_ROOT)


# Chip specific flags for IAR
# Suppress unaligned structure warning for processors that allow unaligned access
ifeq ($(HOST_ARCH),ARM_CM4)
CPU_CFLAGS   := --cpu_mode thumb --cpu Cortex-M4  --diag_suppress Pa039
CPU_ASMFLAGS := --cpu_mode thumb --cpu Cortex-M4
endif

ifeq ($(HOST_ARCH),ARM_CM3)
CPU_CFLAGS   := --cpu_mode thumb --cpu Cortex-M3  --diag_suppress Pa039
CPU_ASMFLAGS := --cpu_mode thumb --cpu Cortex-M3
endif

ENDIAN_CFLAGS_LITTLE   := --endian little
ENDIAN_ASMFLAGS_LITTLE := --endian little

endif



OBJDUMP := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)objdump$(EXECUTABLE_SUFFIX)"
OBJCOPY := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)objcopy$(EXECUTABLE_SUFFIX)"
STRIP   := "$(TOOLCHAIN_PATH)$(TOOLCHAIN_PREFIX)strip$(EXECUTABLE_SUFFIX)"

LINK_OUTPUT_SUFFIX :=.elf
FINAL_OUTPUT_SUFFIX :=.bin




endif #ifneq ($(filter $(HOST_ARCH), ARM_CM3 ARM_CM4),)
