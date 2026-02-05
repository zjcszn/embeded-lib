
#include <stdio.h>
#include <string.h>

#include "bsp_i2c_sw_port.h"


/* Defines -------------------------------------------------------------------*/
#define TEST_BUFFER_SIZE 32

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Simple hex dump helper
 */
static void hex_dump(const char *desc, const uint8_t *data, uint16_t len) {
    printf("%s (%d bytes):\n", desc, len);
    for (uint16_t i = 0; i < len; i++) {
        printf(" %02X", data[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Scan I2C bus for devices
 * @param  i2c_dev Initialized I2C handle
 */
void sw_i2c_test_scan(sw_i2c_t *i2c_dev) {
    // Wrapper around the driver's scan function
    if (sw_i2c_scan(i2c_dev) != SOFT_I2C_OK) {
        printf("[I2C Test] Scan failed or driver error.\r\n");
    }
}

/**
 * @brief  Test Read/Write to a specific memory address
 * @param  i2c_dev Initialized I2C handle with target address set
 * @param  mem_addr Memory address to write/read
 * @param  addr_len Length of memory address
 */
void sw_i2c_test_rw(sw_i2c_t *i2c_dev, uint32_t mem_addr, uint16_t addr_len) {
    uint8_t tx_buf[TEST_BUFFER_SIZE];
    uint8_t rx_buf[TEST_BUFFER_SIZE];
    sw_i2c_err_t ret;

    printf("[I2C] RW Test @ Addr 0x%02X, Reg 0x%X\n", i2c_dev->dev_addr, mem_addr);

    // Prepare pattern
    for (int i = 0; i < TEST_BUFFER_SIZE; i++) {
        tx_buf[i] = (uint8_t) (i + 0x55);
    }

    // Write
    printf("[I2C] Writing...\n");
    ret = sw_i2c_master_mem_write(i2c_dev, mem_addr, addr_len, tx_buf, TEST_BUFFER_SIZE);
    if (ret != SOFT_I2C_OK) {
        printf("[I2C] Write Failed! Err: %d\n", ret);
        return;
    }

    // Read back
    memset(rx_buf, 0, TEST_BUFFER_SIZE);
    printf("[I2C] Reading...\n");
    ret = sw_i2c_master_mem_read(i2c_dev, mem_addr, addr_len, rx_buf, TEST_BUFFER_SIZE);
    if (ret != SOFT_I2C_OK) {
        printf("[I2C] Read Failed! Err: %d\n", ret);
        return;
    }

    // Compare
    if (memcmp(tx_buf, rx_buf, TEST_BUFFER_SIZE) == 0) {
        printf("[I2C] Verify SUCCESS!\n");
    } else {
        printf("[I2C] Verify FAILED!\n");
        hex_dump("Expected", tx_buf, TEST_BUFFER_SIZE);
        hex_dump("Actual", rx_buf, TEST_BUFFER_SIZE);
    }
}
