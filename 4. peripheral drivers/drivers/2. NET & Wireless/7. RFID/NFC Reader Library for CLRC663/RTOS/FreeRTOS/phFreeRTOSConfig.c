/*----------------------------------------------------------------------------*/
/* Copyright 2016-2022 NXP                                                    */
/*                                                                            */
/* NXP Confidential. This software is owned or controlled by NXP and may only */
/* be used strictly in accordance with the applicable license terms.          */
/* By expressly accepting such terms or by downloading, installing,           */
/* activating and/or otherwise using the software, you are agreeing that you  */
/* have read, and that you agree to comply with and are bound by, such        */
/* license terms. If you do not agree to be bound by the applicable license   */
/* terms, then you may not retain, install, activate or otherwise use the     */
/* software.                                                                  */
/*----------------------------------------------------------------------------*/

/** @file
* File used to implement Callback/Hooks required for FreeRTOS.
* $Author: NXP $
* $Revision:  $
* $Date: $
*
*/

/*----------------------------------------------------------------------------------------------
 * Includes
 ---------------------------------------------------------------------------------------------*/
#include <FreeRTOS.h>
#include <task.h>

/*-----------------------------------------------------------*/

/* Stack overflow hook. */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Force an assert. */
    configASSERT(pcTaskName == 0);
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that
 * is used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                           StackType_t ** ppxIdleTaskStackBuffer,
                           uint32_t * pulIdleTaskStackSize )
{
   /* If the buffers to be provided to the Idle task are declared inside this
    * function then they must be declared static - otherwise they will be
    * allocated on the stack and so not exists after this function exits. */
   static StaticTask_t xIdleTaskTCB;
   static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ] __attribute__( ( aligned( 32 ) ) );

   /* Pass out a pointer to the StaticTask_t structure in which the Idle
    * task's state will be stored. */
   *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

   /* Pass out the array that will be used as the Idle task's stack. */
   *ppxIdleTaskStackBuffer = uxIdleTaskStack;

   /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    * Note that, as the array is necessarily of type StackType_t,
    * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
   *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                            StackType_t ** ppxTimerTaskStackBuffer,
                            uint32_t * pulTimerTaskStackSize )
{
   /* If the buffers to be provided to the Timer task are declared inside this
    * function then they must be declared static - otherwise they will be
    * allocated on the stack and so not exists after this function exits. */
   static StaticTask_t xTimerTaskTCB;
   static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ] __attribute__( ( aligned( 32 ) ) );

   /* Pass out a pointer to the StaticTask_t structure in which the Timer
    * task's state will be stored. */
   *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

   /* Pass out the array that will be used as the Timer task's stack. */
   *ppxTimerTaskStackBuffer = uxTimerTaskStack;

   /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    * Note that, as the array is necessarily of type StackType_t,
    * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
   *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/


void vApplicationMallocFailedHook(void)
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}
