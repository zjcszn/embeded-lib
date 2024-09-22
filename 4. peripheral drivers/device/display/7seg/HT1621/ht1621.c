#include "ht1621.h"
#include "main.h"
#include "stdint.h"
#include "perf_counter.h"

typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin;
} ht1621_iomap_t;

typedef enum {
    HT1621_CS   = 0,
    HT1621_WR   = 1,
    HT1621_RD   = 2,
    HT1621_DATA = 3,
} ht1621_io_t;

typedef enum {
    HT1621_WR_MODE_MSB,
    HT1621_WR_MODE_LSB,
} ht1621_wr_mode_t;

static const ht1621_iomap_t ht1621_iomap[] = {
    [HT1621_CS] = {HT1621_CS_PORT, HT1621_CS_PIN},
    [HT1621_WR] = {HT1621_WR_PORT, HT1621_WR_PIN},
    [HT1621_RD] = {HT1621_RD_PORT, HT1621_RD_PIN},
    [HT1621_DATA] = {HT1621_DATA_PORT, HT1621_DATA_PIN},
};

static inline void ht1621_io_h(ht1621_io_t io) {
    LL_GPIO_SetOutputPin(ht1621_iomap[io].port, ht1621_iomap[io].pin);
}

static inline void ht1621_io_l(ht1621_io_t io) {
    LL_GPIO_ResetOutputPin(ht1621_iomap[io].port, ht1621_iomap[io].pin);
}

