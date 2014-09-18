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

#include "wiced_rtos.h"
#include "platform_i2s.h"
#include "wiced_audio.h"
#include "wwd_assert.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

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

static wiced_audio_device_interface_t* search_audio_device( const char* device_name );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static audio_device_class_t audio_dev_class =
{
    /* By default there are no devices registered */
    .device_count = 0,
};


#define MAX_AUDIO_SESSIONS    (2)
#define DEFAULT_AUDIO_DEVICE_PERIOD_SIZE    (1024)

typedef struct
{
    uint8_t* buffer;
    uint16_t tail;
    uint16_t head;
    uint16_t count;
    uint16_t length;
} audio_buffer_t;

struct wiced_audio_session_t
{
    int i2s_id;
    wiced_bool_t i2s_running;
    wiced_semaphore_t available_periods;
    uint16_t     num_periods_requested;
    audio_buffer_t audio_buffer;
    uint16_t period_size;
    wiced_audio_device_interface_t* audio_dev;
    wiced_mutex_t session_lock;
    wiced_bool_t underrun_occurred;
};

struct wiced_audio_session_t sessions[MAX_AUDIO_SESSIONS];
extern const platform_i2s_t i2s_interfaces[];

#define MAX_AUDIO_BUFFER_SIZE       (12*1024)
uint8_t wiced_audio_buffer[MAX_AUDIO_BUFFER_SIZE];

#define lock_session(sh)        wiced_rtos_lock_mutex(&(sh)->session_lock)
#define unlock_session(sh)      wiced_rtos_unlock_mutex(&(sh)->session_lock)


#define wiced_required( X, LABEL )                                                                      \
        do                                                                                              \
        {                                                                                               \
            if( !( X ) )                                                                                \
            {                                                                                           \
                goto LABEL;                                                                             \
            }                                                                                           \
                                                                                                        \
        }   while( 0 )

#define wiced_required_action( X, LABEL, ACTION )                                                           \
        do                                                                                                  \
        {                                                                                                   \
            if( !( X ) )                                                                                    \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 0 )


/******************************************************
 *               Function Definitions
 ******************************************************/


static wiced_audio_device_interface_t* search_audio_device( const char* device_name )
{
    int i = 0;
    /* Search device in the list of already registered */
    for ( i = 0; i < audio_dev_class.device_count; i++ )
    {
        if ( strncmp( device_name, audio_dev_class.audio_devices[i].name, strlen( device_name ) ) )
        {
            continue;
        }
        else
        {
            return &audio_dev_class.audio_devices[i];
        }
    }
    return 0;
}


wiced_result_t wiced_audio_init( const char* device_name, wiced_audio_session_ref* sh, uint16_t period_size )
{
    wiced_audio_device_interface_t* audio;
    wiced_result_t result;
    int attempts = 0;
    wiced_required_action(audio_dev_class.device_count != 0, exit2, result = WICED_ERROR );

    audio = search_audio_device( device_name );
    wiced_required_action(audio != NULL, exit2, result = WICED_ERROR );

    do
    {
        result = audio->audio_device_init( audio->audio_device_driver_specific );
    } while (result != WICED_SUCCESS && ++attempts < 50 && (wiced_rtos_delay_milliseconds(1) == WICED_SUCCESS) );

    wiced_assert("Cant initialise audio device", result == WICED_SUCCESS);

    result = wiced_rtos_init_mutex(&sessions[0].session_lock);
    wiced_required_action(result == WICED_SUCCESS, exit, result = result );
    result = wiced_rtos_init_semaphore(&sessions[0].available_periods);
    wiced_required_action(result == WICED_SUCCESS, exit, result = result );

    /* Currently we will use the first one. Only one session can be run */
    *sh = &sessions[0];
    sessions[0].audio_buffer.buffer = NULL;
    sessions[0].audio_buffer.length = 0;
    sessions[0].audio_dev = audio;
    if( period_size != 0 )
    {
        sessions[0].period_size = period_size;
    }
    else
    {
        sessions[0].period_size = DEFAULT_AUDIO_DEVICE_PERIOD_SIZE;
    }
    sessions[0].underrun_occurred = WICED_FALSE;
exit:
    if(result)
    {
        result = audio->audio_device_deinit( audio->audio_device_driver_specific );
        wiced_rtos_deinit_mutex(&sessions[0].session_lock);
        wiced_rtos_deinit_semaphore(&sessions[0].available_periods);
    }
exit2:
    return result;
}

