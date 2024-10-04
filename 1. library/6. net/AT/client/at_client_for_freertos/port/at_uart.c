#include "at_uart.h"
#include "at.h"
#include "dev_usart.h"

void at_device_open(uint32_t uart_idx)
{
  /* init uart port */
  // usart_device_init(DEV_USART2);
}

void at_device_write(uint32_t uart_idx, uint8_t *txbuf, uint32_t size)
{
  /* send data to uart */
  // usart_write(DEV_USART2, txbuf, size);
}

void at_device_read(uint32_t uart_idx, void *rxbuf, uint32_t size)
{
  /* recive data from uart and write to fifo buffer of AT device */
  at_client_rx_ind(uart_idx, rxbuf, size);
}


