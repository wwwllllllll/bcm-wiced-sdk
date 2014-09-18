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
 * Defines board support package for BCM9WCDPLUS114 board
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

/* These are internal module connections only */
typedef enum
{
    WICED_GPIO_SFLASH_CS = WICED_GPIO_MAX,
    WICED_GPIO_SFLASH_CLK,
    WICED_GPIO_SFLASH_MISO,
    WICED_GPIO_SFLASH_MOSI,
} wiced_extended_gpio_t;

typedef enum
{
    WICED_SPI_SFLASH = WICED_SPI_MAX,
} wiced_extended_spi_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* GPIO pin table. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_gpio_t platform_gpio_pins[] =
{
    [WICED_GPIO_1]            = { GPIOB,  0 },
    [WICED_GPIO_2]            = { GPIOB,  1 },
    [WICED_GPIO_3]            = { GPIOC,  0 },
    [WICED_GPIO_4]            = { GPIOA,  8 },
    [WICED_GPIO_5]            = { GPIOC,  3 },
    [WICED_GPIO_6]            = { GPIOC,  4 },
    [WICED_GPIO_7]            = { GPIOB,  5 },
    [WICED_GPIO_8]            = { GPIOC,  7 },
    [WICED_GPIO_9]            = { GPIOC, 13 },
    [WICED_GPIO_10]           = { GPIOB,  6 },
    [WICED_GPIO_11]           = { GPIOB,  7 },
    [WICED_GPIO_12]           = { GPIOC,  1 },
    [WICED_GPIO_13]           = { GPIOC,  2 },
    [WICED_GPIO_14]           = { GPIOC,  5 },
    [WICED_GPIO_15]           = { GPIOB,  4 },
    [WICED_GPIO_16]           = { GPIOB,  3 },
    [WICED_GPIO_17]           = { GPIOA, 15 },
    [WICED_GPIO_18]           = { GPIOA, 13 },
    [WICED_GPIO_19]           = { GPIOA, 14 },
    [WICED_GPIO_20]           = { GPIOA, 12 },
    [WICED_GPIO_21]           = { GPIOA, 11 },
    [WICED_GPIO_22]           = { GPIOA, 10 },
    [WICED_GPIO_23]           = { GPIOA,  9 },
    [WICED_GPIO_24]           = { GPIOA,  6 },
    [WICED_GPIO_25]           = { GPIOA,  5 },
    [WICED_GPIO_26]           = { GPIOA,  7 },
    [WICED_GPIO_27]           = { GPIOA,  4 },
    [WICED_GPIO_28]           = { GPIOA,  1 },
    [WICED_GPIO_29]           = { GPIOA,  0 },
    [WICED_GPIO_30]           = { GPIOA,  3 },
    [WICED_GPIO_31]           = { GPIOA,  2 },

    /* Extended GPIO mappings */
    [WICED_GPIO_SFLASH_CS  ]  = { GPIOB, 12 },
    [WICED_GPIO_SFLASH_CLK ]  = { GPIOB, 13 },
    [WICED_GPIO_SFLASH_MISO]  = { GPIOB, 14 },
    [WICED_GPIO_SFLASH_MOSI]  = { GPIOB, 15 }
};

