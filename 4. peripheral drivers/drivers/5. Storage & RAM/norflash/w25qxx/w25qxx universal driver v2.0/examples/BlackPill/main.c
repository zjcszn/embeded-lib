// Target MCU: STM32F401CCU6

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "bsd.h"

int _sbrk; // missing nano.spec requirement when using <stdio.h>
#define PRINT_BUF_SIZE 255
char PRINT_BUF[PRINT_BUF_SIZE];
//#define printf_dbg(...) { const uint8_t len = snprintf(PRINT_BUF, sizeof(PRINT_BUF), __VA_ARGS__); USART_DBG_DataTxGeneric(USART1, PRINT_BUF, len); }
//__attribute__ ((used)) int _write (int fd, char *ptr, int len) { (void)fd; USART_DBG_DataTxGeneric(USART1, ptr, len); return len; }; // nah, direct use of 'printf()' drags along too much garbage

int printf_dbg(const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	//if (print_buf_lock()) { return 0; };
	//const int len = print(PRINT_BUF, PRINT_BUF + PRINT_BUF_SIZE - 1, format, args);
	const int len = vsnprintf(PRINT_BUF, sizeof(PRINT_BUF), format, args);
	va_end(args);
	USART_DBG_DataTxGeneric(USART1, PRINT_BUF, len);
	//print_buf_unlock();
	return len;
};

uint8_t w25qxx_ow_buf[W25QXX_SECTOR_SIZE];
w25qxx_dev_t w25qxx_dev = {0};
static W25QXX_RET_t w25qxx_spi_transaction_data_write_read(const void *cmd, uint32_t cmd_len, void *data_io, uint32_t data_io_len) {
	//printf_dbg("%s(%p,%u,%p,%u):\n", __func__, cmd, cmd_len, data_io, data_io_len);

	PinLo(W25QXX_PIN_CS);
	do {
		const void *buf_end;
		buf_end = cmd + cmd_len;
		while (cmd < buf_end) { SPI1_ByteTx(*(uint8_t *)cmd++); };

		if (!data_io || !data_io_len) { break; };
		buf_end = data_io + data_io_len;
		while (data_io < buf_end) { *(uint8_t *)data_io = SPI1_ByteTRx(*(uint8_t *)data_io); ++data_io; };
	} while (0);
	PinHi(W25QXX_PIN_CS);

	//printf_dbg("%s(): Done.\n", __func__);
	return W25QXX_RET_SUCCESS;
};
static W25QXX_RET_t w25qxx_spi_transaction_data_write(const void *cmd, uint32_t cmd_len, const void *data_out, uint32_t data_out_len) {
	//printf_dbg("%s(%p,%u,%p,%u):\n", __func__, cmd, cmd_len, data_out, data_out_len);

	PinLo(W25QXX_PIN_CS);
	do {
		const void *buf_end;
		buf_end = cmd + cmd_len;
		while (cmd < buf_end) { SPI1_ByteTx(*(uint8_t *)cmd++); };

		if (!data_out || !data_out_len) { break; };
		buf_end = data_out + data_out_len;
		while (data_out < buf_end) { SPI1_ByteTx(*(uint8_t *)data_out); ++data_out; };
	} while (0);
	PinHi(W25QXX_PIN_CS);

	//printf_dbg("%s(): Done.\n", __func__);
	return W25QXX_RET_SUCCESS;
};

void delay_us(uint32_t microsec) {
	// Waste some time and lower the load on memory bus (less r/w operations)
	// @ 84Mhz 1 tick = 0.0119 us => 1 us =~ 42 'nop' operations, as it takes ~ 2 clock cycles to exec 'nop'

	while (microsec--) {
		__asm volatile (
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			/// 16
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			/// 32
			"nop; nop; nop; nop\n"
			"nop; nop; nop; nop\n"
			"nop; nop;\n"
			/// 42
		);
	};
};
void delay_ms(uint32_t millisec) { delay_us(millisec*1000); };

