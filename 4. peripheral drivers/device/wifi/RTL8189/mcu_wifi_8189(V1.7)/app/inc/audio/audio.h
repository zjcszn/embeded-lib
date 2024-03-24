#ifndef _AUDIO_H
#define _AUDIO_H

#include "api.h"

#define AUDIO_TYPE_PCM			0
#define AUDIO_TYPE_ADPCM		1
#define AUDIO_TYPE_MP3 			2
#define AUDIO_TYPE_UNKOWN 		3

#define MAX_BLANK_BYTES				256

#define DEFAULT_AUDIO_NET_PORT		4700

#define AUDIO_DMA_BUFF_SIZE 		8192

#define AUDIO_DAC_INPUT_BUFF_SIZE  	2048

#define AUDIO_ADC_BUFF_SIZE  		2048

struct AUDIO_DEC_INPUT
{
	uint8_t old_type;
	uint8_t type;
	uint8_t bits;
	uint8_t channels;
	unsigned int samplerate;
	uint8_t *data;
	uint32_t data_len;
	uint32_t buff_len;
};

//dec输出缓存,同时也是dac输入缓存
struct AUDIO_DEC_OUTPUT
{
	uint32_t len;
	uint32_t write_circle_cnt;
	uint32_t play_circle_cnt;
	uint32_t int_flag;
	uint32_t need_wait;
	uint16_t write_pos;
	uint16_t play_pos;
	wait_event_t dac_tx_waitq;
	uint8_t *data;

};

struct AUDIO_ADC
{
	uint8_t type;
	volatile uint8_t int_flag;
	volatile uint8_t pending_flag;
	unsigned int samplerate;
	struct adpcm_state stat;
	uint32_t len;
	uint8_t *data;
	uint8_t *block_data;
};

struct MP3_HEADER
{
	unsigned int sync: 11; //同步信息
	unsigned int version: 2; //版本
	unsigned int layer: 2; //层
	unsigned int protection: 1; // CRC校验
	unsigned int bitrate_index: 4; //位率
	unsigned int sampling_frequency: 2; //采样频率
	unsigned int padding: 1; //帧长调节
unsigned int private:
	1; //保留字
	unsigned int mode: 2; //声道模式
	unsigned int mode_extension: 2; //扩充模式
	unsigned int copyright: 1; // 版权
	unsigned int original: 1; //原版标志
	unsigned int emphasis: 2; //强调模式
};



struct AUDIO_CFG
{
	uint8_t audio_dev_open;
	uint8_t volume; // 0 -100
	BOOL play_mp3;
	uint16_t audio_net_port;
	int audio_socket_num;
	struct AUDIO_DEC_INPUT *dec_input;
	struct AUDIO_DEC_OUTPUT *dec_output;
	struct WAVE_FORMAT_HEAD *pcm;
	struct AUDIO_ADC *adc;
};

extern struct AUDIO_CFG audio_cfg;

void push_to_play_buff(uint8_t *val, int size);
void audio_dev_init(void);
int audio_dev_open(void);
int audio_dev_close(void);
int handle_audio_stream(unsigned char *buff, int size);
void switch_audio_mode(void);
#endif
