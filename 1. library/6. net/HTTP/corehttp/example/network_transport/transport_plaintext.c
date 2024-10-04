/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* Logging configuration for the Sockets. */
#define LOG_TAG "TLS"
#define LOG_LVL ELOG_LVL_INFO

/* Include header that defines log levels. */
#include "corelog.h"

/* Standard includes. */
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* Transport interface include. */
#include "transport_plaintext.h"

/*-----------------------------------------------------------*/

/**
 * @brief Each compilation unit that consumes the NetworkContext must define it.
 * It should contain a single pointer as seen below whenever the header file
 * of this transport implementation is included to your project.
 *
 * @note When using multiple transports in the same compilation unit,
 *       define this pointer as void *.
 */
// struct NetworkContext
//{
//     PlaintextTransportParams_t * pParams;
// };

/*-----------------------------------------------------------*/

PlaintextTransportStatus_t Network_Plaintext_Connect(NetworkContext_t *pNetworkContext,
													 const char *pHostName, uint16_t port,
													 uint32_t receiveTimeoutMs, uint32_t sendTimeoutMs) {
	PlaintextTransportStatus_t plaintextStatus = PLAINTEXT_TRANSPORT_SUCCESS;
	BaseType_t socketStatus = 0;

	if ((pNetworkContext == NULL) || (pHostName == NULL)) {
		LogError(("Invalid input parameter(s): Arguments cannot be NULL. "
				  "pNetworkContext=%p, "
				  "pHostName=%p.",
			pNetworkContext, pHostName));
		plaintextStatus = PLAINTEXT_TRANSPORT_INVALID_PARAMETER;
	} else {

		/* Initialize tcpSocket. */
		pNetworkContext->tcpSocket = NULL;

		/* Establish a TCP connection with the server. */
		socketStatus = TCP_Sockets_Connect(&(pNetworkContext->tcpSocket), pHostName,
										   port, receiveTimeoutMs, sendTimeoutMs);

		/* A non zero status is an error. */
		if (socketStatus != 0) {
			LogError(("Failed to connect to %s with error %d.", pHostName, socketStatus));
			plaintextStatus = PLAINTEXT_TRANSPORT_CONNECT_FAILURE;
		}
	}

	return plaintextStatus;
}

PlaintextTransportStatus_t
Network_Plaintext_Disconnect(const NetworkContext_t *pNetworkContext) {
	PlaintextTransportStatus_t plaintextStatus = PLAINTEXT_TRANSPORT_SUCCESS;

	if ((pNetworkContext == NULL) ||
		(pNetworkContext->tcpSocket == INVALID_SOCKET)) {
		LogError(("pNetworkContext cannot be NULL."));
		plaintextStatus = PLAINTEXT_TRANSPORT_INVALID_PARAMETER;
	} else {
		/* Call socket disconnect function to close connection. */
		TCP_Sockets_Disconnect(pNetworkContext->tcpSocket);
	}

	return plaintextStatus;
}

int32_t Network_Plaintext_Recv(NetworkContext_t *pNetworkContext, void *pBuffer,
							   size_t bytesToRecv) {
	int32_t socketStatus = 1;

	if ((pNetworkContext == NULL) || (pNetworkContext->tcpSocket == INVALID_SOCKET)) {
		LogError(("invalid input, pNetworkContext=%p", pNetworkContext));
		socketStatus = -1;
	} else if (pBuffer == NULL) {
		LogError(("invalid input, pBuffer == NULL"));
		socketStatus = -1;
	} else if (bytesToRecv == 0) {
		LogError(("invalid input, bytesToRecv == 0"));
		socketStatus = -1;
	} else {
		socketStatus = TCP_Sockets_Recv(pNetworkContext->tcpSocket, pBuffer, bytesToRecv);
	}

	return socketStatus;
}

int32_t Network_Plaintext_Send(NetworkContext_t *pNetworkContext,
							   const void *pBuffer, size_t bytesToSend) {
	int32_t socketStatus = 0;

	if ((pNetworkContext == NULL) || (pNetworkContext->tcpSocket == INVALID_SOCKET)) {
		LogError(("invalid input, pNetworkContext=%p", pNetworkContext));
		socketStatus = -1;
	} else if (pBuffer == NULL) {
		LogError(("invalid input, pBuffer == NULL"));
		socketStatus = -1;
	} else if (bytesToSend == 0) {
		LogError(("invalid input, bytesToSend == 0"));
		socketStatus = -1;
	} else {
		socketStatus = TCP_Sockets_Send(pNetworkContext->tcpSocket, pBuffer, bytesToSend);
	}

	return socketStatus;
}
