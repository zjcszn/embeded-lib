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

#define LOG_TAG "TCP"
#define LOG_LVL ELOG_LVL_INFO
/**
 * @file sockets_wrapper.c
 * @brief FreeRTOS Sockets connect and disconnect wrapper implementation.
 */

/* Include header that defines log levels. */
#include "corelog.h"

/* Standard includes. */

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* LwIP includes. */
#include "lwip/arch.h"
#include "lwip/errno.h"
#include "lwip/sockets.h"
#include "lwip/api.h"

/* TCP Sockets Wrapper include.*/
/* Let sockets wrapper know that Socket_t is defined already. */
#include "sockets_wrapper.h"

/**
 * @brief Maximum number of times to call FreeRTOS_recv when initiating a graceful shutdown.
 */
#ifndef FREERTOS_SOCKETS_WRAPPER_SHUTDOWN_LOOPS
#define FREERTOS_SOCKETS_WRAPPER_SHUTDOWN_LOOPS         (  3 )
#endif

/**
 * @brief negative error code indicating a network failure.
 */
#define FREERTOS_SOCKETS_WRAPPER_NETWORK_ERROR          ( -1 )

/**
 * @brief Establish a connection to server.
 *
 * @param[out] pTcpSocket The output parameter to return the created socket descriptor.
 * @param[in] pHostName Server hostname to connect to.
 * @param[in] pServerInfo Server port to connect to.
 * @param[in] receiveTimeoutMs Timeout (in milliseconds) for transport receive.
 * @param[in] sendTimeoutMs Timeout (in milliseconds) for transport send.
 *
 * @note A timeout of 0 means infinite timeout.
 *
 * @return Non-zero value on error, 0 on success.
 */
