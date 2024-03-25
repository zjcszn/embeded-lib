/*
*********************************************************************************************************
*
*	模块名称 : 模拟串口FIFO驱动模块
*	文件名称 : bsp_emul_uart_fifo.h
*	版    本 : V1.0
*	说    明 : 采用GPIO+TIM+EXTI+DMA 实现的模拟串口的

*	Copyright (C), 2024-2030, ifree
*
*********************************************************************************************************
*/

#ifndef _BSP_EMUL_UART_FIFO_H_
#define _BSP_EMUL_UART_FIFO_H_

#define	EMUL_UART1_FIFO_EN		1
#define	EMUL_UART2_FIFO_EN		1

/* 定义端口号 */
typedef enum
{
	ECOM1 = 0,		/* EMUL_UART1 */
	ECOM2 = 1,		/* EMUL_UART2 */
}ECOM_PORT_E;

/* 定义串口波特率和FIFO缓冲区大小，分为发送缓冲区和接收缓冲区, 支持全双工 */
#if EMUL_UART1_FIFO_EN == 1
	#define EMUL_UART1_BAUD				115200
	#define EMUL_UART1_TX_BUF_SIZE		1*512
	#define EMUL_UART1_RX_BUF_SIZE		1*512
#endif

#if EMUL_UART2_FIFO_EN == 1
	#define EMUL_UART2_BAUD				9600
	#define EMUL_UART2_TX_BUF_SIZE		1*512
	#define EMUL_UART2_RX_BUF_SIZE		1*512
#endif

/* 串口设备结构体 */
typedef struct
{
	UART_Emul_HandleTypeDef 	uartEmulHandle;		/* STM32模拟串口设备指针 */
	uint8_t *pTxBuf;			/* 发送缓冲区 */
	uint8_t *pRxBuf;			/* 接收缓冲区 */
	uint16_t usTxBufSize;		/* 发送缓冲区大小 */
	uint16_t usRxBufSize;		/* 接收缓冲区大小 */
	__IO uint16_t usTxWrite;	/* 发送缓冲区写指针 */
	__IO uint16_t usTxRead;		/* 发送缓冲区读指针 */
	__IO uint16_t usTxCount;	/* 等待发送的数据个数 */

	__IO uint16_t usRxWrite;	/* 接收缓冲区写指针 */
	__IO uint16_t usRxRead;		/* 接收缓冲区读指针 */
	__IO uint16_t usRxCount;	/* 还未读取的新数据个数 */

	void (*SendBefor)(void); 	/* 开始发送之前的回调函数指针（主要用于RS485切换到发送模式） */
	void (*SendOver)(void); 	/* 发送完毕的回调函数指针（主要用于RS485将发送模式切换为接收模式） */
	void (*ReciveNew)(uint8_t _byte);	/* 串口收到数据的回调函数指针 */
	uint8_t Sending;			/* 正在发送中 */
}EMUL_UART_T;

void bsp_InitEmulUart(ECOM_PORT_E _ucPort);
void ecomSendBuf(ECOM_PORT_E _ucPort, uint8_t *pBuf, uint32_t bufLen);

#endif
