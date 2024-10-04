#define DEBUG
#include "audio_low_level.h"
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "audio/adpcm.h"
#include "audio/audio.h"
#include "debug.h"

#if SUPPORT_AUDIO

struct AUDIO_DEC_INPUT 	dec_input;
struct AUDIO_DEC_OUTPUT dec_output;
struct AUDIO_ADC		adc;

#define ADC_RECV_STK_SIZE 	128

wait_event_t adc_rx_wait = 0;
bool	audio_record_enable = FALSE;
static void adc_recv_thread(void *arg);
static void DMA_ADC1Configuration(uint32_t *BufferDST, uint32_t BufferSize);

#define TEST_DAC


#if USE_I2S_MODE

static void DMA_I2S3configuration(uint32_t *BufferSRC, uint32_t BufferSize);

void i2s_config(uint16_t _usAudioFreq)
{
	I2S_InitTypeDef I2S_InitStruct;

	I2S_InitStruct.I2S_AudioFreq = _usAudioFreq;
	I2S_InitStruct.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStruct.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStruct.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStruct.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStruct.I2S_Standard = I2S_Standard_LSB;
	I2S_Init(SPI3, &I2S_InitStruct);
}

int dac_switch_samplerate(int val)
{
	p_dbg("dac change samplerate:%d\n", val);
	I2S_Cmd(SPI3, DISABLE);
	i2s_config(val);
	I2S_Cmd(SPI3, ENABLE);
	return 0;
}

int dac_low_level_open(void)
{

	memset(&dec_input, 0, sizeof(struct AUDIO_DEC_INPUT));
	memset(&dec_output, 0, sizeof(struct AUDIO_DEC_OUTPUT));

	if(!dec_output.data)
	{
		dec_output.data = (u8_t*)mem_calloc(AUDIO_DMA_BUFF_SIZE, 1);
		if(!dec_output.data)
		{
			goto err;
		}
	}

	if(!dec_input.data)
	{
		dec_input.data = (u8_t*)mem_calloc(AUDIO_DAC_INPUT_BUFF_SIZE, 1);
		if(!dec_input.data)
		{
			goto err;
		}
	}
		
	dec_output.len = AUDIO_DMA_BUFF_SIZE;
	dec_output.play_pos = dec_output.write_pos = 0;
	dec_output.dac_tx_waitq = init_event();
	
	dec_input.buff_len = AUDIO_DAC_INPUT_BUFF_SIZE;
	dec_input.data_len = 0;
	dec_input.old_type = dec_input.type = AUDIO_TYPE_UNKOWN;

//   @note   This function must be called before enabling the I2S APB clock.
	RCC_I2SCLKConfig(RCC_I2S2CLKSource_PLLI2S);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	RCC_PLLI2SCmd(ENABLE);
	//while(RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY) != SET);
	
	
	GPIO_PinAFConfig(I2S_WS_PORT, I2S_WS_SRC, GPIO_AF_SPI3); 
	GPIO_PinAFConfig(I2S_MCK_PORT , I2S_MCK_SRC, GPIO_AF_SPI3); 
	GPIO_PinAFConfig(I2S_SCK_PORT, I2S_SCK_SRC, GPIO_AF_SPI3); 
	GPIO_PinAFConfig(I2S_SD_PORT, I2S_SD_SRC, GPIO_AF_SPI3);

	gpio_cfg((uint32_t)I2S_WS_PORT, I2S_WS_PIN, GPIO_Mode_AF_PP); 
	gpio_cfg((uint32_t)I2S_MCK_PORT , I2S_MCK_PIN, GPIO_Mode_AF_PP); 
	gpio_cfg((uint32_t)I2S_SCK_PORT, I2S_SCK_PIN, GPIO_Mode_AF_PP); 
	gpio_cfg((uint32_t)I2S_SD_PORT, I2S_SD_PIN, GPIO_Mode_AF_PP);

	//CTROL PIN
	gpio_cfg((uint32_t)PCM1754_FMT_PORT, PCM1754_FMT_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)PCM1754_MUTE_PORT, PCM1754_MUTE_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)PCM1754_DEMP_PORT, PCM1754_DEMP_PIN, GPIO_Mode_Out_PP);

	gpio_cfg((uint32_t)DAC1_PORT, DAC1_PIN, GPIO_Mode_IN_FLOATING);//DAC1
	gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_IN_FLOATING);//DAC2

	//PCM1754_I2S_FORMAT;
	PCM1754_RIGHT_JUSTIFIED_FORMAT;
	PCM1754_MUTE_OFF;
	PCM1754_DEMP_OFF;

	i2s_config(I2S_AudioFreq_32k);

	DMA_I2S3configuration((uint32_t *)dec_output.data, AUDIO_DMA_BUFF_SIZE);
