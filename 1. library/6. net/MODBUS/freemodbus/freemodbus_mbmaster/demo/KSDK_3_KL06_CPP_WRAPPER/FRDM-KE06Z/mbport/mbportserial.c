/* 
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */


/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <string.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include "Cpu.h"
#include "AS1.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/
#define IDX_INVALID				( 255 )
#define UART_BAUDRATE_MIN		( 300 )
#define UART_BAUDRATE_MAX		( 115200 )
#define TIMEOUT_INVALID         ( 255 )

#define HDL_RESET( x ) do { \
	( x )->ubIdx = IDX_INVALID; \
	( x )->pbMBPTransmitterEmptyFN = NULL; \
	( x )->pvMBPReceiveFN = NULL; \
	( x )->xMBMHdl = MB_HDL_INVALID; \
	( x )->pbMBPTimerExpiredFN = NULL; \
	( x )->eSerialMode = MB_RTU; \
	( x )->dev = NULL; \
	memset( ( x )->rxBuffers, 0, sizeof( ( x )->rxBuffers ) ); \
	( x )->bHasData = 0; \
	( x )->ubTimeoutCnt = TIMEOUT_INVALID; \
	( x )->ubTimeoutReloadCnt = TIMEOUT_INVALID; \
	( x )->ubCurrentRcvBuffer = 0; \
	( x )->bHasTimeout = 0; \
	( x )->usLastByteCnt = 0; \
	memset( &( x )->txBuffer, 0, sizeof( ( x )->txBuffer ) ); \
	( x )->bCanTx = 0; \
} while( 0 );

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE           arubBuffer[32];
    UBYTE           arubBufferPos;
} xSerialBuf;

typedef struct
{
    UBYTE           ubIdx;
    pbMBPSerialTransmitterEmptyAPIV2CB pbMBPTransmitterEmptyFN;
    pvMBPSerialReceiverAPIV2CB pvMBPReceiveFN;
    xMBHandle       xMBMHdl;
    pbMBPTimerExpiredCB pbMBPTimerExpiredFN;
    eMBSerialMode   eSerialMode;

    /* PE UART handle */
    LDD_TDeviceData *dev;

    /* RX needs to be double buffered */
    xSerialBuf      rxBuffers[2];
    BOOL            bHasData;
    /* For UART timeout detection */
    UBYTE           ubTimeoutCnt;
    UBYTE           ubTimeoutReloadCnt;
    UBYTE           ubCurrentRcvBuffer;
    BOOL            bHasTimeout;
    USHORT          usLastByteCnt;

    /* TX */
    xSerialBuf      txBuffer;
    BOOL            bCanTx;
} xSerialHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xSerialHandle xSerialHdls[1];
STATIC BOOL     bIsInitalized = FALSE;

