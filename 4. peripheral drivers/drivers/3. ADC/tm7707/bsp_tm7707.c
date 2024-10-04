
#define LOG_LVL ELOG_LVL_INFO
#define LOG_TAG "TM7707"

#include "bsp_tm7707.h"

#include "main.h"
#include "bsp_spi_master_soft.h"
#include "elog.h"



/* 通道1和通道2的增益,输入缓冲，极性 */
#define __CH1_GAIN_BIPOLAR_BUF    (GAIN_1 | UNIPOLAR | BUF_EN)
#define __CH2_GAIN_BIPOLAR_BUF    (GAIN_32 | BIPOLAR | BUF_EN)

#define TM7707_DELAY_MS(x)        vTaskDelay(pdMS_TO_TICKS(x))
#define TM7707_DELAY_US(x)        delay_us(x)

/*      
    TM7707模块   STM32-V7开发板
      SCK   ------  PC4
      DOUT  ------  PC8
      DIN   ------  PC7
      
      CS    ------  PC12
      DRDY  ------  PC5
      RST   ------  (复位 RESET)

*/

/* SPI总线的SCK、MOSI、MISO 在 bsp_spi_bus.c中配置  */

/* CS片选 */    

#define CS_CLK_ENABLE()       LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC)
#define CS_GPIO               GPIOC
#define CS_PIN                LL_GPIO_PIN_12
#define CS_1()                CS_GPIO->BSRR = CS_PIN
#define CS_0()                CS_GPIO->BSRR = (CS_PIN << 16)

/* DRDY */    
#define DRDY_CLK_ENABLE()     LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC)
#define DRDY_GPIO             GPIOC
#define DRDY_PIN              LL_GPIO_PIN_5
#define DRDY_IS_LOW()         ((DRDY_GPIO->IDR & DRDY_PIN) == 0)

/* RESET 使用扩展IO */    
#define RESET_1()             (void)0U
#define RESET_0()             (void)0U

/* 通信寄存器bit定义 */
enum
{
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

    /* 读写操作 */
    WRITE            = 0x00,    /* 写操作 */
    READ             = 0x08,    /* 读操作 */

    /* 通道 */
    CH_1             = 0x00,    /* AIN1+  AIN1- */
    CH_2             = 0x01,    /* AIN2+  AIN2- */
    CH_3             = 0x02,    /* AIN1-  AIN1- */
    CH_4             = 0x03     /* AIN1-  AIN2- */
};

/* 设置寄存器bit定义 */
enum
{
    MD_NORMAL         = (0 << 6),    /* 正常模式 */
    MD_CAL_SELF       = (1 << 6),    /* 自校准模式 */
    MD_CAL_ZERO       = (2 << 6),    /* 校准0刻度模式 */
    MD_CAL_FULL       = (3 << 6),    /* 校准满刻度模式 */

    GAIN_1            = (0 << 3),    /* 增益 */
    GAIN_2            = (1 << 3),    /* 增益 */
    GAIN_4            = (2 << 3),    /* 增益 */
    GAIN_8            = (3 << 3),    /* 增益 */
    GAIN_16           = (4 << 3),    /* 增益 */
    GAIN_32           = (5 << 3),    /* 增益 */
    GAIN_64           = (6 << 3),    /* 增益 */
    GAIN_128          = (7 << 3),    /* 增益 */

    /* 无论双极性还是单极性都不改变任何输入信号的状态，它只改变输出数据的代码和转换函数上的校准点 */
    BIPOLAR           = (0 << 2),    /* 双极性输入 */
    UNIPOLAR          = (1 << 2),    /* 单极性输入 */

    BUF_NO            = (0 << 1),    /* 输入无缓冲（内部缓冲器不启用) */
    BUF_EN            = (1 << 1),    /* 输入有缓冲 (启用内部缓冲器) */

    FSYNC_0            = 0,
    FSYNC_1            = 1           /* 不启用 */
};

