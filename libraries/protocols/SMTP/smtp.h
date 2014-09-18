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

#include "wiced.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *            Enumerations
 ******************************************************/

/* Email encryption */
typedef enum
{
    WICED_EMAIL_NO_ENCRYPTION,  /* Email is not encrypted when transmitted over the network       */
    WICED_EMAIL_ENCRYPTION_TLS, /* Email is encrypted using TLS when transmitted over the network */
} wiced_email_encryption_t;

/******************************************************
 *             Structures
 ******************************************************/

/* Email account structure */
typedef struct
{
    /* Public */
    char*                    email_address;    /* Pointer to this account's email address  */
    char*                    user_name;        /* Pointer to this account's user name      */
    char*                    password;         /* Pointer to this account's password       */
    char*                    smtp_server;      /* Pointer to SMTP server address string    */
    uint16_t                 smtp_server_port; /* SMTP server port                         */
    wiced_email_encryption_t smtp_encryption;  /* Outgoing mail encryption                 */

    /* Private. Internal use only */
    struct wiced_email_account_internal* internal;
} wiced_email_account_t;

/* Email structure */
typedef struct
{
    /* Public */
    char*    to_addresses;     /* Recipients' email addresses separated by commas */
    char*    cc_addresses;     /* Cc email addresses separated by commas          */
    char*    bcc_addresses;    /* Bcc email addresses separated by commas         */
    char*    subject;          /* Email subject                                   */
    char*    content;          /* Email content/body                              */
    uint32_t content_length;   /* Length of email content/body                    */
    char*    signature;        /* Signature content                               */
    uint32_t signature_length; /* Signature length                                */

    /* Private. Internal use only */
    struct wiced_email_internal* internal;
} wiced_email_t;

/******************************************************
 *             Function declarations
 ******************************************************/

wiced_result_t wiced_smtp_account_init  ( wiced_email_account_t* account );
wiced_result_t wiced_smtp_account_deinit( wiced_email_account_t* account );
wiced_result_t wiced_smtp_send          ( wiced_email_account_t* account, const wiced_email_t* email );

#ifdef __cplusplus
} /* extern "C" */
#endif
