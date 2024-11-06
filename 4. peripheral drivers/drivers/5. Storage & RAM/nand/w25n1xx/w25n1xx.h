#ifndef __W25N1XX_H__
#define __W25N1XX_H__


typedef enum {
    W25N1XX_OK,
    W25N1XX_ERR,
    W25N1XX_EERASE,
    W25N1XX_ETIMEOUT,
    W25N1XX_ECORR,
    W25N1XX_EUNCORR,
    W25N1XX_EPARAMS,
    W25N1XX_EPROGRAM,
} w25n1xx_status_t;



#define W25N1XX_CMD_JEDEC_ID        0x9F
#define W25N1XX_CMD_WRITE_ENABLE    0x06
#define W25N1XX_CMD_WRITE_DISABLE   0x04
#define W25N1XX_CMD_READ_SR         0x05
#define W25N1XX_CMD_WRITE_SR        0x01

#define W25N1XX_CMD_BBM             0xA1
#define W25N1XX_CMD_READ_BBM_LUT    0xA5
#define W25N1XX_CMD_BLOCK_ERASE     0xD8
#define W25N1XX_CMD_RESET           0xFF

#define W25N1XX_CMD_PAGE_READ       0x13
#define W25N1XX_CMD_READ_DATA       0x03

#define W25N1XX_CMD_PROGRAM_EXECUTE     0x10
#define W25N1XX_CMD_LOAD_PROGRAM_DATA   0x84


#define W25N1XX_SR1_SRP1            (1 << 0)
#define W25N1XX_SR1_WPE             (1 << 1)
#define W25N1XX_SR1_SRP0            (1 << 7)

#define W25N1XX_SR2_BUF             (1 << 3)
#define W25N1XX_SR2_ECC_ENABLE      (1 << 4)
#define W25N1XX_SR2_SR1_LOC         (1 << 5)

#define W25N1XX_SR3_BUSY            (1 << 0)
#define W25N1XX_SR3_WEL             (1 << 1)
#define W25N1XX_SR3_EFAIL           (1 << 2)
#define W25N1XX_SR3_PFAIL           (1 << 3)
#define W25N1XX_SR3_ECC_ERR         (3 << 4)
#define W25N1XX_SR3_LUT_FULL        (1 << 6)


#define SR1_PROTECTION  	         0xA0
#define SR2_CONFIGURATION            0xB0
#define SR3_STATUS		             0xC0

#define W25N1XX_ECC_OK               0x00
#define W25N1XX_ECC_1BIT_CORRECTION  0x10
#define W25N1XX_ECC_2BIT_ERR         0x20
#define W25N1XX_ECC_2BIT_ERR_MULTI   0x30


#define W25N1XX_PAGE_SHIFT           11     /* Page Size: 2048 + 112 byte */
#define W25N1XX_PAGE_MEM_SIZE        2048
#define W25N1XX_PAGE_TOL_SIZE        (2048 + 64)


void     w25n1xx_init(void);
uint32_t w25n1xx_read_jedec_id(void);
uint8_t w25n1xx_read_status_register(uint8_t sr_addr);
void w25n1xx_write_status_register(uint8_t sr_addr, uint8_t reg_val);
w25n1xx_status_t w25n1xx_block_erase(uint16_t paddr);
w25n1xx_status_t w25n1xx_page_read(uint32_t paddr, uint16_t caddr, uint8_t *buffer, uint32_t len);
w25n1xx_status_t w25n1xx_page_write(uint32_t paddr, uint16_t caddr, uint8_t *buffer, uint32_t len);

#endif