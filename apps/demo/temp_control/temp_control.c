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
 * Temperature Control & Report Application
 *
 * This application measures the temperature of the WICED evaluation
 * board and sends temperature measurements to the 'Internet of Things' data
 * warehouse http://xively.com/. The app also displays temperature & setpoint
 * status on a local webpage.
 *
 * The application demonstrates the following features ...
 *  - Wi-Fi client mode to send temperature measurements to the Internet
 *  - DNS redirect
 *  - Webserver
 *  - Gedday mDNS / DNS-SD Network Discovery
 *  - GPIO interface to capture button inputs
 *  - Timer peripheral interface to control LED brightness (via PWM)
 *  - Factory Programming - please read WICED-AN8xx-R Factory Programming Application Note
 *
 * [TBD : The application will eventually be configured to control the temperature of
 *        a thermistor on a personality board plugged into the WICED evaluation board.
 *        At present, the temperature and setpoint are independent. There is no feedback
 *        mechanism in place to heat or cool the thermistor (to make the board temperature
 *        equal to the setpoint).]
 *
 * Device Configuration
 *    The application is configured to use the Wi-Fi configuration
 *    from the local wifi_config_dct.h file. Application configuration is taken
 *    from the temp_control_dct.c file and stored in the Device Configuration table (DCT).
 *
 * Xively Setup (must be completed before temperature reporting to Xively will work)
 *    1. Go to http://xively.com and create an account
 *
 *    2. After logging in, various views are available by clicking
 *       your username in the top right corner of the window
 *
 *    3. Select the 'Develop' tab
 *       a. Click 'Add Device' to add a new feed
 *       b. Enter device information as desired and click 'Add Device'
 *       c. Note down the Feed ID and auto-generated API key that were created
 *
 *    4. Click the 'Add Channel' button
 *       a. Enter the desired channel ID
 *       b. Complete other fields as desired and click 'Save Channel'
 *
 *    5. In the <WICED-SDK>/apps/demo/temp_control directory, add the
 *       feed ID, API key, and channel ID into the temp control DCT config file : temp_control_dct.c
 *
 *    6. Compile and run the temp_control application
 *
 *    7. View the temperature log at https://xively.com/feeds/<YOUR_FEED_ID>
 *
 * Application Operation
 * This section provides a description of the application flow and usage.
 * The app runs in a thread, the entry point is application_start()
 *
 *    Startup
 *      - Initialise the device
 *      - Check the device has a valid configuration in the DCT
 *      - Setup peripherals including buttons, LEDs, ADC and timers
 *      - Start the network interface to connect the device to the network
 *      - Set the local time from a time server on the internet
 *      - Setup a timer to take temperature measurements & send data to Xively
 *      - Start a webserver to display temperature & setpoint values
 *      - Start Gedday to advertise the webserver on the network
 *
 *    Usage
 *        Two buttons on the eval board are used to increase or decrease
 *        the temperature setpoint. The D1 & D2 LEDs on the eval board change
 *        in brightness according to the temperature setpoint.
 *
 *        The current time, date, board temperature and setpoint are published to
 *        a webpage by a local webserver. The setpoint can also be changed using
 *        buttons on the webpage.
 *
 *        A mDNS (or Bonjour) browser may be used to find the webpage, or alternately, the
 *        IP address of the device (which is printed to the UART) may be entered
 *        into a web browser.
 *
 *        As described above, the temperature of the eval board can be viewed
 *        online at https://xively.com/feeds/<YOUR_FEED_ID>
 *
 */


#include "temp_control.h"
#include "temp_control_dct.h"
#include <math.h>
#include "wiced.h"
#include "thermistor.h" /* Using Murata NCP18XH103J03RB thermistor */
#include "http_server.h"
#include "sntp.h"
#include "gedday.h"
#include "gpio_keypad.h"
#include "resources.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define MAX_NTP_ATTEMPTS     (3)

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct
{
    float         temperature;
    wiced_mutex_t mutex;
} setpoint_t;

