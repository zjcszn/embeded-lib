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


/* *****************************************************************************************************************
 * Includes
 * ***************************************************************************************************************** */
#include "phDriver.h"
#include "BoardSelection.h"
#include "fsl_device_registers.h"
#include <fsl_port.h>
#include <fsl_gpio.h>
#include <fsl_pit.h>

/* *****************************************************************************************************************
 * Internal Definitions
 * ***************************************************************************************************************** */
#define KINETIS_TIMER_MAX_32BIT      0xFFFFFFFFU

/* *****************************************************************************************************************
 * Type Definitions
 * ***************************************************************************************************************** */

/* *****************************************************************************************************************
 * Global and Static Variables
 * Total Size: NNNbytes
 * ***************************************************************************************************************** */
/* Array initializer of PORT peripheral base pointers */
static const PORT_Type *pPortsBaseAddr[] = PORT_BASE_PTRS;
/* Array initializer of GPIO peripheral base pointers */
static const GPIO_Type *pGpiosBaseAddr[] = GPIO_BASE_PTRS;
/* Clock ip name array for PORT. */
static const clock_ip_name_t pPortsClock[] = PORT_CLOCKS;
/* Used to map phDriver Interrupt triggers to Kinetis */
static const port_interrupt_t aInterruptTypes[] = {kPORT_InterruptLogicZero, /* Unused. */
        kPORT_InterruptLogicZero,
        kPORT_InterruptLogicOne,
        kPORT_InterruptRisingEdge,
        kPORT_InterruptFallingEdge,
        kPORT_InterruptEitherEdge,
};

static pphDriver_TimerCallBck_t pPitTimerCallBack;
static volatile uint8_t dwTimerExp;

/* *****************************************************************************************************************
 * Private Functions Prototypes
 * ***************************************************************************************************************** */
static void phDriver_PitTimerIsrCallBack(void);

/* *****************************************************************************************************************
 * Public Functions
 * ***************************************************************************************************************** */
phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack)
{
    uint64_t          qwTimerCnt;
    uint32_t          dwTimerFreq;

    dwTimerFreq = CLOCK_GetFreq(PH_DRIVER_KSDK_PIT_CLK);

    /* Timer count = (delay * freq)/Units. */
    qwTimerCnt = dwTimerFreq;
    qwTimerCnt = (qwTimerCnt / eTimerUnit);
    qwTimerCnt = (dwTimePeriod * qwTimerCnt);

    /* 32-bit timers. */
    if(qwTimerCnt > (uint64_t)KINETIS_TIMER_MAX_32BIT)
    {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

    if(pTimerCallBack == NULL)
    {    /* Timer Start is blocking call. */
        dwTimerExp = 0;
        pPitTimerCallBack = phDriver_PitTimerIsrCallBack;
    }
    else
    {   /* Call the Timer callback. */
        pPitTimerCallBack = pTimerCallBack;
    }

    PIT_SetTimerChainMode(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, false);
    PIT_StopTimer(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL);
    PIT_ClearStatusFlags(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, kPIT_TimerFlag);
    PIT_EnableInterrupts(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, kPIT_TimerInterruptEnable);

    /* Configure timer period */
    PIT_SetTimerPeriod(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, (uint32_t)qwTimerCnt);

    /* Set timer priority of the interrupt */
    NVIC_SetPriority(PH_DRIVER_KSDK_TIMER_NVIC, PH_DRIVER_KSDK_TIMER_PRIORITY);

    /* Clear timer for any pending interrupt */
    NVIC_ClearPendingIRQ(PH_DRIVER_KSDK_TIMER_NVIC);

    /* Enable at the NVIC & start timer */
    EnableIRQ(PH_DRIVER_KSDK_TIMER_NVIC);
    PIT_StartTimer(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL);

    if(pTimerCallBack == NULL)
    {
        /* Block until timer expires. */
        while(!dwTimerExp);
    }

    return PH_DRIVER_SUCCESS;
}


phStatus_t phDriver_TimerStop(void)
{
    PIT_StopTimer(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL);
    PIT_DisableInterrupts(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, kPIT_TimerInterruptEnable);

    /* Disable at the NVIC */
    DisableIRQ(PH_DRIVER_KSDK_TIMER_NVIC);

    return PH_DRIVER_SUCCESS;
}


phStatus_t phDriver_PinConfig(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Pin_Config_t *pPinConfig)
{
    gpio_pin_config_t sGpioConfig;
    uint8_t bPinNum;
    uint8_t bPortGpio;

    port_interrupt_t eInterruptType;
    port_pin_config_t sPinConfig =
    {
        .pullSelect = kPORT_PullDisable,
        .slewRate = kPORT_FastSlewRate,
        .passiveFilterEnable = kPORT_PassiveFilterDisable,
        .openDrainEnable = kPORT_OpenDrainDisable,
        .driveStrength = kPORT_HighDriveStrength,
        .mux = kPORT_MuxAsGpio,
        .lockRegister = kPORT_UnlockRegister
    };

    if((ePinFunc == PH_DRIVER_PINFUNC_BIDIR) || (pPinConfig == NULL))
    {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }

    /* Extract the Pin, Gpio, Port details from dwPinNumber */
    bPinNum = (uint8_t)(dwPinNumber & 0xFF);
    bPortGpio = (uint8_t)((dwPinNumber & 0xFF00)>>8);

    sGpioConfig.pinDirection = (ePinFunc == PH_DRIVER_PINFUNC_OUTPUT)?kGPIO_DigitalOutput:kGPIO_DigitalInput;
    sGpioConfig.outputLogic  =  pPinConfig->bOutputLogic;
    sPinConfig.pullSelect = (pPinConfig->bPullSelect == PH_DRIVER_PULL_DOWN)? kPORT_PullDown : kPORT_PullUp;

    CLOCK_EnableClock(pPortsClock[bPortGpio]);
    PORT_SetPinConfig((PORT_Type *)pPortsBaseAddr[bPortGpio], bPinNum, &sPinConfig);

    if(ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT)
    {
        eInterruptType = aInterruptTypes[(uint8_t)pPinConfig->eInterruptConfig];
        GPIO_PortClearInterruptFlags((GPIO_Type *)pGpiosBaseAddr[bPortGpio], bPinNum);
        PORT_SetPinInterruptConfig((PORT_Type *)pPortsBaseAddr[bPortGpio], bPinNum, eInterruptType);
    }

    GPIO_PinInit((GPIO_Type *)pGpiosBaseAddr[bPortGpio], bPinNum, &sGpioConfig);

    return PH_DRIVER_SUCCESS;
}


uint8_t phDriver_PinRead(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc)
{
    uint8_t bValue;
    uint8_t bGpioNum;
    uint8_t bPinNum;

    /* Extract the Pin, Gpio details from dwPinNumber */
    bPinNum = (uint8_t)(dwPinNumber & 0xFF);
    bGpioNum = (uint8_t)((dwPinNumber & 0xFF00)>>8);

    if(ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT)
    {
        bValue = (uint8_t)((GPIO_PortGetInterruptFlags((GPIO_Type *)pGpiosBaseAddr[bGpioNum]) >> bPinNum) & 0x01);
    }
    else
    {
        bValue = (uint8_t)GPIO_PinRead((GPIO_Type *)pGpiosBaseAddr[bGpioNum], bPinNum);
    }

    return bValue;
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
    uint8_t bGpioNum;
    uint8_t bPinNum;

    /* Extract the Pin, Gpio details from dwPinNumber */
    bPinNum = (uint8_t)(dwPinNumber & 0xFF);
    bGpioNum = (uint8_t)((dwPinNumber & 0xFF00)>>8);

    GPIO_PinWrite((GPIO_Type *)pGpiosBaseAddr[bGpioNum], bPinNum, bValue);
}


void phDriver_PinClearIntStatus(uint32_t dwPinNumber)
{
    uint8_t bGpioNum;
    uint8_t bPinNum;

    /* Extract the Pin, Gpio details from dwPinNumber */
    bPinNum = (uint8_t)(dwPinNumber & 0xFF);
    bGpioNum = (uint8_t)((dwPinNumber & 0xFF00)>>8);

    GPIO_PortClearInterruptFlags((GPIO_Type *)pGpiosBaseAddr[bGpioNum], (1<<bPinNum));
}

phStatus_t phDriver_IRQPinRead(uint32_t dwPinNumber)
{
	phStatus_t bGpioVal = false;

	bGpioVal = phDriver_PinRead(dwPinNumber, PH_DRIVER_PINFUNC_INPUT);

	return bGpioVal;
}

void PIT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    PIT_ClearStatusFlags(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, kPIT_TimerFlag);

    /* Single shot timer. Stop it. */
    PIT_StopTimer(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL);
    PIT_DisableInterrupts(PH_DRIVER_KSDK_PIT_TIMER, PH_DRIVER_KSDK_TIMER_CHANNEL, kPIT_TimerInterruptEnable);

    pPitTimerCallBack();

}

static void phDriver_PitTimerIsrCallBack(void)
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
