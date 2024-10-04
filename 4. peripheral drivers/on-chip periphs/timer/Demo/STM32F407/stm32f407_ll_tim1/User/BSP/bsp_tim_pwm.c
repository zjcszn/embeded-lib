#include "bsp_tim_pwm.h"


/********************************************** 函数声明 *********************************************/

static void pwm_gpio_init(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af);
static void pwm_tim_init(TIM_TypeDef *tim_x, uint32_t channel);
static void pwm_tim_clk_enable(TIM_TypeDef *tim_x);
static void pwm_tim_clk_disable(TIM_TypeDef *tim_x);
static void pwm_gpio_clk_enable(GPIO_TypeDef *gpio_x);
static void pwm_freq_config(TIM_TypeDef *tim_x, uint32_t pwm_freq);
static void pwm_duty_config(TIM_TypeDef *tim_x, uint32_t channel, uint32_t duty);
static uint32_t pwm_tim_clk_freq(TIM_TypeDef *tim_x);
static uint32_t pwm_alternate_select(TIM_TypeDef *tim_x);

/********************************************** 函数定义 *********************************************/


/**
 * @brief PWM频率更新
 * 
 * @param tim_x 定时器编号
 * @param pwm_freq PWM频率
 */
void bsp_pwm_freq_update(TIM_TypeDef *tim_x, uint32_t pwm_freq) {
  pwm_freq_config(tim_x, pwm_freq);
}

/**
 * @brief PWM占空比更新
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 * @param duty_cycle 占空比
 */
void bsp_pwm_duty_update(TIM_TypeDef *tim_x, uint32_t channel, uint32_t duty_cycle) {
  pwm_duty_config(tim_x, channel, duty_cycle);
}

/**
 * @brief PWM单通道输出初始化
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 * @param gpio_x GPIO端口号
 * @param pin GPIO引脚
 */
void bsp_pwm_output_init(TIM_TypeDef *tim_x, uint32_t channel, GPIO_TypeDef *gpio_x, uint32_t pin) {
  pwm_gpio_clk_enable(gpio_x);
  pwm_tim_clk_enable(tim_x);
  pwm_gpio_init(gpio_x, pin, pwm_alternate_select(tim_x));
  pwm_tim_init(tim_x, channel);
}

/**
 * @brief PWM单通道输出使能
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 * @param pwm_freq PWM频率
 * @param duty_cycle 占空比
 */
void bsp_pwm_output_start(TIM_TypeDef *tim_x, uint32_t channel, uint32_t pwm_freq, uint32_t duty_cycle) {
  pwm_freq_config(tim_x, pwm_freq);
  pwm_duty_config(tim_x, channel, duty_cycle);
  LL_TIM_CC_EnableChannel(tim_x, channel);
  LL_TIM_EnableCounter(tim_x);
}

/**
 * @brief PWM单通道输出关闭
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 */
void bsp_pwm_output_stop(TIM_TypeDef *tim_x, uint32_t channel) {
  LL_TIM_CC_DisableChannel(tim_x, channel);
}

/**
 * @brief PWM双通道互补输出初始化
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 * @param gpiop_x P.GPIO端口号
 * @param ppin P.GPIO引脚
 * @param gpion_x N.GPIO端口号
 * @param npin N.GPIO引脚
 */
void bsp_pwm_output_dual_init(TIM_TypeDef *tim_x, uint32_t channel, GPIO_TypeDef *gpiop_x, uint32_t ppin,
                            GPIO_TypeDef *gpion_x, uint32_t npin) {
  if (tim_x == TIM1 || tim_x == TIM8) {
    pwm_gpio_clk_enable(gpiop_x);
    pwm_gpio_clk_enable(gpion_x);
    pwm_tim_clk_enable(tim_x);
    pwm_gpio_init(gpiop_x, ppin, pwm_alternate_select(tim_x));
    pwm_gpio_init(gpion_x, npin, pwm_alternate_select(tim_x));
    pwm_tim_init(tim_x, channel);
  }
}

/**
 * @brief PWM双通道互补输出使能
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 * @param pwm_freq PWM频率
 * @param duty_cycle 占空比
 * @param deadtime 死区时间
 */
