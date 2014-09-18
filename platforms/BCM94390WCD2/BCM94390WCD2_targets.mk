#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

.PHONY: bootloader download_bootloader no_dct download_dct download

BOOTLOADER_TARGET := waf.bootloader-NoOS-NoNS-$(PLATFORM_FULL)-$(BUS)
BOOTLOADER_LINK_FILE := $(BUILD_DIR)/$(call CONV_COMP,$(subst .,/,$(BOOTLOADER_TARGET)))/binary/$(call CONV_COMP,$(subst .,/,$(BOOTLOADER_TARGET)))$(LINK_OUTPUT_SUFFIX)
BOOTLOADER_OUT_FILE  := $(BOOTLOADER_LINK_FILE:$(LINK_OUTPUT_SUFFIX)=$(FINAL_OUTPUT_SUFFIX))
BOOTLOADER_EPM_FILE  := $(BOOTLOADER_LINK_FILE:$(LINK_OUTPUT_SUFFIX)=.epm.$(FINAL_OUTPUT_SUFFIX))
BOOTLOADER_LOG_FILE  ?= $(BUILD_DIR)/bootloader.log
SFLASH_LOG_FILE      ?= $(BUILD_DIR)/sflash_writer.log
GENERATED_MAC_FILE   := $(SOURCE_ROOT)generated_mac_address.txt
EPM_CREATOR          := $(SOURCE_ROOT)/WICED/platform/MCU/BCM439x/make_epm.pl
MAC_GENERATOR        := $(TOOLS_ROOT)/mac_generator/mac_generator.pl

SFLASH_APP_TARGET := waf.sflash_write-NoOS-NoNS-$(PLATFORM_FULL)-$(BUS)
SFLASH_APP_OUTFILE := $(BUILD_DIR)/$(call CONV_COMP,$(subst .,/,$(SFLASH_APP_TARGET)))/binary/$(call CONV_COMP,$(subst .,/,$(SFLASH_APP_TARGET)))

SFLASH_DCT_LOC:= 0x00000000

# this must match EPM_NVRAM_LOCATION_OFFSET
SFLASH_BOOTLOADER_LOC := 0x00010000

SFLASH_FS_LOC := 0x00020000


OPENOCD_LOG_FILE ?= build/openocd_log.txt
DOWNLOAD_LOG := >> $(OPENOCD_LOG_FILE)

ifneq ($(VERBOSE),1)
BOOTLOADER_REDIRECT	= > $(BOOTLOADER_LOG_FILE)
SFLASH_REDIRECT	= > $(SFLASH_LOG_FILE)
endif


ifeq (,$(and $(OPENOCD_PATH),$(OPENOCD_FULL_NAME)))
	$(error Path to OpenOCD has not been set using OPENOCD_PATH and OPENOCD_FULL_NAME)
endif


# If Downloading is required, then the Serial Flash app need to be built
ifeq (download,$(findstring download,$(MAKECMDGOALS)))
sflash_app:
	$(QUIET)$(ECHO) Building Serial Flash Loader App
	$(QUIET)$(MAKE) -r -f $(SOURCE_ROOT)Makefile $(SFLASH_APP_TARGET) -I$(OUTPUT_DIR)  SFLASH= EXTERNAL_WICED_GLOBAL_DEFINES=$(EXTERNAL_WICED_GLOBAL_DEFINES) SUB_BUILD=sflash_app $(SFLASH_REDIRECT)
	$(QUIET)$(ECHO) Finished Building Serial Flash Loader App
	$(QUIET)$(ECHO_BLANK_LINE)

SFLASH_APP_DEPENDENCY := sflash_app
else
SFLASH_APP_DEPENDENCY :=
endif


# If the current build string is building the bootloader, don't recurse to build another bootloader
ifneq (,$(findstring waf/bootloader, $(BUILD_STRING)))
NO_BOOTLOADER_REQUIRED:=1
endif

# Bootloader is not needed when debugger downloads to RAM
ifneq (download,$(findstring download,$(MAKECMDGOALS)))
NO_BOOTLOADER_REQUIRED:=1
endif

# Bootloader Targets
ifeq (1,$(NO_BOOTLOADER_REQUIRED))
bootloader:
	@:

download_bootloader:
	@:

copy_bootloader_output_for_eclipse:
	@:

else
ifeq (1,$(NO_BUILD_BOOTLOADER))
bootloader:
	$(QUIET)$(ECHO) Skipping building bootloader due to commandline spec

download_bootloader:
	@:

copy_bootloader_output_for_eclipse:
	@:

else
bootloader:
	$(QUIET)$(ECHO) Building Bootloader
	$(QUIET)$(MAKE) -r -f $(SOURCE_ROOT)Makefile $(BOOTLOADER_TARGET) -I$(OUTPUT_DIR)  SFLASH= EXTERNAL_WICED_GLOBAL_DEFINES=$(EXTERNAL_WICED_GLOBAL_DEFINES) SUB_BUILD=bootloader $(BOOTLOADER_REDIRECT)
	$(QUIET)$(PERL) $(EPM_CREATOR) $(BOOTLOADER_LINK_FILE) $(BOOTLOADER_EPM_FILE)
	$(QUIET)$(ECHO) Finished Building Bootloader
	$(QUIET)$(ECHO_BLANK_LINE)

