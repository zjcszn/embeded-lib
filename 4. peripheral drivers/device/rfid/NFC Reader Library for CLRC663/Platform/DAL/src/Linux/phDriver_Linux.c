/*----------------------------------------------------------------------------*/
/* Copyright 2017-2022 NXP                                                    */
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
* $Author$
* $Revision$
* $Date$
*
*/

#include <phDriver.h>
#include <BoardSelection.h>

#include "phDriver_Linux_Int.h"

#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static timer_t pPiTimer;
static pphDriver_TimerCallBck_t pxTimerCallBack;
static volatile bool bTimerExpired;

static void phDriver_Linux_TimerCallBack(union sigval uTimerSignal);

static void phDriver_Linux_TimerCallBack(union sigval uTimerSignal)
{
    if(pxTimerCallBack == NULL)
    {
        bTimerExpired = true;
    }else
    {
        pxTimerCallBack();
    }
}

phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack)
{
    struct sigevent sSigEvent;
    struct itimerspec xTimerVal = {{0, 0}, {0, 0}};
    phStatus_t wStatus = PH_DRIVER_SUCCESS;

    pxTimerCallBack = pTimerCallBack;
    bTimerExpired = false;

    memset(&sSigEvent, 0, sizeof(sSigEvent));
    sSigEvent.sigev_notify  = SIGEV_THREAD;
    sSigEvent.sigev_value.sival_int = 1;
    sSigEvent.sigev_notify_function = phDriver_Linux_TimerCallBack;
    sSigEvent.sigev_notify_attributes = NULL;

    if(eTimerUnit == PH_DRIVER_TIMER_SECS)
    {
        xTimerVal.it_value.tv_sec = dwTimePeriod;
    }
    else if(eTimerUnit == PH_DRIVER_TIMER_MILLI_SECS)
    {
        xTimerVal.it_value.tv_sec = (dwTimePeriod / 1000);
        xTimerVal.it_value.tv_nsec = (dwTimePeriod % 1000) * 1000 * 1000;
    }
    else
    {
        xTimerVal.it_value.tv_sec = (dwTimePeriod / 1000000);
        xTimerVal.it_value.tv_nsec = (dwTimePeriod % 1000000) * 1000;
    }

    do{
        if(timer_create(CLOCK_REALTIME, &sSigEvent, &pPiTimer) !=0)
        {
            wStatus = PH_DRIVER_FAILURE | PH_COMP_DRIVER;
            break;
        }

        if(timer_settime(pPiTimer, 0, &xTimerVal, NULL) !=0)
        {
            wStatus = PH_DRIVER_FAILURE | PH_COMP_DRIVER;
            break;
        }

        if(pTimerCallBack == NULL)
        {
            /* Wait until timer expires. */
            while(bTimerExpired == false);
        }
    }while(0);

    return wStatus;
}


phStatus_t phDriver_TimerStop(void)
{
    phStatus_t wStatus = PH_DRIVER_SUCCESS;

    /* Disarm the Timer. */
    struct itimerspec xTimerVal = {{0, 0}, {0, 0}};

    do{
        if(timer_settime(pPiTimer, 0, &xTimerVal, NULL) !=0)
        {
            wStatus = PH_DRIVER_FAILURE | PH_COMP_DRIVER;
            break;
        }

        if(timer_delete(pPiTimer) != 0)
        {
            wStatus = PH_DRIVER_FAILURE | PH_COMP_DRIVER;
        }
    }while(0);

    return wStatus;
}