/* ----------------------- Static functions ---------------------------------*/
STATIC void
vMBPSerialSwitchBuffer( xSerialHandle * hdl, BaseType_t *pxHigherPriorityTaskWoken );

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPSerialInit( xMBPSerialHandle * pxSerialHdl, UCHAR ucPort, ULONG ulBaudRate,
                UCHAR ucDataBits, eMBSerialParity eParity, UCHAR ucStopBits, xMBHandle xMBMHdl, pbMBPTimerExpiredCB pbFrameTimeoutFN, eMBSerialMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    UBYTE           ubIdx;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( !bIsInitalized )
    {
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
        {
            HDL_RESET( &xSerialHdls[ubIdx] );
        }
        bIsInitalized = TRUE;
    }

    if( NULL == pxSerialHdl )
    {
        eStatus = MB_EINVAL;
    }
    else if( ( ucPort < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID == xSerialHdls[ucPort].ubIdx ) )
    {
        if( MB_HDL_INVALID != xMBMHdl )
        {
        	LDD_TDeviceData *dev;
        	switch( ucPort )
        	{
        	case 0:
        		dev = AS1_Init( ( LDD_TUserData * )ucPort );
        		break;
        	default:
        		dev = NULL;
        		break;
        	}

        	if( NULL != dev )
        	{
        		eStatus = MB_ENOERR;
        		switch( ulBaudRate )
        		{
        		case 9600:
        			AS1_SelectBaudRate( dev, 0 );
        			break;
        		case 19200:
        			AS1_SelectBaudRate( dev, 1 );
        			break;
        		case 38400:
        			AS1_SelectBaudRate( dev, 2 );
        			break;
        		case 57600:
        			AS1_SelectBaudRate( dev, 3 );
        			break;
        		default:
        			eStatus = MB_EINVAL;
        		}
        		switch( eParity )
        		{
        		case MB_PAR_ODD:
        			( void )AS1_SetParity( dev, LDD_SERIAL_PARITY_ODD );
        			break;
        		case MB_PAR_EVEN:
        			( void )AS1_SetParity( dev, LDD_SERIAL_PARITY_ODD );
        			break;
        		case MB_PAR_NONE:
        			( void )AS1_SetParity( dev, LDD_SERIAL_PARITY_ODD );
        			break;
        		default:
        			eStatus = MB_EINVAL;
        			break;
        		}
        		switch( ucDataBits )
        		{
        		case 8:
        			( void )AS1_SetDataWidth( dev, 8 );
        			break;
        		case 7:
        			( void )AS1_SetDataWidth( dev, 7 );
					break;
        		default:
        			eStatus = MB_EINVAL;
        			break;
        		}
        		switch( ucStopBits )
        		{
        		case 1:
        			( void )AS1_SetStopBitLength( dev, LDD_SERIAL_STOP_BIT_LEN_1 );
        			break;
        		case 2:
        			( void )AS1_SetStopBitLength( dev, LDD_SERIAL_STOP_BIT_LEN_2 );
        			break;
        		default:
        			eStatus = MB_EINVAL;
        			break;
        		}
        		if( MB_ENOERR == eStatus )
        		{
					xSerialHdls[ucPort].ubIdx = ucPort;
					xSerialHdls[ucPort].xMBMHdl = xMBMHdl;
					xSerialHdls[ucPort].pbMBPTimerExpiredFN = pbFrameTimeoutFN;
					xSerialHdls[ucPort].eSerialMode = eMode;
					xSerialHdls[ucPort].dev = dev;
					xSerialHdls[ucPort].ubTimeoutReloadCnt = ( UBYTE ) ( 38500UL / ulBaudRate );
					if( xSerialHdls[ucPort].ubTimeoutReloadCnt < 2 )
					{
						xSerialHdls[ucPort].ubTimeoutReloadCnt = 2;
					}
					xSerialHdls[ucPort].ubTimeoutCnt = TIMEOUT_INVALID;
					vMBPSerialSwitchBuffer( &xSerialHdls[ucPort], NULL );
					*pxSerialHdl = &xSerialHdls[ucPort];
        		}
        		else
        		{
        			AS1_Deinit( dev );
        		}
        	}
        	else
        	{
        		eStatus = MB_ENORES;
        	}
        }
        else
        {
            eStatus = MB_EINVAL;
        }
    }
    else
    {
        eStatus = MB_ENORES;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPSerialClose( xMBPSerialHandle xSerialHdl )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
    	AS1_Deinit( pxSerialIntHdl->dev );
    	HDL_RESET( pxSerialIntHdl );
        eStatus = MB_ENOERR;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPSerialTxEnable( xMBPSerialHandle xSerialHdl, pbMBPSerialTransmitterEmptyCB pbMBPTransmitterEmptyFN )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;
        if( NULL != pbMBPTransmitterEmptyFN )
        {
            pxSerialIntHdl->pbMBPTransmitterEmptyFN = pbMBPTransmitterEmptyFN;
        }
        else
        {
            pxSerialIntHdl->pbMBPTransmitterEmptyFN = NULL;
        }
        pxSerialIntHdl->bCanTx = TRUE;
        BaseType_t status = xSemaphoreGive( g_xModbusSem );
        MBP_ASSERT( pdTRUE == status );
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPSerialRxEnable( xMBPSerialHandle xSerialHdl, pvMBPSerialReceiverCB pvMBPReceiveFN )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xSerialHandle  *pxSerialIntHdl = xSerialHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        eStatus = MB_ENOERR;
        if( NULL != pvMBPReceiveFN )
        {
            MBP_ASSERT( NULL == pxSerialIntHdl->pvMBPReceiveFN );
            pxSerialIntHdl->pvMBPReceiveFN = pvMBPReceiveFN;
        }
        else
        {
            pxSerialIntHdl->pvMBPReceiveFN = NULL;
        }
        BaseType_t status = xSemaphoreGive( g_xModbusSem );
        MBP_ASSERT( pdTRUE == status );
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

STATIC xSerialBuf *
vMBPSerialGetReadBuffer( xSerialHandle * hdl )
{
    return 1 == hdl->ubCurrentRcvBuffer ? &hdl->rxBuffers[0] : &hdl->rxBuffers[1];
}

STATIC void
vMBPSerialSwitchBuffer( xSerialHandle * hdl, BaseType_t *pxHigherPriorityTaskWoken )
{
    /* set flags depending if the buffer has received some data and/or if a timeout has occurred */
    if( hdl->rxBuffers[hdl->ubCurrentRcvBuffer].arubBufferPos )
    {
        hdl->bHasData = TRUE;
    }

    /* switch the buffer and enable UART reception on the second buffer */
    hdl->ubCurrentRcvBuffer = hdl->ubCurrentRcvBuffer == 1 ? 0 : 1;
    xSerialBuf *pxCurrentBuffer = &hdl->rxBuffers[hdl->ubCurrentRcvBuffer];
    pxCurrentBuffer->arubBufferPos = 0;
    AS1_ReceiveBlock( hdl->dev, pxCurrentBuffer->arubBuffer, sizeof( pxCurrentBuffer->arubBuffer ) );

    /* signal polling thread that there is new data */
    BaseType_t status = xSemaphoreGiveFromISR( g_xModbusSem, pxHigherPriorityTaskWoken );
    MBP_ASSERT( pdTRUE == status );
}

BaseType_t
prvMBPSerialTimeoutDetect( void )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if( bIsInitalized )
	{
		for( UBYTE ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
		{
			xSerialHandle *pxHdl =  &xSerialHdls[ubIdx];
			if( IDX_INVALID != pxHdl->ubIdx )
			{
				/* Decativate hardware RS485 direction control */
				if( AS1_GetTxCompleteStatus( pxHdl->dev) )
				{
					GPIO1_ClearFieldBits( GPIO1_DeviceData, RS485, 0x01 );
				}

				/* We compare the number of characters in the receive buffer to the
				 * previous value. If it has changed a character has been received
				 * and we reload the timeout counter.
				 */
				USHORT usNBytesReceived = AS1_GetReceivedDataNum( pxHdl->dev );
				if( pxHdl->usLastByteCnt != usNBytesReceived )
				{
					pxHdl->ubTimeoutCnt = pxHdl->ubTimeoutReloadCnt;
					pxHdl->usLastByteCnt = usNBytesReceived;
				}

				/* If the timeout has not expired then decrement the counter. If zero
				 * signal the stack that a frame timeout has been detected.
				 */
				if(  TIMEOUT_INVALID != pxHdl->ubTimeoutCnt )
				{
					xSerialHdls[ubIdx].ubTimeoutCnt--;
					if( 0 == xSerialHdls[ubIdx].ubTimeoutCnt )
					{
						pxHdl->ubTimeoutCnt = TIMEOUT_INVALID;
						pxHdl->bHasTimeout = TRUE;
						pxHdl->rxBuffers[pxHdl->ubCurrentRcvBuffer].arubBufferPos = AS1_GetReceivedDataNum( pxHdl->dev );
						/* Cancel current receive operation for the UART as the buffer will be switched. Reset
						 * the byte counter to avoid false triggering of a timeout event.
						 */
						AS1_CancelBlockReception( pxHdl->dev );
						pxHdl->usLastByteCnt = AS1_GetReceivedDataNum( pxHdl->dev );
						vMBPSerialSwitchBuffer( pxHdl, &xHigherPriorityTaskWoken );
					}
				}
			}
		}
	}
	return xHigherPriorityTaskWoken;
}

BaseType_t
prvMBPSerialReceiveBlock( UBYTE ubIdx )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if( ( ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID != xSerialHdls[ubIdx].ubIdx ) )
    {
    	xSerialHandle *pxHdl = &xSerialHdls[ubIdx];
    	/* The current receive double buffer for the UART is filled up. Store the
    	 * number of received characters, switch the buffers and signal processing thread
    	 * for new data
    	 */
    	pxHdl->rxBuffers[pxHdl->ubCurrentRcvBuffer].arubBufferPos = AS1_GetReceivedDataNum( pxHdl->dev );
		vMBPSerialSwitchBuffer( pxHdl, &xHigherPriorityTaskWoken );
    }
    return xHigherPriorityTaskWoken;
}

BaseType_t
prvMBPSerialCanTransmit( UBYTE ubIdx )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if( ( ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID != xSerialHdls[ubIdx].ubIdx ) )
    {
        xSerialHdls[ubIdx].bCanTx = TRUE;
        BaseType_t status = xSemaphoreGiveFromISR( g_xModbusSem, &xHigherPriorityTaskWoken );
        MBP_ASSERT( pdTRUE == status );
    }
    return xHigherPriorityTaskWoken;
}

