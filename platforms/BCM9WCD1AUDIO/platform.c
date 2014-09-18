/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 * Defines board support package for BCM9WCD1AUDIO board
 */
#include "platform.h"
#include "platform_config.h"
#include "platform_init.h"
#include "platform_isr.h"
#include "platform_peripheral.h"
#include "wwd_platform_common.h"
#include "wwd_rtos.h"
#include "wiced_defaults.h"
#include "wiced_platform.h"
#include "wiced_bt_platform.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/* GPIO pin table. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_gpio_t platform_gpio_pins[] =
{
    [WICED_GPIO_1]  = { GPIOA,  0 },
    [WICED_GPIO_2]  = { GPIOA,  1 },
    [WICED_GPIO_3]  = { GPIOA,  2 },
    [WICED_GPIO_4]  = { GPIOA,  3 },
    [WICED_GPIO_5]  = { GPIOA,  4 },
    [WICED_GPIO_6]  = { GPIOA,  5 },
    [WICED_GPIO_7]  = { GPIOA,  6 },
    [WICED_GPIO_8]  = { GPIOA,  7 },
    [WICED_GPIO_9]  = { GPIOA,  9 },
    [WICED_GPIO_10] = { GPIOA, 10 },
    [WICED_GPIO_11] = { GPIOB,  6 },
    [WICED_GPIO_12] = { GPIOB,  7 },
    [WICED_GPIO_13] = { GPIOA, 13 },
    [WICED_GPIO_14] = { GPIOA, 14 },
    [WICED_GPIO_15] = { GPIOA, 15 },
    [WICED_GPIO_16] = { GPIOB,  3 },
    [WICED_GPIO_17] = { GPIOB,  4 },
    [WICED_GPIO_18] = { GPIOC,  6 },
    [WICED_GPIO_19] = { GPIOC, 10 },
    [WICED_GPIO_20] = { GPIOD,  7 },
    [WICED_GPIO_21] = { GPIOD,  8 },
    [WICED_GPIO_22] = { GPIOD,  9 },
    [WICED_GPIO_23] = { GPIOB, 12 },
    [WICED_GPIO_24] = { GPIOB, 10 },
    [WICED_GPIO_25] = { GPIOB, 15 },
    [WICED_GPIO_26] = { GPIOB,  9 },
    [WICED_GPIO_27] = { GPIOG,  6 },
    [WICED_GPIO_28] = { GPIOA, 11 },
    [WICED_GPIO_29] = { GPIOA, 12 },
    [WICED_GPIO_30] = { GPIOG, 10 },
    [WICED_GPIO_31] = { GPIOI,  9 },
    [WICED_GPIO_32] = { GPIOH, 13 },
    [WICED_GPIO_33] = { GPIOH, 15 },
    [WICED_GPIO_34] = { GPIOC,  2 }, /* BT_RESET_N  */
    [WICED_GPIO_35] = { GPIOC,  1 }, /* BT_REG_EN   */
    [WICED_GPIO_36] = { GPIOG, 14 }, /* BT_UART_RX  */
    [WICED_GPIO_37] = { GPIOC,  7 }, /* BT_UART_TX  */
    [WICED_GPIO_38] = { GPIOG, 15 }, /* BT_UART_RTS */
    [WICED_GPIO_39] = { GPIOG,  8 }, /* BT_UART_CTS */
    [WICED_GPIO_40] = { GPIOH, 11 },
};

