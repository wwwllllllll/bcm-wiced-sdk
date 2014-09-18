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

#include "thermistor.h"
#include "wiced_platform.h"
#include "math.h"

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

wiced_result_t thermistor_take_sample(wiced_adc_t adc, uint16_t* sample_value)
{
    wiced_result_t result = wiced_adc_take_sample(adc, sample_value);

    /* Thermistor is Murata NCP18XH103J03RB  (Digi-key 490-2436-1-ND )
     *
     * Part Number details:
     * NC  : NTC Chip Thermistor
     * P   : Plated termination
     * 18  : Size 0603
     * XH  : Temperature Characteristics : Nominal B-Constant 3350-3399K
     * 103 : Resistance 10k
     * J   : Tolerance   +/- 5%
     * 03  : Individual Specs: Standard
     * RB  : Paper Tape 4mm pitch, 4000pcs
     *
     *
     * It has a 43K feed resistor from 3V3
     *
     * Thermistor Voltage    = V_supply * ADC_value / 4096
     * Thermistor Resistance = R_feed / ( ( V_supply / V_thermistor ) - 1 )
     * Temp in kelvin = 1 / ( ( ln( R_thermistor / R_0 ) / B ) + 1 / T_0 )
     * Where: R_feed = 43k, V_supply = 3V3, R_0 = 10k, B = 3375, T_0 = 298.15°K (25°C)
     */
    if (result == WICED_SUCCESS)
    {
        double thermistor_resistance = 43000.0 / ( ( 4096.0 / (double) *sample_value ) - 1 );
        double logval = log( thermistor_resistance / 10000.0 );
        double temperature = 1.0 / ( logval / 3380.0 + 1.0 / 298.15 ) - 273.15;

        *sample_value = (uint16_t)(temperature*10);
        return WICED_SUCCESS;
    }
    else
    {
        *sample_value = 0;
        return result;
    }
}
