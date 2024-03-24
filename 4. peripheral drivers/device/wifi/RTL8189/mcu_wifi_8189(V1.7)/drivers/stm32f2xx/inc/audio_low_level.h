#ifndef _AUDIO_LOW_H
#define _AUDIO_LOW_H


//使用片内DAC或者片外I2S开关
#define USE_I2S_MODE	1


#define AUDIO_DAC_DMA 	DMA1_Stream5

#define AUDIO_SAMPLERATE32000			32000
#define AUDIO_SAMPLERATE44100			44100
#define AUDIO_SAMPLERATE48000			48000

#define DEFAULT_PLAY_SAMPLERATE		AUDIO_SAMPLERATE32000
#define DEFAULT_RECORD_SAMPLERATE	AUDIO_SAMPLERATE32000

#define DAC1_PORT 	GPIOA
#define DAC2_PORT 	GPIOA
#define ADC_PORT 	GPIOA

#define DAC1_PIN 	GPIO_Pin_4
#define DAC2_PIN 	GPIO_Pin_5

#define ADC_PORT 	GPIOA
#define ADC_PIN 		GPIO_Pin_3


#define I2S_WS_PORT 		GPIOA
#define I2S_MCK_PORT 	GPIOC
#define I2S_SCK_PORT 	GPIOC
#define I2S_SD_PORT		GPIOB

#define I2S_WS_PIN 		GPIO_Pin_15
#define I2S_MCK_PIN 		GPIO_Pin_7
#define I2S_SCK_PIN		GPIO_Pin_10
#define I2S_SD_PIN		GPIO_Pin_5

#define I2S_WS_SRC 		GPIO_PinSource15
#define I2S_MCK_SRC		GPIO_PinSource7
#define I2S_SCK_SRC		GPIO_PinSource10
#define I2S_SD_SRC		GPIO_PinSource5

#define PCM1754_FMT_PORT 			GPIOC
#define PCM1754_MUTE_PORT 		GPIOC
#define PCM1754_DEMP_PORT 		GPIOA

#define PCM1754_FMT_PIN		GPIO_Pin_5
#define PCM1754_MUTE_PIN	GPIO_Pin_4
#define PCM1754_DEMP_PIN	GPIO_Pin_7


//格式选择
#define PCM1754_RIGHT_JUSTIFIED_FORMAT	GPIO_SET(PCM1754_FMT_PORT, PCM1754_FMT_PIN)
#define PCM1754_I2S_FORMAT					GPIO_CLR(PCM1754_FMT_PORT, PCM1754_FMT_PIN)

//静音
#define PCM1754_MUTE_ON	GPIO_SET(PCM1754_MUTE_PORT, PCM1754_MUTE_PIN)
#define PCM1754_MUTE_OFF	GPIO_CLR(PCM1754_MUTE_PORT, PCM1754_MUTE_PIN)

//重低音
#define PCM1754_DEMP_ON	GPIO_SET(PCM1754_DEMP_PORT, PCM1754_DEMP_PIN)
#define PCM1754_DEMP_OFF	GPIO_CLR(PCM1754_DEMP_PORT, PCM1754_DEMP_PIN)

int dac_low_level_open(void);
void dac_low_level_close(void);
int adc_low_level_open(void);
void adc_low_level_close(void);
int dac_switch_samplerate(int val);
int adc_switch_samplerate(int val);
void dac_close_channel(int channel);
void audio_button_event(int event);

#endif

