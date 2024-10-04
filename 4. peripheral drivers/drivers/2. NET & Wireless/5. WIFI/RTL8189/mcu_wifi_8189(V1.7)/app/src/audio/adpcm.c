#define DEBUG
#include "audio/adpcm.h"
#include "audio/audio.h"
#include "debug.h"

#if SUPPORT_AUDIO

struct WAVE_FORMAT_HEAD pcm_format;
extern struct AUDIO_CFG audio_cfg;

DECLARE_MONITOR_ITEM("pcm_data_block_in_cnt", pcm_data_block_cnt);
int parse_pcm_head(char *data)
{
	if (data[0] == 'd' && data[1] == 'a' && data[2] == 't' && data[3] == 'a')
	{
		struct DATA_CHUNK *chunk = (struct DATA_CHUNK*)data;

		pcm_format.block_size = chunk->dwSize;
		pcm_format.sync = 1;
		pcm_format.stat.valprev = chunk->pre_val;
		pcm_format.stat.index = chunk->index;
		INC_MONITOR_VALUE(pcm_data_block_cnt);
		return 1;
	}

	return 0;
}


int do_play_pcm_stream()
{

	int i, len;
	int16_t *out,  *stereo_out;
#if USE_I2S_MODE
	int volume_div = (100 - audio_cfg.volume)/10;
#endif
	if (!audio_cfg.dec_input || !audio_cfg.dec_input->data)
		return  - 1;

	out = (int16_t*)mem_malloc(ONCE_PUSH_SIZE *3 * sizeof(int16_t));

	if (!out)
		return  - 1;

	stereo_out = &out[ONCE_PUSH_SIZE];
	again: 
	if (!pcm_format.sync)
	{
		if (audio_cfg.dec_input->data_len < PCM_HEAD_LEN)
		{

			mem_free(out);
			return 0;
		}

		for (i = 0; i < (audio_cfg.dec_input->data_len - PCM_HEAD_LEN); i++)
		{
			if (parse_pcm_head((char*)audio_cfg.dec_input->data + i))
			{
				//if ((i + 8) < audio_cfg.dec_input->data_len)
				{
					memmove(audio_cfg.dec_input->data, audio_cfg.dec_input->data + i + PCM_HEAD_LEN, audio_cfg.dec_input->data_len - i - PCM_HEAD_LEN);

					audio_cfg.dec_input->data_len = audio_cfg.dec_input->data_len - i - PCM_HEAD_LEN;

					break;
				}
			}
		}
	}

	if (pcm_format.sync)
	{
		if (audio_cfg.dec_input->data_len >= pcm_format.block_size)
		{
			len = pcm_format.block_size / ONCE_DECODE_SIZE;
			for (i = 0; i < len; i++)
			{
				int j;
				adpcm_decoder((char*)audio_cfg.dec_input->data + i * ONCE_DECODE_SIZE, (short*)out, ONCE_DECODE_SIZE, &pcm_format.stat);

				for (j = 0; j < ONCE_PUSH_SIZE; j++)
				{
#if USE_I2S_MODE
					stereo_out[j *2+1] = stereo_out[j *2] = out[j] >>volume_div;
#else
					stereo_out[j *2+1] = stereo_out[j *2] = out[j] *audio_cfg.volume / 100;
#endif
				}

				push_to_play_buff((uint8_t*)stereo_out, ONCE_PUSH_SIZE *2);
			}

			pcm_format.sync = 0;

			if (audio_cfg.dec_input->data_len > pcm_format.block_size)
			{

				//p_dbg("remain1:%d", audio_cfg.dec_input->data_len - i * ONCE_DECODE_SIZE);
				memmove(audio_cfg.dec_input->data, audio_cfg.dec_input->data + pcm_format.block_size, audio_cfg.dec_input->data_len - pcm_format.block_size);

				audio_cfg.dec_input->data_len -= pcm_format.block_size;

				goto again;
			}
			else
				audio_cfg.dec_input->data_len = 0;
		}
	}

	mem_free(out);
	return 0;
}


/* Intel ADPCM step variation table */
const short indexTable[16] =
{
	 - 1,  - 1,  - 1,  - 1, 2, 4, 6, 8,  - 1,  - 1,  - 1,  - 1, 2, 4, 6, 8,
};

// const short
const short stepsizeTable[89] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

