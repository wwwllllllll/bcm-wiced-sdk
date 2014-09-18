#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

default: Help

export SOURCE_ROOT:=$(dir $(word $(words $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))
export MAKEFILES_PATH := $(SOURCE_ROOT)tools/makefiles

MAKEFILE_TARGETS := clean test testlist iar_project

include $(MAKEFILES_PATH)/wiced_toolchain_common.mk

define USAGE_TEXT
Aborting due to invalid targets

Usage: make <target> [download] [run | debug] [JTAG=xxx] [no_dct]
       make run

  <target>
    One each of the following mandatory [and optional] components separated by '-'
      * Application (apps in sub-directories are referenced by subdir.appname)
      * Hardware Platform ($(filter-out common  include README.txt,$(notdir $(wildcard WICED/platforms/*))))
      * [RTOS] ($(notdir $(wildcard WICED/RTOS/*)))
      * [Network Stack] ($(notdir $(wildcard WICED/network/*)))
      * [MCU-WLAN Interface Bus] (SDIO SPI)

  [download]
    Download firmware image to target platform

  [run]
    Reset and run an application on the target hardware (no download)

  [debug]
    Connect to the target platform and run the debugger

  [JTAG=xxx]
    JTAG interface configuration file from the tools/OpenOCD dirctory
    Default option is BCM9WCD1EVAL1, direct JTAG option is jlink

  [no_dct]
    DCT downloading is disabled and the DCT remains unmodified.
    Only valid when the 'download' option is present

  [factory_reset_dct]
    Generates a factory reset DCT

  [VERBOSE=1]
    Shows the commands as they are being executed

  [JOBS=x]
    Sets the maximum number of parallel build threads (default=4)

  Notes
    * Component names are case sensitive
    * 'WICED', 'SDIO', 'SPI' and 'debug' are reserved component names
    * Component names MUST NOT include space or '-' characters
    * Building for release is assumed unless '-debug' is appended to the target
    * Some platforms may only support a single interface bus option

  Example Usage
    Build for Release
      $> make snip.scan-BCM943362WCD4
      $> make snip.scan-BCM943362WCD4-ThreadX-NetX_Duo-SDIO

    Build, Download and Run using the default USB-JTAG programming interface
      $> make snip.scan-BCM943362WCD4 download run

    Build for Debug
      $> make snip.scan-BCM943362WCD4-debug

    Build, Download and Debug using command line GDB
      $> make snip.scan-BCM943362WCD4-debug download debug

    Reset and run an application on the target hardware
      $> make run

    Clean output directory
      $> make clean

    Build a Factory Reset image for the SPI serial flash
      $> make snip.ping-BCM943362WCD4  OTA=waf.ota_upgrade  SFLASH=app-dct-ota-download

         where: [OTA]=<OTA application build string>
                         Build string options are App name, RTOS and TCP stack ONLY
                <SFLASH>=<app>-[dct]-[ota]-[download]
                         where:
                            app      : Add the application to the image
                            dct      : Add the DCT to the image
                            ota      : Add the OTA application to the image
                            download : Download the image after creation

endef

############################
# Extra options:
#                LINT=1 : Sends source files for parsing by LINT
#                CHECK_HEADERS=1 : builds header files to test for their completeness
############################

OPENOCD_LOG_FILE ?= build/openocd_log.txt
DOWNLOAD_LOG := >> $(OPENOCD_LOG_FILE)

BOOTLOADER_LOG_FILE ?= build/bootloader.log
export HOST_OS
export VERBOSE
export SUB_BUILD
export OPENOCD_LOG_FILE
export EXTERNAL_WICED_GLOBAL_DEFINES

.PHONY: $(BUILD_STRING) main_app bootloader download_only test testlist clean iar_project Help download no_dct download_dct download_work copy_elf_for_eclipse run debug download_bootloader sflash_image .gdbinit factory_reset_dct

Help: $(TOOLCHAIN_HOOK_TARGETS)
	$(error $(USAGE_TEXT))

IAR_PROJECT_BUILDER = $(TOOLS_ROOT)/IAR/wiced_sdk_project/build_iar_project.exe

iar_project:
	$(QUIET)$(ECHO) Generating Wiced_SDK IAR project file...
	$(QUIET) $(RM) $(TOOLS_ROOT)/IAR/wiced_sdk_project/wiced_sdk_project.ewp
	$(QUIET) cd $(TOOLS_ROOT)/IAR/wiced_sdk_project
	$(QUIET) $(IAR_PROJECT_BUILDER) $(subst /,\,$(CURDIR))
	$(QUIET)$(ECHO) Generated. Now opening.... This may take several seconds
	$(QUIET)cmd /C start "$(IAR_WORKBENCH_EXECUTABLE)" "$(TOOLS_ROOT)/IAR/wiced_sdk_project/wiced_sdk_workspace.eww"



clean:
	$(QUIET)$(ECHO) Cleaning...
	$(QUIET)$(CLEAN_COMMAND)
	$(QUIET)$(RM) -rf .gdbinit
	$(QUIET)$(ECHO) Done


test:
	$(QUIET)$(MAKE) $(SILENT) -f $(MAKEFILES_PATH)/wiced_test.mk test

testlist:
	$(QUIET)$(MAKE) $(SILENT) -f $(MAKEFILES_PATH)/wiced_test.mk testlist



# Processing of factory_reset_dct
ifneq (,$(findstring factory_reset_dct,$(MAKECMDGOALS)))
$(filter-out factory_reset_dct, $(MAKECMDGOALS)):
	@:
FACTORY_RESET_TARGET := $(BUILD_STRING)
BUILD_STRING :=
endif

ifneq ($(BUILD_STRING),)
-include build/$(CLEANED_BUILD_STRING)/config.mk
# Now we know the target architecture - include all toolchain makefiles and check one of them can handle the architecture
include $(MAKEFILES_PATH)/wiced_toolchain_ARM_GNU.mk

build/$(CLEANED_BUILD_STRING)/config.mk: $(SOURCE_ROOT)Makefile $(MAKEFILES_PATH)/wiced_config.mk $(MAKEFILES_PATH)/wiced_toolchain_common.mk $(MAKEFILES_PATH)/wiced_toolchain_ARM_GNU.mk $(WICED_SDK_MAKEFILES)
	$(QUIET)$(ECHO) $(if $(WICED_SDK_MAKEFILES),Applying changes made to: $?,Making config file for first time)
	$(QUIET)$(MAKE) -r $(SILENT) -f $(MAKEFILES_PATH)/wiced_config.mk $(DIR_BUILD_STRING)
endif

ifeq ($(IAR),1)
#IAR cannot support multi threaded build
JOBS=1
endif

JOBS ?=4
ifeq (,$(SUB_BUILD))
JOBSNO := $(if $(findstring 1,$(LINT)), , -j$(JOBS) )
endif

PASSDOWN_TARGETS := $(strip $(filter-out $(MAKEFILE_TARGETS) $(BUILD_STRING),$(MAKECMDGOALS)))

$(BUILD_STRING): main_app $(if $(SFLASH),sflash_image) copy_elf_for_eclipse

$(PASSDOWN_TARGETS):
	@:

main_app: build/$(CLEANED_BUILD_STRING)/config.mk $(WICED_SDK_PRE_APP_BUILDS) $(MAKEFILES_PATH)/wiced_elf.mk $(if $(SUB_BUILD),,.gdbinit)
	$(QUIET)$(COMMON_TOOLS_PATH)mkdir -p $(OUTPUT_DIR)/binary $(OUTPUT_DIR)/modules $(OUTPUT_DIR)/libraries $(OUTPUT_DIR)/resources
	$(QUIET)$(MAKE) -r $(JOBSNO) $(SILENT) -f $(MAKEFILES_PATH)/wiced_elf.mk $(DIR_BUILD_STRING) $(PASSDOWN_TARGETS)
	$(QUIET)$(ECHO) Build complete

.gdbinit: build/$(CLEANED_BUILD_STRING)/config.mk $(MAKEFILES_PATH)/wiced_toolchain_common.mk
	$(QUIET)$(ECHO) Making .gdbinit
	$(QUIET)$(ECHO) set remotetimeout 20 > .gdbinit
	$(QUIET)$(ECHO) $(GDBINIT_STRING) >> .gdbinit


ifneq ($(SFLASH),)
sflash_image: main_app
	$(QUIET)$(ECHO) Building Serial Flash Image
	$(QUIET)$(MAKE) $(SILENT) -f $(MAKEFILES_PATH)/mfg_image.mk $(SFLASH) FRAPP=$(CLEANED_BUILD_STRING) SFLASH=
endif

factory_reset_dct: $(SOURCE_ROOT)wiced_factory_reset.mk Makefile
	$(QUIET)$(MAKE) $(SILENT) -f $(SOURCE_ROOT)wiced_factory_reset.mk $(FACTORY_RESET_TARGET)
