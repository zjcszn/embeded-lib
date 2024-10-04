//#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>

#include <moal_sdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NULL 0
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00002710)
#define SDIO_FIFO_Address               ((uint32_t)0x40012C80)

/* Mask for errors Card Status R1 (OCR Register) */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/* Masks for R6 Response */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)
#define SD_CARD_PROGRAMMING             ((uint32_t)0x00000007)
#define SD_CARD_RECEIVING               ((uint32_t)0x00000006)
#define SD_DATATIMEOUT                  ((uint32_t)0x000FFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/* Command Class Supported */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/* Following commands are SD Card Specific commands.
   SDIO_APP_CMD should be sent before sending these commands. */
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)

#define SDIO_INIT_CLK_DIV               ((uint8_t)0x80)

#if 1
#define SDIO_TRANSFER_CLK_DIV		    ((uint8_t)0x0)   //45m/(x+2)
#define ENABLE_DATA_WIDE 				0
#else
#define SDIO_TRANSFER_CLK_DIV		    ((uint8_t)0x6)   //45m/(x+2)
#define ENABLE_DATA_WIDE 	1			//4bit模式下时钟不能太高，否则会出错，CLK_DIV = 6是速率跟1bit模式差不多
#endif


struct sdio_cccr cccr;
struct sdio_func func;

static wait_event_t sd_waitq;
static SD_Error	 sd_cmd_res;
static mutex_t sd_mutex;

/* Private function prototypes -----------------------------------------------*/
static SD_Error CmdError(void);
static SD_Error CmdResp1Error(uint8_t cmd);
//static SD_Error CmdResp7Error(void);
//static SD_Error CmdResp3Error(void);
//static SD_Error CmdResp2Error(void);
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca);
//static SD_Error SDEnWideBus(FunctionalState NewState);
//static SD_Error IsCardProgramming(uint8_t *pstatus);
//static SD_Error FindSCR(uint16_t rca, uint32_t *pscr);
static uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes);
static void GPIO_Configuration(void);
static void DMA_TxConfiguration(uint32_t *BufferSRC, uint32_t BufferSize);
static void DMA_RxConfiguration(uint32_t *BufferDST, uint32_t BufferSize);
int mmc_send_relative_addr( u16 *rca);
int sdio_read_cccr(void);
int sdio_read_common_cis(void);
int sdio_enable_wide(void);
int mmc_sdio_switch_hs(int enable);
int mmc_send_if_cond(u32 ocr);

void enable_sdio_int()
{ 
	SDIO_ITConfig(SDIO_IT_SDIOIT , ENABLE);
}

/* Private functions ---------------------------------------------------------*/

/*
 * 函数名：SD_Init
 * 描述  ：初始化SD卡，使卡处于就绪状态(准备传输数据)
 * 输入  ：无
 * 输出  ：-SD_Error SD卡错误代码
 *         成功时则为 SD_OK
 * 调用  ：外部调用
 */