void adpcm_coder(short *indata, char *outdata, long len, struct adpcm_state *state)
{
	short *inp; /* Input buffer pointer */
	signed char *outp; /* output buffer pointer */
	long val; /* Current input sample value */
	long sign; /* Current adpcm sign bit */
	long delta; /* Current adpcm output value */
	long step; /* Stepsize */
	long valprev; /* virtual previous output value */
	long vpdiff; /* Current change to valprev */
	long index; /* Current step change index */
	long outputbuffer = 0; /* place to keep previous 4-bit value */
	long bufferstep; /* toggle between outputbuffer/output */

	outp = (signed char*)outdata;
	inp = indata;

	valprev = state->valprev;
	index = state->index;
	step = stepsizeTable[index];

	bufferstep = 1;

	for (; len > 0; len--)
	{
		val =  *inp++;

		/* Step 1 - compute difference with previous value */
		delta = val - valprev;
		sign = (delta < 0) ? 8 : 0;
		if (sign)
			delta = ( - delta);

		/* Step 2 - Divide and clamp */
		#ifdef NODIVMUL
		{
			long tmp = 0;

			vpdiff = 0;
			if (delta > step)
			{
				tmp = 4;
				delta -= step;
				vpdiff = step;
			} step >>= 1;
			if (delta > step)
			{
				tmp |= 2;
				delta -= step;
				vpdiff += step;
			}
			step >>= 1;
			if (delta > step)
			{
				tmp |= 1;
				vpdiff += step;
			}
			delta = tmp;
		}
		#else
		delta = (delta << 2) / step;
		if (delta > 7)
			delta = 7;

		vpdiff = (delta *step) >> 2;
		#endif

		/* Step 3 - Update previous value */
		if (sign)
			valprev -= vpdiff;
		else
			valprev += vpdiff;

		/* Step 4 - Clamp previous value to 16 bits */
		if (valprev > 32767)
			valprev = 32767;
		else if (valprev <  - 32768)
			valprev =  - 32768;

		/* Step 5 - Assemble value, update index and step values */
		delta |= sign;

		index += indexTable[delta];
		if (index < 0)
			index = 0;
		if (index > 88)
			index = 88;
		step = stepsizeTable[index];

		/* Step 6 - Output value */
		if (bufferstep)
		{
			outputbuffer = (delta << 4) &0xf0;
		}
		else
		{
			*outp++ = (delta &0x0f) | outputbuffer;
		}
		bufferstep = !bufferstep;
	}

	/* Output last step, if needed */
	if (!bufferstep)
		*outp++ = outputbuffer;

	state->valprev = valprev;
	state->index = index;
}

void adpcm_decoder(char *indata, short *outdata, int len, struct adpcm_state *state)
{
	signed char *inp; /* Input buffer pointer */
	short *outp; /* output buffer pointer */
	long sign; /* Current adpcm sign bit */
	long delta; /* Current adpcm output value */
	long step; /* Stepsize */
	long valprev; /* virtual previous output value */
	long vpdiff; /* Current change to valprev */
	long index; /* Current step change index */
	long inputbuffer = 0; /* place to keep next 4-bit value */
	long bufferstep; /* toggle between inputbuffer/input */

	outp = outdata;
	inp = (signed char*)indata;

	valprev = state->valprev;
	index = state->index;
	step = stepsizeTable[index];

	bufferstep = 0;

	len*=2;

	for (; len > 0; len--)
	{

		/* Step 1 - get the delta value and compute next index */
		if (bufferstep)
		{
			delta = inputbuffer &0xf;
		}
		else
		{
			inputbuffer =  *inp++;
			delta = (inputbuffer >> 4) &0xf;
		}
		bufferstep = !bufferstep;

		/* Step 2 - Find new index value (for later) */
		index += indexTable[delta];
		if (index < 0)
		{
			index = 0;
		}
		else if (index > 88)
		{
			index = 88;
		}

		/* Step 3 - Separate sign and magnitude */
		sign = delta &8;
		delta = delta &7;

		/* Step 4 - update output value */
		#ifdef NODIVMUL
		vpdiff = 0;
		if (delta &4)
		{
			vpdiff = (step << 2);
		}
		if (delta &2)
		{
			vpdiff += (step << 1);
		}
		if (delta &1)
		{
			vpdiff += step;
		}
		vpdiff >>= 2;
		#else
		vpdiff = (delta *step) >> 2;
		#endif
		if (sign)
			valprev -= vpdiff;
		else
			valprev += vpdiff;

		/* Step 5 - clamp output value */
		if (valprev > 32767)
			valprev = 32767;
		else if (valprev <  - 32768)
			valprev =  - 32768;

		/* Step 6 - Update step value */
		step = stepsizeTable[index];

		/* Step 7 - Output value */
		*outp++ = valprev;
	}

	state->valprev = valprev;
	state->index = index;
}

#endif