static inline void ht1621_data_in(void) {
    LL_GPIO_SetPinPull(ht1621_iomap[HT1621_DATA].port, ht1621_iomap[HT1621_DATA].pin, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinMode(ht1621_iomap[HT1621_DATA].port, ht1621_iomap[HT1621_DATA].pin, LL_GPIO_MODE_INPUT);
}

static inline void ht1621_data_out(void) {
    LL_GPIO_SetPinPull(ht1621_iomap[HT1621_DATA].port, ht1621_iomap[HT1621_DATA].pin, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinMode(ht1621_iomap[HT1621_DATA].port, ht1621_iomap[HT1621_DATA].pin, LL_GPIO_MODE_OUTPUT);
}

#define HT1621_CS_H              ht1621_io_h(HT1621_CS)
#define HT1621_CS_L              ht1621_io_l(HT1621_CS)
#define HT1621_WR_H              ht1621_io_h(HT1621_WR)
#define HT1621_WR_L              ht1621_io_l(HT1621_WR)
#define HT1621_RD_H              ht1621_io_h(HT1621_RD)
#define HT1621_RD_L              ht1621_io_l(HT1621_RD)

#define HT1621_DATA_SET(v)                  \
    do {                                    \
        if (v) {                            \
            ht1621_io_h(HT1621_DATA);       \
        } else {                            \
            ht1621_io_l(HT1621_DATA);       \
        }                                   \
    } while (0U)

#define HT1621_DATA_READ()       (uint8_t)(LL_GPIO_IsInputPinSet(ht1621_iomap[HT1621_DATA].port, ht1621_iomap[HT1621_DATA].pin))

#define HT1621_DelayUS(us)       delay_us(us) 
#define HT1621_DelayMS(ms)       delay_ms(ms) 

void ht1621_send_cmd(uint8_t cmd);

void ht1621_serial_com_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;

    for (int i = 0; i < ARRAY_SIZE(ht1621_iomap); i++) {
        GPIO_InitStruct.Pin = ht1621_iomap[i].pin;
        LL_GPIO_Init(ht1621_iomap[i].port, &GPIO_InitStruct);
        LL_GPIO_SetOutputPin(ht1621_iomap[i].port, ht1621_iomap[i].pin);
    }
}

void ht1621_init(void) {
    ht1621_serial_com_init();
    ht1621_send_cmd(HT1621_COMMAND_XTAL_32);
    ht1621_send_cmd(HT1621_COMMAND_BIAS_1_3);
    ht1621_send_cmd(HT1621_COMMAND_SYS_EN);
    ht1621_display_all_seg();
    ht1621_send_cmd(HT1621_COMMAND_LCD_ON);
    HT1621_DelayMS(2000);
    ht1621_clear_all_seg();
}

void ht1621_display_all_seg(void) {
    for (int i = 0; i < 32; i++) {
        ht1621_write(i, 0xF);
    }
}

void ht1621_clear_all_seg(void) {
    for (int i = 0; i < 32; i++) {
        ht1621_write(i, 0);
    }
}

void ht1621_send_data_msb(uint8_t data, uint8_t bits_num) {
    for (int i = 0; i < bits_num; i++, data <<= 1) {
        HT1621_WR_L;
        HT1621_DATA_SET(data & 0x80);  
        HT1621_DelayUS(tCLK_WR);
        HT1621_WR_H;
        HT1621_DelayUS(tCLK_WR);
    }
}

void ht1621_send_data_lsb(uint8_t data, uint8_t bits_num) {
    for (int i = 0; i < bits_num; i++, data >>= 1) {
        HT1621_WR_L;
        HT1621_DATA_SET(data & 0x01);  
        HT1621_DelayUS(tCLK_WR);
        HT1621_WR_H;
        HT1621_DelayUS(tCLK_WR);
    }
}

uint8_t ht1621_read_data(void) {
    uint8_t data = 0;

    ht1621_data_in();
    for (int i = 0; i < HT1621_DATA_LEN; i++) {
        HT1621_RD_L;
        HT1621_DelayUS(tCLK_RD);
        HT1621_RD_H;
        data |= (HT1621_DATA_READ() << i);
        HT1621_DelayUS(tCLK_RD);
    }
    ht1621_data_out();
    return data;
}

void ht1621_send_cmd(uint8_t cmd) {
    HT1621_CS_L;
    ht1621_send_data_msb(HT1621_ID_COMMAND, 3);
    ht1621_send_data_msb(cmd, 8);
    ht1621_send_data_msb(0, 1);
    HT1621_CS_H;
}

/**
 * @brief HT1621写数据
 * 
 * @param addr RAM地(SEG)
 * @param data 待写入数据(COM)
 */
void ht1621_write(uint8_t addr, uint8_t data) {
    HT1621_CS_L;
    ht1621_send_data_msb(HT1621_ID_WRITE, 3);
    ht1621_send_data_msb(addr << HT1621_ADDR_SHIFT, HT1621_ADDR_LEN);   /* 发送地址， 高位在前 */
    ht1621_send_data_lsb(data, 4);  /* 发送数据，低位在前 */
    HT1621_CS_H;
}


/**
 * @brief HT1621读数据
 * 
 * @param addr RAM地(SEG)
 * @param data 读出数据缓冲区(COM)
 */
void ht1621_read(uint8_t addr, uint8_t *data) {
    HT1621_CS_L;
    ht1621_send_data_msb(HT1621_ID_READ, 3);
    ht1621_send_data_msb(addr << HT1621_ADDR_SHIFT, HT1621_ADDR_LEN);
    *data = ht1621_read_data();
    HT1621_CS_H;
}


/**
 * @brief HT1621改写数据
 * 
 * @param addr RAM地(SEG)
 * @param data 待改写的数据内容
 * @param mask 待改写的数据掩码
 */
void ht1621_read_modify_write(uint8_t addr, uint8_t data, uint8_t mask) {
    uint8_t tmp;

    HT1621_CS_L;
    ht1621_send_data_msb(HT1621_ID_WRITE, 3);
    ht1621_send_data_msb(addr << HT1621_ADDR_SHIFT, HT1621_ADDR_LEN);

    tmp = ht1621_read_data();
    tmp &= ~mask;
    tmp |= (data & mask);

    ht1621_send_data_lsb(tmp, 4);
    HT1621_CS_H;
}





