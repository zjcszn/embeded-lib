#include "bsp_tim_dma.h"



/********************************************** 函数声明 *********************************************/

static void pwm_gpio_init(void);
static void pwm_tim_init(void);
static void pwm_dma_init(void);
static void pwm_dma_clk_enable(DMA_TypeDef *dma);
static void pwm_tim_clk_enable(TIM_TypeDef *tim_x);
static void pwm_gpio_clk_enable(GPIO_TypeDef *gpio_x);
static void pwm_freq_config(uint32_t pwm_freq);
static void pwm_dma_config(uint32_t *dmabuf_addr, uint32_t dmabuf_size);
static uint32_t pwm_tim_clk_freq(TIM_TypeDef *tim_x);

/********************************************** 函数定义 *********************************************/


/**
 * @brief PWM初始化程序
 * 
 */
void bsp_pwm_dma_init(void) {
  pwm_gpio_clk_enable(PWM_GPIO);
  pwm_dma_clk_enable(PWM_DMA);
  pwm_tim_clk_enable(PWM_TIM);

  pwm_gpio_init();      
  pwm_dma_init();
  pwm_tim_init();
  LL_SYSCFG_EnableCompensationCell();   // 使能IO补偿单元，降低高速IO翻转时的斜率  
}

/**
 * @brief 使能PWM输出
 * 
 * @param dmabuf_addr DMA缓冲区地址 
 * @param dmabuf_size DMA缓冲区大小
 * @param pwm_freq PWM频率
 */
void bsp_pwm_dma_start(uint32_t *dmabuf_addr, uint32_t dmabuf_size, uint32_t pwm_freq) {
  LL_TIM_DisableCounter(PWM_TIM);
  pwm_freq_config(pwm_freq << 1);
  pwm_dma_config(dmabuf_addr, dmabuf_size);
  LL_TIM_EnableCounter(PWM_TIM);
}

/**
 * @brief 关闭PWM输出
 * 
 */
void bsp_pwm_dma_stop(void) {
  LL_TIM_DisableCounter(PWM_TIM);
}


/**
 * @brief PWM GPIO初始化
 * 
 */
static void pwm_gpio_init(void) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = PWM_PIN;                                        // GPIO PIN设置
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;                           // GPIO 模式：输出模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;                 // GPIO 速率：VERY HIGH
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                 // GPIO 输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                               // GPIO 上下拉：无
  LL_GPIO_Init(PWM_GPIO, &GPIO_InitStruct);                             // GPIO 初始化
}

/**
 * @brief PWM 定时器初始化
 * 
 */
static void pwm_tim_init(void) {
  LL_TIM_DeInit(PWM_TIM);
  LL_TIM_InitTypeDef      TIM_InitStruct     = {0};
                                                                        // 更新事件周期 = TIM_CLK / ((预分频系数+1)*（自动重载值+1)*(重复计数值+1))
  TIM_InitStruct.Prescaler = 0;                                         // 预分频系数PSC
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;                   // 计数器模式：递增计数 | 递减计数 | 中心对齐计数
  TIM_InitStruct.Autoreload = SystemCoreClock / 1000000 - 1;            // 计数器自动重载值ARR                             
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;             // 时钟分割：定时器时钟CK_INT与死区发生器及数字滤波器时钟之间的分频比
  TIM_InitStruct.RepetitionCounter = 0;                                 // 计数器重复计数值
  LL_TIM_Init(PWM_TIM, &TIM_InitStruct);                                // 初始化定时器
  LL_TIM_EnableARRPreload(PWM_TIM);                                     // 使能自动重载预装载
  LL_TIM_EnableDMAReq_UPDATE(PWM_TIM);                                  // 使能定时器DMA请求（UPDATE)
}

/**
 * @brief PWM频率设置
 * 
 * @param tim_x 定时器编号
 * @param pwm_freq PWM频率
 */
static void pwm_freq_config(uint32_t pwm_freq) {
  uint32_t tim_psc;
  uint32_t tim_arr;
  uint32_t tim_clk = pwm_tim_clk_freq(PWM_TIM);

  // 假定PSC和ARR寄存器均为16位，为避免数据超出范围，需分段分频，主频设为168mhz
  if(pwm_freq < 100) {
    tim_psc = 2800 - 1;
    tim_arr = tim_clk / 2800 / pwm_freq - 1;
  }
  else if (pwm_freq < 3000) {
    tim_psc = 28 - 1;
    tim_arr = tim_clk / 28 / pwm_freq - 1;
  }
  else {
    tim_psc = 0;
    tim_arr = tim_clk / pwm_freq - 1;
  }
  LL_TIM_SetPrescaler(PWM_TIM, tim_psc);
  LL_TIM_SetAutoReload(PWM_TIM, tim_arr);
}


/**
 * @brief PWM DMA配置
 * 
 * @param dmabuf_addr DMA缓冲区地址 
 * @param dmabuf_size DMA缓冲区大小
 */
