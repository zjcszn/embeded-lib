#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "jpeglib.h"

#define CONVERT_TO_JPEG	1

#define CAMERA_WIDTH	320
#define CAMERA_HIGH		240
//#define CAMERA_WIDTH		160
//#define CAMERA_HIGH			120

//#define IMAGE_WIDTH		160
//#define  IMAGE_HIGH		120

#define IMAGE_WIDTH		320
#define  IMAGE_HIGH		240

#define CAMERA_LINE_BUFF_SIZE 	(CAMERA_WIDTH*2)

#define CAMERA_BUFF_LINES	8
#define CAMERA_BUFF_SIZE  (CAMERA_LINE_BUFF_SIZE*CAMERA_BUFF_LINES)

#define IMG_FRAMES			4

struct CAMERA_CFG{
	char *dma_buff;
	uint32_t dma_read_pos;
	uint32_t line_cnt;
	uint32_t img_line_cnt;
	 struct jpeg_compress_struct cinfo;
 	struct jpeg_error_mgr jerr;
	int  outfile;
  	JSAMPROW row_pointer[1];
  	int row_stride;
	unsigned char row_buff[IMAGE_WIDTH*3];
	BOOL image_recording;
	BOOL compress_running;
	BOOL img_write_finish;
	int sub_frame_cnt;
	int line_int_cnt;
	unsigned char *jpeg_buff;
	int jpeg_size;
	uint32_t img_rw_pos;
	sys_mbox_t send_queue;
	sys_sem_t   send_queue_sem;
};

struct CAMERA_CFG camera_cfg;


#define 	IMAGE_STORE_ADDR		0X0
#define 	IMAGE_SIZE 				(IMAGE_WIDTH*IMAGE_HIGH*2)

int reset_image_erea()
{
	camera_cfg.img_rw_pos = 0;
	camera_cfg.img_write_finish = FALSE;
	return m25p80_erase(IMAGE_STORE_ADDR, IMAGE_SIZE);
}

int write_image_data(void *buff, uint32_t size)
{
	m25p80_write(camera_cfg.img_rw_pos, size, (const u_char *)buff);
	camera_cfg.img_rw_pos += size;
	return 0;
}

int read_image_data(void *buff, uint32_t size)
{
	m25p80_read(camera_cfg.img_rw_pos, size, (u_char *)buff);
	camera_cfg.img_rw_pos += size;
	return 0;
}


void img_send_thread(void *arg);

static void DMA_DCMIConfiguration(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);
	DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_HTIF1);
	
	DMA_Cmd(DMA2_Stream1, DISABLE);
	memset(&DMA_InitStructure, 0, sizeof(DMA_InitTypeDef));

	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(DCMI->DR);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BufferSize/4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
 	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);

	////使能出错中断,半完成和完成中断
	DMA_ITConfig(DMA2_Stream1, /*DMA_IT_TC|*/DMA_IT_TE/*|DMA_IT_HT*/, ENABLE);  //使能传输完成和出错中断

	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DMA2_Stream1_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_Cmd(DMA2_Stream1, ENABLE);

	p_dbg("p1:%d", DMA2_Stream1->NDTR);
}



