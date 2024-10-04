/*
 *         Copyright (c), NXP Semiconductors Bangalore / India
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 */

/** \file
 * Winos OSAL Component of Reader Library Framework.
 * $Author$
 * $Revision$
 * $Date$
 *
 * History:
 *  PC: Generated 23. Aug 2012
 *
 */

#include <ph_Status.h>

#ifdef NXPBUILD__PH_OSAL_WINOS

#include <phOsal.h>
#include <phOsal_Config.h>
#include <windows.h>
#include <math.h>
#include <time.h>
#include "phOsal_WinOs.h"
#include <ph_TypeDefs.h>

#ifdef _CRT_UNUSED
#   define UNUSED(X) _CRT_UNUSED(X)
#else
#   define UNUSED(X)
#endif

typedef struct phOsalWinOsEventHandle
{
    pphOsal_EventObj_t pEventObj;
    uint32_t gdwEvents;
    phOsal_MutexObj_t MutexObj;
}phOsalWinOsEventHandle_t;

static volatile phOsalWinOsEventHandle_t  gphOsalWinOsEventHandles[PH_OSAL_CONFIG_WINOS_MAX_NUM_EVENTS];
static volatile uint32_t gdwEventBitMap;

static phStatus_t phOsal_NullOs_GetFreeIndex(uint32_t * dwFreeIndex, uint32_t dwBitMap, uint32_t dwMaxLimit);
static phStatus_t phOsal_NullOs_ReturnUnsupportedCmd(void);

static phStatus_t phOsal_Object_Take(HANDLE pObj, phOsal_TimerPeriodObj_t timePeriodToWait);

static volatile phOsalWinOsEventHandle_t * getEventStructure(HANDLE eventHandle)
{
    int i;
    for ( i = 0 ;i < sizeof(gphOsalWinOsEventHandles)/sizeof(gphOsalWinOsEventHandles[0]);i++){
        if ( NULL != gphOsalWinOsEventHandles[i].pEventObj &&
            eventHandle == gphOsalWinOsEventHandles[i].pEventObj->EventHandle)
        {
            return &gphOsalWinOsEventHandles[i];
        }
    }
    return NULL;
}

