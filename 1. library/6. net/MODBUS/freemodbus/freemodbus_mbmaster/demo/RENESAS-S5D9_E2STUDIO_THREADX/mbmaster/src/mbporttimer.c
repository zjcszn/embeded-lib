/* 
 * MODBUS Library: THREADX port
 * Copyright (c) 2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * The timer is implemented as a software timer. A periodic interrupt
 * decrements the timer ticks. When a timer is enabled and the tick count
 * reaches zero the semaphore is signaled and the timer is set to expired.
 * The polling function will then execute the callback.
 *
 * Locking:
 *  - For the MODBUS core stack locking is by MBP_PORT_ENTER/EXIT using
 *    the semaphore.
 *  - For modifying the state of the variables changed in the ISR short
 *    locking by enabling/disabling interrupts is used.
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"
#include <mbportssp.h>

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
            UINT            isr_state = tx_interrupt_control( TX_INT_DISABLE );
            arxTimerHdls[ubIdx].ubIdx = ubIdx;
            arxTimerHdls[ubIdx].usNTimeOutMS = usTimeOut1ms;
            arxTimerHdls[ubIdx].usNTimeLeft = TIMER_TIMEOUT_INVALID;
            arxTimerHdls[ubIdx].xMBMHdl = xHdl;
            arxTimerHdls[ubIdx].pbMBPTimerExpiredFN = pbMBPTimerExpiredFN;
            *xTimerHdl = &arxTimerHdls[ubIdx];
            ( void )tx_interrupt_control( isr_state );
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
        UINT            isr_state = tx_interrupt_control( TX_INT_DISABLE );
        RESET_HDL( pxTimerIntHdl );
        ( void )tx_interrupt_control( isr_state );
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
        UINT            isr_state = tx_interrupt_control( TX_INT_DISABLE );
        pxTimerIntHdl->usNTimeLeft = pxTimerIntHdl->usNTimeOutMS;
        pxTimerIntHdl->bHasExpired = FALSE;
        ( void )tx_interrupt_control( isr_state );
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
        UINT            isr_state = tx_interrupt_control( TX_INT_DISABLE );
        pxTimerIntHdl->usNTimeLeft = TIMER_TIMEOUT_INVALID;
        pxTimerIntHdl->bHasExpired = FALSE;
        ( void )tx_interrupt_control( isr_state );
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

void
prvMBPTimerTickMS( void )
{
    UBYTE           ubIdx;
    for( ubIdx = 0; ubIdx < MB_UTILS_NARRSIZE( arxTimerHdls ); ubIdx++ )
    {
        if( ( IDX_INVALID != arxTimerHdls[ubIdx].ubIdx ) &&
            ( TIMER_TIMEOUT_INVALID != arxTimerHdls[ubIdx].usNTimeLeft ) && ( !arxTimerHdls[ubIdx].bHasExpired ) )
        {
            arxTimerHdls[ubIdx].usNTimeLeft--;
            if( 0 == arxTimerHdls[ubIdx].usNTimeLeft )
            {
                arxTimerHdls[ubIdx].bHasExpired = TRUE;
                UINT            status = tx_semaphore_put( &g_mbport_sem );
                MBP_ASSERT( TX_SUCCESS == status );
            }
        }
    }
}
