/*
*********************************************************************************************************
*
*	模块名称 : 模拟串口FIFO驱动模块
*	文件名称 : bsp_emul_uart_fifo.c
*	版    本 : V1.0
*	说    明 : 采用GPIO+TIM+EXTI+DMA 实现的模拟串口的

*********************************************************************************************************
*/

#include "bsp.h"
#include "stm32h7xx_hal_uart_emul.h"
#include "bsp_emul_uart_fifio.h"

/*  Enable the clock for port EMUL_UART1 */
#define EMUL_UART1_CLK_ENABLE()				   __HAL_RCC_TIM1_CLK_ENABLE();\
											   __HAL_RCC_DMA2_CLK_ENABLE();
											   
#define EMUL_UART1_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOD_CLK_ENABLE()
#define EMUL_UART1_RX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOD_CLK_ENABLE()

/* Initialize GPIO and pin number for EMUL_UART1 */
#define EMUL_UART1_TX_PIN			GPIO_PIN_14
#define EMUL_UART1_TX_PORT			GPIOD
#define EMUL_UART1_RX_PIN			GPIO_PIN_15
#define EMUL_UART1_RX_PORT			GPIOD

/* Definition for EMUL_UART1  NVIC */
#define EMUL_UART1_IRQHandler	EXTI15_10_IRQHandler
#define EMUL_UART1_EXTI_IRQ		EXTI15_10_IRQn

/*  Enable the clock for port EMUL_UART2 */
#define EMUL_UART2_CLK_ENABLE()				   __HAL_RCC_TIM1_CLK_ENABLE();\
											   __HAL_RCC_DMA2_CLK_ENABLE();
											   
#define EMUL_UART2_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define EMUL_UART2_RX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()

/* Initialize GPIO and pin number for EMUL_UART2 */
#define EMUL_UART2_TX_PIN			GPIO_PIN_15
#define EMUL_UART2_TX_PORT			GPIOB
#define EMUL_UART2_RX_PIN			GPIO_PIN_14
#define EMUL_UART2_RX_PORT			GPIOB

/* Definition for EMUL_UART2  NVIC */
#define EMUL_UART2_EXTI_IRQHandler	EXTI15_10_IRQHandler
#define EMUL_UART2_EXTI_IRQ			EXTI15_10_IRQn