static void pwm_dma_config(uint32_t *dmabuf_addr, uint32_t dmabuf_size) {
  LL_DMA_SetMemoryAddress(PWM_DMA, PWM_DMA_STREAM, (uint32_t)dmabuf_addr);                      // 设置内存地址
  LL_DMA_SetDataLength(PWM_DMA, PWM_DMA_STREAM, dmabuf_size);                                   // 设置DMA传输数据长度
  LL_DMA_EnableStream(PWM_DMA, PWM_DMA_STREAM);                                                 // 使能DMA数据流
}

/**
 * @brief PWM DMA初始化
 * 
 */
static void pwm_dma_init(void) {
  LL_DMA_DeInit(PWM_DMA, PWM_DMA_STREAM);                                                       // 复位DMA寄存器
  LL_DMA_SetChannelSelection(PWM_DMA, PWM_DMA_STREAM, PWM_DMA_CHANNEL);                         // DMA 通道选择
  LL_DMA_SetDataTransferDirection(PWM_DMA, PWM_DMA_STREAM, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);  // DMA 数据传输方向：内存->外设
  LL_DMA_SetStreamPriorityLevel(PWM_DMA, PWM_DMA_STREAM, LL_DMA_PRIORITY_VERYHIGH);             // DMA 数据流优先级：低
  LL_DMA_SetMode(PWM_DMA, PWM_DMA_STREAM, LL_DMA_MODE_CIRCULAR);                                // DMA 模式：循环模式
  LL_DMA_SetPeriphIncMode(PWM_DMA, PWM_DMA_STREAM, LL_DMA_PERIPH_NOINCREMENT);                  // DMA 外设地址自增：否 
  LL_DMA_SetMemoryIncMode(PWM_DMA, PWM_DMA_STREAM, LL_DMA_MEMORY_INCREMENT);                    // DMA 内存地址自增：是
  LL_DMA_SetPeriphSize(PWM_DMA, PWM_DMA_STREAM, LL_DMA_PDATAALIGN_WORD);                        // DMA 外设数据宽度：Word
  LL_DMA_SetMemorySize(PWM_DMA, PWM_DMA_STREAM, LL_DMA_MDATAALIGN_WORD);                        // DMA 内存数据宽度：Word
  LL_DMA_DisableFifoMode(PWM_DMA, PWM_DMA_STREAM);                                              // DMA 硬件FIFO配置：关闭
  LL_DMA_SetPeriphAddress(PWM_DMA, PWM_DMA_STREAM, (uint32_t)&PWM_GPIO->BSRR);                  // 设置外设地址
}

/**
 * @brief 使能DMA时钟
 * 
 * @param dma DMA控制器编号
 */
static void pwm_dma_clk_enable(DMA_TypeDef *dma) {
  if (dma == DMA1) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  else LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
}

/**
 * @brief 使能定时器时钟
 * 
 * @param tim_x 定时器编号
 */
static void pwm_tim_clk_enable(TIM_TypeDef *tim_x) {
  if (tim_x == TIM1) LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
  else if (tim_x == TIM2)  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
  else if (tim_x == TIM3)  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
  else if (tim_x == TIM4)  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
  else if (tim_x == TIM5)  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
  else if (tim_x == TIM8)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM8);
  else if (tim_x == TIM9)  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);
  else if (tim_x == TIM10) LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);
  else if (tim_x == TIM11) LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM11);
  else if (tim_x == TIM12) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM12);
  else if (tim_x == TIM13) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM13);
  else if (tim_x == TIM14) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);
}

/**
 * @brief 使能GPIO时钟
 * 
 * @param gpio_x GPIO端口号
 */
static void pwm_gpio_clk_enable(GPIO_TypeDef *gpio_x) {
  if (gpio_x == GPIOA) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  else if (gpio_x == GPIOB) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  else if (gpio_x == GPIOC) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  else if (gpio_x == GPIOD) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  else if (gpio_x == GPIOE) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  else if (gpio_x == GPIOF) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  else if (gpio_x == GPIOG) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  else if (gpio_x == GPIOH) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  else if (gpio_x == GPIOI) LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOI);
}

/**
 * @brief 获取PWM定时器时钟频率
 * 
 * @param tim_x 定时器编号 
 * @return uint32_t 定时器时钟频率
 */
static uint32_t pwm_tim_clk_freq(TIM_TypeDef *tim_x) {
  uint32_t tim_clk = 0;
  LL_RCC_ClocksTypeDef rcc = {0};
  LL_RCC_GetSystemClocksFreq(&rcc);
  if (tim_x == TIM1 || tim_x == TIM8 || tim_x == TIM9 || tim_x == TIM10 ||tim_x == TIM11 ) {
    tim_clk = LL_RCC_GetAPB2Prescaler() ? (rcc.PCLK2_Frequency << 1) : rcc.PCLK2_Frequency;
  }
  else {
    tim_clk = LL_RCC_GetAPB1Prescaler() ? (rcc.PCLK1_Frequency << 1) : rcc.PCLK1_Frequency;
  }
  return tim_clk;
}