/*
	NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SPI3_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_ERR, ENABLE);
	SPI_I2S_ITConfig(SPI3, SPI_I2S_IT_TXE, ENABLE);
*/
	SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);

	I2S_Cmd(SPI3, ENABLE);

	return 0;
err:
	if(dec_input.data)
		mem_free(dec_input.data);
	
	if(dec_output.data)
		mem_free(dec_output.data);

	dec_input.data = 0;
	dec_output.data = 0;
	return -1;
}

void dac_low_level_close(void)
{
	I2S_Cmd(SPI3, DISABLE);
	SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, DISABLE);
}

static void DMA_I2S3configuration(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_HTIF5);
	
	DMA_Cmd(DMA1_Stream5, DISABLE);

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(SPI3->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BufferSize/2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	////使能出错中断,半完成和完成中断
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC|DMA_IT_TE|DMA_IT_HT, ENABLE);  //使能传输完成和出错中断

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_Stream5_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA1_Stream5, ENABLE);
}

DECLARE_MONITOR_ITEM("DMA1_Stream5_IRQHandler_cnt", DMA1_Stream5_IRQHandler_cnt) ;

void DMA1_Stream5_IRQHandler()  //dac1
{	
	enter_interrupt();

	INC_MONITOR_ITEM_VALUE(DMA1_Stream5_IRQHandler_cnt);

	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
	{
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		//if(audio_cfg.pcm->sync)
		{
			dec_output.play_circle_cnt++;
			dec_output.int_flag = 1;
			dec_output.write_pos = 0;
			if(dec_output.data)
			{
				int i;
				uint16_t last_value = *((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE/2 - 2));
				for(i = 0; i < AUDIO_DMA_BUFF_SIZE/4; i++)
					*((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE/2 + i*2)) = last_value;
				//memset(dec_output.data + AUDIO_DMA_BUFF_SIZE/2, 0, AUDIO_DMA_BUFF_SIZE/2);
			}
			if(dec_output.need_wait)
				goto end_and_sched;
		}
		
	}

	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5))
	{
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
		//if(audio_cfg.pcm->sync)
		{
			dec_output.play_circle_cnt++;
			dec_output.int_flag = 0;
			dec_output.write_pos = 0;
			if(dec_output.data){
				//memset(dec_output.data, 0, AUDIO_DMA_BUFF_SIZE/2);
				int i;
				uint16_t last_value = *((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE - 2));
				for(i = 0; i < AUDIO_DMA_BUFF_SIZE/4; i++)
					*((uint16_t*)(dec_output.data + i*2)) = last_value;

			}
			
			if(dec_output.need_wait)
				goto end_and_sched;
		}
	}

	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TEIF5))
	{
		DMA_ClearFlag(DMA1_Stream5,  DMA_IT_TEIF5);
		p_err("DMA_IT_TEIF5");
	}
	
	exit_interrupt(0);
	return;
	
end_and_sched:
	wake_up(dec_output.dac_tx_waitq);
	exit_interrupt(1);
}

int test = 0;
void SPI3_IRQHandler()
{
	if(SPI_GetITStatus(SPI3, SPI_I2S_IT_TXE))
	{
		SPI_ClearFlag(SPI3,  SPI_I2S_IT_TXE);
		//SPI_I2S_SendData(SPI3, test++);
	}
	
}
#else

