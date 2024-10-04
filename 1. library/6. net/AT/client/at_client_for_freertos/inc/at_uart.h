#ifndef __AT_UART_H__
#define __AT_UART_H__


#include "at_def.h"


#define   AT_UART_00            0
#define   AT_UART_01            1
#define   AT_UART_02            2



extern  void at_device_open(uint32_t uart_idx);
extern  void at_device_write(uint32_t uart_idx, uint8_t *txbuf, uint32_t size);
extern  void at_device_read(uint32_t uart_idx, void *rxbuf, uint32_t size);


#endif