wiced_result_t wiced_audio_configure( wiced_audio_session_ref sh, wiced_audio_config_t* config )
{
    wiced_result_t result;

    UNUSED_PARAMETER( config );

    //lock_session(sh);

    /* Set the audio device to the mode required  */
    if ( sh->audio_dev->audio_device_configure )
    {
        wiced_i2s_params_t params;
        uint32_t mclk = 0;
        wiced_i2s_t i2s_id;

        /* Init audio interface, for all codecs the audio interface will be initialised similarly */
        result = sh->audio_dev->audio_device_port(sh->audio_dev->audio_device_driver_specific, &i2s_id);

        sh->i2s_id = i2s_id;
        params.period_size =    sh->period_size;
        params.sample_rate=     config->sample_rate;
        params.bits_per_sample= config->bits_per_sample;
        params.channels =       config->channels;

        result = wiced_i2s_init(sh, i2s_id, &params, &mclk);
        wiced_assert("Cant initialize audio interface\r\n", result == WICED_SUCCESS);
        wiced_required_action( result == WICED_SUCCESS, exit, result = result );

        /* configure the audio codec device */
        result = sh->audio_dev->audio_device_configure( sh->audio_dev->audio_device_driver_specific, config, &mclk );
        wiced_assert("Cant initialize audio device\r\n", result == WICED_SUCCESS);

        if( sh->audio_buffer.buffer != NULL )
        {
            result = wiced_i2s_set_audio_buffer_details(i2s_id, sh->audio_buffer.buffer, sh->audio_buffer.length);
        }
        else
        {
            /* Audio buffer has not been created yet, audio interface cant work without it */
            wiced_i2s_deinit(i2s_id);
            result = WICED_ERROR;
        }
exit:
        //unlock_session(sh);

        return result;
    }
    else
    {
        //unlock_session(sh);
        return WICED_ERROR;
    }
}


wiced_result_t wiced_audio_create_buffer( wiced_audio_session_ref sh, uint16_t size, uint8_t* buffer_ptr, void*(*allocator)(uint16_t size), wiced_bool_t is_buf_shared)
{
    wiced_result_t result;

    //lock_session(sh);
    wiced_required_action((sh->i2s_running == WICED_FALSE), exit, result = WICED_ERROR );

    /* Size of the audio buffer must be divided by the period size without the remainder */
    wiced_required_action((size % sh->period_size) == 0, exit, result = WICED_ERROR);

    if( buffer_ptr != NULL )
    {
        sh->audio_buffer.buffer = buffer_ptr;
        sh->audio_buffer.length = size;
        result = WICED_SUCCESS;
    }
    else if( allocator != NULL )
    {
        uint8_t* buf_ptr_tmp;
        buf_ptr_tmp = (uint8_t*)allocator(size);
        wiced_required_action(buf_ptr_tmp != NULL, exit, result = WICED_ERROR);
        sh->audio_buffer.buffer = buf_ptr_tmp;
        sh->audio_buffer.length = size;
        result = WICED_SUCCESS;
    }
    else if ( is_buf_shared == WICED_TRUE )
    {
        WPRINT_LIB_INFO( ( "using shared buffer for player\n" ) );
        memset(&wiced_audio_buffer, 0, sizeof(wiced_audio_buffer));
        sh->audio_buffer.buffer = wiced_audio_buffer;
        sh->audio_buffer.length = size;
        result = WICED_SUCCESS;
    }
    else
    {
        result = WICED_ERROR;
        goto exit;
    }

exit:
    sh->audio_buffer.head = sh->audio_buffer.tail = 0;
    sh->audio_buffer.count = 0;
    //unlock_session(sh);

    return result;
}


