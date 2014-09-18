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
 *  Defines cryptography functions for encryption, decryption and hashing
 *
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include "crypto_structures.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 * @cond               Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define AES_ENCRYPT     1
#define AES_DECRYPT     0

#define DES_ENCRYPT     1
#define DES_DECRYPT     0

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
 *              Global Variables
 ******************************************************/

/******************************************************
 *              Function Declarations
 * @endcond
 ******************************************************/

/*****************************************************************************/
/** @defgroup crypto       Crypto functions
 *
 *  WICED Cryptography functions
 */
/*****************************************************************************/


/*****************************************************************************/
/** @addtogroup aes       AES
 *  @ingroup crypto
 *
 * AES functions
 *
 *
 *  @{
 */
/*****************************************************************************/

/**
 * @brief          AES key schedule (encryption)
 *
 * @param ctx      AES context to be initialized
 * @param key      encryption key
 * @param keysize  must be 128, 192 or 256
 */
void aes_setkey_enc( aes_context_t *ctx, const unsigned char *key, int32_t keysize );

/**
 * @brief          AES key schedule (decryption)
 *
 * @param ctx      AES context to be initialized
 * @param key      decryption key
 * @param keysize  must be 128, 192 or 256
 */
void aes_setkey_dec( aes_context_t *ctx, const unsigned char *key, int32_t keysize );

/**
 * @brief          AES-ECB block encryption/decryption
 *
 * @param ctx      AES context
 * @param mode     AES_ENCRYPT or AES_DECRYPT
 * @param input    16-byte input block
 * @param output   16-byte output block
 */
void aes_crypt_ecb( aes_context_t *ctx, int32_t mode, const unsigned char input[16], unsigned char output[16] );

/**
 * @brief          AES-CBC buffer encryption/decryption
 *
 * @param ctx      AES context
 * @param mode     AES_ENCRYPT or AES_DECRYPT
 * @param length   length of the input data
 * @param iv       initialization vector (updated after use)
 * @param input    buffer holding the input data
 * @param output   buffer holding the output data
 */
void aes_crypt_cbc( aes_context_t *ctx, int32_t mode, int32_t length, unsigned char iv[16], const unsigned char *input, unsigned char *output );

/**
 * @brief          AES-CFB128 buffer encryption/decryption
 *
 * @param ctx      AES context
 * @param mode     AES_ENCRYPT or AES_DECRYPT
 * @param length   length of the input data
 * @param iv_off   offset in IV (updated after use)
 * @param iv       initialization vector (updated after use)
 * @param input    buffer holding the input data
 * @param output   buffer holding the output data
 */
void aes_crypt_cfb128( aes_context_t *ctx, int32_t mode, int32_t length, int32_t *iv_off, unsigned char iv[16], const unsigned char *input, unsigned char *output );

/**
 * @brief
 *
 * @param rk
 * @param key_len
 * @param nonce
 * @param aad_len
 * @param aad
 * @param data_len
 * @param ptxt
 * @param ctxt
 * @param mac
 */
int aes_ccm_encrypt( uint32_t *rk, const size_t key_len, const uint8_t *nonce, const size_t aad_len, const uint8_t *aad, const size_t data_len, const uint8_t *ptxt, uint8_t *ctxt, uint8_t *mac );

/**
 * @brief
 *
 * @param rk
 * @param key_len
 * @param nonce
 * @param aad_len
 * @param aad
 * @param data_len
 * @param ctxt
 * @param ptxt
 */
int aes_ccm_decrypt( uint32_t *rk, const size_t key_len, const uint8_t *nonce, const size_t aad_len, const uint8_t *aad, const size_t data_len, const uint8_t *ctxt, uint8_t *ptxt );

/** @} */

/*****************************************************************************/
/** @addtogroup des       DES
 *  @ingroup crypto
 *
 * DES functions
 *
 *  @{
 */
/*****************************************************************************/

/**
 * @brief          DES key schedule (56-bit, encryption)
 *
 * @param ctx      DES context to be initialized
 * @param key      8-byte secret key
 */
