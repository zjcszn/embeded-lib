/* 
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/

#define MAX_TIMER_HDLS          ( 4 )
#define IDX_INVALID             ( 255 )
#define TIMER_TIMEOUT_INVALID	( 65535U )

#define RESET_HDL( x ) do { \
    ( x )->ubIdx = IDX_INVALID; \
    ( x )->bHasExpired = FALSE; \
	( x )->usNTimeOutMS = 0; \
	( x )->usNTimeLeft = TIMER_TIMEOUT_INVALID; \
    ( x )->xMBMHdl = MB_HDL_INVALID; \
    ( x )->pbMBPTimerExpiredFN = NULL; \
} while( 0 );

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE           ubIdx;
    BOOL            bHasExpired;
    USHORT          usNTimeOutMS;
    USHORT          usNTimeLeft;
    xMBHandle       xMBMHdl;
    pbMBPTimerExpiredCB pbMBPTimerExpiredFN;
} xTimerInternalHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC xTimerInternalHandle arxTimerHdls[MAX_TIMER_HDLS];
STATIC BOOL     bIsInitalized = FALSE;

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPTimerInit( xMBPTimerHandle * xTimerHdl, USHORT usTimeOut1ms, pbMBPTimerExpiredCB pbMBPTimerExpiredFN, xMBHandle xHdl )
{
    eMBErrorCode    eStatus = MB_EPORTERR;
    UBYTE           ubIdx;


    MBP_ENTER_CRITICAL_SECTION(  );
    if( ( NULL != xTimerHdl ) && ( NULL != pbMBPTimerExpiredFN ) && ( MB_HDL_INVALID != xHdl ) )
    {
        if( !bIsInitalized )
        {
            /* Initialize a hardware timer for 1 millisecond. */
            for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
            {
                RESET_HDL( &arxTimerHdls[ubIdx] );
            }

            bIsInitalized = TRUE;

        }
        for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
        {
            if( IDX_INVALID == arxTimerHdls[ubIdx].ubIdx )
            {
                break;
            }
        }
        if( MAX_TIMER_HDLS != ubIdx )
        {
        	EnterCritical();
            arxTimerHdls[ubIdx].ubIdx = ubIdx;
            arxTimerHdls[ubIdx].usNTimeOutMS = usTimeOut1ms;
            arxTimerHdls[ubIdx].usNTimeLeft = TIMER_TIMEOUT_INVALID;
            arxTimerHdls[ubIdx].xMBMHdl = xHdl;
            arxTimerHdls[ubIdx].pbMBPTimerExpiredFN = pbMBPTimerExpiredFN;
            *xTimerHdl = &arxTimerHdls[ubIdx];
            ExitCritical();
            eStatus = MB_ENOERR;
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
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

void
vMBPTimerClose( xMBPTimerHandle xTimerHdl )
{
    xTimerInternalHandle *pxTimerIntHdl = xTimerHdl;

    if( MB_IS_VALID_HDL( pxTimerIntHdl, arxTimerHdls ) )
    {
    	EnterCritical();
        RESET_HDL( pxTimerIntHdl );
        ExitCritical();
    }
}

eMBErrorCode
eMBPTimerSetTimeout( xMBPTimerHandle xTimerHdl, USHORT usTimeOut1ms )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xTimerInternalHandle *pxTimerIntHdl = xTimerHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxTimerIntHdl, arxTimerHdls ) && ( usTimeOut1ms > 0 ) && ( usTimeOut1ms != TIMER_TIMEOUT_INVALID ) )
    {
        pxTimerIntHdl->usNTimeOutMS = usTimeOut1ms;
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
    	EnterCritical();
        pxTimerIntHdl->usNTimeLeft = pxTimerIntHdl->usNTimeOutMS;
        pxTimerIntHdl->bHasExpired = FALSE;
        ExitCritical();
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
    	EnterCritical();
        pxTimerIntHdl->usNTimeLeft = TIMER_TIMEOUT_INVALID;
        pxTimerIntHdl->bHasExpired = FALSE;
        ExitCritical();
        eStatus = MB_ENOERR;
    }
    MBP_EXIT_CRITICAL_SECTION(  );
    return eStatus;
}

void
prvMBPTimerPoll( void )
{
    UBYTE           ubIdx;
    MBP_ENTER_CRITICAL_SECTION(  );
    for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
    {
        if( ( IDX_INVALID != arxTimerHdls[ubIdx].ubIdx ) && ( arxTimerHdls[ubIdx].bHasExpired ) )
        {
            arxTimerHdls[ubIdx].bHasExpired = FALSE;
            ( void )arxTimerHdls[ubIdx].pbMBPTimerExpiredFN( arxTimerHdls[ubIdx].xMBMHdl );
        }
    }
    MBP_EXIT_CRITICAL_SECTION(  );
}

BaseType_t
prvMBPTimerTickMS( void )
{
    UBYTE           ubIdx;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
    {
        if( ( IDX_INVALID != arxTimerHdls[ubIdx].ubIdx ) &&
            ( TIMER_TIMEOUT_INVALID != arxTimerHdls[ubIdx].usNTimeLeft ) && ( !arxTimerHdls[ubIdx].bHasExpired ) )
        {
            arxTimerHdls[ubIdx].usNTimeLeft--;
            if( 0 == arxTimerHdls[ubIdx].usNTimeLeft )
            {
                arxTimerHdls[ubIdx].bHasExpired = TRUE;
                BaseType_t status = xSemaphoreGiveFromISR( g_xModbusSem, &xHigherPriorityTaskWoken );
                MBP_ASSERT( pdTRUE == status );
            }
        }
    }
	return xHigherPriorityTaskWoken;
}
