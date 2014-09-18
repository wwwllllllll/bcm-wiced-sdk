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
#include "platform.h"
#include "platform_init.h"
#include "platform_peripheral.h"
#include "platform_config.h"
#include "wm8533_dac.h"
#include "platform_external_memory.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define PLLI2S_N   258
#define PLLI2S_R   3

#define WICED_PLL0_FREQ_SEL0          WICED_GPIO_32
#define WICED_PLL0_FREQ_SEL1          WICED_GPIO_33

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/* AK8128C table 1: support ckout frequencies. */
typedef struct
{
    uint32_t mclk;
    uint8_t sel_hi[2];
} pll_ckout_mapping_t;

wiced_i2c_device_t wm8533_control_port =
{
    .port          = WICED_I2C_1,
    .address       = 0x1A, /* 0011010b */ //0x25,  /* 100101 */
    .address_width = I2C_ADDRESS_WIDTH_7BIT,
    .speed_mode    = I2C_STANDARD_SPEED_MODE, //I2C_LOW_SPEED_MODE,
};

wm8533_device_data_t wm8533 =
{
    /* assign a proper GPIO to a reset pin */
    .addr0          = WICED_GPIO_30,
    .cifmode        = WICED_GPIO_MAX,
    .i2c_data       = &wm8533_control_port,
    .data_port      = WICED_I2S_1,

#ifdef DAC_EXTERNAL_OSCILLATOR
    .fmt            = WM8533_FMT_CCS_CFM,
#else
    .fmt            = WM8533_FMT_CCS_CFS,
#endif
};

/******************************************************
 *               Static Function Declarations
 ******************************************************/

extern wiced_result_t wiced_wm8533_device_register( wm8533_device_data_t* device_data, const char* name );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const pll_ckout_mapping_t pll_ckout_mapping[] =
{
    { 33868800, {0, 0} },
    { 36864000, {1, 0} },
    { 12288000, {0, 1} },
    { 24576000, {1, 1} },
};

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t platform_init_audio( void )
{
    /* Register audio device */
    wm8533_device_register( &wm8533, "wm8533_dac" );

    /* XXX will be moving in re-architected API. */
    wiced_gpio_init( WICED_PLL0_FREQ_SEL0, OUTPUT_PUSH_PULL );
    wiced_gpio_init( WICED_PLL0_FREQ_SEL1, OUTPUT_PUSH_PULL );

    return WICED_SUCCESS;
}

wiced_result_t wm8533_platform_configure( wm8533_device_data_t* device_data, uint32_t mclk, uint32_t fs, uint8_t width )
{
    size_t i;
    extern const platform_i2s_t i2s_interfaces[];

    UNUSED_PARAMETER( device_data );
    UNUSED_PARAMETER( width );

    if( i2s_interfaces[WICED_I2S_1].is_master == WICED_FALSE )
    {
        for ( i = 0; i < sizeof( pll_ckout_mapping ) / sizeof( pll_ckout_mapping[0] ); i++ )
        {
            if ( mclk == pll_ckout_mapping[i].mclk )
            {
                if ( pll_ckout_mapping[i].sel_hi[0] )
                {
                    wiced_gpio_output_high( WICED_PLL0_FREQ_SEL0 );
                }
                else
                {
                    wiced_gpio_output_low( WICED_PLL0_FREQ_SEL0 );
                }
                if ( pll_ckout_mapping[i].sel_hi[1] )
                {
                    wiced_gpio_output_high( WICED_PLL0_FREQ_SEL1 );
                }
                else
                {
                    wiced_gpio_output_low( WICED_PLL0_FREQ_SEL1 );
                }
                return WICED_SUCCESS;
            }
        }
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}