int SD_Init(void)
{
	int errorstatus = SD_OK;
	int ocr;
	u16 rca;
	SDIO_InitTypeDef SDIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
	
	sd_waitq = init_event();
	sd_mutex = mutex_init(__FUNCTION__);
	/* Configure SDIO interface GPIO */
	GPIO_Configuration();

	/* Enable the SDIO AHB Clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);

	/* Enable the DMA2 Clock */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	SDIO_DeInit();
	
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC | DMA_IT_TE | DMA_IT_HT,DISABLE);

	SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV; /* HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz */
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising ;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init(&SDIO_InitStructure);

	SDIO_SetSDIOOperation(ENABLE);

	/* Set Power State to ON */
	SDIO_SetPowerState(SDIO_PowerState_ON);

	/* Enable SDIO Clock */
	SDIO_ClockCmd(ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SDIO_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

again:
	/* CMD0: GO_IDLE_STATE -------------------------------------------------------*/
	/* No CMD response required */

	SDIO_PDN_LOW;
	sleep(50);
	SDIO_PDN_HIGH;
	sleep(50);
	
	SDIO_SetSDIOReadWaitMode(SDIO_ReadWaitMode_CLK);
	
	errorstatus = sdio_reset();

	errorstatus = mmc_go_idle();
	if(errorstatus != SD_OK)
	{
		goto out;
	}	
	
	mmc_send_if_cond(SD_CHECK_PATTERN);

	errorstatus = mmc_send_io_op_cond(0, (u32*)&ocr);
	if (errorstatus != SD_OK)
	{
		goto out;
	}

	errorstatus = mmc_send_io_op_cond(ocr, (u32*)&ocr);
	if (errorstatus != SD_OK)
	{
		goto out;
	}
	
	errorstatus = mmc_send_relative_addr( &rca);
	if (errorstatus != SD_OK)
	{
		goto out;
	}

	errorstatus = SD_SelectDeselect(rca << 16);
	if (errorstatus != SD_OK)
	{
		goto out;
	}

	errorstatus = sdio_read_cccr();
	if (errorstatus != SD_OK)
	{
		goto out;
	}

	sdio_read_common_cis();

	errorstatus = mmc_sdio_switch_hs(ENABLE);
	if (errorstatus != SD_OK)
	{
		goto out;
	}

#if ENABLE_DATA_WIDE
	errorstatus = sdio_enable_wide();
	if (errorstatus != SD_OK)
	{
		goto out;
	}
#endif
	SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV;
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
#if ENABLE_DATA_WIDE
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
#else
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
#endif
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init(&SDIO_InitStructure);

out:
	if (errorstatus != SD_OK)
	{
		//p_err("SD_Init err:%d\n",errorstatus);
		p_err("SDIO设备初始化失败");
		sleep(1000);
		goto again;
	}
	
	return errorstatus;

}


int mmc_go_idle()
{
	SD_Error errorstatus = SD_OK;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_GO_IDLE_STATE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdError();

	return (errorstatus);
}

/*
 * 函数名：SD_SelectDeselect
 * 描述  ：Selects od Deselects the corresponding card
 * 输入  ：-addr 选择卡的地址
 * 输出  ：-SD_Error SD卡错误代码
 *         成功时则为 SD_OK
 * 调用  ：外部调用
 */
SD_Error SD_SelectDeselect(uint32_t addr)
{
	SD_Error errorstatus = SD_OK;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
	/* Send CMD7 SDIO_SEL_DESEL_CARD */
	SDIO_CmdInitStructure.SDIO_Argument = addr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEL_DESEL_CARD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SDIO_SEL_DESEL_CARD);

	if (errorstatus != SD_OK)
		p_err("SD_SelectDeselect err:%d\n", errorstatus);
	else
		p_dbg("SD_SelectDeselect ok\n");

	return (errorstatus);
}


/**
  * @brief  Checks for error conditions for CMD0.
  * @param  None
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout;

	timeout = SDIO_CMD0TIMEOUT; /* 10000 */

	while ((timeout > 0) && (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET))
	{
		timeout--;
	}

	if (timeout == 0)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		return (errorstatus);
	}

	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	return (errorstatus);
}

