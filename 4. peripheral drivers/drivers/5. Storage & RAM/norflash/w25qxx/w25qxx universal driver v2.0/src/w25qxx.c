/**
 * Copyright (c) 2015 LibDriver, All rights reserved
 * Copyright (c) 2022 Serenity Island Engineering, All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file		w25qxx.c
 * @brief		w25qxx driver source file
 * @version		2.0.0
 * @author		Nikita Kuchinskiy
 * @date		2022-07-05
 *
 * <style>table, th, td {border:1px solid black;border-collapse:collapse} table {width:50%} th {text-align:left}</style>
 * <h3>history</h3>
 * <table>
 *   <tr><th>Date</th><th>Version</th><th>Author</th><th>Description</th></tr>
 *   <tr><td>2021/07/15</td><td>1.0.0</td><td>Shifeng Li</td><td>first upload</td></tr>
 *   <tr><td>2022/07/05</td><td>2.0.0</td><td>Nikita Kuchinskiy</td><td>Radically optimized + interface and API changed</td></tr>
 * </table>
 *
 * 1 tab = 4 spaces
 *
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "w25qxx.h"

#define DRIVER_VERSION				0x0200

#define SPI_READ_ERR_VAL			0xFF00
#define CHECK_DEV()					{ } /* { if (dev == NULL) { return W25QXX_RET_ERR_NULL_PTR; };---[useless since using attribute] */ /* if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };----[needed not everywhere] */
#define DEV_SPI_MODE_MASK			0xC0
#define DEV_SPI_MODE_SPI			0x00
#define MIN(x,y)					(((x) < (y))?(x):(y))
#define MAX(x,y)					(((x) > (y))?(x):(y))
#define DELAY_STEP					10		// check state every X us or ms
#define DELAY_USE_MS_OVER_US		1000	// when 'x' > 'max_val', use 'delay_ms(x/max_val)' instead of delay_us(x)
#define RESET_RECOVER_MS			10


#ifdef W25QXX_DEBUG
	extern int printf_dbg(const char *restrict format, ...);
#	define printf_dbg_ printf_dbg
//#	include <stdio.h>
//#	define printf_dbg_ printf
#elif !defined(printf_dbg_)
#	define printf_dbg_(...)
#endif


enum MANUF_e {
	MANUF_WINBOND,
	MANUF_MACRONIX,
	MANUF_UNK,
};
enum MANUFACTURER_ID_e {
	MANUFACTURER_ID_MACRONIX	= 0xC2,
	MANUFACTURER_ID_WINBOND		= 0xEF,
};

static const char *const manuf_text[] = {
	[MANUF_WINBOND] = "WINBOND",
	[MANUF_MACRONIX] = "MACRONIX",
	[MANUF_UNK] = "UNKNOWN",
};

typedef void (*delay_func)(uint32_t);
// these functions names are quite standard, so let's keep them as 'extern's for lower memory usage of 'w25qxx_dev_t'
extern void delay_us(uint32_t microsec);
extern void delay_ms(uint32_t millisec);

// ARM processors allow performing following operations as 1-tick instructions.
#ifdef __GNUC__
#define bsd_rev16 __builtin_bswap16
#define bsd_rev32 __builtin_bswap32
#else
extern unsigned short bsd_rev16(unsigned short val);
extern unsigned long  bsd_rev32(unsigned long val);
//extern unsigned long  bsd_rbit32(unsigned long val); // not used in this lib
__attribute__((weak)) unsigned short bsd_rev16(unsigned short val) { return (((val & 0xFF) >> 8) | ((val & 0xFF) << 8)); };
__attribute__((weak)) unsigned long  bsd_rev32(unsigned long val) { return (((val & 0xFF) >> 24) | ((val & 0xFF) >> 16) | ((val & 0xFF) >> 8) | ((val & 0xFF) << 8)); };
// Example for ARM + CMSIS:
//unsigned short bsd_rev16(unsigned short val) { return __REV16(val); };
//unsigned long  bsd_rev32(unsigned long  val) { return __REV(val); };
#endif

#ifdef W25QXX_DYNAMIC_OVERWRITE_BUFFER
// These functions are not specific to particular chip interface, so let's keep them as 'extern's too for portability and lower memory usage of 'w25qxx_dev_t'
// Example1: void w25qxx_free(void *buf) { vPortFree(buf); }; // direct FreeRTOS binding
// Example2: void *w25qxx_malloc(size_t size) __attribute__((alias("bsd_malloc"))); // indirect RTOS binding
extern void *w25qxx_malloc(size_t size);
extern void w25qxx_free(void *buf);
__attribute__((weak)) void *w25qxx_malloc(size_t size) { return malloc(size); };
__attribute__((weak)) void w25qxx_free(void *buf) { free(buf); buf = NULL; }; // NOTE: no need to check for 'NULL' here, as 'free()' handles that case properly internally
#endif

//extern const char *const fmt_func_fail; 			__attribute__((weak)) const char *const fmt_func_fail			= "%s(): %s(): Failed\r\n";
extern const char *const fmt_func_fail_ret;			__attribute__((weak)) const char *const fmt_func_fail_ret 		= "%s(): %s(): Failed: %02X\r\n";
extern const char *const fmt_func_fail_ret_desc;	__attribute__((weak)) const char *const fmt_func_fail_ret_desc	= "%s(): %s(): Failed: %02X ('%s')\r\n";

#define W25QXX_RET_TEXT(x)			[W25QXX_RET_ ## x] = #x
static const char *w25qxx_ret_text[] = {
	W25QXX_RET_TEXT(SUCCESS),
	W25QXX_RET_TEXT(ERR_IO_FAIL),
	W25QXX_RET_TEXT(ERR_NULL_PTR),
	W25QXX_RET_TEXT(ERR_UNINITIALIZED),
	W25QXX_RET_TEXT(ERR_INVALID_DATA),
	W25QXX_RET_TEXT(ERR_TIMEOUT),
	W25QXX_RET_TEXT(ERR_INVALID_MODE),
	W25QXX_RET_TEXT(ERR_FORBIDDEN),
	W25QXX_RET_TEXT(ERR_INVALID_STATE),
	W25QXX_RET_TEXT(ERR_OVERFLOW),
	W25QXX_RET_TEXT(ERR_NO_MEM),
	W25QXX_RET_TEXT(ERR_DATA_CORRUPTED),
};
static const char *const intf_name = "QSPI";

#define W25QXX_CHIP_TEXT(x)			[x] = #x
static const char *const chip_name[] = {
	W25QXX_CHIP_TEXT(W25Q10),
	W25QXX_CHIP_TEXT(W25Q20),
	W25QXX_CHIP_TEXT(W25Q40),
	W25QXX_CHIP_TEXT(W25Q80),
	W25QXX_CHIP_TEXT(W25Q16),
	W25QXX_CHIP_TEXT(W25Q32),
	W25QXX_CHIP_TEXT(W25Q64),
	W25QXX_CHIP_TEXT(W25Q128),
	W25QXX_CHIP_TEXT(W25Q256),
	W25QXX_CHIP_TEXT(W25Q512),
};

typedef union {
	__INT8_TYPE__    i8;
	__INT16_TYPE__   i16;
	__INT32_TYPE__   i32;
	int              i;
	long             l;

	__UINT8_TYPE__   u8;
	__UINT16_TYPE__  u16;
	__UINT32_TYPE__  u32;
	unsigned int     ui;
	unsigned long    ul;
	float            f;

	__INT8_TYPE__    I8[4];
	__INT16_TYPE__   I16[2];
	__INT32_TYPE__   I32[1];
	int              I[1];
	long             L[1];

	__UINT8_TYPE__   U8[4];
	__UINT16_TYPE__  U16[2];
	__UINT32_TYPE__  U32[1];
	unsigned int     UI[1];
	unsigned long    UL[1];

	float            F[1];

	__SIZE_TYPE__    sz;
	__INTPTR_TYPE__  p;
	__PTRDIFF_TYPE__ pd;
	_Bool            b;
} ub_t; // unified buffer type

enum W25QXX_TIMEOUT_e {
	W25QXX_TIMEOUT_US_ERASE_BLOCK_4K		= 400000,
	W25QXX_TIMEOUT_US_ERASE_BLOCK_32K		= 1600000,
	W25QXX_TIMEOUT_US_ERASE_BLOCK_64K		= 2000000,
	W25QXX_TIMEOUT_US_ERASE_SECURITY_REG	= 10000,
	W25QXX_TIMEOUT_US_WRITE_STATUS_REG1		= 1000000,
	W25QXX_TIMEOUT_US_WRITE_BLOCK_256		= 300,
	W25QXX_TIMEOUT_US_WRITE_SECURITY_REG	= W25QXX_TIMEOUT_US_WRITE_BLOCK_256,
};
#define W25QXX_TIMEOUT_US_ERASE_ALL			4e8f // 400000000.0f // NOTE: using float. Otherwise, requires 64 bits to operate as integer

enum	W25QXX_CMD_e {
	W25QXX_CMD_WRITE_ENABLE					= 0x06, // write enable
	W25QXX_CMD_WRITE_DISABLE				= 0x04, // write disable
	W25QXX_CMD_WRITE_ENABLE_VOLATILE_SR		= 0x50, // sr write enable

	W25QXX_CMD_READ_STATUS_REG1				= 0x05, // read status register-1
	W25QXX_CMD_READ_STATUS_REG2				= 0x35, // read status register-2
	W25QXX_CMD_READ_STATUS_REG3				= 0x15, // read status register-3
	W25QXX_CMD_WRITE_STATUS_REG1			= 0x01, // write status register-1
	W25QXX_CMD_WRITE_STATUS_REG2			= 0x31, // write status register-2
	W25QXX_CMD_WRITE_STATUS_REG3			= 0x11, // write status register-3

	W25QXX_CMD_BLOCK_WRITE_256				= 0x02, // page program
	W25QXX_CMD_BLOCK_WRITE_256_QUAD_IO		= 0x32, // page program using quad I/O for shifting data (only!)

	W25QXX_CMD_BLOCK_ERASE_4K				= 0x20, // sector erase
	W25QXX_CMD_BLOCK_ERASE_32K				= 0x52, // block erase
	W25QXX_CMD_BLOCK_ERASE_64K				= 0xD8, // block erase
	W25QXX_CMD_CHIP_ERASE					= 0xC7, // chip erase
	W25QXX_CMD_ERASE_WRITE_SUSPEND			= 0x75, // erase suspend
	W25QXX_CMD_ERASE_WRITE_RESUME			= 0x7A, // erase resume

