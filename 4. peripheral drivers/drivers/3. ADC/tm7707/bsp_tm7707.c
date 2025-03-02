
#define LOG_LVL ELOG_LVL_DEBUG
#define LOG_TAG "TM7707"

#include "bsp_tm7707.h"
#include "bsp_spi_tm7707.h"

#include "main.h"
#include "elog.h"


typedef struct {
    GPIO_TypeDef *port;
    uint32_t pin;
} tm7707_io_t;


#define CS_1(port, pin)           ((GPIO_TypeDef *)(port))->BSRR = (pin)
#define CS_0(port, pin)           ((GPIO_TypeDef *)(port))->BSRR = ((pin) << 16)
#define DRDY_IS_LOW(port, pin)    (((port)->IDR & (pin)) == 0)

#define TM7707_DelayMS(x)           vTaskDelay(pdMS_TO_TICKS(x))
#define TM7707_DelayUS(x)           delay_us(x)


/* TM7707 CS IOMAP */
static const tm7707_io_t tm7707_cs[TM7707_CNT] = {
    {PRESSURE_CS_GPIO, PRESSURE_CS_PIN},
    {TEMP_CS_GPIO, TEMP_CS_PIN},
    {WEIGHT_CS_GPIO, WEIGHT_CS_PIN},
};

/* TM7707 OKL IOMAP */
static const tm7707_io_t tm7707_okl[TM7707_CNT] = {
    {PRESSURE_DRDY_GPIO, PRESSURE_DRDY_PIN},
    {TEMP_DRDY_GPIO, TEMP_DRDY_PIN},
    {WEIGHT_DRDY_GPIO, WEIGHT_DRDY_PIN},
};

/* TM7707 EXTI LINE*/
static const uint32_t tm7707_exti_line[TM7707_CNT] = {
    LL_EXTI_LINE_5,
    LL_EXTI_LINE_7,
    LL_EXTI_LINE_8,
};

static uint8_t tm7707_ok[TM7707_CNT];

/* TM7707通道配置, 增益 极性 缓冲 */
static uint8_t tm7707_chcfg[TM7707_CNT] = {
    (GAIN_64  | BIPOLAR  | BUF_NO),   /* 压力 */
    (GAIN_16  | UNIPOLAR | BUF_NO),   /* 温度 */
    (GAIN_128 | BIPOLAR  | BUF_NO),   /* 称重 */
};

/* TM7707 BWT OSCDIS配置 */
static uint8_t tm7707_setup[TM7707_CNT] = {
    (BWT_1 | OSCDIS_1),
    (BWT_1 | OSCDIS_1),
    (BWT_1 | OSCDIS_1),
};

static uint16_t tm7707_filter[TM7707_CNT] = {
    0x0140, /*60HZ*/
    0x0780,
    0x0640, /*12HZ*/
};

static SemaphoreHandle_t tm7707_signal[TM7707_CNT];


uint8_t  tm7707_txbuf[64];
uint8_t  tm7707_rxbuf[64];	/* 必须32字节对齐 */


static void tm7707_reset(void);
static void tm7707_filter_config(tm7707_id_t chip_id);
static void tm7707_write_register(tm7707_id_t chip_id, uint8_t reg_id, uint32_t reg_val);
static uint32_t tm7707_read_register(tm7707_id_t chip_id, uint8_t reg_id);
static void tm7707_calibration(tm7707_id_t chip_id, tm7707_ch_t ch);
static void tm7707_sync(tm7707_id_t chip_id);



static void tm7707_chip_select(tm7707_id_t chip_id, uint8_t val) {

    if (val == 0) { 
        CS_0(tm7707_cs[chip_id].port, tm7707_cs[chip_id].pin);
    } else {        
        CS_1(tm7707_cs[chip_id].port, tm7707_cs[chip_id].pin);    
    }

    TM7707_DelayUS(10);
}


static void tm7707_write_1byte(tm7707_id_t chip_id, uint8_t dat) {
    tm7707_chip_select(chip_id, 0);

    tm7707_txbuf[0] = (dat);
    bsp_spi_tm7707_write_then_stop(tm7707_txbuf, 1);

    tm7707_chip_select(chip_id, 1);
}

static void tm7707_write_3byte(tm7707_id_t chip_id, uint32_t dat) {
    tm7707_chip_select(chip_id, 0);

    tm7707_txbuf[0] = ((dat >> 16) & 0xFF);
    tm7707_txbuf[1] = ((dat >> 8) & 0xFF);
    tm7707_txbuf[2] = ( dat);
    bsp_spi_tm7707_write_then_stop(tm7707_txbuf, 3);

    tm7707_chip_select(chip_id, 1);
}

static void tm7707_reset(void) {
    RESET_1();
    TM7707_DelayMS(1);
    RESET_0();
    TM7707_DelayMS(10);
    RESET_1();
    TM7707_DelayMS(10);
}


