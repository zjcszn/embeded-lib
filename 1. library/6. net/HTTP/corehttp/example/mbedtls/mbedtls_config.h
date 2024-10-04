/*
 *  Copyright (C) 2006-2018, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 *
 *  This file is provided under the Apache License 2.0, or the
 *  GNU General Public License v2.0 or later.
 *
 *  **********
 *  Apache License 2.0:
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  **********
 *
 *  **********
 *  GNU General Public License v2.0 or later:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  **********
 *
 *  This repository uses Mbed TLS under Apache 2.0
 */

/* This file configures mbed TLS for FreeRTOS. */

#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include <stdint.h>
#include <stddef.h>

/* Error messages and TLS debugging traces
 * (huge code size increase, needed for tests/ssl-opt.sh) */
//#define MBEDTLS_DEBUG_C
//#define MBEDTLS_ERROR_C


/* Generate errors if deprecated functions are used. */
#define MBEDTLS_DEPRECATED_REMOVED

/* Select SSL Ciphersuites */
#define MBEDTLS_SSL_CIPHERSUITES            MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256

#define MBEDTLS_MPI_MAX_SIZE                512
#define MBEDTLS_MPI_WINDOW_SIZE             2
#define MBEDTLS_ENTROPY_MAX_SOURCES         2

/* Platform support */
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HAVE_TIME
#define MBEDTLS_PLATFORM_TIME_ALT
#define MBEDTLS_HAVE_TIME_DATE
#define MBEDTLS_PLATFORM_GMTIME_R_ALT
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_PRINTF_MACRO                elog_raw

/* Enable the following cipher modes. */
//#define MBEDTLS_CIPHER_MODE_CBC
//#define MBEDTLS_CIPHER_MODE_CFB
//#define MBEDTLS_CIPHER_MODE_CTR
//#define MBEDTLS_CIPHER_MODE_OFB
//#define MBEDTLS_CIPHER_MODE_XTS

/* Enable the following cipher padding modes. */
//#define MBEDTLS_CIPHER_PADDING_PKCS7
//#define MBEDTLS_CIPHER_PADDING_ONE_AND_ZEROS
//#define MBEDTLS_CIPHER_PADDING_ZEROS_AND_LEN
//#define MBEDTLS_CIPHER_PADDING_ZEROS

/* Significant speed benefit at the expense of some ROM */
#define MBEDTLS_ECP_NIST_OPTIM

/* Cipher suite configuration. */
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_DHE_RSA_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_DHE_PSK_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_RSA_PSK_ENABLED
//#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED

/* Enable all SSL alert messages. */
#define MBEDTLS_SSL_ALL_ALERT_MESSAGES

/* Enable the following SSL features. */
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_IN_CONTENT_LEN                  (5 * 1024)
#define MBEDTLS_SSL_OUT_CONTENT_LEN                 (5 * 1024)
//#define MBEDTLS_SSL_ENCRYPT_THEN_MAC
//#define MBEDTLS_SSL_EXTENDED_MASTER_SECRET
//#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_ALPN
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
//#define MBEDTLS_SSL_TRUNCATED_HMAC
//#define MBEDTLS_X509_RSASSA_PSS_SUPPORT



/* Config ECC crypto */
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
//#define MBEDTLS_ECDSA_C
//#define MBEDTLS_ECDSA_DETERMINISTIC
//#define MBEDTLS_ECP_DP_SECP192R1_ENABLED
//#define MBEDTLS_ECP_DP_SECP224R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
//#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
//#define MBEDTLS_ECP_DP_SECP521R1_ENABLED
//#define MBEDTLS_ECP_DP_SECP192K1_ENABLED
//#define MBEDTLS_ECP_DP_SECP224K1_ENABLED
//#define MBEDTLS_ECP_DP_SECP256K1_ENABLED
//#define MBEDTLS_ECP_DP_BP256R1_ENABLED
//#define MBEDTLS_ECP_DP_BP384R1_ENABLED
//#define MBEDTLS_ECP_DP_BP512R1_ENABLED
//#define MBEDTLS_ECP_DP_CURVE25519_ENABLED

/* Enable the following mbed TLS features. */
#define MBEDTLS_AES_C
#define MBEDTLS_AES_ROM_TABLES
//#define MBEDTLS_CCM_C
#define MBEDTLS_GCM_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CTR_DRBG_C
//#define MBEDTLS_DES_C
//#define MBEDTLS_DHM_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_MD_C
//#define MBEDTLS_MD5_C
#define MBEDTLS_OID_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_RSA_C
#define MBEDTLS_SHA1_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA384_C
//#define MBEDTLS_SHA512_C
#define MBEDTLS_THREADING_ALT
#define MBEDTLS_THREADING_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C

#include "mbedtls/check_config.h"

#endif /* ifndef MBEDTLS_CONFIG_H */