/* ADC peripherals. Used WICED/platform/MCU/wiced_platform_common.c */
const platform_adc_t platform_adc_peripherals[] =
{
    [WICED_ADC_1] = {ADC1, ADC_Channel_1,  RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_28]},
    [WICED_ADC_2] = {ADC1, ADC_Channel_2,  RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_31]},
    [WICED_ADC_3] = {ADC1, ADC_Channel_13, RCC_APB2Periph_ADC1, 1, &platform_gpio_pins[WICED_GPIO_5]},
};

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_pwm_t platform_pwm_peripherals[] =
{
    [WICED_PWM_1]  = {TIM3, 2, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_26]},  /* or TIM8/Ch1N, TIM14/Ch1           */
    [WICED_PWM_2]  = {TIM1, 4, RCC_APB2Periph_TIM1, GPIO_AF_TIM1, &platform_gpio_pins[WICED_GPIO_21]},
    [WICED_PWM_3]  = {TIM2, 2, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_28]},  /* or TIM5/Ch2                       */
    [WICED_PWM_4]  = {TIM2, 3, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_31]},  /* or TIM5/Ch3, TIM9/Ch1             */
    [WICED_PWM_5]  = {TIM2, 4, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_30]},  /* or TIM5/Ch4, TIM9/Ch2             */
    [WICED_PWM_6]  = {TIM2, 1, RCC_APB1Periph_TIM2, GPIO_AF_TIM2, &platform_gpio_pins[WICED_GPIO_25]},  /* or TIM2_CH1_ETR, TIM8/Ch1N        */
    [WICED_PWM_7]  = {TIM3, 1, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_24]},  /* or TIM1_BKIN, TIM8_BKIN, TIM13/Ch1*/
    [WICED_PWM_8]  = {TIM3, 2, RCC_APB1Periph_TIM3, GPIO_AF_TIM3, &platform_gpio_pins[WICED_GPIO_26]},  /* or TIM8/Ch1N, TIM14/Ch1           */
    [WICED_PWM_9]  = {TIM5, 2, RCC_APB1Periph_TIM5, GPIO_AF_TIM5, &platform_gpio_pins[WICED_GPIO_28]},  /* or TIM2/Ch2                       */
};

/* I2C peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_i2c_t platform_i2c_peripherals[] =
{
    [WICED_I2C_1] =
    {
        .port                    = I2C1,
        .pin_scl                 = &platform_gpio_pins[WICED_GPIO_10],
        .pin_sda                 = &platform_gpio_pins[WICED_GPIO_11],
        .peripheral_clock_reg    = RCC_APB1Periph_I2C1,
        .tx_dma                  = DMA1,
        .tx_dma_peripheral_clock = RCC_AHB1Periph_DMA1,
        .tx_dma_stream           = DMA1_Stream7,
        .rx_dma_stream           = DMA1_Stream0,
        .tx_dma_stream_id        = 7,
        .rx_dma_stream_id        = 0,
        .tx_dma_channel          = DMA_Channel_1,
        .rx_dma_channel          = DMA_Channel_1,
        .gpio_af                 = GPIO_AF_I2C1
    },
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
        .pin_mosi              = &platform_gpio_pins[WICED_GPIO_26],
        .pin_miso              = &platform_gpio_pins[WICED_GPIO_24],
        .pin_clock             = &platform_gpio_pins[WICED_GPIO_25],
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
    },
    [WICED_SPI_SFLASH]  =
    {
        .port                  = SPI2,
        .gpio_af               = GPIO_AF_SPI2,
        .peripheral_clock_reg  = RCC_APB1Periph_SPI2,
        .peripheral_clock_func = RCC_APB1PeriphClockCmd,
        .pin_mosi              = &platform_gpio_pins[WICED_GPIO_SFLASH_MOSI],
        .pin_miso              = &platform_gpio_pins[WICED_GPIO_SFLASH_MISO],
        .pin_clock             = &platform_gpio_pins[WICED_GPIO_SFLASH_CLK],
        .tx_dma =
        {
            .controller        = DMA1,
            .stream            = DMA1_Stream4,
            .channel           = DMA_Channel_0,
            .irq_vector        = DMA1_Stream4_IRQn,
            .complete_flags    = DMA_HISR_TCIF4,
            .error_flags       = ( DMA_HISR_TEIF4 | DMA_HISR_FEIF4 | DMA_HISR_DMEIF4 ),
        },
        .rx_dma =
        {
            .controller        = DMA1,
            .stream            = DMA1_Stream3,
            .channel           = DMA_Channel_0,
            .irq_vector        = DMA1_Stream3_IRQn,
            .complete_flags    = DMA_LISR_TCIF3,
            .error_flags       = ( DMA_LISR_TEIF3 | DMA_LISR_FEIF3 | DMA_LISR_DMEIF3 ),
        },
    }
};

/* UART peripherals and runtime drivers. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_uart_t platform_uart_peripherals[] =
{
    [WICED_UART_1] =
    {
        .port               = USART1,
        .tx_pin             = &platform_gpio_pins[WICED_GPIO_23],
        .rx_pin             = &platform_gpio_pins[WICED_GPIO_22],
        .cts_pin            = &platform_gpio_pins[WICED_GPIO_21],
        .rts_pin            = &platform_gpio_pins[WICED_GPIO_20],
        .tx_dma_config =
        {
            .controller     = DMA2,
            .stream         = DMA2_Stream7,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA2_Stream7_IRQn,
            .complete_flags = DMA_HISR_TCIF7,
            .error_flags    = ( DMA_HISR_TEIF7 | DMA_HISR_FEIF7 ),
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
        .port               = USART2,
        .tx_pin             = &platform_gpio_pins[WICED_GPIO_31],
        .rx_pin             = &platform_gpio_pins[WICED_GPIO_30],
        .cts_pin            = &platform_gpio_pins[WICED_GPIO_29],
        .rts_pin            = &platform_gpio_pins[WICED_GPIO_28],
        .tx_dma_config =
        {
            .controller     = DMA1,
            .stream         = DMA1_Stream6,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA1_Stream6_IRQn,
            .complete_flags = DMA_HISR_TCIF6,
            .error_flags    = ( DMA_HISR_TEIF6 | DMA_HISR_FEIF6 ),
        },
        .rx_dma_config =
        {
            .controller     = DMA1,
            .stream         = DMA1_Stream5,
            .channel        = DMA_Channel_4,
            .irq_vector     = DMA1_Stream5_IRQn,
            .complete_flags = DMA_HISR_TCIF5,
            .error_flags    = ( DMA_HISR_TEIF5 | DMA_HISR_FEIF5 | DMA_HISR_DMEIF5 ),
        },
    },
};
platform_uart_driver_t platform_uart_drivers[WICED_UART_MAX];

/* SPI flash. Exposed to the applications through include/wiced_platform.h */
#if defined ( WICED_PLATFORM_INCLUDES_SPI_FLASH )
const wiced_spi_device_t wiced_spi_flash =
{
    .port        = WICED_SPI_SFLASH,
    .chip_select = WICED_GPIO_SFLASH_CS,
    .speed       = 1000000,
    .mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST),
    .bits        = 8
};
#endif

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

