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
 *
 *This implemenration is not complete.
 */
#include <stdint.h>
#include <string.h>
#include "platform_cmsis.h"
#include "platform_constants.h"
#include "platform_peripheral.h"
#include "platform_isr_interface.h"
#include "wwd_assert.h"

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

void SPI_Init(LPC_SPI_T *pSPI, platform_spi_peripheral_config_t* spi_config);

/******************************************************
 *                   Variables
 ******************************************************/

/* SPI peripheral clock functions */
static const platform_peripheral_clock_function_t spi_peripheral_clock_functions[NUMBER_OF_SPI_PORTS] =
{
    [0] = Chip_Clock_Enable,
};

/* SPI peripheral clocks */
static const uint32_t spi_peripheral_clocks[NUMBER_OF_SPI_PORTS] =
{
    [0] = CLK_SPI,
};

/******************************************************
 *               Function Definitions
 ******************************************************/

uint8_t platform_spi_get_port_number( platform_spi_port_t* spi )
{
    UNUSED_PARAMETER (spi );
    return 0;
}

platform_result_t platform_spi_init( const platform_spi_t* spi, const platform_spi_config_t* config )
{
    /* TODO: unimplemented */
    UNUSED_PARAMETER( spi );
    UNUSED_PARAMETER( config );

    return WICED_SUCCESS;
}

void SPI_Init(LPC_SPI_T *pSPI, platform_spi_peripheral_config_t* spi_config)
{
    UNUSED_PARAMETER( spi_config );
}

platform_result_t platform_spi_deinit( const platform_spi_t* spi )
{
    UNUSED_PARAMETER( spi );
    /* TODO: unimplemented */
    return PLATFORM_UNSUPPORTED;
}

platform_result_t platform_spi_transfer( const platform_spi_t* spi, const platform_spi_config_t* config, const platform_spi_message_segment_t* segments, uint16_t number_of_segments )
{
    platform_result_t result = PLATFORM_SUCCESS;
    /* TODO: unimplemented */
    UNUSED_PARAMETER( spi );
    UNUSED_PARAMETER( number_of_segments );
    UNUSED_PARAMETER( config );
    UNUSED_PARAMETER( segments );

    return result;
}

