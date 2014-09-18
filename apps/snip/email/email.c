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
 * SMTP e-mail Application
 *
 * This application demonstrates how to use the WICED SMTP API
 * to send a secure email.
 *
 * Features demonstrated
 * - Wi-Fi client
 * - SNTP time synchronisation
 * - DNS lookup
 * - SMTP (email) send
 *
 * Application Instructions
 * 1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *    in the wifi_config_dct.h header file to match your Wi-Fi access point
 * 2. Change the #define RECIPIENT email address to match your email address
 * 3. If desired, change the email account used to send email (or use
 *    the default wiced.test@gmail.com account)
 * 4. Connect a PC terminal to the serial port of the WICED Eval board,
 *    then build and download the application as described in the WICED
 *    Quick Start Guide
 *
 * After the download completes, the application :
 *  - Connects to the Wi-Fi network specified
 *  - Updates the local time from the network using the SNTP library
 *  - Attempts to send an email to the RECIPIENT email address and
 *    prints the result to the terminal
 *  - Shuts down
 *
 * IMPORTANT NOTE
 *   The username/password of the wiced.test@gmail.com email account used
 *   in this application is provided in good faith to enable demonstration
 *   of the WICED SMTP API. Please *DO NOT* use the email address in a
 *   malicious way or use the email address to send unsolicited SPAM email
 *   to others. Thankyou.
 *
 *   Google occasionally blocks the WICED email account used to send email,
 *   it may be necessary to specify your own email account as noted in point
 *   (3) above.
 *
 */

#include "wiced.h"
#include "smtp.h"
#include "sntp.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define RECIPIENT     "wiced.test@gmail.com"
#define EMAIL_SUBJECT "Hello World! from WICED"
#define EMAIL_CONTENT "Hi, my name is WICED. Use me to add Wi-Fi to your embedded device!\n"

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

static void send_email( void );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    /* Initialise WICED device */
    wiced_init( );

    /* Configure the device */
    /* wiced_configure_device( NULL ); */  /* Config bypassed in local makefile and wifi_config_dct.h */

    /* Bring network up in STA mode */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Email requires timestamp.
     * Enable automatic time synchronisation and configure to synchronise once a day.
     */
    sntp_start_auto_time_sync( 1 * DAYS );

    /* Send the email */
    send_email( );

    /* Clean-up */
    sntp_stop_auto_time_sync();
    wiced_deinit();
}


static void send_email( void )
{
    wiced_email_account_t account;
    wiced_email_t         email;

    memset( &account, 0, sizeof( account ) );
    memset( &email,   0, sizeof( email ) );

    /* Setup email account */
    account.email_address     = "wiced.test@gmail.com";      /* or use your email account    */
    account.user_name         = "wiced.test";                /* or use your username         */
    account.password          = "WicedEmailSnippetApp";      /* or use your password         */
    account.smtp_server       = "smtp.gmail.com";            /* or select your SMTP server   */
    account.smtp_server_port  = 587;                         /* or select another port       */
    account.smtp_encryption   = WICED_EMAIL_ENCRYPTION_TLS;  /* or WICED_EMAIL_NO_ENCRYPTION */

    /* Initialise email account */
    if ( wiced_smtp_account_init( &account ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "Failed to initialise email account!\n\r" ) );
        wiced_smtp_account_deinit( &account );
        return;
    }

    /* Prepare email content. If RECIPIENT is not redefined above,
     * the WICED device sends an email to itself
     */
    email.to_addresses   = RECIPIENT;
    email.cc_addresses   = NULL;
    email.bcc_addresses  = NULL;
    email.subject        = (char*)EMAIL_SUBJECT;
    email.content        = (char*)EMAIL_CONTENT;
    email.content_length = sizeof( EMAIL_CONTENT ) - 1;

    WPRINT_APP_INFO(( "Sending email ... " ));

    if ( wiced_smtp_send( &account, &email ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(( "success!\n" ));
    }
    else
    {
        WPRINT_APP_INFO(( "FAILED!\n" ));
    }

    wiced_smtp_account_deinit( &account );
}
