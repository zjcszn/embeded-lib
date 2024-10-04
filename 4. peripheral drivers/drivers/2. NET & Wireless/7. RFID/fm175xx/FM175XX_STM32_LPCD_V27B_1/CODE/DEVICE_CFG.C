/* Includes ------------------------------------------------------------------*/
#include "DEVICE_CFG.h"

  GPIO_InitTypeDef   GPIO_InitStructure;
  USART_InitTypeDef  USART_InitStructure;
  UART_Com_Para_Def  UART_Com1_Para;
	UART_Com_Para_Def  UART_Com3_Para;
  NVIC_InitTypeDef   NVIC_InitStructure;

/*******************************************************************************
* Function Name  : RCC_Config
* Description    : Configures Main system clocks & power
* Input          : None.
* Return         : None.
*******************************************************************************/
void RCC_Config(void)
{
  u32 Clk_Enable_Sel1,Clk_Enable_Sel2;
	u8 status;

  /* RCC system reset */
  RCC_DeInit();

  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);	   //????????
	//RCC_HSICmd(ENABLE); 

  /* Wait till HSE is ready */
  status = RCC_WaitForHSEStartUp();		//??????????

  if (status == SUCCESS)				    //?????????
  {
    	/* Enable Prefetch Buffer */
    	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    	/* Flash 2 wait state */
    	FLASH_SetLatency(FLASH_Latency_2);
 
    	/* HCLK = SYSCLK */
    	RCC_HCLKConfig(RCC_SYSCLK_Div1); 
    	/* PCLK2 = HCLK */					   
    	RCC_PCLK2Config(RCC_HCLK_Div1); 	   //APB2 CLK : 72MHz
    	/* PCLK1 = HCLK/2 */
    	RCC_PCLK1Config(RCC_HCLK_Div2);		   //APB1 CLK :36MHz
		
		/* PLLCLK = 8 * 8MHz= 64 MHz */ 
		//#define HSE_Value    ((u32)8000000) /* Value of the External oscillator in Hz  8MHZ*/	
	  
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);  //
			  	
		RCC_PLLCmd(ENABLE);	    /* Enable PLL */ 
    	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){} //??PLL???

    	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	/* Select PLL as system clock source */

    	while(RCC_GetSYSCLKSource() != 0x08){}		/* Wait till PLL is used as system clock source */
	
		Clk_Enable_Sel2 = 0;
		if(UART1_EN_SEL)
			Clk_Enable_Sel2  |= RCC_APB2Periph_USART1;	
		if(SPI1_EN_SEL)
			Clk_Enable_Sel2  |= RCC_APB2Periph_SPI1;				
		Clk_Enable_Sel2 |= (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_TIM1 |\
				RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO);

		Clk_Enable_Sel1 = 0;
		if(UART3_EN_SEL)
			Clk_Enable_Sel1  |= RCC_APB1Periph_USART3;
		if(I2C_EN_SEL)
			Clk_Enable_Sel1  |= RCC_APB1Periph_I2C2;
		if(USB_EN_SEL)
			Clk_Enable_Sel1  |= RCC_APB1Periph_USB;	
		Clk_Enable_Sel1  |= RCC_APB1Periph_SPI2;

		Clk_Enable_Sel1  |= RCC_APB1Periph_PWR | RCC_APB1Periph_BKP;
		
		RCC_APB2PeriphClockCmd(Clk_Enable_Sel2, ENABLE);		//??APB2????????
   	RCC_APB1PeriphClockCmd(Clk_Enable_Sel1, ENABLE);		//??APB1????????
		//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);			//????????
  }
  else		 //????????....
  { 
  		/* If HSE fails to start-up, the application will have wrong clock configuration.
       	User can add here some code to deal with this error */    

    	/* Go to infinite loop */
    	while (1)
    	{
    	}
  }
}


