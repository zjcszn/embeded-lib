/*----------------------------------------------------------------------------*/
/* Copyright 2021,2023 NXP                                                    */
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
* Implementation of Operating System Abstraction Layer for PN76xx NFC Controllers.
*
* This file contains TickTimer related functionality implementation required
* by the NullOs.
*
* $Author:: NXP $
* $Revision$
* History:
*/

#include "phOsal.h"
#include "phOsal_NullOs_Port.h"
#include "ph_Datatypes.h"
#include "PN76xx.h"
#include "core_cm33.h"
/**
 * NULL OS implementation of PN76XX controller is just stub.
 * In NULL OS implementation, RdLib with PN76XX HAL will not use any phOsal functionality.
 */
#if defined(PH_OSAL_NULLOS) && defined(__PN76XX__)

#include "phOsal_Cortex_Port.h"

//#ifdef __GNUC__
//    #define __ENABLE_IRQ() __asm volatile ("cpsie i")
//    #define __DISABLE_IRQ() __asm volatile ("cpsid i")
//    #define __WFE() __asm volatile ("wfe")
//    #define __SEV() __asm volatile ("sev")
//#endif /* __GNUC__ */

#ifndef __ENABLE_IRQ
    #define __ENABLE_IRQ __enable_irq
#endif
#ifndef __DISABLE_IRQ
    #define __DISABLE_IRQ __disable_irq
#endif
#ifndef __WFE
    #define __WFE __wfe
#endif
#ifndef __SEV
    #define __SEV __sev
#endif

//#ifdef __ICCARM__
//#   include "intrinsics.h"
//#   define __NOP             __no_operation
//#   define __ENABLE_IRQ      __enable_interrupt
//#   define __DISABLE_IRQ     __disable_interrupt
//#endif

#define SYSTICK_TIMER_MAX           0xFFFFFFU    /* [23:0] bits Timer. */
#define SYSTICK_TIMER_CLK           0x00000004
#define SYSTICK_TIMER_INT           0x00000002
#define SYSTICK_TIMER_ENABLE        0x00000001

#define PH_PLATFORM_TIMER_UNIT_MS       1000U      /**< Indicates that the specified delay is in milliseconds. */

static pphOsal_TickTimerISRCallBck_t pTickCallBack;
static  uint64_t qwLoadValue;

/* Timer rate. */
static uint32_t dwSysTickTimerFreq;


phStatus_t phOsal_InitTickTimer(pphOsal_TickTimerISRCallBck_t pTickTimerCallback)
{
   pTickCallBack = pTickTimerCallback;

   qwLoadValue = 0;

   /* SysTick Timer rate is system clock rate. */
   dwSysTickTimerFreq = 45000000; //45Mhz

   /* Disable systick and clear the Load value. */
   SysTick->CTRL = 0x0;
   SysTick->LOAD = 0x0;

   return PH_OSAL_SUCCESS;
}

static void phOsal_ConfigTick(void)
{
    /* Disable systick */
   SysTick->CTRL = 0x0;


    if(qwLoadValue > SYSTICK_TIMER_MAX)
    {
        qwLoadValue -= SYSTICK_TIMER_MAX;
        SysTick->LOAD = SYSTICK_TIMER_MAX;
    }
    else
    {
       SysTick->LOAD = (uint32_t)(qwLoadValue & SYSTICK_TIMER_MAX);
       qwLoadValue = 0;
    }

    /*Clear the current count value and also SysTick CTRL.COUNTFLAG. */
    SysTick->VAL = 0;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

phStatus_t phOsal_StartTickTimer(uint32_t dwTimeMilliSecs)
{
   qwLoadValue = ((uint64_t)dwTimeMilliSecs * (uint64_t)dwSysTickTimerFreq)/PH_PLATFORM_TIMER_UNIT_MS;

   phOsal_ConfigTick();

   return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_StopTickTimer(void)
{
   /* Disable systick and clear the Load value. */
   SysTick->CTRL = 0x0;
   SysTick->LOAD = 0x0;

   return PH_OSAL_SUCCESS;
}

void phOsal_EnterCriticalSection(void)
{
    __DISABLE_IRQ();
}

void phOsal_ExitCriticalSection(void)
{
    __ENABLE_IRQ();
}

void phOsal_Sleep(void)
{
    __WFE();
}

void phOsal_WakeUp(void)
{
    __SEV();
}

void SysTick_Handler(void)
{
    if(qwLoadValue)
    {
        if(qwLoadValue > SYSTICK_TIMER_MAX)
        {
            qwLoadValue -= SYSTICK_TIMER_MAX;
        }
        else
        {
            /* Timer to be configured. */
            phOsal_ConfigTick();
        }
    }
    else
    {
        /* Stop the Timer and call the callBack. */
        phOsal_StopTickTimer();

        pTickCallBack();
    }
}

#endif /* defined(PH_OSAL_NULLOS) && defined(__PN76XX__) */

