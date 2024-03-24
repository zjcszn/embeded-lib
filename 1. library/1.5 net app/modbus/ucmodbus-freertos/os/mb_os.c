/*
*********************************************************************************************************
*                                              uC/Modbus
*                                       The Embedded Modbus Stack
*
*                    Copyright 2003-2020 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   MODBUS uC/OS-III LAYER INTERFACE
*
* Filename : mb_os.c
* Version  : V2.14.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define   MB_OS_MODULE
#include "mb.h"




/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  SemaphoreHandle_t   MB_OS_RxSemTbl[MODBUS_CFG_MAX_CH];
#endif

#if (MODBUS_CFG_SLAVE_EN  == DEF_ENABLED)
static  TaskHandle_t        MB_OS_RxTaskTCB;
static  QueueHandle_t       MB_OS_RxMsgQueue;
#endif


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  void  MB_OS_InitMaster(void);
static  void  MB_OS_ExitMaster(void);
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static  void  MB_OS_InitSlave (void);
static  void  MB_OS_ExitSlave (void);
static  void  MB_OS_RxTask    (void  *p_arg);
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MB_OS_Init()
*
* Description : This function initializes the RTOS interface.  This function creates the following:
*
*               (1) A message queue to signal the reception of a packet.
*
*               (2) A task that waits for packets to be received.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_OS_Init (void)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    MB_OS_InitMaster();
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    MB_OS_InitSlave();
#endif
}


/*
*********************************************************************************************************
*                                          MB_OS_InitMaster()
*
* Description : This function initializes and creates the kernel objectes needed for Modbus Master
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  void  MB_OS_InitMaster (void)
{
    CPU_INT08U   i;

    for (i = 0; i < MODBUS_CFG_MAX_CH; i++) {                 /* Create a semaphore for each channel   */
        MB_OS_RxSemTbl[i] = xSemaphoreCreateBinary();         
    }
}
#endif


/*
*********************************************************************************************************
*                                          MB_OS_InitSlave()
*
* Description : This function initializes and creates the kernel objectes needed for Modbus Salve
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static  void  MB_OS_InitSlave (void)
{
    xTaskCreate((TaskFunction_t )MB_OS_RxTask,
                (const char*    )"Modbus Rx Task",
                (const uint16_t )MB_OS_CFG_RX_TASK_STK_SIZE / sizeof(StackType_t),
                (void*          )NULL,
                (UBaseType_t    )MB_OS_CFG_RX_TASK_PRIO,
                (TaskHandle_t*  )&MB_OS_RxTaskTCB);

    MB_OS_RxMsgQueue = xQueueCreate(MB_OS_MSG_QUEUE_LENGTH, sizeof(void *));
}
#endif


/*
*********************************************************************************************************
*                                             MB_OS_Exit()
*
* Description : This function is called to terminate the RTOS interface for Modbus channels.  We delete
*               the following uC/OS-II objects:
*
*               (1) An event flag group to signal the reception of a packet.
*               (2) A task that waits for packets to be received.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_Exit().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_OS_Exit (void)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    MB_OS_ExitMaster();
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
    MB_OS_ExitSlave();
#endif
}


/*
*********************************************************************************************************
*                                          MB_OS_ExitMaster()
*
* Description : This function is called to terminate the RTOS interface for Modbus Master channels.  The
*               following objects are deleted.
*
*               (1) An event flag group to signal the reception of a packet.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Exit().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
static  void  MB_OS_ExitMaster (void)
{
    CPU_INT08U  i;

    for (i = 0; i < MODBUS_CFG_MAX_CH; i++) {                 /* Delete semaphore for each channel     */
        vSemaphoreDelete(MB_OS_RxSemTbl[i]);
    }
}
#endif


/*
*********************************************************************************************************
*                                          MB_OS_ExitSlave()
*
* Description : This function is called to terminate the RTOS interface for Modbus Salve channels.
*               The following objects are deleted.
*
*               (1) A task that waits for packets to be received.
*               (2) A message queue to signal the reception of a packet.
*
* Argument(s) : none
*
* Return(s)   : none.
*
* Caller(s)   : MB_OS_Exit().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
void  MB_OS_ExitSlave (void)
{
    vTaskDelete(MB_OS_RxTaskTCB);                                /* Delete Modbus Rx Task                 */
    vQueueDelete(MB_OS_RxMsgQueue);

    MB_OS_RxTaskTCB  = (void *)0U;
    MB_OS_RxMsgQueue = (void *)0U;
   
}
#endif