void des_setkey_enc( des_context *ctx, unsigned char key[8] );

/**
 * @brief          DES key schedule (56-bit, decryption)
 *
 * @param ctx      DES context to be initialized
 * @param key      8-byte secret key
 */
void des_setkey_dec( des_context *ctx, unsigned char key[8] );

/**
 * @brief          Triple-DES key schedule (112-bit, encryption)
 *
 * @param ctx      3DES context to be initialized
 * @param key      16-byte secret key
 */
void des3_set2key_enc( des3_context *ctx, unsigned char key[16] );

/**
 * @brief          Triple-DES key schedule (112-bit, decryption)
 *
 * @param ctx      3DES context to be initialized
 * @param key      16-byte secret key
 */
void des3_set2key_dec( des3_context *ctx, unsigned char key[16] );

/**
 * @brief          Triple-DES key schedule (168-bit, encryption)
 *
 * @param ctx      3DES context to be initialized
 * @param key      24-byte secret key
 */
void des3_set3key_enc( des3_context *ctx, unsigned char key[24] );

/**
 * @brief          Triple-DES key schedule (168-bit, decryption)
 *
 * @param ctx      3DES context to be initialized
 * @param key      24-byte secret key
 */
void des3_set3key_dec( des3_context *ctx, unsigned char key[24] );

/**
 * @brief          DES-ECB block encryption/decryption
 *
 * @param ctx      DES context
 * @param input    64-bit input block
 * @param output   64-bit output block
 */
void des_crypt_ecb( des_context *ctx, unsigned char input[8], unsigned char output[8] );

/**
 * @brief          DES-CBC buffer encryption/decryption
 *
 * @param ctx      DES context
 * @param mode     DES_ENCRYPT or DES_DECRYPT
 * @param length   length of the input data
 * @param iv       initialization vector (updated after use)
 * @param input    buffer holding the input data
 * @param output   buffer holding the output data
 */
void des_crypt_cbc( des_context *ctx, int32_t mode, int32_t length, unsigned char iv[8], unsigned char *input, unsigned char *output );

/**
 * @brief          3DES-ECB block encryption/decryption
 *
 * @param ctx      3DES context
 * @param input    64-bit input block
 * @param output   64-bit output block
 */
void des3_crypt_ecb( des3_context *ctx, unsigned char input[8], unsigned char output[8] );

/**
 * @brief          3DES-CBC buffer encryption/decryption
 *
 * @param ctx      3DES context
 * @param mode     DES_ENCRYPT or DES_DECRYPT
 * @param length   length of the input data
 * @param iv       initialization vector (updated after use)
 * @param input    buffer holding the input data
 * @param output   buffer holding the output data
 */
void des3_crypt_cbc( des3_context *ctx, int32_t mode, int32_t length, unsigned char iv[8], unsigned char *input, unsigned char *output );

/** @} */

/*****************************************************************************/
/** @addtogroup sha1       SHA1
 *  @ingroup crypto
 *
 * SHA1 functions
 *
 *  @{
 */
/*****************************************************************************/

/**
 * @brief          SHA-1 context setup
 *
 * @param ctx      context to be initialized
 */
void sha1_starts( sha1_context *ctx );

/**
 * @brief          SHA-1 process buffer
 *
 * @param ctx      SHA-1 context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void sha1_update( sha1_context *ctx, unsigned char *input, int32_t ilen );

/**
 * @brief          SHA-1 final digest
 *
 * @param ctx      SHA-1 context
 * @param output   SHA-1 checksum result
 */
void sha1_finish( sha1_context *ctx, unsigned char output[20] );

/**
 * @brief          Output = SHA-1( input buffer )
 *
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   SHA-1 checksum result
 */
void sha1( unsigned char *input, int32_t ilen, unsigned char output[20] );

/**
 * @brief          Output = SHA-1( file contents )
 *
 * @param path     input file name
 * @param output   SHA-1 checksum result
 *
 * @return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int32_t sha1_file( char *path, unsigned char output[20] );

/**
 * @brief          SHA-1 HMAC context setup
 *
 * @param ctx      HMAC context to be initialized
 * @param key      HMAC secret key
 * @param keylen   length of the HMAC key
 */
