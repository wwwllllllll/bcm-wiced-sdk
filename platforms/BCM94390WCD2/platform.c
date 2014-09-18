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
 * Defines board support package for BCM94390WCD2 board
 */
#include "platform.h"
#include "platform_assert.h"
#include "platform_cmsis.h"
#include "platform_config.h"
#include "platform_init.h"
#include "platform_peripheral.h"
#include "platform_mcu_peripheral.h"
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

/* NOTE: Peripheral interrupts are managed internally in BCM4390 peripheral libraries
 */

/* GPIO pin table. Used by WICED/platform/MCU/wiced_platform_common.c.*/
const platform_gpio_t platform_gpio_pins[] =
{
    [WICED_GPIO_1          ] = { PA00 }, /* UART1_RTS_N_A0 */
    [WICED_GPIO_2          ] = { PA01 }, /* UART1_CTS_N_A1 */
    [WICED_GPIO_3          ] = { PA02 }, /* JTAG_TMS_A2    */
    [WICED_GPIO_4          ] = { PA03 }, /* JTAG_TCK_A3    */
    [WICED_GPIO_5          ] = { PA04 }, /* JTAG_TDI_A4    */
    [WICED_GPIO_6          ] = { PA05 }, /* JTAG_TDO_A5    */
    [WICED_GPIO_7          ] = { PA06 }, /* I2S_DI_A6      */
    [WICED_GPIO_8          ] = { PA07 }, /* I2S_WS_A7      */
    [WICED_GPIO_9          ] = { PA08 }, /* I2S_D0_A8      */
    [WICED_GPIO_10         ] = { PA09 }, /* I2S_CLK_A9     */
    [WICED_GPIO_11         ] = { PA10 }, /* WAKE_A10       */
    [WICED_GPIO_12         ] = { PA11 }, /* GPIO_A11       */
    [WICED_GPIO_13         ] = { PB00 },
    [WICED_GPIO_14         ] = { PB01 },
    [WICED_GPIO_15         ] = { PB02 },
    [WICED_GPIO_16         ] = { PB03 },
    [WICED_GPIO_17         ] = { PB04 },
    [WICED_GPIO_18         ] = { PB05 },
    [WICED_GPIO_19         ] = { PB06 },
    [WICED_GPIO_20         ] = { PB07 },
    [WICED_GPIO_21         ] = { PB08 },
    [WICED_GPIO_22         ] = { PB09 },
    [WICED_GPIO_23         ] = { PB10 },
    [WICED_GPIO_24         ] = { PB11 },

    [WICED_PERIPHERAL_PIN_1] = { PA12 }, /* UART1_RX_GPIO_A5 & UART2_TX */
    [WICED_PERIPHERAL_PIN_2] = { PA13 }, /* UART1_TX_GPIO_A4 & UART2_RX */

#if defined ( EXPOSED_4390_SFLASH_PINS )
    [WICED_PERIPHERAL_PIN_3] = { SFLASH_SPI_CLK  },
    [WICED_PERIPHERAL_PIN_4] = { SFLASH_SPI_CS   },
    [WICED_PERIPHERAL_PIN_5] = { SFLASH_SPI_MISO },
    [WICED_PERIPHERAL_PIN_6] = { SFLASH_SPI_MOSI },
#endif /* ifdef EXPOSED_4390_SFLASH_PINS */

    [WICED_PERIPHERAL_PIN_7] = { PB12 }, /* UART2_TX */
    [WICED_PERIPHERAL_PIN_8] = { PB13 }, /* UART2_RX */
};

/* I2C peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_i2c_t platform_i2c_peripherals[] =
{
    [WICED_I2C_1] =
    {
        .scl_pin = &platform_gpio_pins[WICED_PERIPHERAL_PIN_2],
        .sda_pin = &platform_gpio_pins[WICED_PERIPHERAL_PIN_1],
    },
};

/* UART peripherals and runtime drivers. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_uart_t platform_uart_peripherals[] =
{
    [WICED_UART_1] =
    {
        .port    = UART1,
        .tx_pin  = &platform_gpio_pins[WICED_PERIPHERAL_PIN_2],
        .rx_pin  = &platform_gpio_pins[WICED_PERIPHERAL_PIN_1],
        .cts_pin = &platform_gpio_pins[WICED_GPIO_2],
        .rts_pin = &platform_gpio_pins[WICED_GPIO_1],
    },
    [WICED_UART_2] =
    {
        .port    = UART2,
        .tx_pin  = &platform_gpio_pins[WICED_GPIO_10],
        .rx_pin  = &platform_gpio_pins[WICED_GPIO_8],
        .cts_pin = &platform_gpio_pins[WICED_GPIO_7],
        .rts_pin = &platform_gpio_pins[WICED_GPIO_9],

        /* Alternate pins for UART2 */
        /* .tx_pin  = &platform_gpio_pins[WICED_PERIPHERAL_PIN_2], */
        /* .rx_pin  = &platform_gpio_pins[WICED_PERIPHERAL_PIN_1], */
        /* .cts_pin = &platform_gpio_pins[WICED_GPIO_2],           */
        /* .rts_pin = &platform_gpio_pins[WICED_GPIO_1],           */
    },
    [WICED_UART_3] =
    {
        .port    = UART3,
        .tx_pin  = &platform_gpio_pins[WICED_GPIO_8],
        .rx_pin  = &platform_gpio_pins[WICED_GPIO_9],
        .cts_pin = NULL,
        .rts_pin = NULL,
    },
    [WICED_UART_4] = /* this uart resides in wlan core */
    {
        .port    = UART4,
        .tx_pin  = &platform_gpio_pins[WICED_PERIPHERAL_PIN_7],
        .rx_pin  = &platform_gpio_pins[WICED_PERIPHERAL_PIN_8],
        .cts_pin = NULL,
        .rts_pin = NULL,
    }
};
platform_uart_driver_t platform_uart_drivers[WICED_UART_MAX];

