/*
*********************************************************************************************************
*
*	模块名称 : CAN驱动模块
*	文件名称 : bsp_can.c
*	版    本 : V1.0
*	说    明 : CAN驱动. 
*
*	修改记录 :
*		版本号  日期        作者        说明
*		V1.0    2023-10-22  eric2013   正式发布
*
*	Copyright (C), 2023-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"


/*
	启用CAN1，需要将V6主板上的J12跳线帽短接PB8，J13跳线帽短接PB9。
	启用CNA2，无需任何设置。
*/

/*
*********************************************************************************************************
*                                             CAN1配置
*********************************************************************************************************
*/
/* CAN1 GPIO定义 */
#define CAN1_TX_PIN       			GPIO_PIN_13
#define CAN1_TX_GPIO_PORT 			GPIOH
#define CAN1_TX_AF        			GPIO_AF9_CAN1
#define CAN1_TX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOH_CLK_ENABLE()

#define CAN1_RX_PIN       			GPIO_PIN_9
#define CAN1_RX_GPIO_PORT 			GPIOI
#define CAN1_RX_AF        			GPIO_AF9_CAN1
#define CAN1_RX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOI_CLK_ENABLE()

uint8_t g_Can1RxData[8];
uint8_t g_Can1TxData[8];

CAN_HandleTypeDef     Can1Handle;
CAN_TxHeaderTypeDef   CAN1TxHeader;
CAN_RxHeaderTypeDef   CAN1RxHeader;
uint32_t              CAN1TxMailbox;
CAN_FilterTypeDef  	  CAN1FilterConfig;

/*
*********************************************************************************************************
*                                             CAN2配置
*********************************************************************************************************
*/
/* CAN2 GPIO定义 */
#define CAN2_TX_PIN       			GPIO_PIN_13
#define CAN2_TX_GPIO_PORT 			GPIOB
#define CAN2_TX_AF        			GPIO_AF9_CAN2
#define CAN2_TX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOB_CLK_ENABLE()

#define CAN2_RX_PIN       			GPIO_PIN_5
#define CAN2_RX_GPIO_PORT 			GPIOB
#define CAN2_RX_AF       			GPIO_AF9_CAN2
#define CAN2_RX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOB_CLK_ENABLE()

uint8_t g_Can2RxData[8];
uint8_t g_Can2TxData[8];

CAN_HandleTypeDef     Can2Handle;
CAN_TxHeaderTypeDef   CAN2TxHeader;
CAN_RxHeaderTypeDef   CAN2RxHeader;
uint32_t              CAN2TxMailbox;
CAN_FilterTypeDef  	  CAN2FilterConfig;

