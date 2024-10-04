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


#include "phApp_Init.h"
#include "BoardSelection.h"

#include "FreeRTOS.h"
#include "perf_counter.h"

#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_exti.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_utils.h"

phStatus_t phDriver_TimerStart(phDriver_Timer_Unit_t eTimerUnit, uint32_t dwTimePeriod, pphDriver_TimerCallBck_t pTimerCallBack) {
    switch (eTimerUnit) {
        case PH_DRIVER_TIMER_MICRO_SECS:
            delay_us((int32_t)dwTimePeriod);
            break;
        
        case PH_DRIVER_TIMER_MILLI_SECS:
            delay_ms((int32_t)dwTimePeriod);
            break;
        
        default:
            break;
        
    }
    
    if (pTimerCallBack) {
        pTimerCallBack();
    }
    
    return PH_DRIVER_SUCCESS;
}

phStatus_t phDriver_TimerStop(void) {
    return PH_DRIVER_SUCCESS;
}

static phStatus_t phDriver_PinConfigInterrupt(uint32_t dwPinNumber, phDriver_Pin_Config_t *pPinConfig) {
    GPIO_TypeDef *pxGpioPort;
    uint32_t dwGpioPin;
    uint32_t dwGpioPinNum, dwGpioPortNum;
    uint32_t dwSysCfgPort, dwSysCfgExtiLine;
    phStatus_t wStatus = PH_DRIVER_SUCCESS;
    
    dwGpioPinNum = dwPinNumber & 0x0000000F;
    dwGpioPortNum = (dwPinNumber & 0x00000F00) >> 8;
    
    dwGpioPin = 1UL << dwGpioPinNum;
    pxGpioPort = (GPIO_TypeDef *)(GPIOA_BASE + ((0x0400UL) * dwGpioPortNum));
    
    dwSysCfgPort = dwGpioPortNum;
    dwSysCfgExtiLine = (0x0000000F << (16 + (dwGpioPinNum & 3))) | (dwGpioPinNum >> 2);
    
    /* All I/Os default to input with pullup after reset, configure the GPIO as input */
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    
    GPIO_InitStruct.Pin = dwGpioPin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = (pPinConfig->bPullSelect == PH_DRIVER_PULL_DOWN) ? LL_GPIO_PULL_DOWN : LL_GPIO_PULL_UP;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    
    EXTI_InitStruct.Line_0_31 = dwGpioPin;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.LineCommand = ENABLE;
    
    switch (pPinConfig->eInterruptConfig) {
        case PH_DRIVER_INTERRUPT_RISINGEDGE:
            EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
            break;
        
        case PH_DRIVER_INTERRUPT_FALLINGEDGE:
            EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
            break;
        
        case PH_DRIVER_INTERRUPT_EITHEREDGE:
            EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
            break;
        
        default:
            wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
            break;
    }
    
    if (wStatus == PH_DRIVER_SUCCESS) {
        LL_AHB4_GRP1_EnableClock(1 << dwGpioPortNum);
        LL_GPIO_Init(pxGpioPort, &GPIO_InitStruct);
        
        LL_SYSCFG_SetEXTISource(dwSysCfgPort, dwSysCfgExtiLine);
        LL_EXTI_Init(&EXTI_InitStruct);
        
        NVIC_SetPriority(EINT_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), EINT_PRIORITY, 0));
        NVIC_EnableIRQ(EINT_IRQn);
    }
    
    return wStatus;
}

static phStatus_t phDriver_PinConfigGpio(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc,
                                         phDriver_Pin_Config_t *pPinConfig) {
    GPIO_TypeDef *pxGpioPort;
    uint32_t dwGpioPin;
    uint32_t dwGpioPortNum, dwGpioPinNUm;
    phStatus_t wStatus = PH_DRIVER_SUCCESS;
    
    dwGpioPinNUm = dwPinNumber & 0x0000000F;
    dwGpioPortNum = (dwPinNumber & 0x00000F00) >> 8;
    
    dwGpioPin = 1UL << dwGpioPinNUm;
    pxGpioPort = (GPIO_TypeDef *)(GPIOA_BASE + ((0x0400UL) * dwGpioPortNum));
    
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = dwGpioPin;
    GPIO_InitStruct.Pull = (pPinConfig->bPullSelect == PH_DRIVER_PULL_DOWN) ? LL_GPIO_PULL_DOWN : LL_GPIO_PULL_UP;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    
    switch (ePinFunc) {
        case PH_DRIVER_PINFUNC_INPUT:
            GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
            break;
        
        case PH_DRIVER_PINFUNC_OUTPUT:
            GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
            break;
        
        default:
            wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }
    
    if (wStatus == PH_DRIVER_SUCCESS) {
        LL_AHB4_GRP1_EnableClock(1 << dwGpioPortNum);
        LL_GPIO_Init(pxGpioPort, &GPIO_InitStruct);
        
        if (ePinFunc == PH_DRIVER_PINFUNC_OUTPUT) {
            pxGpioPort->BSRR = dwGpioPin << ((!pPinConfig->bOutputLogic) * 16);
        }
    }
    
    return wStatus;
}

