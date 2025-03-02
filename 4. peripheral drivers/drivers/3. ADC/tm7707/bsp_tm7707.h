#ifndef __BSP_TM7707_H__
#define __BSP_TM7707_H__

#include "stdint.h"

#define TM7707_MCLK_TIMER              TIM1
#define TM7707_MCLK_TIMER_CHANNEL      LL_TIM_CHANNEL_CH2
#define TM7707_MCLK_PORT               GPIOA
#define TM7707_MCLK_PIN                LL_GPIO_PIN_9

#define TM7707_FILTER_VAL              (uint16_t)(0x0C80)   /* 60hz */


#define TM7707_RESET_GPIO              GPIOG
#define TM7707_RESET_PIN               LL_GPIO_PIN_6

/* CS片选 */    

#define PRESSURE_CS_GPIO               GPIOC
#define PRESSURE_CS_PIN                LL_GPIO_PIN_12

#define TEMP_CS_GPIO                   GPIOC
#define TEMP_CS_PIN                    LL_GPIO_PIN_7

#define WEIGHT_CS_GPIO                 GPIOD
#define WEIGHT_CS_PIN                  LL_GPIO_PIN_13

/* DRDY */    
#define PRESSURE_DRDY_GPIO             GPIOC
#define PRESSURE_DRDY_PIN              LL_GPIO_PIN_5

#define TEMP_DRDY_GPIO                 GPIOB
#define TEMP_DRDY_PIN                  LL_GPIO_PIN_7

#define WEIGHT_DRDY_GPIO               GPIOB
#define WEIGHT_DRDY_PIN                LL_GPIO_PIN_8


typedef enum {
    TM7707_PRESSURE,
    TM7707_TEMP,
    TM7707_WEIGHT,
    TM7707_CNT,

} tm7707_id_t;


typedef enum {
    /* 寄存器选择  RS2 RS1 RS0  */
    REG_COMM        = 0x00,    /* 通信寄存器 */
    REG_SETUP       = 0x10,    /* 设置寄存器 */
    REG_FILTERH     = 0x20,    /* 滤波器高寄存器 */
    REG_DATA        = 0x30,    /* 数据寄存器 */
    REG_TEST        = 0x40,    /* 测试寄存器 */
    REG_FILTERL     = 0x50,    /* 滤波器低寄存器 */
    REG_ZERO_CH1    = 0x60,    /* 零刻度校准寄存器 */
    REG_FULL_CH1    = 0x70,    /* 满量程校准寄存器 */
    REG_ZERO_CH2    = 0x61,    /* 零刻度校准寄存器 */
    REG_FULL_CH2    = 0x71,    /* 满量程校准寄存器 */
} tm7707_reg_t;

typedef enum {
    /* 读写操作 */
    WRITE           = 0x00,    /* 写操作 */
    READ            = 0x08,    /* 读操作 */
} tm7707_opr_t;

typedef enum {
    /* 通道 */
    CH_1            = 0x00,    /* AIN1+  AIN1- */
    CH_2            = 0x01,    /* AIN2+  AIN2- */
    CH_3            = 0x02,    /* AIN1-  AIN1- */
    CH_4            = 0x03     /* AIN1-  AIN2- */
} tm7707_ch_t;



/* 设置寄存器bit定义 */
typedef enum {
    MD_NORMAL       = (0 << 6),    /* 正常模式 */
    MD_CAL_SELF     = (1 << 6),    /* 自校准模式 */
    MD_CAL_ZERO     = (2 << 6),    /* 校准0刻度模式 */
    MD_CAL_FULL     = (3 << 6),    /* 校准满刻度模式 */

    GAIN_1          = (0 << 3),    /* 增益 */
    GAIN_2          = (1 << 3),    /* 增益 */
    GAIN_4          = (2 << 3),    /* 增益 */
    GAIN_8          = (3 << 3),    /* 增益 */
    GAIN_16         = (4 << 3),    /* 增益 */
    GAIN_32         = (5 << 3),    /* 增益 */
    GAIN_64         = (6 << 3),    /* 增益 */
    GAIN_128        = (7 << 3),    /* 增益 */

    BIPOLAR         = (0 << 2),    /* 双极性输入 */
    UNIPOLAR        = (1 << 2),    /* 单极性输入 */

    BUF_NO          = (0 << 1),    /* 输入无缓冲（内部缓冲器不启用) */
    BUF_EN          = (1 << 1),    /* 输入有缓冲 (启用内部缓冲器) */

    FSYNC_0         = 0,
    FSYNC_1         = 1            /* 不启用 */
} tm7707_setup_t;

typedef enum {
    OSCDIS_0        = 0x00,        /* 时钟输出使能 （当外接晶振时，必须使能才能振荡） */
    OSCDIS_1        = 0x10,        /* 时钟禁止 （当外部提供时钟时，设置该位可以禁止MCK_OUT引脚输出时钟以省电 */

    BWT_0           = 0x00,        /* Fclkin = 2.4576Mhz, GAIN = 8~128时，必须置1 */
    BWT_1           = 0x20,

} tm7707_filterh_t;

/* RESET 使用扩展IO */    
#define RESET_1()             LL_GPIO_SetOutputPin(TM7707_RESET_GPIO, TM7707_RESET_PIN)
#define RESET_0()             LL_GPIO_ResetOutputPin(TM7707_RESET_GPIO, TM7707_RESET_PIN)




void bsp_tm7707_init(void);
uint32_t tm7707_read_adc(tm7707_id_t chip_id, tm7707_ch_t ch);
uint32_t tm7707_read_adc_ex(tm7707_id_t chip_id, tm7707_ch_t ch);
int tm7707_wait_okl(tm7707_id_t chip_id, uint32_t timeout);
int tm7707_comm_sync(tm7707_id_t chip_id);



#endif