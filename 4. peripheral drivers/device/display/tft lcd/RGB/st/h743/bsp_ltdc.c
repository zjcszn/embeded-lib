//
// Created by zjcszn on 2024/2/21.
//

#define LOG_TAG "LTDC"
#define LOG_LVL ELOG_LVL_INFO

#include "bsp_ltdc.h"

#include <string.h>

#include "stm32h7xx.h"
#include "stm32h7xx_hal_ltdc.h"
#include "stm32h7xx_ll_gpio.h"

#include "elog.h"
#include "perf_counter.h"

#include "FreeRTOS.h"
#include "semphr.h"

LTDC_HandleTypeDef ltdc;
SemaphoreHandle_t ltdc_sync;

uint32_t ltdc_framebuf_1[LCD_WIDTH * LCD_HEIGHT] __attribute__((section(".ARM.__at_0xC0000000")));
uint32_t ltdc_framebuf_2[LCD_WIDTH * LCD_HEIGHT] __attribute__((section(".ARM.__at_0xC0180000")));

static int ltdc_clk_config(void) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    
    /* LTDC freq = HSE freq (25Mhz) / PLL3M * PLL3N / PLL3R = 30Mhz */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLL3.PLL3M = 5;
    PeriphClkInitStruct.PLL3.PLL3N = 48;
    PeriphClkInitStruct.PLL3.PLL3P = 5;
    PeriphClkInitStruct.PLL3.PLL3Q = 5;
    PeriphClkInitStruct.PLL3.PLL3R = 8;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = 0;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        log_e("ltdc clock config failed");
        return 1;
    }
    
    __HAL_RCC_LTDC_CLK_ENABLE();
    __HAL_RCC_DMA2D_CLK_ENABLE();
    return 0;
}

static void ltdc_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    
    /*------------------------------LTDC GPIO Configuration--------------------------
        
    PG13    ------> LTDC_R0      PE5     ------> LTDC_G0       PG14    ------> LTDC_B0
    PA2     ------> LTDC_R1      PE6     ------> LTDC_G1       PG12    ------> LTDC_B1
    PH8     ------> LTDC_R2      PH13    ------> LTDC_G2       PD6     ------> LTDC_B2
    PH9     ------> LTDC_R3      PH14    ------> LTDC_G3       PA8     ------> LTDC_B3
    PH10    ------> LTDC_R4      PH15    ------> LTDC_G4       PI4     ------> LTDC_B4
    PH11    ------> LTDC_R5      PI0     ------> LTDC_G5       PI5     ------> LTDC_B5
    PH12    ------> LTDC_R6      PI1     ------> LTDC_G6       PI6     ------> LTDC_B6
    PG6     ------> LTDC_R7      PI2     ------> LTDC_G7       PI7     ------> LTDC_B7
     
    PG7     ------> LTDC_CLK
    PF10    ------> LTDC_DE
    PI9     ------> LTDC_VSYNC
    PI10    ------> LTDC_HSYNC
--*/
    
    /*** LTDC Pins configuration ***/
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    
    /* PA2 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PA8 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* PD6 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    /* PE5 PE6 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5 | LL_GPIO_PIN_6;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    /* PF10 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    /* PG6 PG7 PG12 PG13 PG14 */
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_6 | LL_GPIO_PIN_7 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    
    /* PH8 PH9 PH10 PH11 PH12 PH13 PH14 PH15 */
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 |
        LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    
    /* PI0 PI1 PI2 PI4 PI5 PI6 PI7 PI9 PI10 */
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 |
        LL_GPIO_PIN_6 | LL_GPIO_PIN_7 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    LL_GPIO_Init(GPIOI, &GPIO_InitStruct);
    
    /* turn off backlight */
    LCD_BACKLIGHT_OFF();
    
    /* BL -> PH6 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Alternate = 0;
    LL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}

static int ltdc_periph_init(void) {
    /* 复位LTDC */
    __HAL_RCC_LTDC_FORCE_RESET();
    __HAL_RCC_LTDC_RELEASE_RESET();
    
    memset(&ltdc, 0x00, sizeof(LTDC_HandleTypeDef));
    ltdc.Instance = LTDC;
    
    /* 配置信号极性 */
    /* NOTE: DE控制信号应与LCD datasheet指示的极性相反， 参考AN4861 P61 */
    ltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;     /* Hsync极性：低电平有效 */
    ltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;     /* Vsync极性：低电平有效 */
    ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;     /* DE极性：低电平有效 */
    ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;    /* 像素时钟极性： 低电平有效 */
    
    /* 配置时序 */
    ltdc.Init.HorizontalSync = HSW - 1;            /* 水平同步宽度 */
    ltdc.Init.VerticalSync = VSW - 1;              /* 垂直同步宽度 */
    ltdc.Init.AccumulatedHBP = HSW + HBP - 1;      /* 水平后沿配置：HSYNC Width + HBP - 1 */
    ltdc.Init.AccumulatedVBP = VSW + VBP - 1;      /* 垂直后沿配置：VSYNC Width + VBP - 1 */
    ltdc.Init.AccumulatedActiveW = LCD_WIDTH + HSW + HBP - 1;      /* 有效宽度 */
    ltdc.Init.AccumulatedActiveH = LCD_HEIGHT + VSW + VBP - 1;     /* 有效高度 */
    ltdc.Init.TotalWidth = LCD_WIDTH + HSW + HBP + HFP - 1;        /* 总宽度 */
    ltdc.Init.TotalHeigh = LCD_HEIGHT + VSW + VBP + VFP - 1;       /* 总高度 */
    
    /* 配置背景层颜色 */
    ltdc.Init.Backcolor.Red = 0;
    ltdc.Init.Backcolor.Green = 0;
    ltdc.Init.Backcolor.Blue = 0;
    
    if (HAL_LTDC_Init(&ltdc) != HAL_OK) {
        log_e("ltdc init failed");
        return 1;
    }