void sha1_hmac_starts( sha1_context *ctx, unsigned char *key, int32_t keylen );

/**
 * @brief          SHA-1 HMAC process buffer
 *
 * @param ctx      HMAC context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void sha1_hmac_update( sha1_context *ctx, unsigned char *input, int32_t ilen );

/**
 * @brief          SHA-1 HMAC final digest
 *
 * @param ctx      HMAC context
 * @param output   SHA-1 HMAC checksum result
 */
void sha1_hmac_finish( sha1_context *ctx, unsigned char output[20] );

/**
 * @brief          Output = HMAC-SHA-1( hmac key, input buffer )
 *
 * @param key      HMAC secret key
 * @param keylen   length of the HMAC key
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   HMAC-SHA-1 result
 */
void sha1_hmac( unsigned char *key, int32_t keylen, unsigned char *input, int32_t ilen, unsigned char output[20] );

/** @} */

/*****************************************************************************/
/** @addtogroup sha256       SHA256
 *  @ingroup crypto
 *
 * SHA256 functions
 *
 *  @{
 */
/*****************************************************************************/


/**
 * @brief          SHA-256 context setup
 *
 * @param ctx      context to be initialized
 * @param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2_starts( sha2_context *ctx, int32_t is224 );

/**
 * @brief          SHA-256 process buffer
 *
 * @param ctx      SHA-256 context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void sha2_update( sha2_context *ctx, unsigned char *input, int32_t ilen );

/**
 * @brief          SHA-256 final digest
 *
 * @param ctx      SHA-256 context
 * @param output   SHA-224/256 checksum result
 */
void sha2_finish( sha2_context *ctx, unsigned char output[32] );

/**
 * @brief          Output = SHA-256( input buffer )
 *
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   SHA-224/256 checksum result
 * @param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2( unsigned char *input, int32_t ilen, unsigned char output[32], int32_t is224 );

/**
 * @brief          Output = SHA-256( file contents )
 *
 * @param path     input file name
 * @param output   SHA-224/256 checksum result
 * @param is224    0 = use SHA256, 1 = use SHA224
 *
 * @return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int32_t sha2_file( char *path, unsigned char output[32], int32_t is224 );

/**
 * @brief          SHA-256 HMAC context setup
 *
 * @param ctx      HMAC context to be initialized
 * @param key      HMAC secret key
 * @param keylen   length of the HMAC key
 * @param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2_hmac_starts( sha2_context *ctx, unsigned char *key, int32_t keylen, int32_t is224 );

/**
 * @brief          SHA-256 HMAC process buffer
 *
 * @param ctx      HMAC context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void sha2_hmac_update( sha2_context *ctx, unsigned char *input, int32_t ilen );

/**
 * @brief          SHA-256 HMAC final digest
 *
 * @param ctx      HMAC context
 * @param output   SHA-224/256 HMAC checksum result
 */
void sha2_hmac_finish( sha2_context *ctx, unsigned char output[32] );

/**
 * @brief          Output = HMAC-SHA-256( hmac key, input buffer )
 *
 * @param key      HMAC secret key
 * @param keylen   length of the HMAC key
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   HMAC-SHA-224/256 result
 * @param is224    0 = use SHA256, 1 = use SHA224
 */
void sha2_hmac( unsigned char *key, int32_t keylen, unsigned char *input, int32_t ilen, unsigned char output[32], int32_t is224 );

/** @} */

/*****************************************************************************/
/** @addtogroup md5       MD5
 *  @ingroup crypto
 *
 * MD5 functions
 *
 *  @{
 */
/*****************************************************************************/

/**
 * @brief          MD5 context setup
 *
 * @param ctx      context to be initialized
 */
void md5_starts( md5_context *ctx );