/* ADC peripherals. Used WICED/platform/MCU/wiced_platform_common.c */
const platform_adc_t platform_adc_peripherals[] =
{
    [WICED_ADC_1] = {ADC1, ADC_Channel_1, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_2]},
    [WICED_ADC_2] = {ADC1, ADC_Channel_2, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_3]},
    [WICED_ADC_3] = {ADC1, ADC_Channel_3, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_4]},
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_pwm_t platform_pwm_peripherals[] =
{
    [WICED_PWM_1]  = {TIM4, 1, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, &platform_gpio_pins[WICED_GPIO_11]},
    [WICED_PWM_2]  = {TIM4, 2, RCC_APB1Periph_TIM4, GPIO_AF_TIM4, &platform_gpio_pins[WICED_GPIO_12]},
    [WICED_PWM_3]  = {TIM2, 2, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_2] }, /* or TIM5/Ch2                       */
    [WICED_PWM_4]  = {TIM2, 3, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_3] }, /* or TIM5/Ch3, TIM9/Ch1             */
    [WICED_PWM_5]  = {TIM2, 4, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_4] }, /* or TIM5/Ch4, TIM9/Ch2             */
    [WICED_PWM_6]  = {TIM2, 1, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_6] }, /* or TIM2_CH1_ETR, TIM8/Ch1N        */
    [WICED_PWM_7]  = {TIM3, 1, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_7] }, /* or TIM1_BKIN, TIM8_BKIN, TIM13/Ch1*/
    [WICED_PWM_8]  = {TIM3, 2, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_8] }, /* or TIM8/Ch1N, TIM14/Ch1           */
    [WICED_PWM_9]  = {TIM5, 2, RCC_APB1Periph_TIM5, GPIO_AF_TIM5, &platform_gpio_pins[WICED_GPIO_2] }, /* or TIM2/Ch2                       */
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_spi_t platform_spi_peripherals[] =
{
    [WICED_SPI_1]  =
    {
        .port                  = SPI1,
        .gpio_af               = GPIO_AF_SPI1,
        .peripheral_clock_reg  = RCC_APB2Periph_SPI1,
        .peripheral_clock_func = RCC_APB2PeriphClockCmd,
        .pin_mosi              = &platform_gpio_pins[WICED_GPIO_8],
        .pin_miso              = &platform_gpio_pins[WICED_GPIO_7],
        .pin_clock             = &platform_gpio_pins[WICED_GPIO_6],
        .tx_dma =
        {
            .controller        = DMA2,
            .stream            = DMA2_Stream5,
            .channel           = DMA_Channel_3,
            .irq_vector        = DMA2_Stream5_IRQn,
            .complete_flags    = DMA_HISR_TCIF5,
            .error_flags       = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
        },
        .rx_dma =
        {
            .controller        = DMA2,
            .stream            = DMA2_Stream0,
            .channel           = DMA_Channel_3,
            .irq_vector        = DMA2_Stream0_IRQn,
            .complete_flags    = DMA_LISR_TCIF0,
            .error_flags       = ( DMA_LISR_TEIF0 | DMA_LISR_FEIF0 | DMA_LISR_DMEIF0 ),
        },
    }
};