/**
  * @brief  Checks for error conditions for R1.
  *   response
  * @param  cmd: The sent command index.
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp1Error(uint8_t cmd)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;
	int cmd_timeout = 10000;
	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
		if(!(cmd_timeout--))
			return SD_ERROR;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	/* Check response received is of desired command */
	if (SDIO_GetCommandResponse() != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}

	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/* We have received response, retrieve it for analysis  */
	response_r1 = SDIO_GetResponse(SDIO_RESP1);

	if ((response_r1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
	{
		return (errorstatus);
	}

	if (response_r1 & SD_OCR_ADDR_OUT_OF_RANGE)
	{
		return (SD_ADDR_OUT_OF_RANGE);
	}

	if (response_r1 & SD_OCR_ADDR_MISALIGNED)
	{
		return (SD_ADDR_MISALIGNED);
	}

	if (response_r1 & SD_OCR_BLOCK_LEN_ERR)
	{
		return (SD_BLOCK_LEN_ERR);
	}

	if (response_r1 & SD_OCR_ERASE_SEQ_ERR)
	{
		return (SD_ERASE_SEQ_ERR);
	}

	if (response_r1 & SD_OCR_BAD_ERASE_PARAM)
	{
		return (SD_BAD_ERASE_PARAM);
	}

	if (response_r1 & SD_OCR_WRITE_PROT_VIOLATION)
	{
		return (SD_WRITE_PROT_VIOLATION);
	}

	if (response_r1 & SD_OCR_LOCK_UNLOCK_FAILED)
	{
		return (SD_LOCK_UNLOCK_FAILED);
	}

	if (response_r1 & SD_OCR_COM_CRC_FAILED)
	{
		return (SD_COM_CRC_FAILED);
	}

	if (response_r1 & SD_OCR_ILLEGAL_CMD)
	{
		return (SD_ILLEGAL_CMD);
	}

	if (response_r1 & SD_OCR_CARD_ECC_FAILED)
	{
		return (SD_CARD_ECC_FAILED);
	}

	if (response_r1 & SD_OCR_CC_ERROR)
	{
		return (SD_CC_ERROR);
	}

	if (response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
	{
		return (SD_GENERAL_UNKNOWN_ERROR);
	}

	if (response_r1 & SD_OCR_STREAM_READ_UNDERRUN)
	{
		return (SD_STREAM_READ_UNDERRUN);
	}

	if (response_r1 & SD_OCR_STREAM_WRITE_OVERRUN)
	{
		return (SD_STREAM_WRITE_OVERRUN);
	}

	if (response_r1 & SD_OCR_CID_CSD_OVERWRIETE)
	{
		return (SD_CID_CSD_OVERWRITE);
	}

	if (response_r1 & SD_OCR_WP_ERASE_SKIP)
	{
		return (SD_WP_ERASE_SKIP);
	}

	if (response_r1 & SD_OCR_CARD_ECC_DISABLED)
	{
		return (SD_CARD_ECC_DISABLED);
	}

	if (response_r1 & SD_OCR_ERASE_RESET)
	{
		return (SD_ERASE_RESET);
	}

	if (response_r1 & SD_OCR_AKE_SEQ_ERROR)
	{
		return (SD_AKE_SEQ_ERROR);
	}
	return (errorstatus);
}



static SD_Error CmdResp4Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	int cmd_timeout = 10000;
	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
		if(!(cmd_timeout--))
			return SD_ERROR;
	}
	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	}
	//R4b 没有crc校验，直接返回OK
	if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_OK;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}
	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	return (errorstatus);
}


/**
  * @brief  Checks for error conditions for R6 (RCA).
  *   response.
  * @param  cmd: The sent command index.
  * @param  prca: pointer to the variable that will contain the SD
  *   card relative address RCA. 
  * @retval SD_Error: SD Card Error code.
  */
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;
	
	int cmd_timeout = 10000;	//超过1ms

	status = SDIO->STA;

	
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
	{
		status = SDIO->STA;
		if(!(cmd_timeout--))
			return SD_ERROR;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	/* Check response received is of desired command */
	if (SDIO_GetCommandResponse() != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}

	/* Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/* We have received response, retrieve it.  */
	response_r1 = SDIO_GetResponse(SDIO_RESP1);

	if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
	{
		*prca = (uint16_t) (response_r1 >> 16);
		return (errorstatus);
	}

	if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
	{
		return (SD_GENERAL_UNKNOWN_ERROR);
	}

	if (response_r1 & SD_R6_ILLEGAL_CMD)
	{
		return (SD_ILLEGAL_CMD);
	}

	if (response_r1 & SD_R6_COM_CRC_FAILED)
	{
		return (SD_COM_CRC_FAILED);
	}

	return (errorstatus);
}



static SD_Error CmdResp5Error(uint8_t cmd, uint8_t *data)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;
	int cmd_timeout = 10000;
	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
	{
		status = SDIO->STA;
		if(!(cmd_timeout--)){
			p_err("sta:%x", status);
			return SD_ERROR;
		}
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return (errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return (errorstatus);
	}

	/* Check response received is of desired command */
	if (SDIO_GetCommandResponse() != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return (errorstatus);
	}

	/* Clear all the static flags 只清除部分*/
	SDIO_ClearFlag(SDIO_FLAG_CMDACT | SDIO_FLAG_CMDSENT | SDIO_FLAG_CMDREND);

	/* We have received response, retrieve it.  */
	response_r1 = SDIO_GetResponse(SDIO_RESP1);


	if (response_r1 & R5_ERROR)
		return SD_ILLEGAL_CMD;
	if (response_r1 & R5_FUNCTION_NUMBER)
		return SD_SDIO_UNKNOWN_FUNCTION;
	if (response_r1 & R5_OUT_OF_RANGE)
		return SD_CMD_OUT_OF_RANGE;

	if (data)
		*data = response_r1 & 0xff;

	return (errorstatus);
}

