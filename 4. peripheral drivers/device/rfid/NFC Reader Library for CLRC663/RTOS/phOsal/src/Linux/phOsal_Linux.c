/*
 *         Copyright (c), NXP Semiconductors Bangalore / India
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 * particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */

/** \file
 * Linux OSAL Component of Reader Library Framework.
 * $Author: Ankur Srivastava (nxp79569) $
 * $Revision: 6397 $ (v4.040.05.001642)
 * $Date: 2016-10-20 23:09:50 +0530 (Thu, 20 Oct 2016) $
 *
 * History:
 *  PC: Generated 23. Aug 2012
 *
 */
#include <phOsal.h>

#ifdef PH_OSAL_LINUX

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "phOsal_Linux.h"
#include <errno.h>


/*******************************************************************************
**   Defines
*******************************************************************************/
#define PH_SCHEDULER_OFF    0
#define PH_SCHEDULER_ON     1


/* *****************************************************************************************************************
 * Global and Static Variables
 * Total Size: NNNbytes
 * ***************************************************************************************************************** */
static volatile uint8_t gbSchedulerStarted;
static pthread_t gpTaskArr[PH_OSAL_CONFIG_LINUX_MAX_NUM_TASKS];
static volatile uint32_t dwNumTaskCreated;

typedef struct {
    uint32_t dwEvents;
    pthread_cond_t EventCond;
    pthread_condattr_t attr;
}phOsal_LinuxEventGroup_t;

static volatile phOsal_LinuxEventGroup_t gsEventGroup[PH_OSAL_CONFIG_LINUX_MAX_NUM_EVENTS];
static volatile uint32_t gdwEventBitMap;
static volatile phOsal_MutexObj_t gsEventMutexObj;

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
static phStatus_t phOsal_Linux_GetFreeIndex(uint32_t * dwFreeIndex, uint32_t dwBitMap, uint32_t dwMaxLimit);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
phStatus_t phOsal_Init(void)
{
    gbSchedulerStarted= PH_SCHEDULER_OFF;
    dwNumTaskCreated = 0;
    phStatus_t status;
    memset(gpTaskArr, 0, sizeof(gpTaskArr));

    /* Create the mutex for events. */
    status = phOsal_MutexCreate((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle), (pphOsal_MutexObj_t)(&gsEventMutexObj));

    return status;
}

