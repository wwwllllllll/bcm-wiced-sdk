#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME = WICED

CERTIFICATE := $(SOURCE_ROOT)resources/certificates/brcm_demo_server_cert.cer
PRIVATE_KEY := $(SOURCE_ROOT)resources/certificates/brcm_demo_server_cert_key.key

ifndef USES_BOOTLOADER_OTA
USES_BOOTLOADER_OTA :=1
endif

ifeq ($(BUILD_TYPE),debug)
#$(NAME)_COMPONENTS += test/malloc_debug
endif

# Check if the WICED API is being used
ifeq (,$(APP_WWD_ONLY)$(NS_WWD_ONLY)$(RTOS_WWD_ONLY))

$(NAME)_SOURCES += internal/wifi.c \
                   internal/config.c \
                   internal/config_http_content.c \
                   internal/time.c \
                   internal/wiced_lib.c \
                   internal/management.c \
                   internal/system_monitor.c \
                   internal/wiced_cooee.c \
                   internal/wiced_easy_setup.c \
                   internal/waf.c

$(NAME)_RESOURCES := images/brcmlogo.png \
                     images/brcmlogo_line.png \
                     images/favicon.ico \
                     images/scan_icon.png \
                     images/wps_icon.png \
                     images/64_0bars.png \
                     images/64_1bars.png \
                     images/64_2bars.png \
                     images/64_3bars.png \
                     images/64_4bars.png \
                     images/64_5bars.png \
                     images/tick.png \
                     images/cross.png \
                     images/lock.png \
                     images/progress.gif \
                     scripts/general_ajax_script.js \
                     scripts/wpad.dat \
                     config/device_settings.html \
                     config/scan_page_outer.html \
                     config/redirect.html \
                     styles/buttons.css \
                     styles/border_radius.htc

$(NAME)_INCLUDES := security/BESL/TLV \
                    security/BESL/crypto \
                    security/BESL/include



$(NAME)_CHECK_HEADERS := \
                         internal/wiced_internal_api.h \
                         ../include/default_wifi_config_dct.h \
                         ../include/resource.h \
                         ../include/wiced.h \
                         ../include/wiced_defaults.h \
                         ../include/wiced_easy_setup.h \
                         ../include/wiced_framework.h \
                         ../include/wiced_management.h \
                         ../include/wiced_platform.h \
                         ../include/wiced_rtos.h \
                         ../include/wiced_security.h \
                         ../include/wiced_tcpip.h \
                         ../include/wiced_time.h \
                         ../include/wiced_utilities.h \
                         ../include/wiced_wifi.h

ifeq (NetX,$(NETWORK))
$(NAME)_COMPONENTS += WICED/security/BESL
$(NAME)_COMPONENTS += daemons/HTTP_server
$(NAME)_COMPONENTS += daemons/DNS_redirect
$(NAME)_COMPONENTS += protocols/DNS
GLOBAL_DEFINES += ADD_NETX_EAPOL_SUPPORT USE_MICRORNG
endif

ifeq (NetX_Duo,$(NETWORK))
$(NAME)_COMPONENTS += WICED/security/BESL
$(NAME)_COMPONENTS += daemons/HTTP_server
$(NAME)_COMPONENTS += daemons/DNS_redirect
$(NAME)_COMPONENTS += protocols/DNS
GLOBAL_DEFINES += ADD_NETX_EAPOL_SUPPORT USE_MICRORNG
endif

ifeq (LwIP,$(NETWORK))
$(NAME)_COMPONENTS += WICED/security/BESL
$(NAME)_COMPONENTS += daemons/HTTP_server
$(NAME)_COMPONENTS += daemons/DNS_redirect
$(NAME)_COMPONENTS += protocols/DNS
endif

endif

# Add standard WICED 1.x components
$(NAME)_COMPONENTS += WICED/WWD

# Add WICEDFS as standard filesystem
$(NAME)_COMPONENTS += filesystems/wicedfs

# Add MCU component
$(NAME)_COMPONENTS += WICED/platform/MCU/$(HOST_MCU_FAMILY)

# Define the default ThreadX and FreeRTOS starting stack sizes
FreeRTOS_START_STACK := 800
ThreadX_START_STACK  := 800

GLOBAL_DEFINES += WWD_STARTUP_DELAY=10 \
                  BOOTLOADER_MAGIC_NUMBER=0x4d435242
                  APP_NAME=$$(SLASH_QUOTE)$(APP)$$(SLASH_QUOTE)

GLOBAL_INCLUDES := .

$(NAME)_CFLAGS  = $(COMPILER_SPECIFIC_PEDANTIC_CFLAGS)
