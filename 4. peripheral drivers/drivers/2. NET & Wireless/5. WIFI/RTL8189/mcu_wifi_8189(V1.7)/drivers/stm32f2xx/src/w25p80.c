#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_WRSR		0x01	/* Write status register 1 byte */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* Used for SST flashes only. */
#define	OPCODE_BP		0x02	/* Byte program */
#define	OPCODE_WRDI		0x04	/* Write disable */
#define	OPCODE_AAI_WP		0xad	/* Auto address increment word program */

/* Used for Macronix flashes only. */
#define	OPCODE_EN4B		0xb7	/* Enter 4-byte mode */
#define	OPCODE_EX4B		0xe9	/* Exit 4-byte mode */

/* Used for Spansion flashes only. */
#define	OPCODE_BRWR		0x17	/* Bank register write */

/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_JIFFIES	(40 * 100)	/* M25P16 specs 40s max chip erase */
#define	MAX_CMD_SIZE		5

#ifdef CONFIG_M25PXX_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#define JEDEC_MFR(_jedec_id)	((_jedec_id) >> 16)

/****************************************************************************/

struct m25p {
	mutex_t	lock;
	uint32_t 	size;
	u16			sector_size;
	u16			page_size;
	u16			addr_width;
	u8			enable;
	u8			command[6];
};

struct m25p m25_flash;



int spi_r(char *buff, int size)
{
	int i;
	if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
		i = SPI_I2S_ReceiveData(SPI2);
	
	for(i = 0; i < size; i++)
	{
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
		SPI_I2S_SendData(SPI2, 0xff);
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		buff[i] = SPI_I2S_ReceiveData(SPI2);
	}
	return 0;
}


int spi_w(char *buff, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		while(!(SPI2->SR & SPI_I2S_FLAG_TXE));
		SPI2->DR = buff[i];
	}
	while(!(SPI2->SR & SPI_I2S_FLAG_TXE));
	while(SPI2->SR & SPI_I2S_FLAG_BSY);
	
	return 0;
}

int spi_write(char *buff, int size)
{
	FLASH_CS_ENABLE;
	spi_w(buff, size);
	FLASH_CS_DISABLE;
	return 0;
}

int spi_write_then_write(char *w1_buff, int w1_size, char *w2_buff, int w2_size)
{
	FLASH_CS_ENABLE;
	spi_w(w1_buff, w1_size);
	spi_w(w2_buff, w2_size);
	FLASH_CS_DISABLE;
	return 0;
}


int spi_write_then_read(char *w_buff, int w_size, char *r_buff, int r_size)
{
	FLASH_CS_ENABLE;
	
	spi_w(w_buff, w_size);

	delay_us(1);
	
	spi_r(r_buff, r_size);

	FLASH_CS_DISABLE;

	return 0;
}


