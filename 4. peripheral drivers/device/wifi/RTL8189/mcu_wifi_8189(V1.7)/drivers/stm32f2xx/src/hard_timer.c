#include "drivers.h"
#include "bsp.h"

 void (*hard_timer_call_back)(void) = 0;
void start_timer(uint32_t freq, void *call_back)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	uint32_t apbclock, scaler = 0, Counter;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	hard_timer_call_back = call_back;
		
	TIM_DeInit(TIM2); 
  	TIM_TimeBaseStructure.TIM_Period=100;
  	TIM_TimeBaseStructure.TIM_Prescaler=0;
  	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
  	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
  	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	Counter = apbclock*2/(scaler + 1)/freq;
	TIM_SetAutoreload(TIM2, Counter);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM2_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM2,ENABLE);
}

void pause_timer()
{
	TIM_Cmd(TIM2,DISABLE);	
}

void continue_timer()
{
	TIM_Cmd(TIM2,ENABLE);	
}

void TIM2_IRQHandler(void)
{
	static int test = 0;

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	if(hard_timer_call_back)
		hard_timer_call_back();
	test = !test;
	if(test)
		GPIO2_SET;
	else
		GPIO2_CLR;
}



