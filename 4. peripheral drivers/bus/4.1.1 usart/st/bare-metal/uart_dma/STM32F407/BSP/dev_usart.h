#ifndef __DEV_USART_H__
#define __DEV_USART_H__

#include "bsp_usart.h"
#include "fifo.h"

#ifdef __cplusplus
extern C{
#endif

#define USART1_RX_BUF_SIZE              (1024U)
#define USART1_TX_BUF_SIZE              (1024U)
#define USART1_RX_DMA_BUF_SIZE          ( 128U)
#define USART1_TX_DMA_BUF_SIZE          ( 128U)

#define USART2_RX_BUF_SIZE              (1024U)
#define USART2_TX_BUF_SIZE              (1024U)
#define USART2_RX_DMA_BUF_SIZE          ( 128U)
#define USART2_TX_DMA_BUF_SIZE          ( 128U)

enum ENUM_UsartDev {
  DEV_USART1,
  DEV_USART2,
  DEV_USART_COUNT,
};

enum ENUM_UsartStatus {
  USART_DMA_IDLE,
  USART_DMA_BUSY,
};

// 串口设备结构体
typedef struct {
  FIFO_TypeDef  rx_fifo;          // rx fifo缓冲区
  FIFO_TypeDef  tx_fifo;          // tx fifo缓冲区
  uint8_t      *dmarx_buf;        // rx DMA缓冲区
  uint8_t      *dmatx_buf;        // tx DMA缓冲区
  uint16_t      dmarx_buf_size;   // rx DMA缓冲区大小
  uint16_t      dmatx_buf_size;   // tx DMA缓冲区大小
  uint16_t      dmarx_buf_read;   // rx DMA缓冲区读指针
  uint8_t       status;
}UsartDevice_T;

/********************************************** 接口  **********************************************/

extern void     usart_device_init(uint8_t usart_id);
extern uint16_t usart_write(uint8_t usart_id, const uint8_t *src_buf, uint16_t size);
extern uint16_t usart_read(uint8_t usart_id, uint8_t *dst_buf, uint16_t size);
extern void     usart_dmarx_ht_isr(uint8_t usart_id);
extern void     usart_dmarx_tc_isr(uint8_t usart_id);
extern void     usart_dmarx_idle_isr(uint8_t usart_id);
extern void     usart_dmatx_tc_isr(uint8_t usart_id);
extern void     usart_dmatx_poll(uint8_t usart_id);


#ifdef __cplusplus
}
#endif

#endif