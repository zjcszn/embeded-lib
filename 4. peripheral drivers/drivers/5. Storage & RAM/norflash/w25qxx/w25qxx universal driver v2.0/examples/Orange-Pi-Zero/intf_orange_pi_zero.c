#include <stdint.h>
#include <stdio.h>
#include <linux/spi/spidev.h>

#include <unistd.h> // close()
#include <time.h>
//#include <stdlib.h> // abort()
#include <sys/ioctl.h>
#include <fcntl.h> // open()

//#include <errno.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/mman.h>

#include "../../src/w25qxx.h"

uint8_t w25qxx_ow_buf[W25QXX_SECTOR_SIZE];
w25qxx_dev_t w25qxx_dev = {0};

#define SPI_DATA_MAX_LEN	4095
#define SPI_CLOCK_BASE		16000000		// 16Mhz
#define MSBFIRST			0
#define LSBFIRST			SPI_LSB_FIRST
#define MIN(x,y)			((x < y)?(x):(y))
typedef struct spi_ioc_transfer spi_ioc_transfer_t;

typedef struct {
	uint8_t busid; // 0 => '/dev/spidev0.0'; 01 => '/dev/spidev0.1'; 10 => '/dev/spidev1.0'; etc.
	uint8_t mode; // SPI_MODE_0, SPI_MODE_1, ... SPI_MODE_3
	uint8_t bits;
	uint8_t bord;
	uint32_t freq_hz;
	int fd;
	//struct spi_ioc_transfer tr;
} spi_dev_t;

static spi_dev_t spi00 = { .busid = 0, .fd = -1, .bits = 8, .bord = MSBFIRST, .mode = SPI_MODE_3, .freq_hz = SPI_CLOCK_BASE, };

void delay_us(uint32_t microsec) {
	struct timespec sleeper;
	sleeper.tv_sec  = (time_t)(microsec / 1000000);
	sleeper.tv_nsec = (long)(microsec % 1000000) * 1000;
	nanosleep(&sleeper, NULL);
};
void delay_ms(uint32_t millisec) {
	struct timespec sleeper;
	sleeper.tv_sec  = (time_t)(millisec / 1000);
	sleeper.tv_nsec = (long)(millisec % 1000) * 1000000;
	nanosleep(&sleeper, NULL);
};

_Static_assert((sizeof(spi00.freq_hz) == sizeof(((spi_ioc_transfer_t *)NULL)->speed_hz)), "ERROR: 'freq_hz != speed_hz' is not true!");

static void spi_deinit(spi_dev_t *dev) { if (dev->fd < 0) { return; }; close(dev->fd); dev->fd = -1; };
static int spi_init(spi_dev_t *dev) {
	int ret = -1;
	//dev->tr.speed_hz = speed_hz;

	if (dev->fd >= 0) { return 1; };// set spidev accordingly to busNo like: busNo = 0x23 -> /dev/spidev2.3
	char dev_path[] = "/dev/spidev0.0";
	dev_path[11] += (dev->busid >> 4) & 0x0F;
	dev_path[13] += (dev->busid >> 0) & 0x0F;

	dev->fd = open(dev_path, O_RDWR);
	if (dev->fd < 0) { fprintf(stderr, "ERROR: Could not open SPI device '%s'\n", dev_path); return ret; };

	ret = ioctl(dev->fd, SPI_IOC_WR_MODE, &dev->mode);
	if (ret == -1) { fprintf(stderr, "Can't set SPI write mode.\n"); return ret; }

	ret = ioctl(dev->fd, SPI_IOC_RD_MODE, &dev->mode);
	if (ret == -1) { fprintf(stderr, "Can't set SPI read mode.\n"); return ret; }

	const uint8_t bits = 8;	// 8 bits per word
	ret = ioctl(dev->fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) { fprintf(stderr, "Can't set SPI write bits per word.\n"); return ret; }

	ret = ioctl(dev->fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) { fprintf(stderr, "Can't set SPI read bits per word.\n"); return ret; }

	ret = ioctl(dev->fd, SPI_IOC_WR_MAX_SPEED_HZ, &dev->freq_hz);
	if (ret == -1) { fprintf(stderr, "Can't set SPI write max speed hz.\n"); return ret; }

	ret = ioctl(dev->fd, SPI_IOC_RD_MAX_SPEED_HZ, &dev->freq_hz);
	if (ret == -1) { fprintf(stderr, "Can't set SPI read max speed hz.\n"); return ret; }

	ret = ioctl(dev->fd, SPI_IOC_WR_LSB_FIRST, &dev->bord);
	if (ret == -1) { fprintf(stderr, "Can't set SPI write bit order.\n"); return ret; }

	ret = ioctl(dev->fd, SPI_IOC_RD_LSB_FIRST, &dev->bord);
	if (ret == -1) { fprintf(stderr, "Can't set SPI read bit order.\n"); return ret; };

	return 0;
};

static W25QXX_RET_t spi_data_write_read(const void *buf_out, uint32_t len_out, void *data_io, uint32_t data_io_len) {
	//printf("%s(%p,%u,%p,%u): called\n", __func__, buf_out, len_out, data_io, data_io_len);

	const uint8_t txn = 2;
	const spi_ioc_transfer_t tx_data[2] = {
		{ .speed_hz = spi00.freq_hz, .tx_buf = (unsigned long)buf_out, .rx_buf =                      0, .len = len_out, },
		{ .speed_hz = spi00.freq_hz, .tx_buf = (unsigned long)data_io, .rx_buf = (unsigned long)data_io, .len = data_io_len, },
	};

	const int ret = ioctl(spi00.fd, SPI_IOC_MESSAGE(txn), tx_data);
	if (ret < 1) { fprintf(stderr, "ERROR: spi msg trx failed: %d\n", ret); return W25QXX_RET_ERR_IO_FAIL; };

	return W25QXX_RET_ERR_NONE;
};
static W25QXX_RET_t spi_data_write(const void *cmd_buf, uint32_t cmd_len, const void *data_out, uint32_t data_out_len) {
	//printf("%s(%p,%u,%p,%u): called\n", __func__, cmd_buf, cmd_len, data_out, data_out_len);

	const uint8_t txn = 2;
	const spi_ioc_transfer_t tx_data[2] = {
		{ .speed_hz = spi00.freq_hz, .tx_buf = (unsigned long)cmd_buf, .rx_buf = 0, .len = cmd_len, },
		{ .speed_hz = spi00.freq_hz, .tx_buf = (unsigned long)data_out, .rx_buf = 0, .len = data_out_len, },
	};

	const int ret = ioctl(spi00.fd, SPI_IOC_MESSAGE(txn), tx_data);
	if (ret < 1) { fprintf(stderr, "ERROR: spi msg tx failed: %d\n", ret); return W25QXX_RET_ERR_IO_FAIL; };

	return W25QXX_RET_ERR_NONE;
};


int w25qxx_intf_init(uint8_t bus_id, uint8_t dev_id) {
	int ret = 1;

	spi00.busid = ((bus_id % 10) << 4) | (dev_id % 10);
	ret = spi_init(&spi00);
	if (ret) { return ret; };

	w25qxx_dev.ow_buf = w25qxx_ow_buf;
	w25qxx_dev.spi_data_write_read = spi_data_write_read;
	w25qxx_dev.spi_data_write = spi_data_write;

	return 0;
};

