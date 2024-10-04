/* 
 * MODBUS Library: STM32 CubeMX, FreeRTOS port
 * Copyright (c)  2016 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbportserial.c 1074 2017-03-07 10:48:48Z cwalter $
 */
/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <math.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include "usart.h"
#include "tim.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbframe.h"
#include "common/mbutils.h"
#include "common/mbportlayer.h"

/* ----------------------- Defines ------------------------------------------*/
#define MBP_SERIAL_TASK_PRIORITY          	( MBP_TASK_PRIORITY )
#define MBP_SERIAL_TASK_STACKSIZE           ( 256 )
#define MBP_SERIAL_BUFFER_SIZE	            ( 16 )

#define IDX_INVALID                         ( 255 )

#define PORT_0_INIT_FUNC										MX_USART4_UART_Init
#define PORT_0_DEINIT_FUNC									HAL_UART_MspDeInit

#define TXRX_BUFFER_LEN											( 16 )
#define RX_FIFO_LEN													( 32 )

#define HDL_RESET( x )						do { \
	( x )->ubIdx = IDX_INVALID; \
	( x )->xMBHdl = FALSE; \
	( x )->pbMBPTransmitterEmptyFN = NULL; \
	( x )->pvMBPReceiveFN = NULL; \
	( x )->pvMBPTimerExpiredFN = NULL; \
	( x )->xQueue = NULL; \
	( x )->pubCurrentRXPtr = NULL; \
	( x )->usCurrentRXLen = 0; \
	( x )->pubCurrentWRPtr = NULL; \
	( x )->usCurrentWRLen = 0; \
	memset( ( x )->arubTXRXDoubleBuffers, 0, sizeof( ( x )->arubTXRXDoubleBuffers ) ); \
	memset( ( x )->arubRXFIFO, 0, sizeof( ( x )->arubRXFIFO ) ); \
	( x )->usIdleTime = 0xFFFFU; \
	( x )->usIdleTimeMax = 0xFFFFU; \
	( x )->usFIFORxPos = 0; \
} while( 0 )

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
    MSG_END_OF_FRAME,
    MSG_NEW_TX_FRAME,
    MSG_DMA_SEND_COMPLETE,
    MSG_DMA_RECEIVE_COMPLETE,
		MSG_DMA_TX_OVERFLOW,
    MSG_SHUTDOWN
} xQueueMsg;

typedef struct
{
    UBYTE           ubIdx;
    xMBHandle       xMBHdl;
    pbMBPSerialTransmitterEmptyAPIV2CB pbMBPTransmitterEmptyFN;
    pvMBPSerialReceiverAPIV2CB pvMBPReceiveFN;
    pbMBPTimerExpiredCB pvMBPTimerExpiredFN;

    QueueHandle_t   xQueue;
    /* These buffers are used to communicate new received data to the serial handler task.
     * The current write buffer (pubCurrentWRPtr) is always filled with new data from the FIFO
     * whereas the current read buffer (pubCurrentRXPtr) can be read by the task. The buffers
     * always point to one of the elements within the double buffer arubTXRXDoubleBuffers
     */
    UBYTE          *pubCurrentRXPtr;
    USHORT          usCurrentRXLen;
    UBYTE          *pubCurrentWRPtr;
    USHORT          usCurrentWRLen;
    UBYTE           arubTXRXDoubleBuffers[2][TXRX_BUFFER_LEN];

    /* The STM32 supports a circular DMA. The content of the FIFO is checked every 750us
     * within a very short ISR and if there is new data it is taken from the FIFO and buffered
     * for later use by the MODBUS stack. It has to be ensured that the FIFO has enough space
     * because at 115200 baud the FIFO already receives around 8 characters.
     */
    UBYTE           arubRXFIFO[RX_FIFO_LEN];    /* this is written by the DMA of the UART */
    USHORT          usIdleTime; /* number of intervals (750us) in which no character has been received */
    USHORT          usIdleTimeMax;      /* T3.5 character timeout */
    USHORT          usFIFORxPos;        /* read position for FIFO written by DMA */
} xSerialHandle;

typedef struct
{
    UART_HandleTypeDef *handle;
    USART_TypeDef  *usart;
    GPIO_TypeDef *sw_driver_enable;	/* if there is a software driver enable pin */
    uint16_t sw_pin;
} xSerialHWHandle;

/* ----------------------- Static functions ---------------------------------*/
STATIC void     vMBPSerialHandlerTask( void *pvArg );
STATIC void 		vMBPNextBuffer( UBYTE ucPort, BOOL copyToRx );
extern void     HAL_UART_MspInit( UART_HandleTypeDef * uartHandle );
extern void     HAL_UART_MspDeInit( UART_HandleTypeDef * uartHandle );