	/* NOTE: Below are some max clock frequencies for chips and instructions
	+----------------------------------------------------------------------------------------+
	| W25Q64  | For all instructions, except Read Data (03h) & Octal Word Read (E3h)|  80 MHz|
	|         | For Octal Word Read (E3h)											|  50 MHz|
	|         | For Read Data (03h)													|  33 MHz|
	+----------------------------------------------------------------------------------------+
	| W25Q256 | For all instructions, except Read Data (03h)						| 104 MHz|
	|         | For Read Data (03h)													|  50 MHz|
	+---------------------------------------------------------------------------------------*/
	W25QXX_CMD_READ_DATA					= 0x03, // read data
	W25QXX_CMD_READ_DATA_4BA				= 0x13, // read data, using 4-byte address, avail on W25Q256+
	W25QXX_CMD_READ_DATA_OCTAL_WORD_QUAD_IO	= 0xE3, // octal word read quad I/O
	W25QXX_CMD_READ_DATA_WORD_QUAD_IO		= 0xE7, // word read quad I/O
	W25QXX_CMD_READ_DATA_FAST				= 0x0B, // fast read // allows higher clocking
	W25QXX_CMD_READ_DATA_FAST_DUAL_OUT		= 0x3B, // fast read dual output
	W25QXX_CMD_READ_DATA_FAST_QUAD_OUT		= 0x6B, // fast read quad output
	W25QXX_CMD_READ_DATA_FAST_DUAL_IO		= 0xBB, // fast read dual I/O
	W25QXX_CMD_READ_DATA_FAST_QUAD_IO		= 0xEB, // fast read quad I/O

	W25QXX_CMD_READ_UNIQUE_ID				= 0x4B, // read unique id
	W25QXX_CMD_READ_MANUFACTURER			= 0x90, // manufacturer
	W25QXX_CMD_DEVICE_ID_DUAL_IO			= 0x92, // device id dual I/O
	W25QXX_CMD_DEVICE_ID_QUAD_IO			= 0x94, // device id quad I/O
	W25QXX_CMD_JEDEC_ID						= 0x9F, // jedec id

	W25QXX_CMD_READ_SFDP_REG				= 0x5A, // read SFDP register

	W25QXX_CMD_ERASE_SECURITY_REG			= 0x44, // erase security register
	W25QXX_CMD_WRITE_SECURITY_REG			= 0x42, // program security register
	W25QXX_CMD_READ_SECURITY_REG			= 0x48, // read security register

	W25QXX_CMD_BLOCK_LOCK					= 0x36, // individual block lock
	W25QXX_CMD_BLOCK_UNLOCK					= 0x39, // individual block unlock
	W25QXX_CMD_BLOCK_READ_LOCK				= 0x3D, // read block lock
	W25QXX_CMD_BLOCK_LOCK_GLOBAL			= 0x7E, // global block lock
	W25QXX_CMD_BLOCK_UNLOCK_GLOBAL			= 0x98, // global block unlock

	W25QXX_CMD_3BYTE_ADDR_MODE				= 0xE9, // avail on W25Q256+
	W25QXX_CMD_4BYTE_ADDR_MODE				= 0xB7, // avail on W25Q256+
	W25QXX_CMD_WRITE_EXT_ADDR				= 0xC5, // avail on W25Q256+

	W25QXX_CMD_POWER_DOWN					= 0xB9, // power down
	W25QXX_CMD_RELEASE_POWER_DOWN			= 0xAB, // release power down
	W25QXX_CMD_ENABLE_RESET					= 0x66, // enable reset
	W25QXX_CMD_RESET_DEVICE					= 0x99, // reset device

	W25QXX_CMD_QPI_MODE_BEG					= 0x38, // enter qpi mode (W25Q256)
	W25QXX_CMD_QPI_MODE_END					= 0xFF, // exit qpi mode  (W25Q256)
	W25QXX_CMD_CONTINUOUS_READ_MODE_RESET	= 0xFF, // send 0xFFFF for Dual I/O Mode (W25Q64)
	W25QXX_CMD_SET_READ_PARAMETERS			= 0xC0, // avail on W25Q256+, config QPI / QSPI data reading parameters
	W25QXX_CMD_SET_BURST_WITH_WRAP			= 0x77, // set burst with wrap
};


#define STATUS_REG_WRITE_FLAG 0x80
static uint8_t regid_to_cmd(uint8_t reg_id) {

	// WARNING: Not all STATUS_REGs are available across all cihps. At lest, not documented.
	// TODO: Check which 'STATUS_REG's are awailable for which chip, fill the table below and implement related error checks.

	// NOTE: CMD = 0/3/1(reg1/reg2/reg3)<<8 + 5/1(read/write)
	uint8_t cmd = W25QXX_CMD_READ_STATUS_REG1;
	if (reg_id & STATUS_REG_WRITE_FLAG) { reg_id &= ~STATUS_REG_WRITE_FLAG; cmd = W25QXX_CMD_WRITE_STATUS_REG1; };

	if (reg_id == 1) { return cmd; };
	if (reg_id == 2) { return cmd | W25QXX_CMD_WRITE_STATUS_REG2; };
	if (reg_id == 3) { return cmd | W25QXX_CMD_WRITE_STATUS_REG3; };

	return 0x00; // Indicate (elevate) error in case of invalid 'reg_id'

	/* old code
	if (reg_id & STATUS_REG_WRITE_FLAG) {
		reg_id &= ~STATUS_REG_WRITE_FLAG;
		if (reg_id == 1) { return W25QXX_CMD_WRITE_STATUS_REG1; };
		if (reg_id == 2) { return W25QXX_CMD_WRITE_STATUS_REG2; };
		if (reg_id == 3) { return W25QXX_CMD_WRITE_STATUS_REG3; };
		return 0x00;
	};

	if (reg_id == 1) { return W25QXX_CMD_READ_STATUS_REG1; };
	if (reg_id == 2) { return W25QXX_CMD_READ_STATUS_REG2; };
	if (reg_id == 3) { return W25QXX_CMD_READ_STATUS_REG3; };
	return 0x00;
	*/

};
__attribute__((nonnull(1,2))) static uint8_t  w25qxx_write_read_buf(const w25qxx_dev_t *const dev, const void *const cmd_buf, const uint16_t cmd_len, void *const buf_io, const uint16_t len_io) {
	// NOTE: this function, is shifting content of 'buf_io[x]' into SPI and then immediatelly overwrites 'buf_io[x]' with incoming byte;
	uint8_t ret = W25QXX_RET_SUCCESS;

	if ((dev->intf == W25QXX_INTF_SPI) && !dev->dual_quad_spi_enabled) {
		ret = dev->spi_data_write_read(cmd_buf, cmd_len, buf_io, len_io);
	} else {
		// WARNING: QSPI not tested!
		const uint8_t QSPI_lines = (dev->dual_quad_spi_enabled)?(1):(4);

		//ret = dev->qspi_write_read_ext(((uint8_t *)cmd_buf)[0], QSPI_lines, 0x00000000, QSPI_lines, 0, 0x00000000, QSPI_lines, 0x00, dev->dummy, buf_io, len_io, buf_io, len_io, QSPI_lines);
		//Alternative:
		ret = dev->qspi_write_read_ext(((uint8_t *)cmd_buf)[0], QSPI_lines, 0x00000000, QSPI_lines, 0, 0x00000000, QSPI_lines, 0x00, dev->dummy, buf_io+1, len_io-1, buf_io, len_io, QSPI_lines);

		// let's keep it simple for now
		//ret = dev->qspi_write_read(QSPI_lines, cmd_buf, cmd_len, buf_io, len_io);
	};

	return ret;
};
__attribute__((nonnull(1,2))) static uint8_t  w25qxx_write_buf(const w25qxx_dev_t *const dev, const void *const cmd_buf, const uint16_t cmd_len, const void *const buf_out, const uint16_t len_out) {
	// NOTE: this function, is shifting content of 'buf_io[x]' into SPI and then immediatelly overwrites 'buf_io[x]' with incoming byte;
	uint8_t ret = W25QXX_RET_SUCCESS;

	if ((dev->intf == W25QXX_INTF_SPI) && !dev->dual_quad_spi_enabled) {
		ret = dev->spi_data_write(cmd_buf, cmd_len, buf_out, len_out);
	} else {
		// WARNING: QSPI not tested!
		//const uint8_t QSPI_lines = (dev->dual_quad_spi_enabled)?(1):(4);

		//ret = dev->qspi_write_read_ext(((uint8_t *)buf_out)[0], QSPI_lines, 0x00000000, QSPI_lines, 0, 0x00000000, QSPI_lines, 0x00, dev->dummy, buf_in, len_in, buf_in, len_in, QSPI_lines);
		//Alternative:
		//ret = dev->qspi_write_read_ext(((uint8_t *)buf_out)[0], QSPI_lines, 0x00000000, QSPI_lines, 0, 0x00000000, QSPI_lines, 0x00, dev->dummy, buf_out+1, len_out-1, buf_in, len_in, QSPI_lines);

		// let's keep it simple for now
		//ret = dev->qspi_write_read(QSPI_lines, cmd_buf, cmd_len, buf_out, len_out);
		ret = W25QXX_RET_ERR_IO_FAIL;
	};

	return ret;
};

