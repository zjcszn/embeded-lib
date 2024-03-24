#ifndef __DEV_USART_H__
#define __DEV_USART_H__

#include "bsp_usart.h"
#include "fifo.h"

#ifdef __cplusplus
extern C{
#endif

/********************************************** 宏定义 **********************************************/

#define USART1_RX_BUF_SIZE              (1024U)     // 串口1 RX缓冲区大小
#define USART1_TX_BUF_SIZE              (1024U)     // 串口1 TX缓冲区大小

#define USART2_RX_BUF_SIZE              (1024U)     // 串口2 RX缓冲区大小
#define USART2_TX_BUF_SIZE              (1024U)     // 串口2 TX缓冲区大小

/********************************************* 类型声明 *********************************************/

// 串口设备号
enum ENUM_UsartDev {
  DEV_USART1,
  DEV_USART2,
  DEV_USART_COUNT,
};

// 串口设备结构体
typedef struct {
  USART_TypeDef *usart;            // 串口结构体指针
  FIFO_TypeDef   rx_fifo;          // rx fifo缓冲区
  FIFO_TypeDef   tx_fifo;          // tx fifo缓冲区
}UsartDevice_T;

/********************************************* 外部接口  *********************************************/

extern void     usart_device_init(uint8_t usart_id);
extern uint16_t usart_write(uint8_t usart_id, const uint8_t *src_buf, uint16_t size);
extern uint16_t usart_read(uint8_t usart_id, uint8_t *dst_buf, uint16_t size);
extern uint16_t usart_write_byte(uint8_t usart_id, const uint8_t *src_buf);
extern uint16_t usart_read_byte(uint8_t usart_id, uint8_t *dst_buf);

extern void     usart_rxne_isr(uint8_t usart_id);
extern void     usart_txe_isr(uint8_t usart_id);
extern void     usart_tc_isr (uint8_t usart_id);

#ifdef __cplusplus
}
#endif

#endif