SD_Error CmdResp7Error(void)
{
	SD_Error errorstatus=SD_OK;
	u32 status;
	u32 timeout=SDIO_CMD0TIMEOUT;
	while(timeout--)
	{
		status=SDIO->STA;
		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC错误/命令响应超时/已经收到响应(CRC校验成功)	
	}
	if((timeout==0)||(status&(1<<2)))	//响应超时
	{	
		errorstatus=SD_CMD_RSP_TIMEOUT;	//当前卡不是2.0兼容卡,或者不支持设定的电压范围
		SDIO->ICR|=1<<2;	 //清除命令响应超时标志
		return errorstatus;
	}	
	if(status&1<<6)	 //成功接收到响应
	{	
		errorstatus=SD_OK;
		SDIO->ICR|=1<<6;	 //清除响应标志
	}
	return errorstatus;
}	



int mmc_send_if_cond(u32 ocr)
{
	SD_Error errorstatus = SD_OK;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;

	SDIO_CmdInitStructure.SDIO_Argument =  ocr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_SEND_IF_COND ;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;

	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp7Error();
	if (SD_OK != errorstatus)
	{
		goto end;
	}

end:
	if(errorstatus != SD_OK)
		p_dbg("mmc_send_io_op_cond err:%d\n",errorstatus);

	return errorstatus;
}



/**
  * @brief  Converts the number of bytes in power of two and returns the
  *   power.
  * @param  NumberOfBytes: number of bytes.
  * @retval None
  */
static uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes)
{
	uint8_t count = 0;

	while (NumberOfBytes != 1)
	{
		NumberOfBytes >>= 1;
		count++;
	}
	return (count);
}

/**
  * @brief  Configures the SDIO Corresponding GPIO Ports
  * @param  None
  * @retval : None
  */
static void GPIO_Configuration(void)
{
	/* GPIOC and GPIOD Periph clock enable */
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO); 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO); 
#if ENABLE_DATA_WIDE
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO); 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO); 
#endif
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO); 
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO); 

	/* Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */
	gpio_cfg((uint32_t)GPIOC, GPIO_Pin_8 | 
#if ENABLE_DATA_WIDE	
	GPIO_Pin_10 | GPIO_Pin_11 | 
#endif	
	GPIO_Pin_12, GPIO_Mode_AF_PP);

	gpio_cfg((uint32_t)GPIOC, GPIO_Pin_9, GPIO_Mode_AF_IPU);
	
	/* Configure PD.02 CMD line */
	gpio_cfg((uint32_t)GPIOD, GPIO_Pin_2, GPIO_Mode_AF_PP);

	gpio_cfg((uint32_t)SDIO_RESET_PORT, SDIO_RESET_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)SDIO_PDN_PORT, SDIO_PDN_PIN, GPIO_Mode_Out_PP);

	SDIO_PDN_HIGH;
	SDIO_RESET_LOW;
 
}



/**
  * @brief  Configures the DMA2 Channel4 for SDIO Tx request.
  * @param  BufferSRC: pointer to the source buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void DMA_TxConfiguration(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;
	
	
	/* DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Stream3, DISABLE);

	DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_DMEIF3);

	DMA_DeInit(DMA2_Stream3);
	/* DMA2 Channel4 Config */
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_Address;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferSRC;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = (BufferSize+3) / 4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
 	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);

	 DMA_FlowControllerConfig(DMA2_Stream3, DMA_FlowCtrl_Peripheral);
	/* DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Stream3, ENABLE);
}

/**
  * @brief  Configures the DMA2 Channel4 for SDIO Rx request.
  * @param  BufferDST: pointer to the destination buffer
  * @param  BufferSize: buffer size
  * @retval None
  */
