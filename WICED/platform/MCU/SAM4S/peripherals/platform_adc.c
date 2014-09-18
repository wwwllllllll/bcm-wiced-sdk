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
 */

#include "stdint.h"
#include "string.h"
#include "platform_peripheral.h"
#include "platform_isr.h"
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

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

platform_result_t platform_adc_init( const platform_adc_t* adc, uint32_t sample_cycle )
{
    UNUSED_PARAMETER(adc);
    UNUSED_PARAMETER(sample_cycle);

    platform_mcu_powersave_disable();

    sysclk_enable_peripheral_clock( adc->peripheral_id );
    ioport_set_pin_mode( adc->adc_pin->pin, 0 );
    ioport_set_pin_dir ( adc->adc_pin->pin, IOPORT_DIR_INPUT );

    adc_init( adc->peripheral, CPU_CLOCK_HZ, adc->adc_clock_hz, 8 );

    /* Maximum track time is 16 cycles */
    if ( sample_cycle > 16 )
    {
        sample_cycle = 16;
    }

    /* Tracking time = TRACKTIM + 1 */
    sample_cycle--;
    adc_configure_timing( adc->peripheral, sample_cycle, adc->settling_time, 1 );

    adc_set_resolution( adc->peripheral, adc->resolution );

    adc_enable_channel( adc->peripheral, adc->channel );

    adc_configure_trigger( adc->peripheral, adc->trigger, 0 );

    platform_mcu_powersave_enable();


    return PLATFORM_SUCCESS;
}

platform_result_t platform_adc_deinit( const platform_adc_t* adc )
{
    UNUSED_PARAMETER(adc);

    platform_mcu_powersave_disable();

    adc_disable_channel( adc->peripheral, adc->channel );
    platform_gpio_deinit( adc->adc_pin );

    platform_mcu_powersave_enable();

    return PLATFORM_SUCCESS;
}

platform_result_t platform_adc_take_sample( const platform_adc_t* adc, uint16_t* output )
{

    uint32_t temp;

    adc_start( adc->peripheral );

    while ( ( adc_get_status( adc->peripheral ) & ADC_ISR_DRDY ) == 0 )
    {
    }

    temp = adc_get_latest_value( ADC );

    *output = (uint16_t)( temp & 0xffffffffU ) ;

    return PLATFORM_SUCCESS;
}

platform_result_t platform_adc_take_sample_stream( const platform_adc_t* adc, void* buffer, uint16_t buffer_length )
{
    UNUSED_PARAMETER( adc );
    UNUSED_PARAMETER( buffer );
    UNUSED_PARAMETER( buffer_length );
    wiced_assert( "unimplemented", 0!=0 );
    return PLATFORM_SUCCESS;
}