static void DMA_DAC1Configuration(uint32_t *BufferSRC, uint32_t BufferSize);
int dac_low_level_open(void)
{
	DAC_InitTypeDef DAC_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_ClocksTypeDef RCC_ClocksStatus;
	
	uint32_t apbclock, Counter;
	p_dbg_enter;
	DAC_DeInit();

	memset(&dec_input, 0, sizeof(struct AUDIO_DEC_INPUT));
	memset(&dec_output, 0, sizeof(struct AUDIO_DEC_OUTPUT));

	if(!dec_output.data)
	{
		dec_output.data = (u8_t*)mem_calloc(AUDIO_DMA_BUFF_SIZE, 1);
		if(!dec_output.data)
		{
			goto err;
		}
	}

	if(!dec_input.data)
	{
		dec_input.data = (u8_t*)mem_calloc(AUDIO_DAC_INPUT_BUFF_SIZE, 1);
		if(!dec_input.data)
		{
			goto err;
		}
	}
		
	dec_output.len = AUDIO_DMA_BUFF_SIZE;
	dec_output.play_pos = dec_output.write_pos = 0;
	dec_output.dac_tx_waitq = init_event();
	
	dec_input.buff_len = AUDIO_DAC_INPUT_BUFF_SIZE;
	dec_input.data_len = 0;
	dec_input.old_type = dec_input.type = AUDIO_TYPE_UNKOWN;
	
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	Counter = apbclock/DEFAULT_PLAY_SAMPLERATE;

	p_dbg("set samplerate:%d, conter:%d, apbclock:%d\n", DEFAULT_PLAY_SAMPLERATE, Counter, apbclock);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

//dac
	gpio_cfg((uint32_t)DAC1_PORT, DAC1_PIN, GPIO_Mode_AIN);//DAC1
	gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_AIN);//DAC2

	DAC_InitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStruct.DAC_Trigger = DAC_Trigger_T4_TRGO;
	DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_Init(DAC_Channel_1, &DAC_InitStruct);

	DAC_InitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_InitStruct.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_Init(DAC_Channel_2, &DAC_InitStruct);
	
	DMA_DAC1Configuration((uint32_t *)dec_output.data, AUDIO_DMA_BUFF_SIZE);
	DAC_ITConfig(DAC_Channel_1, DAC_IT_DMAUDR, ENABLE);
	DAC_ITConfig(DAC_Channel_2, DAC_IT_DMAUDR, ENABLE);

//timer
	TIM_TimeBaseStructure.TIM_Period = 1250;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM4, TIM_TRGOSource_Update);
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	#ifdef TEST_DAC
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM4_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM4_IRQn);
	#endif
	
	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);
	
	DAC_DMACmd(DAC_Channel_1, ENABLE);

	TIM_Cmd(TIM4, ENABLE);
	return 0;

err:
	if(dec_input.data)
		mem_free(dec_input.data);
	
	if(dec_output.data)
		mem_free(dec_output.data);

	dec_input.data = 0;
	dec_output.data = 0;
	return -1;
}

int dac_switch_samplerate(int val)
{
	RCC_ClocksTypeDef RCC_ClocksStatus;

	uint32_t apbclock, scaler = 0, Counter;
	p_dbg("dac change samplerate:%d\n", val);
	if(val == 0)
		return -1;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	

	Counter = apbclock*2/(scaler + 1)/val;

	TIM_SetAutoreload(TIM4, Counter);
	return 0;
}

void dac_low_level_close(void)
{
//dac 恢复为IO口
	gpio_cfg((uint32_t)DAC1_PORT, DAC1_PIN, GPIO_Mode_Out_PP);//
	gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_Out_PP);//

	DAC_DMACmd(DAC_Channel_1, DISABLE);
	DAC_ITConfig(DAC_Channel_1, DAC_IT_DMAUDR, DISABLE);
	DAC_Cmd(DAC_Channel_1, DISABLE);
	DAC_Cmd(DAC_Channel_2, DISABLE);
	