phStatus_t phDriver_PinConfig(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Pin_Config_t *pPinConfig) {
    
    phStatus_t wStatus;
    
    do {
        if (pPinConfig == NULL) {
            wStatus = PH_DRIVER_ERROR | PH_COMP_DRIVER;
            break;
        }
        
        if (ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT) {
            wStatus = phDriver_PinConfigInterrupt(dwPinNumber, pPinConfig);
        } else {
            wStatus = phDriver_PinConfigGpio(dwPinNumber, ePinFunc, pPinConfig);
        }
    } while (0);
    
    return wStatus;
}

uint8_t phDriver_PinRead(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc) {
    GPIO_TypeDef *pxGpioPort;
    uint32_t dwGpioPin;
    uint32_t dwExtiLine;
    uint8_t bPinStatus = 0;
    
    dwGpioPin = 1UL << (dwPinNumber & 0x0000000F);
    pxGpioPort = (GPIO_TypeDef *)(GPIOA_BASE + ((0x0400UL) * ((dwPinNumber & 0x00000F00) >> 8)));
    dwExtiLine = dwGpioPin;
    
    if (ePinFunc == PH_DRIVER_PINFUNC_INTERRUPT) {
        if (LL_EXTI_IsEnabledFallingTrig_0_31(dwExtiLine) || LL_EXTI_IsEnabledRisingTrig_0_31(dwExtiLine)) {
            if (LL_EXTI_IsActiveFlag_0_31(dwExtiLine)) {
                bPinStatus = 1;
            }
        }
    } else {
        bPinStatus = (uint8_t)LL_GPIO_IsInputPinSet(pxGpioPort, dwGpioPin);
    }
    return bPinStatus;
}

phStatus_t phDriver_IRQPinPoll(uint32_t dwPinNumber, phDriver_Pin_Func_t ePinFunc, phDriver_Interrupt_Config_t eInterruptType) {
    uint8_t bGpioState = 0;
    
    if ((eInterruptType != PH_DRIVER_INTERRUPT_RISINGEDGE) && (eInterruptType != PH_DRIVER_INTERRUPT_FALLINGEDGE)) {
        return PH_DRIVER_ERROR | PH_COMP_DRIVER;
    }
    
    if (eInterruptType == PH_DRIVER_INTERRUPT_FALLINGEDGE) {
        bGpioState = 1;
    }
    
    while (phDriver_PinRead(dwPinNumber, ePinFunc) == bGpioState);
    
    return PH_DRIVER_SUCCESS;
}

void phDriver_PinWrite(uint32_t dwPinNumber, uint8_t bValue) {
    GPIO_TypeDef *pxGpioPort;
    uint32_t dwGpioPin;
    
    pxGpioPort = (GPIO_TypeDef *)(GPIOA_BASE + ((0x0400UL) * ((dwPinNumber & 0x00000F00) >> 8)));
    dwGpioPin = 1UL << (dwPinNumber & 0x0000000F);
    
    pxGpioPort->BSRR = dwGpioPin << ((!bValue) * 16);
}

void phDriver_PinClearIntStatus(uint32_t dwPinNumber) {
    uint32_t dwExtiLine;
    
    dwExtiLine = 1 << (dwPinNumber & 0x0000000F);
    LL_EXTI_ClearFlag_0_31(dwExtiLine);
}

void phDriver_EnterCriticalSection(void) {
    vPortEnterCritical();
}

void phDriver_ExitCriticalSection(void) {
    vPortExitCritical();
}

phStatus_t phDriver_IRQPinRead(uint32_t dwPinNumber) {
    phStatus_t bGpioVal = false;
    
    bGpioVal = phDriver_PinRead(dwPinNumber, PH_DRIVER_PINFUNC_INPUT);
    
    return bGpioVal;
}

void EINT_IRQHandler(void) {
    CLIF_IRQHandler();
}