static void DMA_RxConfiguration(uint32_t *BufferDST, uint32_t BufferSize)
{
	DMA_InitTypeDef DMA_InitStructure;

	
	/* DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Stream3, DISABLE);

	DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_DMEIF3);

	DMA_DeInit(DMA2_Stream3);
	/* DMA2 Channel4 Config */
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)SDIO_FIFO_Address;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = (BufferSize+3) / 4;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
	
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);
	DMA_FlowControllerConfig(DMA2_Stream3, DMA_FlowCtrl_Peripheral);
	/* DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Stream3, ENABLE);
}




int mmc_send_io_op_cond(u32 ocr, u32 *rocr)
{
	int i;
	u32 response;
	SD_Error errorstatus = SD_OK;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;

	SDIO_CmdInitStructure.SDIO_Argument = ocr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_IO_SEND_OP_COND ;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	for (i = 100; i; i--)
	{
		SDIO_SendCommand(&SDIO_CmdInitStructure);

		errorstatus = CmdResp4Error();

		if (SD_OK != errorstatus)
		{
			goto end;
		}
		response = SDIO_GetResponse(SDIO_RESP1);
		if (response & MMC_CARD_BUSY)
			break;
		sleep(10);
	}
	if (i == 0)
	{
		
		errorstatus = SD_ERROR;
		goto end;
	}
	if (rocr)
		*rocr = response;

end:
	if(errorstatus != SD_OK)
		p_err("mmc_send_io_op_cond err:%x\n",errorstatus);

	return errorstatus;
}

int mmc_io_rw_direct_host(int write, unsigned fn,
		unsigned addr, u8 in, u8 *out)
{
	SD_Error errorstatus = SD_OK;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;

	SDIO_CmdInitStructure.SDIO_Argument = write ? 0x80000000 : 0x00000000;
	SDIO_CmdInitStructure.SDIO_Argument |= fn << 28;
	SDIO_CmdInitStructure.SDIO_Argument |= (write && out) ? 0x08000000 : 0x00000000;
	SDIO_CmdInitStructure.SDIO_Argument |= addr << 9;
	SDIO_CmdInitStructure.SDIO_Argument |= in;

	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_IO_RW_DIRECT;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;

	mutex_lock(sd_mutex);
	
	if(SDIO_GetFlagStatus(SDIO_FLAG_TXACT) || SDIO_GetFlagStatus(SDIO_FLAG_RXACT) || SDIO_GetFlagStatus(SDIO_FLAG_CMDACT))
	{
		p_err("mmc_io_rw_direct_host busy:%08x\n" , SDIO->STA);
	}

	SDIO_ClearFlag(SDIO_FLAG_ALL & (~SDIO_FLAG_SDIOIT));
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp5Error(SD_IO_RW_DIRECT, out);
	mutex_unlock(sd_mutex);
	if (SD_OK != errorstatus)
	{
		//p_err("mmc_io_rw_direct_host err:%d\n", errorstatus);
	}
	return errorstatus;
}

int mmc_io_rw_direct(int write, unsigned fn, unsigned addr, u8 in, u8 *out)
{

	return mmc_io_rw_direct_host(write, fn, addr, in, out);
}

#define DCTRL_CLEAR_MASK         ((uint32_t)0xFFFFFF08)

SD_Error dma_start_send_data()
{
	int ret;
	uint32_t  cpu_sr;
	clear_wait_event (sd_waitq);    //先清零	
	SDIO_ClearFlag(SDIO_FLAG_ALL & (~SDIO_FLAG_SDIOIT));
	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXUNDERR | SDIO_IT_STBITERR, ENABLE);
	SDIO_DMACmd(ENABLE);   //DMA开启
	ret = wait_event_timeout(sd_waitq, 100);  //阻塞100ms
	if(ret != 0)
	{
		p_err("dma_send timeout\n");
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXUNDERR | SDIO_IT_STBITERR, DISABLE);
		cpu_sr = local_irq_save(); 
		SDIO->DCTRL &= DCTRL_CLEAR_MASK;
		local_irq_restore(cpu_sr);
		SDIO_ClearFlag(SDIO_FLAG_ALL & (~SDIO_FLAG_SDIOIT));
		sd_cmd_res = SD_ERROR;
	}
	return sd_cmd_res;
}

#define CCR_ENABLE_Reset        ((uint32_t)0xFFFFFFFE)
extern void woal_sdio_interrupt(struct sdio_func *func);
DECLARE_MONITOR_ITEM("SDIO INT CNT", sdio_int_cnt);
void SDIO_IRQHandler(void)
{
	uint32_t  cpu_sr;
	uint32_t int_stat;
	int need_sched = 0;
	enter_interrupt();
      
	int_stat = SDIO->STA & SDIO->MASK;
	INC_MONITOR_ITEM_VALUE(sdio_int_cnt);
	if(int_stat & SDIO_FLAG_SDIOIT)
	{
		cpu_sr = local_irq_save();
		SDIO->ICR = SDIO_IT_SDIOIT;
		local_irq_restore(cpu_sr);
		need_sched = 1;    
		wake_up(func.sdio_int_wait);

		goto end;
	}
	
	if(int_stat & (SDIO_FLAG_DATAEND | SDIO_FLAG_DTIMEOUT |SDIO_FLAG_DCRCFAIL
	| SDIO_FLAG_TXUNDERR | SDIO_FLAG_RXOVERR | SDIO_FLAG_STBITERR))
	{					 
		if(int_stat & SDIO_FLAG_DATAEND)
			sd_cmd_res = SD_OK;
		else{
			p_err("t:%x\n", int_stat);
			sd_cmd_res = SD_ERROR;
		}
		cpu_sr = local_irq_save();
		SDIO->DCTRL &= DCTRL_CLEAR_MASK;
	 	DMA2_Stream3->CR &= CCR_ENABLE_Reset;
	 	SDIO->MASK &= SDIO_IT_SDIOIT;
		SDIO->ICR = SDIO_FLAG_DATAEND | SDIO_FLAG_DTIMEOUT |SDIO_FLAG_DCRCFAIL
		| SDIO_FLAG_TXUNDERR | SDIO_FLAG_RXOVERR | SDIO_FLAG_STBITERR;
		SDIO_DMACmd(DISABLE);
		local_irq_restore(cpu_sr);
		need_sched = 1;
		wake_up (sd_waitq);
	}
end:
	exit_interrupt(need_sched);
}

int mmc_io_rw_extended(int write, unsigned fn,
		unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
{
	int ret;
	u8 out;
	uint32_t irq_flag;
	SD_Error errorstatus = SD_OK;
	uint8_t power = 0;
	uint32_t *tempbuff = (uint32_t *)buf;
	SDIO_DataInitTypeDef SDIO_DataInitStructure;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
	uint32_t TotalNumberOfBytes;
	
	mutex_lock(sd_mutex);
	
	if(SDIO_GetFlagStatus(SDIO_FLAG_TXACT) || SDIO_GetFlagStatus(SDIO_FLAG_RXACT) || SDIO_GetFlagStatus(SDIO_FLAG_CMDACT))
	{
		p_err("mmc_io_rw_extended busy\n");
	}
	
	SDIO_ClearFlag(SDIO_FLAG_ALL & (~SDIO_FLAG_SDIOIT));

	power = convert_from_bytes_to_power_of_two(blksz);

	TotalNumberOfBytes = blocks * blksz;
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = blocks * blksz;
	SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
	if(write)
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	else
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;

	if (blocks == 1 && blksz < 256)
		SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Stream;
	else
		SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;

	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	if(!write)
	{
		clear_wait_event(sd_waitq);    //先清零	
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
		SDIO_ClearFlag(SDIO_FLAG_ALL & (~SDIO_FLAG_SDIOIT));
		DMA_RxConfiguration(tempbuff, TotalNumberOfBytes);	
		SDIO_DMACmd(ENABLE);
		SDIO_DataConfig(&SDIO_DataInitStructure);
	}

	SDIO_CmdInitStructure.SDIO_Argument = write ? 0x80000000 : 0x00000000;
	SDIO_CmdInitStructure.SDIO_Argument |= fn << 28;
	SDIO_CmdInitStructure.SDIO_Argument |= incr_addr ? 0x04000000 : 0x00000000;
	SDIO_CmdInitStructure.SDIO_Argument |= addr << 9;
	if (blocks == 1 && blksz < 256)
		SDIO_CmdInitStructure.SDIO_Argument |= /*(blksz == 256) ? 0 : */blksz;	/* byte mode */
	else
		SDIO_CmdInitStructure.SDIO_Argument |= 0x08000000 | blocks;		/* block mode */

	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_IO_RW_EXTENDED;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;

	SDIO_SendCommand(&SDIO_CmdInitStructure); 

	errorstatus = CmdResp5Error(SD_IO_RW_EXTENDED, &out);
	if (SD_OK != errorstatus)
	{
		p_err("mmc_io_rw_extended err:%d \n", errorstatus);

	}
	
	if(write)
	{
		SDIO_DataConfig(&SDIO_DataInitStructure); 
		DMA_TxConfiguration(tempbuff, TotalNumberOfBytes);
		errorstatus = dma_start_send_data(); //开始传送
	}
	else
	{
		ret = wait_event_timeout(sd_waitq, 100);  //阻塞1000ms
		if(ret != 0)
		{
			p_err("dma_read timeout:%08x\n", SDIO->STA);
			SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
			irq_flag = local_irq_save(); 
			SDIO->DCTRL &= DCTRL_CLEAR_MASK;
			local_irq_restore(irq_flag);
			SDIO_ClearFlag(SDIO_FLAG_ALL & (~SDIO_FLAG_SDIOIT));
			errorstatus = SD_ERROR;
		}
	}
	SDIO_DMACmd(DISABLE);
	if(errorstatus != SD_OK)
		p_err("mmc_iw_ew_externded err:%s,%d,%d\n",write?"w":"r", blocks, blksz);
	mutex_unlock(sd_mutex);

	return (errorstatus == SD_OK)?0:-1;
}