/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;
	u8 val;

	retval = spi_write_then_read((char*)&code, 1, (char*)&val, 1);

	if (retval < 0) {
		p_err("error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}


/*
 * Write status register 1 byte
 * Returns negative if error occurred.
 */
 /*
static int write_sr(struct m25p *flash, u8 val)
{
	flash->command[0] = OPCODE_WRSR;
	flash->command[1] = val;

	return spi_write((char*)flash->command, 2);
}
*/
/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
static int write_enable(struct m25p *flash)
{
	u8	code = OPCODE_WREN;

	return spi_write_then_read((char*)&code, 1, NULL, 0);
}

/*
 * Send write disble instruction to the chip.
 */
/*
static int write_disable(struct m25p *flash)
{
	u8	code = OPCODE_WRDI;

	return spi_write_then_read((char*)&code, 1, NULL, 0);
}
*/
/*
 * Enable/disable 4-byte addressing mode.
 */
 #if 0
static inline int set_4byte(struct m25p *flash, u32 jedec_id, int enable)
{
	switch (JEDEC_MFR(jedec_id)) {
	case CFI_MFR_MACRONIX:
		flash->command[0] = enable ? OPCODE_EN4B : OPCODE_EX4B;
		return spi_write(flash->spi, flash->command, 1);
	default:
		/* Spansion style */
		flash->command[0] = OPCODE_BRWR;
		flash->command[1] = enable << 7;
		return spi_write(flash->spi, flash->command, 2);
	}
}
#endif
/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct m25p *flash)
{
	unsigned long deadline;
	int sr;

	deadline = jiffies + MAX_READY_WAIT_JIFFIES;

	do {
		if ((sr = read_sr(flash)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;

//		cond_resched();
		sleep(10);

	} while (jiffies < deadline);

	p_err("wait_till_ready err");

	return 1;
}

/*
 * Erase the whole flash memory
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_chip(struct m25p *flash)
{
	p_dbg("%s: %s %lldKiB\n",
	      "", __FUNCTION__,
	      (long long)(flash->size >> 10));

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_CHIP_ERASE;

	spi_write((char*)flash->command, 1);

	return 0;
}

static void m25p_addr2cmd(struct m25p *flash, unsigned int addr, u8 *cmd)
{
	/* opcode is in cmd[0] */
	cmd[1] = addr >> (flash->addr_width * 8 -  8);
	cmd[2] = addr >> (flash->addr_width * 8 - 16);
	cmd[3] = addr >> (flash->addr_width * 8 - 24);
	cmd[4] = addr >> (flash->addr_width * 8 - 32);
}

static int m25p_cmdsz(struct m25p *flash)
{
	return 1 + flash->addr_width;
}

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct m25p *flash, u32 offset)
{
	
	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = OPCODE_BE_4K;
	m25p_addr2cmd(flash, offset, flash->command);

	spi_write((char*)flash->command, m25p_cmdsz(flash));

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
#define MTD_ERASE_DONE 		0
#define MTD_ERASE_FAILED 	1


int m25p80_erase(u32 addr, int len)
{
	struct m25p *flash = &m25_flash;
	uint32_t state;

	p_dbg("%s: %s %s 0x%x, len %d\n",
	      "", __FUNCTION__, "at",
	      addr, len);

	/* sanity checks */
	if (addr + len > flash->size)
		return -EINVAL;

	//mutex_lock(flash->lock);
	FLASH_WP_DISABLE;

	/* whole-chip erase? */
	if (len == flash->size) {
		if (erase_chip(flash)) {
			state = MTD_ERASE_FAILED;
			//mutex_unlock(flash->lock);
			FLASH_WP_ENABLE;
			return -EIO;
		}

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K.  We may have set up
	 * to use "small sector erase", but that's not always optimal.
	 */

	/* "sector"-at-a-time erase */
	} else {
		while (len > 0) {
			if (erase_sector(flash, addr)) {
				state = MTD_ERASE_FAILED;
				//mutex_unlock(flash->lock);
				FLASH_WP_ENABLE;
				return -EIO;
			}

			addr += flash->sector_size;
			len -= flash->sector_size;
		}
	}

	//mutex_unlock(flash->lock);

	state = MTD_ERASE_DONE;

	FLASH_WP_ENABLE;

	return state;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
int m25p80_read(u32 from, size_t len, u_char *buf)
{
	struct m25p *flash = &m25_flash;
	
	p_dbg("%s: %s %s 0x%08x, len %zd\n",
			"", __FUNCTION__, "from",
			(u32)from, len);

	/* sanity checks */
	if (!len)
		return 1;

	if (from + len > flash->size)
		return -EINVAL;

	//mutex_lock(flash->lock);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		//mutex_unlock(flash->lock);
		return 1;
	}

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	m25p_addr2cmd(flash, from, flash->command);

	spi_write_then_read((char*)flash->command, m25p_cmdsz(flash) + FAST_READ_DUMMY_BYTE, (char*)buf,len);

	//mutex_unlock(flash->lock);

	return 0;
}


int cmp_flash_data(int addr, int size, const u_char *buff)
{
	struct m25p *flash = &m25_flash;
	u_char *check_buff = 0, check_cmd[6];

	check_buff = (u_char*)mem_malloc(size);
	if(!check_buff)
	{
		return -1;
	}
	wait_till_ready(flash);
	check_cmd[0] = OPCODE_READ;
	m25p_addr2cmd(flash, addr, check_cmd);
	spi_write_then_read((char*)check_cmd, 
		m25p_cmdsz(flash) + FAST_READ_DUMMY_BYTE, 
		(char*)check_buff,
		size);
	
	if(memcmp(buff, check_buff, size))
	{	
		p_err("check_flash_data err");
		p_hex(buff, size);
		p_hex(check_buff, size);
		mem_free(check_buff);
		return -1;
	}
	
	mem_free(check_buff);
	return 0;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
int m25p80_write(u32 to, size_t len,const u_char *buf)
{
	struct m25p *flash = &m25_flash;
	u32 page_offset, page_size;
	
	int ret = 0;

	p_dbg("%s: %s %s 0x%08x, len %zd\n",
			"", __FUNCTION__, "to",
			(u32)to, len);
	
	//mutex_lock(flash->lock);
	FLASH_WP_DISABLE;;
	
	/* sanity checks */
	if (!len)
	{
		ret = -1;
		goto end;
	}

	if (to + len > flash->size)
	{
		ret = -1;
		goto end;
	}

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		
		ret = -1;
		goto end;
	}
	
	write_enable(flash);

	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	m25p_addr2cmd(flash, to, flash->command);

	page_offset = to & (flash->page_size - 1);

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= flash->page_size) {

		spi_write_then_write((char*)flash->command, m25p_cmdsz(flash), (char*)buf, len);
/*
		if(cmp_flash_data(to, len, buf))
		{
			ret = -1;
			goto end;
		}*/
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = flash->page_size - page_offset;

		spi_write_then_write((char*)flash->command, m25p_cmdsz(flash), (char*)buf, page_size);
/*
		if(cmp_flash_data(to, page_size, buf))
		{
			ret = -1;
			goto end;
		}
*/
		/* write everything in flash->page_size chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > flash->page_size)
				page_size = flash->page_size;

			/* write the next page to flash */
			m25p_addr2cmd(flash, to + i, flash->command);

			wait_till_ready(flash);

			write_enable(flash);

			spi_write_then_write((char*)flash->command, m25p_cmdsz(flash), (char*)buf + i, page_size);
/*
			if(cmp_flash_data(to + i, page_size, buf + i))
			{
				ret = -1;
				goto end;
			}*/
		}
	}
	
end:
	if(ret)
		p_err("m25p80 write err");
	FLASH_WP_ENABLE;
	//mutex_unlock(flash->lock);

	return ret;
}

