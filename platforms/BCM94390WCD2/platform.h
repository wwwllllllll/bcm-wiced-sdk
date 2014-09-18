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
 * Defines peripherals available for use on BCM94390WCD2 board
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
/*
BCM94390WCD2 platform pin definitions ...
+-------------------------------------------------------------------------------------------+
| Enum ID                |Pin |   Pin Name on    | Peripheral  |    Board     | Peripheral  |
|                        | #  |      Chip        | Available   |  Connection  |   Alias     |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_1           | 51 | UART1_RTS_N_A0   | A_GPIO0     |              |             |
|                        |    |                  | UART1_RTS_N |              |             |
|                        |    |                  | SPI_CS_N    |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_2           | 50 | UART1_CTS_N_A1   | A_GPIO1     |              |             |
|                        |    |                  | UART1_CTS_N |              |             |
|                        |    |                  | SPI_CLK     |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_3           | 59 | JTAG_TMS_A2      | GPIO2       |              |             |
|                        |    |                  | JTAG_TMS    |              |             |
|                        |    |                  | I2S_DI      |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_4           | 60 | JTAG_TCK_A3      | GPIO3       |              |             |
|                        |    |                  | JTAG_TCK    |              |             |
|                        |    |                  | I2S_WS      |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_5           | 61 | JTAG_TDI_A4      | GPIO4       |              |             |
|                        |    |                  | JTAG_TDI    |              |             |
|                        |    |                  | I2S_DO      |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_6           | 62 | JTAG_TDO_A5      | GPIO5       |              |             |
|                        |    |                  | JTAG_TDO    |              |             |
|                        |    |                  | I2S_CLK     |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_7           | 43 | I2S_DI_A6        | A_GPIO6     | SW2          |             |
|                        |    |                  | I2S_DI      |              |             |
|                        |    |                  | UART2_CTS_N |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_8           | 45 | I2S_WS_A7        | GPIO7       |              |             |
|                        |    |                  | I2S_WS      |              |             |
|                        |    |                  | UART2_TXD   |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_9           | 42 | I2S_D0_A8        | A_GPIO5     | SW1          |             |
|                        |    |                  | I2S_D0      |              |             |
|                        |    |                  | UART2_RTS_N |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_10          | 44 | I2S_CLK_A9       | GPIO6       |              |             |
|                        |    |                  | I2S_CLK     |              |             |
|                        |    |                  | UART2_RXD   |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_11          | 57 | WAKE_A10         | GPIO0       |              |             |
|                        |    |                  | WAKE        |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_GPIO_12          | 58 | GPIO_A11         | GPIO1       |              |             |
|                        |    |                  | SPI_IRQ     |              |             |
|                        |    |                  | UART3_TX/RX |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_PERIPHERAL_PIN_1 | 52 | UART1_RX_GPIO_A5 | UART1_RXD   |              |             |
|                        |    |                  | SPI_MISO    |              |             |
|                        |    |                  | I2C_SDA     |              |             |
|                        |    |                  | UART2_RXD   |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_PERIPHERAL_PIN_2 | 53 | UART1_TX_GPIO_A4 | UART1_TXD   |              |             |
|                        |    |                  | SPI_MISO    |              |             |
|                        |    |                  | I2C_SCL     |              |             |
|                        |    |                  | SPI_MISO    |              |             |
|                        |    |                  | UART2_TXD   |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_PERIPHERAL_PIN_7 | 33 | UART2_TX         | UART4_TXD   |              |             |
|------------------------+----+------------------+-------------+--------------+-------------|
| WICED_PERIPHERAL_PIN_8 | 34 | UART2_RX         | UART4_RXD   |              |             |
+------------------------+----+------------------+-------------+--------------+-------------+
*/

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    WICED_GPIO_1,
    WICED_GPIO_2,
    WICED_GPIO_3,
    WICED_GPIO_4,
    WICED_GPIO_5,
    WICED_GPIO_6,
    WICED_GPIO_7,
    WICED_GPIO_8,
    WICED_GPIO_9,
    WICED_GPIO_10,
    WICED_GPIO_11,
    WICED_GPIO_12,
    WICED_GPIO_13,
    WICED_GPIO_14,
    WICED_GPIO_15,
    WICED_GPIO_16,
    WICED_GPIO_17,
    WICED_GPIO_18,
    WICED_GPIO_19,
    WICED_GPIO_20,
    WICED_GPIO_21,
    WICED_GPIO_22,
    WICED_GPIO_23,
    WICED_GPIO_24,
    WICED_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
    WICED_GPIO_32BIT = 0x7FFFFFFF
} wiced_gpio_t;

