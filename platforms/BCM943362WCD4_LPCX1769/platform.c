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
 * Defines board support package for BCM943362WCD4_LPCX1769 board
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
 *               Static Function Declarations
 ******************************************************/

/******************************************************
 *               Variable Definitions
 ******************************************************/

/* GPIO pin table. Used by WICED/platform/MCU/wiced_platform_common.c. Incomplete ... */
const platform_gpio_t platform_gpio_pins[ ] =
{
    [WICED_GPIO_1] = { LPC_17xx_P1, 4, },
    [WICED_GPIO_2] = { LPC_17xx_P1, 5, },
    [WICED_GPIO_3] = { LPC_17xx_P1, 6, },
    [WICED_GPIO_4] = { LPC_17xx_P1, 7, },
};

/* UART peripherals and runtime drivers. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_uart_t platform_uart_peripherals[] =
{
    [WICED_UART_1] =
    {
        .uart_base       = LPC_UART0,
        .tx_pin          =
        {
            .config.port = LPC_17xx_P0,     //LPC_17xx_P0,
            .config.pin  = LPC_17xx_PIN_2,  //LPC_17xx_PIN_10,
            .mode        = IOCON_MODE_PULLDOWN,
            .function    = IOCON_FUNC1
        },
        .rx_pin          =
        {
            .config.port = LPC_17xx_P0,     //LPC_17xx_P2,
            .config.pin  = LPC_17xx_PIN_3,  //LPC_17xx_PIN_11,
            .mode        = IOCON_MODE_INACT,
            .function    = IOCON_FUNC1      //(SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | IOCON_FUNC1 | SCU_MODE_INACT)//SCU_MODE_INACT
        },
        .cts_pin         =
        {
            .config.port = 0xFF,
            .config.pin  = 0xFF,
            .mode        = 0xFF,
            .function    = 0xFF
        },//usart has no rts/cts lines
        .rts_pin         =
        {
            .config.port = 0xFF,
            .config.pin  = 0xFF,
            .mode        = 0xFF,
            .function    = 0xFF
        },//usart has no rts/cts lines

    },
    [WICED_UART_2] =
    {
        .uart_base       = LPC_UART3,
        .tx_pin          =
        {
            .config.port = LPC_17xx_P0,
            .config.pin  = LPC_17xx_PIN_25,
            .mode        = IOCON_MODE_PULLDOWN,
            .function    = IOCON_FUNC3
        },
        .rx_pin          =
        {
            .config.port = LPC_17xx_P0,
            .config.pin  = LPC_17xx_PIN_26,
            .mode        = IOCON_MODE_INACT,
            .function    = IOCON_FUNC3//(SCU_MODE_INBUFF_EN | SCU_MODE_ZIF_DIS | SCU_MODE_FUNC2 |SCU_MODE_INACT)//SCU_MODE_INACT
        },
        .cts_pin         =
        {
            .config.port = 0xFF,
            .config.pin  = 0xFF,
            .mode        = 0xFF,
            .function    = 0xFF
        },//usart has no rts/cts lines
        .rts_pin         =
        {
            .config.port = 0xFF,
            .config.pin  = 0xFF,
            .mode        = 0xFF,
            .function    = 0xFF
        },//usart has no rts/cts lines

    },
};
platform_uart_driver_t platform_uart_drivers[WICED_UART_MAX];

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
 * gSPI: WWD_PIN_BOOTSTRAP[1:0] = b'01
 */
const platform_gpio_t wifi_control_pins[] =
{
    [WWD_PIN_POWER  ]     = { LPC_17xx_P0,  7 },
    [WWD_PIN_RESET  ]     = { LPC_17xx_P0,  6 },
    [WWD_PIN_32K_CLK]     = { LPC_17xx_P1, 27 },
    [WWD_PIN_BOOTSTRAP_0] = { LPC_17xx_P0,  0 },
    [WWD_PIN_BOOTSTRAP_1] = { LPC_17xx_P0,  1 },
};

/* Wi-Fi gSPI bus pins. Used by WICED/platform/STM32F2xx/WWD/wwd_SPI.c */
const platform_gpio_t wifi_spi_pins[] =
{
    [WWD_PIN_SPI_IRQ ] = { LPC_17xx_P2, 13 },
    [WWD_PIN_SPI_CS  ] = { LPC_17xx_P0, 16 },
    [WWD_PIN_SPI_CLK ] = { LPC_17xx_P0, 15 },
    [WWD_PIN_SPI_MOSI] = { LPC_17xx_P0, 18 },
    [WWD_PIN_SPI_MISO] = { LPC_17xx_P0, 17 },
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void platform_init_peripheral_irq_priorities( void )
{
    /* Interrupt priority setup. Called by WICED/platform/MCU/LPC17xx/platform_init.c */
    NVIC_SetPriority( UART0_IRQn ,  6 ); /* WICED_UART_1            */
    NVIC_SetPriority( UART1_IRQn ,  6 ); /* Unused in this platform */
    NVIC_SetPriority( UART2_IRQn ,  6 ); /* Unused in this platform */
    NVIC_SetPriority( UART3_IRQn ,  6 ); /* WICED_UART_2            */
    NVIC_SetPriority( EINT3_IRQn , 14 ); /* GPIO                    */
}

void platform_init_external_devices( void )
{
    /* Initialise LEDs and turn off by default */
    /* platform_gpio_init( &platform_gpio_pins[ WICED_LED1 ], OUTPUT_PUSH_PULL ); */
    /* platform_gpio_init( &platform_gpio_pins[ WICED_LED2 ], OUTPUT_PUSH_PULL ); */
    /* platform_gpio_output_low( &platform_gpio_pins[ WICED_LED1 ] );             */
    /* platform_gpio_output_low( &platform_gpio_pins[ WICED_LED2 ] );             */

    /* Initialise buttons to input by default */
    /* platform_gpio_init( &platform_gpio_pins[WICED_BUTTON1],INPUT_PULL_UP ); */
    /* platform_gpio_init( &platform_gpio_pins[WICED_BUTTON2],INPUT_PULL_UP ); */

#ifndef WICED_DISABLE_STDIO
    /* Initialise UART standard I/O */
    platform_stdio_init( &platform_uart_drivers[STDIO_UART], &platform_uart_peripherals[STDIO_UART], &stdio_config ); //mc_hijack
#endif
}

/******************************************************
 *           Interrupt Handler Definitions
 ******************************************************/

WWD_RTOS_DEFINE_ISR( usart0_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( usart3_irq )
{
    platform_uart_irq( &platform_uart_drivers[WICED_UART_2] );
}

/******************************************************
 *            Interrupt Handlers Mapping
 ******************************************************/

/* These DMA assignments can be found LPC17xx datasheet DMA section */
WWD_RTOS_MAP_ISR( usart0_irq, UART0_irq )
WWD_RTOS_MAP_ISR( usart3_irq, UART3_irq )
