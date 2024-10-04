/*
 * MODBUS Library: LINUX/CYGWIN serial to UDP gateway
 * Copyright (c) 2015 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbportserial.c,v 1.8 2014-08-23 09:36:06 embedded-solutions.cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/
#define DEMO_BUILD           ( 0 )
#define IDX_INVALID          ( 255 )
#define UART_BAUDRATE_MIN    ( 9600 )
#define SER_MAX_INSTANCES    ( 8 )
#define SER_BUFFER_SIZE      ( 513 )
#define SER_TIMEOUT          ( 5UL )
#define THREAD_STACKSIZE_MAX ( 16384 )

#define HDL_RESET( x )                                            \
    do                                                            \
    {                                                             \
        ( x )->ubIdx = IDX_INVALID;                               \
        ( x )->pbMBPSerialTransmitterEmptyFN = NULL;              \
        ( x )->pvMBPSerialReceiverFN = NULL;                      \
        ( x )->pbMBPFrameTimeoutFN = NULL;                        \
        ( x )->bIsRunning = FALSE;                                \
        memset( &( ( x )->xThreadHdl ), 0, sizeof( pthread_t ) ); \
        memset( &( ( x )->xIdleSem ), 0, sizeof( sem_t ) );       \
        ( x )->xSocket = -1;                                      \
        ( x )->xMBHdl = MB_HDL_INVALID;                           \
    } while( 0 )

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE                              ubIdx;
    pbMBPSerialTransmitterEmptyAPIV2CB pbMBPSerialTransmitterEmptyFN;
    pvMBPSerialReceiverAPIV2CB         pvMBPSerialReceiverFN;
    pbMBPTimerExpiredCB                pbMBPFrameTimeoutFN;
    BOOL                               bIsRunning;
    pthread_t                          xThreadHdl;
    sem_t                              xIdleSem;
    int                                xSocket;
    UBYTE                              ubSendPrefix;
    xMBHandle                          xMBHdl;
} xSerialHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xSerialHandle xSerialHdls[SER_MAX_INSTANCES];
STATIC BOOL          bIsInitalized = FALSE;

/* ----------------------- Static functions ---------------------------------*/
STATIC void *
pvMBPSerialHandlerThread( void *pvArg );

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPSerialInit( xMBPSerialHandle *pxSerialHdl, UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits,
                eMBSerialParity eParity, UCHAR ucStopBits, xMBHandle xMBHdl, pbMBPTimerExpiredCB pbFrameTimeoutFN,
                eMBSerialMode eMode )
{
    UBYTE          ubIdx;
    eMBErrorCode   eStatus = MB_EINVAL;
    pthread_attr_t xThreadAttr;
    xSerialHandle *pxSerHdl = NULL;

    MBP_ENTER_CRITICAL_SECTION();
    if( !bIsInitalized )
    {
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
        {
            HDL_RESET( &xSerialHdls[ubIdx] );
        }
        bIsInitalized = TRUE;
    }

    for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
    {
        if( IDX_INVALID == xSerialHdls[ubIdx].ubIdx )
        {
            pxSerHdl = &( xSerialHdls[ubIdx] );

            HDL_RESET( pxSerHdl );
            pxSerHdl->ubIdx = ubIdx;
            break;
        }
    }
    if( NULL != pxSerHdl )
    {
        eStatus = MB_ENOERR;
        pxSerHdl->bIsRunning = TRUE;
        pxSerHdl->xMBHdl = xMBHdl;
        pxSerHdl->pbMBPFrameTimeoutFN = pbFrameTimeoutFN;
        switch( ulBaudRate )
        {
            case 115200:
                pxSerHdl->ubSendPrefix = '0';
                break;
            case 57600:
                pxSerHdl->ubSendPrefix = '1';
                break;
            case 38400:
                pxSerHdl->ubSendPrefix = '2';
                break;
            case 19200:
                pxSerHdl->ubSendPrefix = '3';
                break;
            case 9600:
                pxSerHdl->ubSendPrefix = '4';
                break;
            case 4800:
                pxSerHdl->ubSendPrefix = '5';
                break;
            case 2400:
                pxSerHdl->ubSendPrefix = '6';
                break;
            case 1200:
                pxSerHdl->ubSendPrefix = '7';
                break;
            default:
                eStatus = MB_EINVAL;
                break;
        }
        if( MB_ENOERR == eStatus )
        {
            if( -1 == ( pxSerHdl->xSocket = socket( PF_INET6, SOCK_DGRAM, 0 ) ) )
            {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't create socket: %s\n", strerror( errno ) );
                }
#endif
            }
            else if( -1 == sem_init( &pxSerHdl->xIdleSem, 1, 0 ) )
            {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't create semaphore: %s\n", strerror( errno ) );
                }
#endif
                eStatus = MB_EPORTERR;
            }
            else if( 0 != pthread_attr_init( &xThreadAttr ) )
            {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't initialize thread attributes: %s\n",
                                 strerror( errno ) );
                }
