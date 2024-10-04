//
// Created by zjcszn on 2024/1/26.
//

#ifndef _BSP_UBLOX_H_
#define _BSP_UBLOX_H_

#include "stdint.h"

#define UBLOX_BUFFER_SIZE       128

void bsp_ublox_reset(void);
void bsp_ublox_init(void);
void bsp_ublox_rate_set(uint8_t freq);

#endif //_BSP_UBLOX_H_