wiced_result_t wiced_audio_set_volume( wiced_audio_session_ref sh, double volume_in_db )
{
    wiced_result_t result;

    UNUSED_PARAMETER( volume_in_db );

    result = sh->audio_dev->audio_device_set_volume( sh->audio_dev->audio_device_driver_specific, volume_in_db );
    if ( result != WICED_SUCCESS )
    {
        /* Do some error handling... */
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_audio_deinit( wiced_audio_session_ref sh )
{
    wiced_result_t result;

    /* Stop audio device. Initiate power-down sequence on the audio device */
    result = sh->audio_dev->audio_device_deinit( sh->audio_dev->audio_device_driver_specific );
    result = wiced_i2s_deinit (sh->i2s_id);

    //wiced_rtos_deinit_mutex(&sh->session_lock);
    //wiced_rtos_deinit_semaphore(&sh->available_periods);

    if ( result != WICED_SUCCESS )
    {
        /* Do some error handling... */
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

wiced_result_t wiced_register_audio_device( const char* name, wiced_audio_device_interface_t* interface )
{
    static wiced_bool_t first_device = WICED_FALSE;

    UNUSED_PARAMETER(name);

    /* Add interface to the first empty slot */
    if ( first_device == WICED_FALSE )
    {
        int i = 0;

        for ( i = 0; i < MAX_NUM_AUDIO_DEVICES; i++ )
        {
            memset( &audio_dev_class.audio_devices[i], 0x00, sizeof(wiced_audio_device_interface_t) );
        }

        audio_dev_class.device_count = 0;
    }

    memcpy( &audio_dev_class.audio_devices[audio_dev_class.device_count++], interface, sizeof( *interface ) );

    return WICED_SUCCESS;
}

wiced_result_t wiced_audio_get_buffer( wiced_audio_session_ref sh, uint8_t** ptr, uint16_t* size)
{
    wiced_result_t result;

    *ptr = NULL;

    /* If Underrun situation occurs during playback - let the application know about it,
     *  hence application will be able to do a recovery when it happens */
    wiced_required_action( sh->underrun_occurred == WICED_FALSE, exit, result = WICED_ERROR);
    __asm("CPSID i");
    //lock_session(sh);
    wiced_required_action( (sh->audio_buffer.length - sh->audio_buffer.count) > 0, exit, result = WICED_WWD_BUFFER_UNAVAILABLE_TEMPORARY);

    wiced_assert("size must be a non-zero value", (*size > 0));
    wiced_required_action( *size != 0, exit, result = WICED_BADARG );

    if ( (sh->audio_buffer.head + *size) > sh->audio_buffer.length)
    {
        if( sh->audio_buffer.head == sh->audio_buffer.length )
        {
            /* If head points right to the edge of the audio buffer, give everything from the beginning of the buffer */
            *ptr = &sh->audio_buffer.buffer[0];

            /* Cap the requested size */
            *size = (uint16_t)(sh->audio_buffer.length - sh->audio_buffer.count);
        }
        else
        {
            /* Give everything which is below the top edge of the buffer */
            *ptr = &sh->audio_buffer.buffer[sh->audio_buffer.head];

            /* Cap the requested size */
            *size = (uint16_t)(sh->audio_buffer.length - sh->audio_buffer.head);
        }
    }
    else if( (sh->audio_buffer.head + *size) == sh->audio_buffer.length )
    {
        *ptr = &sh->audio_buffer.buffer[sh->audio_buffer.head];
        *size = (uint16_t)(sh->audio_buffer.length - sh->audio_buffer.head);
    }
    else /* (sh->audio_buffer.head + *size) < sh->audio_buffer.length */
    {
        /* Do not update size since audio buffer has enough free space left */
        *ptr = &sh->audio_buffer.buffer[sh->audio_buffer.head];
    }

    result = WICED_SUCCESS;
exit:
    __asm("CPSIE i");
    //unlock_session(sh);
    return result;
}


wiced_result_t wiced_audio_stop( wiced_audio_session_ref sh )
{
    wiced_result_t result = WICED_SUCCESS;
    if( sh->i2s_running == WICED_TRUE )
    {
        result = wiced_i2s_stop(sh->i2s_id);
        wiced_required_action( result == WICED_SUCCESS, exit, result = result );
        sh->i2s_running = WICED_FALSE;
    }
    sh->audio_buffer.head = sh->audio_buffer.tail = 0;
    sh->audio_buffer.count = 0;
    sh->underrun_occurred = WICED_FALSE;
    sh->num_periods_requested = 0;

    result = wiced_rtos_deinit_semaphore(&sh->available_periods);
    result = wiced_rtos_init_semaphore(&sh->available_periods);
exit:
    return result;
}


wiced_result_t wiced_audio_start( wiced_audio_session_ref sh )
{
    wiced_result_t result;
    if( sh->i2s_running == WICED_FALSE )
    {
        if( sh->audio_buffer.count/sh->period_size >=1 )
        {
            if( sh->audio_dev->audio_device_start_streaming != NULL )
            {
                result = sh->audio_dev->audio_device_start_streaming( sh->audio_dev->audio_device_driver_specific );
            }
            result = wiced_i2s_start(sh->i2s_id);
            if( result == WICED_SUCCESS )
            {
                sh->i2s_running = WICED_TRUE;
                return WICED_SUCCESS;
            }
            else
            {
                return result;
            }
        }
        else
        {
            return WICED_ERROR;
        }
    }
    else
    {
        return WICED_ERROR;
    }
}

wiced_result_t wiced_audio_release_buffer(wiced_audio_session_ref sh, uint16_t size)
{
    wiced_result_t result;
    uint16_t spare;

    /* Make sure that an application knows about "underrun" situation ,
     * so that it will be able to do a recovery when it happens */
    wiced_required_action( sh->underrun_occurred == WICED_FALSE, exit, result = WICED_ERROR );
    //lock_session(sh);
    spare = sh->audio_buffer.length - sh->audio_buffer.count;

    wiced_required_action( spare >= size, exit, result = WICED_WWD_BUFFER_UNAVAILABLE_TEMPORARY );
    __asm("CPSID i");
    wiced_required_action( ( (sh->audio_buffer.count + size) <= sh->audio_buffer.length ), exit, result = WICED_WWD_BUFFER_UNAVAILABLE_TEMPORARY) ;

    sh->audio_buffer.head = (uint16_t)(( sh->audio_buffer.head + size ) % ( sh->audio_buffer.length ));
    sh->audio_buffer.count = (uint16_t)( sh->audio_buffer.count + size);

    __asm("CPSIE i");
    result = WICED_SUCCESS;
exit:
    if( result != WICED_SUCCESS )
    {
        __asm("CPSIE i");
    }
    //unlock_session(sh);

    return result;
}


wiced_result_t wiced_audio_buffer_platform_event(wiced_audio_session_ref sh, wiced_audio_platform_event_t event)
{
    switch(event)
    {
        case WICED_AUDIO_PERIOD_ELAPSED:
            sh->audio_buffer.tail  = (uint16_t)((sh->audio_buffer.tail + sh->period_size) % (sh->audio_buffer.length));
            sh->audio_buffer.count = (uint16_t)(sh->audio_buffer.count- sh->period_size);
            if( sh->num_periods_requested > 0 )
            {
                sh->num_periods_requested--;
                if( sh->num_periods_requested == 0 )
                {
                    wiced_rtos_set_semaphore(&sh->available_periods);
                }
            }
        break;

        case WICED_AUDIO_UNDERRUN:
            sh->underrun_occurred = WICED_TRUE;
        break;

        default:
            break;
    }
    return WICED_SUCCESS;
}



uint16_t wiced_audio_buffer_platform_get_periods(wiced_audio_session_ref sh)
{
    if( sh->audio_buffer.count >= sh->period_size )
    {
        return sh->audio_buffer.count / sh->period_size;
    }
    else
    {
        return 0;
    }
}


wiced_result_t wiced_audio_get_current_buffer_weight( wiced_audio_session_ref sh, uint32_t* weight )
{
    __asm("CPSID i");

    /* Return how many bytes are in the buffer already */
    *weight = sh->audio_buffer.count;
    __asm("CPSIE i");

    return WICED_SUCCESS;
}

/* wait till platform moves in the buffer to the position where the requested size gets available */
wiced_result_t wiced_audio_wait_buffer( wiced_audio_session_ref sh, uint16_t size, uint32_t timeout )
{
    wiced_result_t result;
    uint16_t spare;


    //lock_session(sh);
    __asm("CPSID i");

    /* Make sure that application knows about "underrun" condition,
     * so that it will be able to do a recovery when it happens */
    wiced_required_action( sh->underrun_occurred == WICED_FALSE, exit, result = WICED_ERROR);

    spare = (uint16_t)(sh->audio_buffer.length - sh->audio_buffer.count);

    /* Don't wait if we have enough bytes in the audio buffer already */
    wiced_required_action( spare < size, exit, result = WICED_SUCCESS );
    sh->num_periods_requested = size / sh->period_size;
    if( (size % sh->period_size) > 0 )
    {
        sh->num_periods_requested++;
    }

    __asm("CPSIE i");
    result = wiced_rtos_get_semaphore( &sh->available_periods, timeout);
    if( result == WICED_SUCCESS )
    {
        wiced_assert("requested periods number must be decremented by the audio interface down to 0", (sh->num_periods_requested == 0));
        if( sh->num_periods_requested != 0 )
        {
            __asm("bkpt");
        }
    }

    return result;

exit:
    __asm("CPSIE i");
    //unlock_session(sh);
    return result;
}

