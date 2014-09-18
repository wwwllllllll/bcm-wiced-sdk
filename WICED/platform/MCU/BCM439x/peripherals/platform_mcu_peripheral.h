/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#pragma once

#include "platform_constants.h"
#include "wwd_constants.h"
#include "ring_buffer.h"
#include "RTOS/wwd_rtos_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/* Default STDIO buffer size */
#ifndef STDIO_BUFFER_SIZE
#define STDIO_BUFFER_SIZE         (64)
#endif

#define UART_CONSOLE_MASK (0x01)

/******************************************************
 *                   Enumerations
 ******************************************************/

/* 4390 pins */
typedef enum
{
    PA00,  /* UART1_RTS_N_A0 */
    PA01,  /* UART1_CTS_N_A1 */
    PA02,  /* JTAG_TMS_A2 */
    PA03,  /* JTAG_TCK_A3 */
    PA04,  /* JTAG_TDI_A4 */
    PA05,  /* JTAG_TDO_A5 */
    PA06,  /* I2S_DI_A6 */
    PA07,  /* I2S_WS_A7 */
    PA08,  /* I2S_D0_A8 */
    PA09,  /* I2S_CLK_A9 */
    PA10,  /* WAKE_A10 */
    PA11,  /* GPIO_A11 */
    PA12,  /* UART1_RX_GPIO_A5 */
    PA13,  /* UART1_TX_GPIO_A4 */
    PB00,
    PB01,
    PB02,
    PB03,
    PB04,
    PB05,
    PB06,
    PB07,
    PB08,
    PB09,
    PB10,
    PB11,
    PB12,  /* UART4_TX_RF_SW_CTRL_8 */
    PB13,  /* UART4_RX_RF_SW_CTRL_8 */
    DEBUG_EN,

#ifdef EXPOSED_4390_SFLASH_PINS
    SFLASH_SPI_CLK,
    SFLASH_SPI_MOSI,
    SFLASH_SPI_MISO,
    SFLASH_SPI_CS,
#endif /* ifdef EXPOSED_4390_SFLASH_PINS */

    PIN_MAX  /* Denotes maximum value. Not a valid pin */
} platform_pin_t;


/* Pin alternate functions */
typedef enum
{
    PIN_FUNCTION_GPIO,
    PIN_FUNCTION_UART1,
    PIN_FUNCTION_UART2,
    PIN_FUNCTION_UART3,
    PIN_FUNCTION_SPI1,
    PIN_FUNCTION_SPI2,
    PIN_FUNCTION_I2C,
    PIN_FUNCTION_I2S_SLAVE,
    PIN_FUNCTION_I2S_MASTER,
    PIN_FUNCTION_JTAG,
    PIN_FUNCTION_MAX /* Denotes max value. Not a valid alternate function */
} platform_pin_function_t;

typedef enum
{
    UART1,
    UART2,
    UART3,
    UART4
} platform_uart_port_t;

typedef enum
{
    SPI1,
#ifdef EXPOSED_4390_SFLASH_PINS
    SPI2,
#endif /* ifdef EXPOSED_4390_SFLASH_PINS */
} platform_spi_port_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

typedef struct
{
    uint32_t output_disable              ;
    uint32_t pullup_enable               ;
    uint32_t pulldown_enable             ;
    uint32_t schmitt_trigger_input_enable;
    uint32_t drive_strength              ; // Add one to get drive strength in milliamps (1mA - 8mA)
    uint32_t input_disable               ;
} platform_pin_gpio_config_t;

typedef struct
{
    platform_pin_t pin;
} platform_gpio_t;

typedef struct
{
    platform_uart_port_t   port;
    const platform_gpio_t* tx_pin;
    const platform_gpio_t* rx_pin;
    const platform_gpio_t* cts_pin;
    const platform_gpio_t* rts_pin;
} platform_uart_t;

typedef struct
{
    wiced_ring_buffer_t*   rx_buffer;
    host_semaphore_type_t  rx_complete;
    host_semaphore_type_t  tx_complete;
    volatile uint32_t      tx_size;
    volatile uint32_t      rx_size;
    platform_uart_t*       interface;
    wiced_bool_t           hw_flow_control_is_on;
    const platform_gpio_t* rts_gpio;
    wiced_bool_t           rts_is_deasserted;
} platform_uart_driver_t;

typedef struct
{
    platform_spi_port_t    port;
    const platform_gpio_t* clk_pin;
    const platform_gpio_t* mosi_pin;
    const platform_gpio_t* miso_pin;
    const platform_gpio_t* intr_pin;
} platform_spi_t;

typedef struct
{
    const platform_gpio_t* scl_pin;
    const platform_gpio_t* sda_pin;
} platform_i2c_t;

typedef struct
{
    uint32_t dummy; /* Interface unimplemented */
} platform_pwm_t;

typedef struct
{
    uint32_t dummy; /* Interface unimplemented */
} platform_adc_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

platform_result_t platform_gpio_irq_manager_init( void );
platform_result_t platform_pin_init             ( platform_pin_t pin, platform_pin_gpio_config_t config );
platform_result_t platform_pin_set_function     ( platform_pin_t pin, platform_pin_function_t function );
platform_result_t platform_watchdog_init        ( void );
void              platform_filesystem_init      ( void );
void              platform_setup_otp            ( void );
void              init_4390_after_restart       ( void );
void              init_4390_after_global_init   ( void );
int               set_OTP_sflash_boot_439x      ( void );

/* Call this from GPIO interrupt handler */
void              platform_gpio_irq             ( void );

/* Call this from UART interrupt handler */
void              platform_uart_irq             ( platform_uart_driver_t* driver );

#ifdef __cplusplus
} /*extern "C" */
#endif