/* 时钟寄存器bit定义 */
enum
{
    CLKDIS_0    = 0x00,        /* 时钟输出使能 （当外接晶振时，必须使能才能振荡） */
    CLKDIS_1    = 0x10,        /* 时钟禁止 （当外部提供时钟时，设置该位可以禁止MCK_OUT引脚输出时钟以省电 */

    /*
        fCLKIN 为1Mhz或2.4576Mhz，增益为1至4情况下工作时，此位应当为 “0”。
        fCLKIN 为2.4576Mhz，增益为8至128时，BST必须置  “1”
    */
    BST_0       = 0x00,
    BST_1       = 0x01,


    /*
        四十九、电子秤应用中提高TM7707 精度的方法
            当使用主时钟为 2.4576MHz 时，强烈建议将时钟寄存器设为 84H,此时数据输出更新率为10Hz,即每0.1S 输出一个新数据。
            当使用主时钟为 1MHz 时，强烈建议将时钟寄存器设为80H, 此时数据输出更新率为4Hz, 即每0.25S 输出一个新数据
    */
};

uint8_t  g_spiTxBuf[64];
uint8_t  g_spiRxBuf[64];	/* 必须32字节对齐 */
uint32_t g_spiLen;

static void tm7707_sync(void);
static void tm7707_write_byte(uint8_t _data);
static void tm7707_write_3byte(uint32_t _data);

static void tm7707_wait_drdy(void);
static void tm7707_hard_reset(void);


uint8_t g_tm7707_ok = 0;    /* 全局标志，表示TM7707芯片是否连接正常  */

uint16_t g_TM7707_Adc1;
uint16_t g_TM7707_Adc2;

void tm7707_mclk_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_TIM_InitTypeDef      TIM_InitStruct     = {0};
    LL_TIM_OC_InitTypeDef   TIM_OC_InitStruct  = {0};
    LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};

    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);     /* 240Mhz */

    GPIO_InitStruct.Pin = TM7707_MCLK_PIN;                                        
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                   
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;             
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;             
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                          
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;                                  
    LL_GPIO_Init(TM7707_MCLK_PORT, &GPIO_InitStruct);
                                                                         
    TIM_InitStruct.Prescaler = 0;                                       
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;                   
    TIM_InitStruct.Autoreload = 97;                                                           
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;           
    TIM_InitStruct.RepetitionCounter = 0;                               
    LL_TIM_Init(TM7707_MCLK_TIMER, &TIM_InitStruct);                    
    LL_TIM_DisableARRPreload(TM7707_MCLK_TIMER);                          

    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;                      
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;                 
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;                 
    TIM_OC_InitStruct.CompareValue = 49;                                   
    TIM_OC_InitStruct.OCPolarity  = LL_TIM_OCPOLARITY_HIGH;            
    TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;              
    LL_TIM_OC_Init(TM7707_MCLK_TIMER, TM7707_MCLK_TIMER_CHANNEL, &TIM_OC_InitStruct);  
    LL_TIM_OC_DisableFast(TM7707_MCLK_TIMER, TM7707_MCLK_TIMER_CHANNEL);              
    LL_TIM_OC_EnablePreload(TM7707_MCLK_TIMER, TM7707_MCLK_TIMER_CHANNEL);             

    LL_TIM_SetTriggerOutput(TM7707_MCLK_TIMER, LL_TIM_TRGO_RESET);       
    LL_TIM_DisableMasterSlaveMode(TM7707_MCLK_TIMER);                     
    TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;                  
    TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;                   
    TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;                  
    TIM_BDTRInitStruct.DeadTime = 0;                                     
    TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;                
    TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;        
    TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;  
    LL_TIM_BDTR_Init(TM7707_MCLK_TIMER, &TIM_BDTRInitStruct);             
    LL_TIM_EnableAllOutputs(TM7707_MCLK_TIMER);                           

    LL_TIM_CC_EnableChannel(TM7707_MCLK_TIMER, TM7707_MCLK_TIMER_CHANNEL);
    LL_TIM_EnableCounter(TM7707_MCLK_TIMER);
}