/**
 * @brief          MD5 process buffer
 *
 * @param ctx      MD5 context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void md5_update( md5_context *ctx, unsigned char *input, int32_t ilen );

/**
 * @brief          MD5 final digest
 *
 * @param ctx      MD5 context
 * @param output   MD5 checksum result
 */
void md5_finish( md5_context *ctx, unsigned char output[16] );

/**
 * @brief          Output = MD5( input buffer )
 *
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   MD5 checksum result
 */
void md5( unsigned char *input, int32_t ilen, unsigned char output[16] );

/**
 * @brief          Output = MD5( file contents )
 *
 * @param path     input file name
 * @param output   MD5 checksum result
 *
 * @return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int32_t md5_file( char *path, unsigned char output[16] );

/**
 * @brief          MD5 HMAC context setup
 *
 * @param ctx      HMAC context to be initialized
 * @param key      HMAC secret key
 * @param keylen   length of the HMAC key
 */
void md5_hmac_starts( md5_context *ctx, unsigned char *key, int32_t keylen );

/**
 * @brief          MD5 HMAC process buffer
 *
 * @param ctx      HMAC context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void md5_hmac_update( md5_context *ctx, unsigned char *input, int32_t ilen );

/**
 * @brief          MD5 HMAC final digest
 *
 * @param ctx      HMAC context
 * @param output   MD5 HMAC checksum result
 */
void md5_hmac_finish( md5_context *ctx, unsigned char output[16] );

/**
 * @brief          Output = HMAC-MD5( hmac key, input buffer )
 *
 * @param key      HMAC secret key
 * @param keylen   length of the HMAC key
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 * @param output   HMAC-MD5 result
 */
void md5_hmac( unsigned char *key, int32_t keylen, unsigned char *input, int32_t ilen, unsigned char output[16] );

/** @} */

/*****************************************************************************/
/** @addtogroup arc4       ARC4
 *  @ingroup crypto
 *
 * ARC4 functions
 *
 *  @{
 */
/*****************************************************************************/

/**
 * @brief          ARC4 key schedule
 *
 * @param ctx      ARC4 context to be initialized
 * @param key      the secret key
 * @param keylen   length of the key
 */
void arc4_setup( arc4_context *ctx, unsigned char *key, int32_t keylen );

/**
 * @brief          ARC4 cipher function
 *
 * @param ctx      ARC4 context
 * @param buf      buffer to be processed
 * @param buflen   amount of data in buf
 */
void arc4_crypt( arc4_context *ctx, unsigned char *buf, int32_t buflen );

/** @} */

/*****************************************************************************/
/** @addtogroup rsa       RSA
 *  @ingroup crypto
 *
 * RSA functions
 *
 *  @{
 */
/*****************************************************************************/


/**
 * @brief          Initialize an RSA context
 *
 * @param ctx      RSA context to be initialized
 * @param padding  RSA_PKCS_V15 or RSA_PKCS_V21
 * @param hash_id  RSA_PKCS_V21 hash identifier
 * @param f_rng    RNG function
 * @param p_rng    RNG parameter
 *
 * @note           The hash_id parameter is actually ignored when using RSA_PKCS_V15 padding.
 * @note           RSA_PKCS_V21 padding is not supported.
 */
void rsa_init( rsa_context *ctx, int32_t padding, int32_t hash_id, int32_t (*f_rng)( void * ), void *p_rng );

/**
 * @brief          Generate an RSA keypair
 *
 * @param ctx      RSA context that will hold the key
 * @param nbits    size of the public key in bits
 * @param exponent public exponent (e.g., 65537)
 *
 * @note           rsa_init() must be called beforehand to setup the RSA context (especially f_rng and p_rng).
 *
 * @return         0 if successful
 */
int32_t rsa_gen_key( rsa_context *ctx, int32_t nbits, int32_t exponent );

/**
 * @brief          Check a public RSA key
 *
 * @param ctx      RSA context to be checked
 *
 * @return         0 if successful
 */
int32_t rsa_check_pubkey( rsa_context *ctx );

