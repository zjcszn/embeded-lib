/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
    See http://www.freertos.org/a00110.html for an explanation of the
    definitions contained in this file.
******************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#if defined(PHFL_APP_NXBUILD_CONFIG) || defined(NXPBUILD_CUSTOMER_HEADER_INCLUDED)
#   include <ph_NxpBuild.h>
#endif
#ifdef __GNUC__
#  include <stdint.h>
#endif
/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 * http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#if defined(__GNUC__) || defined (__ARMCC_VERSION) || defined (__ICCARM__)
extern uint32_t SystemCoreClock;
#endif

/* Cortex M33 port configuration. */
#if defined (__PN76XX__)
#define configENABLE_MPU                                0
#define configENABLE_FPU                                0
#define configENABLE_TRUSTZONE                          1
#endif

#define configUSE_PREEMPTION                            1
#define configUSE_IDLE_HOOK                             0
#define configMAX_PRIORITIES                            ( 6 )
#define configUSE_TICK_HOOK                             0
#define configCPU_CLOCK_HZ                          ( ( unsigned long ) SystemCoreClock)
#define configTICK_RATE_HZ                          ( ( TickType_t ) 1000 )
#define configMINIMAL_STACK_SIZE                    ( ( unsigned short ) 256/sizeof( StackType_t ) )
#define configSUPPORT_STATIC_ALLOCATION                 0

#define configMINIMAL_SECURE_STACK_SIZE     ( ( unsigned short ) 1024/sizeof( StackType_t ) )



#if defined (__PN74XXXX__) || defined (__PN73XXXX__) || defined (__PN76XX__)
#   if ( configSUPPORT_STATIC_ALLOCATION == 1 )
#      error "Not supported for PN7462AU"
#   else
#       ifndef configTOTAL_HEAP_SIZE
#           define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 5 * 1024 ) )
#       endif
#   endif
#else
#   if ( configSUPPORT_STATIC_ALLOCATION == 1 )
#      define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 0 * 1024 ) )
#   else
#      define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 12 * 1024 ) )
#   endif
#endif

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
#define configSUPPORT_DYNAMIC_ALLOCATION                0
#else
#define configSUPPORT_DYNAMIC_ALLOCATION                1
#endif
#define configMAX_TASK_NAME_LEN                         ( 25 )
#define configUSE_TRACE_FACILITY                        0
#define configUSE_16_BIT_TICKS                          0
#define configUSE_TIME_SLICING                          1
#define configIDLE_SHOULD_YIELD                         1
#define configUSE_CO_ROUTINES                           0
#define configUSE_MUTEXES                               1

/* Constants that build features in or out. */
#if defined (__PN76XX__)
#define configUSE_TICKLESS_IDLE                         1
#else
#define configUSE_TICKLESS_IDLE                         0
#endif /* __PN76XX__ */
#define configUSE_APPLICATION_TASK_TAG                  0
#define configUSE_NEWLIB_REENTRANT                      0
#define configUSE_COUNTING_SEMAPHORES                   1
#define configCHECK_FOR_STACK_OVERFLOW                  2
#define configUSE_RECURSIVE_MUTEXES                     0
#define configUSE_QUEUE_SETS                            0
#define configUSE_TASK_NOTIFICATIONS                    1

#define configGENERATE_RUN_TIME_STATS                   0
#define configUSE_MALLOC_FAILED_HOOK                    1

/* Constants provided for debugging and optimisation assistance. */
#define configASSERT( x )                               if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }
#define configQUEUE_REGISTRY_SIZE                       0

/* Software timer definitions. */
#define configUSE_TIMERS                                1
#define configTIMER_TASK_PRIORITY                       ( 5 )
#if defined (__PN74XXXX__) || defined (__PN73XXXX__) || defined (__PN76XX__)
/* Since PN7462 I2cm, RF, SPIM, CT and other hals are all Blocking calls.
 * Shall use the event mechanism in ISR contexts. This would lead to Message posting to
 * Timer Daemon Task at run time, message queue length of 2 is not enough.
 * So making the queue length to 8.
 *  */
#    define configTIMER_QUEUE_LENGTH        8
#else
#    define configTIMER_QUEUE_LENGTH        2
#endif

#define configTIMER_TASK_STACK_DEPTH                    ( ( unsigned short ) 400/sizeof( StackType_t ) )

/* Set the following definitions to 1 to include the API function, or zero
 * to exclude the API function.  NOTE:  Setting an INCLUDE_ parameter to 0 is
 * only necessary if the linker does not automatically remove functions that are
 * not referenced anyway. */
#define INCLUDE_vTaskPrioritySet                        1
#define INCLUDE_uxTaskPriorityGet                       1
#define INCLUDE_vTaskDelete                             1
#define INCLUDE_vTaskCleanUpResources                   1
#define INCLUDE_vTaskSuspend                            1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_vTaskDelay                              1
#define INCLUDE_uxTaskGetStackHighWaterMark             0
#define INCLUDE_xTaskGetIdleTaskHandle                  0
#define INCLUDE_eTaskGetState                           1
#define INCLUDE_xTaskResumeFromISR                      0
#define INCLUDE_xTaskGetCurrentTaskHandle               1
#define INCLUDE_xTaskGetSchedulerState                  0
#define INCLUDE_xTimerPendFunctionCall                  1
#define INCLUDE_xSemaphoreGetMutexHolder                1

/*
 * Set configUSE_TRACE_FACILITY and configUSE_STATS_FORMATTING_FUNCTIONS to 1 to include the vTaskList() and vTaskGetRunTimeStats()
 * functions in the build.
 *
 * This demo makes use of one or more example stats formatting functions.  These
 * format the raw data provided by the uxTaskGetSystemState() function in to
 * human readable ASCII form.  See the notes in the implementation of vTaskList()
 * within FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS            0

/* Interrupt priority configuration follows...................... */

/* Use the system definition, if there is one */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS __NVIC_PRIO_BITS
#else
    #if defined (__PN76XX__)
        #define configPRIO_BITS       3 /* 8 priority levels. */
    #else
        #if defined (CORE_M3)
            #define configPRIO_BITS   5 /* 32 priority levels */
        #else
            #define configPRIO_BITS   4 /* 16 priority levels */
        #endif
    #endif
#endif


#if defined (__PN76XX__)
/* CORE_M33 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION         0

/* The lowest interrupt priority that can be used in a call to a "set priority"
 * function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0x07

/* The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT
 * CALL INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A
 * HIGHER PRIORITY THAN THIS! (higher priorities are lower numeric values). */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
 * to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY                 ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#else
/* The lowest priority. */
#define configKERNEL_INTERRUPT_PRIORITY     ( 31 << (8 - configPRIO_BITS) )
/* Priority 5, or 160 as only the top three bits are implemented. */
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5 << (8 - configPRIO_BITS) )
#endif

#ifdef CORE_M3
/*
 * Use the Cortex-M3 optimisations, rather than the generic C implementation.
 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif   /* CORE_M3 */

#ifdef PH_OSAL_FREERTOS
#   define vPortSVCHandler     SVC_Handler
#   define xPortPendSVHandler  PendSV_Handler
#   define xPortSysTickHandler SysTick_Handler
#endif

/* The #ifdef guards against the file being included from IAR assembly files. */
#ifndef __IASMARM__

    /* Constants related to the generation of run time stats. */
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
    #define portGET_RUN_TIME_COUNTER_VALUE()            0

#endif /* __IASMARM__ */

/*
#define configASSERT( x )   if( ( x ) == 0 ) {printf("File %s Line %d :", __FILE__, __LINE__); }
*/

#endif /* FREERTOS_CONFIG_H */