__attribute__((nonnull)) static inline W25QXX_RET_t w25qxx_cmd(const w25qxx_dev_t *const dev, uint8_t cmd) { return w25qxx_write_read_buf(dev, &cmd, 1, NULL, 0); };
__attribute__((nonnull(1))) static W25QXX_RET_t w25qxx_wait_erase_write_finished(const w25qxx_dev_t *const dev, float timeout_us) {
	W25QXX_RET_t ret;
	delay_func delay = delay_us;
	int32_t itimeout = timeout_us;

	if (timeout_us >= DELAY_USE_MS_OVER_US) { delay = delay_ms; itimeout = timeout_us / DELAY_USE_MS_OVER_US; };

	uint8_t buf = W25QXX_STATUS1_ERASE_WRITE_BUSY;
	for ( ; itimeout > 0; itimeout -= DELAY_STEP, delay(DELAY_STEP)) {
		buf = W25QXX_CMD_READ_STATUS_REG1;
		ret = w25qxx_write_read_buf(dev, &buf, 1, &buf, 1);
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_write_read_buf", ret, w25qxx_ret2str(ret)); return ret; };
		if ((buf & W25QXX_STATUS1_ERASE_WRITE_BUSY) == 0) { return W25QXX_RET_SUCCESS; };
	};

	printf_dbg_("%s(): timeout occured\r\n", __func__);
	return W25QXX_RET_ERR_TIMEOUT;
};
__attribute__((nonnull(1))) static W25QXX_RET_t w25qxx_cmd_addr(w25qxx_dev_t *const dev, uint8_t cmd, uint32_t addr, void *data_io, const void *data_out, uint16_t data_len) {
	W25QXX_RET_t ret;
	uint8_t buf[6];

	/* cmd summary
	W25QXX_CMD_READ_DATA;					// no timeout			no dummy byte!
	W25QXX_CMD_FAST_READ;					// no timeout			dummy byte after 'addr'
	W25QXX_CMD_SECTOR_ERASE_4K;				// timeout = 400ms		no dummy byte
	W25QXX_CMD_BLOCK_ERASE_32K;				// timeout = 1600ms		no dummy byte
	W25QXX_CMD_BLOCK_ERASE_64K;				// timeout = 2000ms		no dummy byte
	W25QXX_CMD_PAGE_PROGRAM;				// timeout = 300us		no dummy byte
	W25QXX_CMD_READ_UNIQUE_ID;				// no timeout			no dummy byte				addr = 0x00000000
	W25QXX_CMD_READ_SECURITY_REGISTER;		// no timeout			dummy byte after 'addr'
	W25QXX_CMD_PROGRAM_SECURITY_REGISTER;	// timeout = 300us		no dummy byte
	W25QXX_CMD_ERASE_SECURITY_REGISTER;		// timeout = 10000us	no dummy byte
	W25QXX_CMD_READ_BLOCK_LOCK;				// no timeout			no dummy byte
	W25QXX_CMD_INDIVIDUAL_BLOCK_LOCK;		// no timeout			no dummy byte
	W25QXX_CMD_INDIVIDUAL_BLOCK_UNLOCK;		// no timeout			no dummy byte

	I suggest using W25QXX_CMD_FAST_READ inside separate function for fastest access.

	*/

	uint8_t cmd_len = 4; // default val: 3-Byte addr, no dumy
	if (((cmd & 0x0F) == W25QXX_CMD_READ_DATA_FAST) || (cmd == W25QXX_CMD_READ_SECURITY_REG)) { cmd_len = 5; }; // add dummy byte

	do {
		addr = bsd_rev32(addr);
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			buf[5] = addr & 0xFF;
			if ((dev->type >= W25Q256) && (dev->ext_addr_msb != buf[5])) {
				ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE);
				if (ret) { printf_dbg_(fmt_func_fail_ret, __func__, "w25qxx_cmd", ret); break; }; // buf[0] = W25QXX_CMD_WRITE_ENABLE; res = w25qxx_write_read_buf(dev, buf, 1, NULL, 0);
				buf[0] = W25QXX_CMD_WRITE_EXT_ADDR;
				buf[1] = buf[5]; // addr & 0xFF;
				ret = w25qxx_write_read_buf(dev, buf, 2, NULL, 0);
				if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_write_read_buf", ret, "W25QXX_CMD_WRITE_EXT_ADDR"); break; }; // res = w25qxx_write_read_buf(dev, buf, 2, NULL, 0);
				dev->ext_addr_msb = buf[5];
			};
			*(uint32_t *)buf = addr; // hw-accelerated + drop 1st byte // *(uint32_t *)&buf[1] = (bsd_rev32(addr) << 8); // buf[1] = (addr >> 16) & 0xFF; buf[2] = (addr >> 8) & 0xFF; buf[3] = (addr >> 0) & 0xFF; buf[4] = 0x00;
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			++cmd_len; // 4-Byte addr
			*(uint32_t *)&buf[1] = addr; // hw-accelerated //buf[1] = (addr >> 24) & 0xFF; buf[2] = (addr >> 16) & 0xFF; buf[3] = (addr >> 8) & 0xFF; buf[4] = (addr >> 0) & 0xFF; buf[5] = 0x00;
		} else { printf_dbg_("%s(): invalid address mode\r\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };

		buf[0] = cmd;
		//ret = W25QXX_RET_SUCCESS;
		if (data_io) { ret = w25qxx_write_read_buf(dev, buf, cmd_len, data_io, data_len); }
		else { ret = w25qxx_write_buf(dev, buf, cmd_len, data_out, data_len); };
		if (ret != W25QXX_RET_SUCCESS) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_write_(read)_buf", ret, w25qxx_ret2str(ret)); break; };


		/* unused
		int32_t timeout; // in 'us' by default
		switch (cmd) {
			case (W25QXX_CMD_BLOCK_WRITE_256):
			case (W25QXX_CMD_BLOCK_WRITE_256_QUAD_IO):
			case (W25QXX_CMD_WRITE_SECURITY_REG):	{ timeout = W25QXX_TIMEOUT_US_WRITE_BLOCK_256; break; };
			case (W25QXX_CMD_ERASE_SECURITY_REG):	{ timeout = W25QXX_TIMEOUT_US_ERASE_SECURITY_REG; break; };
			case (W25QXX_CMD_BLOCK_ERASE_4K):		{ timeout = W25QXX_TIMEOUT_US_ERASE_BLOCK_4K; break; };
			case (W25QXX_CMD_BLOCK_ERASE_32K):		{ timeout = W25QXX_TIMEOUT_US_ERASE_BLOCK_32K; break; };
			case (W25QXX_CMD_BLOCK_ERASE_64K):		{ timeout = W25QXX_TIMEOUT_US_ERASE_BLOCK_64K; break; };

			// lines below are not used in this function, but let's keep them for timeouts summary
			//case (W25QXX_CMD_WRITE_STATUS_REG1):	{ timeout = W25QXX_TIMEOUT_US_WRITE_STATUS_REG1; break; };
			//case (W25QXX_CMD_CHIP_ERASE):			{ timeout = W25QXX_TIMEOUT_US_ERASE_BLOCK_ALL; break; }; // yeah, up to 400sec! NOTE: 400M require 35bits
			default: { return ret; };

		};//switch(cmd)

		ret = w25qxx_wait_erase_write_finished(dev, timeout);
		*/

	} while (0);

	return ret;
};
__attribute__((nonnull)) static W25QXX_RET_t w25qxx_cmd_reset(const w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;
	ret = w25qxx_cmd(dev, W25QXX_CMD_ENABLE_RESET);
	if (ret) { printf_dbg_("%s(): reset device failed\r\n", __func__); };

	ret = w25qxx_cmd(dev, W25QXX_CMD_RESET_DEVICE);
	if (ret) { printf_dbg_("%s(): reset device failed\r\n", __func__); };

	delay_ms(RESET_RECOVER_MS);

	return ret;
};

/**
 * @brief     write data without verifying that chip memory in the specified block at the specified address is empty
 * @param[in] *dev points to a w25qxx dev structure
 * @param[in] addr is the write address
 * @param[in] *data points to a data buffer
 * @param[in] len is the data length
 * @return    status code
 */
__attribute__((nonnull)) static uint8_t w25qxx_data_write_no_check(w25qxx_dev_t *const dev, uint32_t addr, const uint8_t *data, uint32_t len) {
	W25QXX_RET_t ret;

	uint16_t block_remain = W25QXX_WRITE_BLOCK_SIZE - addr % W25QXX_WRITE_BLOCK_SIZE;
	//if (len <= page_remain) { page_remain = len; };
	block_remain = MIN(len, block_remain);

//	printf_dbg_("%s(%p,*data,%uB): block_remain=%u\r\n", __func__, addr, len, block_remain);

	while (block_remain > 0) {

		ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE);
		if (ret) { printf_dbg_(fmt_func_fail_ret, __func__, "w25qxx_cmd", ret); return ret; };

		//printf_dbg_("%s(): writing [%4u/%u] @ %p...", __func__, block_remain, len, addr);
		ret = w25qxx_cmd_addr(dev, W25QXX_CMD_BLOCK_WRITE_256, addr, NULL, data, block_remain);
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_cmd_addr", ret, "page write failed"); return ret; };

		ret = w25qxx_wait_erase_write_finished(dev, W25QXX_TIMEOUT_US_WRITE_BLOCK_256);
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_wait_erase_write_finished", ret, w25qxx_ret2str(ret)); return ret; };
		//printf_dbg_("done.\r\n");

		data += block_remain;
		addr += block_remain;
		len -= block_remain;
		block_remain = MIN(len, W25QXX_WRITE_BLOCK_SIZE);
	};

	return W25QXX_RET_SUCCESS;
};

const char *w25qxx_ret2str(W25QXX_RET_t code) { if (code >= (sizeof(w25qxx_ret_text)/sizeof(w25qxx_ret_text[0]))) { return "UNKNOWN RET CODE"; }; return w25qxx_ret_text[code]; };

