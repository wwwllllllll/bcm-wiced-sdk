#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

NAME = STM32F4xx_Peripheral_Libraries

GLOBAL_INCLUDES :=  . \
                    inc \
                    ../../../$(HOST_ARCH)/CMSIS

$(NAME)_SOURCES := \
                   src/misc.c \
                   src/stm32f4xx_adc.c \
                   src/stm32f4xx_can.c \
                   src/stm32f4xx_crc.c \
                   src/stm32f4xx_dac.c \
                   src/stm32f4xx_dbgmcu.c \
                   src/stm32f4xx_dma.c \
                   src/stm32f4xx_exti.c \
                   src/stm32f4xx_flash.c \
                   src/stm32f4xx_fsmc.c \
                   src/stm32f4xx_gpio.c \
                   src/stm32f4xx_rng.c \
                   src/stm32f4xx_i2c.c \
                   src/stm32f4xx_iwdg.c \
                   src/stm32f4xx_pwr.c \
                   src/stm32f4xx_rcc.c \
                   src/stm32f4xx_rtc.c \
                   src/stm32f4xx_sdio.c \
                   src/stm32f4xx_spi.c \
                   src/stm32f4xx_syscfg.c \
                   src/stm32f4xx_tim.c \
                   src/stm32f4xx_usart.c \
                   src/stm32f4xx_wwdg.c