typedef enum
{
    WICED_PERIPHERAL_PIN_1 = WICED_GPIO_MAX,
    WICED_PERIPHERAL_PIN_2,
#if defined ( EXPOSED_4390_SFLASH_PINS )
    WICED_PERIPHERAL_PIN_3,
    WICED_PERIPHERAL_PIN_4,
    WICED_PERIPHERAL_PIN_5,
    WICED_PERIPHERAL_PIN_6,
#endif /* ifdef EXPOSED_4390_SFLASH_PINS */
    WICED_PERIPHERAL_PIN_7,
    WICED_PERIPHERAL_PIN_8,
    WICED_PERIPHERAL_PIN_MAX, /* Denotes the total number of GPIO and peripheral pins. Not a valid pin alias */
    WICED_PERIPHERAL_PIN_32BIT = 0x7FFFFFF,
} wiced_peripheral_pin_t;

typedef enum
{
    WICED_SPI_1,
    WICED_SPI_2,
    WICED_SPI_3,
    WICED_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
    WICED_SPI_32BIT = 0x7FFFFFFF,
} wiced_spi_t;

typedef enum
{
    WICED_I2C_1,
    WICED_I2C_MAX, /* Denotes the total number of I2C port aliases. Not a valid I2C alias */
    WICED_I2C_32BIT = 0x7FFFFFFF,
} wiced_i2c_t;

typedef enum
{
    WICED_PWM_NONE,
    WICED_PWM_MAX,  /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
    WICED_PWM_32BIT = 0x7FFFFFFF,
} wiced_pwm_t;

typedef enum
{
    WICED_ADC_NONE, /* ADC unavailable */
    WICED_ADC_MAX,  /* Denotes the total number of ADC port aliases. Not a valid ADC alias */
    WICED_ADC_32BIT = 0x7FFFFFFF,
} wiced_adc_t;

typedef enum
{
    WICED_UART_1,
    WICED_UART_2,
    WICED_UART_3,
    WICED_UART_4,
    WICED_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
    WICED_UART_32BIT = 0x7FFFFFFF,
} wiced_uart_t;

/******************************************************
 *                    Constants
 ******************************************************/

/* UART port used for standard I/O */
#define STDIO_UART  ( WICED_UART_1 )

/* #define WICED_SPI_FLASH is internal to the module */
#define WICED_PLATFORM_INCLUDES_SPI_FLASH

/* Components connected to external I/Os */
#define WICED_LED1        ( WICED_GPIO_10 )
#define WICED_LED2        ( WICED_GPIO_8 )
#define WICED_BUTTON1     ( WICED_GPIO_9 )
#define WICED_BUTTON2     ( WICED_GPIO_7 )
#define WICED_THERMISTOR  ( WICED_ADC_NONE )

/* I/O connection <-> Peripheral Connections */
#define WICED_LED1_JOINS_PWM        ( WICED_LED1 )
#define WICED_LED2_JOINS_PWM        ( WICED_LED2 )
#define WICED_THERMISTOR_JOINS_ADC  ( WICED_THERMISTOR )

/*  Bootloader LED D1 */
#define BOOTLOADER_LED_GPIO  ( WICED_LED1 )

 /* Bootloader Button S1 */
#define BOOTLOADER_BUTTON_GPIO  ( WICED_BUTTON1 )


#ifdef __cplusplus
} /*extern "C" */
#endif