int mmc_send_relative_addr( u16 *rca)
{
	SD_Error errorstatus = SD_OK;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
	if (rca == 0)
		return SD_ERROR;

	SDIO_CmdInitStructure.SDIO_Argument = 0x00;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SET_REL_ADDR;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp6Error(SDIO_SET_REL_ADDR, rca);

	if (SD_OK != errorstatus)
	{
		p_err("mmc_send_relative_addr err:%d\n", errorstatus);
	}

	p_dbg("mmc_send_relative_addr ok:%x\n", *rca);
	return (errorstatus);
}

int sdio_read_cccr()
{
	int ret;
	int cccr_vsn;
	unsigned char data;

	memset(&cccr, 0, sizeof(struct sdio_cccr));

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_CCCR, 0, &data);
	if (ret != SD_OK)
		goto out;

	cccr_vsn = data & 0x0f;
	p_dbg("SDIO_CCCR_CCCR:%x\n", cccr_vsn);

	if (cccr_vsn > SDIO_CCCR_REV_1_20)
	{
		p_err("unrecognised CCCR structure version %d\n", cccr_vsn);
		return SD_ERROR;
	}

	cccr.sdio_vsn = (data & 0xf0) >> 4;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_CAPS, 0, &data);
	if (ret != SD_OK)
		goto out;

	p_dbg("SDIO_CCCR_CAPS:%x\n", data);
	if (data & SDIO_CCCR_CAP_SMB)
		cccr.multi_block = 1;
	if (data & SDIO_CCCR_CAP_LSC)
		cccr.low_speed = 1;
	if (data & SDIO_CCCR_CAP_4BLS)
		cccr.wide_bus = 1;

	if (cccr_vsn >= SDIO_CCCR_REV_1_10)
	{
		ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_POWER, 0, &data);

		p_dbg("SDIO_CCCR_POWER:%x\n", data);
		if (ret != SD_OK)
			goto out;

		if (data & SDIO_POWER_SMPC)
			cccr.high_power = 1;
	}

	if (cccr_vsn >= SDIO_CCCR_REV_1_20)
	{
		ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_SPEED, 0, &data);

		p_dbg("SDIO_CCCR_SPEED:%x\n", data);
		if (ret != SD_OK)
			goto out;

		if (data & SDIO_SPEED_SHS)
			cccr.high_speed = 1;
	}