typedef struct
{
    temperature_reading_t temperature_readings[MAX_TEMPERATURE_READINGS];
    uint16_t              temperature_reading_index;
    uint16_t              last_sent_temperature_index;
    uint16_t              last_sample;
    wiced_mutex_t         mutex;
} temp_data_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static wiced_result_t take_temperature_reading       ( void* arg );
static wiced_result_t send_data_to_xively            ( void* arg );
static float          get_setpoint                   ( void );
static void           increase_setpoint              ( void );
static void           decrease_setpoint              ( void );
static void           adjust_setpoint_led_brightness ( void );
static void           setpoint_control_keypad_handler( gpio_key_code_t code, gpio_key_event_t event );
static int32_t        process_temperature_update     ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );
static int32_t        process_temperature_up         ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );
static int32_t        process_temperature_down       ( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static temp_data_t         xively_data;
static setpoint_t          setpoint;
static wiced_http_server_t http_server;
static wiced_timed_event_t temperature_timed_event;
static wiced_timed_event_t xively_timed_event;
static gpio_keypad_t       setpoint_control_keypad;

static const configuration_entry_t const app_config[] =
{
    {"Xively feed ID",    DCT_OFFSET(user_dct_data_t, xively_feed_id),    11, CONFIG_STRING_DATA },
    {"Xively API key",    DCT_OFFSET(user_dct_data_t, xively_api_key),    49, CONFIG_STRING_DATA },
    {"Xively Channel ID", DCT_OFFSET(user_dct_data_t, xively_channel_id), 51, CONFIG_STRING_DATA },
    {"Sample interval (ms)",   DCT_OFFSET(user_dct_data_t, sample_interval),   4 , CONFIG_UINT32_DATA },
    {0,0,0,0}
};

static START_OF_HTTP_PAGE_DATABASE(web_pages)
    ROOT_HTTP_PAGE_REDIRECT("/apps/temp_control/main.html"),
    { "/apps/temp_control/main.html",    "text/html",                WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_apps_DIR_temp_control_DIR_main_html, },
    { "/temp_report.html",               "text/html",                WICED_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {process_temperature_update, 0 }, },
    { "/temp_up",                        "text/html",                WICED_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {process_temperature_up, 0 }, },
    { "/temp_down",                      "text/html",                WICED_DYNAMIC_URL_CONTENT,   .url_content.dynamic_data   = {process_temperature_down, 0 }, },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon", WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_favicon_ico, },
    { "/scripts/general_ajax_script.js", "application/javascript",   WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_scripts_DIR_general_ajax_script_js, },
    { "/images/brcmlogo.png",            "image/png",                WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_brcmlogo_png, },
    { "/images/brcmlogo_line.png",       "image/png",                WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_images_DIR_brcmlogo_line_png, },
    { "/styles/buttons.css",             "text/css",                 WICED_RESOURCE_URL_CONTENT,  .url_content.resource_data  = &resources_styles_DIR_buttons_css, },
END_OF_HTTP_PAGE_DATABASE();

static const gpio_key_t setpoint_control_key_list[] =
{
    {
        .gpio     = SETPOINT_UP_KEY_GPIO,
        .polarity = KEY_POLARITY_HIGH,
        .code     = SETPOINT_UP_KEY_CODE,
    },
    {
        .gpio     = SETPOINT_DOWN_KEY_GPIO,
        .polarity = KEY_POLARITY_HIGH,
        .code     = SETPOINT_DOWN_KEY_CODE,
    },
};

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( void )
{
    user_dct_data_t* dct;
    uint32_t sample_interval;

    wiced_init( );

    /* Configure device */
    wiced_configure_device( app_config );

    /* Initialise Xively data. Xively data is shared among multiple threads; therefore a mutex is required */
    memset( &xively_data, 0, sizeof( xively_data ) );
    wiced_rtos_init_mutex( &xively_data.mutex );

    /* Initialise temperature set point. Set point is shared among multiple threads; therefore a mutex is required */
    wiced_rtos_init_mutex( &setpoint.mutex );
    setpoint.temperature = DEFAULT_SETPOINT;
    adjust_setpoint_led_brightness( );

    /* Initialise Thermistor */
    wiced_adc_init( THERMISTOR_ADC, 5 );

    /* Initialise Set Point Control keypad and assigns it's callback function to run on hardware_io_worker_thread's context */
    gpio_keypad_enable( &setpoint_control_keypad, WICED_HARDWARE_IO_WORKER_THREAD, setpoint_control_keypad_handler, 250, 2, setpoint_control_key_list );

    /* Disable roaming to other access points */
    wiced_wifi_set_roam_trigger( -99 ); /* -99dBm ie. extremely low signal level */

    /* Bringup the network interface */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Timestamp is needed for sending data to Xively.
     * Start automatic time synchronisation and synchronise once every day.
     */
    sntp_start_auto_time_sync( 1 * DAYS );

    wiced_dct_read_lock( (void**) &dct, WICED_FALSE, DCT_APP_SECTION, 0, sizeof(user_dct_data_t) );
    sample_interval = dct->sample_interval;
    wiced_dct_read_unlock( dct, WICED_FALSE );

    /* Setup timed events that will take a measurement & activate Xively thread to send measurements to Xively */
    wiced_rtos_register_timed_event( &xively_timed_event,      WICED_NETWORKING_WORKER_THREAD,  send_data_to_xively,     10 * SECONDS, 0 );
    wiced_rtos_register_timed_event( &temperature_timed_event, WICED_HARDWARE_IO_WORKER_THREAD, take_temperature_reading, sample_interval, 0 );

    /* Start web server to display current temperature & setpoint */
    wiced_http_server_start( &http_server, 80, web_pages, WICED_STA_INTERFACE );

    /* Advertise webpage services using Gedday */
    gedday_init( WICED_STA_INTERFACE, "wiced-temp-controller" );
    gedday_add_service( "temp_control web server", "_http._tcp.local", 80, 300, "" );
}

/*
 * Takes a temperature reading
 */
static wiced_result_t take_temperature_reading( void* arg )
{
    UNUSED_PARAMETER(arg);

    wiced_rtos_lock_mutex( &xively_data.mutex );

    /* Take thermistor reading */
    if ( thermistor_take_sample( THERMISTOR_ADC, &xively_data.last_sample ) != WICED_SUCCESS )
    {
        wiced_rtos_unlock_mutex( &xively_data.mutex );
        return WICED_ERROR;
    }

    /* Get the current ISO8601 time */
    wiced_time_get_iso8601_time( &xively_data.temperature_readings[xively_data.temperature_reading_index].timestamp );

    /* Create sample string */
    xively_u16toa( xively_data.last_sample / 10, xively_data.temperature_readings[xively_data.temperature_reading_index].sample, 2 );
    xively_data.temperature_readings[xively_data.temperature_reading_index].sample[2] = '.';
    xively_u16toa( xively_data.last_sample % 10, &xively_data.temperature_readings[xively_data.temperature_reading_index].sample[3], 1 );

    if ( ( ++xively_data.temperature_reading_index ) == MAX_TEMPERATURE_READINGS )
    {
        xively_data.temperature_reading_index = 0;
    }

    wiced_rtos_unlock_mutex( &xively_data.mutex );

    return WICED_SUCCESS;
}

/*
 * Sends temperature readings to Xively
 */
static wiced_result_t send_data_to_xively( void* arg )
{
    xively_feed_t       sensor_feed;
    xively_datastream_t stream;
    uint16_t            data_points;
    wiced_result_t      result;

    /* Setup sensor feed info */
    memset( &sensor_feed, 0, sizeof( sensor_feed ) );
    user_dct_data_t* dct;

    wiced_dct_read_lock( (void**) &dct, WICED_FALSE, DCT_APP_SECTION, 0, sizeof(user_dct_data_t) );

    if ( ( dct->xively_feed_id[0]    == '\x00' ) ||
         ( dct->xively_channel_id[0] == '\x00' ) ||
         ( dct->xively_api_key[0]    == '\x00' ) )
    {
        WPRINT_APP_INFO(("Xively feed details not in DCT\n"));
        wiced_dct_read_unlock( dct, WICED_FALSE );
        return WICED_ERROR;
    }

    sensor_feed.id       = dct->xively_feed_id;
    sensor_feed.api_key  = dct->xively_api_key;

    /* Open Xively feed */
    result = xively_open_feed( &sensor_feed );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Failed to open Xively feed\n"));
        wiced_dct_read_unlock( dct, WICED_FALSE );
        return result;
    }

    /* Get the amount of temperature readings to send (required for creating Xively datastream). Mutex protection required. */
    wiced_rtos_lock_mutex( &xively_data.mutex );
    data_points = ( xively_data.temperature_reading_index - xively_data.last_sent_temperature_index ) & ( MAX_TEMPERATURE_READINGS - 1 );
    wiced_rtos_unlock_mutex( &xively_data.mutex );

    /* Create Xively datastream */
    result = xively_create_datastream( &sensor_feed, &stream, dct->xively_channel_id, 4, data_points );
    wiced_dct_read_unlock( dct, WICED_FALSE );
    if ( result != WICED_SUCCESS )
    {
        xively_close_feed( &sensor_feed );
        WPRINT_APP_INFO( ("Failed to init Xively datastream\n") );
        return result;
    }


    /* Write data to TCP stream (compensating for looping around the end of the buffer). Mutex protection required */
    wiced_rtos_lock_mutex( &xively_data.mutex );

    while ( data_points > 0 )
    {
        xively_write_datapoint( &stream, (const uint8_t*) &xively_data.temperature_readings[ xively_data.last_sent_temperature_index ].sample, &xively_data.temperature_readings[ xively_data.last_sent_temperature_index ].timestamp );
        xively_data.last_sent_temperature_index = ( xively_data.last_sent_temperature_index + 1 ) & ( MAX_TEMPERATURE_READINGS - 1 );
        data_points--;
    }

    wiced_rtos_unlock_mutex( &xively_data.mutex );

    /* Flush (send) temperature readings in the TCP stream to Xively */
    if ( xively_flush_datastream( &stream ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("data sent!\n") );
    }
    else
    {
        WPRINT_APP_INFO( ("Failed to send data!\n") );
    }

    /* Close Xively */
    return xively_close_feed( &sensor_feed );
}

