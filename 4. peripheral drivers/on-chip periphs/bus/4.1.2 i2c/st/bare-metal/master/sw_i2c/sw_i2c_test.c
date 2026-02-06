
#include <stdio.h>
#include <string.h>

#include "sw_i2c_port.h"

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
    printf("[I2C Test] Clock Streching: %s\r\n",
           i2c_dev->enable_clock_stretch ? "Enabled" : "Disabled");
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

/**
 * @brief  Test I2C Bus Unlock/Recovery
 * @param  i2c_dev Initialized I2C handle
 */
void sw_i2c_test_unlock(sw_i2c_t *i2c_dev) {
    printf("[I2C] Testing Bus Unlock...\n");
    sw_i2c_err_t ret = sw_i2c_unlock(i2c_dev);
    if (ret == SOFT_I2C_OK) {
        printf("[I2C] Bus Unlock Successful (or Bus was already Free)\n");
    } else {
        printf("[I2C] Bus Unlock FAILED (Bus still stuck)\n");
    }
}

/**
 * @brief  Test new API extensions
 * @param  i2c_dev Initialized I2C handle
 */
void sw_i2c_test_api_ext(sw_i2c_t *i2c_dev) {
    uint16_t old_addr = sw_i2c_get_addr(i2c_dev);
    printf("[I2C] Testing API Extensions...\n");

    // Test Set/Get Addr
    sw_i2c_set_addr(i2c_dev, 0x55);
    if (sw_i2c_get_addr(i2c_dev) == 0x55) {
        printf("  [PASS] Set/Get Addr\n");
    } else {
        printf("  [FAIL] Set/Get Addr\n");
    }

    // Restore addr
    sw_i2c_set_addr(i2c_dev, old_addr);

    // Test Device Ready (Assumes device at old_addr exists, or at least we test the call)
    if (sw_i2c_is_device_ready(i2c_dev, old_addr) == SOFT_I2C_OK) {
        printf("  [INFO] Device at 0x%02X is READY\n", old_addr);
    } else {
        printf("  [INFO] Device at 0x%02X is NOT READY (NACK)\n", old_addr);
    }
}

/**
 * @brief  Test Register Access APIs (8-bit and 16-bit)
 * @param  i2c_dev Initialized I2C handle
 * @param  reg_addr Register address to test
 */
void sw_i2c_test_reg_access(sw_i2c_t *i2c_dev, uint8_t reg_addr) {
    printf("[I2C] Testing Register Access APIs...\n");
    uint8_t val8 = 0;
    uint16_t val16 = 0;

    // Test 8-bit Write/Read
    printf("  [TEST] Write Reg8 0x%02X <- 0xAB\n", reg_addr);
    if (sw_i2c_write_reg8(i2c_dev, reg_addr, 0xAB) == SOFT_I2C_OK) {
        if (sw_i2c_read_reg8(i2c_dev, reg_addr, &val8) == SOFT_I2C_OK) {
            if (val8 == 0xAB)
                printf("    [PASS] Read back 0xAB\n");
            else
                printf("    [FAIL] Read back 0x%02X != 0xAB\n", val8);
        } else {
            printf("    [FAIL] Read Reg8 Failed\n");
        }
    } else {
        printf("    [FAIL] Write Reg8 Failed\n");
    }

    // Test 16-bit Write/Read
    printf("  [TEST] Write Reg16 0x%02X <- 0x1234\n", reg_addr);
    if (sw_i2c_write_reg16(i2c_dev, reg_addr, 0x1234) == SOFT_I2C_OK) {
        if (sw_i2c_read_reg16(i2c_dev, reg_addr, &val16) == SOFT_I2C_OK) {
            if (val16 == 0x1234)
                printf("    [PASS] Read back 0x1234\n");
            else
                printf("    [FAIL] Read back 0x%04X != 0x1234\n", val16);
        } else {
            printf("    [FAIL] Read Reg16 Failed\n");
        }
    } else {
        printf("    [FAIL] Write Reg16 Failed\n");
    }
}

/**
 * @brief  Test Clock Stretching API
 * @param  i2c_dev Initialized I2C handle
 */
void sw_i2c_test_clock_stretch_api(sw_i2c_t *i2c_dev) {
    printf("[I2C] Testing Clock Stretch API...\n");

    // Test Enable
    if (sw_i2c_clock_stretch_enable(i2c_dev, true) == SOFT_I2C_OK) {
        if (i2c_dev->enable_clock_stretch == true) {
            printf("  [PASS] Enable Clock Stretch\n");
        } else {
            printf("  [FAIL] Enable Clock Stretch (internal flag mismatch)\n");
        }
    } else {
        printf("  [FAIL] Enable Clock Stretch call failed\n");
    }

    // Test Disable
    if (sw_i2c_clock_stretch_enable(i2c_dev, false) == SOFT_I2C_OK) {
        if (i2c_dev->enable_clock_stretch == false) {
            printf("  [PASS] Disable Clock Stretch\n");
        } else {
            printf("  [FAIL] Disable Clock Stretch (internal flag mismatch)\n");
        }
    } else {
        printf("  [FAIL] Disable Clock Stretch call failed\n");
    }
}

/**
 * @brief  Example of how to use the new Init API
 * @return 0 on success, error code otherwise
 */
sw_i2c_err_t sw_i2c_test_init_example(void) {
    sw_i2c_t my_i2c_dev;

    // Define configuration
    sw_i2c_port_cfg_t cfg = {
        .scl_port = GPIOB,  // Use actual GPIO_TypeDef* if available or void* cast
        .scl_pin = 0x0040,  // Example PIN_6
        .sda_port = GPIOB,
        .sda_pin = 0x0080,  // Example PIN_7
        .freq_khz = 400,
        .sys_clk_hz = 0  // Auto-detect
    };

    // Initialize
    if (sw_i2c_init_default(&my_i2c_dev, &cfg) != SOFT_I2C_OK) {
        printf("[I2C] Init Failed\n");
        return SOFT_I2C_ERR_BUS;
    }

    printf("[I2C] Init Success\n");
    return SOFT_I2C_OK;
}
