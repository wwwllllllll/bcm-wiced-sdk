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

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

typedef enum
{
    BASE64_ENC_STANDARD                         = 0x0,
    BASE64_ENC_URL_SAFE_CHARSET                 = 0x1,
    BASE64_ENC_NO_PADDING                       = 0x2,
    BASE64_ENC_URL_SAFE_CHARSET_AND_NO_PADDING  = BASE64_ENC_URL_SAFE_CHARSET | BASE64_ENC_NO_PADDING,
} base64_encode_options_t;

typedef enum
{
    BASE64_DEC_STANDARD         = 0x0,
    BASE64_DEC_URL_SAFE_CHARSET = 0x1,
} base64_decode_options_t;

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

int isspace( int c );
int base64_encode( unsigned char const* src, uint32_t src_length, unsigned char* target, uint32_t target_size, base64_encode_options_t options );
int base64_decode( unsigned char const* src, uint32_t src_length, unsigned char* target, uint32_t target_size, base64_decode_options_t options );

#ifdef __cplusplus
} /* extern "C" */
#endif
