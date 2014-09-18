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

#include "wiced_time.h"
#include "wiced_utilities.h"
#include "string.h"

/******************************************************
 *                      Macros
 ******************************************************/

#define UTC_TIME_TO_TIME( utc_time ) ( utc_time / 1000 )

/******************************************************
 *                    Constants
 ******************************************************/

#define SECONDS_IN_365_DAY_YEAR  (31536000)
#define SECONDS_IN_A_DAY         (86400)
#define SECONDS_IN_A_HOUR        (3600)
#define SECONDS_IN_A_MINUTE      (60)
static const uint32_t secondsPerMonth[12] =
{
    31*SECONDS_IN_A_DAY,
    28*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
    30*SECONDS_IN_A_DAY,
    31*SECONDS_IN_A_DAY,
};

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

static wiced_utc_time_ms_t current_utc_time = 0;
static wiced_time_t        last_utc_time_wiced_reference = 0;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t wiced_time_get_utc_time( wiced_utc_time_t* utc_time )
{
    wiced_utc_time_ms_t utc_time_ms;

    wiced_time_get_utc_time_ms( &utc_time_ms );

    *utc_time = (uint32_t)( utc_time_ms / 1000 );
    return WICED_SUCCESS;
}

wiced_result_t wiced_time_get_utc_time_ms( wiced_utc_time_ms_t* utc_time_ms )
{
    wiced_time_t temp_wiced_time;
    uint32_t     time_since_last_reference;

    /* Update the UTC time by the time difference between now and the last update */
    wiced_time_get_time( &temp_wiced_time );
    time_since_last_reference = ( temp_wiced_time - last_utc_time_wiced_reference );

    if ( time_since_last_reference != 0 )
    {
        current_utc_time += time_since_last_reference;
        last_utc_time_wiced_reference = temp_wiced_time;
    }

    *utc_time_ms = current_utc_time;
    return WICED_SUCCESS;
}

wiced_result_t wiced_time_set_utc_time_ms( const wiced_utc_time_ms_t* utc_time_ms )
{
    wiced_time_get_time( &last_utc_time_wiced_reference );
    current_utc_time = *utc_time_ms;
    return WICED_SUCCESS;
}

wiced_result_t wiced_time_get_iso8601_time(wiced_iso8601_time_t* iso8601_time)
{
    wiced_utc_time_ms_t utc_time_ms;
    uint32_t            a;
    uint16_t            year;
    uint8_t             number_of_leap_years;
    uint8_t             month;
    uint8_t             day;
    uint8_t             hour;
    uint8_t             minute;
    uint64_t            second;
    uint16_t            sub_second;
    wiced_bool_t        is_a_leap_year;

    wiced_time_get_utc_time_ms( &utc_time_ms );

    second     = utc_time_ms / 1000;               /* Time is in milliseconds. Convert to seconds */
    sub_second = (uint16_t) (( utc_time_ms % 1000 ) * 1000 ); /* Sub-second is in microseconds               */

    /* Calculate year */
    year = (uint16_t)( 1970 + second / SECONDS_IN_365_DAY_YEAR );
    number_of_leap_years = (uint8_t) (( year - 1968 - 1 ) / 4 );
    second -= (uint64_t)( ( year - 1970 ) * SECONDS_IN_365_DAY_YEAR );
    if ( second > ( number_of_leap_years * SECONDS_IN_A_DAY ) )
    {
        second -= (uint64_t) ( ( number_of_leap_years * SECONDS_IN_A_DAY ) );
    }
    else
    {
        second -= (uint64_t) ( ( number_of_leap_years * SECONDS_IN_A_DAY ) + SECONDS_IN_365_DAY_YEAR );
        --year;
    }

    /* Remember if the current year is a leap year */
    is_a_leap_year = ( ( year - 1968 ) % 4 == 0 ) ? WICED_TRUE : WICED_FALSE;

    /* Calculate month */
    month = 1;

    for ( a = 0; a < 12; ++a )
    {
        uint32_t seconds_per_month = secondsPerMonth[a];
        /* Compensate for leap year */
        if ( ( a == 1 ) && is_a_leap_year )
        {
            seconds_per_month += SECONDS_IN_A_DAY;
        }
        if ( second > seconds_per_month )
        {
            second -= seconds_per_month;
            month++;
        }
        else
        {
            break;
        }
    }

    /* Calculate day */
    day    = (uint8_t) ( second / SECONDS_IN_A_DAY );
    second -= (uint64_t) ( day * SECONDS_IN_A_DAY );
    ++day;

    /* Calculate hour */
    hour   = (uint8_t) ( second / SECONDS_IN_A_HOUR );
    second -= (uint64_t)  ( hour * SECONDS_IN_A_HOUR );

    /* Calculate minute */
    minute = (uint8_t) ( second / SECONDS_IN_A_MINUTE );
    second -= (uint64_t) ( minute * SECONDS_IN_A_MINUTE );

    /* Write iso8601 time */
    utoa( year,             iso8601_time->year,       4, 4 );
    utoa( month,            iso8601_time->month,      2, 2 );
    utoa( day,              iso8601_time->day,        2, 2 );
    utoa( hour,             iso8601_time->hour,       2, 2 );
    utoa( minute,           iso8601_time->minute,     2, 2 );
    utoa( (uint8_t)second,  iso8601_time->second,     2, 2 );
    utoa( sub_second,       iso8601_time->sub_second, 6, 6 );

    iso8601_time->T          = 'T';
    iso8601_time->Z          = 'Z';
    iso8601_time->colon1     = ':';
    iso8601_time->colon2     = ':';
    iso8601_time->dash1      = '-';
    iso8601_time->dash2      = '-';
    iso8601_time->decimal    = '.';

    return WICED_SUCCESS;
}
