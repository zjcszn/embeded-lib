/*
 * FreeModbus Libary: STM32 Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portevent_m.c v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"
#include "port.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Defines ------------------------------------------*/
#define MBMasterEventGetBlockTime portMAX_DELAY
//#define MBMasterEventGetBlockTime (100/portTICK_RATE_MS)
#define xModbusMasterEvent_QUEUE_SIZE 1
/* ----------------------- Variables ----------------------------------------*/
/*用于主机状态转换的事件*/
static EventGroupHandle_t xModbusMasterOsEvent;
/*使用主栈权利的信号量*/
static SemaphoreHandle_t xModbusMasterSemaphore;
static u8 semerrcount=0;
/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBMasterPortEventInit( void )
{
    xModbusMasterOsEvent = xEventGroupCreate();
    return TRUE;
}

BOOL
xMBMasterPortEventPost( eMBMasterEventType eEvent )
{
    xEventGroupSetBits(
                        xModbusMasterOsEvent,    /* The event group being updated. */
                        eEvent );/* The bits being set. */
    return TRUE;
}

BOOL
xMBMasterPortEventGet( eMBMasterEventType * eEvent )
{
    /* waiting forever OS event */
    EventBits_t uxModbusMasterEventBits;
    /* waiting forever OS event */
    uxModbusMasterEventBits = xEventGroupWaitBits(
                xModbusMasterOsEvent,   /* The event group being tested. */
                EV_MASTER_READY | EV_MASTER_FRAME_RECEIVED | EV_MASTER_EXECUTE |
                EV_MASTER_FRAME_SENT | EV_MASTER_ERROR_PROCESS, /* The bits within the event group to wait for. */
                pdTRUE,        /* BIT_0 and BIT_4 should be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                MBMasterEventGetBlockTime );/* Wait forever for either bit to be set. */

    switch (uxModbusMasterEventBits)
    {
    case EV_MASTER_READY:
        *eEvent = EV_MASTER_READY;
        break;
    case EV_MASTER_FRAME_RECEIVED:
        *eEvent = EV_MASTER_FRAME_RECEIVED;
        break;
    case EV_MASTER_EXECUTE:
        *eEvent = EV_MASTER_EXECUTE;
        break;
    case EV_MASTER_FRAME_SENT:
        *eEvent = EV_MASTER_FRAME_SENT;
        break;
    case EV_MASTER_ERROR_PROCESS:
        *eEvent = EV_MASTER_ERROR_PROCESS;
        break;
    }
    return TRUE;
}
/**
 * This function is initialize the OS resource for modbus master.
 * Note:The resource is define by OS.If you not use OS this function can be empty.
 *
 */
void vMBMasterOsResInit( void )
{
    /* Attempt to create a semaphore. */
    xModbusMasterSemaphore = xSemaphoreCreateBinary();

    if( xModbusMasterSemaphore == NULL )
    {
        /* There was insufficient OpenRTOS heap available for the semaphore to
        be created successfully. */
    }
    else
    {
        if(xSemaphoreGive(xModbusMasterSemaphore)!=pdTRUE)
		{
		   semerrcount++;
		}
		/* The semaphore can now be used. Its handle is stored in the xSemahore
        variable.  Calling xSemaphoreTake() on the semaphore here will fail until
        the semaphore has first been given. */
    }
}

/**
 * This function is take Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be just return TRUE.
 *
 * @param lTimeOut the waiting time.
 *
 * @return resource taked result
 */
BOOL xMBMasterRunResTake( LONG lTimeOut )
{
    /*If waiting time is -1 .It will wait forever */
    return xSemaphoreTake(xModbusMasterSemaphore,lTimeOut)? TRUE : FALSE; 
}

/**
 * This function is release Mobus Master running resource.
 * Note:The resource is define by Operating System.If you not use OS this function can be empty.
 *
 */
void vMBMasterRunResRelease( void )
{
    /* release resource */
    if(xSemaphoreGive(xModbusMasterSemaphore)!=pdTRUE)
	{
		semerrcount++;
	}
}

/**
 * This is modbus master respond timeout error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBRespondTimeout(UCHAR ucDestAddress, const UCHAR* pucPDUData,
        USHORT ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xEventGroupSetBits(
                        xModbusMasterOsEvent,    /* The event group being updated. */
                        EV_MASTER_ERROR_RESPOND_TIMEOUT );/* The bits being set. */
    /* You can add your code under here. */

}

