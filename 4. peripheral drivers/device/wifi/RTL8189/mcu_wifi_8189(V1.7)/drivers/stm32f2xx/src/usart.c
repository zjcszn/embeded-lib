//#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"


#define UART1_USE_DMA	1

UART_RECV 	u1_recv;

UART_SEND 	u3_send;

UART_SEND 	u1_send;

wait_event_t uart1_rx_waitq = 0;

volatile unsigned int uart1_cnt = 0,uart1_cnt_ex = 0;

#if UART1_USE_DMA			  
static void DMA_TxConfiguration(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART1, USART_IT_TC, DISABLE);  //关闭发送完成和发送空中断
	
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_HTIF7);
	
	/* DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Stream7, DISABLE);
	DMA_StructInit(&DMA_InitStructure);
	/* DMA2 Channel4 Config */
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	//DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	//DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	//DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	//DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);

	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC | DMA_IT_TE,ENABLE);  //使能传输完成和出错中断
	/* DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Stream7, ENABLE);
	
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
	
	NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void DMA_RxConfiguration(uint32_t *BufferDST, uint32_t BufferSize)
{

	DMA_InitTypeDef DMA_InitStructure;

	DMA_ITConfig(DMA2_Stream5, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_HTIF5);
   
	/* DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Stream5, DISABLE);

	DMA_StructInit(&DMA_InitStructure);
	/* DMA2 Channel4 Config */
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BufferSize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA2_Stream5, &DMA_InitStructure);
	u1_recv.pos = 0;   //当前读取位置清零
	/* DMA2 Channel4 enable */
	DMA_ITConfig(DMA2_Stream5, DMA_IT_TE,ENABLE); //使能出错中断
	DMA_Cmd(DMA2_Stream5, ENABLE);
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}
#endif
/*
 * 函数名：USART1_Config
 * 描述  ：USART1 GPIO 配置,工作模式配置。115200 8-N-1
 * 输入  ：无
 * 输出  : 无
 * 调用  ：外部调用
 */
void uart1_init(void)
{
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	memset(&u1_recv,0,sizeof(UART_RECV));
	
	u1_recv.pkg.pending.val = 1;  //防止用户输入
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);

	gpio_cfg((uint32_t)UART1_TX_PORT_GROUP, UART1_TX_PIN, GPIO_Mode_AF_PP);
	gpio_cfg((uint32_t)UART1_RX_PORT_GROUP, UART1_RX_PIN, GPIO_Mode_AF_IF);
	GPIO_PinAFConfig(UART1_TX_PORT_GROUP, UART1_TX_PIN_SOURSE, GPIO_AF_USART1);
	GPIO_PinAFConfig(UART1_RX_PORT_GROUP, UART1_RX_PIN_SOURSE, GPIO_AF_USART1);

	
	u1_send.wait = init_event();
	if(u1_send.wait  == 0)
	{
		p_err("uart_init sys_sem_new1 err\n");
		return;
	}

	USART_InitStructure.USART_BaudRate = UART1_DEFAULT_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
	USART_Cmd(USART1, ENABLE);
	//USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#if UART1_USE_DMA
	DMA_RxConfiguration((uint32_t *)u1_recv.c_buff, UART_RECV_BUFF_SIZE);  //循环缓冲区
#endif	
	USART_ClearITPendingBit(USART1, USART_IT_TC);
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	NVIC_EnableIRQ(USART1_IRQn);

#if UART1_USE_DMA
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA2_Stream5_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA2_Stream7_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}


void uart3_init(void)
{
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	memset(&u3_send,0,sizeof(UART_SEND));

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	gpio_cfg((uint32_t)UART3_TX_PORT_GROUP, UART3_TX_PIN, GPIO_Mode_AF_PP);
	gpio_cfg((uint32_t)UART3_RX_PORT_GROUP, UART3_RX_PIN, GPIO_Mode_AF_IF);
	GPIO_PinAFConfig(UART3_TX_PORT_GROUP, UART3_TX_PIN_SOURSE, GPIO_AF_USART3);
	GPIO_PinAFConfig(UART3_RX_PORT_GROUP, UART3_RX_PIN_SOURSE, GPIO_AF_USART3);

	u3_send.wait = init_event();
	if(u3_send.wait == 0)
	{
		p_err("uart_init sys_sem_new2 err\n");
		return;
	}

	USART_InitStructure.USART_BaudRate = UART3_DEFAULT_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure); 
   	USART_Cmd(USART3, ENABLE);

	USART_ITConfig(USART3, USART_IT_TC, ENABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
//	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART3_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	USART_ClearITPendingBit(USART3, USART_IT_TC);
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	
	NVIC_EnableIRQ(USART3_IRQn);

}

