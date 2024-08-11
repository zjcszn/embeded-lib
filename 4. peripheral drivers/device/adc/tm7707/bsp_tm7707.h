#ifndef __BSP_TM7707_H__
#define __BSP_TM7707_H__

#include "stdint.h"

#define TM7707_MCLK_TIMER           TIM1
#define TM7707_MCLK_TIMER_CHANNEL   LL_TIM_CHANNEL_CH2
#define TM7707_MCLK_PORT            GPIOA
#define TM7707_MCLK_PIN             LL_GPIO_PIN_9




void bsp_InitTM7707(void);
void tm7707_calibself(uint8_t _ch);
uint32_t tm7707_read_register(uint8_t _RegID);
uint32_t TM7707_ReadAdc(uint8_t _ch);
void tm7707_write_register(uint8_t _RegID, uint32_t _RegValue);


#endif