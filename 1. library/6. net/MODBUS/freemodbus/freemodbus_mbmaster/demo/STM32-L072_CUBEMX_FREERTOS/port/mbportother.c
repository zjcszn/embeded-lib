/* 
 * MODBUS Library: STM32 CubeMX, FreeRTOS port
 * Copyright (c) 2016 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
 * $Id: mbportother.c 1074 2017-03-07 10:48:48Z cwalter $
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

/* ----------------------- Platform includes --------------------------------*/

#include "mbport.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "common/mbtypes.h"
#include "common/mbportlayer.h"
#include "common/mbframe.h"
#include "common/mbutils.h"

/* ----------------------- Defines ------------------------------------------*/

/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Static variables ---------------------------------*/
STATIC xSemaphoreHandle xCritSection;

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/

/* ----------------------- Start implementation -----------------------------*/

void
vMBPInit( void )
{
    xCritSection = xSemaphoreCreateRecursiveMutex(  );
    MBP_ASSERT( NULL != xCritSection );
}

void
vMBPEnterCritical( void )
{
    signed portBASE_TYPE xResult;
    xResult = xSemaphoreTakeRecursive( xCritSection, portMAX_DELAY );
    MBP_ASSERT( pdTRUE == xResult );
}

void
vMBPExitCritical( void )
{
    signed portBASE_TYPE xResult;
    xResult = xSemaphoreGiveRecursive( xCritSection );
    MBP_ASSERT( pdTRUE == xResult );
}

void
vMBPAssert( const char *pszFile, int iLineNo )
{
    portDISABLE_INTERRUPTS(  );
    /* Let the watchdog trigger a reset here. */
    for( ;; );
}