#if UART1_USE_DMA
void DMA2_Stream7_IRQHandler()  //发送中断
{	
	uint32_t irq_flag;
	enter_interrupt();

	DMA_Cmd(DMA2_Stream7, DISABLE);
	NVIC_DisableIRQ(DMA2_Stream7_IRQn);
	irq_flag = local_irq_save(); 

	if(DMA_GetFlagStatus(DMA2_Stream7, DMA_FLAG_TEIF7))
	{
		
		p_err("uart dma err");
	}

	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_HTIF7);
   
	u1_send.cnt = DMA2_Stream7->NDTR;   //取剩余字数,应该为0
	local_irq_restore(irq_flag);
	USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
	p_dbg("u_send irq\n");
	wake_up(u1_send.wait);   //唤醒等待线程
	
	exit_interrupt(1);
}


void DMA2_Stream5_IRQHandler()  //接收中断
{
	DMA_ClearFlag(DMA2_Stream5,  DMA_FLAG_TEIF5); //清除出错标志
}
#endif

int fputc(int ch, FILE *f)
{
	uint32_t timer = 100*1000, irq_flag;
	while(u1_send.sending.val)   //循环等待
	{
		if(timer-- == 0)
			break;
	}

	irq_flag = local_irq_save();
	if(!u1_send.sending.val)
		u1_send.sending.val = TRUE;
	else{
		local_irq_restore(irq_flag);
		return -1;
	}
	while(!(USART1->SR & USART_FLAG_TXE));
	USART1->DR = ch;
	u1_send.sending.val = FALSE;
	local_irq_restore(irq_flag);
	return (ch);
}

int uart3_send_next()
{
	if(u3_send.cnt < u3_send.size)
	{

		USART3->DR = u3_send.buff[u3_send.cnt++];
		return 0;
	}
	else
	{
		USART3->SR = ~ ((uint16_t)((uint16_t)0x01 << 6));
		u3_send.sending.val = 0;    
		wake_up(u3_send.wait);
		return 1;
	}
}

int uart3_send(u8_t *buff,u32_t size)
{
	uint32_t irq_flag;
	int ret;
	if((size == 0) || (buff == 0))
		return -1;
	irq_flag = local_irq_save(); 
	while(!(USART3->SR & USART_FLAG_TXE));
	u3_send.size = size;
	u3_send.cnt = 0;
	u3_send.buff = buff;
	u3_send.sending.val = 1;
	local_irq_restore(irq_flag);
	uart3_send_next();
	ret = wait_event_timeout(u3_send.wait, 2000);
	u3_send.sending.val = 0;  
	if((-1 == ret) || (u3_send.cnt  !=  u3_send.size))
	{
		p_err("uart2_send err:%d,%d,%d\n",ret,u3_send.cnt,u3_send.size);
	}
	return 0;
}

#if UART1_USE_DMA
int uart1_send(u8_t *buff,u32_t size)
{
	int ret;
	if((size == 0) || (buff == 0))
		return -1;
	p_dbg("uart1 send start\n");
	clear_wait_event(u1_send.wait);
	u1_send.cnt = size;
 	DMA_TxConfiguration((uint32_t*)buff, size);
	
again:
	ret = wait_event_timeout(u1_send.wait, 2000);
	if(ret != 0)
		goto again;
	
	p_dbg("uart1 send end\n");

	if(u1_send.cnt)
		p_err("uart1_send err:%d,%d,%d\n",ret,u1_send.cnt,size);
	else
		p_dbg("uart1 send a  pkg:%d\n", size);
	
	return 0;
}
#else
int uart1_send_next()
{
	if(u1_send.cnt < u1_send.size)
	{

		USART1->DR = u1_send.buff[u1_send.cnt++];
		return 0;
	}
	else
	{
		USART1->SR = ~ ((uint16_t)((uint16_t)0x01 << 6));
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		u1_send.sending.val = 0;    
		wake_up(u1_send.wait);
		return 1;
	}
}

