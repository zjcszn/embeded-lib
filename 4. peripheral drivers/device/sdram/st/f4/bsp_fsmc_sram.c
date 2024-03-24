#include "bsp_fsmc_sram.h"


static SRAM_HandleTypeDef fsmc_sram;

static void fsmc_sram_gpio_init(void);
static void fsmc_sram_periph_init(void);

void bsp_fsmc_sram_init(void) {
  fsmc_sram_gpio_init();
  fsmc_sram_periph_init();
}

static void fsmc_sram_gpio_init(void) {
  // 使能GPIO时钟
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);            
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;                     // GPIO 输出类型：推挽输出
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;                             // GPIO 上下拉：无
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;                // GPIO 速率：VERY HIGH

  GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;                      // GPIO 模式：复用模式
  GPIO_InitStruct.Alternate  = LL_GPIO_AF_12;                               // GPIO 复用选择：FSMC

  // 初始化 PD0  | PD1  | PD4  | PD5  | PD8  | PD9  | PD10 | PD11 | PD12 | PD13 | PD14 | PD15
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0  | LL_GPIO_PIN_1  | LL_GPIO_PIN_4  | LL_GPIO_PIN_5  | LL_GPIO_PIN_8  | 
                        LL_GPIO_PIN_9  | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 |
                        LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);                            

  // 初始化 PE0  | PE1  | PE7  | PE8  | PE9  | PE10 | PE11 | PE12 | PE13 | PE14 | PE15
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0  | LL_GPIO_PIN_1  | LL_GPIO_PIN_7  | LL_GPIO_PIN_8  | LL_GPIO_PIN_9  | 
                        LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | 
                        LL_GPIO_PIN_15;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);                            

  // 初始化 PF0  | PF1  | PF2  | PF3  | PF4  | PF5  | PF12 | PF13 | PF14 | PF15
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0  | LL_GPIO_PIN_1  | LL_GPIO_PIN_2  | LL_GPIO_PIN_3  | LL_GPIO_PIN_4  |
                        LL_GPIO_PIN_5  | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
  LL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  // 初始化 PG0  | PG1  | PG2  | PG3  | PG4  | PG5  | PG10
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0  | LL_GPIO_PIN_1  | LL_GPIO_PIN_2  | LL_GPIO_PIN_3  | LL_GPIO_PIN_4  |
                        LL_GPIO_PIN_5  | LL_GPIO_PIN_10;
  LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
}

static void fsmc_sram_periph_init(void) {
  fsmc_sram.Instance = FSMC_NORSRAM_DEVICE;                                                           
  fsmc_sram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

  FSMC_NORSRAM_TimingTypeDef RwTiming = {0};
  
  fsmc_sram.Init.NSBank = FSMC_NORSRAM_BANK3;                                // 设置要控制的Bank 区域：BANK3
  fsmc_sram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;             // 设置地址总线与数据总线是否复用：否
  fsmc_sram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;                         // 设置存储器的类型：SRAM
  fsmc_sram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;            // 设置存储器的数据宽度：16bit 
  fsmc_sram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;           // 设置是否支持突发访问模式：否（仅支持同步存储器）
  fsmc_sram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;         // 设置等待信号的极性：低电平
  fsmc_sram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;                          // 设置是否支持对齐的突发模式：否
  fsmc_sram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;              // 配置等待信号在等待前有效还是等待期间有效：等待前
  fsmc_sram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;               // 设置是否写使能：是
  fsmc_sram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;                      // 设置是否使能等待状态插入：否
  fsmc_sram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;                  // 设置是否使能扩展模式：否（读写使用相同时序）
  fsmc_sram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;          // 设置是否使能等待信号：否
  fsmc_sram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;                      // 设置是否使能写突发操作：否
  fsmc_sram.Init.PageSize = FSMC_PAGE_SIZE_NONE;                             // 指定页的大小：无

  RwTiming.AddressSetupTime = 0x02;                                          // 地址建立时间：0-15
  RwTiming.DataSetupTime    = 0x08;                                          // 数据建立时间：1-255
  RwTiming.BusTurnAroundDuration = 0x00;                                     // 总线转换周期：0-15
  RwTiming.AccessMode = FSMC_ACCESS_MODE_A;                                  // 设置访问模式：Mode A

  __HAL_RCC_FSMC_CLK_ENABLE();                                               // 使能FSMC时钟

  extern void Error_Handler(void);
  if (HAL_SRAM_Init(&fsmc_sram, &RwTiming, NULL) != HAL_OK) {
    printf("FSMC Init Failed\r\n");
    Error_Handler( );
  }
}