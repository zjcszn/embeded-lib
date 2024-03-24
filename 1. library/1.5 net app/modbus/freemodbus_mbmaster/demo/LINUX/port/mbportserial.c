/*
 * MODBUS Library: Linux port
 * Copyright (c) 2009-2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <termios.h>
#include <sys/select.h>

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
#define UART_BAUDRATE_MIN    ( 300 )
#define UART_BAUDRATE_MAX    ( 115200 )
#define SER_MAX_INSTANCES    ( 8 )
#define SER_BUFFER_SIZE      ( 32 )
#define SER_TIMEOUT          ( 5UL )
#define THREAD_STACKSIZE_MAX ( 16384 )

#define HDL_RESET( x )                               \
    do                                               \
    {                                                \
        ( x )->ubIdx = IDX_INVALID;                  \
        ( x )->xSerHdl = -1;                         \
        ( x )->pbMBPSerialTransmitterEmptyFN = NULL; \
        ( x )->pvMBPSerialReceiverFN = NULL;         \
        ( x )->xMBHdl = MB_HDL_INVALID;              \
        ( x )->bIsRunning = FALSE;                   \
    } while( 0 )

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE                              ubIdx;
    pbMBPSerialTransmitterEmptyAPIV2CB pbMBPSerialTransmitterEmptyFN;
    pvMBPSerialReceiverAPIV2CB         pvMBPSerialReceiverFN;
    BOOL                               bIsRunning;
    pthread_t                          xThreadHdl;
    int                                xSerHdl;
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
                eMBSerialParity eParity, UCHAR ucStopBits, xMBHandle xMBHdl )
{
    UBYTE          ubIdx;
    CHAR           arcDevice[16];
    CHAR *         paDeviceOverride;
    eMBErrorCode   eStatus = MB_EINVAL;
    struct termios xNewTIO;
    speed_t        xNewSpeed;
    pthread_attr_t xThreadAttr;
    BOOL           bThreadAttrInitialzed = FALSE;
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
        memset( &xNewTIO, 0, sizeof( struct termios ) );
        xNewTIO.c_iflag |= IGNBRK | INPCK;
        xNewTIO.c_cflag |= CREAD | CLOCAL;
        xNewTIO.c_oflag = 0;
        xNewTIO.c_lflag = 0;
        switch( eParity )
        {
            case MB_PAR_NONE:
                break;
            case MB_PAR_EVEN:
                xNewTIO.c_cflag |= PARENB;
                break;
            case MB_PAR_ODD:
                xNewTIO.c_cflag |= PARENB | PARODD;
                break;
            default:
                eStatus = MB_EINVAL;
        }

        switch( ucDataBits )
        {
            case 8:
                xNewTIO.c_cflag |= CS8;
                break;
            case 7:
                xNewTIO.c_cflag |= CS7 | CSTOPB;
                break;
            default:
                eStatus = MB_EINVAL;
        }

        switch( ulBaudRate )
        {
            case 9600:
                xNewSpeed = B9600;
                break;
            case 19200:
                xNewSpeed = B19200;
                break;
            case 38400:
                xNewSpeed = B38400;
                break;
            case 57600:
                xNewSpeed = B57600;
                break;
            case 115200:
                xNewSpeed = B115200;
                break;
            default:
                eStatus = MB_EINVAL;
        }

        pxSerHdl->bIsRunning = TRUE;
        pxSerHdl->xMBHdl = xMBHdl;

        paDeviceOverride = getenv( "EE_MBM_DEVICE" );

        if( paDeviceOverride == NULL )
        {
            snprintf( arcDevice, MB_UTILS_NARRSIZE( arcDevice ), "/dev/ttyS%d", ( int )ucPort );
        }
        else
        {
            strncpy( arcDevice, paDeviceOverride, MB_UTILS_NARRSIZE( arcDevice ) );
        }
        arcDevice[MB_UTILS_NARRSIZE( arcDevice )] = 0;

        if( -1 == ( pxSerHdl->xSerHdl = open( arcDevice, O_RDWR | O_NOCTTY ) ) )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't open serial port %s: %s\n", arcDevice,
                             strerror( errno ) );
            }
#endif

            eStatus = MB_EPORTERR;
        }
        else if( cfsetispeed( &xNewTIO, xNewSpeed ) != 0 )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't set baud rate %ld for port %s: %s\n", ulBaudRate,
                             arcDevice, strerror( errno ) );
            }
#endif
            eStatus = MB_EPORTERR;
        }
        else if( cfsetospeed( &xNewTIO, xNewSpeed ) != 0 )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't set baud rate %ld for port %s: %s\n", ulBaudRate,
                             arcDevice, strerror( errno ) );
            }
#endif
            eStatus = MB_EPORTERR;
        }
        else if( tcsetattr( pxSerHdl->xSerHdl, TCSANOW, &xNewTIO ) != 0 )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't set settings for port %s: %s\n", arcDevice,
                             strerror( errno ) );
            }
#endif
            eStatus = MB_EPORTERR;
        }
        else if( (0 != pthread_attr_init( &xThreadAttr )) && (bThreadAttrInitialzed = TRUE /* wanted side-effect */) )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't initialize thread attributes: %s",
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
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't set thread attributes: %s\n", strerror( errno ) );
            }
