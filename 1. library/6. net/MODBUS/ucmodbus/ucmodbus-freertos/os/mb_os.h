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
* Filename : mb_os.h
* Version  : V2.14.00
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  MB_OS_MODULE_PRESENT
#define  MB_OS_MODULE_PRESENT


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/


#include "FreeRTOS.h"
#include "semphr.h"


/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   MB_OS_MODULE
#define  MB_OS_EXT
#else
#define  MB_OS_EXT  extern
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define MB_OS_CFG_RX_TASK_STK_SIZE            512U

#define MB_OS_CFG_RX_TASK_PRIO                configMAX_PRIORITIES

#define MB_OS_MSG_QUEUE_LENGTH                1U

#define MB_OS_NO_WAIT                         0U

#define MB_OS_WAIT_FOREVER                    portMAX_DELAY

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if      (configUSE_QUEUE_SETS == 0          )
#error  "MODBUS Slave requires FreeRTOS Message Queue Services."
#endif


#ifndef  MB_OS_CFG_RX_TASK_PRIO
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_PRIO."
#endif


#ifndef  MB_OS_CFG_RX_TASK_STK_SIZE
#error  "MODBUS Missing Rx Task's MB_OS_CFG_RX_TASK_STK_SIZE."
#endif



/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of MB_OS module                                */

