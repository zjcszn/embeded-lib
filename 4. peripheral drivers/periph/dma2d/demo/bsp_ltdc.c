//
// Created by zjcszn on 2024/2/21.
//

#define LOG_TAG "LTDC"
#define LOG_LVL ELOG_LVL_INFO

#include "bsp_ltdc.h"

#include <string.h>

#include "main.h"


LTDC_HandleTypeDef ltdc;
SemaphoreHandle_t ltdc_sync;

uint16_t ltdc_buf1[LCD_WIDTH * LCD_HEIGHT] __attribute__((section(".bss.ARM.__at_0xC0000000")));


static int ltdc_clk_config(void) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    
    /* LTDC freq = HSE freq (25Mhz) / PLL3M * PLL3N / PLL3R = 30Mhz */
    /* Init in SystemConfig() */

    LL_APB3_GRP1_EnableClock(LL_APB3_GRP1_PERIPH_LTDC);    
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_DMA2D);

    return 0;
}

static void ltdc_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIOs clock */

    /*------------------------------LTDC GPIO Configuration--------------------------
        
    PI15    ------> LTDC_R0      PJ7     ------> LTDC_G0       PJ12    ------> LTDC_B0
    PJ0     ------> LTDC_R1      PJ8     ------> LTDC_G1       PJ13    ------> LTDC_B1
    PJ1     ------> LTDC_R2      PJ9     ------> LTDC_G2       PJ14    ------> LTDC_B2
    PJ2     ------> LTDC_R3      PJ10    ------> LTDC_G3       PJ15    ------> LTDC_B3
    PJ3     ------> LTDC_R4      PJ11    ------> LTDC_G4       PK3     ------> LTDC_B4
    PJ4     ------> LTDC_R5      PK0     ------> LTDC_G5       PK4     ------> LTDC_B5
    PJ5     ------> LTDC_R6      PK1     ------> LTDC_G6       PK5     ------> LTDC_B6
    PJ6     ------> LTDC_R7      PK2     ------> LTDC_G7       PK6     ------> LTDC_B7
     
    PI14    ------> LTDC_CLK
    PK7     ------> LTDC_DE
    PI13    ------> LTDC_VSYNC
    PI12    ------> LTDC_HSYNC
    PB5     ------> LCD_BL

    --*/
    
    /*** LTDC Pins configuration ***/
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_14;
    
    /* PI 12~15 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    /* PJ ALL*/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_ALL;
    LL_GPIO_Init(GPIOJ, &GPIO_InitStruct);
    
    /* PK 0~7 */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | 
                          LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    LL_GPIO_Init(GPIOK, &GPIO_InitStruct);
    
    /* turn off backlight */
    LCD_BACKLIGHT_OFF();
    
    /* BL -> PH6 */
    GPIO_InitStruct.Pin = LCD_BACKLIGHT_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Alternate = 0;
    LL_GPIO_Init(LCD_BACKLIGHT_PORT, &GPIO_InitStruct);
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
    pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    
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
    memset(ltdc_buf1, 0x00, LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t));
    pLayerCfg.FBStartAdress = (uint32_t)ltdc_buf1;
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
    
    vTaskDelay(pdMS_TO_TICKS(100));
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

void LTDC_IRQHandler(void) {
    HAL_LTDC_IRQHandler(&ltdc);
}

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc) {
    BaseType_t xHigherPriorityTaskWoken;
    xSemaphoreGiveFromISR(ltdc_sync, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
    LTDC->ICR = LTDC_FLAG_LI;
    LTDC->IER |= LTDC_IT_LI;
}



