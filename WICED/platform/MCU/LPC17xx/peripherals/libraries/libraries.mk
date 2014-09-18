#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME = LPC17xx_Peripheral_Libraries

GLOBAL_INCLUDES :=  . \
                    inc \
                    ../../../$(HOST_ARCH)/CMSIS

$(NAME)_SOURCES := \
                   src/adc_17xx_40xx.c \
                   src/clock_17xx_40xx.c \
                   src/eeprom_17xx_40xx.c \
                   src/gpdma_17xx_40xx.c \
                   src/i2c_17xx_40xx.c \
                   src/i2s_17xx_40xx.c \
                   src/iocon_17xx_40xx.c \
                   src/ritimer_17xx_40xx.c \
                   src/rtc_17xx_40xx.c \
                   src/sdc_17xx_40xx.c \
                   src/sdmmc_17xx_40xx.c \
                   src/spi_17xx_40xx.c \
                   src/ssp_17xx_40xx.c \
                   src/sysctl_17xx_40xx.c \
                   src/timer_17xx_40xx.c \
                   src/uart_17xx_40xx.c \
                   src/wwdt_17xx_40xx.c \
                   src/atimer_001.c \
                   src/eeprom_001.c \
                   src/eeprom_002.c \
                   src/gpdma_001.c \
                   src/gpio_003.c \
                   src/gpiogrpint_001.c \
                   src/gpioint_001.c \
                   src/gpiopinint_001.c \
                   src/i2c_001.c \
                   src/i2s_001.c \
                   src/ritimer_001.c \
                   src/rtc_001.c \
                   src/sct_001.c \
                   src/sdc_001.c \
                   src/sdmmc_001.c \
                   src/spi_002.c \
                   src/ssp_001.c \
                   src/timer_001.c \
                   src/usart_001.c \
                   src/usart_002.c \
                   src/usart_004.c \
                   src/wwdt_001.c
