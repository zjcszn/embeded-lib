/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/**
 * @file mbedtls_bio_tcp_sockets_wrapper.c
 * @brief Implements mbed TLS platform send/receive functions for the TCP sockets wrapper.
 */


/* MbedTLS includes. */
#if !defined( MBEDTLS_CONFIG_FILE )
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif
#include "mbedtls/ssl.h"

/* TCP Sockets Wrapper include.*/
#include "sockets_wrapper.h"

/* Lwip include */
#include "lwip/errno.h"
#include "lwip/sockets.h"

/* MbedTLS Bio TCP sockets wrapper include. */
#include "mbedtls_bio_wrapper.h"

/**
 * @brief Sends data over TCP socket.
 *
 * @param[in] ctx The network context containing the socket handle.
 * @param[in] buf Buffer containing the bytes to send.
 * @param[in] len Number of bytes to send from the buffer.
 *
 * @return Number of bytes sent on success; else a negative value.
 */
int xMbedTLSBioTCPSocketsWrapperSend(void *ctx,
                                     const unsigned char *buf,
                                     size_t len) {
    int xReturnStatus;
    
    configASSERT(ctx != NULL);
    configASSERT(buf != NULL);
    
    Socket_t tcpSocket = *((Socket_t *)ctx);
    xReturnStatus = lwip_send(tcpSocket, buf, len, 0);
    
    if (xReturnStatus < 0) {
        if (errno == EAGAIN) {
            /* A timeout occurred before any data could be sent. */
            xReturnStatus = MBEDTLS_ERR_SSL_TIMEOUT;
        } else {
            xReturnStatus = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
        }
    }
    return xReturnStatus;
}

/**
 * @brief Receives data from TCP socket.
 *
 * @param[in] ctx The network context containing the socket handle.
 * @param[out] buf Buffer to receive bytes into.
 * @param[in] len Number of bytes to receive from the network.
 *
 * @return Number of bytes received if successful; Negative value on error.
 */
int xMbedTLSBioTCPSocketsWrapperRecv(void *ctx,
                                     unsigned char *buf,
                                     size_t len) {
    int xReturnStatus;
    
    configASSERT(ctx != NULL);
    configASSERT(buf != NULL);
    
    Socket_t tcpSocket = *((Socket_t *)ctx);
    xReturnStatus = lwip_recv(tcpSocket, buf, len, 0);
    
    if (xReturnStatus < 0) {
        if (errno == EAGAIN) {
            /* A timeout occurred before any data could be sent. */
            xReturnStatus = MBEDTLS_ERR_SSL_TIMEOUT;
        } else {
            xReturnStatus = MBEDTLS_ERR_SSL_INTERNAL_ERROR;
        }
    }
    return xReturnStatus;
}