/**
 * This is modbus master receive data error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBReceiveData(UCHAR ucDestAddress, const UCHAR* pucPDUData,
        USHORT ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xEventGroupSetBits(
                        xModbusMasterOsEvent,    /* The event group being updated. */
                        EV_MASTER_ERROR_RECEIVE_DATA );/* The bits being set. */
    
    /* You can add your code under here. */

}

/**
 * This is modbus master execute function error process callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 * @param ucDestAddress destination salve address
 * @param pucPDUData PDU buffer data
 * @param ucPDULength PDU buffer length
 *
 */
void vMBMasterErrorCBExecuteFunction(UCHAR ucDestAddress, const UCHAR* pucPDUData,
        USHORT ucPDULength) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
    xEventGroupSetBits(
                        xModbusMasterOsEvent,    /* The event group being updated. */
                        EV_MASTER_ERROR_EXECUTE_FUNCTION );/* The bits being set. */
    
    /* You can add your code under here. */

}

/**
 * This is modbus master request process success callback function.
 * @note There functions will block modbus master poll while execute OS waiting.
 * So,for real-time of system.Do not execute too much waiting process.
 *
 */
void vMBMasterCBRequestScuuess( void ) {
    /**
     * @note This code is use OS's event mechanism for modbus master protocol stack.
     * If you don't use OS, you can change it.
     */
	xEventGroupSetBits(
                        xModbusMasterOsEvent,    /* The event group being updated. */
                        EV_MASTER_PROCESS_SUCESS );/* The bits being set. */
    
    
    /* You can add your code under here. */

}

/**
 * This function is wait for modbus master request finish and return result.
 * Waiting result include request process success, request respond timeout,
 * receive data error and execute function error.You can use the above callback function.
 * @note If you are use OS, you can use OS's event mechanism. Otherwise you have to run
 * much user custom delay for waiting.
 *
 * @return request error code
 */
eMBMasterReqErrCode eMBMasterWaitRequestFinish( void ) 
{
    eMBMasterReqErrCode    eErrStatus = MB_MRE_NO_ERR;
    EventBits_t uxModbusMasterEventBits;
    
	/* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
    the event group.  Clear the bits before exiting. */
    uxModbusMasterEventBits = xEventGroupWaitBits(
                xModbusMasterOsEvent,   /* The event group being tested. */
                EV_MASTER_PROCESS_SUCESS | EV_MASTER_ERROR_RESPOND_TIMEOUT
                | EV_MASTER_ERROR_RECEIVE_DATA | EV_MASTER_ERROR_EXECUTE_FUNCTION, /* The bits within the event group to wait for. */
                pdTRUE,        /* BIT_0 and BIT_4 should be cleared before returning. */
                pdFALSE,       /* Don't wait for both bits, either bit will do. */
                MBMasterEventGetBlockTime );/* Wait forever for either bit to be set. */

	switch (uxModbusMasterEventBits)
    {
    case EV_MASTER_PROCESS_SUCESS:
        break;
    case EV_MASTER_ERROR_RESPOND_TIMEOUT:
    {
        eErrStatus = MB_MRE_TIMEDOUT;
        break;
    }
    case EV_MASTER_ERROR_RECEIVE_DATA:
    {
        eErrStatus = MB_MRE_REV_DATA;
        break;
    }
    case EV_MASTER_ERROR_EXECUTE_FUNCTION:
    {
        eErrStatus = MB_MRE_EXE_FUN;
        break;
    }
    }
    return eErrStatus;
}

#endif