int open_camera()
{
	DCMI_InitTypeDef DCMI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);//DMA2

	GPIO_PinAFConfig(DCMI_PCLK_PORT_GROUP, DCMI_PCLK_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_HS_PORT_GROUP, DCMI_HS_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_VS_PORT_GROUP, DCMI_VS_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D0_PORT_GROUP, DCMI_D0_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D1_PORT_GROUP, DCMI_D1_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D2_PORT_GROUP, DCMI_D2_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D3_PORT_GROUP, DCMI_D3_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D4_PORT_GROUP, DCMI_D4_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D5_PORT_GROUP, DCMI_D5_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D6_PORT_GROUP, DCMI_D6_SOURCE, GPIO_AF_DCMI); 
	GPIO_PinAFConfig(DCMI_D7_PORT_GROUP, DCMI_D7_SOURCE, GPIO_AF_DCMI); 
	//GPIO_PinAFConfig(DCMI_D8_PORT_GROUP, DCMI_D8_SOURCE, GPIO_AF_DCMI); 
	//GPIO_PinAFConfig(DCMI_D9_PORT_GROUP, DCMI_D9_SOURCE, GPIO_AF_DCMI);
	
	gpio_cfg((uint32_t)DCMI_PCLK_PORT_GROUP, DCMI_PCLK_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_HS_PORT_GROUP, DCMI_HS_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_VS_PORT_GROUP, DCMI_VS_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D0_PORT_GROUP, DCMI_D0_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D1_PORT_GROUP, DCMI_D1_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D2_PORT_GROUP, DCMI_D2_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D3_PORT_GROUP, DCMI_D3_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D4_PORT_GROUP, DCMI_D4_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D5_PORT_GROUP, DCMI_D5_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D6_PORT_GROUP, DCMI_D6_PIN, GPIO_Mode_AF_IF); 
	gpio_cfg((uint32_t)DCMI_D7_PORT_GROUP, DCMI_D7_PIN, GPIO_Mode_AF_IF); 
	//gpio_cfg((uint32_t)DCMI_D8_PORT_GROUP, DCMI_D8_PIN, GPIO_Mode_AF_IF); 
	//gpio_cfg((uint32_t)DCMI_D9_PORT_GROUP, DCMI_D9_PIN, GPIO_Mode_AF_IF);

	gpio_cfg((uint32_t)MCO_PORT_GROUP, MCO_PIN, GPIO_Mode_AF_PP);
	RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
	//RCC_MCO1Config(RCC_MCO1Source_PLLCLK, RCC_MCO1Div_5);
	
	DCMI_DeInit();
	DCMI_InitStructure.DCMI_CaptureMode = DCMI_CaptureMode_Continuous;
	DCMI_InitStructure.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
	DCMI_InitStructure.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
	DCMI_InitStructure.DCMI_HSPolarity = DCMI_VSPolarity_High;
	DCMI_InitStructure.DCMI_PCKPolarity = DCMI_PCKPolarity_Falling;
	DCMI_InitStructure.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
	DCMI_InitStructure.DCMI_VSPolarity = DCMI_VSPolarity_High;
	
	DCMI_Init(&DCMI_InitStructure); 

	memset(&camera_cfg, 0, sizeof(struct CAMERA_CFG));

	camera_cfg.dma_buff = (char*)mem_malloc(CAMERA_BUFF_SIZE);
	
	DMA_DCMIConfiguration((uint32_t*)camera_cfg.dma_buff, CAMERA_BUFF_SIZE);

	DCMI_ITConfig(/*DCMI_IT_FRAME | DCMI_IT_OVF 
		| DCMI_IT_ERR |*/DCMI_IT_VSYNC | DCMI_IT_LINE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DCMI_IRQn;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DCMI_IRQn_Priority;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	if(OV7670_init() != 0)
		return -1;;

	thread_create(img_send_thread,
                        0,
                        TASK_IMG_SEND_PRIO,
                        0,
                        TASK_IMG_SEND_STACK_SIZE,
                        "img_snd");

	DCMI_Cmd(ENABLE);

	DCMI_CaptureCmd(ENABLE);

//	start_compress_jpeg();

#if 0
{
	uint32_t i, t1, t2, t3;
	t1 = os_time_get();
	start_compress_jpeg();
	t2 = os_time_get();
	for(i = 0; i < CAMERA_HIGH; i++)
	{
		camera_cfg.row_pointer[0] =  camera_cfg.dma_buff;
		(void) jpeg_write_scanlines(&camera_cfg.cinfo, camera_cfg.row_pointer, 1);
	}
	t3 = os_time_get();
	p_dbg("t1:%d,%d,%d", t1, t2, t3);
	end_compress_jpeg();
	while(1);
}
#endif

	
	return 0;
}

int close_camera()
{
	DCMI_Cmd(DISABLE);

	DMA_Cmd(DMA2_Stream1, DISABLE);

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, DISABLE);
	
	return 0;
}

void start_record_image()
{
	reset_image_erea();
	camera_cfg.image_recording = TRUE;
	DCMI_CaptureCmd(ENABLE);
}

void end_record_image()
{
	DCMI_CaptureCmd(DISABLE);
	camera_cfg.image_recording = FALSE;
}

int start_compress_jpeg()
{
  camera_cfg.cinfo.err = jpeg_std_error(&camera_cfg.jerr);
  jpeg_create_compress(&camera_cfg.cinfo);
  /*
  if ((outfile = _fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }*/
  jpeg_stdio_dest(&camera_cfg.cinfo, 0);
  camera_cfg.cinfo.image_width = IMAGE_WIDTH; 	/* image width and height, in pixels */
  camera_cfg.cinfo.image_height = IMAGE_HIGH;
  camera_cfg.cinfo.input_components = 3;		/* # of color components per pixel */
  camera_cfg.cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&camera_cfg.cinfo);
  jpeg_set_quality(&camera_cfg.cinfo, 0, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&camera_cfg.cinfo, TRUE);
  camera_cfg.row_stride = IMAGE_WIDTH * 3;
  //camera_cfg.compress_running = TRUE;
  //mem_slide_check(TRUE);
  return 0;
}

int end_compress_jpeg()
{
	camera_cfg.compress_running = FALSE;
	
	jpeg_finish_compress(&camera_cfg.cinfo);
	jpeg_destroy_compress(&camera_cfg.cinfo);
	return 0;
}
/*
unsigned char img_buff[CAMERA_LINE_BUFF_SIZE + 16] = {
0x55, 0xaa, 0xff, 0x00
};*/
#define RGB565_MASK_RED 0xF800 
#define RGB565_MASK_GREEN 0x07E0 
#define RGB565_MASK_BLUE 0x001F
extern int remote_socket_fd;

