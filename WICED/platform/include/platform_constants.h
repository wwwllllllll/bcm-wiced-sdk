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
 * Defines platform constants
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                    Macros
 ******************************************************/

#ifndef  TO_STRING
#define TO_STRING( a ) #a
#endif

#ifndef RESULT_ENUM
#define RESULT_ENUM( prefix, name, value )  prefix ## name = (value)
#endif /* ifndef RESULT_ENUM */

/* These Enum result values are for platform errors
 * Values: 1000 - 1999
 */
#define PLATFORM_RESULT_LIST( prefix ) \
        RESULT_ENUM( prefix, SUCCESS,             0 ),   /**< Success */               \
        RESULT_ENUM( prefix, PENDING,             1 ),   /**< Pending */               \
        RESULT_ENUM( prefix, TIMEOUT,             2 ),   /**< Timeout */               \
        RESULT_ENUM( prefix, PARTIAL_RESULTS,     3 ),   /**< Partial results */       \
        RESULT_ENUM( prefix, ERROR,               4 ),   /**< Error */                 \
        RESULT_ENUM( prefix, BADARG,              5 ),   /**< Bad Arguments */         \
        RESULT_ENUM( prefix, BADOPTION,           6 ),   /**< Mode not supported */    \
        RESULT_ENUM( prefix, UNSUPPORTED,         7 ),   /**< Unsupported function */  \
        RESULT_ENUM( prefix, UNINITLIASED,     6008 ),   /**< Unitialised */           \
        RESULT_ENUM( prefix, INIT_FAIL,        6009 ),   /**< Initialisation failed */ \
        RESULT_ENUM( prefix, NO_EFFECT,        6010 ),   /**< No effect */             \
        RESULT_ENUM( prefix, FEATURE_DISABLED, 6011 ),   /**< Feature disabled */      \
        RESULT_ENUM( prefix, NO_WLAN_POWER,    6012 ),   /**< WLAN core is not powered */

/******************************************************
 *                   Enumerations
 ******************************************************/

/* Platform result enumeration */
typedef enum
{
    PLATFORM_RESULT_LIST( PLATFORM_ )
} platform_result_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /* extern "C" */
#endif

