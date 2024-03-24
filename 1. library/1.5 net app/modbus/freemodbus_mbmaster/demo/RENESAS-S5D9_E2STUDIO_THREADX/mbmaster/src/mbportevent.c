/* 
 * MODBUS Library: THREADX port
 * Copyright (c) 2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
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
#define MAX_EVENT_HDLS          ( 1 )
#define IDX_INVALID             ( 255 )
#define EV_NONE                 ( 0 )

#define HDL_RESET( x ) do { \
    ( x )->ubIdx = IDX_INVALID; \
    memset( &( x )->xQueue, 0, sizeof( TX_QUEUE ) ); \
} while( 0 );

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE           ubIdx;
    TX_QUEUE        xQueue;
    UBYTE           arubBuffer[4];
} xEventInternalHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC BOOL     bIsInitialized = FALSE;
STATIC xEventInternalHandle arxEventHdls[MAX_EVENT_HDLS];

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPEventCreate( xMBPEventHandle * pxEventHdl )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    UBYTE           i;

    if( NULL != pxEventHdl )
    {
        MBP_ENTER_CRITICAL_SECTION(  );
        if( !bIsInitialized )
        {
            for( i = 0; i < MAX_EVENT_HDLS; i++ )
            {
                HDL_RESET( &arxEventHdls[i] );
            }
            bIsInitialized = TRUE;
        }
        for( i = 0; i < MAX_EVENT_HDLS; i++ )
        {
            if( IDX_INVALID == arxEventHdls[i].ubIdx )
            {

                UINT            status =
                    tx_queue_create( &arxEventHdls[i].xQueue, "MBP", TX_1_ULONG, arxEventHdls[i].arubBuffer, sizeof( arxEventHdls[i].arubBuffer ) );
                if( TX_SUCCESS == status )
                {
                    arxEventHdls[i].ubIdx = i;
                    *pxEventHdl = &arxEventHdls[i];
                }
                else
                {
                    HDL_RESET( &arxEventHdls[i] );
                }
                eStatus = MB_ENOERR;
                break;
            }
        }
        MBP_EXIT_CRITICAL_SECTION(  );
    }
    return eStatus;
}

eMBErrorCode
eMBPEventPost( const xMBPEventHandle xEventHdl, xMBPEventType xEvent )
{
    eMBErrorCode    eStatus = MB_EINVAL;
    xEventInternalHandle *pxEventHdl = xEventHdl;

    if( MB_IS_VALID_HDL( pxEventHdl, arxEventHdls ) )
    {
        UINT            status;
        status = tx_queue_flush( &pxEventHdl->xQueue );
        MBP_ASSERT( TX_SUCCESS == status );
        uint32_t        payload = xEvent;
        status = tx_queue_front_send( &pxEventHdl->xQueue, &payload, TX_NO_WAIT );
        MBP_ASSERT( TX_SUCCESS == status );
        eStatus = MB_ENOERR;
    }
    return eStatus;
}

BOOL
bMBPEventGet( const xMBPEventHandle xEventHdl, xMBPEventType * pxEvent )
{
    BOOL            bEventInQueue = FALSE;
    xEventInternalHandle *pxEventHdl = xEventHdl;

    if( MB_IS_VALID_HDL( pxEventHdl, arxEventHdls ) )
    {
        UINT            status;
        uint32_t        payload;
        status = tx_queue_receive( &pxEventHdl->xQueue, &payload, 100 );
        if( TX_SUCCESS == status )
        {
            bEventInQueue = payload != EV_NONE;
            *pxEvent = ( xMBPEventType ) payload;
        }
    }
    return bEventInQueue;
}

void
vMBPEventDelete( xMBPEventHandle xEventHdl )
{
    xEventInternalHandle *pxEventIntHdl = xEventHdl;

    MBP_ENTER_CRITICAL_SECTION(  );
    if( MB_IS_VALID_HDL( pxEventIntHdl, arxEventHdls ) )
    {
        ( void )tx_queue_delete( &pxEventIntHdl->xQueue );
        HDL_RESET( pxEventIntHdl );
    }
    MBP_EXIT_CRITICAL_SECTION(  );
}