void dcmi_read_data(uint8_t *buff, uint32_t size)
{
	int i, lines, ret;
	unsigned char *img_buff;
	uint16_t *p_img_buff, *p_buff = (uint16_t *)buff;
	
	lines = size/CAMERA_LINE_BUFF_SIZE;
	camera_cfg.sub_frame_cnt += size;
	for(i = 0; i < lines; i++)
	{

		int j;
		uint16_t tmp;		
#if 	1
		img_buff = (unsigned char *)mem_malloc(IMAGE_WIDTH*2 + 4);
		if(!img_buff)
			break;
		p_img_buff = (uint16_t*)(img_buff + 4);
		img_buff[0] = 0x55;
		img_buff[1] = 0xaa;
		img_buff[2] = 0xff;
		img_buff[3] = camera_cfg.line_cnt + i;

		for(j = 0; j < IMAGE_WIDTH; j++)
		{
			p_img_buff[j] = p_buff[CAMERA_WIDTH*i + j];
		}
	/*
		if(camera_cfg.send_queue){
			sys_mbox_post(&camera_cfg.send_queue, &camera_cfg.send_queue_sem, img_buff);
		}else
			mem_free(img_buff);

		if(camera_cfg.line_cnt + i >= CAMERA_HIGH)
			end_record_image();
*/
		if(camera_cfg.image_recording && (camera_cfg.img_rw_pos < IMAGE_SIZE)){
			write_image_data(img_buff, IMAGE_WIDTH*2 + 4);
				
			if(camera_cfg.img_rw_pos >= IMAGE_SIZE){
					p_dbg("write image finish");
					camera_cfg.img_write_finish = TRUE;
			}
		}

		mem_free(img_buff);
		if(!camera_cfg.image_recording && camera_cfg.img_write_finish)
		{
				p_dbg("start compress image");
		}
#else
		for(j = 0; j < IMAGE_WIDTH; j++)
		{
			tmp = p_buff[CAMERA_WIDTH*i + j];
			camera_cfg.row_buff[j*3]= (tmp&RGB565_MASK_RED) >> 11;   
			camera_cfg.row_buff[j*3 + 1]= (tmp&RGB565_MASK_GREEN) >> 5; 
			camera_cfg.row_buff[j*3 + 2]= (tmp&RGB565_MASK_BLUE); 
		}

		if(camera_cfg.compress_running)
		{
			if(camera_cfg.cinfo.next_scanline < camera_cfg.cinfo.image_height) {
			    	camera_cfg.row_pointer[0] =  camera_cfg.row_buff;
			   	(void) jpeg_write_scanlines(&camera_cfg.cinfo, camera_cfg.row_pointer, 1);
		 	 }else
		 		 p_err("1");
		}
#endif
	}
	camera_cfg.line_cnt += lines;
	//p_dbg("s:%d", size);
}

int handle_dcmi_line()
{
	__IO u32_t cur_pos, i, size, ret, cnt;
	do
	{
		cnt = CAMERA_BUFF_SIZE - DMA2_Stream1->NDTR*4;   //取当前缓冲区数据位置
		cur_pos = cnt/CAMERA_LINE_BUFF_SIZE*CAMERA_LINE_BUFF_SIZE;

		if(cur_pos == camera_cfg.dma_read_pos)
		{
			return 0;
		}
		if(cur_pos > camera_cfg.dma_read_pos)
		{
			size = cur_pos - camera_cfg.dma_read_pos;

			dcmi_read_data((uint8_t *)&camera_cfg.dma_buff[camera_cfg.dma_read_pos], size);

			camera_cfg.dma_read_pos = cur_pos;
		}
		else
		{
			cnt = 0;
			if(cur_pos < camera_cfg.dma_read_pos)
			{
				size = CAMERA_BUFF_SIZE - camera_cfg.dma_read_pos;

				dcmi_read_data((uint8_t *)&camera_cfg.dma_buff[camera_cfg.dma_read_pos], size);
				
				camera_cfg.dma_read_pos = 0;

				size = cur_pos;
				if(size)
					dcmi_read_data((uint8_t *)camera_cfg.dma_buff, size);

				camera_cfg.dma_read_pos = cur_pos;
			}
		}
	}while(0);
	return 0;
}


