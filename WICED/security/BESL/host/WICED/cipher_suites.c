/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include <string.h>  /* for NULL */
#include "cipher_suites.h"

/* This prevents the linker from pulling in code for unused ciphers */

struct cipher_api_t;

extern const struct cipher_api_t null_cipher_driver;
extern const struct cipher_api_t rc4_40_cipher_driver;
extern const struct cipher_api_t rc4_128_cipher_driver;
extern const struct cipher_api_t rc2_cbc_40_cipher_driver;
extern const struct cipher_api_t idea_cbc_cipher_driver;
extern const struct cipher_api_t des40_cbc_cipher_driver;
extern const struct cipher_api_t des_cbc_cipher_driver;
extern const struct cipher_api_t des_cbc_40_cipher_driver;
extern const struct cipher_api_t triple_des_ede_cbc_cipher_driver;
extern const struct cipher_api_t aes_128_cbc_cipher_driver;
extern const struct cipher_api_t aes_256_cbc_cipher_driver;
extern const struct cipher_api_t aes_128_gcm_cipher_driver;
extern const struct cipher_api_t aes_256_gcm_cipher_driver;
extern const struct cipher_api_t aes_128_ccm_cipher_driver;
extern const struct cipher_api_t aes_256_ccm_cipher_driver;
extern const struct cipher_api_t aes_128_ccm_8_cipher_driver;
extern const struct cipher_api_t aes_256_ccm_8_cipher_driver;
extern const struct cipher_api_t camellia_128_cbc_cipher_driver;
extern const struct cipher_api_t camellia_256_cbc_cipher_driver;
extern const struct cipher_api_t camellia_128_gcm_cipher_driver;
extern const struct cipher_api_t camellia_256_gcm_cipher_driver;
extern const struct cipher_api_t seed_cbc_cipher_driver;
extern const struct cipher_api_t aria_128_cbc_cipher_driver;
extern const struct cipher_api_t aria_256_cbc_cipher_driver;
extern const struct cipher_api_t aria_128_gcm_cipher_driver;
extern const struct cipher_api_t aria_256_gcm_cipher_driver;
extern const struct cipher_api_t chacha20_poly1305_cipher_driver;

extern const struct keyscheme_api_t null_keyscheme_driver;
extern const struct keyscheme_api_t krb5_keyscheme_driver;
extern const struct keyscheme_api_t krb5_export_keyscheme_driver;
extern const struct keyscheme_api_t rsa_export_keyscheme_driver;
extern const struct keyscheme_api_t dh_dss_export_keyscheme_driver;
extern const struct keyscheme_api_t dhe_rsa_export_keyscheme_driver;
extern const struct keyscheme_api_t dh_anon__keyscheme_driver;
extern const struct keyscheme_api_t dh_anon_export_keyscheme_driver;
extern const struct keyscheme_api_t dh_rsa_export_keyscheme_driver;
extern const struct keyscheme_api_t dhe_dss_export_keyscheme_driver;
extern const struct keyscheme_api_t ecdh_anon_keyscheme_driver;
extern const struct keyscheme_api_t rsa_keyscheme_driver;
extern const struct keyscheme_api_t dh_dss_keyscheme_driver;
extern const struct keyscheme_api_t dh_rsa_keyscheme_driver;
extern const struct keyscheme_api_t dhe_dss_keyscheme_driver;
extern const struct keyscheme_api_t dhe_rsa_keyscheme_driver;
extern const struct keyscheme_api_t ecdh_ecdsa_keyscheme_driver;
extern const struct keyscheme_api_t ecdh_rsa_keyscheme_driver;
extern const struct keyscheme_api_t ecdhe_rsa_keyscheme_driver;
extern const struct keyscheme_api_t ecdhe_ecdsa_keyscheme_driver;
extern const struct keyscheme_api_t psk_keyscheme_driver;
extern const struct keyscheme_api_t rsa_psk_keyscheme_driver;
extern const struct keyscheme_api_t dhe_psk_keyscheme_driver;
extern const struct keyscheme_api_t ecdhe_psk_keyscheme_driver;
extern const struct keyscheme_api_t srp_sha_keyscheme_driver;
extern const struct keyscheme_api_t srp_sha_rsa_keyscheme_driver;
extern const struct keyscheme_api_t srp_sha_dss_keyscheme_driver;

extern const struct mac_api_t null_mac_driver;
extern const struct mac_api_t md5_mac_driver;
extern const struct mac_api_t sha_mac_driver;
extern const struct mac_api_t sha256_mac_driver;
extern const struct mac_api_t sha384_mac_driver;
extern const struct mac_api_t aes_128_ccm_mac_driver;
extern const struct mac_api_t aes_256_ccm_mac_driver;
extern const struct mac_api_t aes_128_8_ccm_mac_driver;
extern const struct mac_api_t aes_256_ccm_8_mac_driver;


struct ssl3_driver;

#ifdef USE_SSL3

extern const struct ssl3_driver ssl3_driver_impl;
const struct ssl3_driver*       ssl3_driver       = &ssl3_driver_impl;

tls_version_num_t tls_minimum_version = SSL3_0;
tls_version_num_t tls_maximum_version = TLS1_2;    /* This exists to allow testing of all TLS versions */

#else