/* ----------------------- Static variables ---------------------------------*/
STATIC BOOL     bIsInitalized = FALSE;
extern UART_HandleTypeDef huart2;

STATIC xSerialHWHandle xSerialHWHdls[] = { {.handle = &huart2,.usart = USART2,.sw_driver_enable = NULL}, {.handle = &huart4,.usart = USART4,.sw_driver_enable = NULL} };

STATIC xSerialHandle xSerialHdls[MB_UTILS_NARRSIZE( xSerialHWHdls )];

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPSerialInit( xMBPSerialHandle * pxSerialHdl, UCHAR ucPort, ULONG ulBaudRate,
                UCHAR ucDataBits, eMBSerialParity eParity, UCHAR ucStopBits, xMBHandle xMBHdl, pbMBPTimerExpiredCB pbFrameTimeoutFN, eMBSerialMode eMode )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    UBYTE           ubIdx;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( !bIsInitalized )
    {
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
        {
            HDL_RESET( &xSerialHdls[ubIdx] );
        }
        bIsInitalized = TRUE;
        MX_TIM3_Init(  );
        HAL_TIM_Base_Start_IT( &htim3 );
    }

    if( ( ucPort < MB_UTILS_NARRSIZE( xSerialHdls ) ) && ( IDX_INVALID == xSerialHdls[ucPort].ubIdx ) )
    {
        HDL_RESET( &xSerialHdls[ucPort] );
        eStatus = MB_ENOERR;

        xSerialHWHdls[ucPort].handle->Instance = xSerialHWHdls[ucPort].usart;
        xSerialHWHdls[ucPort].handle->Init.BaudRate = ulBaudRate;
        switch ( ucDataBits )
        {
        case 7:
						xSerialHWHdls[ucPort].handle->Init.WordLength = eParity == MB_PAR_NONE ? UART_WORDLENGTH_7B : UART_WORDLENGTH_8B;
            break;
        case 8:
						xSerialHWHdls[ucPort].handle->Init.WordLength = eParity == MB_PAR_NONE ? UART_WORDLENGTH_8B : UART_WORDLENGTH_9B;
            break;
        default:
            eStatus = MB_EINVAL;
            break;
        }
        switch ( ucStopBits )
        {
        case 2:
            xSerialHWHdls[ucPort].handle->Init.StopBits = UART_STOPBITS_2;
            break;
        case 1:
            xSerialHWHdls[ucPort].handle->Init.StopBits = UART_STOPBITS_1;
            break;
        default:
            eStatus = MB_EINVAL;
            break;
        }
        switch ( eParity )
        {
        case MB_PAR_NONE:
            xSerialHWHdls[ucPort].handle->Init.Parity = UART_PARITY_NONE;
            break;
        case MB_PAR_ODD:
            xSerialHWHdls[ucPort].handle->Init.Parity = UART_PARITY_ODD;
            break;
        case MB_PAR_EVEN:
            xSerialHWHdls[ucPort].handle->Init.Parity = UART_PARITY_EVEN;
            break;
        default:
            eStatus = MB_EINVAL;
            break;
        }
        xSerialHWHdls[ucPort].handle->Init.Mode = UART_MODE_TX_RX;
        xSerialHWHdls[ucPort].handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
        xSerialHWHdls[ucPort].handle->Init.OverSampling = UART_OVERSAMPLING_16;
        xSerialHWHdls[ucPort].handle->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
        xSerialHWHdls[ucPort].handle->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

        float           timeout_us = 11.0f / ( float )ulBaudRate * 3.5f * 1E6f;
        if( timeout_us < 1500.0f )
        {
            timeout_us = 1500.0f;
        }
        xSerialHdls[ucPort].usIdleTimeMax = roundf( timeout_us / 750.0f );

        if( HAL_OK == HAL_UART_Init( xSerialHWHdls[ucPort].handle ) )
        {
            vMBPNextBuffer( ucPort, FALSE );
            if( HAL_OK != HAL_UART_Receive_DMA( xSerialHWHdls[ucPort].handle, xSerialHdls[ucPort].arubRXFIFO, RX_FIFO_LEN ) )
            {
                eStatus = MB_EIO;
            }
            else if( NULL == ( xSerialHdls[ucPort].xQueue = xQueueCreate( 10, sizeof( xQueueMsg ) ) ) )
            {
                eStatus = MB_ENORES;
            }
            else if( pdPASS !=
                     xTaskCreate( vMBPSerialHandlerTask, "MBP-SER", MBP_SERIAL_TASK_STACKSIZE, &xSerialHdls[ucPort], MBP_SERIAL_TASK_PRIORITY, NULL ) )
            {
                eStatus = MB_EPORTERR;
            }
            else
            {
                xSerialHdls[ucPort].pvMBPTimerExpiredFN = pbFrameTimeoutFN;
                xSerialHdls[ucPort].xMBHdl = xMBHdl;
                xSerialHdls[ucPort].ubIdx = ucPort;
                *pxSerialHdl = &xSerialHdls[ucPort];
                eStatus = MB_ENOERR;
            }

            if( MB_ENOERR != eStatus )
            {
                HAL_UART_DeInit( xSerialHWHdls[ucPort].handle );
                HDL_RESET( &xSerialHdls[ucPort] );
            }
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
    if( ( NULL != pxSerialIntHdl ) && MB_IS_VALID_HDL( pxSerialIntHdl, xSerialHdls ) )
    {
        xQueueMsg       xMsg = MSG_SHUTDOWN;
        if( pdTRUE != xQueueSend( pxSerialIntHdl->xQueue, &xMsg, 0 ) )
        {
            MBP_ASSERT( 0 );
            eStatus = MB_EIO;
        }
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
        if( NULL != pbMBPTransmitterEmptyFN )
        {
            pxSerialIntHdl->pbMBPTransmitterEmptyFN = pbMBPTransmitterEmptyFN;
            xQueueMsg       xMsg = MSG_NEW_TX_FRAME;
            if( pdTRUE != xQueueSend( pxSerialIntHdl->xQueue, &xMsg, 0 ) )
            {
                MBP_ASSERT( 0 );
                eStatus = MB_EIO;
            }
        }
        else
        {
            pxSerialIntHdl->pbMBPTransmitterEmptyFN = NULL;
        }
        eStatus = MB_ENOERR;
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
        if( NULL != pvMBPReceiveFN )
        {
						portDISABLE_INTERRUPTS(  );
						MODIFY_REG( xSerialHWHdls[pxSerialIntHdl->ubIdx].handle->Instance->CR1, 0, USART_CR1_RE );
						portENABLE_INTERRUPTS(  );
            pxSerialIntHdl->pvMBPReceiveFN = pvMBPReceiveFN;
        }
        else
        {
						portDISABLE_INTERRUPTS(  );
						MODIFY_REG( xSerialHWHdls[pxSerialIntHdl->ubIdx].handle->Instance->CR1, USART_CR1_RE, 0 );
						portENABLE_INTERRUPTS(  );
            pxSerialIntHdl->pvMBPReceiveFN = NULL;
        }
        eStatus = MB_ENOERR;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

STATIC void
vMBPSerialHandlerTask( void *pvArg )
{
    xSerialHandle  *pxSerialIntHdl = pvArg;
    BOOL            bIsRunning = TRUE;
    do
    {
        xQueueMsg       xMsg;
        if( pdTRUE == xQueueReceive( pxSerialIntHdl->xQueue, &xMsg, portMAX_DELAY ) )
        {
            switch ( xMsg )
            {
            case MSG_SHUTDOWN:
                bIsRunning = FALSE;
                break;
            case MSG_DMA_RECEIVE_COMPLETE:
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != pxSerialIntHdl->pvMBPReceiveFN )
                {
                    pxSerialIntHdl->pvMBPReceiveFN( pxSerialIntHdl->xMBHdl, pxSerialIntHdl->pubCurrentRXPtr, pxSerialIntHdl->usCurrentRXLen );
                }
                MBP_EXIT_CRITICAL_SECTION(  );
                break;
            case MSG_END_OF_FRAME:
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != pxSerialIntHdl->pvMBPReceiveFN )
                {
                    if( pxSerialIntHdl->usCurrentRXLen )
                    {
                        pxSerialIntHdl->pvMBPReceiveFN( pxSerialIntHdl->xMBHdl, pxSerialIntHdl->pubCurrentRXPtr, pxSerialIntHdl->usCurrentRXLen );
                    }
                    if( NULL != pxSerialIntHdl->pvMBPTimerExpiredFN )
                    {
                        pxSerialIntHdl->pvMBPTimerExpiredFN( pxSerialIntHdl->xMBHdl );
                    }
                }
                MBP_EXIT_CRITICAL_SECTION(  );
                break;

            case MSG_NEW_TX_FRAME:
                MBP_ENTER_CRITICAL_SECTION(  );
                if( NULL != pxSerialIntHdl->pbMBPTransmitterEmptyFN )
                {
										vMBPNextBuffer( pxSerialIntHdl->ubIdx, FALSE );
                    if( !pxSerialIntHdl->pbMBPTransmitterEmptyFN( pxSerialIntHdl->xMBHdl, pxSerialIntHdl->pubCurrentWRPtr, TXRX_BUFFER_LEN, &pxSerialIntHdl->usCurrentWRLen ) )
                    {
                        pxSerialIntHdl->pbMBPTransmitterEmptyFN = NULL;
                    }
                    if( pxSerialIntHdl->usCurrentWRLen > 0 )
                    {
                    	if( NULL != xSerialHWHdls[pxSerialIntHdl->ubIdx].sw_driver_enable )
                    	{
                    		HAL_GPIO_WritePin(xSerialHWHdls[pxSerialIntHdl->ubIdx].sw_driver_enable, xSerialHWHdls[pxSerialIntHdl->ubIdx].sw_pin, GPIO_PIN_SET);
                    	}
                        HAL_UART_Transmit_DMA( xSerialHWHdls[pxSerialIntHdl->ubIdx].handle, pxSerialIntHdl->pubCurrentWRPtr, pxSerialIntHdl->usCurrentWRLen );
												vMBPNextBuffer( pxSerialIntHdl->ubIdx, FALSE );
												xQueueMsg       xMsgTmp = MSG_DMA_SEND_COMPLETE;
												BaseType_t pdStatus = xQueueSend( pxSerialIntHdl->xQueue, &xMsgTmp, 0 );
												MBP_ASSERT( pdTRUE == pdStatus );
                    }
                }
                MBP_EXIT_CRITICAL_SECTION(  );
                break;
								
						case MSG_DMA_SEND_COMPLETE:
								MBP_ENTER_CRITICAL_SECTION(  );
								if( NULL != pxSerialIntHdl->pbMBPTransmitterEmptyFN )
                {
										if( !pxSerialIntHdl->pbMBPTransmitterEmptyFN( pxSerialIntHdl->xMBHdl, pxSerialIntHdl->pubCurrentWRPtr, TXRX_BUFFER_LEN, &pxSerialIntHdl->usCurrentWRLen ) )
                    {
                        pxSerialIntHdl->pbMBPTransmitterEmptyFN = NULL;
                    }										
								}
								MBP_EXIT_CRITICAL_SECTION(  );
								break;
								
						case MSG_DMA_TX_OVERFLOW:
								MBP_ENTER_CRITICAL_SECTION(  );
								/* DMA TX overflow - drain the transmitter */
								while( NULL != pxSerialIntHdl->pbMBPTransmitterEmptyFN )
								{
										if( !pxSerialIntHdl->pbMBPTransmitterEmptyFN( pxSerialIntHdl->xMBHdl, pxSerialIntHdl->pubCurrentWRPtr, TXRX_BUFFER_LEN, &pxSerialIntHdl->usCurrentWRLen ) )
                    {
                        pxSerialIntHdl->pbMBPTransmitterEmptyFN = NULL;
                    }		
								}
								MBP_EXIT_CRITICAL_SECTION(  );
            }
        }
    }
    while( bIsRunning );
    HDL_RESET( pxSerialIntHdl );
    vTaskDelete( NULL );
}

static void
vMBPNextBuffer( UBYTE ucPort, BOOL copyToRx )
{
    if( copyToRx )
    {
        xSerialHdls[ucPort].pubCurrentRXPtr = xSerialHdls[ucPort].pubCurrentWRPtr;
        xSerialHdls[ucPort].usCurrentRXLen = xSerialHdls[ucPort].usCurrentWRLen;
    }

    if( xSerialHdls[ucPort].pubCurrentWRPtr == &xSerialHdls[ucPort].arubTXRXDoubleBuffers[0][0] )
    {
        xSerialHdls[ucPort].pubCurrentWRPtr = &xSerialHdls[ucPort].arubTXRXDoubleBuffers[1][0];
    }
    else
    {
        xSerialHdls[ucPort].pubCurrentWRPtr = &xSerialHdls[ucPort].arubTXRXDoubleBuffers[0][0];
    }
    xSerialHdls[ucPort].usCurrentWRLen = 0;
}

void
HAL_UART_TxCpltCallback( UART_HandleTypeDef * huart )
{
    for( UBYTE ucPort = 0; ucPort < MB_UTILS_NARRSIZE( xSerialHdls ); ucPort++ )
    {
        if( ( IDX_INVALID != xSerialHdls[ucPort].ubIdx ) && ( xSerialHWHdls[ucPort].handle == huart ) )
        {
						if( xSerialHdls[ucPort].usCurrentWRLen > 0 )
						{
								HAL_UART_Transmit_DMA( xSerialHWHdls[ucPort].handle, xSerialHdls[ucPort].pubCurrentWRPtr, xSerialHdls[ucPort].usCurrentWRLen );
								vMBPNextBuffer( xSerialHdls[ucPort].ubIdx, FALSE );
								xQueueMsg       xMsg = MSG_DMA_SEND_COMPLETE;
								BaseType_t      pdStatus = xQueueSendFromISR( xSerialHdls[ucPort].xQueue, &xMsg, NULL );
								MBP_ASSERT( pdTRUE == pdStatus );	
						}
						else if( NULL != xSerialHdls[ucPort].pbMBPTransmitterEmptyFN )
						{
								/* WARNING - This indicates an overflow condition as the serial transmitter 
							   * empty callback is registered but no data was provided.
							   */
								xQueueMsg       xMsg = MSG_DMA_TX_OVERFLOW;
								BaseType_t      pdStatus = xQueueSendFromISR( xSerialHdls[ucPort].xQueue, &xMsg, NULL );
								MBP_ASSERT( pdTRUE == pdStatus );	

                /* Disable software driver enable pin if used */
            	if( NULL != xSerialHWHdls[ucPort].sw_driver_enable )
            	{
            		HAL_GPIO_WritePin(xSerialHWHdls[ucPort].sw_driver_enable, xSerialHWHdls[ucPort].sw_pin, GPIO_PIN_RESET );
            	}
            }
            else
            {
            	/* Disable software driver enable pin if used */
            	if( NULL != xSerialHWHdls[ucPort].sw_driver_enable )
            	{
            		HAL_GPIO_WritePin(xSerialHWHdls[ucPort].sw_driver_enable, xSerialHWHdls[ucPort].sw_pin, GPIO_PIN_RESET );
            	}
						}
        }
    }
}

void
vMBPHandleUART( UBYTE ucPort )
{
    if( IDX_INVALID != xSerialHdls[ucPort].ubIdx )
    {
        USHORT          usFIFOWrPos = RX_FIFO_LEN - __HAL_DMA_GET_COUNTER( xSerialHWHdls[ucPort].handle->hdmarx );

        /* No character has been received. Increment idle time counter by 1 (corresponds to 750us) */
        if( xSerialHdls[ucPort].usFIFORxPos == usFIFOWrPos )
        {
            if( xSerialHdls[ucPort].usIdleTime < xSerialHdls[ucPort].usIdleTimeMax )
            {
                xSerialHdls[ucPort].usIdleTime++;
            }
            else if( xSerialHdls[ucPort].usIdleTime == xSerialHdls[ucPort].usIdleTimeMax )
            {
                vMBPNextBuffer( ucPort, TRUE );
                xQueueMsg       xMsg = MSG_END_OF_FRAME;
                BaseType_t      pdStatus = xQueueSendFromISR( xSerialHdls[ucPort].xQueue, &xMsg, NULL );
                MBP_ASSERT( pdTRUE == pdStatus );
                xSerialHdls[ucPort].usIdleTime = 0xFFFFU;
            }
        }
        else
        {
            xSerialHdls[ucPort].usIdleTime = 0;
            while( xSerialHdls[ucPort].usFIFORxPos != usFIFOWrPos )
            {
                if( NULL != xSerialHdls[ucPort].pvMBPReceiveFN )
                {
                    xSerialHdls[ucPort].pubCurrentWRPtr[xSerialHdls[ucPort].usCurrentWRLen] = xSerialHdls[ucPort].arubRXFIFO[xSerialHdls[ucPort].usFIFORxPos];
                    xSerialHdls[ucPort].usCurrentWRLen++;
                    if( xSerialHdls[ucPort].usCurrentWRLen >= TXRX_BUFFER_LEN )
                    {
                        vMBPNextBuffer( ucPort, TRUE );
                        xQueueMsg       xMsg = MSG_DMA_RECEIVE_COMPLETE;
                        BaseType_t      pdStatus = xQueueSendFromISR( xSerialHdls[ucPort].xQueue, &xMsg, NULL );
                        MBP_ASSERT( pdTRUE == pdStatus );
                    }
                }
                xSerialHdls[ucPort].usFIFORxPos++;
                if( RX_FIFO_LEN == xSerialHdls[ucPort].usFIFORxPos )
                {
                    xSerialHdls[ucPort].usFIFORxPos = 0;
                }
            }
        }
    }
}

void
HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef * htim )
{
    if( htim == &htim3 )
    {
        for( UBYTE ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( xSerialHdls ); ubIdx++ )
        {
            vMBPHandleUART( ubIdx );
        }
    }
}
