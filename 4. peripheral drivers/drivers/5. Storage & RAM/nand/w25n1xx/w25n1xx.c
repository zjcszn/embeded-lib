#include "main.h"
#include "w25n1xx.h"
#include "bsp_spi1_nandflash.h"
#include "stdbool.h"

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

static uint8_t w25n1xx_buf[W25N1XX_PAGE_TOL_SIZE];


#define NAND_ROW_ADDR_H(paddr)      (uint8_t)(((paddr) >> (W25N1XX_PAGE_SHIFT + 8)) & 0xFF)
#define NAND_ROW_ADDR_L(paddr)      (uint8_t)(((paddr) >> (W25N1XX_PAGE_SHIFT)) & 0xFF)

#define NAND_COL_ADDR_H(caddr)      (uint8_t)(((caddr) >> 8) & 0xFF)
#define NAND_COL_ADDR_L(caddr)      (uint8_t)(((caddr) & 0xFF))

void w25n1xx_init(void) {
    bsp_spi_nandflash_init();
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
}

void w25n1xx_write_enable(bool enable) {
    uint8_t cmd;
    cmd = enable ? W25N1XX_CMD_WRITE_ENABLE : W25N1XX_CMD_WRITE_DISABLE;

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(cmd);
    NAND_FLASH_CS_H();
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

bool w25n1xx_ready(void) {
    uint8_t sr3;
    sr3 = w25n1xx_read_status_register(SR3_STATUS);
    LOG("sr3: %02X\r\n", sr3);
	return !(sr3 & W25N1XX_SR3_BUSY);
} 

w25n1xx_status_t w25n1xx_block_erase(uint16_t paddr) {
    uint8_t dummy = 0, status;

    /* TODO: check bad block */

    w25n1xx_write_enable(true);

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_BLOCK_ERASE);
    w25n1xx_write_read_byte(dummy);
    w25n1xx_write_read_byte(NAND_ROW_ADDR_H(paddr));
    w25n1xx_write_read_byte(NAND_ROW_ADDR_L(paddr));
    NAND_FLASH_CS_H();

    w25n1xx_write_enable(false);

    while (!w25n1xx_ready());
    status = w25n1xx_read_status_register(SR3_STATUS);

    if (status & W25N1XX_SR3_EFAIL) {
        LOG("w25n1xx erase failed\r\n");
        return W25N1XX_EERASE;
    }

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

    if (caddr >= W25N1XX_PAGE_MEM_SIZE) {
        return W25N1XX_EPARAMS;
    }

    if ((caddr + len) > W25N1XX_PAGE_MEM_SIZE) {
        len = W25N1XX_PAGE_MEM_SIZE - caddr;
    }

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_PAGE_READ);
    w25n1xx_write_read_byte(dummy);
    w25n1xx_write_read_byte(NAND_ROW_ADDR_H(paddr));
    w25n1xx_write_read_byte(NAND_ROW_ADDR_L(paddr));
    NAND_FLASH_CS_H();

    while(!w25n1xx_ready());
    status = w25n1xx_ecc_check();

    if ((status != W25N1XX_OK) && (status != W25N1XX_ECORR)) {
        return W25N1XX_ERR;
    }

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_READ_DATA);
    w25n1xx_write_read_byte(NAND_COL_ADDR_H(caddr));
    w25n1xx_write_read_byte(NAND_COL_ADDR_L(caddr));
    w25n1xx_write_read_byte(dummy);
    #if 1
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

    if (caddr >= W25N1XX_PAGE_MEM_SIZE) {
        return W25N1XX_EPARAMS;
    }

    if ((caddr + len) > W25N1XX_PAGE_MEM_SIZE) {
        len = W25N1XX_PAGE_MEM_SIZE - caddr;
    }

    w25n1xx_write_enable(true);

    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_LOAD_PROGRAM_DATA);
    w25n1xx_write_read_byte(NAND_COL_ADDR_H(caddr));
    w25n1xx_write_read_byte(NAND_COL_ADDR_L(caddr));
    #if 1
    w25n1xx_write_buffer(buffer, len);
    #else 
    for (int i = 0; i < len; i++) {
        w25n1xx_write_read_byte(buffer[i]);
    }
    #endif
    NAND_FLASH_CS_H();

    while(!w25n1xx_ready());


    NAND_FLASH_CS_L();
    w25n1xx_write_read_byte(W25N1XX_CMD_PROGRAM_EXECUTE);
    w25n1xx_write_read_byte(dummy);
    w25n1xx_write_read_byte(NAND_ROW_ADDR_H(paddr));
    w25n1xx_write_read_byte(NAND_ROW_ADDR_L(paddr));
    NAND_FLASH_CS_H();

    while(!w25n1xx_ready());
    status = w25n1xx_read_status_register(SR3_STATUS);

    w25n1xx_write_enable(false);

    if (status & W25N1XX_SR3_PFAIL) {
        LOG("w25n1xx program failed\r\n");
        return W25N1XX_EPROGRAM;
    }

    if (w25n1xx_page_read(paddr, caddr, w25n1xx_buf, len) != W25N1XX_OK) {
        LOG("w25n1xx program compare failed\r\n");
        return W25N1XX_EPROGRAM;
    }

    for (int i = 0; i < len; i++) {
        if (buffer[i] != w25n1xx_buf[i]) {
            LOG("w25n1xx program compare failed\r\n");
            return W25N1XX_EPROGRAM;
        }
    }

    return W25N1XX_OK;
}
