/* 
 * MODBUS Library: THREADX port
 * Copyright (c) 2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 */

/* ----------------------- System includes ----------------------------------*/
#include <mbportssp.h>
#include <stdlib.h>

/* ----------------------- Platform includes --------------------------------*/
#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Static variables ---------------------------------*/
extern TX_THREAD mbportssp;

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
void
vMBPInit( void )
{
    tx_thread_resume( &mbportssp );
}

void
vMBPAssert( void )
{
    volatile BOOL   bBreakOut = FALSE;

    vMBPEnterCritical(  );
    while( !bBreakOut );
}

void
vMBPEnterCritical( void )
{
    UINT            status;
    status = tx_mutex_get( &g_mbport_mutex0, TX_WAIT_FOREVER );
    MBP_ASSERT( TX_SUCCESS == status );
}

void
vMBPExitCritical( void )
{
    UINT            status;
    status = tx_mutex_put( &g_mbport_mutex0 );
    MBP_ASSERT( TX_SUCCESS == status );
}