static char *bytes2str(const uint8_t *data, uint16_t data_len, char *buf, size_t buf_size) {
	const char RADIX_SEQ[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const uint8_t *const data_end = data + data_len;
	char *const buf_beg = buf;
	char *const buf_end = buf + buf_size;

	for(;;) {
		if (data >= data_end) { break; };
		if (buf+3 >= buf_end) { break; };
		*buf++ = RADIX_SEQ[(((*data)>>4) & 0xF)];
		*buf++ = RADIX_SEQ[( (*data++)   & 0xF)];
	};
	*buf = 0;

	return buf_beg;
};

static void w25qxx_init_io(void) {

	// NOTE: SPI peripheral itself and it's related GPIOs initialized inside 'board_init()'

	PinHi(W25QXX_PIN_CS);
	w25qxx_dev.ow_buf = w25qxx_ow_buf;
	w25qxx_dev.spi_data_write_read = w25qxx_spi_transaction_data_write_read;
	w25qxx_dev.spi_data_write = w25qxx_spi_transaction_data_write;
};
static int w25qxx_test(void) {
	int ret;
	uint8_t *const read_buf = (uint8_t *)PRINT_BUF + PRINT_BUF_SIZE - W25QXX_UID_LEN; // NOTE: re-using bottom blocks of the buffer, W25QXX_UID_LEN == 8
	const uint8_t uid_str_buf_size = W25QXX_UID_LEN * 2 + 1;
	char *const uid_str_buf = PRINT_BUF + PRINT_BUF_SIZE - W25QXX_UID_LEN - uid_str_buf_size;

	ret = w25qxx_init(&w25qxx_dev);
	if (ret != W25QXX_RET_SUCCESS) { // W25QXX_RET_SUCCESS == 0
		printf_dbg("w25qxx_init(): Failed with code: %02X\n", ret);
		return ret;
	};

	ret = w25qxx_get_unique_id(&w25qxx_dev, read_buf);
	if (ret) { printf_dbg("w25qxx_get_unique_id(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret)); return ret; };
	printf_dbg("Chip 1 UID: '%s'\n", bytes2str(read_buf, W25QXX_UID_LEN, uid_str_buf, uid_str_buf_size));

	w25qxx_info_t info;
	ret = w25qxx_get_info(&w25qxx_dev, &info);
	if (ret) { printf_dbg("w25qxx_get_info(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret)); return ret; };
	printf_dbg(
		"Manufacturer:\t%s\n"
		"Dev_type:\t%s\n"
		"Dev_cap:\t%u Kbytes\n"
		"Interface:\t%s\n"
		"Intf_mode:\t%s\n"
		"Addr_mode:\t%.2s\n"
		//"Supply volt:\t%.1f..%.1f\n" // NOTE: when using 'nano.specs', 'printf()' doesn't support printing floats
		"VCC x10:\t%d..%d\n"
		"Supply curr:\t%u ma\n"
		"Temp_range:\t%d*C..%d*C\n"
		"Driver vers:\t%X.%X\n\n",
		info.manufacturer,
		info.chip,
		w25qxx_get_storage_capacity_bytes(&w25qxx_dev)/1024,
		info.interface,
		info.intf_mode,
		info.addr_mode,
		//info.supply_volt_min, info.supply_volt_max,
		(int)(info.supply_volt_min*10), (int)(info.supply_volt_max*10),
		info.max_current_ma,
		info.temperature_max, info.temperature_min,
		(info.driver_version >> 4) & 0xF, info.driver_version & 0xF
	);

	const uint32_t addr = w25qxx_get_storage_capacity_bytes(&w25qxx_dev) - W25QXX_WRITE_BLOCK_SIZE;
	const uint32_t val = 0x12345678;
//	const uint32_t val = 0xABCDEF09;

	ret = w25qxx_read_fast(&w25qxx_dev, addr, read_buf, sizeof(ret));
	if (ret) { printf_dbg("w25qxx_read_fast(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret));  return ret; };
	ret = *(int *)read_buf;
	printf_dbg("Chip 1 read data_pre: [%u bytes @ 0x%08X]: '%X'\n", sizeof(ret), addr, ret);

	printf_dbg("Chip 1 writing data: [%u bytes @ 0x%08X]: '%X'... ", sizeof(val), addr, val);
	ret = w25qxx_overwrite_verify(&w25qxx_dev, addr, &val, sizeof(val));
	if (ret) { printf_dbg("w25qxx_overwrite_verify(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret));  return ret; } else { printf_dbg("OK\n"); };

	ret = w25qxx_read_fast(&w25qxx_dev, addr, read_buf, sizeof(ret));
	if (ret) { printf_dbg("w25qxx_read_fast(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret));  return ret; };
	ret = *(int *)read_buf;
	printf_dbg("Chip 1 read data_post: [%u bytes @ 0x%08X]: '%X'\n", sizeof(ret), addr, ret);


	return 0;
};

int main(void) {

	board_init();
	printf_dbg("Starting @ %uMHz\n", SystemCoreClock/1000000);

	w25qxx_init_io();
	w25qxx_test();

	while(1) { LedTog(LED_BLUE); delay_ms(100); };

	return 0;
};