void bsp_tm7707_init(void) {
    uint8_t reg_val;

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;

    /* 1.初始化CS */
    for (int i = 0; i < TM7707_CNT; i++) {
        GPIO_InitStruct.Pin = tm7707_cs[i].pin;
        LL_GPIO_Init(tm7707_cs[i].port, &GPIO_InitStruct);    
        CS_1(tm7707_cs[i].port, tm7707_cs[i].pin);
    }

    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;

    /* 2.初始化OKL */
    for (int i = 0; i < TM7707_CNT; i++) {
        GPIO_InitStruct.Pin = tm7707_okl[i].pin;
        LL_GPIO_Init(tm7707_okl[i].port, &GPIO_InitStruct);   
    }

    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_5;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE5);
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_5);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_7;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE7);
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_7);

    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_8;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE8);
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_8);

    NVIC_SetPriority(EXTI9_5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    /* 初始化RESET */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Pin = TM7707_RESET_PIN;
    LL_GPIO_Init(TM7707_RESET_GPIO, &GPIO_InitStruct);

    /* 初始化信号量 */
    for (int i = 0; i < TM7707_CNT; i++) {
        tm7707_signal[i] = xSemaphoreCreateBinary();
    }

    /* 3.初始化SPI */
    bsp_spi_tm7707_init();

    /* 4.使能TN7707时钟 */      
    TM7707_DelayMS(10);

    /* 5.硬件复位 */
    tm7707_reset();

    TM7707_DelayMS(5);

    /* 6.寄存器配置 */
    for (int i = 0; i < TM7707_CNT; i++) {

        tm7707_sync(i);
        TM7707_DelayMS(10);

        /* 配置滤波器 */
        tm7707_filter_config(i);

        /* 通道自校准 */
        tm7707_calibration(i, CH_1);
        
        TM7707_DelayMS(100);
        tm7707_calibration(i, CH_2);
    }

    log_d("TM7707 Init Complete");
}




static void tm7707_filter_config(tm7707_id_t chip_id) {
    uint8_t reg_val, filter_l, filter_h;

    filter_l = (uint8_t)(tm7707_filter[chip_id] & 0xFF);
    filter_h = (uint8_t)(tm7707_setup[chip_id]) | (uint8_t)((tm7707_filter[chip_id] >> 8) & 0x0F);

    tm7707_write_register(chip_id, REG_FILTERL, filter_l);
    tm7707_write_register(chip_id, REG_FILTERH, filter_h);

    reg_val = tm7707_read_register(chip_id, REG_FILTERH);
    log_d("tm7707 init %s", reg_val == filter_h ? "OK" : "failed");
}


static void tm7707_sync(tm7707_id_t chip_id) {
    /* TM7707串行接口失步后将其复位。复位后要延时500us再访问 */
    tm7707_chip_select(chip_id, 0);
    
    tm7707_txbuf[0] = (0xFF);
    tm7707_txbuf[1] = (0xFF);
    tm7707_txbuf[2] = (0xFF);
    tm7707_txbuf[3] = (0xFF);
    bsp_spi_tm7707_write_then_stop(tm7707_txbuf, 4);
    
    tm7707_chip_select(chip_id, 1);
}

int tm7707_comm_sync(tm7707_id_t chip_id) {
    uint8_t filter_h;

    filter_h = (uint8_t)(tm7707_setup[chip_id]) | (uint8_t)((tm7707_filter[chip_id] >> 8) & 0x0F);

    if (tm7707_read_register(chip_id, REG_FILTERH) == filter_h) {
        return 0;
    }

    log_e("tm7707 comm error, chip_id:%d", chip_id);
    
    tm7707_sync(chip_id);

    TM7707_DelayMS(1);

    tm7707_filter_config(chip_id);

    /* 通道自校准 */
    tm7707_calibration(chip_id, CH_1);    
    TM7707_DelayMS(100);
    tm7707_calibration(chip_id, CH_2);

    return 1;
}


int tm7707_wait_okl(tm7707_id_t chip_id, uint32_t timeout) {
    
    if (DRDY_IS_LOW(tm7707_okl[chip_id].port, tm7707_okl[chip_id].pin)) {
        return 0;
    }

    if (timeout == 0) {
        return 1;
    }

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5 | LL_EXTI_LINE_7 | LL_EXTI_LINE_8);
    LL_EXTI_EnableIT_0_31(tm7707_exti_line[chip_id]);

    if (xSemaphoreTake(tm7707_signal[chip_id], pdMS_TO_TICKS(timeout)) != pdTRUE) {
        LL_EXTI_DisableIT_0_31(tm7707_exti_line[chip_id]);
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5 | LL_EXTI_LINE_7 | LL_EXTI_LINE_8);
        return (xSemaphoreTake(tm7707_signal[chip_id], 0) != pdTRUE);
    }

    return 0;
}