const struct ssl3_driver*    ssl3_driver      = NULL;

tls_version_num_t tls_minimum_version = TLS1_0;
tls_version_num_t tls_maximum_version = TLS1_2;    /* This exists to allow testing of all TLS versions */

#endif /* ifdef USE_SSL3 */

const char * tls_version_names[] =
{
    [SSL3_0] = "SSL 3.0",
    [TLS1_0] = "TLS 1.0",
    [TLS1_1] = "TLS 1.1",
    [TLS1_2] = "TLS 1.2",

};

const char * cipher_names[] =
{
    [NULL_CIPHER              ] = "NULL",
    [RC4_40_CIPHER            ] = "RC4_40",
    [RC4_128_CIPHER           ] = "RC4_128",
    [RC2_CBC_40_CIPHER        ] = "RC2_CBC_40",
    [IDEA_CBC_CIPHER          ] = "IDEA_CBC",
    [DES40_CBC_CIPHER         ] = "DES40_CBC",
    [DES_CBC_CIPHER           ] = "DES_CBC",
    [DES_CBC_40_CIPHER        ] = "DES_CBC_40",
    [TRIPLE_DES_EDE_CBC_CIPHER] = "3DES_EDE_CBC",
    [AES_128_CBC_CIPHER       ] = "AES_128_CBC",
    [AES_256_CBC_CIPHER       ] = "AES_256_CBC",
    [AES_128_GCM_CIPHER       ] = "AES_128_GCM",
    [AES_256_GCM_CIPHER       ] = "AES_256_GCM",
    [AES_128_CCM_CIPHER       ] = "AES_128_CCM",
    [AES_256_CCM_CIPHER       ] = "AES_256_CCM",
    [AES_128_CCM_8_CIPHER     ] = "AES_128_CCM_8",
    [AES_256_CCM_8_CIPHER     ] = "AES_256_CCM_8",
    [CAMELLIA_128_CBC_CIPHER  ] = "CAMELLIA_128_CBC",
    [CAMELLIA_256_CBC_CIPHER  ] = "CAMELLIA_256_CBC",
    [CAMELLIA_128_GCM_CIPHER  ] = "CAMELLIA_128_GCM",
    [CAMELLIA_256_GCM_CIPHER  ] = "CAMELLIA_256_GCM",
    [SEED_CBC_CIPHER          ] = "SEED_CBC",
    [ARIA_128_CBC_CIPHER      ] = "ARIA_128_CBC",
    [ARIA_256_CBC_CIPHER      ] = "ARIA_256_CBC",
    [ARIA_128_GCM_CIPHER      ] = "ARIA_128_GCM",
    [ARIA_256_GCM_CIPHER      ] = "ARIA_256_GCM",
    [CHACHA20_POLY1305_CIPHER ] = "CHACHA20_POLY1305",
};

const char* keyscheme_names[] =
{
    [NULL_KEYSCHEME           ] = "NULL",
    [KRB5_KEYSCHEME           ] = "KRB5",
    [KRB5_EXPORT_KEYSCHEME    ] = "KRB5_EXPORT",
    [RSA_EXPORT_KEYSCHEME     ] = "RSA_EXPORT",
    [DH_DSS_EXPORT_KEYSCHEME  ] = "DH_DSS_EXPORT",
    [DHE_RSA_EXPORT_KEYSCHEME ] = "DHE_RSA_EXPORT",
    [DH_anon_KEYSCHEME        ] = "DH_anon",
    [DH_anon_EXPORT_KEYSCHEME ] = "DH_anon_EXPORT",
    [DH_RSA_EXPORT_KEYSCHEME  ] = "DH_RSA_EXPORT",
    [DHE_DSS_EXPORT_KEYSCHEME ] = "DHE_DSS_EXPORT",
    [ECDH_anon_KEYSCHEME      ] = "ECDH_anon",
    [RSA_KEYSCHEME            ] = "RSA",
    [DH_DSS_KEYSCHEME         ] = "DH_DSS",
    [DH_RSA_KEYSCHEME         ] = "DH_RSA",
    [DHE_DSS_KEYSCHEME        ] = "DHE_DSS",
    [DHE_RSA_KEYSCHEME        ] = "DHE_RSA",
    [ECDH_ECDSA_KEYSCHEME     ] = "ECDH_ECDSA",
    [ECDH_RSA_KEYSCHEME       ] = "ECDH_RSA",
    [ECDHE_RSA_KEYSCHEME      ] = "ECDHE_RSA",
    [ECDHE_ECDSA_KEYSCHEME    ] = "ECDHE_ECDSA",
    [PSK_KEYSCHEME            ] = "PSK",
    [RSA_PSK_KEYSCHEME        ] = "RSA_PSK",
    [DHE_PSK_KEYSCHEME        ] = "DHE_PSK",
    [ECDHE_PSK_KEYSCHEME      ] = "ECDHE_PSK",
    [SRP_SHA_KEYSCHEME        ] = "SRP_SHA",
    [SRP_SHA_RSA_KEYSCHEME    ] = "SRP_SHA_RSA",
    [SRP_SHA_DSS_KEYSCHEME    ] = "SRP_SHA_DSS",
};