W25QXX_RET_t w25qxx_set_dual_quad_spi(w25qxx_dev_t *const dev, uint8_t enable) {
	CHECK_DEV();
	dev->dual_quad_spi_enabled = enable;
	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_dual_quad_spi(const w25qxx_dev_t *const dev, uint8_t *enable) {
	CHECK_DEV();
	*enable = dev->dual_quad_spi_enabled;
	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_set_type(w25qxx_dev_t *const dev, enum W25Qxx_e type) {
	CHECK_DEV();
	dev->type = type;
	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_type(const w25qxx_dev_t *const dev, enum W25Qxx_e *type) {
	CHECK_DEV();
	*type = dev->type;
	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_set_interface(w25qxx_dev_t *const dev, enum W25QXX_INTF_e interface) {
	CHECK_DEV();
	dev->intf = (uint8_t)interface;
	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_interface(const w25qxx_dev_t *const dev, enum W25QXX_INTF_e *interface) {
	CHECK_DEV();
	*interface = dev->intf;
	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_set_address_mode(w25qxx_dev_t *const dev, enum W25QXX_ADDR_MODE_e mode) {
	uint8_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->type < W25Q256) { printf_dbg_("%s(): current type can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	if (mode == W25QXX_ADDR_MODE_3_BYTE) { ret = W25QXX_CMD_3BYTE_ADDR_MODE; } else { ret = W25QXX_CMD_4BYTE_ADDR_MODE; };

	ret = w25qxx_cmd(dev, ret);
	if (ret) { printf_dbg_("%s(): set address mode failed\r\n", __func__); return ret; };

	dev->addr_mode = mode;

	return ret;
};
W25QXX_RET_t w25qxx_get_address_mode(const w25qxx_dev_t *const dev, enum W25QXX_ADDR_MODE_e *mode) {
	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	*mode = dev->addr_mode;
	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_writing_set_enabled(const w25qxx_dev_t *const dev, enum W25QXX_LOCK_STATE_e new_state) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, (new_state == W25QXX_UNLOCKED)?(W25QXX_CMD_WRITE_ENABLE):(W25QXX_CMD_WRITE_DISABLE));
	if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_writing_set_enabled_for_volatile_sr(const w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE_VOLATILE_SR);
	if (ret) { printf_dbg_("%s(): sr write enable failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_get_status_reg(const w25qxx_dev_t *const dev, uint8_t reg_id, uint8_t *status) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	//if (!status) { return W25QXX_RET_ERR_NULL_PTR; };

	reg_id = regid_to_cmd(reg_id);
	if (reg_id == 0x00) { return W25QXX_RET_ERR_INVALID_DATA; };

	ret = w25qxx_write_read_buf(dev, &reg_id, 1, status, 1);
	if (ret) { printf_dbg_("%s(): get status1 failed\r\n", __func__); };

	return ret;
};
W25QXX_RET_t w25qxx_set_status_reg(const w25qxx_dev_t *const dev, uint8_t reg_id, uint8_t status) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE_VOLATILE_SR);
	if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };

	reg_id = regid_to_cmd(STATUS_REG_WRITE_FLAG | reg_id);
	if (reg_id == 0x00) { return W25QXX_RET_ERR_INVALID_DATA; };

	uint8_t buf[2] = { reg_id, status }; // using variable as buffer
	ret = w25qxx_write_read_buf(dev, buf, 2, NULL, 0);
	if (ret) { printf_dbg_("%s(): set status (%X) failed\r\n", __func__, reg_id); return ret; };

	return w25qxx_wait_erase_write_finished(dev, 1000000);
};

W25QXX_RET_t w25qxx_power_down(const w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_POWER_DOWN);
	if (ret) { printf_dbg_("%s(): power down failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_release_power_down(const w25qxx_dev_t *const dev) {

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	W25QXX_RET_t ret;

	//ub_t buf = { .u32 = W25QXX_CMD_RELEASE_POWER_DOWN };//{ .u8 = { W25QXX_CMD_RELEASE_POWER_DOWN, 0xFF, 0xFF, 0xFF } }; // dummy bytes vals are not specified on datasheets for this command, so lets keep them as '0' for simplicity
	//ret = w25qxx_write_read_buf(dev, buf.U8, 4, buf.U8, 1); // NOTE it's not necessary to shift dummy bytes and then read 'device_id' if it's not used
	ret = w25qxx_cmd(dev, W25QXX_CMD_RELEASE_POWER_DOWN);
	if (ret) { printf_dbg_("%s(): release power down failed\r\n", __func__); return ret; };
	delay_us(3); // wait for boot-up

	/* old code, with some strange 'dummy_byte' manipulation...
	// NOTE: datasheets don't specify any dummy bytes for this operation...

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled) {
			ret = dev->qspi_write_read(W25QXX_CMD_RELEASE_POWER_DOWN, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 3 * 8, NULL, 0x00, buf, 1, 1);
		} else {
			buf[0] = W25QXX_CMD_RELEASE_POWER_DOWN;
			buf[1] = 0xFF;
			buf[2] = 0xFF;
			buf[3] = 0xFF;
			ret = dev->spi_write_read(buf, 4, buf, 1);
		};
	} else {
		ret = dev->qspi_write_read(W25QXX_CMD_RELEASE_POWER_DOWN, 4, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 6, NULL, 0x00, buf, 1, 4);
	};
	*/

	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_set_burst_with_wrap(const w25qxx_dev_t *const dev, enum W25QXX_BURST_WRAP_e wrap) {

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { return W25QXX_RET_ERR_FORBIDDEN; };

	W25QXX_RET_t ret;
	uint8_t buf[5] = { W25QXX_CMD_SET_BURST_WITH_WRAP, 0x00, 0x00, 0x00, wrap };
	ret = w25qxx_write_read_buf(dev, buf, 5, NULL, 0);
	if (ret) { printf_dbg_("%s(): set burst with wrap failed\r\n", __func__); };

	/* old code, strange dummy_byte usage...
	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled) {
			buf[0] = wrap;
			ret = dev->qspi_write_read( W25QXX_CMD_SET_BURST_WITH_WRAP, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 3 * 8, (uint8_t *)buf, 1, NULL, 0x00, 1);
			if (ret) { printf_dbg_("%s(): set burst with wrap failed\r\n", __func__); return W25QXX_RET_ERR_IO_FAIL; };
		} else {
			buf[0] = W25QXX_CMD_SET_BURST_WITH_WRAP; buf[1] = 0x00; buf[2] = 0x00; buf[3] = 0x00; buf[4] = wrap;
			ret = w25qxx_write_read_buf(dev, buf, 5, NULL, 0);
			if (ret) { printf_dbg_("%s(): set burst with wrap failed\r\n", __func__); return W25QXX_RET_ERR_IO_FAIL; };
		};
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
	*/

	return ret;
};
W25QXX_RET_t w25qxx_set_read_parameters(w25qxx_dev_t *const dev, enum W25QXX_QSPI_READ_DUMMY_e dummy, enum W25QXX_QSPI_READ_WRAP_LENGTH_e length) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf == W25QXX_INTF_SPI) { printf_dbg_("%s(): spi interface can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };


	dev->param = (uint8_t)((dummy << 4) | (length << 0));
	buf[0] = dev->param;
	ret = dev->qspi_write_read_ext(W25QXX_CMD_SET_READ_PARAMETERS, 4, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 1, NULL, 0x00, 4);
	if (ret) { printf_dbg_("%s(): set read parameters failed\r\n", __func__); return ret; };
	if      (((buf[0] >> 4) & 0x03) == 0x00) { dev->dummy = 2; }
	else if (((buf[0] >> 4) & 0x03) == 0x01) { dev->dummy = 4; }
	else if (((buf[0] >> 4) & 0x03) == 0x02) { dev->dummy = 6; }
	else { dev->dummy = 8; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_enter_qspi_mode(w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf == W25QXX_INTF_QPI) { printf_dbg_("%s(): qspi interface can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	ret = dev->qspi_write_read_ext(W25QXX_CMD_QPI_MODE_BEG, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, NULL, 0x00, NULL, 0x00, 0x00);
	if (ret) { printf_dbg_("%s(): enter qspi mode failed\r\n", __func__); return ret; };
	delay_ms(10);
	buf[0] = dev->param;
	ret = dev->qspi_write_read_ext(W25QXX_CMD_SET_READ_PARAMETERS, 4, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 1, NULL, 0x00, 4);
	if (ret) { printf_dbg_("%s(): set read parameters failed\r\n", __func__); return ret; };
	if      (((buf[0] >> 4) & 0x03) == 0x00) { dev->dummy = 2; }
	else if (((buf[0] >> 4) & 0x03) == 0x01) { dev->dummy = 4; }
	else if (((buf[0] >> 4) & 0x03) == 0x02) { dev->dummy = 6; }
	else { dev->dummy = 8; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_exit_qspi_mode(const w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) { printf_dbg_("%s(): spi interface can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	ret = dev->qspi_write_read_ext(W25QXX_CMD_QPI_MODE_END, 4, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, NULL, 0x00, NULL, 0x00, 0x00);
	if (ret) { printf_dbg_("%s(): exit qspi mode failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_reset_device(const w25qxx_dev_t *const dev) {
	// WARNING: w25qxx_reset_device() available not for all devices

	CHECK_DEV();
	//if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; }; // doesn't make sense

	return w25qxx_cmd_reset(dev);
};
W25QXX_RET_t w25qxx_get_unique_id(const w25qxx_dev_t *const dev, uint8_t id[W25QXX_UID_LEN]) {
	W25QXX_RET_t ret;
	uint8_t buf[6] = { W25QXX_CMD_READ_UNIQUE_ID };

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type < W25Q256)) { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };

	ret = w25qxx_write_read_buf(dev, buf, (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE)?(5):(6), id, W25QXX_UID_LEN);
	if (ret) { printf_dbg_("%s(): read unique id failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_sfdp(const w25qxx_dev_t *const dev, uint8_t sfdp[W25QXX_SFDP_BLOCK_SIZE]) {
	W25QXX_RET_t ret;
	uint8_t buf[5] = { W25QXX_CMD_READ_SFDP_REG };

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	ret = w25qxx_write_read_buf(dev, buf, 5, sfdp, W25QXX_SFDP_BLOCK_SIZE);
	if (ret) { printf_dbg_("%s(): get sfdp failed\r\n", __func__); };

	return ret;
};

W25QXX_RET_t w25qxx_read_security_register(const w25qxx_dev_t *const dev, enum W25QXX_SECURITY_REG_e security_reg_addr, uint8_t data[W25QXX_SFDP_BLOCK_SIZE]) {
	W25QXX_RET_t ret;
	const uint32_t addr = bsd_rev32(security_reg_addr);

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	uint8_t buf[6];
	if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
		*(uint32_t *)&buf[0] = addr;
		buf[0] = W25QXX_CMD_READ_SECURITY_REG;
		ret = w25qxx_write_read_buf(dev, buf, 5, data, W25QXX_SFDP_BLOCK_SIZE);
	} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
		*(uint32_t *)&buf[1] = addr;
		buf[0] = W25QXX_CMD_READ_SECURITY_REG;
		ret = w25qxx_write_read_buf(dev, buf, 6, data, W25QXX_SFDP_BLOCK_SIZE);
	} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	if (ret) { printf_dbg_("%s(): program security register failed\r\n", __func__); return ret; };

	return ret;
};
W25QXX_RET_t w25qxx_write_security_register(const w25qxx_dev_t *const dev, enum W25QXX_SECURITY_REG_e reg, const uint8_t data[W25QXX_SECURITY_BLOCK_SIZE]) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE);
	if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };

	uint8_t buf[5] = { W25QXX_CMD_WRITE_SECURITY_REG };
	if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
		buf[2] = reg;
		ret = w25qxx_write_buf(dev, buf, 4, data, W25QXX_SECURITY_BLOCK_SIZE);
	} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
		buf[3] = reg;
		ret = w25qxx_write_buf(dev, buf, 5, data, W25QXX_SECURITY_BLOCK_SIZE);
	} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	if (ret) { printf_dbg_("%s(): program security register failed\r\n", __func__); return ret; };

	uint16_t timeout_us;
	for (timeout_us = 300; timeout_us != 0; --timeout_us, delay_us(10)) {
		buf[0] = W25QXX_CMD_READ_STATUS_REG1;
		ret = w25qxx_write_read_buf(dev, buf, 1, buf, 1);
		if (ret) { printf_dbg_("%s(): get status1 failed\r\n", __func__); return ret; };
		if ((buf[0] & W25QXX_STATUS1_ERASE_WRITE_BUSY) == 0) { break; };
	};

	if (timeout_us == 0) { printf_dbg_("%s(): program security register timeout\r\n", __func__); return W25QXX_RET_ERR_TIMEOUT; };

	return ret;
};
W25QXX_RET_t w25qxx_erase_security_register(const w25qxx_dev_t *const dev, enum W25QXX_SECURITY_REG_e reg) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_RESET_DEVICE);
	if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };

	uint8_t buf[5] = { W25QXX_CMD_ERASE_SECURITY_REG };
	if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
		buf[2] = reg;
		ret = w25qxx_write_read_buf(dev, buf, 4, NULL, 0);
	} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
		buf[3] = reg;
		ret = w25qxx_write_read_buf(dev, buf, 5, NULL, 0);
	} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	if (ret) { printf_dbg_("%s(): erase security register failed\r\n", __func__); return ret; };

	uint32_t timeout_us;
	for (timeout_us = 10 * 1000; timeout_us > 0; --timeout_us, delay_us(10)) {
		buf[0] = W25QXX_CMD_READ_STATUS_REG1;
		ret = w25qxx_write_read_buf(dev, buf, 1, buf, 1);
		if (ret) { printf_dbg_("%s(): get status1 failed\r\n", __func__); return ret; };
		if ((buf[0] & W25QXX_STATUS1_ERASE_WRITE_BUSY) == 0) { break; };
	};

	if (timeout_us == 0) { printf_dbg_("%s(): erase security register timeout\r\n", __func__); return W25QXX_RET_ERR_TIMEOUT; };

	return ret;
};


W25QXX_RET_t w25qxx_mem_check_empty(w25qxx_dev_t *const dev, uint32_t addr_beg, const uint32_t addr_end, uint32_t *non_empty_addr) {
	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	#ifdef W25QXX_DYNAMIC_OVERWRITE_BUFFER
	void *const tmp_buf = w25qxx_malloc(W25QXX_SECTOR_SIZE);
	if (!tmp_buf) { printf_dbg_(fmt_func_fail_ret, __func__, "w25qxx_malloc", tmp_buf); return W25QXX_RET_ERR_NO_MEM; };
	#else
	//uint8_t tmp_buf[W25QXX_SECTOR_SIZE]; // NOTE: task switching will be requied while executing this function, so this buffer will be either pushed into task's context stack and back ot placed there initially... which is painfully expensive. So, instead of dynamic buffer, consider using either dedicated external device buffer (4kb RAM frozen!), or calling 'malloc()' every time. Ref.: https://www.freertos.org/FreeRTOS_Support_Forum_Archive/June_2015/freertos_allocating_a_buffer_vs_declaring_it_60193d15j.html
	uint8_t *const tmp_buf = dev->ow_buf; // let's hope it's large enough
	#endif

	//memset(tmp_buf, W25QXX_MEM_DEFAULT_VAL, W25QXX_SECTOR_SIZE);
	if (non_empty_addr) { *non_empty_addr = W25QXX_ADDR_INVALID_VAL; };

	/* memory alignment required after 'malloc()' for this optimization to work properly
	#if (__UINTPTR_MAX__ == 0xFFFFFFFF) // CPU Supports native 32-bit arithmetic
	uint32_t *ptr;
	const uint32_t exp_val = (W25QXX_MEM_DEFAULT_VAL << 24) | (W25QXX_MEM_DEFAULT_VAL << 16) | (W25QXX_MEM_DEFAULT_VAL << 8) | (W25QXX_MEM_DEFAULT_VAL);
	#elif (__UINTPTR_MAX__ > 0xFFFFFFFF) // CPU Supports native 64-bit arithmetic
	uint64_t *ptr;
	const uint64_t exp_val =	(W25QXX_MEM_DEFAULT_VAL << 56) | (W25QXX_MEM_DEFAULT_VAL << 48) | (W25QXX_MEM_DEFAULT_VAL << 40) | (W25QXX_MEM_DEFAULT_VAL << 32) |
								(W25QXX_MEM_DEFAULT_VAL << 24) | (W25QXX_MEM_DEFAULT_VAL << 16) | (W25QXX_MEM_DEFAULT_VAL << 8) | (W25QXX_MEM_DEFAULT_VAL);
	#else  // Fallback to bytes
	uint8_t *ptr;
	const uint8_t exp_val = W25QXX_MEM_DEFAULT_VAL;
	#endif
	*/
	uint8_t *ptr;
	const uint8_t exp_val = W25QXX_MEM_DEFAULT_VAL;
	//const typeof(ptr) ptr_end = (tmp_buf + W25QXX_SECTOR_SIZE);
	const uint8_t *const ptr_end = (tmp_buf + W25QXX_SECTOR_SIZE);

	W25QXX_RET_t ret = W25QXX_RET_SUCCESS;
	for ( ; addr_beg < addr_end; addr_beg += W25QXX_SECTOR_SIZE) {
		ret = w25qxx_cmd_addr(dev, W25QXX_CMD_READ_DATA_FAST, addr_beg, tmp_buf, NULL, W25QXX_SECTOR_SIZE);
		if (ret != W25QXX_RET_SUCCESS) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_cmd_addr", ret, w25qxx_ret2str(ret)); break; };

		for (ptr = tmp_buf; ptr < ptr_end; ++ptr) { if (*ptr != exp_val) { ret = W25QXX_RET_ERR_INVALID_STATE; break; }; };
		//if (ptr < ptr_end) { if (non_empty_addr) { *non_empty_addr = addr_beg + ((void *)ptr - (void *)tmp_buf); }; break; };  // better return block addr to erase it momentarily (if needed) wihtout addr caculations
		if (ret) { break; };
	};//for(addr)

	#ifdef W25QXX_DYNAMIC_OVERWRITE_BUFFER
	w25qxx_free(tmp_buf);
	#endif

	if (non_empty_addr) { *non_empty_addr = addr_beg; };
	//if (ret != W25QXX_RET_SUCCESS) { return ret; };
	return ret;//(addr_beg >= addr_end)?(W25QXX_RET_SUCCESS):(W25QXX_RET_ERR_INVALID_STATE);
};
W25QXX_RET_t w25qxx_chip_check_empty(w25qxx_dev_t *const dev, uint32_t *non_empty_addr) {

	const uint32_t addr_end = w25qxx_get_storage_capacity_bytes(dev);

	return w25qxx_mem_check_empty(dev, 0x00000000, addr_end, non_empty_addr);
};

W25QXX_RET_t w25qxx_read(w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	uint8_t ret = w25qxx_cmd_addr(dev, W25QXX_CMD_READ_DATA, addr, data, NULL, len);
	if (ret) { printf_dbg_("%s(): read failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_spi_only(w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { printf_dbg_("%s(): only spi interface can use this funciton\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	uint8_t ret = w25qxx_cmd_addr(dev, W25QXX_CMD_READ_DATA, addr, data, NULL, len);
	if (ret) { printf_dbg_("%s(): fast read failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_fast(w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	uint8_t ret = w25qxx_cmd_addr(dev, W25QXX_CMD_READ_DATA_FAST, addr, data, NULL, len);
	if (ret) { printf_dbg_("%s(): read failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_fast_dual_output(const w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_DUAL_OUT, 1, addr, 1, 3, 0x00000000, 0x00, 0x00, 8, NULL, 0x00, data, len, 2);
			if (ret) { printf_dbg_("%s(): fast read dual output failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_DUAL_OUT, 1, addr, 1, 4, 0x00000000, 0x00, 0x00, 8, NULL, 0x00, data, len, 2);
			if (ret) { printf_dbg_("%s(): fast read dual output failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_fast_quad_output(const w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_QUAD_OUT, 1, addr, 1, 3, 0x00000000, 0x00, 0x00, 8, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): fast read quad output failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_QUAD_OUT, 1, addr, 1, 4, 0x00000000, 0x00, 0x00, 8, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): fast read quad output failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_fast_dual_io(const w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_DUAL_IO, 1, addr, 2, 3, 0x000000FF, 2, 1, 0, NULL, 0x00, data, len, 2);
			if (ret) { printf_dbg_("%s(): fast read dual io failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_DUAL_IO, 1, addr, 2, 4, 0x000000FF, 2, 1, 0, NULL, 0x00, data, len, 2);
			if (ret) { printf_dbg_("%s(): fast read dual io failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_fast_quad_io(const w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_QUAD_IO, 1, addr, 4, 3, 0x000000FF, 4, 1, 4, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): fast read quad io failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_QUAD_IO, 1, addr, 4, 4, 0x000000FF, 4, 1, 4, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): fast read quad io failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else {
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 4, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 4, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 4);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_QUAD_IO, 4, addr, 4, 3, 0x000000FF, 4, 1, dev->dummy, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): fast read quad io failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_FAST_QUAD_IO, 4, addr, 4, 4, 0x000000FF, 4, 1, dev->dummy, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): fast read quad io failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	};

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_word_quad_io(const w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_WORD_QUAD_IO, 1, addr, 4, 3, 0x000000FF, 4, 1, 2, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): word read quad io failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_WORD_QUAD_IO, 1, addr, 4, 4, 0x000000FF, 4, 1, 2, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): word read quad io failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_read_octal_word_quad_io(const w25qxx_dev_t *const dev, uint32_t addr, void *data, uint32_t len) {
	W25QXX_RET_t ret;
	uint8_t buf[1];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			if (dev->type >= W25Q256) {
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
				if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
				buf[0] = (addr >> 24) & 0xFF;
				ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
				if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			};
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_OCTAL_WORD_QUAD_IO, 1, addr, 4, 3, 0x000000FF, 4, 1, 0, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): octal word read quad io failed\r\n", __func__); return ret; };
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_READ_DATA_OCTAL_WORD_QUAD_IO, 1, addr, 4, 4, 0x000000FF, 4, 1, 0, NULL, 0x00, data, len, 4);
			if (ret) { printf_dbg_("%s(): octal word read quad io failed\r\n", __func__); return ret; };
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_write(w25qxx_dev_t *const dev, uint32_t addr, const void *data, uint16_t len) {
	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	return w25qxx_data_write_no_check(dev, addr, data, len);
};
/*W25QXX_RET_t w25qxx_write_verify(w25qxx_dev_t *const dev, uint32_t addr, const void *data, uint16_t len) {
	W25QXX_RET_t ret;
	ret = w25qxx_data_write_no_check(dev, addr, data, len);
	if (ret != W25QXX_RET_SUCCESS) { printf_dbg_(fmt_func_fail_ret, __func__, "a_w25qxx_write_no_check", ret); return ret; };
	// TODO: implement verification procedure
	return W25QXX_RET_SUCCESS;
};*/
W25QXX_RET_t w25qxx_write_block(w25qxx_dev_t *const dev, uint32_t addr, const void *data, uint16_t len) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if ((addr % W25QXX_WRITE_BLOCK_SIZE) != 0) { printf_dbg_("%s(): addr is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_DATA; }; // NOTE: this is actually not a problem, we should only avoid crossing 256-byte alignment border
	if (len > W25QXX_WRITE_BLOCK_SIZE) { printf_dbg_("%s(): length is over %u\r\n", __func__, W25QXX_WRITE_BLOCK_SIZE); return W25QXX_RET_ERR_OVERFLOW; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE);
	if (ret) { printf_dbg_(fmt_func_fail_ret, __func__, "w25qxx_cmd", ret); return ret; };

	ret = w25qxx_cmd_addr(dev, W25QXX_CMD_BLOCK_WRITE_256, addr, NULL, data, len);
	if (ret) { printf_dbg_("%s(): fast read failed\r\n", __func__); return ret; };

	ret = w25qxx_wait_erase_write_finished(dev, W25QXX_TIMEOUT_US_WRITE_BLOCK_256);
	if (ret) { printf_dbg_("%s(): fast read failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_write_block_quad_input(const w25qxx_dev_t *const dev, uint32_t addr, const void *data, uint16_t len) {
	W25QXX_RET_t ret;


	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (dev->intf != W25QXX_INTF_SPI) { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; }
	if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	if ((addr % W25QXX_WRITE_BLOCK_SIZE) != 0) { printf_dbg_("%s(): addr is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_DATA; };
	if (len > W25QXX_WRITE_BLOCK_SIZE) { printf_dbg_("%s(): length is over %u\r\n", __func__, W25QXX_WRITE_BLOCK_SIZE); return W25QXX_RET_ERR_OVERFLOW; };

	ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
	if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
	if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
		if (dev->type >= W25Q256) {
			uint8_t buf[2];
			buf[0] = (addr >> 24) & 0xFF;
			ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_EXT_ADDR, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0, (uint8_t *)buf, 0x01, NULL, 0x00, 1);
			if (ret) { printf_dbg_("%s(): write extended addr register failed\r\n", __func__); return ret; };
			ret = dev->qspi_write_read_ext(W25QXX_CMD_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
			if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };
		};
		ret = dev->qspi_write_read_ext(W25QXX_CMD_BLOCK_WRITE_256_QUAD_IO, 1, addr, 1, 3, 0x00000000, 0x00, 0x00, 0, NULL, 0x00, data, len, 4);
		if (ret) { printf_dbg_("%s(): quad page program failed\r\n", __func__); return ret; };
	} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
		ret = dev->qspi_write_read_ext(W25QXX_CMD_BLOCK_WRITE_256_QUAD_IO, 1, addr, 1, 4, 0x00000000, 0x00, 0x00, 0, NULL, 0x00, data, len, 4);
		if (ret) { printf_dbg_("%s(): quad page program failed\r\n", __func__); return ret; };
	} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };

	return w25qxx_wait_erase_write_finished(dev, 300);
};
W25QXX_RET_t w25qxx_overwrite_verify(w25qxx_dev_t *const dev, uint32_t addr, const void *data, uint32_t len) {
	W25QXX_RET_t ret = W25QXX_RET_SUCCESS;
	uint16_t i;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	uint16_t sect = addr / W25QXX_SECTOR_SIZE;
	uint16_t sect_offset = addr % W25QXX_SECTOR_SIZE;
	uint16_t sect_remain = W25QXX_SECTOR_SIZE - sect_offset;
	//if (len <= sect_remain) { sect_remain = len; };
	sect_remain = MIN(sect_remain, len);
	printf_dbg_("%s(%p,*data,%uB): sect_remain=%u\r\n", __func__, addr, len, sect_remain);
	if (sect_remain == 0) { return W25QXX_RET_SUCCESS; };

	#ifdef W25QXX_DYNAMIC_OVERWRITE_BUFFER
	uint8_t *const tmp_buf = w25qxx_malloc(W25QXX_SECTOR_SIZE);
	if (!tmp_buf) { printf_dbg_(fmt_func_fail_ret, __func__, "w25qxx_malloc", tmp_buf); return W25QXX_RET_ERR_NO_MEM; };
	#else
	//uint8_t tmp_buf[W25QXX_SECTOR_SIZE]; // NOTE: task switching will be requied while executing this function, so this buffer will be either pushed into task's context stack and back ot placed there initially... which is painfully expensive. So, instead of dynamic buffer, consider using either dedicated external device buffer (4kb RAM frozen!), or calling 'malloc()' every time. Ref.: https://www.freertos.org/FreeRTOS_Support_Forum_Archive/June_2015/freertos_allocating_a_buffer_vs_declaring_it_60193d15j.html
	uint8_t *const tmp_buf = dev->ow_buf; // let's hope it's large enough
	#endif

	 for( ; sect_remain > 0; ++sect, sect_offset = 0, data += sect_remain, addr += sect_remain, len -= sect_remain, sect_remain = MIN(len, W25QXX_SECTOR_SIZE)) {
		//printf_dbg_("%s(): addr=%p; data[%4u/%u]=%16.1A\r\n", __func__, addr, sect_remain, len, data);

		printf_dbg_("%s(): Reading %4u bytes @ %p...\r\n", __func__, W25QXX_SECTOR_SIZE, sect * W25QXX_SECTOR_SIZE);
		ret = w25qxx_cmd_addr(dev, W25QXX_CMD_READ_DATA_FAST, sect * W25QXX_SECTOR_SIZE, tmp_buf, NULL, W25QXX_SECTOR_SIZE); // read 4k block // TODO: (?) Read only block we're going to overwrite. If it's not empty, re-read the whole block.
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "READ_DATA_FAST1", ret,w25qxx_ret2str(ret)); break; };

		for (i = 0; i < sect_remain; ++i) { if (tmp_buf[sect_offset + i] != W25QXX_MEM_DEFAULT_VAL) { break; }; };	// verify that block which we're going to overwrite is empty

		if (i < sect_remain) {																		// if it's not empty, conduct over-writing procedure
			if (memcmp(&tmp_buf[sect_offset], data, sect_remain) == 0) { printf_dbg_("writing skipped\r\n"); continue; }; // The chunk of data that we're trying to overwrite, exaclty matches content of the chip's memory, so just skip dealing with this chunk of data.
			printf_dbg_("%s(): Data[%u] mismatch => cannot skip => need erase @ %p\r\n\tdata_src: %16.1A\r\n\tdata_dst: %16.1A\r\n", __func__, sect_remain, sect * W25QXX_SECTOR_SIZE, data, &tmp_buf[sect_offset]);

			ret = w25qxx_block_erase(dev, sect * W25QXX_SECTOR_SIZE, W25QXX_BLOCK_SIZE_4K);
			if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_block_erase", ret,w25qxx_ret2str(ret)); break; };

			memcpy(&tmp_buf[i + sect_offset], data, sect_remain);									// change actual block data before writing it back
			ret = w25qxx_data_write_no_check(dev, sect * W25QXX_SECTOR_SIZE, tmp_buf, W25QXX_SECTOR_SIZE);
		} else {																					// if it's empty, just keep writing
			ret = w25qxx_data_write_no_check(dev, addr, data, sect_remain);
		};
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_data_write_no_check", ret,w25qxx_ret2str(ret)); break; };

		/* Verify data
		// UPD: for some reason, reading right after writing fails without significant delay
		// UPD2: nope, it was just malloc failing
		*/
//		printf_dbg_("%s(): Re-reading %4u bytes @ %p... ", __func__, sect_remain, addr);
		ret = w25qxx_cmd_addr(dev, W25QXX_CMD_READ_DATA_FAST, addr, tmp_buf, NULL, sect_remain);
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "READ_DATA_FAST2", ret,w25qxx_ret2str(ret)); break; };
		//ret = memcmp(tmp_buf, data, sect_remain); // less verbose compare
		for (i = 0; i < sect_remain; ++i) {
			if (tmp_buf[i] == ((uint8_t *)data)[i]) { continue; };
			printf_dbg_("data mismatch @ %p.\r\ndata_src: %16.1A\r\ndata_dst: %16.1A\r\n", addr + i, data + i, tmp_buf + i);
			ret = W25QXX_RET_ERR_DATA_CORRUPTED;
			break;
		};
//		printf_dbg_("Done.\r\n");
		if (ret) { printf_dbg_(fmt_func_fail_ret, __func__, "memcmp", i); break; }; // same as above, but prints failure position
	};

	#ifdef W25QXX_DYNAMIC_OVERWRITE_BUFFER
	w25qxx_free(tmp_buf);
	#endif

	return ret;
};

W25QXX_RET_t w25qxx_chip_erase(const w25qxx_dev_t *const dev) {
	uint8_t ret;
	uint8_t status;
	uint32_t timeout_ms;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE);
	if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); return ret; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_CHIP_ERASE);
	if (ret) { printf_dbg_("%s(): chip erase failed\r\n", __func__); return ret; };

	ret = w25qxx_wait_erase_write_finished(dev, W25QXX_TIMEOUT_US_ERASE_ALL);

	// yeah, 15..30sec for W25Q64 and 80..400sec for W25Q256
	for (timeout_ms = 400 * 1000; timeout_ms != 0; timeout_ms =- DELAY_STEP, delay_ms(DELAY_STEP)) {
		ret = W25QXX_CMD_READ_STATUS_REG1;
		ret = w25qxx_write_read_buf(dev, &ret, 1, (uint8_t *)&status, 1);
		if (ret) { printf_dbg_("%s(): get status1 failed\r\n", __func__); return ret; };
		if ((status & W25QXX_STATUS1_ERASE_WRITE_BUSY) == 0) { break; };
	};

	if (timeout_ms == 0) { printf_dbg_("%s(): erase timeout\r\n", __func__); return W25QXX_RET_ERR_TIMEOUT; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_block_erase(w25qxx_dev_t *const dev, const uint32_t addr, enum W25QXX_BLOCK_SIZE_e block_size_kb) {
	W25QXX_RET_t ret;
	uint8_t cmd;
	float timeout_us;
	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

//	printf_dbg_("%s(%p,%uKB)\r\n", __func__, addr, block_size_kb);

	// NOTE: selected max timeouts. For chips with lower memory density, timeouts are shorter.
	if      (block_size_kb == W25QXX_BLOCK_SIZE_4K)  { cmd = W25QXX_CMD_BLOCK_ERASE_4K;  timeout_us = W25QXX_TIMEOUT_US_ERASE_BLOCK_4K; }
	else if (block_size_kb == W25QXX_BLOCK_SIZE_32K) { cmd = W25QXX_CMD_BLOCK_ERASE_32K; timeout_us = W25QXX_TIMEOUT_US_ERASE_BLOCK_32K; }
	else if (block_size_kb == W25QXX_BLOCK_SIZE_64K) { cmd = W25QXX_CMD_BLOCK_ERASE_64K; timeout_us = W25QXX_TIMEOUT_US_ERASE_BLOCK_64K; }
	else { printf_dbg_("%s(): invalid block size\r\n", __func__);  return W25QXX_RET_ERR_INVALID_DATA; };

	if ((addr % (block_size_kb * 1024)) != 0) { printf_dbg_("%s(): addr is invalid (not aligned to %ukb)\r\n", __func__, block_size_kb); return W25QXX_RET_ERR_INVALID_DATA; };

	do {
		ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE);
		if (ret) { printf_dbg_("%s(): write enable failed\r\n", __func__); break; };

		ret = w25qxx_cmd_addr(dev, cmd, addr, NULL, NULL, 0);
		if (ret) { printf_dbg_("%s(%u): sector erase failed\r\n", __func__, block_size_kb); break; };

		ret = w25qxx_wait_erase_write_finished(dev, timeout_us);
		if (ret) { printf_dbg_("%s(): sector erase timeout\r\n", __func__); break; };
	} while (0);


	return ret;
};
W25QXX_RET_t w25qxx_erase_write_suspend(const w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_ERASE_WRITE_SUSPEND);
	if (ret) { printf_dbg_("%s(): erase program suspend failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_erase_write_resume(const w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, W25QXX_CMD_ERASE_WRITE_RESUME);
	if (ret) { printf_dbg_("%s(): erase program resume failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};

W25QXX_RET_t w25qxx_block_lock_get_state(w25qxx_dev_t *const dev, uint32_t addr, uint8_t *value) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	//if (!value) { return W25QXX_RET_ERR_NULL_PTR; };

	ret = w25qxx_cmd_addr(dev, W25QXX_CMD_BLOCK_READ_LOCK, addr, value, NULL, 1);
	if (ret) { printf_dbg_("%s(): read block lock failed\r\n", __func__); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_block_lock_set_state(w25qxx_dev_t *const dev, uint32_t addr, enum W25QXX_LOCK_STATE_e new_state) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd_addr(dev, (new_state == W25QXX_UNLOCKED)?(W25QXX_CMD_BLOCK_UNLOCK):(W25QXX_CMD_BLOCK_LOCK), addr, NULL, NULL, 0);
	if (ret) { printf_dbg_("%s(): block set lock state(%d) failed\r\n", __func__, new_state); return ret; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_block_lock_set_state_global(const w25qxx_dev_t *const dev, enum W25QXX_LOCK_STATE_e new_state) {
	W25QXX_RET_t ret;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	ret = w25qxx_cmd(dev, ((new_state == W25QXX_UNLOCKED)?(W25QXX_CMD_BLOCK_UNLOCK_GLOBAL):(W25QXX_CMD_BLOCK_LOCK_GLOBAL)));
	if (ret) { printf_dbg_("%s(): setting global block lock state failed\r\n", __func__); };

	return ret;
};

W25QXX_RET_t w25qxx_get_jedec_id(const w25qxx_dev_t *const dev, uint8_t *manufacturer, uint8_t device_id[2]) {
	W25QXX_RET_t ret;
	uint8_t buf[3];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (!manufacturer && !device_id) { return W25QXX_RET_ERR_NULL_PTR; };

	buf[0] = W25QXX_CMD_JEDEC_ID;
	ret = w25qxx_write_read_buf(dev, buf, 1, buf, 3);
	if (ret) { printf_dbg_("%s(): get jedec id failed\r\n", __func__); return ret; };

	if (manufacturer) { *manufacturer = buf[0]; };
	if (device_id) { device_id[0] = buf[1]; device_id[1] = buf[2]; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_manufacturer_device_id(const w25qxx_dev_t *const dev, uint8_t *manufacturer, uint8_t *device_id) {
	CHECK_DEV();

	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (!manufacturer && !device_id) { return W25QXX_RET_ERR_NULL_PTR; };

	W25QXX_RET_t ret;
	uint8_t buf[4] = { W25QXX_CMD_READ_MANUFACTURER };
	ret = w25qxx_write_read_buf(dev, buf, 4, buf, 2);
	if (ret) { printf_dbg_("%s(): get manufacturer device id failed\r\n", __func__); return ret; };

	if (manufacturer) { *manufacturer = buf[0]; };
	if (device_id) { *device_id = buf[1]; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_manufacturer_device_id_dual_io(const w25qxx_dev_t *const dev, uint8_t *manufacturer, uint8_t *device_id) {
	W25QXX_RET_t ret;
	uint8_t out[2];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (!manufacturer && !device_id) { return W25QXX_RET_ERR_NULL_PTR; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_DEVICE_ID_DUAL_IO, 1, 0x00000000, 2, 3, 0x000000FF, 2, 1, 0x00, NULL, 0x00, (uint8_t *)out, 2, 2);
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_DEVICE_ID_DUAL_IO, 1, 0x00000000, 2, 4, 0x000000FF, 2, 1, 0x00, NULL, 0x00, (uint8_t *)out, 2, 2);
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	if (ret) { printf_dbg_("%s(): get manufacturer device id dual io failed\r\n", __func__); return ret; };
	if (manufacturer) { *manufacturer = out[0]; };
	if (device_id) { *device_id = out[1]; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_manufacturer_device_id_quad_io(const w25qxx_dev_t *const dev, uint8_t *manufacturer, uint8_t *device_id) {
	W25QXX_RET_t ret;
	uint8_t out[2];

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };
	if (!manufacturer && !device_id) { return W25QXX_RET_ERR_NULL_PTR; };

	if (dev->intf == W25QXX_INTF_SPI) {
		if (dev->dual_quad_spi_enabled == 0) { printf_dbg_("%s(): standard spi can't use this function failed\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };
		if (dev->addr_mode == W25QXX_ADDR_MODE_3_BYTE) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_DEVICE_ID_QUAD_IO, 1, 0x00000000, 4, 3, 0x000000FF, 4, 1, 4, NULL, 0x00, (uint8_t *)out, 2, 4);
		} else if ((dev->addr_mode == W25QXX_ADDR_MODE_4_BYTE) && (dev->type >= W25Q256)) {
			ret = dev->qspi_write_read_ext(W25QXX_CMD_DEVICE_ID_QUAD_IO, 1, 0x00000000, 4, 4, 0x000000FF, 4, 1, 4, NULL, 0x00, (uint8_t *)out, 2, 4);
		} else { printf_dbg_("%s(): address mode is invalid\r\n", __func__); return W25QXX_RET_ERR_INVALID_MODE; };
	} else { printf_dbg_("%s(): qspi can't use this function\r\n", __func__); return W25QXX_RET_ERR_FORBIDDEN; };

	if (ret) { printf_dbg_("%s(): get manufacturer device id quad io failed\r\n", __func__); return ret; };
	if (manufacturer) { *manufacturer = out[0]; };
	if (device_id) { *device_id = out[1]; };

	return W25QXX_RET_SUCCESS;
};
W25QXX_RET_t w25qxx_get_info(const w25qxx_dev_t *const dev, w25qxx_info_t *const info_struct) {

	if (!info_struct) { return W25QXX_RET_ERR_NULL_PTR; };
	if (dev->type >= W25Qxx_max) { return W25QXX_RET_ERR_INVALID_STATE; };

	*info_struct = (w25qxx_info_t){
		.manufacturer			= manuf_text[MANUF_UNK],
		.chip					= chip_name[dev->type],												// "W25QXX", // NOTE: it is safe, since we have already validated 'dev->type'
		.interface				= intf_name + (1 - dev->intf),										// "QPI" / "SPI"
		.intf_mode				= (dev->dual_quad_spi_enabled)?("QUAD/DUAL"):("NORMAL"),
		.addr_mode				= { '3' + dev->addr_mode, 'B' },									// '3' => '4' if (addr_mode == true)
		.supply_volt_min		= 2.7f,
		.supply_volt_max		= 3.6f,
		.max_current_ma			= 25,
		.temperature_max		= 85,
		.temperature_min		= -40,
		.driver_version			= DRIVER_VERSION>>4,
		.vol_size_kb			= (w25qxx_get_storage_capacity_bytes(dev) / 1024),
	};

	W25QXX_RET_t ret;
	ub_t buf;
	buf.u32 = W25QXX_CMD_READ_MANUFACTURER; //buf[0] = W25QXX_CMD_READ_MANUFACTURER; buf[1] = 0x00; buf[2] = 0x00; buf[3] = 0x00;
	if ((ret = w25qxx_write_read_buf(dev, buf.U8, 4, buf.U8, 2)))   { printf_dbg_("%s(): get manufacturer device id failed (%X)\r\n", __func__, ret); return W25QXX_RET_ERR_IO_FAIL; };

	if (buf.U8[0] == MANUFACTURER_ID_WINBOND)  { info_struct->manufacturer = manuf_text[MANUF_WINBOND]; };
	if (buf.U8[0] == MANUFACTURER_ID_MACRONIX) { info_struct->manufacturer = manuf_text[MANUF_MACRONIX]; };

	return W25QXX_RET_SUCCESS;
};
uint32_t     w25qxx_get_storage_capacity_bytes(const w25qxx_dev_t *const dev) {
	#define MBit2Bytes (1024 * 1024 / 8)
	if (dev->type >= W25Qxx_max) { return 0; };
	return (1 << dev->type) * MBit2Bytes;
	/* old code, explicit way
	switch (dev->type) {
		case (W25Q10):  return   1 * MBit2Bytes;
		case (W25Q20):  return   2 * MBit2Bytes;
		case (W25Q40):  return   4 * MBit2Bytes;
		case (W25Q80):  return   8 * MBit2Bytes;
		case (W25Q16):  return  16 * MBit2Bytes;
		case (W25Q32):  return  32 * MBit2Bytes;
		case (W25Q64):  return  64 * MBit2Bytes;
		case (W25Q128): return 128 * MBit2Bytes;
		case (W25Q256): return 256 * MBit2Bytes;
		case (W25Q512): return 512 * MBit2Bytes;
		default: return 0;
	};//switch(dev->type)
	*/
};
W25QXX_RET_t w25qxx_deinit(const w25qxx_dev_t *const dev) {
	uint8_t res;

	CHECK_DEV();
	if (!dev->initialized) { return W25QXX_RET_ERR_UNINITIALIZED; };

	res = w25qxx_cmd(dev, W25QXX_CMD_POWER_DOWN);
	if (res) { printf_dbg_("%s(): power down failed\r\n", __func__); };

	return res;
};
W25QXX_RET_t w25qxx_chip_detect(w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;
	ub_t buf;

	do {
		// NOTE: Datasheet: it is recommended to issue a 'Continuous Read Mode Reset' instruction as the first instruction after a system Reset.
		buf.u16 = (W25QXX_CMD_CONTINUOUS_READ_MODE_RESET | (W25QXX_CMD_CONTINUOUS_READ_MODE_RESET << 8));
		if ((ret = w25qxx_write_read_buf(dev, buf.U8, 2, NULL, 0))) { printf_dbg_("%s(): release power down failed\r\n", __func__); return ret; };

		buf.u32 = W25QXX_CMD_RELEASE_POWER_DOWN;
		if ((ret = w25qxx_write_read_buf(dev, buf.U8, 4, buf.U8, 1))) { printf_dbg_("%s(): release power down failed\r\n", __func__); return ret; };

		if ((buf.u8 < W25Qxx_base) || (buf.u8 >= (W25Qxx_base + W25Qxx_max))) { printf_dbg_("%s(): invalid device id (%02X)\r\n", __func__, buf.u8); return W25QXX_RET_ERR_INVALID_STATE; };
		dev->type = buf.u8 & W25Qxx_mask;
		printf_dbg_("%s(): dev->dev_id: %u (chip val: %X)\r\n", __func__, dev->type, buf.u8);

		return W25QXX_RET_SUCCESS;
	} while (0);

	return W25QXX_RET_ERR_INVALID_STATE;
};

W25QXX_RET_t w25qxx_init(w25qxx_dev_t *const dev) {
	W25QXX_RET_t ret;
	ub_t buf;

	CHECK_DEV();
	if (!dev->spi_data_write_read) { printf_dbg_("%s(): spi/qspi_write_read is null\r\n", __func__); return W25QXX_RET_ERR_NULL_PTR; };
	#ifndef W25QXX_DYNAMIC_OVERWRITE_BUFFER
	// if we're not allowing tp use dynamic overwrite buffers, static buffer must be provided
	if (!dev->ow_buf) { return W25QXX_RET_ERR_NULL_PTR; }; // must point to 'uint8_t array[W25QXX_SECTOR_SIZE]'
	#endif

	dev->type = W25Qxx_invalid_dev_id_val;
	if ((ret = w25qxx_chip_detect(dev))) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_detect", ret, w25qxx_ret2str(ret)); return 7; }; // buf[0] = W25QXX_CMD_RELEASE_POWER_DOWN; buf[1] = 0xFF; buf[2] = 0xFF; buf[3] = 0xFF; res = w25qxx_write_read_buf(dev, buf, 4, (uint8_t *)&id, 1);
	if (dev->type >= W25Qxx_max) { printf_dbg_("%s(): device id is invalid (%u)\r\n", __func__, dev->type); return 6;  };

	if ((ret = w25qxx_cmd_reset(dev)))       { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_reset_device", ret, w25qxx_ret2str(ret)); return 7; }; // buf[0] = W25QXX_CMD_ENABLE_RESET;

	dev->addr_mode = W25QXX_ADDR_MODE_3_BYTE;
	if (dev->type >= W25Q256) {
		// NOTE: On power up, the W25Q256FV can operate in either 3-Byte Address Mode or 4-Byte Address Mode, depending on the Non-Volatile Status Register Bit ADP (S17) setting. If ADP=0, the device will operate in  3-Byte Address Mode; if ADP=1, the device will operate in 4-Byte Address Mode. The factory default value for ADP is 0.
		// NOTE: '3b' is factory-default address mode, however, this config is stored in NVM, so better read it first before re-setting it (twice!) every single power-up, needlessly wearing off the chip.

		ret = w25qxx_get_status_reg(dev, 3, buf.U8);
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_get_status_reg_3", ret, w25qxx_ret2str(ret)); return 8; }; // buf[0] = W25QXX_CMD_4BYTE_ADD_MODE_END; res = w25qxx_write_read_buf(dev, buf, 1, NULL, 0);
		if (buf.u8 & W25QXX_STATUS3_CURRENT_ADDRESS_MODE) { dev->addr_mode = W25QXX_ADDR_MODE_4_BYTE; };

		//ret = w25qxx_cmd(dev, W25QXX_CMD_3BYTE_ADDR_MODE);
		//if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_cmd", ret, "W25QXX_CMD_3BYTE_ADDR_MODE"); return 8; }; // buf[0] = W25QXX_CMD_4BYTE_ADD_MODE_END; res = w25qxx_write_read_buf(dev, buf, 1, NULL, 0);
	};

	// Already done in w25qxx_chip_detect();
	//buf.u32 = W25QXX_CMD_RELEASE_POWER_DOWN;
	// if ((ret = w25qxx_write_read_buf(dev, buf.U8, 4, buf.U8, 1))) { printf_dbg_("%s(): release power down failed\r\n", __func__); return ret; }; // buf[0] = W25QXX_CMD_RELEASE_POWER_DOWN; buf[1] = 0xFF; buf[2] = 0xFF; buf[3] = 0xFF; res = w25qxx_write_read_buf(dev, buf, 4, (uint8_t *)&id, 1);
	//if ((ret = w25qxx_release_power_down(dev))) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_release_power_down", ret, w25qxx_ret2str(ret)); return 7; }; // buf[0] = W25QXX_CMD_RELEASE_POWER_DOWN; buf[1] = 0xFF; buf[2] = 0xFF; buf[3] = 0xFF; res = w25qxx_write_read_buf(dev, buf, 4, (uint8_t *)&id, 1);

	if (dev->intf == W25QXX_INTF_QPI) {
		ret = w25qxx_get_status_reg(dev, 2, buf.U8); //
		if (ret) { printf_dbg_(fmt_func_fail_ret_desc, __func__, "w25qxx_get_status_reg_2", ret, w25qxx_ret2str(ret)); return 5; }; // buf[0] = W25QXX_CMD_4BYTE_ADD_MODE_END; res = w25qxx_write_read_buf(dev, buf, 1, NULL, 0);
		//buf.U8[0] = W25QXX_CMD_READ_STATUS_REG2;
		//if ((ret = w25qxx_write_read_buf(dev, buf.U8, 1, buf.U8, 1)))         { printf_dbg_("%s(): read status 2 failed (%X)\r\n", __func__, ret);       return 5; }; // ret = dev->qspi_write_read(W25QXX_CMD_READ_STATUS_REG2, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, (uint8_t *)&status, 1, 1);
		if ((buf.u8 & W25QXX_STATUS2_QUAD_ENABLE) == 0) {
			if ((ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_ENABLE_VOLATILE_SR))) { printf_dbg_("%s(): set sr write enable failed (%X)\r\n", __func__, ret); return 5; }; // ret = dev->qspi_write_read(W25QXX_CMD_VOLATILE_SR_WRITE_ENABLE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
			if ((ret = w25qxx_cmd(dev, W25QXX_CMD_WRITE_STATUS_REG2)))        { printf_dbg_("%s(): write status 2 failed (%X)\r\n", __func__, ret);      return 5; }; // ret = dev->qspi_write_read(W25QXX_CMD_WRITE_STATUS_REG2, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
		};
		if ((ret = w25qxx_cmd(dev, W25QXX_CMD_QPI_MODE_BEG))) { printf_dbg_("%s(): enter qspi failed (%X)\r\n", __func__, ret); return 5; }; // ret = dev->qspi_write_read(W25QXX_CMD_ENTER_QSPI_MODE, 1, 0x00000000, 0x00, 0x00, 0x00000000, 0x00, 0x00, 0x00, NULL, 0x00, NULL, 0x00, 0x00);
	};

	buf.u32 = W25QXX_CMD_READ_MANUFACTURER; //buf[0] = W25QXX_CMD_READ_MANUFACTURER; buf[1] = 0x00; buf[2] = 0x00; buf[3] = 0x00;
	if ((ret = w25qxx_write_read_buf(dev, buf.U8, 4, buf.U8, 2)))   { printf_dbg_("%s(): get manufacturer device id failed (%X)\r\n", __func__, ret); return 4; };
	//buf.u16 = bsd_rev16(buf.u16);//(uint16_t)out[0] << 8 | out[1];
	//if (buf.U8[0] != MANUFACTURER_ID_WINBOND)     { printf_dbg_("%s(): manufacturer id is invalid (%02X != %02X)\r\n", __func__, buf.U8[0], MANUFACTURER_ID_WINBOND); return 6; };
	if ((buf.U8[0] != MANUFACTURER_ID_WINBOND) && (buf.U8[0] != MANUFACTURER_ID_MACRONIX))     { printf_dbg_("%s(): manufacturer id is invalid (%02X != %02X)\r\n", __func__, buf.U8[0], MANUFACTURER_ID_WINBOND); return 6; };
	if ((buf.U8[1] & W25Qxx_mask) != dev->type) { printf_dbg_("%s(): device id is invalid (%02X)\r\n", __func__, buf.U8[1]); return 6; };

	printf_dbg_("%s(): manufacturer_id=%02X; device_id=%02X;\r\n", __func__, buf.U8[0], buf.U8[1]);

	dev->initialized = 1;

	return W25QXX_RET_SUCCESS;
};





