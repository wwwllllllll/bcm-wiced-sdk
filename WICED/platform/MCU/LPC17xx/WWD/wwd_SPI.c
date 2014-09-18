/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "string.h" /* for memcpy */
#include "wifi_nvram_image.h"
#include "wwd_bus_protocol.h"
#include "wwd_assert.h"
#include "wwd_rtos.h"
#include "wwd_platform_common.h"
#include "network/wwd_buffer_interface.h"
#include "platform/wwd_platform_interface.h"
#include "platform/wwd_bus_interface.h"
#include "platform/wwd_spi_interface.h"
#include "platform_cmsis.h"
#include "platform_config.h"
#include "platform_peripheral.h"

/******************************************************
 *             Constants
 ******************************************************/

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

void        SPI_Init_Custom           ( LPC_SPI_T *pSPI, platform_spi_peripheral_config_t* spi_config );
void        transfer_complete_callback( void );
static void spi_irq_handler           ( void* arg );

/******************************************************
 *             Variables
 ******************************************************/

/******************************************************
 *             Static Function Declarations
 ******************************************************/

/******************************************************
 *             Function definitions
 ******************************************************/

wwd_result_t host_platform_bus_init( void )
{
    platform_spi_peripheral_config_t spi_init;
    platform_spi_t                   spi_config;

    spi_config.spi_base       = LPC_SPI;
    spi_config.cs             = wifi_spi_pins[WWD_PIN_SPI_CS];
    spi_config.irq            = wifi_spi_pins[WWD_PIN_SPI_IRQ];
    spi_config.clock.config   = wifi_spi_pins[WWD_PIN_SPI_CLK];
    spi_config.clock.mode     = IOCON_MODE_PULLDOWN;
    spi_config.clock.function = IOCON_FUNC3;
    spi_config.mosi.config    = wifi_spi_pins[WWD_PIN_SPI_MOSI];
    spi_config.mosi.mode      = IOCON_MODE_PULLDOWN;
    spi_config.mosi.function  = IOCON_FUNC3;
    spi_config.miso.config    = wifi_spi_pins[WWD_PIN_SPI_MISO];
    spi_config.miso.mode      = IOCON_MODE_INACT;
    spi_config.miso.function  = IOCON_FUNC3;

    /* Setup the interrupt input for WLAN_IRQ */
    platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_IRQ], INPUT_HIGH_IMPEDANCE );
    platform_gpio_irq_enable( &wifi_spi_pins[WWD_PIN_SPI_IRQ], IRQ_TRIGGER_RISING_EDGE, spi_irq_handler, 0 );

    /* Setup the CS line */
    platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_CS], OUTPUT_PUSH_PULL );

    /* Mux the port and pin to direct it to SPI */
    platform_gpio_set_alternate_function( &spi_config.clock );
    platform_gpio_set_alternate_function( &spi_config.miso  );
    platform_gpio_set_alternate_function( &spi_config.mosi  );

    /* Setup SPI slave select GPIOs */
    platform_gpio_output_high( &wifi_spi_pins[WWD_PIN_SPI_CS] );

#if defined ( WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP )
    /* Set GPIO_B[1:0] to 01 to put WLAN module into gSPI mode */
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0], OUTPUT_PUSH_PULL );
    platform_gpio_output_high( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0] );
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1] );
#endif

    /* Setup SPI */
    spi_init.master_slave_mode           = SPI_MODE_MASTER;
    spi_init.data_clock_format.clockMode = SPI_CLOCK_CPHA0_CPOL0;
    spi_init.data_clock_format.dataOrder = SPI_DATA_MSB_FIRST;
    spi_init.data_clock_format.bits      = SPI_BITS_8;
    spi_init.bit_rate                    = SPI_BITRATE;

    /* Init SPI and enable*/
    SPI_Init_Custom( LPC_SPI, &spi_init );

    return WICED_SUCCESS;
}

void SPI_Init_Custom(LPC_SPI_T *pSPI, platform_spi_peripheral_config_t* spi_config)
{
    Chip_SPI_Int_Disable( pSPI );
    Chip_Clock_EnablePeriphClock( SYSCTL_CLOCK_SPI );
    Chip_SPI_SetMode( pSPI, spi_config->master_slave_mode );
    IP_SPI_SetFormat( pSPI, spi_config->data_clock_format.bits, spi_config->data_clock_format.clockMode, spi_config->data_clock_format.dataOrder );
    Chip_SPI_SetBitRate( pSPI, spi_config->bit_rate );
}

wwd_result_t host_platform_bus_deinit( void )
{
    return WICED_SUCCESS;
}

wwd_result_t host_platform_spi_transfer( wwd_bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
    wiced_result_t result = WWD_SUCCESS;
    CHIP_SPI_DATA_SETUP_T pXfSetup;

    platform_gpio_output_low( &wifi_spi_pins[WWD_PIN_SPI_CS] );

    pXfSetup.pTxData = buffer;
    if ( dir == BUS_WRITE )
    {
        pXfSetup.pRxData = NULL;
    }
    else
    {
        pXfSetup.pRxData = buffer;
    }

    pXfSetup.length        = buffer_length;
    pXfSetup.cnt           = 0;
    pXfSetup.fnAftFrame    = NULL;
    pXfSetup.fnAftTransfer = transfer_complete_callback;
    pXfSetup.fnBefFrame    = NULL;
    pXfSetup.fnBefTransfer = NULL;

    Chip_SPI_RWFrames_Blocking( LPC_SPI, &pXfSetup );

    platform_gpio_output_high( &wifi_spi_pins[WWD_PIN_SPI_CS] );

    return result;
}

void transfer_complete_callback( void )
{
}

wwd_result_t host_platform_bus_enable_interrupt( void )
{
    return  WICED_SUCCESS;
}

wwd_result_t host_platform_bus_disable_interrupt( void )
{
    return  WICED_SUCCESS;
}

void host_platform_bus_buffer_freed( wwd_buffer_dir_t direction )
{
    UNUSED_PARAMETER( direction );
}

/******************************************************
 *             IRQ Handler definitions
 ******************************************************/

static void spi_irq_handler( void* arg )
{
    UNUSED_PARAMETER( arg );
    wwd_thread_notify_irq( );
}
