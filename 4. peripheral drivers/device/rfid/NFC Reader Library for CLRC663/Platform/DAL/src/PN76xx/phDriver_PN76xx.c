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

/** \file
* Generic phDriver Component of Reader Library Framework.
* $Author $
* $Revision $
* $Date $
*
*/

	


#include <Timer.h>


#include "phDriver.h"

	

#define PN76XX_TIMER_MAX_DELAY    0xFFFFFU   /* 1048.575 milli sec */

static volatile pphDriver_TimerCallBck_t pDriverTimerCallBck = NULL;
static volatile uint32_t dwTimerExp;
static volatile uint64_t lwRemTimePeriod = 0;
static TIMER_ConfigDef_t tTimer;
static TIMER_ConfigDef_t *pTimer = NULL;

static void phDriver_TimerIsrHandler(void * params);
static void phDriver_TimerIsrCallBack(void);

phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack)
{
    phStatus_t wStatus= PH_DRIVER_FAILURE | PH_COMP_DRIVER;
    uint64_t lwTimePeriodTmp;

    lwRemTimePeriod = 0U;
    dwTimerExp = 0U;

    pTimer = &tTimer;
    pDriverTimerCallBck = pTimerCallBack;

    if (eTimerUnit == PH_DRIVER_TIMER_SECS)
    {
        lwTimePeriodTmp = dwTimePeriod * PH_DRIVER_TIMER_MICRO_SECS;
    }
    else if (eTimerUnit == PH_DRIVER_TIMER_MILLI_SECS)
    {
        lwTimePeriodTmp = dwTimePeriod * PH_DRIVER_TIMER_MILLI_SECS;
    }
    else if (eTimerUnit == PH_DRIVER_TIMER_MICRO_SECS)
    {
        lwTimePeriodTmp = dwTimePeriod;
    }
    else
    {
        return wStatus;
    }

    do{
        if (TIMER_Request(pTimer) != PN76_STATUS_SUCCESS)
        {
            break;
        }

        if (lwTimePeriodTmp > PN76XX_TIMER_MAX_DELAY)
        {
            dwTimePeriod = PN76XX_TIMER_MAX_DELAY;
        }
        else
        {
            dwTimePeriod = lwTimePeriodTmp;
        }
	

        if (TIMER_Configure(pTimer, E_TIMER_HAL_INTERRUPTMODE, dwTimePeriod, phDriver_TimerIsrHandler, NULL) != PN76_STATUS_SUCCESS)

        {
            break;
        }

        if(TIMER_Start(pTimer) != PN76_STATUS_SUCCESS)
        {
            break;
        }

        lwRemTimePeriod = lwTimePeriodTmp - dwTimePeriod;

        if(pTimerCallBack == NULL)
        {
            /* Block until timer expires. */
            while(!dwTimerExp);
        }

        wStatus = PH_DRIVER_SUCCESS;
    }while(0);

    return wStatus;
}

phStatus_t phDriver_TimerStop(void)
{
    phStatus_t wStatus = PH_DRIVER_FAILURE | PH_COMP_DRIVER;

    if(pTimer != NULL)
    {
        /* Stop Timer */
        if (TIMER_Stop(pTimer) != PN76_STATUS_SUCCESS)
        {
            return wStatus;
        }
        /* Release Timer */
        if (TIMER_Release(pTimer) != PN76_STATUS_SUCCESS)
        {
            return wStatus;
        }

        pTimer = NULL;
        wStatus = PH_DRIVER_SUCCESS;
    }
    else
    {
        wStatus = PH_DRIVER_SUCCESS;
    }

    return wStatus;
}

static void phDriver_TimerIsrHandler(void * params)
{
   uint32_t dwTimePeriod;
   uint8_t bReleaseTimer = 0U;

    if (lwRemTimePeriod)
    {
        TIMER_Stop(pTimer);
        if (lwRemTimePeriod > PN76XX_TIMER_MAX_DELAY)
        {
            lwRemTimePeriod -= PN76XX_TIMER_MAX_DELAY;
            dwTimePeriod = PN76XX_TIMER_MAX_DELAY;
        }
        else
        {
            dwTimePeriod = lwRemTimePeriod;
            lwRemTimePeriod = 0U;
        }
	

       if (TIMER_Configure(pTimer, E_TIMER_HAL_INTERRUPTMODE, dwTimePeriod, phDriver_TimerIsrHandler, NULL) != PN76_STATUS_SUCCESS)

        {
            /* In case of error, Stop and Release Timer without calling user CallBack */
            bReleaseTimer = 1U;
        }
        if(TIMER_Start(pTimer) != PN76_STATUS_SUCCESS)
        {
            /* In case of error, Stop and Release Timer without calling user CallBack */
            bReleaseTimer = 1U;
        }
    }
    else
    {
        if (pDriverTimerCallBck == NULL)
        {
            phDriver_TimerIsrCallBack();
        }
        else
        {
            pDriverTimerCallBck();
        }
        bReleaseTimer = 1U;
    }

    if(bReleaseTimer)
    {
        lwRemTimePeriod = 0U;
        TIMER_Stop(pTimer);
        TIMER_Release(pTimer);
    }
}

static void phDriver_TimerIsrCallBack(void)
{
    dwTimerExp = 1U;
}
