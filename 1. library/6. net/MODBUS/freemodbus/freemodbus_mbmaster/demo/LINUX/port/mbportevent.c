/*
 * MODBUS Library: LINUX/CYGWIN port
 * Copyright (c) 2009-2015 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbportevent.c,v 1.6 2014-08-23 09:36:06 embedded-solutions.cwalter Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbframe.h"
#include "common/mbutils.h"
#include "common/mbportlayer.h"

/* ----------------------- Defines ------------------------------------------*/
#define MAX_EVENT_HDLS ( 8 )
#define IDX_INVALID    ( 255 )
#define EV_NONE        ( 0 )

#define HDL_RESET( x )              \
    do                              \
    {                               \
        ( x )->ubIdx = IDX_INVALID; \
        ( x )->xEvType = EV_NONE;   \
    } while( 0 )

/* ----------------------- Type definitions ---------------------------------*/
typedef struct
{
    UBYTE         ubIdx;
    xMBPEventType xEvType;
    sem_t         xSem;
} xEventInternalHandle;

/* ----------------------- Static variables ---------------------------------*/
STATIC BOOL                 bIsInitialized = FALSE;
STATIC xEventInternalHandle arxEventHdls[MAX_EVENT_HDLS];

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

eMBErrorCode
eMBPEventCreate( xMBPEventHandle *pxEventHdl )
{
    eMBErrorCode eStatus = MB_EINVAL;
    UBYTE        i;

    if( NULL != pxEventHdl )
    {
        MBP_ENTER_CRITICAL_SECTION();
        if( !bIsInitialized )
        {
            for( i = 0; i < MAX_EVENT_HDLS; i++ )
            {
                HDL_RESET( &arxEventHdls[i] );
            }
            for( i = 0; i < MAX_EVENT_HDLS; i++ )
            {
                bIsInitialized = TRUE;
            }
        }
        if( bIsInitialized )
        {
            for( i = 0; i < MAX_EVENT_HDLS; i++ )
            {
                if( IDX_INVALID == arxEventHdls[i].ubIdx )
                {
                    if( -1 != sem_init( &arxEventHdls[i].xSem, 0, 0 ) )
                    {
                        arxEventHdls[i].ubIdx = i;
                        *pxEventHdl = &arxEventHdls[i];
                        eStatus = MB_ENOERR;
                    }
                    else
                    {
                        eStatus = MB_EPORTERR;
                    }
                    break;
                }
            }
        }
        MBP_EXIT_CRITICAL_SECTION();
    }
    return eStatus;
}

eMBErrorCode
eMBPEventPost( const xMBPEventHandle xEventHdl, xMBPEventType xEvent )
{
    eMBErrorCode          eStatus = MB_EINVAL;
    xEventInternalHandle *pxEventHdl = xEventHdl;

#if defined( MBP_ENABLE_DEBUG_FACILITY ) && ( MBP_ENABLE_DEBUG_FACILITY == 1 )
            if( bMBPPortLogIsEnabled( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL ) )
            {
                vMBPPortLog( MB_LOG_DEBUG, MB_LOG_PORT_SERIAL, "posting event %d\n", xEvent );
            }
#endif

    if( MB_IS_VALID_HDL( pxEventHdl, arxEventHdls ) )
    {
        MBP_ENTER_CRITICAL_SECTION();
        pxEventHdl->xEvType = xEvent;
        eStatus = sem_post( &( pxEventHdl->xSem ) ) != -1 ? MB_ENOERR : MB_EPORTERR;
        MBP_EXIT_CRITICAL_SECTION();
    }
    return eStatus;
}

BOOL
bMBPEventGet( const xMBPEventHandle xEventHdl, xMBPEventType *pxEvent )
{
    BOOL                  bEventInQueue = FALSE;
    xEventInternalHandle *pxEventHdl = xEventHdl;

    if( MB_IS_VALID_HDL( pxEventHdl, arxEventHdls ) )
    {
        if( 0 == sem_wait( &( pxEventHdl->xSem ) ) )
        {
            MBP_ENTER_CRITICAL_SECTION();
            *pxEvent = pxEventHdl->xEvType;
            pxEventHdl->xEvType = EV_NONE;
            bEventInQueue = *pxEvent != EV_NONE;
            MBP_EXIT_CRITICAL_SECTION();
        }
    }
    return bEventInQueue;
}

void
vMBPEventDelete( xMBPEventHandle xEventHdl )
{
    BYTE                  x;
    xEventInternalHandle *pxEventIntHdl = xEventHdl;

    MBP_ENTER_CRITICAL_SECTION();
    if( MB_IS_VALID_HDL( pxEventIntHdl, arxEventHdls ) )
    {
        sem_destroy( &( pxEventIntHdl->xSem ) );
        HDL_RESET( pxEventIntHdl );
    }

    MBP_EXIT_CRITICAL_SECTION();
}