void bsp_pwm_output_dual_start(TIM_TypeDef *tim_x, uint32_t channel, uint32_t pwm_freq, uint32_t duty_cycle, uint32_t deadtime) {
  if (tim_x == TIM1 || tim_x == TIM8) {
    pwm_freq_config(tim_x, pwm_freq);
    pwm_duty_config(tim_x, channel, duty_cycle);
    LL_TIM_OC_SetDeadTime(tim_x, deadtime);
    LL_TIM_CC_EnableChannel(tim_x, channel);
    LL_TIM_CC_EnableChannel(tim_x, channel << 2);
    LL_TIM_EnableCounter(tim_x);
  }
}


/**
 * @brief PWM双通道互补输出关闭
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 */
void bsp_pwm_output_dual_stop(TIM_TypeDef *tim_x, uint32_t channel) {
  if (tim_x == TIM1 || tim_x == TIM8) {
    LL_TIM_CC_DisableChannel(tim_x, channel);
    LL_TIM_CC_DisableChannel(tim_x, channel << 2);
  }
}


/**
 * @brief PWM占空比设置
 * 
 * @param tim_x 定时器编号
 * @param channel 定时器通道
 * @param duty 占空比（0~10000）
 */
static void pwm_duty_config(TIM_TypeDef *tim_x, uint32_t channel,  uint32_t duty) {
  uint32_t oc_ccr;
  if (duty == 0) {
    oc_ccr = 0;
  }
  else if (duty >= 10000) {
    oc_ccr = UINT16_MAX;
  }
  else {
    oc_ccr = (duty * LL_TIM_GetAutoReload(tim_x)) / 10000;
    if (!oc_ccr) oc_ccr = 1;
  }
  if (channel == LL_TIM_CHANNEL_CH1) LL_TIM_OC_SetCompareCH1(tim_x, oc_ccr);
  else if (channel == LL_TIM_CHANNEL_CH2) LL_TIM_OC_SetCompareCH2(tim_x, oc_ccr);
  else if (channel == LL_TIM_CHANNEL_CH3) LL_TIM_OC_SetCompareCH3(tim_x, oc_ccr);
  else if (channel == LL_TIM_CHANNEL_CH4) LL_TIM_OC_SetCompareCH4(tim_x, oc_ccr);
}

/**
 * @brief PWM频率设置
 * 
 * @param tim_x 定时器编号
 * @param pwm_freq PWM频率
 */
static void pwm_freq_config(TIM_TypeDef *tim_x, uint32_t pwm_freq) {
  uint32_t tim_psc;
  uint32_t tim_arr;
  uint32_t tim_clk = pwm_tim_clk_freq(tim_x);

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
  LL_TIM_SetPrescaler(tim_x, tim_psc);
  LL_TIM_SetAutoReload(tim_x, tim_arr);
}


/**
 * @brief PWM引脚初始化
 * 
 * @param gpio GPIO端口
 * @param pin GPIO引脚
 * @param af 复用功能选择
 */
static void pwm_gpio_init(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af) {
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = pin;                                        // 设置PWM输出引脚
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;                    // GPIO模式：复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;             // GPIO速率：VERY HIGH
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;             // GPIO输出模式：推挽输出
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;                           // GPIO上下拉：无
  GPIO_InitStruct.Alternate = af;                                   // GPIO复用选择
  LL_GPIO_Init(gpio, &GPIO_InitStruct);                             // GPIO初始化
}

/**
 * @brief PWM定时器初始化
 * 
 * @param tim_x 定时器编号 
 * @param channel 输出通道
 */
