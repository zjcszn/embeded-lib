#ifndef __HAL_CONFIG_H_
#define __HAL_CONFIG_H_

#include "stm32f10x_rcc.h"

#define SPI_Arduino
//#define SPI_2  //W5100S EVB
//#define _WIZCHIP_IO_BASE_				0x60000003
#define PRINTF_USART			USART1
#define PRINTF_USART_RCC		RCC_APB2Periph_USART1
#define PRINTF_USART_TX_PIN		GPIO_Pin_9
#define PRINTF_USART_TX_PORT	GPIOA
#define PRINTF_USART_RX_PIN		GPIO_Pin_10
#define PRINTF_USART_RX_PORT	GPIOA

#define W5x00_GPIO_RCC			RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF
#define W5x00_RESET_PIN		GPIO_Pin_7
#define W5x00_RESET_PORT		GPIOC

#if defined(SPI_Arduino)
	#define W5x00_CS_PIN			GPIO_Pin_6
	#define W5x00_CS_PORT			GPIOB
#else
	#define W5x00_CS_PIN			GPIO_Pin_7
	#define W5x00_CS_PORT			GPIOD
#endif

#define W5x00_INT_PIN			GPIO_Pin_9
#define W5x00_INT_PORT			GPIOD

#if defined(SPI_Arduino)
/* SPI 1*/
#define W5x00_SPI				SPI1
#define W5x00_SPI_RCC			RCC_APB2Periph_SPI1
#define W5x00_SPI_PORT			GPIOA

#define W5x00_SPI_SCK_PIN		GPIO_Pin_5
#define W5x00_SPI_MOSI_PIN		GPIO_Pin_7
#define W5x00_SPI_MISO_PIN		GPIO_Pin_6

#else
/* SPI2 */
#define W5x00_SPI				SPI2
#define W5x00_SPI_RCC			RCC_APB1Periph_SPI2
#define W5x00_SPI_PORT			GPIOB

#define W5x00_SPI_SCK_PIN		GPIO_Pin_13
#define W5x00_SPI_MOSI_PIN		GPIO_Pin_15
#define W5x00_SPI_MISO_PIN		GPIO_Pin_14

#endif

#endif