/* UART peripherals and runtime drivers. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_uart_t platform_uart_peripherals[] =
{
    [WICED_UART_1] =
    {
        .port               = USART1,
        .tx_pin             = &platform_gpio_pins[WICED_GPIO_9],
        .rx_pin             = &platform_gpio_pins[WICED_GPIO_10],
        .cts_pin            = &platform_gpio_pins[WICED_GPIO_28],
        .rts_pin            = &platform_gpio_pins[WICED_GPIO_29],
        .tx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream7,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA2_Stream7_IRQn,
            .complete_flags = DMA_HISR_TCIF7,
            .error_flags    = ( DMA_HISR_TEIF7 | DMA_HISR_FEIF7 | DMA_HISR_DMEIF7 ),
        },
        .rx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream2,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA2_Stream2_IRQn,
            .complete_flags = DMA_LISR_TCIF2,
            .error_flags    = ( DMA_LISR_TEIF2 | DMA_LISR_FEIF2 | DMA_LISR_DMEIF2 ),
        },
    },
    [WICED_UART_2] =
    {
        .port               = USART6,
        .tx_pin             = &platform_gpio_pins[WICED_GPIO_36],
        .rx_pin             = &platform_gpio_pins[WICED_GPIO_37],
        .cts_pin            = &platform_gpio_pins[WICED_GPIO_38],
        .rts_pin            = &platform_gpio_pins[WICED_GPIO_39],
        .tx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream6,
            .channel        = DMA_Channel_5,
            .irq_vector     = DMA2_Stream6_IRQn,
            .complete_flags = DMA_HISR_TCIF6,
            .error_flags    = ( DMA_HISR_TEIF6 | DMA_HISR_FEIF6 | DMA_HISR_DMEIF6 ),
        },
        .rx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream1,
            .channel        = DMA_Channel_5,
            .irq_vector     = DMA2_Stream1_IRQn,
            .complete_flags = DMA_LISR_TCIF1,
            .error_flags    = ( DMA_LISR_TEIF1 | DMA_LISR_FEIF1 | DMA_LISR_DMEIF1 ),
        },
    },
};
platform_uart_driver_t platform_uart_drivers[WICED_UART_MAX];

/* I2C peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_i2c_t platform_i2c_peripherals[] =
{
    [WICED_I2C_1] =
    {
        .port                    = I2C1,
        .pin_scl                 = &platform_gpio_pins[WICED_GPIO_11],
        .pin_sda                 = &platform_gpio_pins[WICED_GPIO_26],
        .peripheral_clock_reg    = RCC_APB1Periph_I2C1,
        .tx_dma                  = DMA1,
        .tx_dma_peripheral_clock = RCC_AHB1Periph_DMA1,
        .tx_dma_stream           = DMA1_Stream7,
        .rx_dma_stream           = DMA1_Stream5,
        .tx_dma_stream_id        = 7,
        .rx_dma_stream_id        = 5,
        .tx_dma_channel          = DMA_Channel_1,
        .rx_dma_channel          = DMA_Channel_1,
        .gpio_af                 = GPIO_AF_I2C1
    },
};

/* I2S peripherals */
const platform_i2s_t i2s_interfaces[] =
{
    [WICED_I2S_1] =
    {
        .spi                          = SPI2,
#ifdef DAC_EXTERNAL_OSCILLATOR
        .is_master                    = 0,
        .enable_mclk                  = 0,
#else /* DAC_EXTERNAL_OSCILLATOR */
        .is_master                    = 1,
        .enable_mclk                  = 1,
#endif /* DAC_EXTERNAL_OSCILLATOR */
        .gpio_af                      = GPIO_AF_SPI2,
        .peripheral_clock             = RCC_APB1Periph_SPI2,
        .peripheral_clock_func        = RCC_APB1PeriphClockCmd,
        .pin_ck                       = &platform_gpio_pins[WICED_GPIO_24],
        .pin_sd                       = &platform_gpio_pins[WICED_GPIO_25],
        .pin_ws                       = &platform_gpio_pins[WICED_GPIO_23],
        .pin_mclk                     = &platform_gpio_pins[WICED_GPIO_18],
        .tx_dma.dma_register          = DMA1,
        .tx_dma.stream                = DMA1_Stream4,
        .tx_dma.channel               = DMA_Channel_0,
        .tx_dma.peripheral_clock      = RCC_AHB1Periph_DMA1,
        .tx_dma.peripheral_clock_func = RCC_AHB1PeriphClockCmd,
        .tx_dma.irq                   = DMA1_Stream4_IRQn,
        .rx_dma.dma_register          = DMA1,
        .rx_dma.stream                = DMA1_Stream3,
        .rx_dma.channel               = DMA_Channel_0,
        .rx_dma.peripheral_clock      = RCC_AHB1Periph_DMA1,
        .rx_dma.peripheral_clock_func = RCC_AHB1PeriphClockCmd,
        .rx_dma.irq                   = DMA1_Stream3_IRQn,
    }
};

/* SPI flash. Exposed to the applications through include/wiced_platform.h */
#if defined ( WICED_PLATFORM_INCLUDES_SPI_FLASH )
const wiced_spi_device_t wiced_spi_flash =
{
    .port        = WICED_SPI_1,
    .chip_select = WICED_SPI_FLASH_CS,
    .speed       = 1000000,
    .mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST),
    .bits        = 8
};
#endif