/* PWM peripherals. Used by WICED/platform/MCU/wiced_platform_common.c */
const platform_spi_t platform_spi_peripherals[] =
{
    [WICED_SPI_1] =
    {
        .port                  = SPI1,
        .is_master_supported   = WICED_TRUE,
        .is_slave_supported    = WICED_TRUE,
        .clock_pin             = &platform_gpio_pins[WICED_GPIO_2],               /* UART1_CTS_N_A1   - pin 20 expansion header */
        .mosi_pin              = &platform_gpio_pins[WICED_PERIPHERAL_PIN_1],     /* UART1_RX_GPIO_A5 - pin 17 expansion header */
        .miso_pin              = &platform_gpio_pins[WICED_PERIPHERAL_PIN_2],     /* UART1_TX_GPIO_A4 - pin 16 expansion header */
        .slave_chip_select_pin = &platform_gpio_pins[WICED_GPIO_1],               /* UART1_RTS_N_A0   - pin 22 expansion header */
        .slave_ready_pin       = &platform_gpio_pins[WICED_GPIO_7],               /* I2S_DI_A6        - pin 15 expansion header */
    },
    [WICED_SPI_2] =
    {
        .port                  = SPI2,
        .is_master_supported   = WICED_TRUE,
        .is_slave_supported    = WICED_TRUE,
        .clock_pin             = &platform_gpio_pins[WICED_PERIPHERAL_PIN_2],     /* UART1_TX_GPIO_A4 - pin 16 expansion header */
        .mosi_pin              = &platform_gpio_pins[WICED_PERIPHERAL_PIN_1],     /* UART1_RX_GPIO_A5 - pin 17 expansion header */
        .miso_pin              = &platform_gpio_pins[WICED_GPIO_1],               /* UART1_RTS_N_A0   - pin 22 expansion header */
        .slave_chip_select_pin = &platform_gpio_pins[WICED_GPIO_2],               /* UART1_CTS_N_A1   - pin 20 expansion header */
        .slave_ready_pin       = &platform_gpio_pins[WICED_GPIO_7],               /* I2S_DI_A6        - pin 15 expansion header */
    },

    [WICED_SPI_3] =
    {
#ifdef EXPOSED_4390_SFLASH_PINS
        .port                  = SPI2,
        .is_master_supported   = WICED_TRUE,
        .is_slave_supported    = WICED_FALSE,
        .clock_pin             = &platform_gpio_pins[WICED_PERIPHERAL_PIN_3],
        .mosi_pin              = &platform_gpio_pins[WICED_PERIPHERAL_PIN_6],
        .miso_pin              = &platform_gpio_pins[WICED_PERIPHERAL_PIN_5],
        .slave_chip_select_pin = NULL,
        .slave_ready_pin       = NULL,
#endif /* ifdef EXPOSED_4390_SFLASH_PINS */
    },
};

/* SPI flash. Exposed to the applications through include/wiced_platform.h */
#if defined ( WICED_PLATFORM_INCLUDES_SPI_FLASH ) && defined ( EXPOSED_4390_SFLASH_PINS )
const wiced_spi_device_t wiced_spi_flash =
{
    .port        = WICED_SPI_1,
    .chip_select = WICED_PERIPHERAL_PIN_4,
    .speed       = 24000000,
    .mode        = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST),
    .bits        = 8
};
#endif

platform_spi_slave_driver_t platform_spi_slave_drivers[WICED_SPI_MAX];

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

/******************************************************
 *               Function Definitions
 ******************************************************/

void platform_init_peripheral_irq_priorities( void )
{
    /* Interrupt priority setup. Called by WICED/platform/MCU/BCM439x/platform_init.c */
    NVIC_SetPriority( DmaDoneInt_IRQn,   2 ); /* DMA */
    NVIC_SetPriority( PTU1_IRQn,         6 ); /* PTU1 (UART1 and SPI1) */
    NVIC_SetPriority( PTU2_IRQn,         6 ); /* PTU2 (UART2 and SPI2) */
    NVIC_SetPriority( GPIOA_BANK0_IRQn, 14 ); /* GPIOA */
    NVIC_SetPriority( GPIOA_BANK1_IRQn, 14 ); /* GPIOA */
}

void platform_init_external_devices( void )
{
    /* Initialise wake up pin */
    platform_gpio_init( &platform_gpio_pins[WICED_GPIO_11], INPUT_HIGH_IMPEDANCE );
    platform_wakeup_pin_enable( platform_gpio_pins[WICED_GPIO_11].pin, WAKEUP_PIN_ACTIVE_HIGH );

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
 *            Interrupt Handlers Mapping
 ******************************************************/

WWD_RTOS_DEFINE_ISR( ptu1_irq )
{
    platform_spi_slave_irq( &platform_spi_slave_drivers[WICED_SPI_1] );
    platform_uart_irq( &platform_uart_drivers[WICED_UART_1] );
}

WWD_RTOS_DEFINE_ISR( ptu2_irq )
{
    platform_spi_slave_irq( &platform_spi_slave_drivers[WICED_SPI_2] );
    platform_uart_irq( &platform_uart_drivers[WICED_UART_2] );
}

WWD_RTOS_MAP_ISR( ptu1_irq, PTU1_irq );
WWD_RTOS_MAP_ISR( ptu2_irq, PTU2_irq );