const char* mac_names[] =
{
    [NULL_MAC          ] = "NULL",
    [MD5_MAC           ] = "MD5",
    [SHA_MAC           ] = "SHA",
    [SHA256_MAC        ] = "SHA256",
    [SHA384_MAC        ] = "SHA384",
    [AES_128_CCM_MAC   ] = "",
    [AES_256_CCM_MAC   ] = "",
    [AES_128_CCM_8_MAC ] = "",
    [AES_266_CCM_8_MAC ] = "",
};

/* Note - The following code was generated using the conv_cipher_suite.pl script */
/* See https://www.iana.org/assignments/tls-parameters/tls-parameters.xhtml#tls-parameters-4 */


#if defined( USE_NULL_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_NULL_MAC )
const cipher_suite_t TLS_NULL_WITH_NULL_NULL = { 0x0000, &null_keyscheme_driver, &null_cipher_driver, &null_mac_driver };
#endif /* if defined( USE_NULL_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_NULL_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_RSA_WITH_NULL_MD5 = { 0x0001, &rsa_keyscheme_driver, &null_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_NULL_SHA = { 0x0002, &rsa_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_EXPORT_KEYSCHEME ) && defined( USE_RC4_40_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_RSA_EXPORT_WITH_RC4_40_MD5 = { 0x0003, &rsa_export_keyscheme_driver, &rc4_40_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_RSA_EXPORT_KEYSCHEME ) && defined( USE_RC4_40    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_RSA_WITH_RC4_128_MD5 = { 0x0004, &rsa_keyscheme_driver, &rc4_128_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_RC4_128_SHA = { 0x0005, &rsa_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_EXPORT_KEYSCHEME ) && defined( USE_RC2_CBC_40_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_RSA_EXPORT_WITH_RC2_CBC_40_MD5 = { 0x0006, &rsa_export_keyscheme_driver, &rc2_cbc_40_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_RSA_EXPORT_KEYSCHEME ) && defined( USE_RC2_CBC_40    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_IDEA_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_IDEA_CBC_SHA = { 0x0007, &rsa_keyscheme_driver, &idea_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_IDEA_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_EXPORT_WITH_DES40_CBC_SHA = { 0x0008, &rsa_export_keyscheme_driver, &des40_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_DES_CBC_SHA = { 0x0009, &rsa_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_3DES_EDE_CBC_SHA = { 0x000A, &rsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA = { 0x000B, &dh_dss_export_keyscheme_driver, &des40_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_DES_CBC_SHA = { 0x000C, &dh_dss_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA = { 0x000D, &dh_dss_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA = { 0x000E, &dh_rsa_export_keyscheme_driver, &des40_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_DES_CBC_SHA = { 0x000F, &dh_rsa_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA = { 0x0010, &dh_rsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA = { 0x0011, &dhe_dss_export_keyscheme_driver, &des40_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_DES_CBC_SHA = { 0x0012, &dhe_dss_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA = { 0x0013, &dhe_dss_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA = { 0x0014, &dhe_rsa_export_keyscheme_driver, &des40_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_DES_CBC_SHA = { 0x0015, &dhe_rsa_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA = { 0x0016, &dhe_rsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_RC4_40_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_DH_anon_EXPORT_WITH_RC4_40_MD5 = { 0x0017, &dh_anon_export_keyscheme_driver, &rc4_40_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_RC4_40    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_DH_anon_WITH_RC4_128_MD5 = { 0x0018, &dh_anon_keyscheme_driver, &rc4_128_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA = { 0x0019, &dh_anon_export_keyscheme_driver, &des40_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_EXPORT_KEYSCHEME ) && defined( USE_DES40_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_DES_CBC_SHA = { 0x001A, &dh_anon_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_3DES_EDE_CBC_SHA = { 0x001B, &dh_anon_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

