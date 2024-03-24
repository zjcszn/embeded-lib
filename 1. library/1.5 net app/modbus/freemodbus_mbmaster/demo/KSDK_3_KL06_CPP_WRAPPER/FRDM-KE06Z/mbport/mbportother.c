/* 
 * MODBUS Library: KL06 KDS/PE/FreeRTOS port
 * Copyright (c)  2019 Christian Walter <cwalter@embedded-solutions.at>
 * All rights reserved.
 *
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
#define MBM_LOGFACILITIES                       ( 0xFFFFU )
#define MBM_LOGLEVELS                           ( MB_LOG_ERROR )
#define MBP_LOCK_TIMEOUT                        ( 10000 / portTICK_RATE_MS )
#define MBP_FLUSH_AFTER_WRITE                   ( 1 )
/* ----------------------- Type definitions ---------------------------------*/

/* ----------------------- Static variables ---------------------------------*/
STATIC xSemaphoreHandle xCritSection;
QueueHandle_t g_xModbusSem;

/* ----------------------- Static functions ---------------------------------*/

/* ----------------------- Function prototypes ------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
static
void vMBPTask(void *pvParameters)
{
	for( ;; )
	{
		if( pdTRUE == xSemaphoreTake( g_xModbusSem, 100 ) )
		{
			prvMBPTimerPoll( );
			prvMBPSerialPoll( );
		}
	}
}

void
vMBPInit( void )
{
    xCritSection = xSemaphoreCreateRecursiveMutex(  );
    MBP_ASSERT( NULL != xCritSection );
    g_xModbusSem = xQueueCreateCountingSemaphore( 20, 0 );
    MBP_ASSERT( NULL != g_xModbusSem );

    BaseType_t pdStatus = xTaskCreate(vMBPTask, "MODBUS Port", configMINIMAL_STACK_SIZE, NULL, MBP_TASK_PRIORITY, NULL);
    MBP_ASSERT( pdTRUE == pdStatus);
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
vMBPAssert( void )
{
    /* Let the watchdog trigger a reset here. */
    for( ;; );
}
