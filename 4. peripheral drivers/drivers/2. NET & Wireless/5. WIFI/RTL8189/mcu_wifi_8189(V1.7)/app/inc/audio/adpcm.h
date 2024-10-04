#ifndef _ADPCM_H
#define _ADPCM_H

#include "type.h"
// for ADPCM.c
#define LeftChannel 0
#define RightChannel 1

#define PCM_HEAD_MIN_SIZE 	64

#define ONCE_PUSH_SIZE 		32	
#define ONCE_DECODE_SIZE 	(ONCE_PUSH_SIZE/4)
#define PCM_HEAD_LEN 		sizeof(struct DATA_CHUNK) //"data" + size + pre_val	

/*sample
unsigned char RiffHeader[] = {
'R' , 'I' , 'F' , 'F' , // Chunk ID (RIFF)
0x70, 0x70, 0x70, 0x70, // Chunk payload size (calculate after rec!)
'W' , 'A' , 'V' , 'E' , // RIFF resource format type

'f' , 'm' , 't' , ' ' , // Chunk ID (fmt )
0x10, 0x00, 0x00, 0x00, // Chunk payload size (0x14 = 20 bytes)
0x01, 0x00,             // Format Tag ()
0x01, 0x00,             // Channels (1)
0x40, 0x1f, 0x00, 0x00, // Sample Rate,  = 16.0kHz
0x80, 0x3e, 0x00, 0x00, // Byte rate       32.0K
0x02, 0x00,             // BlockAlign == NumChannels * BitsPerSample/8
0x10, 0x00     // BitsPerSample
};


unsigned char RIFFHeader504[] = {
'd' , 'a' , 't' , 'a' , // Chunk ID (data)
0x70, 0x70, 0x70, 0x70  // Chunk payload size (calculate after rec!)
};*/


struct adpcm_state
{
	short valprev; /* Previous output value */
	char index; /* Index into stepsize table */
};

struct RIFF_CHUNK
{
	uint8_t sync;
	//  char            RIFF[4];                // must be "RIFF"
	unsigned long dwSize; // all bytes of the wave file subtracting 8,
	// which is the size of fccID and dwSize
	char fccType[4]; // must be "WAVE"
};

struct FACT_CHUNK
{
	char fccID[4]; // must be "fact"
	unsigned long id; // must be 0x4
	unsigned long dwSize; // 暂时没发现有啥用
};


#define wave_format_g723_adpcm	0x0014	/* antex electronics corporation */
#define wave_format_antex_adpcme	0x0033	/* antex electronics corporation */
#define wave_format_g721_adpcm	0x0040	/* antex electronics corporation */
#define wave_format_aptx	0x0025	/* audio processing technology */
#define wave_format_audiofile_af36	0x0024	/* audiofile, inc. */
#define wave_format_audiofile_af10	0x0026	/* audiofile, inc. */
#define wave_format_control_res_vqlpc	0x0034	/* control resources limited */
#define wave_format_control_res_cr10	0x0037	/* control resources limited */
#define wave_format_creative_adpcm	0x0200	/* creative labs, inc */
#define wave_format_dolby_ac2	0x0030	/* dolby laboratories */
#define wave_format_dspgroup_truespeech	0x0022	/* dsp group, inc */
#define wave_format_digistd	0x0015	/* dsp solutions, inc. */
#define wave_format_digifix	0x0016	/* dsp solutions, inc. */
#define wave_format_digireal	0x0035	/* dsp solutions, inc. */
#define wave_format_digiadpcm	0x0036	/* dsp solutions, inc. */
#define wave_format_echosc1	0x0023	/* echo speech corporation */
#define wave_format_fm_towns_snd	0x0300	/* fujitsu corp. */
#define wave_format_ibm_cvsd	0x0005	/* ibm corporation */
#define wave_format_oligsm	0x1000	/* ing c. olivetti & c., s.p.a. */
#define wave_format_oliadpcm	0x1001	/* ing c. olivetti & c., s.p.a. */
#define wave_format_olicelp	0x1002	/* ing c. olivetti & c., s.p.a. */
#define wave_format_olisbc	0x1003	/* ing c. olivetti & c., s.p.a. */
#define wave_format_oliopr	0x1004	/* ing c. olivetti & c., s.p.a. */
#define wave_format_dvi_adpcm	0x0011	/* intel corporation */
#define wave_format_ima_adpcm	(wave_format_dvi_adpcm)	/* intel corporation
 */
#define wave_format_unknown	0x0000	/* microsoft corporation */
#define wave_format_pcm	0x0001	/* microsoft corporation */
#define wave_format_adpcm	0x0002	/* microsoft corporation */
#define wave_format_alaw	0x0006	/* microsoft corporation */
#define wave_format_mulaw	0x0007	/* microsoft corporation */
#define wave_format_gsm610	0x0031	/* microsoft corporation */
#define wave_format_mpeg	0x0050	/* microsoft corporation */
#define wave_format_nms_vbxadpcm	0x0038	/* natural microsystems */
#define wave_format_oki_adpcm	0x0010	/* oki */
#define wave_format_sierra_adpcm	0x0013	/* sierra semiconductor corp */
#define wave_format_sonarc	0x0021	/* speech compression */
#define wave_format_mediaspace_adpcm	0x0012	/* videologic */
#define wave_format_yamaha_adpcm	0x0020	/* yamaha corporation of america */

struct FORMAT_CHUNK
{
	uint8_t sync;
	char formatchunkid[4]; //fmt
	DWORD formatchunksize; //20byte
	WORD wformattag;
	//ima-adpcm:0x0011
	//microsoft-adpcm:0x0002
	//microsoft-pcm:0x0001
	WORD nchannels;
	DWORD nsamplespersec; //44100
	DWORD navgbytespersec; //bytes/s
	WORD nblockalign; //0X0800
	WORD nbitpersample; //3//
};

struct DATA_CHUNK
{
	//     uint8_t sync;
	char fccID[4];
	unsigned long dwSize;
	uint16_t pre_val;
	int8_t index;
	int8_t reserved;
};

struct IMA_BLOCK_HEADER
{
	short sample0;
	BYTE index; //上一个block最后一个index，第一个block的index=0;
	BYTE reserved;
};

union BLOCK_HEADER
{
	struct IMA_BLOCK_HEADER ima_header[2];
};



struct WAVE_FORMAT_HEAD
{

	uint8_t sync;
	/*
	struct RIFF_CHUNK riff;

	struct FACT_CHUNK fack;

	struct FORMAT_CHUNK format;

	struct DATA_CHUNK data;

	union BLOCK_HEADER block_header;

	 */
	struct adpcm_state stat;
	uint16_t block_size;
};


int parse_pcm_head(char *data);


int do_play_pcm_stream(void);
void adpcm_decoder(char *indata, short *outdata, int len, struct adpcm_state *state);

void adpcm_coder(short *indata, char *outdata, long len, struct adpcm_state *state);

#endif
