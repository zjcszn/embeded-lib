#define DEBUG

#include "drivers.h"
#include "api.h"
#include "app.h"


#include "audio/adpcm.h"
#include "audio/audio.h"
#include "debug.h"
#include "mad.h"

#if SUPPORT_AUDIO


extern struct WAVE_FORMAT_HEAD pcm_format;
#if ENABLE_MP3
extern struct MP3_FORMAT mp3_format;
#endif
struct AUDIO_CFG audio_cfg =
{
	0, 0,
};

static int play_pcm_stream(uint8_t *buff, int size);
static int audio_dev_write(uint8_t *buff, int size);

extern struct AUDIO_DEC_INPUT dec_input;
extern struct AUDIO_DEC_OUTPUT dec_output;
extern struct AUDIO_ADC adc;

/*
初始化
 */
void audio_dev_init()
{
	memset(&audio_cfg, 0, sizeof(struct AUDIO_CFG));
	memset(&pcm_format, 0, sizeof(struct WAVE_FORMAT_HEAD));
#if ENABLE_MP3
	memset(&mp3_format, 0, sizeof(struct MP3_FORMAT));
#endif
	pcm_format.sync = false;
	audio_cfg.audio_net_port = DEFAULT_AUDIO_NET_PORT;
	audio_cfg.pcm = &pcm_format;

	audio_cfg.dec_input = &dec_input;
	audio_cfg.dec_output = &dec_output;
	audio_cfg.adc = &adc;
	audio_cfg.volume = 100;
	audio_cfg.audio_socket_num =  - 1;

	return ;
}


/*
打开音频驱动
 */
int audio_dev_open()
{
	p_dbg_enter;
	audio_cfg.audio_dev_open = 0;

	if (dac_low_level_open())
	{
		p_err("%s err", __FUNCTION__);
		return  - 1;
	}
	if (adc_low_level_open())
	{
		p_err("%s err1", __FUNCTION__);
		return  - 1;
	}

	audio_cfg.audio_dev_open = 1;

	return 0;
}



/*
关闭音频驱动
 */
int audio_dev_close()
{
	audio_cfg.audio_dev_open = 0;

	dac_low_level_close();
	adc_low_level_close();
	return 0;
}


int handle_audio_stream(unsigned char *buff, int size)
{
	return audio_dev_write(buff, size);
}

static int audio_dev_write(uint8_t *buff, int size)
{
	int i = 0;

	if (!buff || !size)
		return  - 1;

	if (!audio_cfg.audio_dev_open || !audio_cfg.dec_input)
		return  - 1;
#if ENABLE_MP3
	if(audio_cfg.play_mp3)
	{
		i = play_mp3_stream(buff, size);
	}
	else
#endif
	{
		i = play_pcm_stream(buff, size);
	}

	if (i != 0)
	{
		//p_err("play ret err");
		audio_cfg.dec_input->type = AUDIO_TYPE_UNKOWN;
	}

	return 0;
}


int play_pcm_stream(uint8_t *buff, int size)
{
	if (size > 1024 || !audio_cfg.dec_input)
	//size 肯定 <= 1024
		return  - 1;

	if (audio_cfg.dec_input->data_len + size <= audio_cfg.dec_input->buff_len)
	{
		memcpy(audio_cfg.dec_input->data + audio_cfg.dec_input->data_len, buff, size);
		audio_cfg.dec_input->data_len += size;
	}
	else
	{
		audio_cfg.dec_input->data_len = 0;
	}

	return do_play_pcm_stream();
}

int play_mp3_stream(uint8_t *buff, int size)
{
	int ret;
	if(
#if ENABLE_MP3
	!mp3_format.init || 
#endif
	!buff)
		return -1;

	if(dec_input.data_len + size <=  dec_input.buff_len)
	{
		memcpy(dec_input.data + dec_input.data_len, buff, size);
		dec_input.data_len += size;
	}else{
		p_err("%s,%d,%d\n", __FUNCTION__, audio_cfg.dec_input->data_len, size);
		dec_input.data_len = 0;
	}
	#if ENABLE_MP3
	ret = MpegAudioDecoder(&dec_input);
	#else
	ret = 0;
	#endif
	return ret;
}


void switch_audio_mode()
{
	if(!audio_cfg.audio_dev_open)
	{
		p_dbg("请先打开音频");
		return;
	}
	audio_cfg.play_mp3 = !audio_cfg.play_mp3;
	if(audio_cfg.play_mp3){

		p_dbg("mp3播放模式");
#if ENABLE_MP3
		if(!mp3_format.init)
		{
			if(init_mp3_format())
				p_err("init_mp3_format err\n");
		}
		reset_mp3_stat();
#endif
	}else{
		p_dbg("pcm播放模式(语音对讲)");
#if ENABLE_MP3
		deinit_mp3_format();
#endif
	}
}

DECLARE_MONITOR_ITEM("pcm_push_cnt", pcm_push_cnt);
//拷贝到dac发送缓冲区,如满了则延时等待
void push_to_play_buff(uint8_t *val, int size)
{
	int ret;
	uint32_t irq_flg;
	if (!val || !size || !audio_cfg.dec_output)
		return ;

	if (!audio_cfg.audio_dev_open || !audio_cfg.dec_output->data)
	{
		return ;
	}

	again: irq_flg = local_irq_save();
	dec_output.need_wait = 0;
	if (dec_output.int_flag < 2)
	{
		if ((dec_output.write_pos + size) <= AUDIO_DMA_BUFF_SIZE / 2)
		{

			memcpy(dec_output.data + dec_output.int_flag *(AUDIO_DMA_BUFF_SIZE / 2) + dec_output.write_pos, val, size);
			dec_output.write_pos += size;
			INC_MONITOR_VALUE(pcm_push_cnt);
		}
		else
		{
			clear_wait_event(dec_output.dac_tx_waitq);
			dec_output.need_wait = 1;
		}
	}
	local_irq_restore(irq_flg);

	if (dec_output.need_wait)
	{
		ret = wait_event_timeout(dec_output.dac_tx_waitq, 2000);
		if (ret == WAIT_EVENT_TIMEOUT)
		{
			p_err("dac_tx_waitq timeout");
		}
		else
		{
			goto again;
		}
	}

}
#endif

