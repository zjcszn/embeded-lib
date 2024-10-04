#ifndef __DEVICE_CFG_H
#define __DEVICE_CFG_H

#include "DEFINE.h"
#include "stm32f10x_lib.h"

extern  GPIO_InitTypeDef   GPIO_InitStructure;
extern  USART_InitTypeDef  USART_InitStructure;
extern  UART_Com_Para_Def  UART_Com1_Para;
extern  UART_Com_Para_Def  UART_Com3_Para;
extern  NVIC_InitTypeDef   NVIC_InitStructure;

extern void UART_Com_Para_Init(UART_Com_Para_Def* UART_Com_Para);
extern void mDelay(unsigned int ms);
extern void uDelay(unsigned int us);
extern void EXTI15_10_IRQ_CTRL(FunctionalState sw);
extern void RCC_Config(void);
extern void GPIO_Config(void);
extern void EXTI_Config(void);
extern void USART1_Config(void);
extern void USART3_Config(void);
//extern void Timer_Init_TIM1(void);
//extern void Timer_Init_TIM2(void);
extern void SPI_Config(void);
//extern void SysTick_Init(void);

#endif