//timer

	TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	TIM_Cmd(TIM4, DISABLE);
#ifdef TEST_DAC
	NVIC_DisableIRQ(TIM4_IRQn);
#endif
	NVIC_DisableIRQ(DMA1_Stream5_IRQn);

	DMA_Cmd(DMA1_Stream5, DISABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM4, DISABLE);

	if(dec_input.data)
		mem_free(dec_input.data);
	
	if(dec_output.data)
		mem_free(dec_output.data);

	dec_input.data = 0;
	dec_output.data = 0;

}

void dac_close_channel(int channel)
{
	if(channel == 1)
		gpio_cfg((uint32_t)DAC1_PORT, DAC1_PIN, GPIO_Mode_IN_FLOATING);//DAC1
	else if(channel == 2)
		gpio_cfg((uint32_t)DAC2_PORT, DAC2_PIN, GPIO_Mode_IN_FLOATING);//DAC2
}

			  
static void DMA_DAC1Configuration(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_HTIF5);
	
	DMA_Cmd(DMA1_Stream5, DISABLE);

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_7;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(DAC->DHR12RD);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = BufferSize/4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	////使能出错中断,半完成和完成中断
	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC|DMA_IT_TE|DMA_IT_HT, ENABLE);  //使能传输完成和出错中断

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA1_Stream5_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA1_Stream5, ENABLE);
}

DECLARE_MONITOR_ITEM("DMA1_Stream5_IRQHandler_cnt", DMA1_Stream5_IRQHandler_cnt) ;

void DMA1_Stream5_IRQHandler()  //dac1
{	
	enter_interrupt();

	INC_MONITOR_ITEM_VALUE(DMA1_Stream5_IRQHandler_cnt);

	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5))
	{
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
		//if(audio_cfg.pcm->sync)
		{
			dec_output.play_circle_cnt++;
			dec_output.int_flag = 1;
			dec_output.write_pos = 0;
			if(dec_output.data)
			{
				int i;
				uint16_t last_value = *((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE/2 - 2));
				for(i = 0; i < AUDIO_DMA_BUFF_SIZE/4; i++)
					*((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE/2 + i*2)) = last_value;
				//memset(dec_output.data + AUDIO_DMA_BUFF_SIZE/2, 0, AUDIO_DMA_BUFF_SIZE/2);
			}
			if(dec_output.need_wait)
				goto end_and_sched;
		}
		
	}

	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5))
	{
		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
		//if(audio_cfg.pcm->sync)
		{
			dec_output.play_circle_cnt++;
			dec_output.int_flag = 0;
			dec_output.write_pos = 0;
			if(dec_output.data){
				//memset(dec_output.data, 0, AUDIO_DMA_BUFF_SIZE/2);
				int i;
				uint16_t last_value = *((uint16_t*)(dec_output.data + AUDIO_DMA_BUFF_SIZE - 2));
				for(i = 0; i < AUDIO_DMA_BUFF_SIZE/4; i++)
					*((uint16_t*)(dec_output.data + i*2)) = last_value;

			}
			
			if(dec_output.need_wait)
				goto end_and_sched;
		}
	}

	if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TEIF5))
	{
		DMA_ClearFlag(DMA1_Stream5,  DMA_IT_TEIF5);
		p_err("DMA_IT_TEIF5");
	}
	
	exit_interrupt(0);
	return;
	
end_and_sched:
	wake_up(dec_output.dac_tx_waitq);
	exit_interrupt(1);
}

DECLARE_MONITOR_ITEM("DMA1_Stream6_IRQHandler_cnt", DMA1_Stream6_IRQHandler_cnt) ;


