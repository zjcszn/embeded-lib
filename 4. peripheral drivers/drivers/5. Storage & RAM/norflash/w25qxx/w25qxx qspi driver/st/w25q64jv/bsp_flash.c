
#define LOG_TAG "FLASH"
#define LOG_LVL ELOG_LVL_DEBUG

#include <stdlib.h>
#include <string.h>
#include "bsp_flash.h"
#include "w25qxx.h"

#include "elog.h"
#include "perf_counter.h"
#include "bsp_qspi.h"

void bsp_w25qxx_init(void) {
    bsp_qspi_init();
    w25qxx_init();
}

#if 1

#define W25QXX_BUF_SIZE   102400

//ALIGN_32BYTES(uint8_t wr_data[W25QXX_BUF_SIZE]);
//ALIGN_32BYTES(uint8_t rd_data[W25QXX_BUF_SIZE]);

uint8_t *wr_data, *rd_data;

void prv_buffer_compare(void) {
    uint8_t flag = 0;
    for (int i = 0; i < W25QXX_BUF_SIZE; i++) {
        if (rd_data[i] != wr_data[i]) {
            flag = 1;
            log_e("data compare [FAILED], off: %u, write: %02X, read: %02X",
                  i,
                  wr_data[i],
                  rd_data[i]);
            break;
        }
    }
    if (!flag) log_i("data compare [PASS]");
}

void bsp_w25qxx_test(void) {
    uint32_t now, using_time;
    w25x_status_t ret;
    
    wr_data = pvPortMalloc(W25QXX_BUF_SIZE);
    rd_data = pvPortMalloc(W25QXX_BUF_SIZE);
    
    if (!wr_data || !rd_data) goto exit;
    
    log_i("------------w25qxx test start------------");
    log_i("w25qxx test size: %u bytes", W25QXX_BUF_SIZE);
    log_i("write buffer address: %p", wr_data);
    log_i("read  buffer address: %p", rd_data);
    
    /* chip erase test */
    now = (uint32_t)get_system_ms();
    ret = w25qxx_chip_erase();
    using_time = get_system_ms() - now;
    log_d("chip erase %s, using %u ms", ret == W25X_OK ? "success" : "failed", using_time);
    
    /* generate test data */
    srand(get_system_ticks());
    for (int i = 0; i < W25QXX_BUF_SIZE; i++) {
        wr_data[i] = rand() & 0xFF;
    }
    SCB_CleanDCache();
    
    /* write test */
    now = (uint32_t)get_system_us();
    ret = w25qxx_write(wr_data, 0, W25QXX_BUF_SIZE);
    using_time = (uint32_t)get_system_us() - now;
    log_i("write test 1 ---> after chip erase");
    log_i("w25qxx write %s, speed: %.2f Kb/s, time: %u us", ret == W25X_OK ? "success" : "failed",
          ((double)W25QXX_BUF_SIZE / using_time) * 1000, using_time);
    w25qxx_read(rd_data, 0, W25QXX_BUF_SIZE);
    SCB_InvalidateDCache();
    prv_buffer_compare();
    
    now = (uint32_t)get_system_us();
    ret = w25qxx_write(wr_data, 0, W25QXX_BUF_SIZE);
    using_time = (uint32_t)get_system_us() - now;
    log_i("write test 2 ---> sector erase 4K");
    log_i("w25qxx write %s, speed: %.2f Kb/s, time: %u us", ret == W25X_OK ? "success" : "failed",
          ((double)W25QXX_BUF_SIZE / using_time) * 1000, using_time);
    w25qxx_read(rd_data, 0, W25QXX_BUF_SIZE);
    SCB_InvalidateDCache();
    prv_buffer_compare();
    
    /* read test */
    now = (uint32_t)get_system_us();
    ret = w25qxx_read(rd_data, 0, W25QXX_BUF_SIZE);
    using_time = (uint32_t)get_system_us() - now;
    log_i("read test 1 ---> indirect mode");
    log_i("w25qxx read %s, speed: %.2f Kb/s, time: %u us", ret == W25X_OK ? "success" : "failed",
          ((double)W25QXX_BUF_SIZE / using_time) * 1000, using_time);
    SCB_InvalidateDCache();
    prv_buffer_compare();
    
    w25qxx_memory_mapped_enable();
    now = (uint32_t)get_system_us();
    memcpy(rd_data, (const void *)0x90000000, W25QXX_BUF_SIZE);
    using_time = (uint32_t)get_system_us() - now;
    log_i("read test 2 ---> memory mapped mode");
    log_i("w25qxx read speed: %.2f Kb/s, time: %u us",
          ((double)W25QXX_BUF_SIZE / using_time) * 1000, using_time);
    prv_buffer_compare();
    
    log_i("-------------w25qxx test end-------------");

exit:
    if (wr_data) {
        vPortFree(wr_data);
        wr_data = NULL;
    }
    if (rd_data) {
        vPortFree(rd_data);
        rd_data = NULL;
    }
}

#endif