/* Wi-Fi control pins. Used by WICED/platform/MCU/wwd_platform_common.c
 * Note: BCM9WCDPLUS114 only support SDIO i.e WWD_PIN_BOOTSTRAP_0 (WL_GPIO0) is tied to ground
 */
const platform_gpio_t wifi_control_pins[] =
{
    [WWD_PIN_POWER]   = { GPIOA, 12 },
    [WWD_PIN_RESET]   = { GPIOB,  8 },
#if defined ( WICED_USE_WIFI_32K_CLOCK_MCO )
    [WWD_PIN_32K_CLK] = { GPIOA,  8 },
#else
    [WWD_PIN_32K_CLK] = { GPIOC,  6 },
#endif
};

/* Wi-Fi SDIO bus pins. Used by WICED/platform/STM32F2xx/WWD/wwd_SDIO.c */
const platform_gpio_t wifi_sdio_pins[] =
{
    [WWD_PIN_SDIO_OOB_IRQ] = { GPIOB,  9 },
    [WWD_PIN_SDIO_CLK]     = { GPIOC, 12 },
    [WWD_PIN_SDIO_CMD]     = { GPIOD,  2 },
    [WWD_PIN_SDIO_D0]      = { GPIOC,  8 },
    [WWD_PIN_SDIO_D1]      = { GPIOC,  9 },
    [WWD_PIN_SDIO_D2]      = { GPIOC, 10 },
    [WWD_PIN_SDIO_D3]      = { GPIOC, 11 },
};

const platform_gpio_t* wiced_bt_control_pins[] =
{
    [WICED_BT_PIN_POWER]    = &platform_gpio_pins[WICED_GPIO_12],
    [WICED_BT_PIN_RESET]    = &platform_gpio_pins[WICED_GPIO_3],
};