//{ "w25x80", INFO(0xef3014, 0, 64 * 1024,  16, SECT_4K) },

uint32_t m25p80_read_id()
{
	struct m25p *flash = &m25_flash;
	uint32_t id;

	//mutex_lock(flash->lock);

	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		//mutex_unlock(flash->lock);
		return 0;
	}

	flash->command[0] = OPCODE_RDID;

	spi_write_then_read((char*)flash->command, 1, (char*)&id, 3);

	//mutex_unlock(flash->lock);
	
	return id;
}

#if 0
int read_firmware(u32 offset, u32 len, u8 * pbuf)
{
	if(!m25_flash.enable)
	{
		p_err("flash not available");
		return -1;
	}

	return m25p80_read(FIRMWARE_BASE_ADDR + offset, len, pbuf);
}

extern const unsigned char fm_88w8287[249580];
int test_write_firmware_to_spi_flash()
{
	int ret;
	uint32_t fm_size = (sizeof(fm_88w8287) + 255)/256*256;

	p_dbg("开始写入固件，大小:%d", fm_size);
	if(!m25_flash.enable)
	{
		p_err("flash not available");
		return -1;
	}

	ret = m25p80_erase(FIRMWARE_BASE_ADDR, fm_size);
	if(ret){
		p_err("flash erase err");
		return -1;
	}
	
	ret = m25p80_write(FIRMWARE_BASE_ADDR, fm_size, fm_88w8287);
	if(ret){
		p_err("flash write err");
		return -1;
	}

	p_dbg("固件写入成功");

	return 0;
}
#endif

#define W25Q80_ID 0x1440ef
int  m25p80_init(void)
{
	uint32_t id;
	SPI_InitTypeDef  SPI_InitStruct;
	
	p_dbg_enter;

	memset(&m25_flash, 0, sizeof(struct m25p));
	m25_flash.addr_width = 3;
	m25_flash.sector_size = 4*1024;
	m25_flash.page_size = 256;
	m25_flash.enable = TRUE;
	m25_flash.size = FLASH_ROOM_SIZE;	//1mBYTE
	m25_flash.lock = mutex_init(__FUNCTION__);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	gpio_cfg((uint32_t)FLASH_CS_PORT, FLASH_CS_PIN, GPIO_Mode_Out_PP);

	gpio_cfg((uint32_t)FLASH_CLK_PORT, FLASH_CLK_PIN, GPIO_Mode_AF_PP);
	gpio_cfg((uint32_t)FLASH_MISO_PORT, FLASH_MISO_PIN, GPIO_Mode_AF_IPU);
	gpio_cfg((uint32_t)FLASH_MOSI_PORT, FLASH_MOSI_PIN, GPIO_Mode_AF_PP);
	gpio_cfg((uint32_t)FLASH_WP_PORT, FLASH_WP_PIN, GPIO_Mode_Out_PP);

	GPIO_PinAFConfig(FLASH_CLK_PORT, FLASH_CLK_PIN_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(FLASH_MISO_PORT, FLASH_MISO_PIN_SOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(FLASH_MOSI_PORT, FLASH_MOSI_PIN_SOURCE, GPIO_AF_SPI2);
	
	FLASH_CS_DISABLE;
	
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_CPHA =SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_Init(SPI2, &SPI_InitStruct);

	SPI_Cmd(SPI2, ENABLE);

	
	id = m25p80_read_id();
	p_dbg("flash id:%08x", id);

	if(id != W25Q80_ID)
	{
		m25_flash.enable = FALSE;
		return -1;
	}

	return 0;
}


void SPI2_IRQHandler()
{
	SPI_I2S_ClearITPendingBit(SPI2, SPI_I2S_IT_ERR);
}


