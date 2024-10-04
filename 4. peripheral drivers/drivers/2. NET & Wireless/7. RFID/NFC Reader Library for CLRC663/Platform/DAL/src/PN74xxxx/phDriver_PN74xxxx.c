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
* Generic phDriver Component of Reader Library Framework.
* $Author$
* $Revision$
* $Date$
*
* History:
*  RS: Generated 24. Jan 2017
*
*/


#include <phhalTimer.h>
#include "phDriver.h"


static pphDriver_TimerCallBck_t pTimerIsrCallBack;
static volatile uint32_t dwTimerExp;
static phhalTimer_Timers_t *pTimer = NULL;

static void phDriver_TimerIsrHandler(void);
static void phDriver_TimerIsrCallBack(void);

phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack)
{
    phStatus_t wStatus= PH_DRIVER_FAILURE | PH_COMP_DRIVER;
    phhalTimer_Units_t bUnits;

    if(eTimerUnit == PH_DRIVER_TIMER_SECS)
    {
        bUnits = E_TUNIT_SECS;
    }
    else
    {
        bUnits = (eTimerUnit == PH_DRIVER_TIMER_MILLI_SECS)? E_TUNIT_MILLI_SECS: E_TUNIT_MICRO_SECS ;
    }

    dwTimerExp = 0;
    pTimerIsrCallBack = (pTimerCallBack == NULL)? phDriver_TimerIsrCallBack : pTimerCallBack;

    do{
        if(phhalTimer_RequestTimer(bUnits, &pTimer) != PH_ERR_SUCCESS)
        {
            break;
        }

        if(phhalTimer_Configure(pTimer,  dwTimePeriod, phDriver_TimerIsrHandler) != PH_ERR_SUCCESS)
        {
            break;
        }

        if(phhalTimer_Start(pTimer, E_TIMER_SINGLE_SHOT) != PH_ERR_SUCCESS)
        {
            break;
        }

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
    phStatus_t wStatus= PH_DRIVER_FAILURE | PH_COMP_DRIVER;

    if(pTimer != NULL)
    {
        /* Release Timer will perform both Stop and Release the Timer. */
        if(phhalTimer_ReleaseTimer(pTimer) == PH_ERR_SUCCESS)
        {
            wStatus = PH_DRIVER_SUCCESS;
        }
        pTimer = NULL;
    }
    else
    {
        wStatus = PH_DRIVER_SUCCESS;
    }

    return wStatus;
}

static void phDriver_TimerIsrHandler(void)
{
    pTimerIsrCallBack();

    phhalTimer_Stop(pTimer);
    phhalTimer_ReleaseTimer(pTimer);
}

static void phDriver_TimerIsrCallBack(void)
{
    dwTimerExp = 1;
}
