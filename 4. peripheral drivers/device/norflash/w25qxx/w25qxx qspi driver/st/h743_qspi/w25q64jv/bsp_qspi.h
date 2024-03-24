//
// Created by zjcszn on 2024/2/6.
//

#ifndef _BSP_QSPI_H_
#define _BSP_QSPI_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32h7xx_ll_mdma.h"

typedef struct {
    QSPI_HandleTypeDef hqspi;
    MDMA_HandleTypeDef hmdma;
    SemaphoreHandle_t status_match_sem;
    SemaphoreHandle_t qspi_tc;
} qspi_handle_t;

extern qspi_handle_t dev_qspi;

void bsp_qspi_init(void);
int bsp_qspi_wait_busy(void);
int bsp_qspi_wait_tc(void);

#endif //_BSP_QSPI_H_
