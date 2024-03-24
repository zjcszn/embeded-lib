//
// Created by zjcszn on 2024/1/29.
//

#include <stdio.h>
#include <string.h>
#include "bsp_sdram.h"

#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_fmc.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_rcc.h"

#include "FreeRTOS.h"
#include "task.h"

static void prv_sdram_gpio_init(void) {
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOF);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOG);
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);
    
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_HIGH;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10
            | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_7 | LL_GPIO_PIN_8 | LL_GPIO_PIN_9
            | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14
            | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_4
            | LL_GPIO_PIN_5 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14
            | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin =
        LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5
            | LL_GPIO_PIN_8 | LL_GPIO_PIN_15;
    LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_5;
    LL_GPIO_Init(GPIOH, &GPIO_InitStruct);
}

static void prv_sdram_fmc_init(void) {
    LL_RCC_SetFMCClockSource(LL_RCC_FMC_CLKSOURCE_HCLK);
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_FMC);
    
    /* FMC clock freq = HCLK3 div 2 = 120Mhz , 8.3ns per cycle */
    FMC_SDRAM_TimingTypeDef SDRAM_TimingStruct = {0};
    SDRAM_TimingStruct.ExitSelfRefreshDelay = 10;   /* tXSR min: 75ns */
    SDRAM_TimingStruct.RPDelay = 3; /* tRP min: 20ns, Precharge to Active Command Period */
    SDRAM_TimingStruct.RCDDelay = 3; /* tRCD min: 20ns , Active to Read/Write Command Delay Time */
    SDRAM_TimingStruct.SelfRefreshTime = 6; /* tRAS min: 45ns */
    SDRAM_TimingStruct.LoadToActiveDelay = 2; /* tRSC min: 2 tCK, Mode Register Set Cycle Time */
    SDRAM_TimingStruct.WriteRecoveryTime = 2; /* tWR min: 2 tCK */
    SDRAM_TimingStruct.RowCycleDelay = 8; /* tRC min: 65ns, Ref/Active to Ref/Active Command Period */
    FMC_SDRAM_Timing_Init(FMC_SDRAM_DEVICE, &SDRAM_TimingStruct, FMC_SDRAM_BANK1);
    
    FMC_SDRAM_InitTypeDef SDRAM_InitStruct = {0};
    SDRAM_InitStruct.SDBank = FMC_SDRAM_BANK1;
    SDRAM_InitStruct.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_9;
    SDRAM_InitStruct.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
    SDRAM_InitStruct.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    SDRAM_InitStruct.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    SDRAM_InitStruct.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    SDRAM_InitStruct.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    SDRAM_InitStruct.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    SDRAM_InitStruct.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    SDRAM_InitStruct.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
    FMC_SDRAM_Init(FMC_SDRAM_DEVICE, &SDRAM_InitStruct);
    
    /* Enable FMC Peripheral */
    __FMC_ENABLE();
}

static void prv_sdram_chip_init(void) {
    uint32_t tmpmrd = 0;
    
    FMC_SDRAM_CommandTypeDef SDRAM_CommandStruct = {0};
    
    /* STEP1: enable clock */
    SDRAM_CommandStruct.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
    SDRAM_CommandStruct.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
    SDRAM_CommandStruct.AutoRefreshNumber = 1;
    SDRAM_CommandStruct.ModeRegisterDefinition = 0;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &SDRAM_CommandStruct, SDRAM_TIMEOUT);
    
    /* STEP2: delay least 100 us */
    vTaskDelay(pdMS_TO_TICKS(1));
    
    /* STEP3: command-> precharge all */
    SDRAM_CommandStruct.CommandMode = FMC_SDRAM_CMD_PALL;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &SDRAM_CommandStruct, SDRAM_TIMEOUT);
    
    /* STEP4: command-> auto refresh */
    SDRAM_CommandStruct.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    SDRAM_CommandStruct.AutoRefreshNumber = 8;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &SDRAM_CommandStruct, SDRAM_TIMEOUT);
    
    /* STEP5: configure mode register */
    tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL
        | SDRAM_MODEREG_CAS_LATENCY_2 | SDRAM_MODEREG_OPERATING_MODE_STANDARD
        | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
    SDRAM_CommandStruct.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
    SDRAM_CommandStruct.AutoRefreshNumber = 1;
    SDRAM_CommandStruct.ModeRegisterDefinition = tmpmrd;
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &SDRAM_CommandStruct, SDRAM_TIMEOUT);
    
    /* STEP6: set refresh freq */
    /**
     * SDRAM refresh period / numbers of rows * sdram clock speed - 20 =
     * 64ms / 8192 * 80Mhz - 20 = 917
     */
    FMC_SDRAM_ProgramRefreshRate(FMC_SDRAM_DEVICE, 917);
}

void bsp_sdram_init(void) {
    prv_sdram_gpio_init();
    prv_sdram_fmc_init();
    prv_sdram_chip_init();
}

/*--------------------SDRAM TEST---------------------- */

#if SDRAM_TEST == 1

uint8_t *sdram = (uint8_t *)0xC0000000;;

void bsp_sdram_test(void) {
    uint32_t is_failed;
    TickType_t ticks;
    
    bsp_sdram_init();
    
    is_failed = 0;
    printf("-------SDRAM TEST: STEP 1-------\r\n");
    ticks = xTaskGetTickCount();
    memset(sdram, 0xA5, 0x2000000);
    printf("Full Erase by 0xA5, %ums...\r\n", pdTICKS_TO_MS(xTaskGetTickCount() - ticks));
    for (int i = 0; i < 0x2000000; i++) {
        if (sdram[i] != 0xA5) {
            is_failed = 1;
            printf("err: addr-> %p data-> 0x%02X\r\n", &sdram[i], sdram[i]);
        }
    }
    printf("TEST Result: %s\r\n", is_failed ? "FAILED" : "PASS");
    
    is_failed = 0;
    printf("-------SDRAM TEST: STEP 2-------\r\n");
    ticks = xTaskGetTickCount();
    memset(sdram, 0x00, 0x2000000);
    printf("Full Erase by 0x00, %ums...\r\n", pdTICKS_TO_MS(xTaskGetTickCount() - ticks));
    for (int i = 0; i < 0x2000000; i++) {
        if (sdram[i] != 0x00) {
            is_failed = 1;
            printf("err: addr-> %p data-> 0x%02X\r\n", &sdram[i], sdram[i]);
        }
    }
    printf("Test Result: %s\r\n", is_failed ? "FAILED" : "PASS");
    
    printf("--------SDRAM TEST: END--------\r\n");
}

#endif