download_bootloader: bootloader display_map_summary download_filesystem download_dct
	$(QUIET)$(ECHO) Downloading Bootloader ...
	$(QUIET)$(call CONV_SLASHES,$(OPENOCD_FULL_NAME)) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f apps/waf/sflash_write/sflash_write.tcl -c "sflash_write_file $(BOOTLOADER_EPM_FILE) $(SFLASH_BOOTLOADER_LOC) $(subst /,_,$(PLATFORM_FULL)-$(BUS)) 1 1" -c shutdown $(DOWNLOAD_LOG) 2>&1

copy_bootloader_output_for_eclipse: build_done
	$(QUIET)$(call MKDIR, $(BUILD_DIR)/eclipse_debug/)
	$(QUIET)$(CP) $(BOOTLOADER_LINK_FILE) $(BUILD_DIR)/eclipse_debug/last_bootloader.elf

endif
endif



# DCT Targets
ifneq (no_dct,$(findstring no_dct,$(MAKECMDGOALS)))
download_dct: $(FINAL_DCT_FILE) display_map_summary
	$(QUIET)$(ECHO) Downloading DCT ...
	$(QUIET)$(call CONV_SLASHES,$(OPENOCD_FULL_NAME)) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f apps/waf/sflash_write/sflash_write.tcl -c "sflash_write_file $(FINAL_DCT_FILE) $(SFLASH_DCT_LOC) $(subst /,_,$(PLATFORM_FULL)-$(BUS)) 0 1" -c shutdown $(DOWNLOAD_LOG) 2>&1

else
download_dct:
	@:

no_dct:
	$(QUIET)$(ECHO) DCT unmodified

endif


run: $(SHOULD_I_WAIT_FOR_DOWNLOAD)
	$(QUIET)$(ECHO) Resetting target
	$(QUIET)$(call CONV_SLASHES,$(OPENOCD_FULL_NAME)) -c "log_output $(OPENOCD_LOG_FILE)" -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -c init -c "reset run" -c shutdown $(DOWNLOAD_LOG) 2>&1 && $(ECHO) Target running


copy_output_for_eclipse: build_done copy_bootloader_output_for_eclipse
	$(QUIET)$(call MKDIR, $(BUILD_DIR)/eclipse_debug/)
	$(QUIET)$(CP) $(LINK_OUTPUT_FILE) $(BUILD_DIR)/eclipse_debug/last_built.elf



debug: $(BUILD_STRING) $(SHOULD_I_WAIT_FOR_DOWNLOAD)
	$(QUIET)$(GDB_COMMAND) $(LINK_OUTPUT_FILE) -x .gdbinit_attach


$(GENERATED_MAC_FILE): $(MAC_GENERATOR)
	$(QUIET)$(PERL) $<  > $@


PLATFORM_PRE_BUILD_TARGETS  := $(GENERATED_MAC_FILE) $(SFLASH_APP_DEPENDENCY) bootloader
PLATFORM_POST_BUILD_TARGETS := copy_output_for_eclipse $(FS_IMAGE)

STAGING_DIR := $(OUTPUT_DIR)/resources/Staging/
FS_IMAGE    := $(OUTPUT_DIR)/filesystem.bin

$(FS_IMAGE): $(STRIPPED_LINK_OUTPUT_FILE) display_map_summary $(STAGING_DIR).d
	$(QUIET)$(ECHO) Creating Filesystem
	$(QUIET)$(CP) $(STRIPPED_LINK_OUTPUT_FILE) $(STAGING_DIR)app.elf
	$(QUIET)$(COMMON_TOOLS_PATH)mk_wicedfs32 $(FS_IMAGE) $(STAGING_DIR)



download: $(STRIPPED_LINK_OUTPUT_FILE) display_map_summary download_bootloader $(if $(findstring no_dct,$(MAKECMDGOALS)),,download_dct) download_filesystem

ifneq (no_dct,$(findstring no_dct,$(MAKECMDGOALS)))
FS_DEP := download_dct
else
FS_DEP := download_bootloader
endif

download_filesystem: $(FS_IMAGE) display_map_summary  $(FS_DEP)
	$(QUIET)$(ECHO) Downloading filesystem ...
	$(QUIET)$(call CONV_SLASHES,$(OPENOCD_FULL_NAME)) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f apps/waf/sflash_write/sflash_write.tcl -c "sflash_write_file $(FS_IMAGE) $(SFLASH_FS_LOC) $(subst /,_,$(PLATFORM_FULL)-$(BUS)) 0 1" -c shutdown $(DOWNLOAD_LOG) 2>&1




#download_sflash:
#	$(QUIET)$(call CONV_SLASHES,$(OPENOCD_FULL_NAME)) -f $(OPENOCD_PATH)$(JTAG).cfg -f $(OPENOCD_PATH)$(HOST_OPENOCD).cfg -f apps/waf/sflash_write/sflash_write.tcl -c "$(SFLASH_FS_IMAGE).bin $(SFLASH_FS_LOC) BCM4390 1 1" -c shutdown

