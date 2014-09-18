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
 * Defines peripherals available for use on BCM943362WCD4_LPCX1769 board
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/


/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    WICED_GPIO_1,
    WICED_GPIO_2,
    WICED_GPIO_3,
    WICED_GPIO_4,
    WICED_GPIO_MAX, /* Denotes the total number of GPIO port aliases. Not a valid GPIO alias */
    WICED_GPIO_32BIT = 0x7FFFFFFF,
} wiced_gpio_t;

typedef enum
{
    WICED_SPI_NONE,
    WICED_SPI_MAX, /* Denotes the total number of SPI port aliases. Not a valid SPI alias */
    WICED_SPI_32BIT = 0x7FFFFFFF,
} wiced_spi_t;

typedef enum
{
    WICED_I2C_NONE = 0xFF,
    WICED_I2C_MAX,
    WICED_I2C_32BIT = 0x7FFFFFFF,
} wiced_i2c_t;

typedef enum
{
    WICED_PWM_NONE,
    WICED_PWM_MAX, /* Denotes the total number of PWM port aliases. Not a valid PWM alias */
    WICED_PWM_32BIT = 0x7FFFFFFF,
} wiced_pwm_t;

typedef enum
{
    WICED_ADC_NONE,
    WICED_ADC_MAX, /* Denotes the total number of ADC port aliases. Not a valid ADC alias */
    WICED_ADC_32BIT = 0x7FFFFFFF,
} wiced_adc_t;

typedef enum
{
    WICED_UART_1,
    WICED_UART_2,
    WICED_UART_MAX, /* Denotes the total number of UART port aliases. Not a valid UART alias */
    WICED_UART_32BIT = 0x7FFFFFFF,
} wiced_uart_t;

/******************************************************
 *                    Constants
 ******************************************************/

/* UART port used for standard I/O */
#define STDIO_UART  ( WICED_UART_1 )

/* Components connected to external I/Os */
/* #define WICED_LED1       ( WICED_GPIO_1 ) */
/* #define WICED_LED2       ( WICED_GPIO_2 ) */
/* #define WICED_BUTTON1    ( WICED_GPIO_3 ) */
/* #define WICED_BUTTON2    ( WICED_GPIO_4 ) */
/* #define WICED_THERMISTOR ( WICED_GPIO_4 ) */

/* I/O connection <-> Peripheral Connections */
/* #define WICED_LED1_JOINS_PWM       ( WICED_PWM_1 ) */
/* #define WICED_LED2_JOINS_PWM       ( WICED_PWM_2 ) */
/* #define WICED_THERMISTOR_JOINS_ADC ( WICED_ADC_3 ) */

/* Bootloader LED D1 */
/* #define BOOTLOADER_LED_GPIO     WICED_LED1        */
/* #define BOOTLOADER_LED_ON_STATE WICED_ACTIVE_HIGH */

/* Bootloader Button S1 */
/* #define BOOTLOADER_BUTTON_GPIO          WICED_BUTTON1    */
/* #define BOOTLOADER_BUTTON_PRESSED_STATE WICED_ACTIVE_LOW */

#ifdef __cplusplus
} /*extern "C" */
#endif
