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
 * Defines WWD SPI functions for STM32F4xx MCU
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

#define DMA_TIMEOUT_LOOPS     (10000000)
#define SPI_BUS_TX_DMA_STREAM (DMA1_Stream3_IRQn)

/******************************************************
 *             Structures
 ******************************************************/

/******************************************************
 *             Variables
 ******************************************************/

static host_semaphore_type_t spi_transfer_finished_semaphore;

/******************************************************
 *             Static Function Declarations
 ******************************************************/

static void spi_irq_handler( void* arg );
void dma_irq( void );

/******************************************************
 *             Function definitions
 ******************************************************/

wwd_result_t host_platform_bus_init( void )
{
    SPI_InitTypeDef spi_init;
    DMA_InitTypeDef dma_init_structure;
    uint32_t        a;

    platform_mcu_powersave_disable();

    host_rtos_init_semaphore(&spi_transfer_finished_semaphore);

    RCC_PCLK1Config( RCC_HCLK_Div2 ); /* Set clock to 18MHz (assuming 72MHz STM32 system clock) */

    /* Enable SPI_SLAVE DMA clock */
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1, ENABLE );

    /* Enable SPI_SLAVE Periph clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE );

    /* Enable SYSCFG. Needed for selecting EXTI interrupt line */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, ENABLE );

    /* Setup the interrupt input for WLAN_IRQ */
    platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_IRQ], INPUT_HIGH_IMPEDANCE );
    platform_gpio_irq_enable( &wifi_spi_pins[WWD_PIN_SPI_IRQ], IRQ_TRIGGER_RISING_EDGE, spi_irq_handler, 0 );

    /* Setup SPI slave select GPIOs */
    platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_CS], OUTPUT_PUSH_PULL );
    platform_gpio_output_high( &wifi_spi_pins[WWD_PIN_SPI_CS] );

    /* Setup the SPI lines */
    for ( a = WWD_PIN_SPI_CLK; a < WWD_PIN_SPI_MAX; a++ )
    {
        platform_gpio_set_alternate_function( wifi_spi_pins[ a ].port, wifi_spi_pins[ a ].pin_number, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_AF_SPI2 );
    }

#if defined ( WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP )
    /* Set GPIO_B[1:0] to 01 to put WLAN module into gSPI mode */
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0], OUTPUT_PUSH_PULL );
    platform_gpio_output_high( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0] );
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1], OUTPUT_PUSH_PULL );
    platform_gpio_output_low( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1] );
#endif

    /* Setup DMA for SPI2 RX */
    DMA_DeInit( DMA1_Stream3 );
    dma_init_structure.DMA_Channel            = DMA_Channel_0;
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &SPI2->DR;
    dma_init_structure.DMA_Memory0BaseAddr    = 0;
    dma_init_structure.DMA_DIR                = DMA_DIR_PeripheralToMemory;
    dma_init_structure.DMA_BufferSize         = 16; /* This number is arbitrary to keep the assert happy */;
    dma_init_structure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init_structure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_structure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init_structure.DMA_Mode               = DMA_Mode_Normal;
    dma_init_structure.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init_structure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init_structure.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    dma_init_structure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init_structure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init( DMA1_Stream3, &dma_init_structure );

    /* Setup DMA for SPI2 TX */
    DMA_DeInit( DMA1_Stream4 );
    dma_init_structure.DMA_Channel            = DMA_Channel_0;
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &SPI2->DR;
    dma_init_structure.DMA_Memory0BaseAddr    = 0;
    dma_init_structure.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    dma_init_structure.DMA_BufferSize         = 16; /* This number is arbitrary to keep the assert happy */
    dma_init_structure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init_structure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init_structure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init_structure.DMA_Mode               = DMA_Mode_Normal;
    dma_init_structure.DMA_Priority           = DMA_Priority_VeryHigh;
    dma_init_structure.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init_structure.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full;
    dma_init_structure.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init_structure.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init( DMA1_Stream4, &dma_init_structure );

    /* Must be lower priority than the value of configMAX_SYSCALL_INTERRUPT_PRIORITY */
    /* otherwise FreeRTOS will not be able to mask the interrupt */
    /* keep in mind that ARMCM3 interrupt priority logic is inverted, the highest value */
    /* is the lowest priority */
    NVIC_EnableIRQ( SPI_BUS_TX_DMA_STREAM );

    /* Enable DMA for TX */
    SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE );

    /* Setup SPI */
    spi_init.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    spi_init.SPI_Mode              = SPI_Mode_Master;
    spi_init.SPI_DataSize          = SPI_DataSize_8b;
    spi_init.SPI_CPOL              = SPI_CPOL_High;
    spi_init.SPI_CPHA              = SPI_CPHA_2Edge;
    spi_init.SPI_NSS               = SPI_NSS_Soft;
    spi_init.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    spi_init.SPI_FirstBit          = SPI_FirstBit_MSB;
    spi_init.SPI_CRCPolynomial     = (uint16_t) 7;

    /* Init SPI and enable it */
    SPI_Init( SPI2, &spi_init );
    SPI_Cmd( SPI2, ENABLE );

    platform_mcu_powersave_enable();

    return WICED_SUCCESS;
}