#endif
        }
        else if( 0 != pthread_create( &( pxSerHdl->xThreadHdl ), &xThreadAttr, pvMBPSerialHandlerThread, pxSerHdl ) )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_ERROR, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't create polling thread: %s\n", strerror( errno ) );
            }
#endif
            eStatus = MB_EPORTERR;
        }
        else
        {
            *pxSerialHdl = pxSerHdl;
            eStatus = MB_ENOERR;
        }
        if( bThreadAttrInitialzed ) 
	{
		( void )pthread_attr_destroy( &xThreadAttr );
	}

        if( MB_ENOERR != eStatus )
        {
            ( void )close( pxSerHdl->xSerHdl );
            HDL_RESET( pxSerHdl );
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }

    MBP_EXIT_CRITICAL_SECTION();
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
    if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
    {
        vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "serial init done\n" );
    }
#endif
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
    UBYTE              arubBuffer[SER_BUFFER_SIZE];
    xSerialHandle *    pxSerHdl = pvArg;
    int                xSerDevHdl;
    BOOL               bIsRunning = TRUE;
    int                iRetval;
    int                iBytesRead;
    ssize_t            iBytesSent;
    size_t             iBytesToSend;
    USHORT             usBytesToSend;
    fd_set             rd_fds, wr_fds, err_fds;
    struct timeval     tv;
    struct sched_param xSchedParam;

    if( 0 != sched_getparam( getpid(), &xSchedParam ) )
    {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
        if( bMBPPortLogIsEnabled( MB_LOG_WARN, MB_LOG_PORT_SERIAL ) )
        {
            vMBPPortLog( MB_LOG_WARN, MB_LOG_PORT_SERIAL, "Can't get scheduler parameters: %s\n", strerror( errno ) );
        }
#endif
    }
    else
    {
        xSchedParam.sched_priority = sched_get_priority_max( SCHED_RR );
        if( 0 != sched_setscheduler( getpid(), SCHED_RR, &xSchedParam ) )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_WARN, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_ERROR, MB_LOG_PORT_SERIAL, "Can't raise priority: %s\n", strerror( errno ) );
            }
#endif
        }
    }

    do
    {
        FD_ZERO( &rd_fds );
        FD_ZERO( &wr_fds );
        FD_ZERO( &err_fds );

        MBP_ENTER_CRITICAL_SECTION();
        bIsRunning = pxSerHdl->bIsRunning;
        xSerDevHdl = pxSerHdl->xSerHdl;

        if( NULL != pxSerHdl->pvMBPSerialReceiverFN )
        {
            FD_SET( xSerDevHdl, &rd_fds );
        }
        if( NULL != pxSerHdl->pbMBPSerialTransmitterEmptyFN )
        {
            FD_SET( xSerDevHdl, &wr_fds );
        }
        FD_SET( xSerDevHdl, &err_fds );
        MBP_EXIT_CRITICAL_SECTION();

        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        iRetval = select( xSerDevHdl + 1, &rd_fds, &wr_fds, &err_fds, &tv );
        if( iRetval > 0 )
        {
            /* If receiver is enabled poll for data. */
            if( FD_ISSET( xSerDevHdl, &rd_fds ) )
            {
                iBytesRead = read( xSerDevHdl, arubBuffer, SER_BUFFER_SIZE );
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] read %d bytes (errno=%d)\n", pxSerHdl->ubIdx,
                                 iBytesRead, errno );
                }