void DMA1_Stream6_IRQHandler()  //dac2
{
	enter_interrupt();

	INC_MONITOR_ITEM_VALUE(DMA1_Stream6_IRQHandler_cnt);

	DMA_ClearFlag(DMA1_Stream6,  DMA_FLAG_TEIF6);
	DMA_ClearFlag(DMA1_Stream6,  DMA_FLAG_TCIF6);

 	exit_interrupt(0);
}

#endif

int adc_low_level_open(void)
{
	int ret;
	NVIC_InitTypeDef NVIC_InitStructure;
	ADC_InitTypeDef ADC_InitStruct;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	p_dbg_enter;
	
	ADC_DeInit();

	memset(&adc, 0, sizeof(struct AUDIO_ADC));

	adc.data = (uint8_t*)mem_malloc(AUDIO_ADC_BUFF_SIZE);
	if(!adc.data)
		goto end;

	adc.len = AUDIO_ADC_BUFF_SIZE;

	adc.block_data = (uint8_t*)mem_malloc(PCM_HEAD_LEN + adc.len/8);
	if(!adc.block_data)
		goto end;

	adc_rx_wait = init_event();
	if(adc_rx_wait == 0)
	{
		p_err("adc_rx_wait ==0\r\n");
		goto end;
	}
	
	thread_create(adc_recv_thread,
                        0,
                        TASK_ADC_RECV_PKG_PRIO,
                        0,
                        TASK_ADC_RECV_PKG_STACK_SIZE,
                        "adc recv");
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC | RCC_APB2Periph_ADC1, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	gpio_cfg((uint32_t)ADC_PORT, ADC_PIN, GPIO_Mode_AIN);//ADC123_IN3

	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //不能为none
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1, &ADC_InitStruct);

	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADC_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 1, ADC_SampleTime_84Cycles);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_Cmd(ADC1, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 1250;
    	TIM_TimeBaseStructure.TIM_Prescaler = 0;	
    	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

     	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
	TIM_ARRPreloadConfig(TIM3, ENABLE);
	
	#ifdef TEST_DAC
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	//ADC_SoftwareStartConv(ADC1);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = TIM3_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(TIM3_IRQn);
	#endif

	DMA_ADC1Configuration((uint32_t*)adc.data, adc.len);

	TIM_Cmd(TIM3, ENABLE);

	ADC_DMACmd(ADC1, ENABLE);

	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

	return 0;

end:

	p_err_fun;
	
	ret = thread_exit(TASK_ADC_RECV_PKG_PRIO);
	if(ret != 0)
		p_err("thread_exit (TASK_ADC_RECV_PKG_PRIO) err:%d", ret);

	if(adc.data)
		mem_free(adc.data);

	if(adc.block_data)
		mem_free(adc.block_data);

	adc.block_data = 0;
	adc.data = 0;
		
	return -1;
}

void adc_low_level_close(void)
{
	int ret;
	p_dbg_enter;
	gpio_cfg((uint32_t)ADC_PORT, ADC_PIN, GPIO_Mode_Out_PP);//ADC123_IN0

	ADC_DMACmd(ADC1, DISABLE);

	ADC_Cmd(ADC1, DISABLE);
	TIM_Cmd(TIM3, DISABLE);

	DMA_Cmd(DMA2_Stream0, DISABLE);
	NVIC_DisableIRQ(DMA2_Stream0_IRQn);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC | RCC_APB2Periph_ADC1, DISABLE);

	ret = thread_exit(TASK_ADC_RECV_PKG_PRIO);
	if(ret != 0)
		p_err("thread_exit (TASK_ADC_RECV_PKG_PRIO) err:%d", ret);

	if(adc.data)
		mem_free(adc.data);
	if(adc.block_data)
		mem_free(adc.block_data);

	adc.block_data = 0;
	adc.data = 0;
}

int adc_switch_samplerate(int val)
{
	RCC_ClocksTypeDef RCC_ClocksStatus;

	uint32_t apbclock, scaler = 0, Counter;
	p_dbg("adc change samplerate:%d\n", val);
	if(val == 0)
		return -1;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	
	Counter = apbclock*2/(scaler + 1)/val;

	TIM_SetAutoreload(TIM3, Counter);
	return 0;
}


