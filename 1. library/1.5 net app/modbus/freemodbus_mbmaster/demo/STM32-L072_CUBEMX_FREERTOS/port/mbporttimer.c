/* 
 * MODBUS Library: STM32 CubeMX, FreeRTOS port
 * Copyright (c)  2016 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbporttimer.c 1068 2016-08-26 16:05:09Z cwalter $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include "gpio.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/
#define TIMER_TASK_PRIORITY             ( MBP_TASK_PRIORITY )
#define TIMER_TASK_STACKSIZE            ( 256 )
#define TIMER_TICKRATE_MS               ( 10 / portTICK_RATE_MS )
#define MAX_TIMER_HDLS                  ( 3 )
#define IDX_INVALID                     ( 255 )

#define RESET_HDL( x ) do { \
    ( x )->ubIdx = IDX_INVALID; \
    ( x )->bIsRunning = FALSE; \
    ( x )->xNTimeoutTicks = 0; \
    ( x )->xNExpiryTimeTicks = 0; \
    ( x )->xMBMHdl = MB_HDL_INVALID; \
    ( x )->pbMBPTimerExpiredFN = NULL; \
} while( 0 );

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE           ubIdx;
    BOOL            bIsRunning;
    portTickType    xNTimeoutTicks;
    portTickType    xNExpiryTimeTicks;
    xMBHandle       xMBMHdl;
    pbMBPTimerExpiredCB pbMBPTimerExpiredFN;
} xTimerInternalHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xTimerInternalHandle arxTimerHdls[MAX_TIMER_HDLS];

STATIC BOOL     bIsInitalized = FALSE;

/* ----------------------- Static functions ---------------------------------*/
void            vMBPTimerTask( void *pvParameters );

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPTimerInit( xMBPTimerHandle * xTimerHdl, USHORT usTimeOut1ms,
               pbMBPTimerExpiredCB pbMBPTimerExpiredFN, xMBHandle xHdl )
{
    eMBErrorCode    eStatus = MB_EPORTERR;
    UBYTE           ubIdx;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( ( NULL != xTimerHdl ) && ( NULL != pbMBPTimerExpiredFN ) && ( MB_HDL_INVALID != xHdl ) )
    {
        if( !bIsInitalized )
        {
            for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
            {
                RESET_HDL( &arxTimerHdls[ubIdx] );
            }
            if( pdPASS !=
                xTaskCreate( vMBPTimerTask, "MBP-TIMER", TIMER_TASK_STACKSIZE, NULL, TIMER_TASK_PRIORITY, NULL ) )
            {
                eStatus = MB_EPORTERR;
            }
            else
            {
                bIsInitalized = TRUE;
            }
        }
        if( bIsInitalized )
        {
            for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
            {
                if( IDX_INVALID == arxTimerHdls[ubIdx].ubIdx )
                {
                    break;
                }
            }
            if( MAX_TIMER_HDLS != ubIdx )
            {
                arxTimerHdls[ubIdx].ubIdx = ubIdx;
                arxTimerHdls[ubIdx].bIsRunning = FALSE;
                arxTimerHdls[ubIdx].xNTimeoutTicks = MB_INTDIV_CEIL( ( portTickType ) usTimeOut1ms, portTICK_RATE_MS );
                arxTimerHdls[ubIdx].xNExpiryTimeTicks = 0;
                arxTimerHdls[ubIdx].xMBMHdl = xHdl;
                arxTimerHdls[ubIdx].pbMBPTimerExpiredFN = pbMBPTimerExpiredFN;

                *xTimerHdl = &arxTimerHdls[ubIdx];
                eStatus = MB_ENOERR;
            }
            else
            {
                eStatus = MB_ENORES;
            }
        }
    }
    else
    {
        eStatus = MB_EINVAL;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

void
vMBPTimerClose( xMBPTimerHandle xTimerHdl )
{
    xTimerInternalHandle *pxTimerIntHdl = xTimerHdl;

    if( MB_IS_VALID_HDL( pxTimerIntHdl, arxTimerHdls ) )
    {
        RESET_HDL( pxTimerIntHdl );
    }
}

eMBErrorCode
eMBPTimerSetTimeout( xMBPTimerHandle xTimerHdl, USHORT usTimeOut1ms )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xTimerInternalHandle *pxTimerIntHdl = xTimerHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxTimerIntHdl, arxTimerHdls ) && ( usTimeOut1ms > 0 ) )
    {
        pxTimerIntHdl->xNTimeoutTicks = MB_INTDIV_CEIL( ( portTickType ) usTimeOut1ms, portTICK_RATE_MS );
        eStatus = MB_ENOERR;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPTimerStart( xMBPTimerHandle xTimerHdl )
{
    eMBErrorCode    eStatus = MB_EINVAL;

    xTimerInternalHandle *pxTimerIntHdl = xTimerHdl;
    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxTimerIntHdl, arxTimerHdls ) )
    {
        pxTimerIntHdl->bIsRunning = TRUE;
        pxTimerIntHdl->xNExpiryTimeTicks = xTaskGetTickCount(  );
				HAL_GPIO_WritePin( LED_LD1_GPIO_Port, LED_LD1_Pin, GPIO_PIN_SET );
        eStatus = MB_ENOERR;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

eMBErrorCode
eMBPTimerStop( xMBPTimerHandle xTimerHdl )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xTimerInternalHandle *pxTimerIntHdl = xTimerHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxTimerIntHdl, arxTimerHdls ) )
    {
        pxTimerIntHdl->bIsRunning = FALSE;
        pxTimerIntHdl->xNExpiryTimeTicks = 0;
        eStatus = MB_ENOERR;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

void
vMBPTimerTask( void *pvParameters )
{
    UBYTE           ubIdx;
    xTimerInternalHandle *pxTmrHdl;
    portTickType    xLastWakeTime;
    portTickType    xCurrentTime;

    xLastWakeTime = xTaskGetTickCount(  );
    for( ;; )
    {
        vTaskDelayUntil( &xLastWakeTime, ( portTickType ) TIMER_TICKRATE_MS );
				MBP_ENTER_CRITICAL_SECTION(  );
        xCurrentTime = xTaskGetTickCount(  );        
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
        {
            pxTmrHdl = &arxTimerHdls[ubIdx];
            if( ( IDX_INVALID != pxTmrHdl->ubIdx ) && ( pxTmrHdl->bIsRunning ) )
            {
                if( ( xCurrentTime - pxTmrHdl->xNExpiryTimeTicks ) >= pxTmrHdl->xNTimeoutTicks )
                {
										HAL_GPIO_WritePin( LED_LD1_GPIO_Port, LED_LD1_Pin, GPIO_PIN_RESET);
                    pxTmrHdl->bIsRunning = FALSE;
                    if( NULL != pxTmrHdl->pbMBPTimerExpiredFN )
                    {
                        ( void )pxTmrHdl->pbMBPTimerExpiredFN( pxTmrHdl->xMBMHdl );
                    }
                }
            }
        }
        MBP_EXIT_CRITICAL_SECTION(  );
    }
}
