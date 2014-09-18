#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME = STM32F2xx_Peripheral_Libraries

GLOBAL_INCLUDES :=  . \
                    /inc \
                    ../../../$(HOST_ARCH)/CMSIS

$(NAME)_SOURCES := \
                   src/misc.c \
                   src/stm32f2xx_adc.c \
                   src/stm32f2xx_can.c \
                   src/stm32f2xx_crc.c \
                   src/stm32f2xx_dac.c \
                   src/stm32f2xx_dbgmcu.c \
                   src/stm32f2xx_dma.c \
                   src/stm32f2xx_exti.c \
                   src/stm32f2xx_flash.c \
                   src/stm32f2xx_fsmc.c \
                   src/stm32f2xx_gpio.c \
                   src/stm32f2xx_rng.c \
                   src/stm32f2xx_i2c.c \
                   src/stm32f2xx_iwdg.c \
                   src/stm32f2xx_pwr.c \
                   src/stm32f2xx_rcc.c \
                   src/stm32f2xx_rtc.c \
                   src/stm32f2xx_sdio.c \
                   src/stm32f2xx_spi.c \
                   src/stm32f2xx_syscfg.c \
                   src/stm32f2xx_tim.c \
                   src/stm32f2xx_usart.c \
                   src/stm32f2xx_wwdg.c
