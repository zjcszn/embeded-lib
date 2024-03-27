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


#include "phDriver.h"
#include <board.h>

#include "BoardSelection.h"


#define LPC_TIMER_MAX_32BIT            0xFFFFFFFFU
#define LPC_TIMER_MAX_32BIT_W_PRE      0xFFFFFFFE00000000U /* max 32bit x 32bit. */


static pphDriver_TimerCallBck_t pTimerIsrCallBack;
static volatile uint32_t dwTimerExp;

static void phDriver_TimerIsrCallBack(void);

phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack)
{
    uint64_t qwTimerCnt;
    uint32_t dwPrescale;
    uint32_t dwTimerFreq;

    dwTimerFreq = Chip_Clock_GetPeripheralClockRate(PH_DRIVER_LPC_TIMER_CLK);

    /* Timer count = (delay * freq)/Units. */
    qwTimerCnt  = (dwTimerFreq / eTimerUnit);
    qwTimerCnt  *= dwTimePeriod;

    if(qwTimerCnt > (uint64_t)LPC_TIMER_MAX_32BIT_W_PRE)
    {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

    if(pTimerCallBack == NULL)
    {    /* Timer Start is blocking call. */
        dwTimerExp = 0;
        pTimerIsrCallBack = phDriver_TimerIsrCallBack;
    }else
    {   /* Call the Timer callback. */
        pTimerIsrCallBack = pTimerCallBack;
    }

    /* Initialize 16-bit timer 0 clock */
    Chip_TIMER_Init(PH_DRIVER_LPC_TIMER);
    /* Resets the timer terminal and prescale counts to 0 */
    Chip_TIMER_Reset(PH_DRIVER_LPC_TIMER);
    /* Enables match interrupt that fires when terminal count matches the match counter value */
    Chip_TIMER_MatchEnableInt(PH_DRIVER_LPC_TIMER, PH_DRIVER_LPC_TIMER_MATCH_REGISTER);

    /* 32-bit timers, check prescale is required. */
    if(qwTimerCnt > (uint64_t)LPC_TIMER_MAX_32BIT)
    {
        /* prescale is required. */
        for(dwPrescale=2; (qwTimerCnt/dwPrescale)>LPC_TIMER_MAX_32BIT; dwPrescale++);

        qwTimerCnt /= dwPrescale;

        /* Setup 16-bit prescale value to extend range */
        Chip_TIMER_PrescaleSet(PH_DRIVER_LPC_TIMER, dwPrescale);
    }

    /* Setup timer's duration (match time) */
    Chip_TIMER_SetMatch(PH_DRIVER_LPC_TIMER, PH_DRIVER_LPC_TIMER_MATCH_REGISTER, qwTimerCnt);
    Chip_TIMER_ResetOnMatchDisable(PH_DRIVER_LPC_TIMER, PH_DRIVER_LPC_TIMER_MATCH_REGISTER);
    Chip_TIMER_ClearMatch(PH_DRIVER_LPC_TIMER, PH_DRIVER_LPC_TIMER_MATCH_REGISTER);

    /* Start timer */
    Chip_TIMER_Enable(PH_DRIVER_LPC_TIMER);

    /* Set timer priority of the interrupt */
    NVIC_SetPriority(PH_DRIVER_LPC_TIMER_IRQ, PH_DRIVER_LPC_TIMER_IRQ_PRIORITY);

    /* Clear timer for any pending interrupt */
    NVIC_ClearPendingIRQ(PH_DRIVER_LPC_TIMER_IRQ);

    /* Enable timer interrupt */
    NVIC_EnableIRQ(PH_DRIVER_LPC_TIMER_IRQ);

    if(pTimerCallBack == NULL)
    {
        /* Block until timer expires. */
        while(!dwTimerExp);
    }

    return PH_DRIVER_SUCCESS;
}

phStatus_t phDriver_TimerStop(void)
{
    Chip_TIMER_Disable(PH_DRIVER_LPC_TIMER);
    Chip_TIMER_DeInit(PH_DRIVER_LPC_TIMER);

    /* Disable timer interrupt */
    NVIC_DisableIRQ(PH_DRIVER_LPC_TIMER_IRQ);

    return PH_DRIVER_SUCCESS;
}

static phStatus_t phDriver_PinConfigInterrupt(uint8_t bPortNum, uint8_t bPinNum, phDriver_Pin_Config_t *pPinConfig)
{
    uint32_t mode;

    /* Interrupt configs. */
    /* Clear any pending interrupts before enabling interrupt. */
    Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, (LPC_GPIOINT_PORT_T)bPortNum, (1<<bPinNum));

    /* All I/Os default to input with pullup after reset, configure the GPIO as input */
    mode = (pPinConfig->bPullSelect == PH_DRIVER_PULL_DOWN)?IOCON_MODE_PULLDOWN:IOCON_MODE_PULLUP;
    Chip_IOCON_PinMux(LPC_IOCON, bPortNum, bPinNum, mode, IOCON_FUNC0);
    Chip_GPIO_SetPinDIRInput(LPC_GPIO, bPortNum, bPinNum);

    switch(pPinConfig->eInterruptConfig)
    {
    case PH_DRIVER_INTERRUPT_RISINGEDGE:
        Chip_GPIOINT_SetIntRising(LPC_GPIOINT, bPortNum, (1<<bPinNum));
        break;

    case PH_DRIVER_INTERRUPT_FALLINGEDGE:
        Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, bPortNum, (1<<bPinNum));
        break;

    case PH_DRIVER_INTERRUPT_EITHEREDGE:
        Chip_GPIOINT_SetIntRising(LPC_GPIOINT, bPortNum, (1<<bPinNum));
        Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, bPortNum, (1<<bPinNum));
        break;
    default:
        /* Do Nothing. */
        break;
    }

    /* Any GPIO pin used for external interrupts will be using external interrupt channel 3 (EINT3) */
    NVIC_SetPriority(EINT3_IRQn, EINT_PRIORITY);
    NVIC_ClearPendingIRQ(EINT3_IRQn);
    NVIC_EnableIRQ(EINT3_IRQn);

    return PH_DRIVER_SUCCESS;
}

