/*
*********************************************************************************************************
*
*	模块名称 : FDCAN驱动模块
*	文件名称 : bsp_can.c
*	版    本 : V1.1
*	说    明 : CAN驱动. 
*
*	修改记录 :
*		版本号  日期        作者      说明
*		V1.0    2018-11-14  armfly   正式发布
*		V1.1    2021-01-23  armfly   升级
*		V1.2    2023-05-12  Eric2013 升级
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"



/*
	启用CAN1，需要将V7主板上的J12跳线帽短接PA11，J13跳线帽短接PA12。
	启用CNA2，硬件无需跳线，以太网功能需要屏蔽（有引脚复用）。
*/

/*
*********************************************************************************************************
*                                             FDCAN1配置
*********************************************************************************************************
*/
/* FDCAN1 GPIO定义 */
#define FDCAN1_TX_PIN       GPIO_PIN_12
#define FDCAN1_TX_GPIO_PORT GPIOA
#define FDCAN1_TX_AF        GPIO_AF9_FDCAN1
#define FDCAN1_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define FDCAN1_RX_PIN       GPIO_PIN_11
#define FDCAN1_RX_GPIO_PORT GPIOA
#define FDCAN1_RX_AF        GPIO_AF9_FDCAN1
#define FDCAN1_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

FDCAN_HandleTypeDef hfdcan1;
FDCAN_FilterTypeDef sFilterConfig1;

/*
*********************************************************************************************************
*                                             FDCAN2配置
*********************************************************************************************************
*/
/* FDCAN1 GPIO定义 */
#define FDCAN2_TX_PIN       GPIO_PIN_13
#define FDCAN2_TX_GPIO_PORT GPIOB
#define FDCAN2_TX_AF        GPIO_AF9_FDCAN2
#define FDCAN2_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define FDCAN2_RX_PIN       GPIO_PIN_12
#define FDCAN2_RX_GPIO_PORT GPIOB
#define FDCAN2_RX_AF        GPIO_AF9_FDCAN2
#define FDCAN2_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

FDCAN_HandleTypeDef hfdcan2;
FDCAN_FilterTypeDef sFilterConfig2;

FDCAN_RxHeaderTypeDef g_Can1RxHeader;
uint8_t g_Can1RxData[64];

