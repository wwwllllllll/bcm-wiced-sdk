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

#include "xively.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @cond */
/******************************************************
 *                     Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define THERMISTOR_ADC                (WICED_THERMISTOR_JOINS_ADC)  /* Handle for Thermistor                                                                      */
#define SETPOINT_UP_KEY_GPIO          (WICED_BUTTON1)               /* Handle for Set Point UP key (button) GPIO                                                  */
#define SETPOINT_DOWN_KEY_GPIO        (WICED_BUTTON2)               /* Handle for Set Point DOWN key (button) GPIO                                                */
#define SETPOINT_UP_KEY_CODE          (1)                           /* Set Point UP key code                                                                      */
#define SETPOINT_DOWN_KEY_CODE        (2)                           /* Set Point DOWN key code                                                                    */
#define SETPOINT_UP_LED               (WICED_LED1_JOINS_PWM)        /* Handle for Set Point LED 1                                                                 */
#define SETPOINT_DOWN_LED             (WICED_LED2_JOINS_PWM)        /* Handle for Set Point LED 2                                                                 */
#define SETPOINT_LED_PWM_FREQ_HZ      (2000)                        /* Set Point LED pulse-width-modulation (PWM) frequency in Hz                                 */
#define DEFAULT_SETPOINT              (25.0f)                       /* Default Set Point (in degree C) when the application starts                                */
#define MAX_SETPOINT                  (35.0f)                       /* Maximum Set Point in degree C                                                              */
#define MIN_SETPOINT                  (15.0f)                       /* Minimum Set Point in degree C                                                              */
#define SETPOINT_INCREMENT            (1.0f)                        /* Set point increment in degree C                                                            */
#define MAX_TEMPERATURE_READINGS      (64)                          /* Maximum records of temperature readings to be fed to Xively. This needs to be an order of 2  */

#define LED_BRIGHTNESS_CONST_A        (0.08f)                       /* Set Point LED brightness equation: a * b ^ (brightness_level + c) ...                      */
#define LED_BRIGHTNESS_CONST_B        (1.75f)                       /* Constants have been set for the equations to produce distinctive brightness levels         */
#define LED_BRIGHTNESS_CONST_C        (2.00f)
#define LED_BRIGHTNESS_EQUATION(level) (LED_BRIGHTNESS_CONST_A * pow(LED_BRIGHTNESS_CONST_B, level + LED_BRIGHTNESS_CONST_C))


/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    CONFIGURE_NETWORK_USING_WPS,
    CONFIGURE_NETWORK_USING_AP_MODE,
    CONFIGURE_NETWORK_USING_P2P,
} network_configuration_method_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

#pragma pack(1)
typedef struct
{
    wiced_iso8601_time_t timestamp;
    char                 sample[4];
} temperature_reading_t;

#pragma pack()

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/** @endcond */
#ifdef __cplusplus
} /*extern "C" */
#endif