/* Reserved to avoid conflicts with SSLv3 0x001C - 0x001D */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_WITH_DES_CBC_SHA = { 0x001E, &krb5_keyscheme_driver, &des_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_WITH_3DES_EDE_CBC_SHA = { 0x001F, &krb5_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_WITH_RC4_128_SHA = { 0x0020, &krb5_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_IDEA_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_WITH_IDEA_CBC_SHA = { 0x0021, &krb5_keyscheme_driver, &idea_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_IDEA_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_DES_CBC_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_WITH_DES_CBC_MD5 = { 0x0022, &krb5_keyscheme_driver, &des_cbc_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_DES_CBC    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_WITH_3DES_EDE_CBC_MD5 = { 0x0023, &krb5_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_WITH_RC4_128_MD5 = { 0x0024, &krb5_keyscheme_driver, &rc4_128_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_KRB5_KEYSCHEME ) && defined( USE_IDEA_CBC_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_WITH_IDEA_CBC_MD5 = { 0x0025, &krb5_keyscheme_driver, &idea_cbc_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_KEYSCHEME ) && defined( USE_IDEA_CBC    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DES_CBC_40_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA = { 0x0026, &krb5_export_keyscheme_driver, &des_cbc_40_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DES_CBC_40    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC2_CBC_40_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA = { 0x0027, &krb5_export_keyscheme_driver, &rc2_cbc_40_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC2_CBC_40    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC4_40_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_KRB5_EXPORT_WITH_RC4_40_SHA = { 0x0028, &krb5_export_keyscheme_driver, &rc4_40_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC4_40    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DES_CBC_40_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5 = { 0x0029, &krb5_export_keyscheme_driver, &des_cbc_40_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_DES_CBC_40    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC2_CBC_40_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5 = { 0x002A, &krb5_export_keyscheme_driver, &rc2_cbc_40_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC2_CBC_40    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC4_40_CIPHER ) && defined( USE_MD5_MAC )
const cipher_suite_t TLS_KRB5_EXPORT_WITH_RC4_40_MD5 = { 0x002B, &krb5_export_keyscheme_driver, &rc4_40_cipher_driver, &md5_mac_driver };
#endif /* if defined( USE_KRB5_EXPORT_KEYSCHEME ) && defined( USE_RC4_40    _CIPHER ) && defined( USE_MD5_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_PSK_WITH_NULL_SHA = { 0x002C, &psk_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_NULL_SHA = { 0x002D, &dhe_psk_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_NULL_SHA = { 0x002E, &rsa_psk_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_128_CBC_SHA = { 0x002F, &rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_AES_128_CBC_SHA = { 0x0030, &dh_dss_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_AES_128_CBC_SHA = { 0x0031, &dh_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_AES_128_CBC_SHA = { 0x0032, &dhe_dss_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_128_CBC_SHA = { 0x0033, &dhe_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_AES_128_CBC_SHA = { 0x0034, &dh_anon_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_256_CBC_SHA = { 0x0035, &rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_AES_256_CBC_SHA = { 0x0036, &dh_dss_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_AES_256_CBC_SHA = { 0x0037, &dh_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_AES_256_CBC_SHA = { 0x0038, &dhe_dss_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_256_CBC_SHA = { 0x0039, &dhe_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_AES_256_CBC_SHA = { 0x003A, &dh_anon_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_NULL_SHA256 = { 0x003B, &rsa_keyscheme_driver, &null_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_128_CBC_SHA256 = { 0x003C, &rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_256_CBC_SHA256 = { 0x003D, &rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_AES_128_CBC_SHA256 = { 0x003E, &dh_dss_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_AES_128_CBC_SHA256 = { 0x003F, &dh_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_AES_128_CBC_SHA256 = { 0x0040, &dhe_dss_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_CAMELLIA_128_CBC_SHA = { 0x0041, &rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA = { 0x0042, &dh_dss_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA = { 0x0043, &dh_rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA = { 0x0044, &dhe_dss_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA = { 0x0045, &dhe_rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA = { 0x0046, &dh_anon_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

/* Reserved to avoid conflicts with deployed implementations 0x0047 - 0x004F */

/* Reserved to avoid conflicts 0x0050 - 0x0058 */

/* Reserved to avoid conflicts with deployed implementations 0x0059 - 0x005C */

/* Unassigned 0x005D - 0x005F */

/* Reserved to avoid conflicts with widely deployed implementations 0x0060 - 0x0066 */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 = { 0x0067, &dhe_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_AES_256_CBC_SHA256 = { 0x0068, &dh_dss_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_AES_256_CBC_SHA256 = { 0x0069, &dh_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_AES_256_CBC_SHA256 = { 0x006A, &dhe_dss_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 = { 0x006B, &dhe_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_AES_128_CBC_SHA256 = { 0x006C, &dh_anon_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_AES_256_CBC_SHA256 = { 0x006D, &dh_anon_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

/* Unassigned 0x006E - 0x0083 */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_CAMELLIA_256_CBC_SHA = { 0x0084, &rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA = { 0x0085, &dh_dss_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA = { 0x0086, &dh_rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA = { 0x0087, &dhe_dss_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA = { 0x0088, &dhe_rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA = { 0x0089, &dh_anon_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_PSK_WITH_RC4_128_SHA = { 0x008A, &psk_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_PSK_WITH_3DES_EDE_CBC_SHA = { 0x008B, &psk_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_128_CBC_SHA = { 0x008C, &psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_256_CBC_SHA = { 0x008D, &psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_RC4_128_SHA = { 0x008E, &dhe_psk_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA = { 0x008F, &dhe_psk_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_128_CBC_SHA = { 0x0090, &dhe_psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_256_CBC_SHA = { 0x0091, &dhe_psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_RC4_128_SHA = { 0x0092, &rsa_psk_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA = { 0x0093, &rsa_psk_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_AES_128_CBC_SHA = { 0x0094, &rsa_psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_AES_256_CBC_SHA = { 0x0095, &rsa_psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_SEED_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_RSA_WITH_SEED_CBC_SHA = { 0x0096, &rsa_keyscheme_driver, &seed_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_SEED_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_SEED_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_SEED_CBC_SHA = { 0x0097, &dh_dss_keyscheme_driver, &seed_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_SEED_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_SEED_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_SEED_CBC_SHA = { 0x0098, &dh_rsa_keyscheme_driver, &seed_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_SEED_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_SEED_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_SEED_CBC_SHA = { 0x0099, &dhe_dss_keyscheme_driver, &seed_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_SEED_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_SEED_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_SEED_CBC_SHA = { 0x009A, &dhe_rsa_keyscheme_driver, &seed_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_SEED_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_SEED_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_DH_anon_WITH_SEED_CBC_SHA = { 0x009B, &dh_anon_keyscheme_driver, &seed_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_SEED_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_128_GCM_SHA256 = { 0x009C, &rsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_256_GCM_SHA384 = { 0x009D, &rsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_128_GCM_SHA256 = { 0x009E, &dhe_rsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_256_GCM_SHA384 = { 0x009F, &dhe_rsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_AES_128_GCM_SHA256 = { 0x00A0, &dh_rsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_AES_256_GCM_SHA384 = { 0x00A1, &dh_rsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_AES_128_GCM_SHA256 = { 0x00A2, &dhe_dss_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_AES_256_GCM_SHA384 = { 0x00A3, &dhe_dss_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_AES_128_GCM_SHA256 = { 0x00A4, &dh_dss_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_AES_256_GCM_SHA384 = { 0x00A5, &dh_dss_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_AES_128_GCM_SHA256 = { 0x00A6, &dh_anon_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_anon_WITH_AES_256_GCM_SHA384 = { 0x00A7, &dh_anon_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_128_GCM_SHA256 = { 0x00A8, &psk_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_256_GCM_SHA384 = { 0x00A9, &psk_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_128_GCM_SHA256 = { 0x00AA, &dhe_psk_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_256_GCM_SHA384 = { 0x00AB, &dhe_psk_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_AES_128_GCM_SHA256 = { 0x00AC, &rsa_psk_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_AES_256_GCM_SHA384 = { 0x00AD, &rsa_psk_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_128_CBC_SHA256 = { 0x00AE, &psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_256_CBC_SHA384 = { 0x00AF, &psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_NULL_SHA256 = { 0x00B0, &psk_keyscheme_driver, &null_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_NULL_SHA384 = { 0x00B1, &psk_keyscheme_driver, &null_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_128_CBC_SHA256 = { 0x00B2, &dhe_psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_256_CBC_SHA384 = { 0x00B3, &dhe_psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_NULL_SHA256 = { 0x00B4, &dhe_psk_keyscheme_driver, &null_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_NULL_SHA384 = { 0x00B5, &dhe_psk_keyscheme_driver, &null_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_AES_128_CBC_SHA256 = { 0x00B6, &rsa_psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_AES_256_CBC_SHA384 = { 0x00B7, &rsa_psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_NULL_SHA256 = { 0x00B8, &rsa_psk_keyscheme_driver, &null_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_NULL_SHA384 = { 0x00B9, &rsa_psk_keyscheme_driver, &null_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0x00BA, &rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA256 = { 0x00BB, &dh_dss_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0x00BC, &dh_rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA256 = { 0x00BD, &dhe_dss_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0x00BE, &dhe_rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA256 = { 0x00BF, &dh_anon_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256 = { 0x00C0, &rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA256 = { 0x00C1, &dh_dss_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA256 = { 0x00C2, &dh_rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA256 = { 0x00C3, &dhe_dss_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256 = { 0x00C4, &dhe_rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256 = { 0x00C5, &dh_anon_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

/* Unassigned 0x00C7 - 0x00FE */

/* TLS_EMPTY_RENEGOTIATION_INFO_SCSV 0x00FF */

/* Unassigned 0x0100 - 0xBFFF */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_NULL_SHA = { 0xC001, &ecdh_ecdsa_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_RC4_128_SHA = { 0xC002, &ecdh_ecdsa_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA = { 0xC003, &ecdh_ecdsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA = { 0xC004, &ecdh_ecdsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA = { 0xC005, &ecdh_ecdsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_NULL_SHA = { 0xC006, &ecdhe_ecdsa_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_RC4_128_SHA = { 0xC007, &ecdhe_ecdsa_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA = { 0xC008, &ecdhe_ecdsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA = { 0xC009, &ecdhe_ecdsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA = { 0xC00A, &ecdhe_ecdsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_NULL_SHA = { 0xC00B, &ecdh_rsa_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_RC4_128_SHA = { 0xC00C, &ecdh_rsa_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA = { 0xC00D, &ecdh_rsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_AES_128_CBC_SHA = { 0xC00E, &ecdh_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_AES_256_CBC_SHA = { 0xC00F, &ecdh_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_NULL_SHA = { 0xC010, &ecdhe_rsa_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_RC4_128_SHA = { 0xC011, &ecdhe_rsa_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA = { 0xC012, &ecdhe_rsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA = { 0xC013, &ecdhe_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA = { 0xC014, &ecdhe_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_anon_WITH_NULL_SHA = { 0xC015, &ecdh_anon_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_anon_WITH_RC4_128_SHA = { 0xC016, &ecdh_anon_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA = { 0xC017, &ecdh_anon_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_anon_WITH_AES_128_CBC_SHA = { 0xC018, &ecdh_anon_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDH_anon_WITH_AES_256_CBC_SHA = { 0xC019, &ecdh_anon_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDH_anon_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA = { 0xC01A, &srp_sha_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA = { 0xC01B, &srp_sha_rsa_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA = { 0xC01C, &srp_sha_dss_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_WITH_AES_128_CBC_SHA = { 0xC01D, &srp_sha_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA = { 0xC01E, &srp_sha_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA = { 0xC01F, &srp_sha_dss_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_WITH_AES_256_CBC_SHA = { 0xC020, &srp_sha_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA = { 0xC021, &srp_sha_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA = { 0xC022, &srp_sha_dss_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_SRP_SHA_DSS_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 = { 0xC023, &ecdhe_ecdsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 = { 0xC024, &ecdhe_ecdsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256 = { 0xC025, &ecdh_ecdsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384 = { 0xC026, &ecdh_ecdsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 = { 0xC027, &ecdhe_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 = { 0xC028, &ecdhe_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256 = { 0xC029, &ecdh_rsa_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384 = { 0xC02A, &ecdh_rsa_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 = { 0xC02B, &ecdhe_ecdsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 = { 0xC02C, &ecdhe_ecdsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 = { 0xC02D, &ecdh_ecdsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384 = { 0xC02E, &ecdh_ecdsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 = { 0xC02F, &ecdhe_rsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384 = { 0xC030, &ecdhe_rsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256 = { 0xC031, &ecdh_rsa_keyscheme_driver, &aes_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384 = { 0xC032, &ecdh_rsa_keyscheme_driver, &aes_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_AES_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_RC4_128_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_RC4_128_SHA = { 0xC033, &ecdhe_psk_keyscheme_driver, &rc4_128_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_RC4_128    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA = { 0xC034, &ecdhe_psk_keyscheme_driver, &triple_des_ede_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_3DES_EDE_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA = { 0xC035, &ecdhe_psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA = { 0xC036, &ecdhe_psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256 = { 0xC037, &ecdhe_psk_keyscheme_driver, &aes_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384 = { 0xC038, &ecdhe_psk_keyscheme_driver, &aes_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_NULL_SHA = { 0xC039, &ecdhe_psk_keyscheme_driver, &null_cipher_driver, &sha_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_NULL_SHA256 = { 0xC03A, &ecdhe_psk_keyscheme_driver, &null_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_NULL_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_NULL_SHA384 = { 0xC03B, &ecdhe_psk_keyscheme_driver, &null_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_NULL    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_ARIA_128_CBC_SHA256 = { 0xC03C, &rsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_WITH_ARIA_256_CBC_SHA384 = { 0xC03D, &rsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_ARIA_128_CBC_SHA256 = { 0xC03E, &dh_dss_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_ARIA_256_CBC_SHA384 = { 0xC03F, &dh_dss_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_ARIA_128_CBC_SHA256 = { 0xC040, &dh_rsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_ARIA_256_CBC_SHA384 = { 0xC041, &dh_rsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_ARIA_128_CBC_SHA256 = { 0xC042, &dhe_dss_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_ARIA_256_CBC_SHA384 = { 0xC043, &dhe_dss_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_ARIA_128_CBC_SHA256 = { 0xC044, &dhe_rsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_ARIA_256_CBC_SHA384 = { 0xC045, &dhe_rsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_ARIA_128_CBC_SHA256 = { 0xC046, &dh_anon_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_anon_WITH_ARIA_256_CBC_SHA384 = { 0xC047, &dh_anon_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_ARIA_128_CBC_SHA256 = { 0xC048, &ecdhe_ecdsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_ARIA_256_CBC_SHA384 = { 0xC049, &ecdhe_ecdsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_ARIA_128_CBC_SHA256 = { 0xC04A, &ecdh_ecdsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_ARIA_256_CBC_SHA384 = { 0xC04B, &ecdh_ecdsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256 = { 0xC04C, &ecdhe_rsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_ARIA_256_CBC_SHA384 = { 0xC04D, &ecdhe_rsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_ARIA_128_CBC_SHA256 = { 0xC04E, &ecdh_rsa_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_ARIA_256_CBC_SHA384 = { 0xC04F, &ecdh_rsa_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_ARIA_128_GCM_SHA256 = { 0xC050, &rsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_WITH_ARIA_256_GCM_SHA384 = { 0xC051, &rsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_ARIA_128_GCM_SHA256 = { 0xC052, &dhe_rsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_ARIA_256_GCM_SHA384 = { 0xC053, &dhe_rsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_ARIA_128_GCM_SHA256 = { 0xC054, &dh_rsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_ARIA_256_GCM_SHA384 = { 0xC055, &dh_rsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_ARIA_128_GCM_SHA256 = { 0xC056, &dhe_dss_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_ARIA_256_GCM_SHA384 = { 0xC057, &dhe_dss_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_ARIA_128_GCM_SHA256 = { 0xC058, &dh_dss_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_ARIA_256_GCM_SHA384 = { 0xC059, &dh_dss_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_ARIA_128_GCM_SHA256 = { 0xC05A, &dh_anon_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_anon_WITH_ARIA_256_GCM_SHA384 = { 0xC05B, &dh_anon_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256 = { 0xC05C, &ecdhe_ecdsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384 = { 0xC05D, &ecdhe_ecdsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_ARIA_128_GCM_SHA256 = { 0xC05E, &ecdh_ecdsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_ARIA_256_GCM_SHA384 = { 0xC05F, &ecdh_ecdsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256 = { 0xC060, &ecdhe_rsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384 = { 0xC061, &ecdhe_rsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_ARIA_128_GCM_SHA256 = { 0xC062, &ecdh_rsa_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_ARIA_256_GCM_SHA384 = { 0xC063, &ecdh_rsa_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_ARIA_128_CBC_SHA256 = { 0xC064, &psk_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_ARIA_256_CBC_SHA384 = { 0xC065, &psk_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_ARIA_128_CBC_SHA256 = { 0xC066, &dhe_psk_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_ARIA_256_CBC_SHA384 = { 0xC067, &dhe_psk_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_ARIA_128_CBC_SHA256 = { 0xC068, &rsa_psk_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_ARIA_256_CBC_SHA384 = { 0xC069, &rsa_psk_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_ARIA_128_GCM_SHA256 = { 0xC06A, &psk_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_ARIA_256_GCM_SHA384 = { 0xC06B, &psk_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256 = { 0xC06C, &dhe_psk_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384 = { 0xC06D, &dhe_psk_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_ARIA_128_GCM_SHA256 = { 0xC06E, &rsa_psk_keyscheme_driver, &aria_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_ARIA_256_GCM_SHA384 = { 0xC06F, &rsa_psk_keyscheme_driver, &aria_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_ARIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_ARIA_128_CBC_SHA256 = { 0xC070, &ecdhe_psk_keyscheme_driver, &aria_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_ARIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_ARIA_256_CBC_SHA384 = { 0xC071, &ecdhe_psk_keyscheme_driver, &aria_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_ARIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC072, &ecdhe_ecdsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC073, &ecdhe_ecdsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC074, &ecdh_ecdsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC075, &ecdh_ecdsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC076, &ecdhe_rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC077, &ecdhe_rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC078, &ecdh_rsa_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC079, &ecdh_rsa_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC07A, &rsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC07B, &rsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC07C, &dhe_rsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC07D, &dhe_rsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC07E, &dh_rsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_RSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC07F, &dh_rsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC080, &dhe_dss_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_DSS_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC081, &dhe_dss_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC082, &dh_dss_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_DSS_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC083, &dh_dss_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_DSS_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DH_anon_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC084, &dh_anon_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DH_anon_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC085, &dh_anon_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DH_anon_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC086, &ecdhe_ecdsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC087, &ecdhe_ecdsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC088, &ecdh_ecdsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC089, &ecdh_ecdsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_ECDSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC08A, &ecdhe_rsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC08B, &ecdhe_rsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC08C, &ecdh_rsa_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC08D, &ecdh_rsa_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDH_RSA_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC08E, &psk_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC08F, &psk_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC090, &dhe_psk_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC091, &dhe_psk_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256 = { 0xC092, &rsa_psk_keyscheme_driver, &camellia_128_gcm_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_GCM    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384 = { 0xC093, &rsa_psk_keyscheme_driver, &camellia_256_gcm_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_GCM    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC094, &psk_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC095, &psk_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC096, &dhe_psk_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC097, &dhe_psk_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC098, &rsa_psk_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC099, &rsa_psk_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_RSA_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 = { 0xC09A, &ecdhe_psk_keyscheme_driver, &camellia_128_cbc_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_128_CBC    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC_CIPHER ) && defined( USE_SHA384_MAC )
const cipher_suite_t TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 = { 0xC09B, &ecdhe_psk_keyscheme_driver, &camellia_256_cbc_cipher_driver, &sha384_mac_driver };
#endif /* if defined( USE_ECDHE_PSK_KEYSCHEME ) && defined( USE_CAMELLIA_256_CBC    _CIPHER ) && defined( USE_SHA384_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM_CIPHER ) && defined( USE_AES_128_CCM_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_128_CCM = { 0xC09C, &rsa_keyscheme_driver, &aes_128_ccm_cipher_driver, &aes_128_ccm_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM    _CIPHER ) && defined( USE_AES_128_CCM_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM_CIPHER ) && defined( USE_AES_256_CCM_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_256_CCM = { 0xC09D, &rsa_keyscheme_driver, &aes_256_ccm_cipher_driver, &aes_256_ccm_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM    _CIPHER ) && defined( USE_AES_256_CCM_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM_CIPHER ) && defined( USE_AES_128_CCM_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_128_CCM = { 0xC09E, &dhe_rsa_keyscheme_driver, &aes_128_ccm_cipher_driver, &aes_128_ccm_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM    _CIPHER ) && defined( USE_AES_128_CCM_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM_CIPHER ) && defined( USE_AES_256_CCM_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_256_CCM = { 0xC09F, &dhe_rsa_keyscheme_driver, &aes_256_ccm_cipher_driver, &aes_256_ccm_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM    _CIPHER ) && defined( USE_AES_256_CCM_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_128_CCM_8 = { 0xC0A0, &rsa_keyscheme_driver, &aes_128_ccm_8_cipher_driver, &aes_128_ccm_8_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8    _CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

#if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM_8_CIPHER ) && defined( USE_AES_256_CCM_8_MAC )
const cipher_suite_t TLS_RSA_WITH_AES_256_CCM_8 = { 0xC0A1, &rsa_keyscheme_driver, &aes_256_ccm_8_cipher_driver, &aes_256_ccm_8_mac_driver };
#endif /* if defined( USE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM_8    _CIPHER ) && defined( USE_AES_256_CCM_8_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_128_CCM_8 = { 0xC0A2, &dhe_rsa_keyscheme_driver, &aes_128_ccm_8_cipher_driver, &aes_128_ccm_8_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8    _CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM_8_CIPHER ) && defined( USE_AES_256_CCM_8_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_AES_256_CCM_8 = { 0xC0A3, &dhe_rsa_keyscheme_driver, &aes_256_ccm_8_cipher_driver, &aes_256_ccm_8_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_AES_256_CCM_8    _CIPHER ) && defined( USE_AES_256_CCM_8_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_CIPHER ) && defined( USE_AES_128_CCM_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_128_CCM = { 0xC0A4, &psk_keyscheme_driver, &aes_128_ccm_cipher_driver, &aes_128_ccm_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM    _CIPHER ) && defined( USE_AES_128_CCM_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM_CIPHER ) && defined( USE_AES_256_CCM_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_256_CCM = { 0xC0A5, &psk_keyscheme_driver, &aes_256_ccm_cipher_driver, &aes_256_ccm_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM    _CIPHER ) && defined( USE_AES_256_CCM_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_CIPHER ) && defined( USE_AES_128_CCM_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_128_CCM = { 0xC0A6, &dhe_psk_keyscheme_driver, &aes_128_ccm_cipher_driver, &aes_128_ccm_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM    _CIPHER ) && defined( USE_AES_128_CCM_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM_CIPHER ) && defined( USE_AES_256_CCM_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_256_CCM = { 0xC0A7, &dhe_psk_keyscheme_driver, &aes_256_ccm_cipher_driver, &aes_256_ccm_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM    _CIPHER ) && defined( USE_AES_256_CCM_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_128_CCM_8 = { 0xC0A8, &psk_keyscheme_driver, &aes_128_ccm_8_cipher_driver, &aes_128_ccm_8_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_8    _CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

#if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM_8_CIPHER ) && defined( USE_AES_256_CCM_8_MAC )
const cipher_suite_t TLS_PSK_WITH_AES_256_CCM_8 = { 0xC0A9, &psk_keyscheme_driver, &aes_256_ccm_8_cipher_driver, &aes_256_ccm_8_mac_driver };
#endif /* if defined( USE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM_8    _CIPHER ) && defined( USE_AES_256_CCM_8_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_128_CCM_8 = { 0xC0AA, &DHE_PSK_keyscheme_driver, &aes_128_ccm_8_cipher_driver, &aes_128_ccm_8_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_128_CCM_8    _CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

#if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM_8_CIPHER ) && defined( USE_AES_256_CCM_8_MAC )
const cipher_suite_t TLS_DHE_PSK_WITH_AES_256_CCM_8 = { 0xC0AB, &DHE_PSK_keyscheme_driver, &aes_256_ccm_8_cipher_driver, &aes_256_ccm_8_mac_driver };
#endif /* if defined( USE_DHE_PSK_KEYSCHEME ) && defined( USE_AES_256_CCM_8    _CIPHER ) && defined( USE_AES_256_CCM_8_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CCM_CIPHER ) && defined( USE_AES_128_CCM_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_128_CCM = { 0xC0AC, &ecdhe_ecdsa_keyscheme_driver, &aes_128_ccm_cipher_driver, &aes_128_ccm_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CCM    _CIPHER ) && defined( USE_AES_128_CCM_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CCM_CIPHER ) && defined( USE_AES_256_CCM_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_256_CCM = { 0xC0AD, &ecdhe_ecdsa_keyscheme_driver, &aes_256_ccm_cipher_driver, &aes_256_ccm_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CCM    _CIPHER ) && defined( USE_AES_256_CCM_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8_CIPHER ) && defined( USE_AES_128_CCM_8_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8 = { 0xC0AE, &ecdhe_ecdsa_keyscheme_driver, &aes_128_ccm_8_cipher_driver, &aes_128_ccm_8_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_128_CCM_8    _CIPHER ) && defined( USE_AES_128_CCM_8_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CCM_8_CIPHER ) && defined( USE_AES_256_CCM_8_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_AES_256_CCM_8 = { 0xC0AF, &ecdhe_ecdsa_keyscheme_driver, &aes_256_ccm_8_cipher_driver, &aes_256_ccm_8_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_AES_256_CCM_8    _CIPHER ) && defined( USE_AES_256_CCM_8_MAC ) */

#if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CHACHA20_POLY1305_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 = { 0xcc13, &ecdhe_rsa_keyscheme_driver, &chacha20_poly1305_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_RSA_KEYSCHEME ) && defined( USE_CHACHA20_POLY1305    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CHACHA20_POLY1305_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 = { 0xcc14, &ecdhe_ecdsa_keyscheme_driver, &chacha20_poly1305_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_ECDHE_ECDSA_KEYSCHEME ) && defined( USE_CHACHA20_POLY1305    _CIPHER ) && defined( USE_SHA256_MAC ) */

#if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CHACHA20_POLY1305_CIPHER ) && defined( USE_SHA256_MAC )
const cipher_suite_t TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256 = { 0xcc15, &dhe_rsa_keyscheme_driver, &chacha20_poly1305_cipher_driver, &sha256_mac_driver };
#endif /* if defined( USE_DHE_RSA_KEYSCHEME ) && defined( USE_CHACHA20_POLY1305    _CIPHER ) && defined( USE_SHA256_MAC ) */

