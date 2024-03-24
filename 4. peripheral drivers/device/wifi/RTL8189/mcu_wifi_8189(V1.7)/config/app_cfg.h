#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "type.h"

#define SUPPORT_AUDIO			1	
#define ENABLE_MP3				1	//需消耗63k flash
#define SUPPORT_WEBSERVER		1	//需消耗37k flash
#define MACHTALK_ENABLE			1	//MACHTALK平台远程控制

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
* UCOS每个线程优先级不能相同，放在这里统一管理
*********************************************************************************************************
*/
#define WORK_QUEUE_MAX_SIZE 4

enum TASK_PRIO{
TASK_UNUSED_PRIO = 10,
TASK_SDIO_THREAD_PRIO,

TASK_TIMER_TASKLET_PRIO,
RTW_XMIT_THREAD_PRIO,
RTW_RECV_THREAD_PRIO,
RTW_CMD_THREAD_PRIO,
TASK_TCPIP_THREAD_PRIO,
TASK_SYS_TASKLET_PRIO,
//以上顺序勿随意改动

//user task
TASK_MONITOR_PRIO,
TASK_MAIN_PRIO,
TASK_ADC_RECV_PKG_PRIO,
TASK_TCP_RECV_PRIO,
TASK_TCP_ACCEPT_PRIO,
TASK_WEB_PRIO,
TASK_TCP_SEND_PRIO,
OS_TASK_TMR_PRIO,
};

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*单位WORD
*********************************************************************************************************
*/
#define TASK_TCPIP_THREAD_STACK_SIZE		(512 + 0)
#define TIMER_TASKLET_STACK_SIZE			(256 + 0)
#define SYS_TASKLET_STACK_SIZE				(256 + 0)
#define MOAL_WROK_STACK_SIZE				(256 + 0)
#define TASK_MONITOR_STACK_SIZE			(128 + 0)
#define TASK_SDIO_STACK_SIZE				(200 + 0)

#define RTW_XMIT_THREAD_STACK_SIZE 			(200 + 0)
//#define RTW_RECV_THREAD_STACK_SIZE 		(512 + 0)
#define RTW_CMD_THREAD_STACK_SIZE 			(480 + 0)
//以上数值勿随意改动

#define TASK_MAIN_STACK_SIZE				(512 + 0)
#define TASK_ADC_RECV_PKG_STACK_SIZE		(256 + 0)
#if ENABLE_MP3
#define TASK_TCP_RECV_STACK_SIZE			(1024 + 512)
#else
#define TASK_TCP_RECV_STACK_SIZE			(512 + 0)
#endif
#define TASK_TCP_SEND_STACK_SIZE			(256 + 0)
#define TASK_ACCEPT_STACK_SIZE				(128 + 0)
#define TASK_WEB_STK_SIZE					(128 + 0)


/*
*********************************************************************************************************
*                               		IRQ_PRIORITY
*如果中断函数里需要打印信息，则此中断优先级要低于uart中断
*********************************************************************************************************
*/

enum IRQ_PRIORITY{
	
	USART1_IRQn_Priority = 1,
	SDIO_IRQn_Priority,
	TIM2_IRQn_Priority,
	USART3_IRQn_Priority,
	DMA2_Stream5_IRQn_Priority,
	DMA2_Stream7_IRQn_Priority,
	ADC_IRQn_Priority,
	DMA1_Stream5_IRQn_Priority,
	SPI3_IRQn_Priority,
	DMA2_Stream0_IRQn_Priority,
	TIM3_IRQn_Priority,
	TIM4_IRQn_Priority,
	DMA2_Stream1_IRQn_Priority,
	DCMI_IRQn_Priority,
	SysTick_IRQn_Priority
};

/*
*********************************************************************************************************
*                                      kernel 里面常用资源值定义
*建议配合monitor软件确定合适的数值
*********************************************************************************************************
*/
#define EVENTS_MAX		64		//EVENTS_MAX是所有事件的数量之和，包括邮箱、消息队列、信号量、互斥量等
#define TIMER_MAX       48		//定时器
#define MSG_QUEUE_MAX	16		//消息队列
#define TASKS_MAX		16		//任务


/*
*********************************************************************************************************
*                                      定义中断向量表地址
* 因低16k地址用于bootloader，我们的代码需要从16k开始，所以需要在启动时将中断向量重新映射到16k地址
* 目前只对F205有效，F103版本不使用bootloader
*********************************************************************************************************
*/
#define IVECTOR_ADDR 					(16*1024)
//#define IVECTOR_ADDR 					(0)		//如果不使用bootloaer（同时将IROM1的start改为0x8000000）

#endif