static phStatus_t phDriver_PinConfigGpio(uint8_t bPortNum, uint8_t bPinNum, phDriver_Pin_Func_t ePinFunc,
        phDriver_Pin_Config_t *pPinConfig)
{
    uint32_t mode;
    phStatus_t wStatus = PH_DRIVER_SUCCESS;

    mode = (pPinConfig->bPullSelect == PH_DRIVER_PULL_DOWN)?IOCON_MODE_PULLDOWN:IOCON_MODE_PULLUP;
    Chip_IOCON_PinMux(LPC_IOCON, bPortNum, bPinNum, mode, IOCON_FUNC0);

    switch(ePinFunc)
    {
    case PH_DRIVER_PINFUNC_INPUT:
        Chip_GPIO_SetPinDIRInput(LPC_GPIO, bPortNum, bPinNum);
        break;

    case PH_DRIVER_PINFUNC_OUTPUT:
        Chip_GPIO_SetPinDIROutput(LPC_GPIO, bPortNum, bPinNum);
        Chip_GPIO_SetPinState(LPC_GPIO, bPortNum, bPinNum, pPinConfig->bOutputLogic);
        break;

    default:
        wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

    return wStatus;
}

phStatus_t phDriver_PinConfig(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Pin_Config_t *pPinConfig)
{
    uint8_t bPortNum;
    uint8_t bPinNum;
    phStatus_t wStatus;

    bPortNum = (uint8_t)((dwPinNumber & 0x0000FF00) >> 8);
    bPinNum = (uint8_t)(dwPinNumber & 0x000000FF);

    do{
        if(pPinConfig == NULL)
        {
            wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
            break;
        }

        if(ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT)
        {
            /* Level triggered interrupts are NOT supported in LPC1769. */
            if((pPinConfig->eInterruptConfig == PH_DRIVER_INTERRUPT_LEVELZERO) ||
                    (pPinConfig->eInterruptConfig == PH_DRIVER_INTERRUPT_LEVELONE))
            {
                wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
            }
            else
            {
                wStatus = phDriver_PinConfigInterrupt(bPortNum, bPinNum, pPinConfig);
            }
        }
        else
        {
            wStatus = phDriver_PinConfigGpio(bPortNum, bPinNum, ePinFunc, pPinConfig);
        }
    }while(0);

    return wStatus;
}

uint8_t phDriver_PinRead(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc)
{
    uint32_t dwRisEdgeEnabled;
    uint32_t dwFallEdgeEnabled;
    uint8_t bPinStatus = 0;
    uint8_t bPortNum;
    uint8_t bPinNum;

    bPortNum = (uint8_t)((dwPinNumber & 0x0000FF00) >> 8);
    bPinNum = (uint8_t)(dwPinNumber & 0x000000FF);

    if(ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT)
    {
        /* Get the pins that has falling edge interrupt enabled. */
        dwFallEdgeEnabled = (Chip_GPIOINT_GetIntFalling(LPC_GPIOINT, bPortNum)) & (1<<bPinNum);
        if(dwFallEdgeEnabled)
        {
            /* Get status of the pins for falling edge */
            dwFallEdgeEnabled = Chip_GPIOINT_GetStatusFalling(LPC_GPIOINT, bPortNum);
            dwFallEdgeEnabled &= (1<<bPinNum);
        }

        /* Get pins that has rising edge interrupt enabled */
        dwRisEdgeEnabled = (Chip_GPIOINT_GetIntRising(LPC_GPIOINT, bPortNum)) & (1<<bPinNum);
        if(dwRisEdgeEnabled)
        {
            /* Get status of the pins for rising edge */
            dwRisEdgeEnabled = Chip_GPIOINT_GetStatusRising(LPC_GPIOINT, bPortNum);
            dwRisEdgeEnabled &= (1<<bPinNum);
        }

        /* Check whether Rising or Falling interrupt is set. */
        if(dwFallEdgeEnabled || dwRisEdgeEnabled)
        {
            bPinStatus = 1;
        }
    }
    else
    {
        bPinStatus = Chip_GPIO_GetPinState(LPC_GPIO, bPortNum, bPinNum);
    }

    return bPinStatus;
}

phStatus_t phDriver_IRQPinPoll(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Interrupt_Config_t eInterruptType)
{
    uint8_t    bGpioState = 0;

    if ((eInterruptType != PH_DRIVER_INTERRUPT_RISINGEDGE) && (eInterruptType != PH_DRIVER_INTERRUPT_FALLINGEDGE))
    {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

    if (eInterruptType == PH_DRIVER_INTERRUPT_FALLINGEDGE)
    {
        bGpioState = 1;
    }

	while(phDriver_PinRead(dwPinNumber, ePinFunc) == bGpioState);

    return PH_DRIVER_SUCCESS;
}

void phDriver_PinWrite(uint32_t dwPinNumber, uint8_t bValue)
{
    uint8_t bPortNum;
    uint8_t bPinNum;

    bPortNum = (uint8_t)((dwPinNumber & 0x0000FF00) >> 8);
    bPinNum = (uint8_t)(dwPinNumber & 0x000000FF);

    Chip_GPIO_SetPinState(LPC_GPIO, bPortNum, bPinNum, bValue);
}

void phDriver_PinClearIntStatus(uint32_t dwPinNumber)
{
    uint8_t bPortNum;
    uint8_t bPinNum;

    bPortNum = (uint8_t)((dwPinNumber & 0x0000FF00) >> 8);
    bPinNum = (uint8_t)(dwPinNumber & 0x000000FF);

    Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, bPortNum, (1<<bPinNum));
}

void PH_DRIVER_LPC_TIMER_IRQ_HANDLER(void)
{
    Chip_TIMER_ClearMatch(PH_DRIVER_LPC_TIMER, PH_DRIVER_LPC_TIMER_MATCH_REGISTER);

    pTimerIsrCallBack();

    Chip_TIMER_Disable(PH_DRIVER_LPC_TIMER);
    Chip_TIMER_DeInit(PH_DRIVER_LPC_TIMER);

    /* Disable timer interrupt */
    NVIC_DisableIRQ(PH_DRIVER_LPC_TIMER_IRQ);
}

static void phDriver_TimerIsrCallBack(void)
{
    dwTimerExp = 1;
}

void phDriver_EnterCriticalSection(void)
{
    __disable_irq();
}

void phDriver_ExitCriticalSection(void)
{
    __enable_irq();
}

phStatus_t phDriver_IRQPinRead(uint32_t dwPinNumber)
{
	phStatus_t bGpioVal = false;

	bGpioVal = phDriver_PinRead(dwPinNumber, PH_DRIVER_PINFUNC_INPUT);

	return bGpioVal;
}