#endif
                if( iBytesRead > 0 )
                {
                    MBP_ENTER_CRITICAL_SECTION();
                    if( pxSerHdl->bIsRunning && ( NULL != pxSerHdl->pvMBPSerialReceiverFN ) )
                    {
                        pxSerHdl->pvMBPSerialReceiverFN( pxSerHdl->xMBHdl, arubBuffer, ( USHORT )iBytesRead );
                    }
                    MBP_EXIT_CRITICAL_SECTION();
                }
                else
                {
                    if( ( errno != EINTR ) && ( errno != EAGAIN ) )
                    {
                        pxSerHdl->bIsRunning = FALSE;
                    }
                }
            }

            if( FD_ISSET( xSerDevHdl, &wr_fds ) )
            {
                iBytesSent = 0;
                iBytesToSend = 0;
                MBP_ENTER_CRITICAL_SECTION();
                if( pxSerHdl->bIsRunning && ( NULL != pxSerHdl->pbMBPSerialTransmitterEmptyFN ) )
                {
                    if( !pxSerHdl->pbMBPSerialTransmitterEmptyFN( pxSerHdl->xMBHdl, arubBuffer, SER_BUFFER_SIZE,
                                                                  &usBytesToSend ) )
                    {
                        pxSerHdl->pbMBPSerialTransmitterEmptyFN = NULL;
                        iBytesToSend = 0;
                    }
                    else
                    {
                        iBytesToSend = ( size_t )usBytesToSend;
                    }
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                    if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
                    {
                        vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] transmitter callback returned %hu bytes\n",
                                     pxSerHdl->ubIdx, usBytesToSend );
                    }
#endif
                }
                MBP_EXIT_CRITICAL_SECTION();
                while( iBytesToSend > 0 )
                {
                    iBytesSent = write( xSerDevHdl, arubBuffer, iBytesToSend );
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                    if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
                    {
                        vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] wrote %d of %d bytes\n", pxSerHdl->ubIdx,
                                     iBytesSent, ( int )iBytesToSend );
                    }
#endif
                    if( iBytesSent > 0 )
                    {
                        iBytesToSend -= iBytesSent;
                    }
                    else
                    {
                        if( ( errno != EINTR ) && ( errno != EAGAIN ) )
                        {
                            pxSerHdl->bIsRunning = FALSE;
                            break;
                        }
                    }
                }
            }

            if( FD_ISSET( pxSerHdl->xSerHdl, &err_fds ) )
            {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
                if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
                {
                    vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] serial handle failed\n", pxSerHdl->ubIdx );
                }
#endif
                MBP_ENTER_CRITICAL_SECTION();
                pxSerHdl->bIsRunning = FALSE;
                MBP_EXIT_CRITICAL_SECTION();
            }
        }
        else if( iRetval < 0 )
        {
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] select returned with %d (errno=%d)\n",
                             pxSerHdl->ubIdx, iRetval, errno );
            }
#endif
        }

    } while( bIsRunning );
#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
    if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
    {
        vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "[%d] closing serial handler thread\n", pxSerHdl->ubIdx,
                     iBytesSent, ( int )iBytesToSend );
    }
#endif

    ( void )close( pxSerHdl->xSerHdl );
    MBP_ENTER_CRITICAL_SECTION();
    HDL_RESET( pxSerHdl );
    MBP_EXIT_CRITICAL_SECTION();

    pthread_exit( NULL );

    return NULL;
}
