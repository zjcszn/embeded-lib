#include "main.h"
#include "w25n1xx.h"
#include "bsp_spi1_nandflash.h"
#include "stdbool.h"
#include "dwt_delay.h"

static inline uint8_t w25n1xx_write_read_byte(uint8_t c) {
    return bsp_spi_nandflash_read_write_byte(c);
}

static inline void w25n1xx_write_buffer(uint8_t *buffer, uint32_t len) {
    if (bsp_spi_nandflash_write_buffer(buffer, len) != true) {
        LOG("spi dma write failed\r\n");
    }
}

static inline void w25n1xx_read_buffer(uint8_t *buffer, uint32_t len) {
    if (bsp_spi_nandflash_read_buffer(buffer, len) != true) {
        LOG("spi dma read failed\r\n");
    }
}

#define W25N1XX_DELAY_MS(ms)        delay_ms(ms)

#if defined(W25N1XX_USE_READBACK)
static uint8_t w25n1xx_buf[W25N1XX_PAGE_TOL_SIZE];
#endif

#define NAND_ROW_ADDR_H(paddr)      (uint8_t)(((paddr) >> (W25N1XX_PAGE_SHIFT + 8)) & 0xFF)
#define NAND_ROW_ADDR_L(paddr)      (uint8_t)(((paddr) >> (W25N1XX_PAGE_SHIFT)) & 0xFF)

#define NAND_COL_ADDR_H(caddr)      (uint8_t)(((caddr) >> 8) & 0xFF)
#define NAND_COL_ADDR_L(caddr)      (uint8_t)(((caddr) & 0xFF))

w25n1xx_status_t w25n1xx_init(void) {
    uint32_t jedec_id;
    uint8_t sr;

    bsp_spi_nandflash_init();

    w25n1xx_reset();

    jedec_id = w25n1xx_read_jedec_id();

    if (jedec_id != W25N1XX_ID) {
        LOG("w25n1xx init failed\r\n");
        return W25N1XX_ERR;
    }

    sr = w25n1xx_read_status_register(SR1_PROTECTION);
    if (sr != 0x02) {
        LOG("sr1: %02X\r\n", sr);
        w25n1xx_write_status_register(SR1_PROTECTION, 0);
    }

    sr = w25n1xx_read_status_register(SR2_CONFIGURATION);
    if ((sr & (W25N1XX_SR2_ECC_ENABLE | W25N1XX_SR2_BUF)) != 0x08) {
        LOG("sr2: %02X\r\n", sr);
        w25n1xx_write_status_register(SR2_CONFIGURATION, (sr & ~0x18) | 0x08);
    }

    return W25N1XX_OK;
}

uint32_t w25n1xx_read_jedec_id(void) {
    uint32_t jedec_id = 0;

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_JEDEC_ID);
    w25n1xx_write_read_byte(0xFF);
    jedec_id |= (w25n1xx_write_read_byte(0xFF) << 16);
    jedec_id |= (w25n1xx_write_read_byte(0xFF) <<  8); 
    jedec_id |= (w25n1xx_write_read_byte(0xFF));
    NAND_FLASH_CS_H();

    LOG("jedec id: %08X\r\n", jedec_id);
    return jedec_id;
}

void w25n1xx_reset(void) {
    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_RESET);
    NAND_FLASH_CS_H();
    W25N1XX_DELAY_MS(10);
}

void w25n1xx_write_enable(bool enable) {
    uint8_t cmd, sr;
    cmd = enable ? W25N1XX_CMD_WRITE_ENABLE : W25N1XX_CMD_WRITE_DISABLE;

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(cmd);
    NAND_FLASH_CS_H();

    if (enable) {
        do {
            sr = w25n1xx_read_status_register(SR3_STATUS);
        } while (!(sr & W25N1XX_SR3_WEL));
    }
}

uint8_t w25n1xx_read_status_register(uint8_t sr_addr) {
    uint8_t reg_val;

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_READ_SR);
    w25n1xx_write_read_byte(sr_addr);
    reg_val = w25n1xx_write_read_byte(0xFF);
    NAND_FLASH_CS_H();

    return reg_val;
}

void w25n1xx_write_status_register(uint8_t sr_addr, uint8_t reg_val) {
    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_WRITE_SR);
    w25n1xx_write_read_byte(sr_addr);
    w25n1xx_write_read_byte(reg_val);
    NAND_FLASH_CS_H();
}

void w25n1xx_wait_ready(void) {
    uint8_t sr3;
    do {
        sr3 = w25n1xx_read_status_register(SR3_STATUS);
    } while (sr3 & W25N1XX_SR3_BUSY);
} 

w25n1xx_status_t w25n1xx_block_erase(uint32_t paddr) {
    uint8_t dummy = 0, status;
    uint8_t bad_block = 0;

    /* TODO: check bad block */
    if (w25n1xx_page_read(paddr, 2048, &bad_block, 1) != W25N1XX_OK) {
        return W25N1XX_EERASE;
    }

    if (bad_block != 0xFF) {
        LOG("found bad block, %u\r\n", paddr >> (11 + 6));
        return W25N1XX_EBADBLOCK;
    }

    w25n1xx_write_enable(true);

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_BLOCK_ERASE);
    w25n1xx_write_read_byte(dummy);
    w25n1xx_write_read_byte(NAND_ROW_ADDR_H(paddr));
    w25n1xx_write_read_byte(NAND_ROW_ADDR_L(paddr));
    NAND_FLASH_CS_H();

    w25n1xx_wait_ready();
    w25n1xx_write_enable(false);
    status = w25n1xx_read_status_register(SR3_STATUS);

    if (status & W25N1XX_SR3_EFAIL) {
        LOG("w25n1xx erase failed\r\n");
        return W25N1XX_EERASE;
    }

    // LOG("w25n1xx erase success, 0x%08X\r\n", paddr);

    return W25N1XX_OK;
}