FDCAN_RxHeaderTypeDef g_Can2RxHeader;
uint8_t g_Can2RxData[64];
	
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
	/* 第1步：基础配置 ==================================================================================*/
	hfdcan1.Instance = FDCAN1;                     /* 配置FDCAN1 */             
	hfdcan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS; /* 配置使用FDCAN可变波特率 */  
	hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;         /* 配置使用正常模式 */ 
	hfdcan1.Init.AutoRetransmission = ENABLE;      /* 使能自动重发 */ 
	hfdcan1.Init.TransmitPause = DISABLE;          /* 配置禁止传输暂停特性 */
	hfdcan1.Init.ProtocolException = ENABLE;       /* 协议异常处理使能 */
	
	/* 第2步：波特率配置 ==================================================================================*/
	/* 
		配置仲裁阶段波特率 
		CAN时钟20MHz时，仲裁阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / （1+0x1F + 8） = 0.5Mbps	
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = NominalTimeSeg1， Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan1.Init.NominalPrescaler = 0x02; 	  /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck), 范围1-512 */
	hfdcan1.Init.NominalSyncJumpWidth = 0x04; /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大，范围1-128 */
	hfdcan1.Init.NominalTimeSeg1 = 0x05; 	  /* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1，范围2-256 */
	hfdcan1.Init.NominalTimeSeg2 = 0x04;      /* 对应位时间特性图的 Phase_Seg2，范围2- 128 */


	/* 
		配置数据阶段波特率 
		CAN时钟20MHz时，数据阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / 2 / （1+5+ 4） = 1Mbps
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = DataTimeSeg1， Phase_Seg2 = DataTimeSeg2
	*/
	hfdcan1.Init.DataPrescaler = 0x02;      /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck)，范围1-32 */
	hfdcan1.Init.DataSyncJumpWidth = 0x04;  /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大，范围1-16 */
	hfdcan1.Init.DataTimeSeg1 = 0x05; 		/* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1，范围 */
	hfdcan1.Init.DataTimeSeg2 = 0x04;       /* 对应位时间特性图的 Phase_Seg2 */    
	
	/* 第3步：RAM管理器配置 ==================================================================================*/
	hfdcan1.Init.MessageRAMOffset = 0;      			/* CAN1和CAN2共享2560个字, 这里CAN1分配前1280字 */
	hfdcan1.Init.StdFiltersNbr = 4;         			/* 设置标准ID过滤器个数，范围0-128 */       
	hfdcan1.Init.ExtFiltersNbr = 4;         			/* 设置扩展ID过滤器个数，范围0-64 */   
	hfdcan1.Init.RxFifo0ElmtsNbr = 3;                   /* 设置Rx FIFO0的元素个数，范围0-64 */  
	hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* 设置Rx FIFO0中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */   
	hfdcan1.Init.RxFifo1ElmtsNbr = 3;                   /* 设置Rx FIFO1的元素个数，范围0-64 */
	hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* 设置Rx FIFO1中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	
	hfdcan1.Init.RxBuffersNbr = 2;                      /* 设置Rx Buffer个数，范围0-64 */
	hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;     /* 设置Rx Buffer中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	

	hfdcan1.Init.TxEventsNbr = 16;							/* 设置Tx Event FIFO中元素个数，范围0-32 */	
	hfdcan1.Init.TxBuffersNbr = 8;							/* 设置Tx Buffer中元素个数，范围0-32 */
	hfdcan1.Init.TxFifoQueueElmtsNbr = 8;                   /* 设置用于Tx FIFO/Queue的Tx Buffers个数。范围0到32 */
	hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* 设置FIFO模式或者QUEUE队列模式 */
	hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* 设置Tx Element中的数据域大小，支持8,12,16,20,24,32,48或者64字节 */
	HAL_FDCAN_Init(&hfdcan1);

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
	/* 第1步：基础配置 ==================================================================================*/
	hfdcan2.Instance = FDCAN2;                     /* 配置FDCAN1 */             
	hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC; /* 配置使用FDCAN可变波特率 */  
	hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;         /* 配置使用正常模式 */ 
	hfdcan2.Init.AutoRetransmission = ENABLE;      /* 使能自动重发 */ 
	hfdcan2.Init.TransmitPause = DISABLE;          /* 配置禁止传输暂停特性 */
	hfdcan2.Init.ProtocolException = ENABLE;       /* 协议异常处理使能 */
	
	/* 第2步：波特率配置 ==================================================================================*/
	/* 
		配置仲裁阶段波特率 
		CAN时钟20MHz时，仲裁阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / （1+0x1F + 8） = 0.5Mbps	
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = NominalTimeSeg1， Phase_Seg2 = NominalTimeSeg2
	*/
	hfdcan2.Init.NominalPrescaler = 0x04; 	  /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck) */
	hfdcan2.Init.NominalSyncJumpWidth = 0x04; /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大 */
	hfdcan2.Init.NominalTimeSeg1 = 0x05; 	  /* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1 */
	hfdcan2.Init.NominalTimeSeg2 = 0x04;      /* 对应位时间特性图的 Phase_Seg2 */


	/* 
		配置数据阶段波特率 
		CAN时钟20MHz时，数据阶段的波特率就是
		CAN FD Freq / (Sync_Seg + Pro_Seg + Phase_Seg1 + Phase_Seg2) = 20MHz / 2 / （1+5+ 4） = 1Mbps
		
		其中Sync_Seg是固定值 = 1 ， Pro_Seg + Phase_Seg1 = DataTimeSeg1， Phase_Seg2 = DataTimeSeg2
	*/
	hfdcan2.Init.DataPrescaler = 0x02;      /* CAN时钟分配设置，一般设置为1即可，全部由PLL配置好，tq = NominalPrescaler x (1/ fdcan_ker_ck)，范围1-32 */
	hfdcan2.Init.DataSyncJumpWidth = 0x04;  /* 用于动态调节  Phase_Seg1和 Phase_Seg1，所以不可以比Phase_Seg1和 Phase_Seg1大，范围1-16 */
	hfdcan2.Init.DataTimeSeg1 = 0x05; 		/* 特别注意这里的Seg1，这里是两个参数之和，对应位时间特性图的 Pro_Seg + Phase_Seg1，范围 */
	hfdcan2.Init.DataTimeSeg2 = 0x04;       /* 对应位时间特性图的 Phase_Seg2 */    
	
	/* 第3步：RAM管理器配置 ==================================================================================*/
	hfdcan2.Init.MessageRAMOffset = 1280;      			/* CAN1和CAN2共享2560个字, 这里CAN1分配后1280字 */
	hfdcan2.Init.StdFiltersNbr = 4;         			/* 设置标准ID过滤器个数，范围0-128 */       
	hfdcan2.Init.ExtFiltersNbr = 4;         			/* 设置扩展ID过滤器个数，范围0-64 */   
	hfdcan2.Init.RxFifo0ElmtsNbr = 64;                  /* 设置Rx FIFO0的元素个数，范围0-64 */  
	hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8; 	/* 设置Rx FIFO0中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */   
	hfdcan2.Init.RxFifo1ElmtsNbr = 16;                  /* 设置Rx FIFO1的元素个数，范围0-64 */
	hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;  /* 设置Rx FIFO1中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	
	hfdcan2.Init.RxBuffersNbr = 16;                     /* 设置Rx Buffer个数，范围0-64 */
	hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;     /* 设置Rx Buffer中每个元素大小，支持8,12,16,20,24,32,48或者64字节 */	

	hfdcan2.Init.TxEventsNbr = 16;							/* 设置Tx Event FIFO中元素个数，范围0-32 */	
	hfdcan2.Init.TxBuffersNbr = 8;							/* 设置Tx Buffer中元素个数，范围0-32 */
	hfdcan2.Init.TxFifoQueueElmtsNbr = 32;                   /* 设置用于Tx FIFO/Queue的Tx Buffers个数。范围0到32 */
	hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; /* 设置FIFO模式或者QUEUE队列模式 */
	hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;           /* 设置Tx Element中的数据域大小，支持8,12,16,20,24,32,48或者64字节 */
	HAL_FDCAN_Init(&hfdcan2);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DeInitCan1