#ifdef TEST_DAC

void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
     TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}


void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		}
}

void ADC_IRQHandler(void)
{
	if(ADC_GetITStatus(ADC1, ADC_IT_EOC))
	{
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	}
}

#endif


static void DMA_ADC1Configuration(uint32_t *BufferDST, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream0, DMA_FLAG_TCIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_HTIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_HTIF0);
   
	DMA_Cmd(DMA2_Stream0, DISABLE);

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BufferSize/2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   //循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC|DMA_IT_HT|DMA_IT_TE, ENABLE); 

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA2_Stream0_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream0, ENABLE);

}

void audio_button_event(int event)
{
	if(event == SWITCH_EVENT_ON)
		audio_record_enable = TRUE;
	else
		audio_record_enable = FALSE;
}


DECLARE_MONITOR_ITEM("DMA2_Stream0_IRQHandler_cnt", DMA2_Stream0_IRQHandler_cnt) ;

//import from test_tcpip.c
extern void send_audio_data_to_remote(char *buff, int len);
DECLARE_MONITOR_ITEM("pcm_data_block_out_cnt", pcm_data_block_out_cnt) ;
DECLARE_MONITOR_ITEM("adc_handle_cnt", adc_handle_cnt) ;

void adc_recv_thread(void *arg)
{
	int ret, i;
	int16_t *src_data;
	uint8_t *out_data;
	struct DATA_CHUNK *chunk;

	p_dbg_enter;
	
	while(1)
	{
		ret = wait_event_timeout(adc_rx_wait, 2000);
		if(ret == WAIT_EVENT_TIMEOUT)
		{
			continue;
		}
		
		if(adc.int_flag == 0)
			src_data = (int16_t*)adc.data;
		else
			src_data = (int16_t*)(adc.data + adc.len/2);

		adc.pending_flag = adc.int_flag;

		for(i = 0; i < adc.len/4; i++)
		{
			src_data[i]  = (src_data[i] - 2048)*(32768/2048);  //转换成有符号数并放大成16位
		}

		out_data = adc.block_data;

		if(!out_data)
			continue;

		chunk = (struct DATA_CHUNK *)out_data;
		memcpy(chunk->fccID, "data", 4);
		chunk->dwSize = adc.len/8;	//256

		chunk->pre_val = adc.stat.valprev;
		chunk->index = adc.stat.index;
		INC_MONITOR_VALUE(adc_handle_cnt);
		adpcm_coder((short*)src_data, (char*)(out_data + PCM_HEAD_LEN), adc.len/4, &adc.stat);

		//p_hex(out_data, PCM_HEAD_LEN + adc.len/8);

		if(!BUTTON_STAT)
		{
			send_audio_data_to_remote((char*)out_data, PCM_HEAD_LEN + adc.len/8);
			INC_MONITOR_VALUE(pcm_data_block_out_cnt);
		}
	}
}

DECLARE_MONITOR_ITEM("adc_sample_cnt", adc_sample_cnt) ;
void DMA2_Stream0_IRQHandler()  //adc1
{
	enter_interrupt();

	INC_MONITOR_VALUE(DMA2_Stream0_IRQHandler_cnt);


	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0))
	{
		DMA_ClearFlag(DMA2_Stream0,  DMA_IT_HTIF0);
		adc.int_flag = 0;
		goto end_and_sched;
	}

	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
	{
		DMA_ClearFlag(DMA2_Stream0,  DMA_IT_TCIF0);
		adc.int_flag = 1;

		goto end_and_sched;
	}

	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TEIF0))
	{
		DMA_ClearFlag(DMA2_Stream0,  DMA_IT_TEIF0);
		p_err("DMA_IT_TEIF0");
	}

	exit_interrupt(0);
	return;
end_and_sched:
	INC_MONITOR_VALUE(adc_sample_cnt);
	wake_up(adc_rx_wait);
	exit_interrupt(1);
}


#endif