out:
	if (ret != SD_OK)
		p_err("sdio_read_cccr err\n");
	return ret;
}

int mmc_sdio_switch_hs(int enable)
{
	int ret;
	u8 speed;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_SPEED, 0, &speed);
	if (ret != SD_OK)
	{
		p_err("mmc_sdio_switch_hs err:%d\n", ret);
		return ret;
	}
	if (enable)
		speed |= SDIO_SPEED_EHS;
	else
		speed &= ~SDIO_SPEED_EHS;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_SPEED, speed, NULL);
	if (ret != SD_OK)
		p_err("mmc_sdio_switch_hs err1:%d\n", ret);
	else
		p_dbg("mmc_sdio_switch_hs ok\n");

	return ret;
}

int sdio_enable_wide()
{
	int ret;
	u8 ctrl;

	ret = mmc_io_rw_direct(0, 0, SDIO_CCCR_IF, 0, &ctrl);
	if (ret != SD_OK)
	{
		p_err("sdio_enable_wide err:%d\n", ret);
		return ret;
	}
	ctrl |= SDIO_BUS_WIDTH_4BIT;

	ret = mmc_io_rw_direct(1, 0, SDIO_CCCR_IF, ctrl, NULL);

	if (ret != SD_OK)
		p_err("sdio_enable_wide err1:%d\n", ret);
	else
		p_dbg("sdio_enable_wide ok\n");

	return ret;
}