/*******************************************************************************
* Function Name  : GPIO_CONFIG
* Description    : Configures GPIO
* Input          : None.
* Return         : None.
*******************************************************************************/
void GPIO_Config(void)
{

	//Configure PC.00~07、PC.08~13 as Output push-pull 
	// PC8~PC13:FM175XX A0~A1
  	GPIO_InitStructure.GPIO_Pin = PIN_EA | PIN_I2C; 
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(PORT_ADDR, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = PIN_NRST;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(PORT_NRST, &GPIO_InitStructure);
	

		GPIO_InitStructure.GPIO_Pin = PIN_IRQ;  
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// Configure PB.12(IRQ) as FLOATING INPUT	
		GPIO_Init(PORT_IRQ, &GPIO_InitStructure);


		/*LED灯控制：PD2*/
		GPIO_InitStructure.GPIO_Pin = PIN_LED0;					
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_2MHz;
		GPIO_Init(PORT_LED, &GPIO_InitStructure);

		/*SPI2 Configuration*/
		// Configure SPI2 NSS (PB.12)、SCK(PB.13)、MISO(PB.14)、MOSI(PB.15) as alternate function push-pull  
		GPIO_InitStructure.GPIO_Pin = PIN_SCK | PIN_MISO | PIN_MOSI; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
		GPIO_Init(PORT_SPI, &GPIO_InitStructure);  

		GPIO_InitStructure.GPIO_Pin = PIN_NSS;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
		GPIO_Init(PORT_SPI, &GPIO_InitStructure);

//		/*USART1 Configuration*/
//		// Configure USART1 Tx (PA.09) as alternate function push-pull 
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	  
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//		GPIO_Init(GPIOA, &GPIO_InitStructure);
//			// Configure USART1 Rx (PA.10) as input floating
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
//		GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*USART3 Configuration*/
		// Configure USART3 Tx (PB.10) as alternate function push-pull 
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		// Configure USART3 Rx (PB.11) as input floating
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
		GPIO_Init(GPIOB, &GPIO_InitStructure);
}


//***********************************************
//函数名称：EXTI15_10_IRQ_CTRL
//函数功能：控制外部中断开关
//入口参数：sw：ENABLE/DISABLE
//出口参数：N/A
//***********************************************
void EXTI15_10_IRQ_CTRL(FunctionalState sw)
{
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQChannel; 	//中断通道　　
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//强占优先级　　
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 					//次优先级　　
	NVIC_InitStructure.NVIC_IRQChannelCmd = sw; 								//通道中断使能　　
	NVIC_Init(&NVIC_InitStructure); 														//初始化中断
}

/*******************************************************************************
* Function Name  : EXTI_CONFIG
* Description    : Configures EXTI 外部中断配置
* Input          : None.
* Return         : None.
*******************************************************************************/
void EXTI_Config(void)
{
	 EXTI_InitTypeDef  EXTI_InitStructure;
	 EXTI_DeInit();		//EXTI初始化
	 NVIC_DeInit();		//中断优先级初始化

	 NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0); 
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);	//抢占优先级：0/1  从优先级0-7

	 /* Enable the USART3 Interrupt */
	 NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);

	 NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQChannel; 	//中断通道　　
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//强占优先级　　
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 			//次优先级　　
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				//通道中断使能　　
	 NVIC_Init(&NVIC_InitStructure); 								//初始化中断

	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource12);	//PC12设置到EXTI12上

	EXTI_DeInit();
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line12; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;			//EXTI_Trigger_Falling; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;  
	EXTI_Init(&EXTI_InitStructure);

	//Timer1 中断使能
//	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQChannel;	//更新事件
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;			//响应优先级
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//允许中断
//  NVIC_Init(&NVIC_InitStructure);

//	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;//指定中断源
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;// 指定响应优先级别1
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//抢占优先级
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;			//响应优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : USART1_CONFIG
* Description    : Configures USART1     UART1配置
* Input          : None.
* Return         : None.
*******************************************************************************/
void USART1_Config(void)
{
  /* USART1 configuration ----------------------------------------------------*/
  /* USART1 configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - Two Stop Bit
        - Odd parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
        - USART Clock disabled
        - USART CPOL: Clock is active low
        - USART CPHA: Data is captured on the second edge 
        - USART LastBit: The clock pulse of the last data bit is not output to 
                         the SCLK pin
  */
  USART_DeInit(USART1);	   //复位
 
  /* Configure the USART1 */
  USART_InitStructure.USART_BaudRate = UART1_BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);			  
  
  USART_Cmd(USART1, ENABLE);
    
  USART_ITConfig(USART1, USART_IT_TC, ENABLE);     //发送完成中断
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);   //接收完成中断
}
/*******************************************************************************
* Function Name  : USART3_CONFIG
* Description    : Configures USART3     UART3配置
* Input          : None.
* Return         : None.
*******************************************************************************/
void USART3_Config(void)
{
  USART_DeInit(USART3);	   //复位
 
  /* Configure the USART3 */
  USART_InitStructure.USART_BaudRate = UART3_BaudRate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART3, &USART_InitStructure);			  
  
  USART_Cmd(USART3, ENABLE);
    
  USART_ITConfig(USART3, USART_IT_TC, ENABLE);     //发送完成中断  
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);   //只有自动收发模式下，才启用中断模式接收
}

void SPI_Config(void)
{
  SPI_InitTypeDef SPI_InitStructure;
  /* SPI2 Config: SPI2 Master*/
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure);  
  SPI_Cmd(SPI2, ENABLE);/* Enable SPI2 */
}


void mDelay(unsigned int ms)			//实测1ms
{
	unsigned int i;
	unsigned int j;
	for(j=0;j<ms;j++)
	{
		for(i=0;i<8100;i++)
			;
	}
}

void uDelay(unsigned int us)			//实测1us
{
	unsigned int i;
	unsigned int j;
	for(j=0;j<us;j++)
	{
		for(i=0;i<81;i++)
			;
	}
}