/*
 * Gets current temperature set point
 */
static float get_setpoint( void )
{
    float current_setpoint;

    wiced_rtos_lock_mutex( &setpoint.mutex );
    current_setpoint = setpoint.temperature;
    wiced_rtos_unlock_mutex( &setpoint.mutex );

    return current_setpoint;
}

/*
 * Adjusts brightness of the set point LEDs
 */
static void adjust_setpoint_led_brightness( void )
{
    float up_led_brightness_level;
    float down_led_brightness_level;
    float up_led_duty_cycle;
    float down_led_duty_cycle;

    if ( setpoint.temperature > DEFAULT_SETPOINT )
    {
        up_led_brightness_level   = setpoint.temperature - DEFAULT_SETPOINT;
        down_led_brightness_level = 0.0f;
    }
    else
    {
        up_led_brightness_level   = 0.0f;
        down_led_brightness_level = DEFAULT_SETPOINT - setpoint.temperature;
    }

    /* Update LED brightness */
    up_led_duty_cycle  = LED_BRIGHTNESS_EQUATION( up_led_brightness_level );
    down_led_duty_cycle = LED_BRIGHTNESS_EQUATION( down_led_brightness_level );

    wiced_pwm_init ( SETPOINT_UP_LED, SETPOINT_LED_PWM_FREQ_HZ, up_led_duty_cycle );
    wiced_pwm_start( SETPOINT_UP_LED );
    wiced_pwm_init ( SETPOINT_DOWN_LED, SETPOINT_LED_PWM_FREQ_HZ, down_led_duty_cycle );
    wiced_pwm_start( SETPOINT_DOWN_LED );
}

