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
 * GPIO API Application
 *
 * This application demonstrates how to use the WICED GPIO API
 * to toggle LEDs and read button states
 *
 * Features demonstrated
 *  - GPIO API
 *
 */

#include "wiced.h"

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

void application_start( )
{
    wiced_bool_t led1 = WICED_FALSE;
    wiced_bool_t led2 = WICED_FALSE;
    wiced_bool_t button1_pressed;
    wiced_bool_t button2_pressed;

    /* Initialise the WICED device */
    wiced_init();

    WPRINT_APP_INFO( ( "The LEDs are flashing. Holding a button will force the corresponding LED on\n" ) );

    while ( 1 )
    {
        /* Read the state of Button 1 */
        button1_pressed = wiced_gpio_input_get( WICED_BUTTON1 ) ? WICED_FALSE : WICED_TRUE;  /* The button has inverse logic */

        if ( button1_pressed == WICED_TRUE )
        {   /* Turn LED1 on */
            wiced_gpio_output_high( WICED_LED1 );
        }
        else
        {   /* Toggle LED1 */
            if ( led1 == WICED_TRUE )
            {
                wiced_gpio_output_low( WICED_LED1 );
                led1 = WICED_FALSE;
            }
            else
            {
                wiced_gpio_output_high( WICED_LED1 );
                led1 = WICED_TRUE;
            }
        }

        /* Read the state of Button 2 */
        button2_pressed = wiced_gpio_input_get( WICED_BUTTON2 ) ? WICED_FALSE : WICED_TRUE;  /* The button has inverse logic */

        if ( button2_pressed == WICED_TRUE )
        {   /* Turn LED2 on */
            wiced_gpio_output_high( WICED_LED2 );
        }
        else
        {   /* Toggle LED2 */
            if ( led2 == WICED_TRUE )
            {
                wiced_gpio_output_low( WICED_LED2 );
                led2 = WICED_FALSE;
            }
            else
            {
                wiced_gpio_output_high( WICED_LED2 );
                led2 = WICED_TRUE;
            }
        }

        wiced_rtos_delay_milliseconds( 250 );
    }
}