#endif
                eStatus = MB_EPORTERR;
            }
            else if( 0 != pthread_attr_setdetachstate( &xThreadAttr, PTHREAD_CREATE_DETACHED ) )
            {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't set thread attributes: %s\n",
                                 strerror( errno ) );
                }
#endif
            }
            else if( 0 !=
                     pthread_create( &( pxSerHdl->xThreadHdl ), &xThreadAttr, pvMBPSerialHandlerThread, pxSerHdl ) )
            {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't create polling thread: %s\n",
                                 strerror( errno ) );
                }
#endif
                eStatus = MB_EPORTERR;
            }
            else
            {
                *pxSerialHdl = pxSerHdl;
                eStatus = MB_ENOERR;
            }
            ( void )pthread_attr_destroy( &xThreadAttr );
        }

        if( MB_ENOERR != eStatus )
        {
            if( -1 != pxSerHdl->xSocket )
            {
                close( pxSerHdl->xSocket );
            }
            ( void )sem_destroy( &( pxSerHdl->xIdleSem ) );
            HDL_RESET( pxSerHdl );
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }

    MBP_EXIT_CRITICAL_SECTION();
    return eStatus;
}

eMBErrorCode
eMBPSerialClose( xMBPSerialHandle xSerialHdl )
{
    eMBErrorCode   eStatus = MB_EINVAL;
    xSerialHandle *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION();
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
        if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
        {
            vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] Closing handle.\n", pxSerialIntHdl->ubIdx );
        }
#endif
        pxSerialIntHdl->bIsRunning = FALSE;
        eStatus = MB_ENOERR;
        ( void )sem_post( &( pxSerialIntHdl->xIdleSem ) );
    }
    MBP_EXIT_CRITICAL_SECTION();
    return eStatus;
}

eMBErrorCode
eMBPSerialTxEnable( xMBPSerialHandle xSerialHdl, pbMBPSerialTransmitterEmptyCB pbMBPTransmitterEmptyFN )
{
    eMBErrorCode   eStatus = MB_EINVAL;
    xSerialHandle *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION();
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) && pxSerialIntHdl->bIsRunning )
    {
        eStatus = MB_ENOERR;
        if( NULL != pbMBPTransmitterEmptyFN )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] enabled transmitter callback %p\n",
                             pxSerialIntHdl->ubIdx, pbMBPTransmitterEmptyFN );
            }
#endif

            pxSerialIntHdl->pbMBPSerialTransmitterEmptyFN =
                ( pbMBPSerialTransmitterEmptyAPIV2CB )pbMBPTransmitterEmptyFN;
        }
        else
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] disabled transmitter callback\n",
                             pxSerialIntHdl->ubIdx );
            }
#endif

            pxSerialIntHdl->pbMBPSerialTransmitterEmptyFN = NULL;
        }
        ( void )sem_post( &( pxSerialIntHdl->xIdleSem ) );
    }
    MBP_EXIT_CRITICAL_SECTION();
    return eStatus;
}

eMBErrorCode
eMBPSerialRxEnable( xMBPSerialHandle xSerialHdl, pvMBPSerialReceiverCB pvMBPReceiveFN )
{
    eMBErrorCode   eStatus = MB_EINVAL;
    xSerialHandle *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION();
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) && pxSerialIntHdl->bIsRunning )
    {
        eStatus = MB_ENOERR;
        if( NULL != pvMBPReceiveFN )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] enabled receiver callback %p\n",
                             pxSerialIntHdl->ubIdx, pvMBPReceiveFN );
            }
#endif
            pxSerialIntHdl->pvMBPSerialReceiverFN = ( pvMBPSerialReceiverAPIV2CB )pvMBPReceiveFN;
        }
        else
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] disabled receiver callback at\n",
                             pxSerialIntHdl->ubIdx );
            }