phStatus_t phDriver_PinConfig(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Pin_Config_t *pPinConfig)
{
    phStatus_t wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
    bool output;

    do{
        if(ePinFunc == PH_DRIVER_PINFUNC_BIDIR)
        {
            break;
        }

        if(ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT)
        {
            if((pPinConfig->eInterruptConfig == PH_DRIVER_INTERRUPT_LEVELZERO)
                    || (pPinConfig->eInterruptConfig == PH_DRIVER_INTERRUPT_LEVELONE))
            {
                break;
            }
        }

        wStatus = PiGpio_export(dwPinNumber);
        if(wStatus != PH_DRIVER_SUCCESS)
        {
            break;
        }

        /* For interrupt/input direction is In. */
        output = (ePinFunc == PH_DRIVER_PINFUNC_OUTPUT)?true:false;
        wStatus = PiGpio_set_direction(dwPinNumber, output);
        if (wStatus != PH_DRIVER_SUCCESS)
        {
            PiGpio_unexport(dwPinNumber);
            break;
        }

        if(ePinFunc == PH_DRIVER_PINFUNC_OUTPUT)
        {
            /* Set the default output value. */
            wStatus = PiGpio_Write(dwPinNumber, pPinConfig->bOutputLogic);
        }
        else if(ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT)
        {
            switch(pPinConfig->eInterruptConfig)
            {
            case PH_DRIVER_INTERRUPT_RISINGEDGE:
                wStatus = PiGpio_set_edge(dwPinNumber, true, false);
                break;

            case PH_DRIVER_INTERRUPT_FALLINGEDGE:
                wStatus = PiGpio_set_edge(dwPinNumber, false, true);
                break;

            case PH_DRIVER_INTERRUPT_EITHEREDGE:
                wStatus = PiGpio_set_edge(dwPinNumber, true, true);
                break;
            default:
                /* Do Nothing. */
                break;
            }
        }

        if(wStatus != PH_DRIVER_SUCCESS)
        {
            PiGpio_unexport(dwPinNumber);
            break;
        }

    }while(0);

    return wStatus;
}

uint8_t phDriver_PinRead(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc)
{
    uint8_t bGpioVal = false;

    (void)PiGpio_read(dwPinNumber, &bGpioVal);

    return bGpioVal;
}

phStatus_t phDriver_IRQPinRead(uint32_t dwPinNumber)
{
    phStatus_t bGpioVal = false;

#ifdef NXPBUILD__PHHAL_HW_PN5190
    bGpioVal = PiGpio_IRQPin_Read();
#else
    bGpioVal = phDriver_PinRead(dwPinNumber, PH_DRIVER_PINFUNC_INPUT);
#endif /* NXPBUILD__PHHAL_HW_PN5190 */

    return bGpioVal;
}

phStatus_t phDriver_IRQPinPoll(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Interrupt_Config_t eInterruptType)
{
    phStatus_t wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;

    if ((eInterruptType != PH_DRIVER_INTERRUPT_RISINGEDGE) && (eInterruptType != PH_DRIVER_INTERRUPT_FALLINGEDGE))
    {
        return wStatus;
    }

#ifdef NXPBUILD__PHHAL_HW_PN5190
    /* Note : PN5190 IRQ Kernel Driver shall configure the Type of IRQ Edge.
     *        Currently PN5190 is tested with Rising Edge Interrupt configuration.
     */
    if (eInterruptType == PH_DRIVER_INTERRUPT_RISINGEDGE)
    {
        (void)PiGpio_Irq();
        wStatus = PH_DRIVER_SUCCESS;
    }
#else
    uint8_t bGpioState = 0;

    if (eInterruptType == PH_DRIVER_INTERRUPT_FALLINGEDGE)
    {
        bGpioState = 1;
    }

    while(phDriver_PinRead(dwPinNumber, ePinFunc) == bGpioState);
#endif /* NXPBUILD__PHHAL_HW_PN5190 */
    return wStatus;
}

void phDriver_PinWrite(uint32_t dwPinNumber, uint8_t bValue)
{
    (void)PiGpio_Write(dwPinNumber, bValue);
}


void phDriver_PinClearIntStatus(uint32_t dwPinNumber)
{
    return;
}

void phDriver_EnterCriticalSection(void)
{
#ifdef NXPBUILD__PHHAL_HW_PN5190
    /* Disable IRQ before entering into critical section */
    (void)PiGpio_Interrupt_Disable();
#endif /* NXPBUILD__PHHAL_HW_PN5190 */
}

void phDriver_ExitCriticalSection(void)
{
#ifdef NXPBUILD__PHHAL_HW_PN5190
    /* Enable IRQ before entering into critical section */
    (void)PiGpio_Interrupt_Enable();
#endif /* NXPBUILD__PHHAL_HW_PN5190 */
}