const platform_gpio_t* wiced_bt_uart_pins[] =
{
    [WICED_BT_PIN_UART_TX]  = &platform_gpio_pins[WICED_GPIO_31],
    [WICED_BT_PIN_UART_RX]  = &platform_gpio_pins[WICED_GPIO_30],
    [WICED_BT_PIN_UART_CTS] = &platform_gpio_pins[WICED_GPIO_29],
    [WICED_BT_PIN_UART_RTS] = &platform_gpio_pins[WICED_GPIO_28],
};

const platform_uart_t*  wiced_bt_uart_peripheral = &platform_uart_peripherals[WICED_UART_2];
platform_uart_driver_t* wiced_bt_uart_driver     = &platform_uart_drivers[WICED_UART_2];

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
    /* Interrupt priority setup. Called by WICED/platform/MCU/STM32F2xx/platform_init.c */
    NVIC_SetPriority( RTC_WKUP_IRQn    ,  1 );  /* RTC Wake-up event   */
    NVIC_SetPriority( SDIO_IRQn        ,  2 );  /* WLAN SDIO           */
    NVIC_SetPriority( DMA2_Stream3_IRQn,  3 );  /* WLAN SDIO DMA       */
    NVIC_SetPriority( DMA1_Stream3_IRQn,  3 );  /* WLAN SPI DMA        */
    NVIC_SetPriority( USART1_IRQn      ,  6 );  /* WICED_UART_1        */
    NVIC_SetPriority( USART2_IRQn      ,  6 );  /* WICED_UART_2        */
    NVIC_SetPriority( DMA2_Stream7_IRQn,  7 );  /* WICED_UART_1 TX DMA */
    NVIC_SetPriority( DMA2_Stream2_IRQn,  7 );  /* WICED_UART_1 RX DMA */
    NVIC_SetPriority( DMA1_Stream6_IRQn,  7 );  /* WICED_UART_2 TX DMA */
    NVIC_SetPriority( DMA1_Stream5_IRQn,  7 );  /* WICED_UART_2 RX DMA */
    NVIC_SetPriority( EXTI0_IRQn       , 14 );  /* GPIO                */
    NVIC_SetPriority( EXTI1_IRQn       , 14 );  /* GPIO                */
    NVIC_SetPriority( EXTI2_IRQn       , 14 );  /* GPIO                */
    NVIC_SetPriority( EXTI3_IRQn       , 14 );  /* GPIO                */
    NVIC_SetPriority( EXTI4_IRQn       , 14 );  /* GPIO                */
    NVIC_SetPriority( EXTI9_5_IRQn     , 14 );  /* GPIO                */
    NVIC_SetPriority( EXTI15_10_IRQn   , 14 );  /* GPIO                */
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

WWD_RTOS_DEFINE_ISR( usart2_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( usart1_tx_dma_irq )
{
    platform_uart_tx_dma_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart2_tx_dma_irq )
{
    platform_uart_tx_dma_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_DEFINE_ISR( usart1_rx_dma_irq )
{
    platform_uart_rx_dma_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart2_rx_dma_irq )
{
    platform_uart_rx_dma_irq( &platform_uart_drivers[WICED_UART_2] );
}

/******************************************************
 *            Interrupt Handlers Mapping
 ******************************************************/

/* These DMA assignments can be found STM32F2xx datasheet DMA section */
WWD_RTOS_MAP_ISR( usart1_irq       , USART1_irq       )
WWD_RTOS_MAP_ISR( usart1_tx_dma_irq, DMA2_Stream7_irq )
WWD_RTOS_MAP_ISR( usart1_rx_dma_irq, DMA2_Stream2_irq )
WWD_RTOS_MAP_ISR( usart2_irq       , USART2_irq       )
WWD_RTOS_MAP_ISR( usart2_tx_dma_irq, DMA1_Stream6_irq )
WWD_RTOS_MAP_ISR( usart2_rx_dma_irq, DMA1_Stream5_irq )