/**
 * @brief          Check a private RSA key
 *
 * @param ctx      RSA context to be checked
 *
 * @return         0 if successful
 */
int32_t rsa_check_privkey( rsa_context *ctx );

/**
 * @brief          Do an RSA public key operation
 *
 * @param ctx      RSA context
 * @param input    input buffer
 * @param output   output buffer
 *
 * @return         0 if successful
 *
 * @note           This function does NOT take care of message padding. Also, be sure to set input[0] = 0.
 * @note           The input and output buffers must be large enough (eg. 128 bytes if RSA-1024 is used).
 */
int32_t rsa_public( rsa_context *ctx, unsigned char *input, unsigned char *output );

/**
 * @brief          Do an RSA private key operation
 *
 * @param ctx      RSA context
 * @param input    input buffer
 * @param output   output buffer
 *
 * @return         0 if successful
 *
 * @note           The input and output buffers must be large enough (eg. 128 bytes if RSA-1024 is used).
 */
int32_t rsa_private( rsa_context *ctx, unsigned char *input, unsigned char *output );

/**
 * @brief          Add the message padding, then do an RSA operation
 *
 * @param ctx      RSA context
 * @param mode     RSA_PUBLIC or RSA_PRIVATE
 * @param ilen     contains the the plaintext length
 * @param input    buffer holding the data to be encrypted
 * @param output   buffer that will hold the ciphertext
 *
 * @return         0 if successful
 *
 * @note           The output buffer must be as large as the size of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int32_t rsa_pkcs1_encrypt( rsa_context *ctx, int32_t mode, int32_t ilen, unsigned char *input, unsigned char *output );

/**
 * @brief          Do an RSA operation, then remove the message padding
 *
 * @param ctx      RSA context
 * @param mode     RSA_PUBLIC or RSA_PRIVATE
 * @param input    buffer holding the encrypted data
 * @param output   buffer that will hold the plaintext
 * @param olen     will contain the plaintext length
 * @param output_max_len    maximum length of the output buffer
 *
 * @return         0 if successful
 *
 * @note           The output buffer must be as large as the size of ctx->N (eg. 128 bytes if RSA-1024 is used)
 */
int32_t rsa_pkcs1_decrypt( rsa_context *ctx, int32_t mode, int32_t *olen, unsigned char *input, unsigned char *output, int32_t output_max_len );

/**
 * @brief          Do a private RSA to sign a message digest
 *
 * @param ctx      RSA context
 * @param mode     RSA_PUBLIC or RSA_PRIVATE
 * @param hash_id  RSA_RAW, RSA_MD{2,4,5} or RSA_SHA{1,256}
 * @param hashlen  message digest length (for RSA_RAW only)
 * @param hash     buffer holding the message digest
 * @param sig      buffer that will hold the ciphertext
 *
 * @return         0 if the signing operation was successful
 *
 * @note           The "sig" buffer must be as large as the size of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int32_t rsa_pkcs1_sign( rsa_context *ctx, int32_t mode, int32_t hash_id, int32_t hashlen, unsigned char *hash, unsigned char *sig );

/**
 * @brief          Do a public RSA and check the message digest
 *
 * @param ctx      points to an RSA public key
 * @param mode     RSA_PUBLIC or RSA_PRIVATE
 * @param hash_id  RSA_RAW, RSA_MD{2,4,5} or RSA_SHA{1,256}
 * @param hashlen  message digest length (for RSA_RAW only)
 * @param hash     buffer holding the message digest
 * @param sig      buffer holding the ciphertext
 *
 * @return         0 if the verify operation was successful
 *
 * @note           The "sig" buffer must be as large as the size of ctx->N (eg. 128 bytes if RSA-1024 is used).
 */
int32_t rsa_pkcs1_verify( rsa_context *ctx, int32_t mode, int32_t hash_id, int32_t hashlen, unsigned char *hash, unsigned char *sig );

/**
 * @brief          Free the components of an RSA key
 */
void rsa_free( rsa_context *ctx );


/** @} */


#ifdef __cplusplus
} /*extern "C" */
#endif