/* iAuth chip */
wiced_i2c_device_t auth_chip =
{
    .port          = WICED_I2C_1,
    .address       = 0x10,
    .address_width = I2C_ADDRESS_WIDTH_7BIT,
    .speed_mode    = I2C_STANDARD_SPEED_MODE,
};
wiced_i2c_device_t* auth_chip_token     = &auth_chip;
const wiced_gpio_t  auth_chip_reset_pin = WICED_GPIO_AUTH_RST;

/* UART standard I/O configuration */
#ifndef WICED_DISABLE_STDIO
static platform_uart_config_t stdio_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};
#endif

/* Wi-Fi control pins. Used by WICED/platform/MCU/wwd_platform_common.c */
const platform_gpio_t wifi_control_pins[] =
{
    [WWD_PIN_POWER]       = { GPIOF, 11 },
    [WWD_PIN_RESET]       = { GPIOB,  5 },
    [WWD_PIN_32K_CLK]     = { GPIOA,  8 },
    [WWD_PIN_BOOTSTRAP_0] = { GPIOB,  0 },
    [WWD_PIN_BOOTSTRAP_1] = { GPIOB,  1 },
};

/* Wi-Fi SDIO bus pins. Used by WICED/platform/STM32F2xx/WWD/wwd_SDIO.c */
const platform_gpio_t wifi_sdio_pins[] =
{
    [WWD_PIN_SDIO_OOB_IRQ] = { GPIOB,  0 },
    [WWD_PIN_SDIO_CLK]     = { GPIOC, 12 },
    [WWD_PIN_SDIO_CMD]     = { GPIOD,  2 },
    [WWD_PIN_SDIO_D0]      = { GPIOC,  8 },
    [WWD_PIN_SDIO_D1]      = { GPIOC,  9 },
    [WWD_PIN_SDIO_D2]      = { GPIOC, 10 },
    [WWD_PIN_SDIO_D3]      = { GPIOC, 11 },
};

/* Bluetooth control pins. Used by libraries/bluetooth/internal/bus/UART/bt_bus.c */
const platform_gpio_t* wiced_bt_control_pins[] =
{
    [WICED_BT_PIN_POWER] = &platform_gpio_pins[WICED_GPIO_35],
    [WICED_BT_PIN_RESET] = &platform_gpio_pins[WICED_GPIO_34],
    /* Device and host wake pins unavailable */
};

/* Bluetooth UART pins. Used by libraries/bluetooth/internal/bus/UART/bt_bus.c */
const platform_gpio_t* wiced_bt_uart_pins[] =
{
    [WICED_BT_PIN_UART_TX]  = &platform_gpio_pins[WICED_GPIO_36],
    [WICED_BT_PIN_UART_RX]  = &platform_gpio_pins[WICED_GPIO_37],
    [WICED_BT_PIN_UART_CTS] = &platform_gpio_pins[WICED_GPIO_38],
    [WICED_BT_PIN_UART_RTS] = &platform_gpio_pins[WICED_GPIO_39],
};

/* Bluetooth UART peripheral and runtime driver. Used by libraries/bluetooth/internal/bus/UART/bt_bus.c */
const platform_uart_t*  wiced_bt_uart_peripheral = &platform_uart_peripherals[WICED_UART_2];
platform_uart_driver_t* wiced_bt_uart_driver     = &platform_uart_drivers    [WICED_UART_2];