static void tm7707_write_register(tm7707_id_t chip_id, uint8_t reg_id, uint32_t reg_val) {
    uint8_t bits;

    switch (reg_id)
    {
        case REG_COMM:        /* 通信寄存器 */
        case REG_SETUP:       /* 设置寄存器 8bit */
        case REG_TEST:        /* 测试寄存器 8bit */
        case REG_FILTERH:
        case REG_FILTERL:
            bits = 8;
            break;

        case REG_ZERO_CH1:    /* CH1 偏移寄存器 24bit */
        case REG_FULL_CH1:    /* CH1 满量程寄存器 24bit */
        case REG_ZERO_CH2:    /* CH2 偏移寄存器 24bit */
        case REG_FULL_CH2:    /* CH2 满量程寄存器 24bit*/
            bits = 24;
            break;

        case REG_DATA:        /* 数据寄存器 16bit */
        default:
            return;
    }

    tm7707_write_1byte(chip_id, reg_id | WRITE);    /* 写通信寄存器, 指定下一步是写操作，并指定写哪个寄存器 */

    if (bits == 8) {
        tm7707_write_1byte(chip_id, (uint8_t)reg_val);
    } else {
        tm7707_write_3byte(chip_id, reg_val);
    }
}

static uint32_t tm7707_read_register(tm7707_id_t chip_id, uint8_t reg_id) {
    uint8_t  btr;
    uint32_t reg_val;

    switch (reg_id)
    {
        case REG_COMM:        /* 通信寄存器 */
        case REG_SETUP:       /* 设置寄存器 8bit */
        case REG_FILTERH:     /* 时钟寄存器 8bit */
        case REG_FILTERL:
        case REG_TEST:
            btr = 1;
            break;

        case REG_ZERO_CH1:    /* CH1 偏移寄存器 24bit */
        case REG_FULL_CH1:    /* CH1 满量程寄存器 24bit */
        case REG_ZERO_CH2:    /* CH2 偏移寄存器 24bit */
        case REG_FULL_CH2:    /* CH2 满量程寄存器 24bit*/
            btr = 3;
            break;

        case REG_DATA:        /* 数据寄存器 24bit */
        default:
            return 0xFFFFFFFF;
    }

    tm7707_txbuf[0] = (reg_id | READ);    /* 写通信寄存器, 指定下一步是写操作，并指定写哪个寄存器 */

    tm7707_chip_select(chip_id, 0);
    bsp_spi_tm7707_write_then_read(tm7707_txbuf, 1, tm7707_rxbuf, btr);    
    tm7707_chip_select(chip_id, 1);

    if (btr == 1) {
        reg_val = (uint32_t)tm7707_rxbuf[0];
    } else {
        reg_val = ((uint32_t)tm7707_rxbuf[0] << 16) +  ((uint32_t)tm7707_rxbuf[1] << 8) + (uint32_t)tm7707_rxbuf[2];
    }

    return reg_val;
}


static void tm7707_calibration(tm7707_id_t chip_id, tm7707_ch_t ch) {

    TM7707_DelayMS(100);
    tm7707_write_1byte(chip_id, REG_SETUP | WRITE | (ch & 0x03));    /* 写通信寄存器，下一步是写设置寄存器 */
    tm7707_write_1byte(chip_id, MD_CAL_SELF | tm7707_chcfg[chip_id] | FSYNC_0);/* 启动自校准 */
    TM7707_DelayMS(10);
    tm7707_wait_okl(chip_id, 5000);    /* 等待内部操作完成 --- 时间较长，约180ms */

}

uint32_t tm7707_read_adc(tm7707_id_t chip_id, tm7707_ch_t ch) {
    uint32_t ad = 0;
    
    tm7707_chip_select(chip_id, 0);

    tm7707_txbuf[0] = REG_DATA | READ | (ch & 0x03);
    bsp_spi_tm7707_write_then_read(tm7707_txbuf, 1, tm7707_rxbuf, 3);
    tm7707_chip_select(chip_id, 1);

    ad = ((uint32_t)(tm7707_rxbuf[0]) << 16) | ((uint32_t)(tm7707_rxbuf[1]) << 8) | (uint32_t)tm7707_rxbuf[2];

    return ad;
}

uint32_t tm7707_read_adc_ex(tm7707_id_t chip_id, tm7707_ch_t ch) {
    uint32_t ad = 0;
    
    tm7707_wait_okl(chip_id, 1000);        /* 等待DRDY口线为0 */
    ad = tm7707_read_adc(chip_id, ch);

    return ad;
}



void EXTI9_5_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (LL_EXTI_IsEnabledIT_0_31(LL_EXTI_LINE_5) && LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_5)) {
        LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_5);
        xSemaphoreGiveFromISR(tm7707_signal[0], &xHigherPriorityTaskWoken);
    }

    if (LL_EXTI_IsEnabledIT_0_31(LL_EXTI_LINE_7) && LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_7)) {
        LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_7);
        xSemaphoreGiveFromISR(tm7707_signal[1], &xHigherPriorityTaskWoken);
    }

    if (LL_EXTI_IsEnabledIT_0_31(LL_EXTI_LINE_8) && LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_8)) {
        LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_8);
        xSemaphoreGiveFromISR(tm7707_signal[2], &xHigherPriorityTaskWoken);
    }

    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5 | LL_EXTI_LINE_7 | LL_EXTI_LINE_8);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

