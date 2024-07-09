#ifndef __BSP_BEEP_H__
#define __BSP_BEEP_H__

#include "stdint.h"




#define BEEP_PORT   GPIOE
#define BEEP_PIN    GPIO_PIN_4

void bsp_beep_init(void);
void bsp_beep_task(void);
void bsp_beep_stop(void);
void bsp_beep_start(uint16_t beep_time, uint16_t stop_time, uint8_t cycle, uint16_t freq);


#endif