static void pwm_tim_init(TIM_TypeDef *tim_x, uint32_t channel) {
  LL_TIM_InitTypeDef      TIM_InitStruct     = {0};
  LL_TIM_OC_InitTypeDef   TIM_OC_InitStruct  = {0};
  LL_TIM_BDTR_InitTypeDef TIM_BDTRInitStruct = {0};
                                                                        // 更新事件周期 = TIM_CLK / ((预分频系数+1)*（自动重载值+1)*(重复计数值+1))
  TIM_InitStruct.Prescaler = 0;                                         // 预分频系数PSC
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;                   // 计数器模式：递增计数 | 递减计数 | 中心对齐计数
  TIM_InitStruct.Autoreload = 0;                                        // 计数器自动重载值ARR                             
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;             // 时钟分割：定时器时钟CK_INT与死区发生器及数字滤波器时钟之间的分频比
  TIM_InitStruct.RepetitionCounter = 0;                                 // 计数器重复计数值
  LL_TIM_Init(tim_x, &TIM_InitStruct);                                  // 初始化定时器
  LL_TIM_DisableARRPreload(tim_x);                                       // 使能自动重载预装载

  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;                        // 输出比较模式：PWM1模式 如TIMx_CNT < TIMx_CCRx输出有效信号，否则输出无效信号
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;                   // 禁止OC输出
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;                  // 禁止OC互补输出
  TIM_OC_InitStruct.CompareValue = 0;                                   // 比较值->TIMx_CCRx
  TIM_OC_InitStruct.OCPolarity  = LL_TIM_OCPOLARITY_HIGH;               // OC输出极性：高电平有效
  TIM_OC_InitStruct.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;               // OC互补输出极性：高电平有效
  LL_TIM_OC_Init(tim_x, channel, &TIM_OC_InitStruct);                   // 初始化PWM OC通道
  LL_TIM_OC_DisableFast(tim_x, channel);                                // 禁止快速模式(仅PWM模式下有效，使能时可快速响应触发输入信号)                   
  LL_TIM_OC_EnablePreload(tim_x, channel);                              // 使能CCR寄存器预装载

  LL_TIM_SetTriggerOutput(tim_x, LL_TIM_TRGO_RESET);                    // TRGO信号选择:REST信号
  LL_TIM_DisableMasterSlaveMode(tim_x);                                 // 禁用主从模式：使能时，当前定时器的TRGI事件动作将被推迟，以使主从定时器同步动作      
  TIM_BDTRInitStruct.OSSRState = LL_TIM_OSSR_DISABLE;                   // 运行模式下的关闭状态选择
  TIM_BDTRInitStruct.OSSIState = LL_TIM_OSSI_DISABLE;                   // 空闲模式下的关闭状态选择
  TIM_BDTRInitStruct.LockLevel = LL_TIM_LOCKLEVEL_OFF;                  // 锁定配置：关闭（使能时不可修改配置，锁定范围依LOCK等级）
  TIM_BDTRInitStruct.DeadTime = 0;                                      // 设置死区时间
  TIM_BDTRInitStruct.BreakState = LL_TIM_BREAK_DISABLE;                 // 断路使能：关闭（使能时，BRK和CSS时钟故障事件可立即切断OC输出）
  TIM_BDTRInitStruct.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;        // 断路输入信号极性：高电平有效
  TIM_BDTRInitStruct.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;  // 自动输出使能：关闭（打开时，MOE可在更新事件发生时由硬件自动置1）
  LL_TIM_BDTR_Init(tim_x, &TIM_BDTRInitStruct);                         // 初始化断路和死区配置
  LL_TIM_EnableAllOutputs(tim_x);                                       // 主输出使能  
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
 * @brief 关闭定时器时钟
 * 
 * @param tim_x 定时器编号
 */
static void pwm_tim_clk_disable(TIM_TypeDef *tim_x) {
  if (tim_x == TIM1) LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM1);
  else if (tim_x == TIM2)  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
  else if (tim_x == TIM3)  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM3);
  else if (tim_x == TIM4)  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM4);
  else if (tim_x == TIM5)  LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM5);
  else if (tim_x == TIM8)  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM8);
  else if (tim_x == TIM9)  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM9);
  else if (tim_x == TIM10) LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM10);
  else if (tim_x == TIM11) LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM11);
  else if (tim_x == TIM12) LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM12);
  else if (tim_x == TIM13) LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM13);
  else if (tim_x == TIM14) LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM14);
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
 * @brief PWM引脚复用选择
 * 
 * @param tim_x 定时器编号
 * @return uint32_t 复用功能编号
 */
static uint32_t pwm_alternate_select(TIM_TypeDef *tim_x) {
  if (tim_x == TIM1 || tim_x == TIM2) return LL_GPIO_AF_1;
  else if (tim_x == TIM3  || tim_x == TIM5  || tim_x == TIM5) return LL_GPIO_AF_2;
  else if (tim_x == TIM8  || tim_x == TIM9  || tim_x == TIM10  || tim_x == TIM11) return LL_GPIO_AF_3;
  else if (tim_x == TIM12 || tim_x == TIM13 || tim_x == TIM14) return LL_GPIO_AF_9;
  else return 0;
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
