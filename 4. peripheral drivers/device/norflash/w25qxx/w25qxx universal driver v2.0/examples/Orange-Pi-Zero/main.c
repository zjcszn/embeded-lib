#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "../../src/w25qxx.h"
//#include "intf_opi_w25qxx.h"

// implemented inside 'intf_opi_w25qxx.c'
extern int w25qxx_intf_init(uint8_t bus_id, uint8_t dev_id);
extern w25qxx_dev_t w25qxx_dev;

extern void delay_us(uint32_t microsec);
extern void delay_ms(uint32_t millisec);

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

int printf_dbg(const char *restrict format, ...) {
	va_list args;
	va_start(args, format);
	const int len = vprintf(format, args);
	va_end(args);
	return len;
};

static int w25qxx_test(void) {
	int ret;
	char read_buf[W25QXX_UID_LEN] = {0}; // W25QXX_UID_LEN == 8

	ret = w25qxx_init(&w25qxx_dev);
	if (ret != W25QXX_RET_SUCCESS) { // W25QXX_RET_SUCCESS == 0
		printf("w25qxx_init(): Failed with code: %02X\r\n", ret);
		return ret;
	};

	ret = w25qxx_get_unique_id(&w25qxx_dev, read_buf);
	if (ret) { printf("w25qxx_get_unique_id(): Failed with code: %02X (desc: '%s')\r\n", ret, w25qxx_ret2str(ret)); return ret; };
	char uid_str[W25QXX_UID_LEN * 2 + 1] = {0};
	printf("Chip 1 UID: '%s'\r\n", bytes2str(read_buf, W25QXX_UID_LEN, uid_str, sizeof(uid_str)));

	w25qxx_info_t info;
	ret = w25qxx_get_info(&w25qxx_dev, &info);
	if (ret) { printf("w25qxx_get_info(): Failed with code: %02X (desc: '%s')\r\n", ret, w25qxx_ret2str(ret)); return ret; };
	printf(
		"Manufacturer:\t%s\n"
		"Dev_type:\t%s\n"
		"Dev_cap:\t%u Kbytes\n"
		"Interface:\t%s\n"
		"Intf_mode:\t%s\n"
		"Addr_mode:\t%.2s\n"
		"Supply volt:\t%.1f..%.1f\n"
		"Supply curr:\t%u ma\n"
		"Temp_range:\t%d°C..%d°C\n"
		"Driver vers:\t%X.%X\n\n",
		info.manufacturer,
		info.chip,
		w25qxx_get_storage_capacity_bytes(&w25qxx_dev)/1024,
		info.interface,
		info.intf_mode,
		info.addr_mode,
		info.supply_volt_min, info.supply_volt_max,
		info.max_current_ma,
		info.temperature_max, info.temperature_min,
		(info.driver_version >> 4) & 0xF, info.driver_version & 0xF
	);

	const uint32_t addr = w25qxx_get_storage_capacity_bytes(&w25qxx_dev) - W25QXX_WRITE_BLOCK_SIZE;
	const uint32_t val = 0x12345678;
//	const uint32_t val = 0xABCDEF09;

	ret = w25qxx_read_fast(&w25qxx_dev, addr, read_buf, sizeof(ret));
	if (ret) { printf("w25qxx_read_fast(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret));  return ret; };
	ret = *(int *)read_buf;
	printf("Chip 1 read data_pre: [%u bytes @ 0x%08X]: '%X'\n", sizeof(ret), addr, ret);

	/* NOTE: Overwriting will not work due to hard-coded limitation of the spidev driver inside kernel (as of Oct 2022),
	         which doesn't allow transferring more than 4095 bytes in one transaction (https://github.com/orangepi-xunlong/orangepi_h3_linux/blob/master/OrangePi-Kernel/linux-4.9/drivers/spi/spidev.c)
	         AND current architecture of the library, which requires performing (5+4096) = 4101 bytes long transactions for overwriting sectors
	         Nevertheless, initializing and reading works just fine, which is enough for this little demo.

	printf("Chip 1 writing data: [%u bytes @ 0x%08X]: '%X'... ", sizeof(val), addr, val);
	ret = w25qxx_overwrite_verify(&w25qxx_dev, addr, &val, sizeof(val));
	if (ret) { printf("w25qxx_overwrite_verify(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret));  return ret; } else { printf("OK\n"); };

	ret = w25qxx_read_fast(&w25qxx_dev, addr, read_buf, sizeof(ret));
	if (ret) { printf("w25qxx_read_fast(): Failed with code: %02X (desc: '%s')\n", ret, w25qxx_ret2str(ret));  return ret; };
	ret = *(int *)read_buf;
	printf("Chip 1 read data_post: [%u bytes @ 0x%08X]: '%X'\n", sizeof(ret), addr, ret);
	*/

	return 0;
};


int main(int argc, char *argv[]) {
	int ret;

	//const char data[] = { 1, 2, 3, 4, 5, 0xAB, 0xCD, 0xEF };
	//char buf[64];
	//printf("bytes2str() test: '%s'\n", bytes2str(data, sizeof(data), buf, sizeof(buf)));
	//delay_ms(1000); // test ok!

	printf("Initializing SPI interface...");
	ret = w25qxx_intf_init(0,0);
	if (ret) {
		puts("Failed!");

		char hr[64]; for (uint8_t i=0;i<sizeof(hr);++i) {hr[i] = '=';}; hr[sizeof(hr)-1] = 0;
		puts(hr);
		puts(
			"  Make sure to enable 'spidev0.0' in device overlay settings file at '/boot/armbianEnv.txt'\n"
			"  By adding following lines:\n\n"
			"    overlays=spi-spidev\n"
			"    param_spidev_spi_bus=0\n\n"
			"  And rebooting the device.\n"
			"  One can test related spi flash chip on an Orange Pi Zero board with following command:\n\n"
			"    flashrom -p linux_spi:dev=/dev/spidev0.0,spispeed=16000 -c MX25L1605A/MX25L1606E/MX25L1608E -VVV\n\n"
			"  NOTE: 'flashrom' might be installed with following command: 'apt install flashrom'\n"
		);
		puts(hr);

		return ret;
	};
	puts("OK");

	puts("Testing spi flash chip...\n");
	ret = w25qxx_test();
	if (ret) { puts("TEST: Failed!"); return 1; };

	puts("TEST: Done.");

	return 0;
};//main()