phStatus_t phOsal_ThreadCreate(phOsal_Thread_t *threadHandle, pphOsal_ThreadObj_t threadObj,
    pphOsal_StartFunc_t startFunc, void *arg)
{
    phStatus_t status;
    struct sched_param sSchedParam;
    pthread_attr_t attr;
    pthread_t pThread;

    if ((threadHandle == NULL) || (threadObj == NULL) || (startFunc == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status =  pthread_attr_init(&attr);
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    sSchedParam.sched_priority = threadObj->priority;
    status =  pthread_attr_setschedparam(&attr,&sSchedParam);
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status =  pthread_attr_setstacksize(&attr, (size_t)(threadObj->stackSizeInNum));
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = pthread_create(&pThread, NULL, (void *)(startFunc), arg);
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    *threadHandle = (phOsal_Thread_t) pThread;
    threadObj->ThreadHandle = *threadHandle;
    if (gbSchedulerStarted == PH_SCHEDULER_OFF)
    {
        gpTaskArr[dwNumTaskCreated] = pThread;
        dwNumTaskCreated++;
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_ThreadDelete(phOsal_Thread_t * threadHandle)
{
    phStatus_t retStatus;
    /* By Default cancellability state of the thread is enabled. *
     * By Default cancellability type for the thread is deferred. Hence this cannot be deleted asynchronously */
    int api_status = pthread_cancel((pthread_t)(*threadHandle));
    if (api_status == 0U)
        retStatus = PH_OSAL_SUCCESS;
    else
        retStatus = PH_OSAL_ERROR;
    return retStatus;
}

phStatus_t phOsal_ThreadChangePrio(phOsal_Thread_t * threadHandle, uint32_t newPrio)
{
    phStatus_t status;
    struct sched_param sSchedParam;

    if ((threadHandle == NULL) || ((*threadHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    sSchedParam.sched_priority = newPrio;
    status = pthread_setschedparam((pthread_t)(*threadHandle), SCHED_RR, &sSchedParam);
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_ThreadExit(void)
{
    pthread_exit(NULL);

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_ThreadSecureStack(uint32_t stackSizeInNum)
{
   return (PH_OSAL_UNSUPPORTED_COMMAND | PH_COMP_OSAL);
}

phStatus_t phOsal_ThreadDelay(phOsal_Ticks_t ticksToSleep)
{
    phStatus_t status;
    struct timespec sTimeSpecReq;
    struct timespec sTimeSpecRem;

    sTimeSpecReq.tv_sec = (ticksToSleep / 1000);
    sTimeSpecReq.tv_nsec = (ticksToSleep % 1000) * 1000 * 1000;

    status = nanosleep(&sTimeSpecReq, &sTimeSpecRem);
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_EventCreate(phOsal_Event_t *eventHandle, pphOsal_EventObj_t eventObj)
{
    uint32_t bEventIndex = 0U;
    phStatus_t status;

    if ((eventHandle == NULL) || (eventObj == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = phOsal_Linux_GetFreeIndex(&bEventIndex, gdwEventBitMap, PH_OSAL_CONFIG_LINUX_MAX_NUM_EVENTS);
    PH_OSAL_CHECK_SUCCESS(status);

    memset((void *)(&gsEventGroup[bEventIndex]), 0, sizeof(phOsal_LinuxEventGroup_t));

    gdwEventBitMap |= (1U << bEventIndex);
    *eventHandle = (phOsal_Event_t)(&gsEventGroup[bEventIndex].dwEvents);
    eventObj->EventHandle = (phOsal_Event_t)(&gsEventGroup[bEventIndex].dwEvents);
    eventObj->dwEventIndex = bEventIndex;

    status = pthread_condattr_init((pthread_condattr_t *)(&gsEventGroup[bEventIndex].attr));
    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = pthread_cond_init((pthread_cond_t *)(&gsEventGroup[bEventIndex].EventCond),
        (pthread_condattr_t *)(&gsEventGroup[bEventIndex].attr));

    if (status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;

}

phStatus_t phOsal_EventPend(volatile phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_Ticks_t ticksToWait,
    phOsal_EventBits_t FlagsToWait, phOsal_EventBits_t *pCurrFlags)
{
    phStatus_t statusTmp;
    phStatus_t status;
    struct timespec sTimeSpec = {0};
    phOsal_TimerPeriodObj_t timePeriodToWait;

    if ((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = PH_OSAL_IO_TIMEOUT;
    statusTmp = PH_OSAL_SUCCESS;
    timePeriodToWait.period = ticksToWait;
    timePeriodToWait.unitPeriod = OS_TIMER_UNIT_MSEC;

    status = phOsal_MutexLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle), timePeriodToWait);
    PH_OSAL_CHECK_SUCCESS(status);

    while(1)
    {
        if (((options & E_OS_EVENT_OPT_PEND_SET_ALL) && (((*((uint32_t *)(*eventHandle))) & FlagsToWait) == FlagsToWait))
            || ((!(options & E_OS_EVENT_OPT_PEND_SET_ALL)) && ((*((uint32_t *)(*eventHandle))) & FlagsToWait)))
        {
            status =  PH_OSAL_SUCCESS;
            break;
        }

        /* Check whether infinite wait, if not config timer. */
        if (ticksToWait != PHOSAL_MAX_DELAY)
        {
            status = clock_gettime(CLOCK_REALTIME, &sTimeSpec);
            if(status != PH_OSAL_SUCCESS)
            {
                status = PH_OSAL_FAILURE;
                break;
            }

            sTimeSpec.tv_sec += (ticksToWait/ 1000);
            sTimeSpec.tv_nsec += (ticksToWait% 1000) * 1000 * 1000;

            /* Make sure sTimeSpec is valid */
            sTimeSpec.tv_sec += sTimeSpec.tv_nsec / (1000 * 1000 * 1000);
            sTimeSpec.tv_nsec %= (1000 * 1000 * 1000);

            status = pthread_cond_timedwait(&(((phOsal_LinuxEventGroup_t *)(*eventHandle))->EventCond), gsEventMutexObj.MutexHandle, &sTimeSpec);
            if (status != PH_OSAL_SUCCESS)
            {
                if (status == ETIMEDOUT)
                {
                    status = PH_OSAL_IO_TIMEOUT;
                }
                else
                {
                    status = PH_OSAL_FAILURE;
                }
                break;
            }
        }
        else
        {
            status = pthread_cond_wait(&(((phOsal_LinuxEventGroup_t *)(*eventHandle))->EventCond), gsEventMutexObj.MutexHandle);
            if (status != PH_OSAL_SUCCESS)
            {
                status = PH_OSAL_FAILURE;
                break;
            }
        }
    }

    if (pCurrFlags != NULL)
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
    }

    if (options & E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT)
    {
        (*((uint32_t *)(*eventHandle))) &= (~(FlagsToWait & (*((uint32_t *)(*eventHandle)))));
    }

    if (status != PH_OSAL_SUCCESS)
    {
        statusTmp = status;
    }

    status = phOsal_MutexUnLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle));

    if (statusTmp != PH_OSAL_SUCCESS)
    {
        status = statusTmp;
    }

    return PH_OSAL_ADD_COMPCODE(status, PH_COMP_OSAL);
}

phStatus_t phOsal_EventPost(phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_EventBits_t FlagsToPost,
    phOsal_EventBits_t *pCurrFlags)
{
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phStatus_t status;
    phStatus_t statusTmp;

    if ((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    statusTmp = PH_OSAL_SUCCESS;
    /* Lock the mutex for 1 sec. */
    timePeriodToWait.unitPeriod = OS_TIMER_UNIT_SEC;
    timePeriodToWait.period = 1;

    status = phOsal_MutexLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle), timePeriodToWait);
    PH_OSAL_CHECK_SUCCESS(status);

    /* Set the events. */
    (*((uint32_t *)(*eventHandle))) |= FlagsToPost;

    if (pCurrFlags != NULL)
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
    }

    status = pthread_cond_broadcast(&(((phOsal_LinuxEventGroup_t *)(*eventHandle))->EventCond));
    if (status != PH_OSAL_SUCCESS)
    {
        statusTmp = PH_OSAL_FAILURE;
    }

    status = phOsal_MutexUnLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle));
    PH_OSAL_CHECK_SUCCESS(status);

    status = statusTmp;

    return PH_OSAL_ADD_COMPCODE(status, PH_COMP_OSAL);
}

phStatus_t phOsal_EventClear(phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_EventBits_t FlagsToClear,
    phOsal_EventBits_t *pCurrFlags)
{
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phStatus_t status;

    if ((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }
    /* Lock the mutex for 1 sec. */
    timePeriodToWait.unitPeriod = OS_TIMER_UNIT_SEC;
    timePeriodToWait.period = 1;

    status = phOsal_MutexLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle), timePeriodToWait);
    PH_OSAL_CHECK_SUCCESS(status);

    if (pCurrFlags != NULL)
    {
        *pCurrFlags = (*((uint32_t *)(*eventHandle)));
    }

    (*((uint32_t *)(*eventHandle))) &= (~FlagsToClear);

    return phOsal_MutexUnLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle));
}

phStatus_t phOsal_EventGet(phOsal_Event_t * eventHandle, phOsal_EventBits_t *pCurrFlags)
{
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phStatus_t status;

    if ((eventHandle == NULL) || ((*eventHandle) == NULL) || (pCurrFlags == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    /* Lock the mutex for 1 sec. */
    timePeriodToWait.unitPeriod = OS_TIMER_UNIT_SEC;
    timePeriodToWait.period = 1;

    status = phOsal_MutexLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle), timePeriodToWait);
    PH_OSAL_CHECK_SUCCESS(status);

    *pCurrFlags = (*((uint32_t *)(*eventHandle)));

    return phOsal_MutexUnLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle));

}

phStatus_t phOsal_EventDelete(phOsal_Event_t * eventHandle)
{
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phStatus_t status;
    phStatus_t statusTmp;

    if ((eventHandle == NULL) || ((*eventHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    statusTmp = PH_OSAL_SUCCESS;
    /* Lock the mutex for 1 sec. */
    timePeriodToWait.unitPeriod = OS_TIMER_UNIT_SEC;
    timePeriodToWait.period = 1;

    status = phOsal_MutexLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle), timePeriodToWait);
    PH_OSAL_CHECK_SUCCESS(status);

    gdwEventBitMap &= ~(1U << (((phOsal_EventObj_t *)(eventHandle))->dwEventIndex));

    status = pthread_cond_destroy(&(((phOsal_LinuxEventGroup_t *)(*eventHandle))->EventCond));
    if (status != PH_OSAL_SUCCESS)
    {
        statusTmp = PH_OSAL_FAILURE;
    }

    memset(*eventHandle, 0, sizeof(phOsal_LinuxEventGroup_t));

    status = phOsal_MutexUnLock((phOsal_Mutex_t *)(&gsEventMutexObj.MutexHandle));
    PH_OSAL_CHECK_SUCCESS(status);

    return PH_OSAL_ADD_COMPCODE(statusTmp, PH_COMP_OSAL);
}

phStatus_t phOsal_SemCreate(phOsal_Semaphore_t *semHandle, pphOsal_SemObj_t semObj, phOsal_SemOpt_t opt)
{
    sem_t *pSem;
    phStatus_t status;

    if ((semHandle == NULL) || (semObj == NULL) || (opt >= E_OS_SEM_OPT_MAX))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    pSem = malloc(sizeof(sem_t));
    if( pSem == NULL )
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    /* pshared = 0; semaphore is shared between threads of a process. */
    status = sem_init(pSem, 0, semObj->semInitialCount);
    if(status != PH_OSAL_SUCCESS)
    {
        free(pSem);
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    *semHandle = pSem;
    semObj->SemHandle = *semHandle;

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_SemPend(phOsal_Semaphore_t * semHandle, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    struct timespec sTimerSpec;
    phStatus_t status;

    if ((semHandle == NULL) || ((*semHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    if (timePeriodToWait.period == PHOSAL_MAX_DELAY)
    {
        status = sem_wait(*semHandle);
        if(status != PH_OSAL_SUCCESS)
        {
            return (PH_OSAL_FAILURE | PH_COMP_OSAL);
        }
    }
    else
    {
        status = clock_gettime(CLOCK_REALTIME, &sTimerSpec);
        if(status != PH_OSAL_SUCCESS)
        {
            return (PH_OSAL_FAILURE | PH_COMP_OSAL);
        }

        if (timePeriodToWait.unitPeriod == OS_TIMER_UNIT_MSEC)
        {
            sTimerSpec.tv_sec += (timePeriodToWait.period / 1000);
            sTimerSpec.tv_nsec += (timePeriodToWait.period % 1000) * 1000 * 1000;
        }
        else
        {
            /* timePeriodToWait.unitPeriod = OS_TIMER_UNIT_SEC */
            sTimerSpec.tv_sec += timePeriodToWait.period;
        }

        /* Make sure sTimerSpec is valid */
        sTimerSpec.tv_sec += sTimerSpec.tv_nsec / (1000 * 1000 * 1000);
        sTimerSpec.tv_nsec %= (1000 * 1000 * 1000);

        status = sem_timedwait(*semHandle, &sTimerSpec);
        if(status != PH_OSAL_SUCCESS)
        {
            return (PH_OSAL_FAILURE | PH_COMP_OSAL);
        }
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_SemPost(phOsal_Semaphore_t * semHandle, phOsal_SemOpt_t opt)
{
    phStatus_t status;

    if ((semHandle == NULL) || ((*semHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = sem_post(*semHandle);
    if(status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_SemDelete(phOsal_Semaphore_t * semHandle)
{
    phStatus_t status;

    if ((semHandle == NULL) || ((*semHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = PH_OSAL_SUCCESS;

    status = sem_destroy(*semHandle);
    if(status != PH_OSAL_SUCCESS)
    {
        status = (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    free(*semHandle);

    return status;
}

phStatus_t phOsal_MutexCreate(phOsal_Mutex_t *mutexHandle, pphOsal_MutexObj_t mutexObj)
{
    phStatus_t status;
    pthread_mutex_t *pThreadMutex;
    pthread_mutexattr_t attr;

    if ((mutexHandle == NULL) || (mutexObj == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    pThreadMutex = malloc(sizeof(pthread_mutex_t));
    if (pThreadMutex == NULL)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    status = pthread_mutexattr_init(&attr);
    if (status != PH_OSAL_SUCCESS)
    {
        free(pThreadMutex);
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (status != PH_OSAL_SUCCESS)
    {
        free(pThreadMutex);
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = pthread_mutex_init(pThreadMutex, &attr);
    if (status != PH_OSAL_SUCCESS)
    {
        free(pThreadMutex);
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    *mutexHandle = pThreadMutex;
    mutexObj->MutexHandle = *mutexHandle;

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_MutexLock(phOsal_Mutex_t * mutexHandle, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    phStatus_t status;
    pthread_mutex_t *pMutex;
    struct timespec tspec;

    if ((mutexHandle == NULL) || ((*mutexHandle) == NULL) || (timePeriodToWait.unitPeriod > OS_TIMER_UNIT_SEC))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    pMutex = (pthread_mutex_t*)(*mutexHandle);

    if (timePeriodToWait.period == PHOSAL_MAX_DELAY)
    {
        status = pthread_mutex_lock(pMutex);
        if(status != PH_OSAL_SUCCESS)
        {
            return (PH_OSAL_FAILURE | PH_COMP_OSAL);
        }
    }
    else
    {
        status = clock_gettime(CLOCK_REALTIME, &tspec);
        if(status != PH_OSAL_SUCCESS)
        {
            return (PH_OSAL_FAILURE | PH_COMP_OSAL);
        }

        if (timePeriodToWait.unitPeriod == OS_TIMER_UNIT_MSEC)
        {
            tspec.tv_sec += (timePeriodToWait.period / 1000);
            tspec.tv_nsec += (timePeriodToWait.period % 1000) * 1000 * 1000;
        }
        else
        {
            /* timePeriodToWait.unitPeriod = OS_TIMER_UNIT_SEC */
            tspec.tv_sec += timePeriodToWait.period;
        }

        /* Make sure tspec is valid */
        tspec.tv_sec += tspec.tv_nsec / (1000 * 1000 * 1000);
        tspec.tv_nsec %= (1000 * 1000 * 1000);

        status = pthread_mutex_timedlock(pMutex, &tspec);
        if(status != PH_OSAL_SUCCESS)
        {
            return (PH_OSAL_FAILURE | PH_COMP_OSAL);
        }
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_MutexUnLock(phOsal_Mutex_t * mutexHandle)
{
    pthread_mutex_t *pMutex;
    phStatus_t status;

    if ((mutexHandle == NULL) || ((*mutexHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    pMutex = (pthread_mutex_t*)(*mutexHandle);

    status = pthread_mutex_unlock(pMutex);
    if(status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_MutexDelete(phOsal_Mutex_t * mutexHandle)
{
    pthread_mutex_t *pMutex;
    phStatus_t status;

    if ((mutexHandle == NULL) || ((*mutexHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    pMutex = (pthread_mutex_t*)(*mutexHandle);

    status = pthread_mutex_destroy(pMutex);
    if(status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    free(pMutex);

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_TimerCreate(phOsal_Timer_t *timerHandle, pphOsal_TimerObj_t timerObj)
{
    typedef void (*sigEvCb_t )(union sigval);
    timer_t *pTimer;
    phStatus_t status;
    struct sigevent sev;

    if ((timerHandle == NULL) || (timerObj == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    pTimer = malloc( sizeof(timer_t) );
    if(pTimer == NULL)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_signo = 0;
    sev.sigev_value.sival_ptr = timerObj->arg;
    sev.sigev_notify_function = (sigEvCb_t) timerObj->timerCb;
    sev.sigev_notify_attributes = NULL;
    /* : sev.sigev_notify_thread_id = 0; */

    status = timer_create(CLOCK_REALTIME, &sev, pTimer);
    if(status != PH_OSAL_SUCCESS)
    {
        free(pTimer);
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    *timerHandle = pTimer;
    timerObj->TimerHandle = *timerHandle;

    return status;
}

phStatus_t phOsal_TimerStart(phOsal_Timer_t *timerHandle)
{
    phStatus_t status;
    struct itimerspec sTimerSpec;
    pphOsal_TimerObj_t pTimerObj;

    pTimerObj = (pphOsal_TimerObj_t)(timerHandle);

    if ((*timerHandle == NULL) || (pTimerObj->opt >= E_OS_TIMER_OPT_MAX) || (pTimerObj->timePeriod.unitPeriod >= E_OS_TIMER_UNIT_MAX))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    /* Update Timing for Free Running Timer. */
    if (pTimerObj->opt == OS_TIMER_OPT_PERIODIC)
    {
        if (pTimerObj->timePeriod.unitPeriod == OS_TIMER_UNIT_MSEC)
        {
            sTimerSpec.it_interval.tv_sec = (pTimerObj->timePeriod.period / 1000);
            sTimerSpec.it_interval.tv_nsec = (pTimerObj->timePeriod.period % 1000) * 1000 * 1000;
        }
        else
        {
            /* pTimerObj->timePeriod.unitPeriod = OS_TIMER_UNIT_SEC */
            sTimerSpec.it_interval.tv_sec = pTimerObj->timePeriod.period;
            sTimerSpec.it_interval.tv_nsec = 0; /**< This evaluates to zero. */
        }
    }
    else
    {
        sTimerSpec.it_interval.tv_sec = 0;
        sTimerSpec.it_interval.tv_nsec = 0;
    }

    /* Update the Timing. */
    if (pTimerObj->timePeriod.unitPeriod == OS_TIMER_UNIT_MSEC)
    {
        sTimerSpec.it_value.tv_sec = (pTimerObj->timePeriod.period / 1000);
        sTimerSpec.it_value.tv_nsec = (pTimerObj->timePeriod.period % 1000) * 1000 * 1000;
    }
    else
    {
        /* pTimerObj->timePeriod.unitPeriod = OS_TIMER_UNIT_SEC */
        sTimerSpec.it_value.tv_sec = pTimerObj->timePeriod.period;
        sTimerSpec.it_value.tv_nsec = 0; /**< This evaluates to zero. */
    }

    /* Start a relative timer : 0 as second parameter. */
    status = timer_settime(*((timer_t *)(*timerHandle)), 0, &sTimerSpec, NULL);
    if(status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_TimerStop(phOsal_Timer_t * timerHandle)
{
    phStatus_t status;
    struct itimerspec sTimerSpec;

    if ((timerHandle == NULL) || ((*timerHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    /* Stop the timer set the timings to zero i.e. spec = 0. */
    memset(&sTimerSpec, 0, sizeof(struct itimerspec));

    status = timer_settime(*((timer_t *)(*timerHandle)), 0, &sTimerSpec, NULL);
    if(status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_TimerGetCurrent(phOsal_Timer_t * timerHandle, uint32_t * dwGetElapsedTime)
{
    struct itimerspec sTimerSpec;
    phStatus_t status;

    if ((timerHandle == NULL) || ((*timerHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    memset(&sTimerSpec, 0, sizeof(struct itimerspec));
    *dwGetElapsedTime = 0;

    status = timer_gettime(*((timer_t *)(*timerHandle)), &sTimerSpec);
    if( status != 0 )
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    *dwGetElapsedTime = (sTimerSpec.it_value.tv_sec * 1000) + (sTimerSpec.it_value.tv_nsec / (1000 * 1000));

    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_TimerModify(phOsal_Timer_t * timerHandle, pphOsal_TimerObj_t timerObj)
{
    return PH_OSAL_SUCCESS;
}

phStatus_t phOsal_TimerDelete(phOsal_Timer_t * timerHandle)
{
    phStatus_t status;

    if ((timerHandle == NULL) || ((*timerHandle) == NULL))
    {
        return (PH_OSAL_ERROR | PH_COMP_OSAL);
    }

    status = timer_delete(*((timer_t *)(*timerHandle)));
    if(status != PH_OSAL_SUCCESS)
    {
        return (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }

    free(*timerHandle);

    return PH_OSAL_SUCCESS;
}

void phOsal_StartScheduler(void)
{
    uint32_t dwTaskLoop;

    if (gbSchedulerStarted == PH_SCHEDULER_OFF)
    {
        gbSchedulerStarted = PH_SCHEDULER_ON;

        for(dwTaskLoop = 0; dwTaskLoop < dwNumTaskCreated; dwTaskLoop++)
        {
            pthread_join((gpTaskArr[dwTaskLoop]), NULL);
        }
    }
}

/* *****************************************************************************************************************
 * Private Functions
 * ***************************************************************************************************************** */
static phStatus_t phOsal_Linux_GetFreeIndex(uint32_t * dwFreeIndex, uint32_t dwBitMap, uint32_t dwMaxLimit)
{
    phStatus_t status;

    (*dwFreeIndex) = 0;

    while(((1U << (*dwFreeIndex)) & dwBitMap) && ((*dwFreeIndex) < dwMaxLimit))
    {
        (*dwFreeIndex)++;
    }

    if (*dwFreeIndex == dwMaxLimit)
    {
        status = (PH_OSAL_FAILURE | PH_COMP_OSAL);
    }
    else
    {
        status = PH_OSAL_SUCCESS;
    }

    return status;
}

#endif /* PH_OSAL_LINUX */
