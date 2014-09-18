#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME = ASF

ASF_VERSION := 3.9.1

$(NAME)_CFLAGS := -Wstrict-prototypes  -W -Wshadow  -Wwrite-strings -std=c99 

$(NAME)_NEVER_OPTIMISE := 1

GLOBAL_INCLUDES += . \
                    ../../CMSIS \
                    asf-$(ASF_VERSION)/common/boards \
                    asf-$(ASF_VERSION)/common/services/clock \
                    asf-$(ASF_VERSION)/common/services/ioport \
                    asf-$(ASF_VERSION)/common/utils \
                    asf-$(ASF_VERSION)/sam/drivers/adc \
                    asf-$(ASF_VERSION)/sam/drivers/efc \
                    asf-$(ASF_VERSION)/sam/drivers/hsmci \
                    asf-$(ASF_VERSION)/sam/drivers/matrix \
                    asf-$(ASF_VERSION)/sam/drivers/pdc \
                    asf-$(ASF_VERSION)/sam/drivers/pio \
                    asf-$(ASF_VERSION)/sam/drivers/pmc \
                    asf-$(ASF_VERSION)/sam/drivers/pwm \
                    asf-$(ASF_VERSION)/sam/drivers/rtc \
                    asf-$(ASF_VERSION)/sam/drivers/rtt \
                    asf-$(ASF_VERSION)/sam/drivers/spi \
                    asf-$(ASF_VERSION)/sam/drivers/twi \
                    asf-$(ASF_VERSION)/sam/drivers/supc \
                    asf-$(ASF_VERSION)/sam/drivers/tc \
                    asf-$(ASF_VERSION)/sam/drivers/uart \
                    asf-$(ASF_VERSION)/sam/drivers/usart \
                    asf-$(ASF_VERSION)/sam/drivers/wdt \
                    asf-$(ASF_VERSION)/sam/utils \
                    asf-$(ASF_VERSION)/sam/utils/cmsis/sam4s/include \
                    asf-$(ASF_VERSION)/sam/utils/cmsis/sam4s/source/templates \
                    asf-$(ASF_VERSION)/sam/utils/header_files \
                    asf-$(ASF_VERSION)/sam/utils/preprocessor \
                    asf-$(ASF_VERSION)/thirdparty/CMSIS/Include \
                    asf-$(ASF_VERSION)/sam/utils/cmsis

$(NAME)_SOURCES :=  asf-$(ASF_VERSION)/common/services/clock/sam4s/sysclk.c \
                    asf-$(ASF_VERSION)/common/utils/interrupt/interrupt_sam_nvic.c \
                    asf-$(ASF_VERSION)/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c \
                    asf-$(ASF_VERSION)/sam/drivers/adc/adc.c \
                    asf-$(ASF_VERSION)/sam/drivers/efc/efc.c \
                    asf-$(ASF_VERSION)/sam/drivers/matrix/matrix.c \
                    asf-$(ASF_VERSION)/sam/drivers/pdc/pdc.c \
                    asf-$(ASF_VERSION)/sam/drivers/pio/pio.c \
                    asf-$(ASF_VERSION)/sam/drivers/pio/pio_handler.c \
                    asf-$(ASF_VERSION)/sam/drivers/pmc/pmc.c \
                    asf-$(ASF_VERSION)/sam/drivers/pwm/pwm.c \
                    asf-$(ASF_VERSION)/sam/drivers/pmc/sleep.c \
                    asf-$(ASF_VERSION)/sam/drivers/spi/spi.c \
                    asf-$(ASF_VERSION)/sam/drivers/supc/supc.c \
                    asf-$(ASF_VERSION)/sam/drivers/rtc/rtc.c \
                    asf-$(ASF_VERSION)/sam/drivers/rtt/rtt.c \
                    asf-$(ASF_VERSION)/sam/drivers/tc/tc.c \
                    asf-$(ASF_VERSION)/sam/drivers/twi/twi.c \
                    asf-$(ASF_VERSION)/sam/drivers/uart/uart.c \
                    asf-$(ASF_VERSION)/sam/drivers/usart/usart.c \
                    asf-$(ASF_VERSION)/sam/drivers/wdt/wdt.c