/*
*********************************************************************************************************
*    函 数 名: uint8_t
*    功能说明: 配置STM32的GPIO和SPI接口，用于连接 TM7707
*    形    参: 无
*    返 回 值: 0 表示失败; 1 表示成功
*********************************************************************************************************
*/
void bsp_InitTM7707(void)
{
    uint32_t reg_val;

    tm7707_mclk_init();

    /* 配置GPIO */
    {
        LL_GPIO_InitTypeDef gpio_init = {0};

        /* 打开GPIO时钟 */
        CS_CLK_ENABLE();
        DRDY_CLK_ENABLE();
        
        gpio_init.Mode = LL_GPIO_MODE_OUTPUT;        /* 设置推挽输出 */
        gpio_init.Pull = LL_GPIO_PULL_NO;            /* 上下拉电阻不使能 */
        gpio_init.Speed = LL_GPIO_SPEED_HIGH;        /* GPIO速度等级 */   
        gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;         
        gpio_init.Pin = CS_PIN;    
        LL_GPIO_Init(CS_GPIO, &gpio_init);    
        CS_1();

        /* DRDY 设置为输入 */
        gpio_init.Mode = LL_GPIO_MODE_INPUT;            /* 设置输入 */
        gpio_init.Pull = LL_GPIO_PULL_NO;               /* 上下拉电阻不使能 */
        gpio_init.Speed = LL_GPIO_SPEED_HIGH;           /* GPIO速度等级 */
        gpio_init.Pin = DRDY_PIN;    
        LL_GPIO_Init(DRDY_GPIO, &gpio_init);      

        soft_spi_init(1, 1);      
    }

    TM7707_DELAY_MS(10);

    tm7707_hard_reset();        /* 硬件复位 */

    /*
        在接口序列丢失的情况下，如果在DIN 高电平的写操作持续了足够长的时间（至少 32个串行时钟周期），
        TM7707 将会回到默认状态。
    */
    TM7707_DELAY_MS(5);

    tm7707_sync();        /* 同步SPI接口时序 */

    TM7707_DELAY_MS(5);

    // tm7707_wait_drdy();

    /* 复位之后, 时钟寄存器应该是 0x05 */
    uint8_t x;

    x = tm7707_read_register(REG_SETUP);
    x = tm7707_read_register(REG_FILTERH);

    if (tm7707_read_register(REG_FILTERH) != 0x00) {
        g_tm7707_ok = 1;
    } else {
        g_tm7707_ok = 0;
    }

    // /* 配置时钟寄存器 */
    // tm7707_write_byte(REG_CLOCK | WRITE | CH_1);            /* 先写通信寄存器，下一步是写时钟寄存器 */

    // tm7707_write_byte(CLKDIS_0 | CLK_4_9152M | FS_50HZ);    /* 刷新速率50Hz */
    // //tm7707_write_byte(CLKDIS_0 | CLK_4_9152M | FS_500HZ);    /* 刷新速率500Hz */

    tm7707_write_register(REG_TEST , 0x01);
    TM7707_DELAY_MS(10);
    // tm7707_write_register(REG_FILTERH, BST_1 | CLKDIS_1);



    tm7707_write_byte(0x51);    /* 滤波 */
    tm7707_write_byte(0x80);
    TM7707_DELAY_MS(10);
    tm7707_write_byte(0x21);
    tm7707_write_byte(0x31);

    TM7707_DELAY_MS(10);

    tm7707_write_byte(0x11);
    tm7707_write_byte(MD_CAL_SELF | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);    /*自校正*/
    x = tm7707_read_register(REG_COMM);

    TM7707_DELAY_MS(1000);

    tm7707_write_byte(0x11);
    tm7707_write_byte(MD_NORMAL | __CH2_GAIN_BIPOLAR_BUF | FSYNC_1);    /*自校正*/
    delay_ms(100);
    tm7707_write_byte(0x11);
    tm7707_write_byte(MD_NORMAL | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);    /*自校正*/
    delay_ms(100);
    // tm7707_write_byte(0x11);
    // tm7707_write_byte(0x40);    /*自校正*/
    // TM7707_DELAY_MS(10);
    // tm7707_write_byte(0x50);    /* 滤波 */
    // tm7707_write_byte(0x00);
    // TM7707_DELAY_MS(10);
    // tm7707_write_byte(0x20);
    // tm7707_write_byte(0x0f);

    // tm7707_write_byte(0x20 | WRITE | CH_1);			/* 先写通信寄存器，下一步是写时钟寄存器 */

	// tm7707_write_byte(CLKDIS_1 | 0x00);	/* 刷新速率50Hz */

    // /* 每次上电进行一次自校准 */
    // tm7707_calibself(1);    /* 内部自校准 CH1 */
    // tm7707_calibself(2);    /* 内部自校准 CH1 */
    TM7707_DELAY_MS(5);
    int32_t val;
    float f;
    while (1) {
        vTaskDelay(200);
        val = TM7707_ReadAdc(2);
        f = ((int32_t)(val - 0x800000) * 1250.0f) / (8388608.0f * 32);
        log_i("adc val: %.3f mv, %08X, %i", f, val, val);
    }
}