/*
 * Increases temperature set point
 */
static void increase_setpoint( void )
{
    wiced_rtos_lock_mutex( &setpoint.mutex );
    setpoint.temperature += ( setpoint.temperature < MAX_SETPOINT ) ? SETPOINT_INCREMENT : 0.0f;
    adjust_setpoint_led_brightness( );
    wiced_rtos_unlock_mutex( &setpoint.mutex );
}

/*
 * Decreases temperature set point
 */
static void decrease_setpoint( void )
{
    wiced_rtos_lock_mutex( &setpoint.mutex );
    setpoint.temperature -= ( setpoint.temperature > MIN_SETPOINT ) ? SETPOINT_INCREMENT : 0.0f;
    adjust_setpoint_led_brightness( );
    wiced_rtos_unlock_mutex( &setpoint.mutex );
}

/*
 * Handles key events
 */
static void setpoint_control_keypad_handler( gpio_key_code_t code, gpio_key_event_t event )
{
    if ( event == KEY_EVENT_PRESSED || event == KEY_EVENT_HELD )
    {
        switch ( code )
        {
            case SETPOINT_UP_KEY_CODE:
                increase_setpoint( );
                break;
            case SETPOINT_DOWN_KEY_CODE:
                decrease_setpoint( );
                break;
            default:
                break;
        }
    }
}