/*
*********************************************************************************************************
*	函 数 名: bsp_InitCan1
*	功能说明: 初始CAN1
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitCan1(void)
{	 
	/*##-1- 配置CAN外设 #######################################*/
	Can1Handle.Instance = CAN1;

	Can1Handle.Init.TimeTriggeredMode = DISABLE;    /* 禁止时间触发模式（不生成时间戳) */
	Can1Handle.Init.AutoBusOff = DISABLE;           /* 禁止自动总线关闭管理 */
	Can1Handle.Init.AutoWakeUp = DISABLE;           /* 禁止自动唤醒模式 */
	Can1Handle.Init.AutoRetransmission = ENABLE;    /* 禁止仲裁丢失或出错后的自动重传功能 */
	Can1Handle.Init.ReceiveFifoLocked = DISABLE;    /* 禁止接收FIFO加锁模式 */ 
	Can1Handle.Init.TransmitFifoPriority = DISABLE; /* 禁止传输FIFO优先级 */
	Can1Handle.Init.Mode = CAN_MODE_NORMAL;         /* 设置CAN为正常工作模式 */  

	/* 
		CAN 波特率 = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为500kbps		
		CAN 波特率 = 420000000 / 2 / (1 + 12 + 8) / = 1Mbps		
	*/
	Can1Handle.Init.SyncJumpWidth = CAN_SJW_1TQ; /* 范围CAN_SJW_1TQ -- CAN_SJW_4TQ  */
	Can1Handle.Init.TimeSeg1 = CAN_BS1_12TQ;     /* 范围CAN_BS1_1TQ -- CAN_BS1_16TQ */
	Can1Handle.Init.TimeSeg2 = CAN_BS2_8TQ;      /* 范围CAN_BS2_1TQ -- CAN_BS2_8TQ  */
	Can1Handle.Init.Prescaler = 2;               /* 范围1-1024 */

	if (HAL_CAN_Init(&Can1Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-2- 配置过滤器 ################################################*/
	CAN1FilterConfig.FilterBank = 0;          				/* 过滤序号，共计28个，供双CAN使用 */
	CAN1FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;   /* ID屏蔽模式 */
	CAN1FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;  /* 32bit过滤 */
	CAN1FilterConfig.FilterIdHigh = 0x0000;                /* 过滤ID，16bit第1个ID，32bit高16bit */
	CAN1FilterConfig.FilterIdLow = 0x0000;                 /* 过滤ID，16bit第2个ID，32bit低16bit */
	CAN1FilterConfig.FilterMaskIdHigh = 0x0000;            /* 屏蔽位，16bit第1个ID，32bit高16bit */ 
	CAN1FilterConfig.FilterMaskIdLow = 0x0000;             /* 屏蔽位，16bit第2个ID，32bit高16bit */ 
	CAN1FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  /* 使用FIFO0 */
	CAN1FilterConfig.FilterActivation = ENABLE;            /* 使能过滤 */
	CAN1FilterConfig.SlaveStartFilterBank = 0;             /* 28个共享过滤器起始地址 */  

	if (HAL_CAN_ConfigFilter(&Can1Handle, &CAN1FilterConfig) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- 启动CAN外设 #################################################*/
	if (HAL_CAN_Start(&Can1Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-4- 配置FIFO0 Pending中断 #######################################*/
	if (HAL_CAN_ActivateNotification(&Can1Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
	
	/*##-5- 配置邮箱发送空中断 ##########################################*/
	if (HAL_CAN_ActivateNotification(&Can1Handle, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitCan2
*	功能说明: 初始CAN2
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitCan2(void)
{	
	/*##-1- 配置外设 #######################################*/
	Can2Handle.Instance = CAN2;

	Can2Handle.Init.TimeTriggeredMode = DISABLE;    /* 禁止时间触发模式（不生成时间戳) */
	Can2Handle.Init.AutoBusOff = DISABLE;           /* 禁止自动总线关闭管理 */
	Can2Handle.Init.AutoWakeUp = DISABLE;           /* 禁止自动唤醒模式 */
	Can2Handle.Init.AutoRetransmission = ENABLE;    /* 禁止仲裁丢失或出错后的自动重传功能 */
	Can2Handle.Init.ReceiveFifoLocked = DISABLE;    /* 禁止接收FIFO加锁模式 */ 
	Can2Handle.Init.TransmitFifoPriority = DISABLE; /* 禁止传输FIFO优先级 */
	Can2Handle.Init.Mode = CAN_MODE_NORMAL;         /* 设置CAN为正常工作模式 */  

	/* 
		CAN 波特率 = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为500kbps		
		CAN 波特率 = 420000000 / 2 / (1 + 12 + 8) / = 1Mbps		
	*/
	Can2Handle.Init.SyncJumpWidth = CAN_SJW_1TQ; /* CAN_SJW_1TQ -- CAN_SJW_4TQ  */
	Can2Handle.Init.TimeSeg1 = CAN_BS1_12TQ;     /* CAN_BS1_1TQ -- CAN_BS1_16TQ */
	Can2Handle.Init.TimeSeg2 = CAN_BS2_8TQ;      /* CAN_BS2_1TQ -- CAN_BS2_8TQ  */
	Can2Handle.Init.Prescaler = 2;               /* 范围1-1024 */

	if (HAL_CAN_Init(&Can2Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-2- 配置过滤器 ###########################################*/
	CAN2FilterConfig.FilterBank = 14;          			   /* 过滤序号，共计28个，供双CAN使用 */
	CAN2FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;   /* ID屏蔽模式 */
	CAN2FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;  /* 32bit过滤 */
	CAN2FilterConfig.FilterIdHigh = 0x0000;                /* 过滤ID，16bit第1个ID，32bit高16bit */
	CAN2FilterConfig.FilterIdLow = 0x0000;                 /* 过滤ID，16bit第2个ID，32bit低16bit */
	CAN2FilterConfig.FilterMaskIdHigh = 0x0000;            /* 屏蔽位，16bit第1个ID，32bit高16bit */ 
	CAN2FilterConfig.FilterMaskIdLow = 0x0000;             /* 屏蔽位，16bit第2个ID，32bit高16bit */ 
	CAN2FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;  /* 使用FIFO0 */
	CAN2FilterConfig.FilterActivation = ENABLE;            /* 使能过滤 */
	CAN2FilterConfig.SlaveStartFilterBank = 14;            /* 28个共享过滤器起始地址 */  

	if (HAL_CAN_ConfigFilter(&Can2Handle, &CAN2FilterConfig) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- 启动CAN ###########################################*/
	if (HAL_CAN_Start(&Can2Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-4- 使能FIFO1 Pending中断 #######################################*/
	if (HAL_CAN_ActivateNotification(&Can2Handle, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
	
	/*##-5- 使能发送空中断 #######################################*/
	if (HAL_CAN_ActivateNotification(&Can2Handle, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_MspInit
*	功能说明: 配置CAN gpio
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	/* 使用HAL库配置双CAN一定要把提前把两个外设GPIO都配置了，否则接线后，会触发错误Wait initialisation acknowledge */
	
	
	/*##-1- 使能CAN时钟 #################################*/
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_CAN2_CLK_ENABLE();
	
	/* 使能GPIO时钟 ****************************************/
	CAN1_TX_GPIO_CLK_ENABLE();
	CAN1_RX_GPIO_CLK_ENABLE();

	/*##-2- 配置GPIO ##########################################*/
	GPIO_InitStruct.Pin = CAN1_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN1_TX_AF;

	HAL_GPIO_Init(CAN1_TX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CAN1_RX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN1_RX_AF;

	HAL_GPIO_Init(CAN1_RX_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- 配置NVIC #################################################*/
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	
	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////	
	
	/*##-1- 使能CAN时钟 #################################*/
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_CAN2_CLK_ENABLE();

	CAN2_TX_GPIO_CLK_ENABLE();
	CAN2_RX_GPIO_CLK_ENABLE();

	/*##-2- 配置GPIO ######################################*/
	GPIO_InitStruct.Pin = CAN2_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN2_TX_AF;

	HAL_GPIO_Init(CAN2_TX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CAN2_RX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN2_RX_AF;

	HAL_GPIO_Init(CAN2_RX_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- 配置NVIC #################################################*/
	HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
	
	HAL_NVIC_SetPriority(CAN2_TX_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_MspDeInit
*	功能说明: 配置CAN gpio, 恢复为普通GPIO，取消中断
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		__HAL_RCC_CAN1_FORCE_RESET();
		__HAL_RCC_CAN1_RELEASE_RESET();

		HAL_GPIO_DeInit(CAN1_TX_GPIO_PORT, CAN1_TX_PIN);
		HAL_GPIO_DeInit(CAN1_RX_GPIO_PORT, CAN1_RX_PIN);

		HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
	}
	
	if(hcan == &Can2Handle)
	{
		__HAL_RCC_CAN2_FORCE_RESET();
		__HAL_RCC_CAN2_RELEASE_RESET();

		HAL_GPIO_DeInit(CAN2_TX_GPIO_PORT, CAN2_TX_PIN);
		HAL_GPIO_DeInit(CAN2_RX_GPIO_PORT, CAN2_RX_PIN);

		HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_RxFifo0MsgPendingCallback
*	功能说明: Rx FIFO0回调函数
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CAN1RxHeader, g_Can1RxData) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);
		}

		if (CAN1RxHeader.StdId == 0x222)
		{
			bsp_PutMsg(MSG_CAN1_RxFIFO0Pending, 0);
		}	
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_RxFifo1MsgPendingCallback
*	功能说明: Rx FIFO1回调函数
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can2Handle)
	{
		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CAN2RxHeader, g_Can2RxData) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);
		}

		if (CAN2RxHeader.StdId == 0x222)
		{
			bsp_PutMsg(MSG_CAN2_RxFIFO1Pending, 0);
		}	
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_TxMailbox0CompleteCallback
*	功能说明: TxMailbox0回调函数
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		bsp_PutMsg(MSG_CAN1_TxMailBox0, 0);		
	}
	
	if(hcan == &Can2Handle)
	{
		bsp_PutMsg(MSG_CAN2_TxMailBox0, 0);			
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_TxMailbox1CompleteCallback
*	功能说明: TxMailbox1回调函数
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		bsp_PutMsg(MSG_CAN1_TxMailBox1, 0);		
	}
	
	if(hcan == &Can2Handle)
	{
		bsp_PutMsg(MSG_CAN2_TxMailBox1, 0);			
	}
}

/*
*********************************************************************************************************
*	函 数 名: HAL_CAN_TxMailbox2CompleteCallback
*	功能说明: TxMailbox2回调函数
*	形    参: hcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		bsp_PutMsg(MSG_CAN1_TxMailBox2, 0);		
	}
	
	if(hcan == &Can2Handle)
	{
		bsp_PutMsg(MSG_CAN2_TxMailBox2, 0);			
	}
}

/*
*********************************************************************************************************
*	函 数 名: can1_SendPacket
*	功能说明: 发送一包数据
*	形    参：_DataBuf 数据缓冲区，_Len 数据长度 0 - 8字节
*	返 回 值: 无
*********************************************************************************************************
*/
void can1_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	CAN1TxHeader.StdId = 0x222;
	CAN1TxHeader.ExtId = 0x00;
	CAN1TxHeader.RTR = CAN_RTR_DATA; /* 数据帧 */
	CAN1TxHeader.IDE = CAN_ID_STD;   /* 标准ID */
	CAN1TxHeader.DLC = _Len;
	CAN1TxHeader.TransmitGlobalTime = DISABLE;
	
	/* 启动发送 */
	if (HAL_CAN_AddTxMessage(&Can1Handle, &CAN1TxHeader, _DataBuf, &CAN1TxMailbox) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);	  
	}
}

/*
*********************************************************************************************************
*	函 数 名: can2_SendPacket
*	功能说明: 发送一包数据
*	形    参：_DataBuf 数据缓冲区，_Len 数据长度 0 - 8字节
*	返 回 值: 无
*********************************************************************************************************
*/
void can2_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	CAN2TxHeader.StdId = 0x222;
	CAN2TxHeader.ExtId = 0x00;
	CAN2TxHeader.RTR = CAN_RTR_DATA;
	CAN2TxHeader.IDE = CAN_ID_STD;
	CAN2TxHeader.DLC = _Len;
	CAN2TxHeader.TransmitGlobalTime = DISABLE;
	
	/* 启动发送 */
	if (HAL_CAN_AddTxMessage(&Can2Handle, &CAN2TxHeader, _DataBuf, &CAN2TxMailbox) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);	  
	}
}

/*
*********************************************************************************************************
*	函 数 名: ---
*	功能说明: CAN中断服务程序
*	形    参: can
*	返 回 值: 无
*********************************************************************************************************
*/
void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can1Handle);
}

void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can1Handle);
}

void CAN2_RX1_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can2Handle);
}

void CAN2_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can2Handle);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