int uart1_send(u8_t *buff,u32_t size)
{
	uint32_t irq_flag;
	int ret;
	if((size == 0) || (buff == 0))
		return -1;
	irq_flag = local_irq_save(); 
	while(!(USART1->SR & USART_FLAG_TXE));
	u1_send.size = size;
	u1_send.cnt = 0;
	u1_send.buff = buff;
	u1_send.sending.val = 1;
	local_irq_restore(irq_flag);
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	uart1_send_next();
	ret = wait_event_timeout(u1_send.wait, 2000);
	u1_send.sending.val = 0;  
	if((-1 == ret) || (u1_send.cnt  !=  u1_send.size))
	{
		p_err("uart1_send err:%d,%d,%d\n",ret,u1_send.cnt,u1_send.size);
	}
	return 0;
}

#endif


//import from main.c
extern wait_event_t command_event;
extern char command;
void uart1_read_data(u8_t *buff, int size)
{
	//TODO
	command = buff[0];
	if(command_event != 0)
		wake_up(command_event);
}

void send_cmd_to_self(char cmd)
{
    command = cmd;
	if(command_event != 0)
		wake_up(command_event);
}

//本函数在1.uart中断里调用;
#if UART1_USE_DMA
int handle_uart1_rcv()
{
#if 1
	__IO u32_t cur_pos, i, size, ret, cnt;
	do
	{
		cur_pos = UART_RECV_BUFF_SIZE - DMA2_Stream5->NDTR;   //取当前缓冲区数据位置
		//如果没有接收到则 cur_pos == u1_recv.pos
		if(cur_pos == u1_recv.pos)
		{
			u1_recv.pkg.pending.val = 0;
			return 0;
		}
		if(cur_pos > u1_recv.pos)
		{
			size = cur_pos - u1_recv.pos;

			uart1_read_data(&u1_recv.c_buff[u1_recv.pos], size);

			u1_recv.pos = cur_pos;
		}
		else
		{
			cnt = 0;
			if(cur_pos < u1_recv.pos)
			{
				size = UART_RECV_BUFF_SIZE - u1_recv.pos;

				uart1_read_data(&u1_recv.c_buff[u1_recv.pos], size);
				
				u1_recv.pos = 0;

				size = cur_pos;

				uart1_read_data(u1_recv.c_buff, size);

				u1_recv.pos = cur_pos;
			}
		}
	}while(0);
	return 0;

#endif
}
#endif

void USART1_IRQHandler()
{
	__IO u32_t sr;
#if !UART1_USE_DMA
	uint8_t data;
#endif
	int need_sched = 0;

	enter_interrupt();
	
 	sr = USART1->SR;
#if UART1_USE_DMA
	if(handle_uart1_rcv())
		need_sched = 1;
#else
	if( sr & (1L<<5))  //rxne
	{
		data = USART1->DR;
		uart1_read_data(&data, 1);
	}
	if( sr & (1L<<6))  //txe
	{
		if(u1_send.sending.val)
		{
			need_sched = uart1_send_next();
		}
		else
		{
			USART_ClearITPendingBit(USART1, USART_IT_TC);
		}
	} 
#endif
	
	exit_interrupt(need_sched);
}


void USART3_IRQHandler()
{
	__IO int i;
	__IO u32_t sr;
	int need_sched = 0;
	
	enter_interrupt();

	sr = USART3->SR;
	if( sr & (1L<<5))  //rxne
	{
		i = USART3->DR;
	}
	if( sr & (1L<<6))  //txe
	{
		if(u3_send.sending.val)
		{
			need_sched = uart3_send_next();
		}
		else
		{
			USART_ClearITPendingBit(USART3, USART_IT_TC);
		}
	} 
	
	exit_interrupt(need_sched);
}