/*
 * Updates temperature information in the web page
 */
static int32_t process_temperature_update( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data )
{
    wiced_iso8601_time_t* curr_time;
    float temp_celcius;
    float temp_fahrenheit;
    float setpoint_celcius = get_setpoint();
    float setpoint_fahrenheit;
    char  temp_char_buffer[6];
    int   temp_char_buffer_length;

    UNUSED_PARAMETER(http_data);

    wiced_rtos_lock_mutex( &xively_data.mutex );

    if ( xively_data.temperature_reading_index == 0 )
    {
        curr_time = &xively_data.temperature_readings[MAX_TEMPERATURE_READINGS - 1].timestamp;
    }
    else
    {
        curr_time = &xively_data.temperature_readings[xively_data.temperature_reading_index - 1].timestamp;
    }

    /* Update temperature report with the most recent temperature reading */
    temp_celcius        = (float) xively_data.last_sample / 10.0f;
    temp_fahrenheit     = temp_celcius / 5.0f * 9.0f + 32.0f;
    setpoint_fahrenheit = setpoint_celcius / 5.0f * 9.0f + 32.0f;

    wiced_rtos_unlock_mutex( &xively_data.mutex );

    /* Write the time */
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_time_start );
    wiced_tcp_stream_write( stream, curr_time->hour, sizeof(curr_time->hour)   +
                                                     sizeof(curr_time->colon1) +
                                                     sizeof(curr_time->minute) +
                                                     sizeof(curr_time->colon2) +
                                                     sizeof(curr_time->second) );
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_time_end );

    /* Write the date */
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_date_start );
    wiced_tcp_stream_write(stream, curr_time->year, sizeof(curr_time->year)  +
                                                    sizeof(curr_time->dash1) +
                                                    sizeof(curr_time->month) +
                                                    sizeof(curr_time->dash2) +
                                                    sizeof(curr_time->day) );

    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_date_end );

    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_temp_start );
    temp_char_buffer_length = sprintf(temp_char_buffer, "%.1f", temp_celcius);
    wiced_tcp_stream_write(stream, temp_char_buffer, temp_char_buffer_length );
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_temp_mid );
    temp_char_buffer_length = sprintf(temp_char_buffer, "%.1f", temp_fahrenheit);
    wiced_tcp_stream_write(stream, temp_char_buffer, temp_char_buffer_length );
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_temp_end );

    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_set_start );
    temp_char_buffer_length = sprintf(temp_char_buffer, "%.1f", setpoint_celcius);
    wiced_tcp_stream_write(stream, temp_char_buffer, temp_char_buffer_length );
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_set_mid );
    temp_char_buffer_length = sprintf(temp_char_buffer, "%.1f", setpoint_fahrenheit);
    wiced_tcp_stream_write(stream, temp_char_buffer, temp_char_buffer_length );
    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_set_end );

    wiced_tcp_stream_write_resource( stream, &resources_apps_DIR_temp_control_DIR_data_html_page_end );

    return 0;
}

/*
 * Handles increase temperature set point button from web page
 */
static int32_t process_temperature_up( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data )
{
    UNUSED_PARAMETER( url_parameters );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_data );
    increase_setpoint( );
    return 0;
}

/*
 * Handles decrease temperature set point button from web page
 */
static int32_t process_temperature_down( const char* url_parameters, wiced_tcp_stream_t* stream, void* arg, wiced_http_message_body_t* http_data )
{
    UNUSED_PARAMETER( url_parameters );
    UNUSED_PARAMETER( arg );
    UNUSED_PARAMETER( http_data );
    decrease_setpoint( );
    return 0;
}