#if 0
    /* 设置行中断, 如果需要在消隐期间刷新帧缓冲，可启用行中断 */
    HAL_LTDC_ProgramLineEvent(&ltdc, 0);
#endif
    
    NVIC_SetPriority(LTDC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0xF, 0));
    NVIC_EnableIRQ(LTDC_IRQn);
    
    return 0;
}

static int ltdc_layer1_init(void) {
    LTDC_LayerCfgTypeDef pLayerCfg;
    
    /* 配置显示窗口 */
    pLayerCfg.WindowX0 = 0;
    pLayerCfg.WindowY0 = 0;
    pLayerCfg.WindowX1 = LCD_WIDTH;
    pLayerCfg.WindowY1 = LCD_HEIGHT;
    
    /* 配置像素颜色格式 */
    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
    
    /* 配置Alpha常数， 255表示不透明 */
    pLayerCfg.Alpha = 255;
    
    /* 配置背景色 */
    pLayerCfg.Alpha0 = 0;
    pLayerCfg.Backcolor.Blue = 0;
    pLayerCfg.Backcolor.Green = 0;
    pLayerCfg.Backcolor.Red = 0;
    
    /* 配置图层混合因数 */
    pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;    /* 像素Alpha * 常数Alpha */
    pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR1_PAxCA;
    
    /* 配置帧缓冲区 */
    pLayerCfg.FBStartAdress = (uint32_t)ltdc_framebuf_1;
    pLayerCfg.ImageWidth = LCD_WIDTH;
    pLayerCfg.ImageHeight = LCD_HEIGHT;
    
    if (HAL_LTDC_ConfigLayer(&ltdc, &pLayerCfg, 0) != HAL_OK) {
        log_e("ltdc config layer1 failed");
        return 1;
    }
    
    return 0;
}

int bsp_ltdc_init(void) {
    ltdc_sync = xSemaphoreCreateBinary();
    if (ltdc_sync == NULL)
        return 1;
    
    ltdc_gpio_init();
    
    if (ltdc_clk_config() || ltdc_periph_init() || ltdc_layer1_init())
        return 1;
    
    delay_ms(100);
    LCD_BACKLIGHT_ON();
    
    return 0;
}

void bsp_ltdc_wait_reload(void) {
    xSemaphoreTake(ltdc_sync, portMAX_DELAY);
}

void bsp_ltdc_set_framebuf(uint32_t fbuf) {
    LTDC_Layer1->CFBAR = fbuf;  /* Set Layer1 frame buffer address */
    LTDC->IER |= LTDC_IT_RR;    /* Enable Reload interrupt */
    LTDC->SRCR = LTDC_RELOAD_VERTICAL_BLANKING;     /* Apply Reload type */
}

void bsp_ltdc_reload_event_callback(void) {
    BaseType_t xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR(ltdc_sync, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void LTDC_IRQHandler(void) {
    HAL_LTDC_IRQHandler(&ltdc);
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc) {

}

void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
    bsp_ltdc_reload_event_callback();
}



