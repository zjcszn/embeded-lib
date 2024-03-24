//
// Created by zjcszn on 2024/1/8.
//

#ifndef _BSP_TOUCHKEY_H_
#define _BSP_TOUCHKEY_H_

/*
 * touchpad io: PA5
 * Charge Time Measurement Demo
 * */

#define TPAD_PORT   GPIOA
#define TPAD_PIN    LL_GPIO_PIN_5

void bsp_touchkey_init(void);

#endif //_BSP_TOUCHKEY_H_