BaseType_t TCP_Sockets_Connect(Socket_t *pTcpSocket,
                               const char *pHostName,
                               uint16_t port,
                               uint32_t receiveTimeoutMs,
                               uint32_t sendTimeoutMs) {
    Socket_t tcpSocket = INVALID_SOCKET;
    BaseType_t socketStatus = 0;
    struct sockaddr_in serverAddress = {0};
    struct timeval xTransportTimeout = {0};
    ip4_addr_t resolveIP = {0};
    
    configASSERT(pTcpSocket != NULL);
    configASSERT(pHostName != NULL);
    
    /* Create a new TCP socket. */
    tcpSocket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (tcpSocket == INVALID_SOCKET) {
        LogError(("Failed to create new socket."));
        socketStatus = FREERTOS_SOCKETS_WRAPPER_NETWORK_ERROR;
    } else {
        LogDebug(("Created new TCP socket."));
        
        /* Connection parameters. */
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = PP_HTONS(port);
        serverAddress.sin_len = (uint8_t)sizeof(serverAddress);
        
        /* Check for errors from DNS lookup. */
        if ((netconn_gethostbyname(pHostName, &resolveIP) == ERR_OK) &&
            resolveIP.addr != 0) {
            serverAddress.sin_addr.s_addr = (in_addr_t)resolveIP.addr;
        } else {
            LogError(("Failed to connect to server: DNS resolution failed: Hostname=%s.",
                pHostName));
            socketStatus = FREERTOS_SOCKETS_WRAPPER_NETWORK_ERROR;
        }
    }
    
    if (socketStatus == 0) {
        /* Establish connection. */
        LogDebug(("Creating TCP Connection to %s.", pHostName));
        socketStatus = lwip_connect(tcpSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        
        if (socketStatus != 0) {
            LogError(("Failed to connect to server: lwip_connect failed: ReturnCode=%d,"
                      " Hostname=%s, Port=%u.",
                socketStatus,
                pHostName,
                port));
        } else {
            /* Set socket receive timeout. */
            xTransportTimeout.tv_sec = (long)receiveTimeoutMs / 1000;
            xTransportTimeout.tv_usec = ((long)receiveTimeoutMs % 1000) * 1000;
            /* Setting the receiving block time cannot fail. */
            (void)lwip_setsockopt(tcpSocket,
                                  SOL_SOCKET,
                                  SO_RCVTIMEO,
                                  &xTransportTimeout,
                                  sizeof(struct timeval));
            
            /* Set socket send timeout. */
            xTransportTimeout.tv_sec = (long)sendTimeoutMs / 1000;
            xTransportTimeout.tv_usec = ((long)sendTimeoutMs % 1000) * 1000;
            /* Setting the send block time cannot fail. */
            (void)lwip_setsockopt(tcpSocket,
                                  SOL_SOCKET,
                                  SO_SNDTIMEO,
                                  &xTransportTimeout,
                                  sizeof(struct timeval));
        }
    }
    
    if (socketStatus == 0) {
        /* Set the socket. */
        *pTcpSocket = tcpSocket;
        LogInfo(("Established TCP connection with %s.", pHostName));
    } else {
        /* Clean up on failure. */
        if (tcpSocket != INVALID_SOCKET) {
            (void)lwip_close(tcpSocket);
            tcpSocket = INVALID_SOCKET;
        }
    }
    
    return socketStatus;
}

/**
 * @brief End connection to server.
 *
 * @param[in] tcpSocket The socket descriptor.
 */
void TCP_Sockets_Disconnect(Socket_t tcpSocket) {
    BaseType_t waitForShutdownLoopCount = 0;
    uint8_t pDummyBuffer[2];
    
    if (tcpSocket != INVALID_SOCKET) {
        /* Initiate graceful shutdown. */
        (void)lwip_shutdown(tcpSocket, SHUT_RDWR);
        
        /* Wait for the socket to disconnect gracefully (indicated by FreeRTOS_recv()
         * returning a FREERTOS_EINVAL error) before closing the socket. */
        while (lwip_recv(tcpSocket, pDummyBuffer, sizeof(pDummyBuffer), 0) >= 0) {
            /* We don't need to delay since FreeRTOS_recv should already have a timeout. */
            
            if (++waitForShutdownLoopCount >= FREERTOS_SOCKETS_WRAPPER_SHUTDOWN_LOOPS) {
                break;
            }
        }
        (void)lwip_close(tcpSocket);
    }
}

/**
 * @brief Transmit data to the remote socket.
 *
 * The socket must have already been created using a call to TCP_Sockets_Connect().
 *
 * @param[in] xSocket The handle of the sending socket.
 * @param[in] pvBuffer The buffer containing the data to be sent.
 * @param[in] xDataLength The length of the data to be sent.
 *
 * @return
 * * On success, the number of bytes actually sent is returned.
 * * If an error occurred, a negative value is returned. @ref SocketsErrors
 */
int32_t TCP_Sockets_Send(Socket_t xSocket,
                         const void *pvBuffer,
                         size_t xBufferLength) {
    int xReturnStatus = TCP_SOCKETS_ERRNO_ERROR;
    
    configASSERT(pvBuffer != NULL);
    
    xReturnStatus = lwip_send(xSocket, pvBuffer, xBufferLength, 0);
    
    if (xReturnStatus < 0) {
        switch (errno) {
            /* A timeout occurred before any data could be sent as the TCP buffer was full. */
            case EAGAIN:
                xReturnStatus = TCP_SOCKETS_ERRNO_EWOULDBLOCK;
                break;
                
                /* Not enough memory for the socket to create either a Rx or Tx stream. */
            case ENOMEM:
                xReturnStatus = TCP_SOCKETS_ERRNO_ENOMEM;
                break;
                
                /* Socket received a signal, causing the read operation to be aborted. */
            case ENOTCONN:
                xReturnStatus = TCP_SOCKETS_ERRNO_ENOTCONN;
                break;
            
            case ECONNRESET:
                xReturnStatus = TCP_SOCKETS_ERRNO_PERIPHERAL_RESET;
            
            default:
                xReturnStatus = TCP_SOCKETS_ERRNO_ERROR;
                break;
        }
    }
    
    return xReturnStatus;
}

/**
 * @brief Receive data from a TCP socket.
 *
 * The socket must have already been created using a call to TCP_Sockets_Connect().
 *
 * @param[in] xSocket The handle of the socket from which data is being received.
 * @param[out] pvBuffer The buffer into which the received data will be placed.
 * @param[in] xBufferLength The maximum number of bytes which can be received.
 * pvBuffer must be at least xBufferLength bytes long.
 *
 * @return
 * * If the receive was successful then the number of bytes received (placed in the
 *   buffer pointed to by pvBuffer) is returned.
 * * If a timeout occurred before data could be received then 0 is returned (timeout
 *   is set using @ref SOCKETS_SO_RCVTIMEO).
 * * If an error occurred, a negative value is returned. @ref SocketsErrors
 */
int32_t TCP_Sockets_Recv(Socket_t xSocket,
                         void *pvBuffer,
                         size_t xBufferLength) {
    int xReturnStatus = TCP_SOCKETS_ERRNO_ERROR;
    
    configASSERT(pvBuffer != NULL);
    
    xReturnStatus = lwip_recv(xSocket, pvBuffer, xBufferLength, 0);
    
    if (xReturnStatus < 0) {
        switch (errno) {
            /* A timeout occurred before any data could be sent as the TCP buffer was full. */
            case EAGAIN:
                xReturnStatus = TCP_SOCKETS_ERRNO_EWOULDBLOCK;
                break;
                
                /* Not enough memory for the socket to create either a Rx or Tx stream. */
            case ENOMEM:
                xReturnStatus = TCP_SOCKETS_ERRNO_ENOMEM;
                break;
                
                /* Socket received a signal, causing the read operation to be aborted. */
            case ENOTCONN:
                xReturnStatus = TCP_SOCKETS_ERRNO_ENOTCONN;
                break;
            
            case ECONNRESET:
                xReturnStatus = TCP_SOCKETS_ERRNO_PERIPHERAL_RESET;
            
            default:
                xReturnStatus = TCP_SOCKETS_ERRNO_ERROR;
                break;
        }
    }
    
    return xReturnStatus;
}