#endif

            pxSerialIntHdl->pvMBPSerialReceiverFN = NULL;
        }
        ( void )sem_post( &( pxSerialIntHdl->xIdleSem ) );
    }
    MBP_EXIT_CRITICAL_SECTION();
    return eStatus;
}

void
vMBPSerialDLLClose( void )
{
    /* If this code is called we know that no MODBUS instances are
     * newly created (Because of the INIT lock) and that all
     * still running serial threads are going to shut down since
     * their bIsRunning flag has been set to FALSE.
     */
    MBP_ENTER_CRITICAL_SECTION();
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
    if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
    {
        vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "Unloading library\n" );
    }
#endif
    /* Nothing to do here for now. */
    MBP_EXIT_CRITICAL_SECTION();
}

void *
pvMBPSerialHandlerThread( void *pvArg )
{
    UBYTE          arubBuffer[SER_BUFFER_SIZE];
    xSerialHandle *pxSerHdl = pvArg;
    USHORT         usBytesToSend;

    struct sockaddr_in6 server;

    do
    {
        MBP_ENTER_CRITICAL_SECTION();
        if( NULL != pxSerHdl->pbMBPSerialTransmitterEmptyFN )
        {
            usBytesToSend = 0;
            if( !pxSerHdl->pbMBPSerialTransmitterEmptyFN( pxSerHdl->xMBHdl, &arubBuffer[1], SER_BUFFER_SIZE - 1,
                                                          &usBytesToSend ) )
            {
                pxSerHdl->pbMBPSerialTransmitterEmptyFN = NULL;
            }
            MBP_EXIT_CRITICAL_SECTION();
            if( usBytesToSend > 0 )
            {
                arubBuffer[0] = pxSerHdl->ubSendPrefix;
                memset( &server, 0, sizeof( server ) );
                server.sin6_family = AF_INET6;
                inet_pton( AF_INET6, "::1", &server.sin6_addr );
                server.sin6_port = htons( 2000 );
                ssize_t bytes_sent = sendto( pxSerHdl->xSocket, arubBuffer, usBytesToSend + 1, MSG_DONTWAIT,
                                             ( struct sockaddr * )&server, sizeof( server ) );
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] sent %d bytes: %d\n", pxSerHdl->ubIdx,
                                 bytes_sent, errno );
                }
#endif
            }
        }
        else if( NULL != pxSerHdl->pvMBPSerialReceiverFN )
        {
            MBP_EXIT_CRITICAL_SECTION();
            struct timeval tv;
            fd_set         read_fds;

            FD_ZERO( &read_fds );
            FD_SET( pxSerHdl->xSocket, &read_fds );
            tv.tv_sec = 0;
            tv.tv_usec = 10000;

            if( select( pxSerHdl->xSocket + 1, &read_fds, NULL, NULL, &tv ) > 0 )
            {
                if( FD_ISSET( pxSerHdl->xSocket, &read_fds ) )
                {
                    ssize_t bytes_read = recvfrom( pxSerHdl->xSocket, arubBuffer, SER_BUFFER_SIZE, 0, NULL, NULL );
                    if( bytes_read > 0 )
                    {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                        if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
                        {
                            vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] received %d bytes\n", pxSerHdl->ubIdx,
                                         bytes_read );
                        }
#endif
                        MBP_ENTER_CRITICAL_SECTION();
                        if( pxSerHdl->bIsRunning && ( NULL != pxSerHdl->pvMBPSerialReceiverFN ) )
                        {
                            pxSerHdl->pvMBPSerialReceiverFN( pxSerHdl->xMBHdl, arubBuffer, bytes_read );
                            if( NULL != pxSerHdl->pbMBPFrameTimeoutFN )
                            {
                                pxSerHdl->pbMBPFrameTimeoutFN( pxSerHdl->xMBHdl );
                            }
                        }
                        MBP_EXIT_CRITICAL_SECTION();
                    }
                }
            }
        }
        else
        {
            MBP_EXIT_CRITICAL_SECTION();
            sem_wait( &( pxSerHdl->xIdleSem ) );
        }
    } while( pxSerHdl->bIsRunning );
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
    if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
    {
        vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] closing serial handler thread\n", pxSerHdl->ubIdx );
    }
#endif

    ( void )close( pxSerHdl->xSocket );
    ( void )sem_destroy( &( pxSerHdl->xIdleSem ) );
    MBP_ENTER_CRITICAL_SECTION();
    HDL_RESET( pxSerHdl );
    MBP_EXIT_CRITICAL_SECTION();

    pthread_exit( NULL );

    return NULL;
}