/* Bluetooth UART configuration. Used by libraries/bluetooth/internal/bus/UART/bt_bus.c */
const platform_uart_config_t wiced_bt_uart_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void platform_init_peripheral_irq_priorities( void )
{
    /* Interrupt priority setup. Called by WICED/platform/MCU/STM32F4xx/platform_init.c */
    NVIC_SetPriority( RTC_WKUP_IRQn    ,  1 ); /* RTC Wake-up event   */
    NVIC_SetPriority( SDIO_IRQn        ,  2 ); /* WLAN SDIO           */
    NVIC_SetPriority( DMA2_Stream3_IRQn,  3 ); /* WLAN SDIO DMA       */
    NVIC_SetPriority( SPI2_IRQn        ,  6 ); /* WICED_I2S_1         */
    NVIC_SetPriority( DMA1_Stream4_IRQn,  6 ); /* WICED_I2S_1 DMA     */
    NVIC_SetPriority( USART1_IRQn      ,  6 ); /* WICED_UART_1        */
    NVIC_SetPriority( USART6_IRQn      ,  6 ); /* WICED_UART_2        */
    NVIC_SetPriority( DMA2_Stream7_IRQn,  7 ); /* WICED_UART_1 TX DMA */
    NVIC_SetPriority( DMA2_Stream2_IRQn,  7 ); /* WICED_UART_1 RX DMA */
    NVIC_SetPriority( DMA2_Stream6_IRQn,  7 ); /* WICED_UART_2 TX DMA */
    NVIC_SetPriority( DMA2_Stream1_IRQn,  7 ); /* WICED_UART_2 RX DMA */
    NVIC_SetPriority( EXTI0_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI1_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI2_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI3_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI4_IRQn       , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI9_5_IRQn     , 14 ); /* GPIO                */
    NVIC_SetPriority( EXTI15_10_IRQn   , 14 ); /* GPIO                */
}

void platform_init_external_devices( void )
{
    /* Initialise LEDs and turn off by default */
    platform_gpio_init( &platform_gpio_pins[WICED_LED1], OUTPUT_PUSH_PULL );
    platform_gpio_init( &platform_gpio_pins[WICED_LED2], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &platform_gpio_pins[WICED_LED1] );
    platform_gpio_output_low( &platform_gpio_pins[WICED_LED2] );

    /* Initialise buttons to input by default */
    platform_gpio_init( &platform_gpio_pins[WICED_BUTTON1], INPUT_PULL_UP );
    platform_gpio_init( &platform_gpio_pins[WICED_BUTTON2], INPUT_PULL_UP );

#ifndef WICED_DISABLE_STDIO
    /* Initialise UART standard I/O */
    platform_stdio_init( &platform_uart_drivers[STDIO_UART], &platform_uart_peripherals[STDIO_UART], &stdio_config );
#endif
}

/******************************************************
 *           Interrupt Handler Definitions
 ******************************************************/

WWD_RTOS_DEFINE_ISR( usart1_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart6_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( usart1_tx_dma_irq )
{
    platform_uart_tx_dma_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart6_tx_dma_irq )
{
    platform_uart_tx_dma_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( usart1_rx_dma_irq )
{
    platform_uart_rx_dma_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart6_rx_dma_irq )
{
    platform_uart_rx_dma_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( i2s_irq )
{
    platform_i2s_irq( WICED_I2S_1 );
}

WWD_RTOS_DEFINE_ISR( i2s_tx_dma_irq )
{
    platform_i2s_tx_dma_irq( WICED_I2S_1 );
}

/******************************************************
 *            Interrupt Handlers Mapping
 ******************************************************/

/* These DMA assignments can be found STM32F2xx datasheet DMA section */
WWD_RTOS_MAP_ISR( usart1_irq        , USART1_irq       )
WWD_RTOS_MAP_ISR( usart6_irq        , USART6_irq       )
WWD_RTOS_MAP_ISR( usart1_tx_dma_irq , DMA2_Stream7_irq )
WWD_RTOS_MAP_ISR( usart6_tx_dma_irq , DMA2_Stream6_irq )
WWD_RTOS_MAP_ISR( usart1_rx_dma_irq , DMA2_Stream2_irq )
WWD_RTOS_MAP_ISR( usart6_rx_dma_irq , DMA2_Stream1_irq )
WWD_RTOS_MAP_ISR( i2s_irq           , SPI2_irq         )
WWD_RTOS_MAP_ISR( i2s_tx_dma_irq    , DMA1_Stream4_irq )