/*
*********************************************************************************************************
*    函 数 名: TM7707_SetCS
*    功能说明: TM7707 片选控制函数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void TM7707_SetCS(uint8_t _Level)
{
    if (_Level == 0)
    { 
        CS_0();
    }
    else
    {        
        CS_1();    
    }

    // TM7707_DELAY_US(10);

}

/*
*********************************************************************************************************
*    函 数 名: tm7707_hard_reset
*    功能说明: 硬件复位 TM7707芯片
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void tm7707_hard_reset(void)
{
    RESET_1();
    TM7707_DELAY_MS(1);
    RESET_0();
    TM7707_DELAY_MS(2);
    RESET_1();
    TM7707_DELAY_MS(1);
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_sync
*    功能说明: 同步TM7707芯片SPI接口时序
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void tm7707_sync(void)
{
    /* AD7705串行接口失步后将其复位。复位后要延时500us再访问 */
    TM7707_SetCS(0);
    
    g_spiLen = 0;
    g_spiTxBuf[g_spiLen++] = (0xFF);
    g_spiTxBuf[g_spiLen++] = (0xFF);
    g_spiTxBuf[g_spiLen++] = (0xFF);
    g_spiTxBuf[g_spiLen++] = (0xFF);
    soft_spi_write_then_stop(g_spiTxBuf, 4);
    
    TM7707_SetCS(1);
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_write_byte
*    功能说明: 写入1个字节。带CS控制
*    形    参: _data ：将要写入的数据
*    返 回 值: 无
*********************************************************************************************************
*/
static void tm7707_write_byte(uint8_t _data)
{
    TM7707_SetCS(0);
    g_spiLen = 0;
    g_spiTxBuf[g_spiLen++] = (_data);
    soft_spi_write_then_stop(g_spiTxBuf, 1);
    TM7707_SetCS(1);
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_write_3byte
*    功能说明: 写入3个字节。带CS控制
*    形    参: _data ：将要写入的数据
*    返 回 值: 无
*********************************************************************************************************
*/
static void tm7707_write_3byte(uint32_t _data)
{
    TM7707_SetCS(0);
    g_spiLen = 0;
    g_spiTxBuf[g_spiLen++] = ((_data >> 16) & 0xFF);
    g_spiTxBuf[g_spiLen++] = ((_data >> 8) & 0xFF);
    g_spiTxBuf[g_spiLen++] = (_data);
    soft_spi_write_then_stop(g_spiTxBuf, 3);
    TM7707_SetCS(1);
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_wait_drdy
*    功能说明: 等待内部操作完成。 自校准时间较长，需要等待。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static void tm7707_wait_drdy(void)
{
    uint32_t i, r;

    /* 如果初始化时未检测到芯片则快速返回，避免影响主程序响应速度 */
    if (g_tm7707_ok == 0)
    {
        return;
    }

    for (i = 0; i < 4000000; i++)
    {
        r = tm7707_read_register(REG_COMM);
        if ((r & 0x80) != 0x80)
        // if (DRDY_IS_LOW())
        {
            break;
        }
        vTaskDelay(1);
    }
    if (i >= 4000000)
    {
        log_i("tm7707_wait_drdy() Time Out ...");        /* 调试语句. 用语排错 */
    }
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_write_register
*    功能说明: 写指定的寄存器
*    形    参:  _RegID : 寄存器ID
*              _RegValue : 寄存器值。 对于8位的寄存器，取32位形参的低8bit
*    返 回 值: 无
*********************************************************************************************************
*/
void tm7707_write_register(uint8_t _RegID, uint32_t _RegValue)
{
    uint8_t bits;

    switch (_RegID)
    {
        case REG_COMM:        /* 通信寄存器 */
        case REG_SETUP:       /* 设置寄存器 8bit */
        case REG_TEST:        /* 时钟寄存器 8bit */
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

    tm7707_write_byte(_RegID | WRITE);    /* 写通信寄存器, 指定下一步是写操作，并指定写哪个寄存器 */

    if (bits == 8)
    {
        tm7707_write_byte((uint8_t)_RegValue);
    }
    else    /* 24bit */
    {
        tm7707_write_3byte(_RegValue);
    }
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_read_register
*    功能说明: 写指定的寄存器
*    形    参:  _RegID : 寄存器ID
*              _RegValue : 寄存器值。 对于8位的寄存器，取32位形参的低8bit
*    返 回 值: 读到的寄存器值。 对于8位的寄存器，取32位形参的低8bit
*********************************************************************************************************
*/
uint32_t tm7707_read_register(uint8_t _RegID)
{
    uint32_t read;

    switch (_RegID)
    {
        case REG_COMM:        /* 通信寄存器 */
        case REG_SETUP:       /* 设置寄存器 8bit */
        case REG_FILTERH:     /* 时钟寄存器 8bit */
        case REG_FILTERL:
        case REG_TEST:
            g_spiLen = 2;
            break;

        case REG_ZERO_CH1:    /* CH1 偏移寄存器 24bit */
        case REG_FULL_CH1:    /* CH1 满量程寄存器 24bit */
        case REG_ZERO_CH2:    /* CH2 偏移寄存器 24bit */
        case REG_FULL_CH2:    /* CH2 满量程寄存器 24bit*/
            g_spiLen = 4;
            break;

        case REG_DATA:        /* 数据寄存器 24bit */
        default:
            return 0xFFFFFFFF;
    }

    g_spiTxBuf[0] = (_RegID | READ);    /* 写通信寄存器, 指定下一步是写操作，并指定写哪个寄存器 */    
    TM7707_SetCS(0);
    soft_spi_write_then_read(g_spiTxBuf, 1, g_spiRxBuf, g_spiLen - 1);    
    TM7707_SetCS(1);
    if (g_spiLen == 2)            /* 返回结果1字节 */
    {
        read = g_spiRxBuf[0];
    }
    else if (g_spiLen == 4)       /* 返回结果3字节 */
    {
        read = (g_spiRxBuf[0] << 16) +  (g_spiRxBuf[1] << 8) + g_spiRxBuf[2];
    }
    return read;
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_calibself
*    功能说明: 启动自校准. 内部自动短接AIN+ AIN-校准0位，内部短接到Vref 校准满位。此函数执行过程较长，
*              实测约 180ms
*    形    参:  _ch : ADC通道，1或2
*    返 回 值: 无
*********************************************************************************************************
*/
void tm7707_calibself(uint8_t _ch)
{
    if (_ch == 1)
    {
        /* 自校准CH1 */
        tm7707_write_byte(REG_SETUP | WRITE | CH_1);    /* 写通信寄存器，下一步是写设置寄存器，通道1 */
        tm7707_write_byte(MD_CAL_SELF | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
        TM7707_DELAY_MS(1);
        tm7707_wait_drdy();    /* 等待内部操作完成 --- 时间较长，约180ms */
    }
    else if (_ch == 2)
    {
        /* 自校准CH2 */
        tm7707_write_byte(REG_SETUP | WRITE | CH_2);    /* 写通信寄存器，下一步是写设置寄存器，通道2 */
        tm7707_write_byte(MD_CAL_SELF | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);    /* 启动自校准 */
        TM7707_DELAY_MS(1);
        tm7707_wait_drdy();    /* 等待内部操作完成  --- 时间较长，约180ms */
    }
}

/*
*********************************************************************************************************
*    函 数 名: TM7707_SytemCalibZero
*    功能说明: 启动系统校准零位. 请将AIN+ AIN-短接后，执行该函数。校准应该由主程序控制并保存校准参数。
*             执行完毕后。可以通过 tm7707_read_register(REG_ZERO_CH1) 和  tm7707_read_register(REG_ZERO_CH2) 读取校准参数。
*    形    参: _ch : ADC通道，1或2
*    返 回 值: 无
*********************************************************************************************************
*/
void TM7707_SytemCalibZero(uint8_t _ch)
{
    if (_ch == 1)
    {
        /* 校准CH1 */
        tm7707_write_byte(REG_SETUP | WRITE | CH_1);    /* 写通信寄存器，下一步是写设置寄存器，通道1 */
        tm7707_write_byte(MD_CAL_ZERO | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
        tm7707_wait_drdy();    /* 等待内部操作完成 */
    }
    else if (_ch == 2)
    {
        /* 校准CH2 */
        tm7707_write_byte(REG_SETUP | WRITE | CH_2);    /* 写通信寄存器，下一步是写设置寄存器，通道1 */
        tm7707_write_byte(MD_CAL_ZERO | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);    /* 启动自校准 */
        tm7707_wait_drdy();    /* 等待内部操作完成 */
    }
}

/*
*********************************************************************************************************
*    函 数 名: TM7707_SytemCalibFull
*    功能说明: 启动系统校准满位. 请将AIN+ AIN-接最大输入电压源，执行该函数。校准应该由主程序控制并保存校准参数。
*             执行完毕后。可以通过 tm7707_read_register(REG_FULL_CH1) 和  tm7707_read_register(REG_FULL_CH2) 读取校准参数。
*    形    参:  _ch : ADC通道，1或2
*    返 回 值: 无
*********************************************************************************************************
*/
void TM7707_SytemCalibFull(uint8_t _ch)
{
    if (_ch == 1)
    {
        /* 校准CH1 */
        tm7707_write_byte(REG_SETUP | WRITE | CH_1);    /* 写通信寄存器，下一步是写设置寄存器，通道1 */
        tm7707_write_byte(MD_CAL_FULL | __CH1_GAIN_BIPOLAR_BUF | FSYNC_0);/* 启动自校准 */
        tm7707_wait_drdy();    /* 等待内部操作完成 */
    }
    else if (_ch == 2)
    {
        /* 校准CH2 */
        tm7707_write_byte(REG_SETUP | WRITE | CH_2);    /* 写通信寄存器，下一步是写设置寄存器，通道1 */
        tm7707_write_byte(MD_CAL_FULL | __CH2_GAIN_BIPOLAR_BUF | FSYNC_0);    /* 启动自校准 */
        tm7707_wait_drdy();    /* 等待内部操作完成 */
    }
}

/*
*********************************************************************************************************
*    函 数 名: tm7707_read_adc_register
*    功能说明: 读通道1或2的ADC数据。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
static uint32_t tm7707_read_adc_register(uint8_t _cmd)
{
    uint32_t read;
    
    TM7707_SetCS(0);
    g_spiTxBuf[0] = _cmd;
    g_spiLen = 4;
    soft_spi_write_then_read(g_spiTxBuf, 1, g_spiRxBuf, 3);
    read = ((uint32_t)(g_spiRxBuf[0]) << 16) | ((uint32_t)(g_spiRxBuf[1]) << 8) | g_spiRxBuf[2];
    TM7707_SetCS(1);
    
    return read;
}
    
/*
*********************************************************************************************************
*    函 数 名: TM7707_ReadAdc1
*    功能说明: 读通道1或2的ADC数据。 该函数执行较慢，但是可以读到正确结果。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint32_t TM7707_ReadAdc(uint8_t _ch)
{
    uint8_t i;
    uint32_t read = 0;

    /* 为了避免通道切换造成读数失效，读2次 */
    for (i = 0; i < 2; i++)
    {
        tm7707_wait_drdy();        /* 等待DRDY口线为0 */

        if (_ch == 1)
        {
            read = tm7707_read_adc_register(0x38);
        }
        else if (_ch == 2)
        {
            read = tm7707_read_adc_register(0x39);
        }    
    }
    return read;
}


/*
*********************************************************************************************************
*    函 数 名: TM7707_Scan2
*    功能说明: 扫描2个通道。插入到主程序循环，空闲时采集ADC值放到全局变量。方便读取。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void TM7707_Scan2(void)
{
    static uint8_t s_ch = 1;

    /* 如果初始化时未检测到芯片则快速返回，避免影响主程序响应速度 */
    if (g_tm7707_ok == 0)
    {
        return;
    }

    if (DRDY_IS_LOW())
    {
        /* 双通道交替读 */
        if (s_ch == 1)
        {
            g_TM7707_Adc2 = tm7707_read_adc_register(0x38);

            s_ch = 2;
        }
        else
        {
            g_TM7707_Adc1 = tm7707_read_adc_register(0x39);

            s_ch = 1;
        }
    }
}

/*
*********************************************************************************************************
*    函 数 名: TM7707_Scan1
*    功能说明: 扫描通道1。插入到主程序循环，空闲时采集ADC值放到全局变量。方便读取。
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void TM7707_Scan1(void)
{
    /* 如果初始化时未检测到芯片则快速返回，避免影响主程序响应速度 */
    if (g_tm7707_ok == 0)
    {
        return;
    }

    if (DRDY_IS_LOW())
    {
        g_TM7707_Adc1 = tm7707_read_adc_register(0x38);
    }
}

/*
*********************************************************************************************************
*    函 数 名: TM7707_GetAdc1
*    功能说明: 读取扫描的结果。需要结合 TM7707_Scan2() 或 TM7707_Scan1() 使用
*    形    参: 无
*    返 回 值: 通道1的ADC值
*********************************************************************************************************
*/
uint16_t TM7707_GetAdc1(void)
{
    /* 如果初始化时未检测到芯片则快速返回，避免影响主程序响应速度 */
    if (g_tm7707_ok == 0)
    {
        return 0;
    }

    return g_TM7707_Adc1;
}

/*
*********************************************************************************************************
*    函 数 名: TM7707_GetAdc2
*    功能说明: 读取扫描的结果。
*    形    参: 无
*    返 回 值: 通道2的ADC值
*********************************************************************************************************
*/
uint16_t TM7707_GetAdc2(void)
{
    /* 如果初始化时未检测到芯片则快速返回，避免影响主程序响应速度 */
    if (g_tm7707_ok == 0)
    {
        return 0;
    }

    return g_TM7707_Adc2;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