void DCMI_IRQHandler(void)
{
	uint32_t stat = DCMI->MISR;
	enter_interrupt();
	DCMI_ClearITPendingBit(DCMI_IT_FRAME | DCMI_IT_OVF 
		| DCMI_IT_ERR |DCMI_IT_VSYNC | DCMI_IT_LINE);

	if(stat & DCMI_IT_LINE)
	{
		DCMI_ClearITPendingBit(DCMI_IT_LINE);
		camera_cfg.line_int_cnt++;
		handle_dcmi_line();
		stat &= ~DCMI_IT_LINE;
	}

	if(!stat)
		goto end;

	if(stat & DCMI_IT_VSYNC)
	{
		//p_dbg("%d, %d", camera_cfg.line_int_cnt, camera_cfg.sub_frame_cnt);
		camera_cfg.sub_frame_cnt = 0;
		camera_cfg.line_cnt = 0;
		camera_cfg.line_int_cnt = 0;
		
		DCMI_ClearITPendingBit(DCMI_IT_VSYNC);
	}

	if(!stat)
		goto end;

	if(stat & DCMI_IT_FRAME)
	{
		//p_dbg("1");
		DCMI_ClearITPendingBit(DCMI_IT_FRAME);
	}

	if(stat & DCMI_IT_OVF)
	{
		p_err("2");
		DCMI_ClearITPendingBit(DCMI_IT_OVF);
	}

	if(stat & DCMI_IT_ERR)
	{
		p_err("3");
		DCMI_ClearITPendingBit(DCMI_IT_ERR);
	}
end:	
	exit_interrupt(1);
}


void DMA2_Stream1_IRQHandler() 
{
	enter_interrupt();

	if(DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_HTIF1))
	{
		DMA_ClearFlag(DMA2_Stream1,  DMA_FLAG_HTIF1);
		
	}

	if(DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_TCIF1))
	{
		DMA_ClearFlag(DMA2_Stream1,  DMA_FLAG_TCIF1);
	}

	if(DMA_GetFlagStatus(DMA2_Stream1, DMA_FLAG_TEIF1))
	{
		p_err("dcmi dma err");
		DMA_ClearFlag(DMA2_Stream1,  DMA_FLAG_TEIF1);
	}

 	exit_interrupt(0);
}

void camera_button_event(int event)
{
	if(camera_cfg.image_recording)
	{
		p_err("image_recording");
		return;
	}else{
		start_record_image();
	}
}


void img_send_thread(void *arg)
{
	int ret;
	uint32_t last_line;
	u16_t *p_short;
	u8_t *p_byte;
	uint32_t t1, t2;
	ret = sys_mbox_new(&camera_cfg.send_queue, &camera_cfg.send_queue_sem, 5);
	if(ret != ERR_OK)
	{
		return;
	}

	while(1)
	{
		ret = sys_arch_mbox_fetch(&camera_cfg.send_queue, &camera_cfg.send_queue_sem, &p_short, 0);
		if(ret != -1)
		{
		#if 0
			int j;
			uint16_t tmp;
			p_byte = (u8_t *)p_short;

			if(p_byte[3] == 0)
			{
				
				if(camera_cfg.compress_running){
					p_dbg("%d", camera_cfg.cinfo.next_scanline);
					end_compress_jpeg();
					p_dbg("e1");
				}
				start_compress_jpeg();
				last_line = 0;
				p_dbg("s");
			}
			
			t1 = os_time_get();
			for(j=0; j< IMAGE_WIDTH; j++)
			{
				tmp = p_short[2 + j];
				camera_cfg.row_buff[j*3]= (tmp&RGB565_MASK_RED) >> 11;   
				camera_cfg.row_buff[j*3 + 1]= (tmp&RGB565_MASK_GREEN) >> 5; 
				camera_cfg.row_buff[j*3 + 2]= (tmp&RGB565_MASK_BLUE); 
			}
			if(camera_cfg.compress_running)
			{
				//p_dbg("%d", p_byte[3]);
				 if(camera_cfg.cinfo.next_scanline < camera_cfg.cinfo.image_height) {
				 	//p_dbg("%d", camera_cfg.cinfo.next_scanline);
				 	if(last_line != (p_byte[3] - 1))
						p_dbg("%d,%d", last_line, p_byte[3]);
					last_line = p_byte[3];
		    			camera_cfg.row_pointer[0] =  camera_cfg.row_buff;
		   			 (void) jpeg_write_scanlines(&camera_cfg.cinfo, camera_cfg.row_pointer, 1);
		 		 }else
		 		 	p_err("1");
			}
			t2 = os_time_get();

			//p_dbg("t:%d", t2 - t1);

			if(p_byte[3] == (CAMERA_HIGH - 1))
			{
				if(camera_cfg.compress_running){
					p_dbg("%d", camera_cfg.cinfo.next_scanline);
					end_compress_jpeg();
					p_dbg("e");
				}

				//_fwrite(0, 1, 100, 0);
			}
		#else
			//_fwrite(p_short, 1, IMAGE_WIDTH*2 + 4, 0);
			
		#endif
			mem_free(p_short);
		}
	}
}