*	功能说明: 释放CAN1
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DeInitCan1(void)
{
	HAL_FDCAN_MspDeInit(&hfdcan1);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DeInitCan2
*	功能说明: 释放CAN2
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_DeInitCan2(void)
{
	HAL_FDCAN_MspDeInit(&hfdcan2);
}

/*
*********************************************************************************************************
*	函 数 名: HAL_FDCAN_MspInit
*	功能说明: 配置CAN gpio
*	形    参: hfdcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	if (hfdcan == &hfdcan1)
	{
		/*##-1- 使能外设这个GPIO时钟 #################################*/
		FDCAN1_TX_GPIO_CLK_ENABLE();
		FDCAN1_RX_GPIO_CLK_ENABLE();

		/* 选择PLL2Q作为FDCANx时钟 */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInitStruct.PLL2.PLL2M = 5;
        PeriphClkInitStruct.PLL2.PLL2N = 80;
        PeriphClkInitStruct.PLL2.PLL2P = 2;
        PeriphClkInitStruct.PLL2.PLL2Q = 20;
        PeriphClkInitStruct.PLL2.PLL2R = 2;
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
        PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }

		__HAL_RCC_FDCAN_CLK_ENABLE();

		GPIO_InitStruct.Pin       = FDCAN1_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = FDCAN1_TX_AF;
		HAL_GPIO_Init(FDCAN1_TX_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin       = FDCAN1_RX_PIN;
		GPIO_InitStruct.Alternate = FDCAN1_RX_AF;
		HAL_GPIO_Init(FDCAN1_RX_GPIO_PORT, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN_CAL_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN_CAL_IRQn);
	}
	
	if (hfdcan == &hfdcan2)
	{
		/* 选择PLL2Q作为FDCANx时钟 */
		FDCAN2_TX_GPIO_CLK_ENABLE();
		FDCAN2_RX_GPIO_CLK_ENABLE();

        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        PeriphClkInitStruct.PLL2.PLL2M = 5;
        PeriphClkInitStruct.PLL2.PLL2N = 80;
        PeriphClkInitStruct.PLL2.PLL2P = 2;
        PeriphClkInitStruct.PLL2.PLL2Q = 20;
        PeriphClkInitStruct.PLL2.PLL2R = 2;
        PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
        PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
        PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler(__FILE__, __LINE__);
        }

		__HAL_RCC_FDCAN_CLK_ENABLE();

		GPIO_InitStruct.Pin       = FDCAN2_TX_PIN;
		GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull      = GPIO_PULLUP;
		GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = FDCAN2_TX_AF;
		HAL_GPIO_Init(FDCAN2_TX_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin       = FDCAN2_RX_PIN;
		GPIO_InitStruct.Alternate = FDCAN2_RX_AF;
		HAL_GPIO_Init(FDCAN2_RX_GPIO_PORT, &GPIO_InitStruct);

		
		HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN2_IT1_IRQn, 2, 0);
		HAL_NVIC_SetPriority(FDCAN_CAL_IRQn, 2, 0);
		
		HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN2_IT1_IRQn);
		HAL_NVIC_EnableIRQ(FDCAN_CAL_IRQn);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: HAL_FDCAN_MspInit