phStatus_t phOsal_Init(void)
{
    memset((void *)&gphOsalWinOsEventHandles[0], 0, sizeof(phOsalWinOsEventHandle_t));
    gdwEventBitMap = 0;

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_ThreadDelete(phOsal_Thread_t * threadHandle)
{
    UNUSED(threadHandle);
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadCreate(phOsal_Thread_t *threadHandle, pphOsal_ThreadObj_t threadObj,
    pphOsal_StartFunc_t startFunc, void *arg)
{
    UNUSED(threadHandle);
    UNUSED(threadObj);
    UNUSED(startFunc);
    UNUSED(arg);
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadChangePrio(phOsal_Thread_t * threadHandle, uint32_t newPrio)
{
    UNUSED(threadHandle);
    UNUSED(newPrio);
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadExit(void)
{
    return phOsal_NullOs_ReturnUnsupportedCmd();
}

phStatus_t phOsal_ThreadDelay(phOsal_Ticks_t ticksToSleep)
{
    Sleep(ticksToSleep);
    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_EventCreate(phOsal_Event_t *eventHandle, pphOsal_EventObj_t eventObj)
{
    uint32_t bEventIndex = 0;
    HANDLE OsalEventHandle;

    if ((eventHandle == NULL) || (eventObj == NULL))
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_OSAL);
    }

    PH_CHECK_SUCCESS(phOsal_NullOs_GetFreeIndex(&bEventIndex, gdwEventBitMap, PH_OSAL_CONFIG_WINOS_MAX_NUM_EVENTS));

    OsalEventHandle = CreateEvent(NULL, /* security attributes, optional */
        FALSE, /* auto reset is true */
        eventObj->intialValue, /* initial state is not signaled */
        (LPCSTR)eventObj->pEvtName /* name - optional */
    );
    if (OsalEventHandle == NULL)
    {
        return (PH_ERR_OSAL_ERROR | PH_COMP_OSAL);
    }

    gphOsalWinOsEventHandles[bEventIndex].pEventObj = eventObj;
    phOsal_MutexCreate((phOsal_Mutex_t *)(&gphOsalWinOsEventHandles[bEventIndex].MutexObj.MutexHandle),
        (pphOsal_MutexObj_t)(&gphOsalWinOsEventHandles[bEventIndex].MutexObj));
    gphOsalWinOsEventHandles[bEventIndex].gdwEvents = 0;
    gphOsalWinOsEventHandles[bEventIndex].pEventObj->dwEventIndex = bEventIndex;
    gphOsalWinOsEventHandles[bEventIndex].pEventObj->EventHandle = OsalEventHandle;

    gdwEventBitMap |= (1 << bEventIndex);

    eventObj->EventHandle = OsalEventHandle;
    *eventHandle = OsalEventHandle;

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_EventPend(volatile phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_Ticks_t ticksToWait,
    phOsal_EventBits_t FlagsToWait, phOsal_EventBits_t *pCurrFlags)
{
    DWORD dwWaitResult;
    phStatus_t statusTmp;
    phOsal_EventBits_t currentEvents;
    phStatus_t eStatus = PH_ERR_SUCCESS;
    clock_t start;
    clock_t end;
    uint32_t elapsedTime;
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phOsalWinOsEventHandle_t * ipEvHandle;

    if (eventHandle == NULL)
    {
        eStatus = (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);

        /* special case for mocking environment: In case init is not called, just return.
         * OSAL is not required for mocking framework.
         */
        if(pCurrFlags != NULL)
        {
            *pCurrFlags = /* TODO E_PH_OSAL_EVT_RF */ 1;
            eStatus = PH_ERR_SUCCESS;
        }
        return eStatus;
    }
    ipEvHandle = (phOsalWinOsEventHandle_t * )getEventStructure((HANDLE) *eventHandle);
    if (NULL == ipEvHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);

    do
    {
        timePeriodToWait.period = INFINITE;

        PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexLock(&(ipEvHandle->MutexObj.MutexHandle), timePeriodToWait));
        currentEvents = ipEvHandle->gdwEvents;
        PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexUnLock(&(ipEvHandle->MutexObj.MutexHandle)));

        if ((((options & E_OS_EVENT_OPT_PEND_SET_ALL)
            && (((ipEvHandle->gdwEvents) & FlagsToWait) == FlagsToWait))
            || ((!(options & E_OS_EVENT_OPT_PEND_SET_ALL)) && ((ipEvHandle->gdwEvents) & FlagsToWait))))
        {
            break;
        }

        start = clock();
        dwWaitResult = WaitForSingleObject((HANDLE) *eventHandle, ticksToWait);
        if (dwWaitResult != WAIT_OBJECT_0)
        {
            eStatus = (PH_ERR_IO_TIMEOUT | PH_COMP_OSAL);
            break;
        }
        /*Do something*/
        end = clock();
        elapsedTime = (uint32_t)(((float)(end - start) / CLOCKS_PER_SEC) * 1000);

        if (elapsedTime >= ticksToWait)
        {
            eStatus = (PH_ERR_IO_TIMEOUT | PH_COMP_OSAL);
            break;
        }
        else
        {
            ticksToWait -= elapsedTime;
        }

    } while (PH_ERR_SUCCESS == eStatus);

    if (pCurrFlags != NULL)
    {
        *pCurrFlags = ipEvHandle->gdwEvents;
    }

    if (options & E_OS_EVENT_OPT_PEND_CLEAR_ON_EXIT)
    {
        ipEvHandle->gdwEvents &=
            (~(FlagsToWait & (ipEvHandle->gdwEvents)));
    }

    return eStatus;
}

phStatus_t phOsal_EventPost(phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_EventBits_t FlagsToPost,
    phOsal_EventBits_t *pCurrFlags)
{
    phStatus_t statusTmp;
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phOsalWinOsEventHandle_t * ipEvHandle;
    UNUSED(pCurrFlags);
    UNUSED(options);

    if (NULL == eventHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);
    ipEvHandle = (phOsalWinOsEventHandle_t *)getEventStructure((HANDLE) *eventHandle);
    if (NULL == ipEvHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);

    timePeriodToWait.period = INFINITE;

    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexLock(&(ipEvHandle->MutexObj.MutexHandle),
        timePeriodToWait));
    (ipEvHandle)->gdwEvents |= FlagsToPost;
    (void) SetEvent(eventHandle);

    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexUnLock(&(ipEvHandle->MutexObj.MutexHandle)));

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_EventClear(phOsal_Event_t * eventHandle, phOsal_EventOpt_t options, phOsal_EventBits_t FlagsToClear,
    phOsal_EventBits_t *pCurrFlags)
{
    phStatus_t statusTmp;
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phOsalWinOsEventHandle_t * ipEvHandle;
    UNUSED(options);
    UNUSED(pCurrFlags);

    if (NULL == eventHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);
    ipEvHandle = (phOsalWinOsEventHandle_t *)getEventStructure((HANDLE) *eventHandle);
    if (NULL == ipEvHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);

    timePeriodToWait.period = INFINITE;

    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexLock(&(ipEvHandle->MutexObj.MutexHandle),
        timePeriodToWait));
    ipEvHandle->gdwEvents &= ~FlagsToClear;
    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexUnLock(&(ipEvHandle->MutexObj.MutexHandle)));

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_EventGet(phOsal_Event_t *  eventHandle, phOsal_EventBits_t *pCurrFlags)
{
    phStatus_t statusTmp;
    phOsal_TimerPeriodObj_t timePeriodToWait;
    phOsalWinOsEventHandle_t * ipEvHandle;

    if (NULL == eventHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);
    ipEvHandle = (phOsalWinOsEventHandle_t *)getEventStructure((HANDLE) *eventHandle);
    if (NULL == ipEvHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);


    timePeriodToWait.period = INFINITE;

    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexLock(&(ipEvHandle->MutexObj.MutexHandle), timePeriodToWait));
    *pCurrFlags = ipEvHandle->gdwEvents;
    PH_CHECK_SUCCESS_FCT(statusTmp, phOsal_MutexUnLock(&(ipEvHandle->MutexObj.MutexHandle)));

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_EventDelete(phOsal_Event_t *  eventHandle)
{
    phOsalWinOsEventHandle_t * ipEvHandle;

    if (NULL == eventHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);
    ipEvHandle = (phOsalWinOsEventHandle_t *)getEventStructure((HANDLE) *eventHandle);
    if (NULL == ipEvHandle)
        return (PH_ERR_SUCCESS | PH_COMP_OSAL);

    phOsal_MutexDelete(&(ipEvHandle->MutexObj.MutexHandle));
    gdwEventBitMap &= (~(1 << ipEvHandle->pEventObj->dwEventIndex));
    //memset(ipEvHandle, 0, sizeof(eventHandle));
    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_SemCreate(phOsal_Semaphore_t *semHandle, pphOsal_SemObj_t semObj, phOsal_SemOpt_t opt)
{
    /* TODO : when binary semaphore update the max count here */
    *semHandle = CreateSemaphore(
        NULL,           /* default security attributes */
        semObj->semInitialCount,  /* initial count */
        semObj->semMaxCount,  /* maximum count */
        (LPCSTR)semObj->pSemName /* no name */
    );

    if (*semHandle == NULL)
    {
        return (PH_ERR_OSAL_ERROR | PH_COMP_OSAL);
    }

    semObj->SemHandle = *semHandle;

    UNUSED(opt);

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_SemPend(phOsal_Semaphore_t * semHandle, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    return phOsal_Object_Take((HANDLE) *semHandle, timePeriodToWait);
}

phStatus_t phOsal_SemPost(phOsal_Semaphore_t * semHandle, phOsal_SemOpt_t opt)
{
    UNUSED(opt);
    if (!ReleaseSemaphore((HANDLE) semHandle, 1, NULL))
    {
        return (PH_ERR_OSAL_ERROR | PH_COMP_OSAL);
    }
    else
    {
        return PH_ERR_SUCCESS;
    }
}

phStatus_t phOsal_SemDelete(phOsal_Semaphore_t * semHandle)
{
    if (semHandle == NULL)
    {
        return PH_ADD_COMPCODE(PH_ERR_INVALID_PARAMETER, PH_COMP_OSAL);
    }

    CloseHandle((HANDLE) *semHandle);

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_MutexCreate(phOsal_Mutex_t *mutexHandle, pphOsal_MutexObj_t mutexObj)
{
    if ((mutexHandle == NULL) || (mutexObj == NULL))
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }
    *mutexHandle = CreateMutex(NULL, /* security attributes, optional */
        FALSE, /* initial owner set to false */
        (LPCSTR) mutexObj->pMutexName /* name - optional */
    );
    if (*mutexHandle == NULL)
    {
        return (PH_ERR_OSAL_ERROR | PH_COMP_OSAL);
    }

    mutexObj->MutexHandle = *mutexHandle;

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_MutexLock(phOsal_Mutex_t * mutexHandle, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    return phOsal_Object_Take((HANDLE)*mutexHandle, timePeriodToWait);
}

phStatus_t phOsal_MutexUnLock(phOsal_Mutex_t * mutexHandle)
{
    if (mutexHandle == NULL)
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    if (!ReleaseMutex((HANDLE)*mutexHandle))
    {
        return (PH_ERR_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_MutexDelete(phOsal_Mutex_t * mutexHandle)
{
    if (mutexHandle == NULL)
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }
    CloseHandle((HANDLE)*mutexHandle);
    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_TimerCreate(phOsal_Timer_t *timerHandle, pphOsal_TimerObj_t timerObj)
{
    if ((timerHandle == NULL) || (timerObj == NULL))
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    *timerHandle = CreateWaitableTimer(
        NULL, /* default security attributes */
        timerObj->opt, /* no auto reset */
        (LPCSTR)timerObj->pTimerName /* name */
    );

    timerObj->TimerHandle = *timerHandle;

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_TimerStart(phOsal_Timer_t * timerHandle)
{
    phOsal_TimerObj_t timerObj;
    uint32_t dwUnitCov[] = {(1000 * 10), (1000 * 1000 * 10)};
    LARGE_INTEGER DueTime;
    uint64_t llDueTime;

    timerObj = *((pphOsal_TimerObj_t)timerHandle);

    if ((timerHandle == NULL) || (timerObj.timePeriod.unitPeriod > OS_TIMER_UNIT_SEC)
        || (timerObj.timePeriod.unitPeriod > OS_TIMER_UNIT_MSEC))
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    llDueTime = -1 * (timerObj.timePeriod.period * dwUnitCov[timerObj.timePeriod.unitPeriod]); /* DueTime is given in 100ns. negative value means wait relative time */
    /* TODO : llDueTime = 0xFFFFFFFFFFFFFFFF * (timerObj.timePeriod.period * dwUnitCov[timerObj.timePeriod.unitPeriod]); */
    DueTime.LowPart = (DWORD)(llDueTime & 0xFFFFFFFF);
    DueTime.HighPart = (LONG)(llDueTime >> 32);

    if (!SetWaitableTimer(timerHandle, /* Handle to the timer object */
        &DueTime, /* When timer will become signaled */
        timerObj.opt ? (uint32_t)(timerObj.timePeriod.period) : 0, /* one shot timer */
            (PTIMERAPCROUTINE)((pphOsal_TimerObj_t)timerHandle)->timerCb, /* Completion routine */
            ((pphOsal_TimerObj_t)timerHandle)->arg, /* Argument to the completion routine */
            FALSE))
    {
        return (PH_ERR_OSAL_FAILURE | PH_COMP_OSAL);
    }

    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_TimerStop(phOsal_Timer_t * timerHandle)
{
    if (timerHandle == NULL)
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    if (!CancelWaitableTimer(timerHandle))
    {
        return (PH_ERR_OSAL_FAILURE | PH_COMP_OSAL);
    }
    return PH_ERR_SUCCESS;
}

phStatus_t phOsal_TimerGetCurrent(phOsal_Timer_t * timerHandle, uint32_t * dwGetElapsedTime);

phStatus_t phOsal_TimerModify(phOsal_Timer_t * timerHandle, pphOsal_TimerObj_t timerObj)
{
    if ((timerHandle == NULL) || (timerObj == NULL))
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    return phOsal_TimerStart(&timerObj->TimerHandle);

}

phStatus_t phOsal_TimerDelete(phOsal_Timer_t * timerHandle)
{
    if (timerHandle == NULL)
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    memset(timerHandle, 0, sizeof(phOsal_TimerObj_t));

    return PH_ERR_SUCCESS;
}

void phOsal_StartScheduler(void)
{
    phOsal_NullOs_ReturnUnsupportedCmd();
}

static phStatus_t phOsal_NullOs_ReturnUnsupportedCmd(void)
{
    return (PH_ERR_UNSUPPORTED_COMMAND | PH_COMP_OSAL);
}

static phStatus_t phOsal_Object_Take(HANDLE pObj, phOsal_TimerPeriodObj_t timePeriodToWait)
{
    uint32_t dwMilliseconds;
    uint32_t dwMsCov[] = {1, 1000};

    if (pObj == NULL)
    {
        return (PH_ERR_INVALID_PARAMETER | PH_COMP_OSAL);
    }

    if (timePeriodToWait.period != INFINITE)
    {
        dwMilliseconds = timePeriodToWait.period * dwMsCov[timePeriodToWait.unitPeriod];
    }
    else
    {
        dwMilliseconds = timePeriodToWait.period;
    }

    if (WaitForSingleObject(pObj, dwMilliseconds) != 0)
    {
        return (PH_ERR_IO_TIMEOUT | PH_COMP_OSAL);
    }

    return PH_ERR_SUCCESS;
}

static phStatus_t phOsal_NullOs_GetFreeIndex(uint32_t * dwFreeIndex, uint32_t dwBitMap, uint32_t dwMaxLimit)
{
    phStatus_t status;

    (*dwFreeIndex) = 0;

    while(((1 << (*dwFreeIndex)) & dwBitMap) && ((*dwFreeIndex) < dwMaxLimit))
    {
        (*dwFreeIndex)++;
    }

    if (*dwFreeIndex == dwMaxLimit)
    {
        status = (PH_ERR_RESOURCE_ERROR | PH_COMP_OSAL);
    }
    else
    {
        status = PH_ERR_SUCCESS;
    }

    return status;
}
#endif /* NXPBUILD__PH_OSAL_WINOS */