w25n1xx_status_t w25n1xx_ecc_check(void) {
    uint8_t ecc;

    ecc = w25n1xx_read_status_register(SR3_STATUS) & W25N1XX_SR3_ECC_ERR;

    if (ecc == W25N1XX_ECC_OK) {
        return W25N1XX_OK;
    } else if (ecc == W25N1XX_ECC_1BIT_CORRECTION) {
        return W25N1XX_ECORR;
    } else {
        return W25N1XX_EUNCORR;
    }
}


w25n1xx_status_t w25n1xx_page_read(uint32_t paddr, uint16_t caddr, uint8_t *buffer, uint32_t len) {
    uint8_t dummy = 0, status;

    if (caddr >= W25N1XX_PAGE_TOL_SIZE) {
        return W25N1XX_EPARAMS;
    }

    if ((caddr + len) > W25N1XX_PAGE_TOL_SIZE) {
        len = W25N1XX_PAGE_TOL_SIZE - caddr;
    }

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_PAGE_READ);
    w25n1xx_write_read_byte(dummy);
    w25n1xx_write_read_byte(NAND_ROW_ADDR_H(paddr));
    w25n1xx_write_read_byte(NAND_ROW_ADDR_L(paddr));
    NAND_FLASH_CS_H();

    w25n1xx_wait_ready();
    // status = w25n1xx_ecc_check();

    // if ((status != W25N1XX_OK) && (status != W25N1XX_ECORR)) {
    //     return W25N1XX_ERR;
    // }

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_READ_DATA);
    w25n1xx_write_read_byte(NAND_COL_ADDR_H(caddr));
    w25n1xx_write_read_byte(NAND_COL_ADDR_L(caddr));
    w25n1xx_write_read_byte(dummy);
    #if defined(W25N1XX_USE_DMA)
    w25n1xx_read_buffer(buffer, len);
    #else 
    for (int i = 0; i < len; i++) {
        buffer[i] = w25n1xx_write_read_byte(0xFF);
    }
    #endif
    NAND_FLASH_CS_H();

    return W25N1XX_OK;
}


w25n1xx_status_t w25n1xx_page_write(uint32_t paddr, uint16_t caddr, uint8_t *buffer, uint32_t len) {
    uint8_t dummy = 0, status;

    if (caddr >= W25N1XX_PAGE_TOL_SIZE) {
        return W25N1XX_EPARAMS;
    }

    if ((caddr + len) > W25N1XX_PAGE_TOL_SIZE) {
        len = W25N1XX_PAGE_TOL_SIZE - caddr;
    }

    // if (caddr == 2048) {
    //     w25n1xx_page_read(paddr, caddr, w25n1xx_buf, len);
    //     LOG("pre-read:\r\n");
    //     LOG_HEX(w25n1xx_buf, len);
    // }

    w25n1xx_write_enable(true);

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_LOAD_PROGRAM_DATA);
    w25n1xx_write_read_byte(NAND_COL_ADDR_H(caddr));
    w25n1xx_write_read_byte(NAND_COL_ADDR_L(caddr));
    #if defined(W25N1XX_USE_DMA)
    w25n1xx_write_buffer(buffer, len);
    #else 
    for (int i = 0; i < len; i++) {
        w25n1xx_write_read_byte(buffer[i]);
    }
    #endif
    NAND_FLASH_CS_H();

    w25n1xx_wait_ready();

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_PROGRAM_EXECUTE);
    w25n1xx_write_read_byte(dummy);
    w25n1xx_write_read_byte(NAND_ROW_ADDR_H(paddr));
    w25n1xx_write_read_byte(NAND_ROW_ADDR_L(paddr));
    NAND_FLASH_CS_H();

    w25n1xx_wait_ready();
    status = w25n1xx_read_status_register(SR3_STATUS);

    w25n1xx_write_enable(false);

    if (status & W25N1XX_SR3_PFAIL) {
        LOG("w25n1xx program failed\r\n");
        return W25N1XX_EPROGRAM;
    }

    #if defined(W25N1XX_USE_READBACK)
    if (w25n1xx_page_read(paddr, caddr, w25n1xx_buf, len) != W25N1XX_OK) {
        LOG("w25n1xx program compare failed\r\n");
        return W25N1XX_EPROGRAM;
    }

    for (int i = 0; i < len; i++) {
        if (buffer[i] != w25n1xx_buf[i]) {
            // LOG("read:\r\n");
            // LOG_HEX(w25n1xx_buf, len);
            // LOG("write:\r\n");
            // LOG_HEX(buffer, len);
            LOG("w25n1xx program compare failed,idx:%u, %02x|%02x, p:0x%08X, c:%u, len:%u\r\n", i, buffer[i], w25n1xx_buf[i], paddr, caddr, len);
            return W25N1XX_EPROGRAM;
        }
    }
    #endif
    // LOG("w25n1xx program success, p:%04X, c:%u, len:%u\r\n", paddr, caddr, len);
    return W25N1XX_OK;
}

w25n1xx_status_t w25n1xx_chip_erase(void) {
    for (int i = 0; i < W25N1XX_BLOCK_NUMBER; i++) {
        if (w25n1xx_block_erase(W25N1XX_ADDR(i, 0)) != W25N1XX_OK) {
            LOG("w25n1xx block erase failed, %u\r\n", i);
        }
    }
    LOG("w25n1xx chip erase success\r\n");
    return W25N1XX_OK;
}