void
prvMBPSerialPoll( void )
{
    for( UBYTE ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
    {
        if( IDX_INVALID != xSerialHdls[ubIdx].ubIdx )
        {
            xSerialHandle  *hdl = &xSerialHdls[ubIdx];
            if( hdl->bHasTimeout )
            {
                hdl->bHasTimeout = FALSE;
                MBP_ENTER_CRITICAL_SECTION(  );
                if( ( NULL != hdl->pvMBPReceiveFN ) && ( hdl->bHasData ) )
                {
                    hdl->bHasData = FALSE;
                    xSerialBuf     *buf = vMBPSerialGetReadBuffer( hdl );
                    ( void )hdl->pvMBPReceiveFN( hdl->xMBMHdl, buf->arubBuffer, buf->arubBufferPos );
                }
                if( ( NULL != hdl->pvMBPReceiveFN ) && ( NULL != hdl->pbMBPTimerExpiredFN ) )
                {
                    ( void )hdl->pbMBPTimerExpiredFN( hdl->xMBMHdl );
                }
                MBP_EXIT_CRITICAL_SECTION(  );
            }
            if( hdl->bHasData )
            {
                hdl->bHasData = FALSE;
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != hdl->pvMBPReceiveFN )
                {
                    xSerialBuf     *buf = vMBPSerialGetReadBuffer( hdl );
                    ( void )hdl->pvMBPReceiveFN( hdl->xMBMHdl, buf->arubBuffer, buf->arubBufferPos );
                }
                MBP_EXIT_CRITICAL_SECTION(  );
            }
            if( hdl->bCanTx )
            {
                hdl->bCanTx = FALSE;
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != hdl->pbMBPTransmitterEmptyFN )
                {
                    USHORT          usTxBytes;
                    BOOL            hasData =
                        hdl->pbMBPTransmitterEmptyFN( hdl->xMBMHdl, hdl->txBuffer.arubBuffer, MB_UTILS_NARRSIZE( hdl->txBuffer.arubBuffer ), &usTxBytes );
                    if( hasData )
                    {
                    	GPIO1_SetFieldBits( GPIO1_DeviceData, RS485, 0x01 );
                    	LDD_TError status = AS1_SendBlock( hdl->dev, hdl->txBuffer.arubBuffer, usTxBytes );
                        ( void )status;
                    }
                    else
                    {
                        hdl->pbMBPTransmitterEmptyFN = NULL;
                    }
                }
                MBP_EXIT_CRITICAL_SECTION(  );
            }
        }
    }
}