struct cis_tpl
{
	unsigned char code;
	unsigned char min_size;
	//	tpl_parse_t *parse;
};

struct sdio_func_tuple
{
	unsigned char code;
	unsigned char size;
	unsigned char data[128];
};

//struct sdio_func_tuple func_tuple[10];

static int sdio_read_cis(int f_n)
{
	int ret;
	struct sdio_func_tuple *this;
	unsigned i, ptr = 0, tuple_cnt = 0;
	unsigned char tpl_code, tpl_link;
	/*
	 * Note that this works for the common CIS (function number 0) as
	 * well as a function's CIS * since SDIO_CCCR_CIS and SDIO_FBR_CIS
	 * have the same offset.
	 */
	for (i = 0; i < 3; i++)
	{
		unsigned char x;

		ret = mmc_io_rw_direct(0, f_n,
				SDIO_FBR_BASE(f_n) + SDIO_FBR_CIS + i, 0, &x);
		if (ret)
			return ret;
		ptr |= x << (i * 8);
	}

	p_dbg("read_cis,fn:%d,addr:%d\n", f_n, ptr);
	
	do
	{
		ret = mmc_io_rw_direct(0, f_n, ptr++, 0, &tpl_code);
		if (ret)
			break;

		/* 0xff means we're done */
		if (tpl_code == 0xff)
			break;

		/* null entries have no link field or data */
		if (tpl_code == 0x00)
			continue;

		ret = mmc_io_rw_direct(0, f_n, ptr++, 0, &tpl_link);
		if (ret)
			break;

		/* a size of 0xff also means we're done */
		if (tpl_link == 0xff)
			break;

		p_info("tpl code:%x,size:%d\n", tpl_code, tpl_link);


		if (tuple_cnt > 9 || tpl_link > 128)
		{
			p_dbg("tuple_cnt over\n");
			break;
		}
//		func_tuple[tuple_cnt].size = tpl_link;
		
		this = (struct sdio_func_tuple *)mem_malloc(sizeof(*this) + tpl_link);
		if (!this)
			return -ENOMEM;

		for (i = 0; i < tpl_link; i++)
		{
			ret = mmc_io_rw_direct(0, f_n,
					ptr + i, 0, &this->data[i]);
			if (ret)
				break;
		}

		dump_hex("cis", this->data, i);

		if (ret)
		{
			mem_free(this);
			break;
		}
		if(tpl_code == CISTPL_VERS_1)
			p_info("%s\n", this->data + 2);
		
		mem_free(this);

		ptr += tpl_link;
		tuple_cnt += 1;
	}
	while (!ret);

	if (tpl_link == 0xff)
		ret = 0;

	return ret;
}

int sdio_read_common_cis()
{
	return sdio_read_cis(NULL);
}