/*
*********************************************************************************************************
*                                              MB_OS_RxSignal()
*
* Description : This function signals the reception of a packet either from the Rx ISR(s) or the RTU timeout
*               timer(s) to indicate that a received packet needs to be processed.
*
* Argument(s) : pch     specifies the Modbus channel data structure in which a packet was received.
*
* Return(s)   : none.
*
* Caller(s)   : MB_ASCII_RxByte(),
*               MB_RTU_TmrUpdate().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  MB_OS_RxSignal (MODBUS_CH *pch)
{
    CPU_INT32S  status;

    if (pch != (MODBUS_CH *)0) {
        switch (pch->MasterSlave) {
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
            case MODBUS_MASTER:
                 status = xSemaphoreGive(MB_OS_RxSemTbl[pch->Ch]);
                 MB_LOG_CONDI(status != pdTRUE, "Modbus Master RxSemaphore Give Failed\r\n");
                 break;
#endif

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
            case MODBUS_SLAVE:
            default:
                 status = xQueueSend(MB_OS_RxMsgQueue, &pch, MB_OS_NO_WAIT);
                 MB_LOG_CONDI(status != pdTRUE, "Modbus Slave RxMsgQueue Send Failed\r\n");
                 break;
#endif
        }
    }
}


/*
*********************************************************************************************************
*                                              MB_OS_RxWait()
*
* Description : This function waits for a response from a slave.
*
* Argument(s) : pch     specifies the Modbus channel data structure to wait on.
*
*               perr    is a pointer to a variable that will receive an error code.  Possible errors are:
*
*                       MODBUS_ERR_NONE        the call was successful and a packet was received
*                       MODBUS_ERR_TIMED_OUT   a packet was not received within the specified timeout
*                       MODBUS_ERR_NOT_MASTER  the channel is not a Master
*                       MODBUS_ERR_INVALID     an invalid error was detected
*
* Return(s)   : none.
*
* Caller(s)   : MBM_FCxx()  Modbus Master Functions
*
* Return(s)   : none.
*********************************************************************************************************
*/

void  MB_OS_RxWait (MODBUS_CH   *pch,
                    CPU_INT16U  *perr)
{
#if (MODBUS_CFG_MASTER_EN == DEF_ENABLED)
    CPU_INT32S  status;

    if (pch != (MODBUS_CH *)0) {
        if (pch->MasterSlave == MODBUS_MASTER) {
            status = xSemaphoreTake(MB_OS_RxSemTbl[pch->Ch], pch->RxTimeout);
            MB_LOG_CONDI(status != pdTRUE, "Modbus Master RxSemaphore Take Failed\r\n");
            switch (status) {
                case pdTRUE:
                     *perr = MODBUS_ERR_NONE;
                     break;

                case pdFALSE:
                     *perr = MODBUS_ERR_TIMED_OUT;
                     break;
            }
        } else {
            *perr = MODBUS_ERR_NOT_MASTER;
        }
    } else {
        *perr = MODBUS_ERR_NULLPTR;
    }
#else
    *perr = MODBUS_ERR_INVALID;
#endif
}

/*
*********************************************************************************************************
*                                            MB_OS_RxTask()
*
* Description : This task is created by MB_OS_Init() and waits for signals from either the Rx ISR(s) or
*               the RTU timeout timer(s) to indicate that a packet needs to be processed.
*
* Argument(s) : p_arg       is a pointer to an optional argument that is passed by uC/OS-II to the task.
*                           This argument is not used.
*
* Return(s)   : none.
*
* Caller(s)   : This is a Task.
*
* Return(s)   : none.
*********************************************************************************************************
*/

#if (MODBUS_CFG_SLAVE_EN == DEF_ENABLED)
static  void  MB_OS_RxTask (void *p_arg)
{
    MODBUS_CH   *pch;
    CPU_INT32S   status;


    (void)p_arg;
    while (DEF_TRUE) {
        /* Wait for a packet to be received */
        status = xQueueReceive(MB_OS_RxMsgQueue, (void *)&pch, MB_OS_WAIT_FOREVER);
        MB_LOG_CONDI(status != pdTRUE, "Modbus Slave MsgQueue Recive Failed\r\n");

        /* Process the packet received */
        MB_RxTask(pch);                          
    }
}
#endif