*	功能说明: 配置CAN gpio, 恢复为普通GPIO，取消中断
*	形    参: hfdcan
*	返 回 值: 无
*********************************************************************************************************
*/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
	if (hfdcan == &hfdcan1)
	{	
		__HAL_RCC_FDCAN_FORCE_RESET();
		__HAL_RCC_FDCAN_RELEASE_RESET();

		HAL_GPIO_DeInit(FDCAN1_TX_GPIO_PORT, FDCAN1_TX_PIN);

		HAL_GPIO_DeInit(FDCAN1_RX_GPIO_PORT, FDCAN1_RX_PIN);

		HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN_CAL_IRQn);
	}
	
	if (hfdcan == &hfdcan2)
	{	
		__HAL_RCC_FDCAN_FORCE_RESET();
		__HAL_RCC_FDCAN_RELEASE_RESET();

		HAL_GPIO_DeInit(FDCAN2_TX_GPIO_PORT, FDCAN2_TX_PIN);

		HAL_GPIO_DeInit(FDCAN2_RX_GPIO_PORT, FDCAN2_RX_PIN);

		HAL_NVIC_DisableIRQ(FDCAN2_IT0_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN2_IT1_IRQn);
		HAL_NVIC_DisableIRQ(FDCAN_CAL_IRQn);
	}	
}

/*
*********************************************************************************************************
*	函 数 名: FDCAN1_IT0_IRQHandler
*	功能说明: CAN中断服务程序
*	形    参: hfdcan
*	返 回 值: 无
*********************************************************************************************************
*/
void FDCAN1_IT0_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN2_IT0_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan2);
}

void FDCAN1_IT1_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan1);
}

void FDCAN2_IT1_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan2);
}

void FDCAN_CAL_IRQHandler(void)
{
	HAL_FDCAN_IRQHandler(&hfdcan1);
	HAL_FDCAN_IRQHandler(&hfdcan2);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
