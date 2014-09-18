/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "platform.h"
#include "wiced_constants.h"
#include <stdio.h>

struct breakout_pins_setting_t
{
    uint16_t          breakout_pin_number;
    int               useful; /**< non-zero if we can actually use this pin for output, else zero */

    /** The below values are meaningless if useful is false/zero */
    GPIO_TypeDef *    port;
    uint16_t          pin;
    GPIOMode_TypeDef  type;
};

const struct breakout_pins_setting_t const breakout_gpio_settings[] =
{
    {  1, WICED_FALSE, GPIOA, GPIO_Pin_4,  GPIO_Mode_AIN    }, /* ignored */
    {  2, WICED_FALSE, GPIOA, GPIO_Pin_4,  GPIO_Mode_AIN    }, /* ignored */
    {  3, WICED_TRUE,  GPIOA, GPIO_Pin_1,  GPIO_Mode_Out_PP },
    {  4, WICED_TRUE,  GPIOA, GPIO_Pin_2,  GPIO_Mode_Out_PP },
    {  5, WICED_TRUE,  GPIOA, GPIO_Pin_3,  GPIO_Mode_Out_PP },
    {  6, WICED_TRUE,  GPIOB, GPIO_Pin_11, GPIO_Mode_Out_PP },
    {  7, WICED_TRUE,  GPIOB, GPIO_Pin_12, GPIO_Mode_Out_PP },
    {  8, WICED_FALSE, GPIOA, GPIO_Pin_4,  GPIO_Mode_AIN    }, /* ignored */
    {  9, WICED_FALSE, GPIOA, GPIO_Pin_0,  GPIO_Mode_AIN    }, /* ignored */
    { 10, WICED_FALSE, GPIOA, GPIO_Pin_4,  GPIO_Mode_AIN    }, /* ignored */
    { 11, WICED_FALSE, GPIOA, GPIO_Pin_9,  GPIO_Mode_AIN    }, /* ignored */
    { 12, WICED_FALSE, GPIOA, GPIO_Pin_10, GPIO_Mode_AIN    }, /* ignored */
    { 13, WICED_FALSE, GPIOB, GPIO_Pin_15, GPIO_Mode_AIN    }, /* ignored */
    { 14, WICED_FALSE, GPIOB, GPIO_Pin_14, GPIO_Mode_AIN    }, /* ignored */
    { 15, WICED_FALSE, GPIOB, GPIO_Pin_13, GPIO_Mode_AIN    }, /* ignored */
    { 16, WICED_FALSE, GPIOB, GPIO_Pin_12, GPIO_Mode_AIN    }, /* ignored */
    { 17, WICED_FALSE, GPIOA, GPIO_Pin_4,  GPIO_Mode_AIN    }, /* ignored */
    { 18, WICED_FALSE, GPIOA, GPIO_Pin_4,  GPIO_Mode_AIN    }  /* ignored */
};

const unsigned int breakout_settings_size = ( sizeof( breakout_gpio_settings ) / sizeof( struct breakout_pins_setting_t ) );
const unsigned int sizeof_breakout_pins_setting_t = sizeof( struct breakout_pins_setting_t );


inline uint16_t breakout_pins_get_pin_number( struct breakout_pins_setting_t *settings )
{
    return ( settings->breakout_pin_number );
}

inline int breakout_pins_gpio_init( struct breakout_pins_setting_t *breakout_pin )
{
    GPIO_InitTypeDef gpio_init_structure;

    /* If we can't utilize this pin for output then there is no point setting it up... */
    if ( !breakout_pin->useful )
    {
        return 0;
    }

    gpio_init_structure.GPIO_Pin   = breakout_pin->pin;
    gpio_init_structure.GPIO_Speed = ( breakout_pin->type == GPIO_Mode_AIN ) ? (GPIOSpeed_TypeDef) 0 : GPIO_Speed_50MHz;
    gpio_init_structure.GPIO_Mode  = breakout_pin->type;
    GPIO_Init( breakout_pin->port, &gpio_init_structure );
    return 1;
}

inline void breakout_pins_gpio_setbits( struct breakout_pins_setting_t *breakout_pin )
{
    GPIO_SetBits( breakout_pin->port, breakout_pin->pin );
}

inline void breakout_pins_gpio_resetbits( struct breakout_pins_setting_t *breakout_pin )
{
    GPIO_ResetBits( breakout_pin->port, breakout_pin->pin );
}