wwd_result_t host_platform_bus_deinit( void )
{
    uint32_t a;

    platform_mcu_powersave_disable();

    /* Disable SPI and SPI DMA */
    SPI_Cmd( SPI2, DISABLE );
    SPI_I2S_DeInit( SPI2 );
    SPI_I2S_DMACmd( SPI2, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE );
    DMA_DeInit( DMA1_Stream4 );
    DMA_DeInit( DMA1_Stream3 );

#if defined ( WICED_WIFI_USE_GPIO_FOR_BOOTSTRAP )
    /* Clear GPIO_B[1:0] */
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_0], INPUT_HIGH_IMPEDANCE );
    platform_gpio_init( &wifi_control_pins[WWD_PIN_BOOTSTRAP_1], INPUT_HIGH_IMPEDANCE );
#endif

    /* Clear SPI slave select GPIOs */
    platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_CS], INPUT_HIGH_IMPEDANCE );

    /* Clear the SPI lines */
    for ( a = WWD_PIN_SPI_CLK; a < WWD_PIN_SPI_MAX; a++ )
    {
        platform_gpio_init( &wifi_spi_pins[ a ], INPUT_HIGH_IMPEDANCE );
    }

    platform_gpio_irq_disable( &wifi_spi_pins[WWD_PIN_SPI_IRQ] );
    platform_gpio_init( &wifi_spi_pins[WWD_PIN_SPI_IRQ], INPUT_HIGH_IMPEDANCE );

    /* Disable SPI_SLAVE Periph clock and DMA1 clock */
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, DISABLE );
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_DMA1, DISABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SYSCFG, DISABLE );

    platform_mcu_powersave_enable();

    return WICED_SUCCESS;
}

wwd_result_t host_platform_spi_transfer( wwd_bus_transfer_direction_t dir, uint8_t* buffer, uint16_t buffer_length )
{
    wiced_result_t result = WWD_SUCCESS;
    uint32_t junk;
//    uint32_t loop_count;

    platform_mcu_powersave_disable();

    DMA1_Stream4->NDTR = buffer_length;
    DMA1_Stream4->M0AR = (uint32_t) buffer;
    if ( dir == BUS_READ )
    {
        DMA1_Stream3->NDTR = buffer_length;
        DMA1_Stream3->M0AR = (uint32_t) buffer;
        DMA1_Stream3->CR |= DMA_MemoryInc_Enable  | ( 1 << 4);
    }
    else
    {
        DMA1_Stream3->NDTR = buffer_length;
        DMA1_Stream3->M0AR = (uint32_t) &junk;
        DMA1_Stream3->CR &= ( ~DMA_MemoryInc_Enable ) | ( 1 << 4);
    }

    platform_gpio_output_low( &wifi_spi_pins[WWD_PIN_SPI_CS] );
    DMA_Cmd( DMA1_Stream3, ENABLE );
    DMA_Cmd( DMA1_Stream4, ENABLE );

    /* Wait for DMA TX to complete */
    result = host_rtos_get_semaphore( &spi_transfer_finished_semaphore, 100, WICED_TRUE );
//    loop_count = 0;
//    while ( ( DMA_GetFlagStatus( DMA1_Stream3, DMA_FLAG_TCIF3 ) == RESET ) && ( loop_count < (uint32_t) DMA_TIMEOUT_LOOPS ) )
//    {
//        loop_count++;
//    }

    DMA_Cmd( DMA1_Stream3, DISABLE );
    DMA_Cmd( DMA1_Stream4, DISABLE );

    /* Clear the CS pin and the DMA status flag */
    platform_gpio_output_high( &wifi_spi_pins[WWD_PIN_SPI_CS] ); /* CS high (to deselect) */
    DMA_ClearFlag( DMA1_Stream3, DMA_FLAG_TCIF4 );
    DMA_ClearFlag( DMA1_Stream4, DMA_FLAG_TCIF3 );

    platform_mcu_powersave_enable( );

    return result;
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

#ifndef WICED_DISABLE_MCU_POWERSAVE
    platform_mcu_powersave_exit_notify( );
#endif /* ifndef WICED_DISABLE_MCU_POWERSAVE */

    wwd_thread_notify_irq( );
}

WWD_RTOS_DEFINE_ISR( dma_irq )
{
    /* Clear interrupt */
    DMA1->LIFCR = (uint32_t) ( 0x3F << 22 );
    host_rtos_set_semaphore( &spi_transfer_finished_semaphore, WICED_TRUE );
}

/******************************************************
 *             IRQ Handler Mapping
 ******************************************************/

WWD_RTOS_MAP_ISR( dma_irq, DMA1_Stream3_irq )