/* 定义每个串口结构体变量 */
#if EMUL_UART1_FIFO_EN == 1
	static EMUL_UART_T g_tEmulUart1;
	static uint8_t g_EmulTxBuf1[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_EmulRxBuf1[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

#if EMUL_UART2_FIFO_EN == 1
	static EMUL_UART_T g_tEmulUart2;
	static uint8_t g_EmulTxBuf2[UART1_TX_BUF_SIZE];		/* 发送缓冲区 */
	static uint8_t g_EmulRxBuf2[UART1_RX_BUF_SIZE];		/* 接收缓冲区 */
#endif

static void EmulUartVarInit(ECOM_PORT_E _ucPort);
static void InitHardEmulUart(ECOM_PORT_E _ucPort);

/*
*********************************************************************************************************
*	函 数 名: bsp_InitEmulUart
*	功能说明: 初始化模拟串口硬件，并对全局变量赋初值.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitEmulUart(ECOM_PORT_E _ucPort)
{	
	EmulUartVarInit(_ucPort);		/* 必须先初始化全局变量,再配置硬件 */
	InitHardEmulUart(_ucPort);		/* 配置串口的硬件参数(波特率等) */
}

/*
*********************************************************************************************************
*	函 数 名: EComToEmulUart
*	功能说明: 将ECOM端口号转换为EMUL_UART指针
*	形    参: _ucPort: 端口号(ECOM1 - ECOM2)
*	返 回 值: emul_uart指针
*********************************************************************************************************
*/
EMUL_UART_T *EComToEmulUart(ECOM_PORT_E _ucPort)
{
	if (_ucPort == ECOM1)
	{
		#if EMUL_UART1_FIFO_EN == 1
			return &g_tEmulUart1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == ECOM2)
	{
		#if EMUL_UART2_FIFO_EN == 1
			return &g_tEmulUart2;
		#else
			return 0;
		#endif
	}
	else
	{
		Error_Handler(__FILE__, __LINE__);
		return 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_SetUartParam
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32- H7开发板
*	形    参: Instance   USART_TypeDef类型结构体
*             BaudRate   波特率
*             Parity     校验类型，奇校验或者偶校验
*             Mode       发送和接收模式使能
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_SetEmulUartParam(UART_Emul_HandleTypeDef *pUartEmulHandle, uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
	/* 配置模拟串口硬件参数 */
	/*
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - BaudRate = 9600 baud
      - Parity = None
	*/
	pUartEmulHandle->Init.Mode        = Mode;
	pUartEmulHandle->Init.BaudRate    = BaudRate;	
	pUartEmulHandle->Init.Parity      = Parity;
	pUartEmulHandle->Init.StopBits    = UART_EMUL_STOPBITS_1;
	pUartEmulHandle->Init.WordLength  = UART_EMUL_WORDLENGTH_8B;

	if (HAL_UART_Emul_Init(pUartEmulHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}		
}

/*
*********************************************************************************************************
*	函 数 名: InitHardUart
*	功能说明: 配置串口的硬件参数（波特率，数据位，停止位，起始位，校验位，中断使能）适合于STM32-H7开发板
*	形    参: _ucPort: 端口号(ECOM1 - ECOM2)
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitHardEmulUart(ECOM_PORT_E _ucPort)
{
	GPIO_InitTypeDef   GPIO_InitStruct;	
	if(_ucPort == ECOM1)
	{
		/* Enable clock for UART Emul */
		EMUL_UART1_CLK_ENABLE();

		/* Enable GPIO TX/RX clock */
		EMUL_UART1_TX_GPIO_CLK_ENABLE();
		EMUL_UART1_RX_GPIO_CLK_ENABLE();

		/* Initialize UART Emulation port name */
		g_tEmulUart1.uartEmulHandle.TxPortName = EMUL_UART1_TX_PORT;
		g_tEmulUart1.uartEmulHandle.RxPortName = EMUL_UART1_RX_PORT;

		/*Initialize UART Emulation pin number for Tx */
		g_tEmulUart1.uartEmulHandle.Init.RxPinNumber = EMUL_UART1_RX_PIN;
		g_tEmulUart1.uartEmulHandle.Init.TxPinNumber = EMUL_UART1_TX_PIN;

		/* Configure GPIOE for UART Emulation Tx */
		GPIO_InitStruct.Pin    = EMUL_UART1_TX_PIN;
		GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull   = GPIO_NOPULL;
		GPIO_InitStruct.Speed  = GPIO_SPEED_HIGH;

		HAL_GPIO_Init(EMUL_UART1_TX_PORT, &GPIO_InitStruct);

		/* Configure GPIOC for UART Emulation Rx */
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pin  = EMUL_UART1_RX_PIN;

		HAL_GPIO_Init(EMUL_UART1_RX_PORT, &GPIO_InitStruct);

		bsp_SetEmulUartParam(&(g_tEmulUart1.uartEmulHandle), EMUL_UART1_BAUD,  UART_EMUL_PARITY_NONE, UART_EMUL_MODE_TX );
		/*##-2- Enable NVIC for line Rx  #################################*/
		/* Enable and set EXTI Line Interrupt to the highest priority */
		HAL_NVIC_SetPriority(EMUL_UART1_EXTI_IRQ, 0, 0);
		HAL_NVIC_EnableIRQ(EMUL_UART1_EXTI_IRQ);		
	}
	
	if(_ucPort == ECOM2)
	{
		/* Enable clock for UART Emul */
		EMUL_UART2_CLK_ENABLE();

		/* Enable GPIO TX/RX clock */
		EMUL_UART2_TX_GPIO_CLK_ENABLE();
		EMUL_UART2_RX_GPIO_CLK_ENABLE();

		/* Initialize UART Emulation port name */
		g_tEmulUart2.uartEmulHandle.TxPortName = EMUL_UART2_TX_PORT;
		g_tEmulUart2.uartEmulHandle.RxPortName = EMUL_UART2_RX_PORT;

		/*Initialize UART Emulation pin number for Tx */
		g_tEmulUart2.uartEmulHandle.Init.RxPinNumber = EMUL_UART2_RX_PIN;
		g_tEmulUart2.uartEmulHandle.Init.TxPinNumber = EMUL_UART2_TX_PIN;

		/* Configure GPIOE for UART Emulation Tx */
		GPIO_InitStruct.Pin    = EMUL_UART2_TX_PIN;
		GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull   = GPIO_NOPULL;
		GPIO_InitStruct.Speed  = GPIO_SPEED_HIGH;

		HAL_GPIO_Init(EMUL_UART2_TX_PORT, &GPIO_InitStruct);

		/* Configure GPIOC for UART Emulation Rx */
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pin  = EMUL_UART2_RX_PIN;

		HAL_GPIO_Init(EMUL_UART2_RX_PORT, &GPIO_InitStruct);

		bsp_SetEmulUartParam(&(g_tEmulUart2.uartEmulHandle), EMUL_UART2_BAUD,  UART_EMUL_PARITY_NONE, UART_EMUL_MODE_TX );
		/*##-2- Enable NVIC for line Rx  #################################*/
		/* Enable and set EXTI Line Interrupt to the highest priority */
		HAL_NVIC_SetPriority(EMUL_UART2_EXTI_IRQ, 0, 0);
		HAL_NVIC_EnableIRQ(EMUL_UART2_EXTI_IRQ);		
	}
}

/*
*********************************************************************************************************
*	函 数 名: EmulUartVarInit
*	功能说明: 初始化串口相关的变量
*	形    参: _ucPort: 端口号(ECOM1 - ECOM2)
*	返 回 值: 无
*********************************************************************************************************
*/
static void EmulUartVarInit(ECOM_PORT_E _ucPort)
{
	if(_ucPort == ECOM1)
	{
		// g_tEmulUart1.uart = USART1;						/* STM32 串口设备 */
		g_tEmulUart1.pTxBuf = g_EmulTxBuf1;				/* 发送缓冲区指针 */
		g_tEmulUart1.pRxBuf = g_EmulRxBuf1;				/* 接收缓冲区指针 */
		g_tEmulUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
		g_tEmulUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
		g_tEmulUart1.usTxWrite = 0;						/* 发送FIFO写索引 */
		g_tEmulUart1.usTxRead = 0;						/* 发送FIFO读索引 */
		g_tEmulUart1.usRxWrite = 0;						/* 接收FIFO写索引 */
		g_tEmulUart1.usRxRead = 0;						/* 接收FIFO读索引 */
		g_tEmulUart1.usRxCount = 0;						/* 接收到的新数据个数 */
		g_tEmulUart1.usTxCount = 0;						/* 待发送的数据个数 */
		g_tEmulUart1.SendBefor = 0;						/* 发送数据前的回调函数 */
		g_tEmulUart1.SendOver = 0;						/* 发送完毕后的回调函数 */
		g_tEmulUart1.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
		g_tEmulUart1.Sending = 0;						/* 正在发送中标志 */		
	}
	
	if(_ucPort == ECOM2)
	{
		// g_tEmulUart1.uart = USART1;						/* STM32 串口设备 */
		g_tEmulUart2.pTxBuf = g_EmulTxBuf2;				/* 发送缓冲区指针 */
		g_tEmulUart2.pRxBuf = g_EmulRxBuf2;				/* 接收缓冲区指针 */
		g_tEmulUart2.usTxBufSize = UART1_TX_BUF_SIZE;	/* 发送缓冲区大小 */
		g_tEmulUart2.usRxBufSize = UART1_RX_BUF_SIZE;	/* 接收缓冲区大小 */
		g_tEmulUart2.usTxWrite = 0;						/* 发送FIFO写索引 */
		g_tEmulUart2.usTxRead = 0;						/* 发送FIFO读索引 */
		g_tEmulUart2.usRxWrite = 0;						/* 接收FIFO写索引 */
		g_tEmulUart2.usRxRead = 0;						/* 接收FIFO读索引 */
		g_tEmulUart2.usRxCount = 0;						/* 接收到的新数据个数 */
		g_tEmulUart2.usTxCount = 0;						/* 待发送的数据个数 */
		g_tEmulUart2.SendBefor = 0;						/* 发送数据前的回调函数 */
		g_tEmulUart2.SendOver = 0;						/* 发送完毕后的回调函数 */
		g_tEmulUart2.ReciveNew = 0;						/* 接收到新数据后的回调函数 */
		g_tEmulUart2.Sending = 0;						/* 正在发送中标志 */			
	}	
}

void ecomSendBuf(ECOM_PORT_E _ucPort, uint8_t *pBuf, uint32_t bufLen)
{
	if(_ucPort == ECOM1)
	{
		HAL_UART_Emul_Transmit_DMA(&(g_tEmulUart1.uartEmulHandle), pBuf, bufLen);	
	}
	
	if(_ucPort == ECOM2)
	{
		HAL_UART_Emul_Transmit_DMA(&(g_tEmulUart2.uartEmulHandle), pBuf, bufLen);	
	}

}
