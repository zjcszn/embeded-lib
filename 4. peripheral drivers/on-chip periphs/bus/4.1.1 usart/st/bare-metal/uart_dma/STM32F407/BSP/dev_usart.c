#include "dev_usart.h"


/******************************************** 静态全局变量  *********************************************/

#define __STATIC  static

// usart1 FIFO缓冲区 | DMA缓冲区
__STATIC uint8_t usart1_rx_buf[USART1_RX_BUF_SIZE];
__STATIC uint8_t usart1_dmarx_buf[USART1_RX_DMA_BUF_SIZE];
__STATIC uint8_t usart1_tx_buf[USART1_TX_BUF_SIZE];
__STATIC uint8_t usart1_dmatx_buf[USART1_TX_DMA_BUF_SIZE];

// usart2 FIFO缓冲区 | DMA缓冲区
__STATIC uint8_t usart2_rx_buf[USART2_RX_BUF_SIZE];
__STATIC uint8_t usart2_dmarx_buf[USART2_RX_DMA_BUF_SIZE];
__STATIC uint8_t usart2_tx_buf[USART2_TX_BUF_SIZE];
__STATIC uint8_t usart2_dmatx_buf[USART2_TX_DMA_BUF_SIZE];

// usart设备表
__STATIC UsartDevice_T s_usart_dev[DEV_USART_COUNT] = {0};

/********************************************** 函数声明  *********************************************/

static int      usart_dmatx_check_busy(uint8_t usart_id);
static uint16_t usart_dmarx_remain_size(uint8_t usart_id);

/********************************************** 函数定义  *********************************************/

/**
 * @brief 串口设备初始化
 * 
 * @param usart_id 
 */
void usart_device_init(uint8_t usart_id) {
  switch (usart_id) {
    case DEV_USART1:
      // 软件初始化
      fifo_init(&s_usart_dev[DEV_USART1].rx_fifo, usart1_rx_buf, USART1_RX_BUF_SIZE);
      fifo_init(&s_usart_dev[DEV_USART1].tx_fifo, usart1_tx_buf, USART1_TX_BUF_SIZE);
      s_usart_dev[DEV_USART1].dmarx_buf      = usart1_dmarx_buf;
      s_usart_dev[DEV_USART1].dmarx_buf_size = USART1_RX_DMA_BUF_SIZE;
      s_usart_dev[DEV_USART1].dmatx_buf      = usart1_dmatx_buf;
      s_usart_dev[DEV_USART1].dmatx_buf_size = USART1_TX_DMA_BUF_SIZE;
      s_usart_dev[DEV_USART1].dmarx_buf_read = 0;
      s_usart_dev[DEV_USART1].status         = USART_DMA_IDLE;
      // 硬件初始化
      bsp_usart1_init(usart1_dmarx_buf, USART1_RX_DMA_BUF_SIZE);
      break;

    case DEV_USART2:
      // 软件初始化
      fifo_init(&s_usart_dev[DEV_USART2].rx_fifo, usart2_rx_buf, USART2_RX_BUF_SIZE);
      fifo_init(&s_usart_dev[DEV_USART2].tx_fifo, usart2_tx_buf, USART2_TX_BUF_SIZE);
      s_usart_dev[DEV_USART2].dmarx_buf      = usart2_dmarx_buf;
      s_usart_dev[DEV_USART2].dmarx_buf_size = USART2_RX_DMA_BUF_SIZE;
      s_usart_dev[DEV_USART2].dmatx_buf      = usart2_dmatx_buf;
      s_usart_dev[DEV_USART2].dmatx_buf_size = USART2_TX_DMA_BUF_SIZE;
      s_usart_dev[DEV_USART2].dmarx_buf_read = 0;
      s_usart_dev[DEV_USART2].status         = USART_DMA_IDLE;
      // 硬件初始化
      bsp_usart2_init(usart2_dmarx_buf, USART2_RX_DMA_BUF_SIZE);
      break;

    default:
      break;
  }
}

/**
 * @brief 串口发送函数：将待发送数据写入TX FIFO
 * 
 * @param usart_id 串口设备号
 * @param src_buf 源数据地址
 * @param size 源数据大小
 * @return uint16_t 成功写入的字节数
 */
uint16_t usart_write(uint8_t usart_id, const uint8_t *src_buf, uint16_t size) {
  return fifo_write(&s_usart_dev[usart_id].tx_fifo, src_buf, size);
}

/**
 * @brief 串口接收函数：从RX FIFO中读取接收到的串口数据
 * 
 * @param usart_id 串口设备号
 * @param dst_buf 目标缓冲区
 * @param size 读取的字节数
 * @return uint16_t 成功读取的字节数
 */
uint16_t usart_read(uint8_t usart_id, uint8_t *dst_buf, uint16_t size) {
  return fifo_read(&s_usart_dev[usart_id].rx_fifo, dst_buf, size);
}

/****************************** USART RX: DMA半满 | DMA 完成 | 串口空闲中断处理  ******************************/

/**
 * @brief DMA 接收缓冲区半满中断处理程序
 * 
 * @param usart_id 串口设备号
 */
void usart_dmarx_ht_isr(uint8_t usart_id) {
  UsartDevice_T *usart_x;
  uint16_t       recv_size;
  usart_x   = &s_usart_dev[usart_id];
  recv_size = usart_x->dmarx_buf_size - (usart_dmarx_remain_size(usart_id) + usart_x->dmarx_buf_read);
  fifo_write(&usart_x->rx_fifo, usart_x->dmarx_buf + usart_x->dmarx_buf_read, recv_size);
  usart_x->dmarx_buf_read += recv_size;
}

/**
 * @brief DMA 接收缓冲区全满中断处理程序
 * 
 * @param usart_id 串口设备号
 */
void usart_dmarx_tc_isr(uint8_t usart_id) {
  UsartDevice_T *usart_x;
  uint16_t       recv_size;
  usart_x   = &s_usart_dev[usart_id]; 
  recv_size = usart_x->dmarx_buf_size - usart_x->dmarx_buf_read;
  fifo_write(&usart_x->rx_fifo, usart_x->dmarx_buf + usart_x->dmarx_buf_read, recv_size);
  usart_x->dmarx_buf_read = 0;
}

/**
 * @brief 串口空闲中断处理程序
 * 
 * @param usart_id 串口设备号
 */
void usart_dmarx_idle_isr(uint8_t usart_id) {
  UsartDevice_T *usart_x;
  uint16_t       recv_size;
  usart_x   = &s_usart_dev[usart_id];
  recv_size = usart_x->dmarx_buf_size - (usart_dmarx_remain_size(usart_id) + usart_x->dmarx_buf_read);
  fifo_write(&usart_x->rx_fifo, usart_x->dmarx_buf + usart_x->dmarx_buf_read, recv_size);
  usart_x->dmarx_buf_read += recv_size;
}

/**
 * @brief 获取串口接收DMA缓冲区剩余空间
 * 
 * @param usart_id 串口设备号
 * @return uint16_t 串口接收DMA缓冲区剩余空间
 */
static uint16_t usart_dmarx_remain_size(uint8_t usart_id) {
  switch (usart_id) {
    case DEV_USART1:
      return bsp_usart1_dmarx_buf_remain_size();
      break;
    case DEV_USART2:
      return bsp_usart2_dmarx_buf_remain_size();
      break;   
    default:
      return 0U;
      break;
  }
}

/******************************* USART TX: 串口发送完成中断 | 发送DMA使能 | 发送轮询 ******************************/

/**
 * @brief 串口发送DMA 发送完成中断处理程序
 * 
 * @param usart_id 串口设备号
 */
void usart_dmatx_tc_isr(uint8_t usart_id) {
  if (usart_id == DEV_USART1) {
    while (LL_USART_IsActiveFlag_TC(USART1));
    LL_DMA_DisableStream(USART1_TX_DMA, USART1_TX_DMA_STREAM);
    LL_USART_DisableDMAReq_TX(USART1);
  }
  else if (usart_id == DEV_USART2) {
    while (LL_USART_IsActiveFlag_TC(USART2));
    LL_DMA_DisableStream(USART2_TX_DMA, USART2_TX_DMA_STREAM);
    LL_USART_DisableDMAReq_TX(USART2);
  }
  s_usart_dev[usart_id].status = USART_DMA_IDLE;
}

/**
 * @brief 串口发送轮询程序
 * 
 * @param usart_id 串口设备号
 */
void usart_dmatx_poll(uint8_t usart_id) {
  UsartDevice_T *usart_x;
  uint16_t       trans_size;

  usart_x = &s_usart_dev[usart_id];
  if (usart_dmatx_check_busy(usart_id)) {
    return;
  }
  trans_size = fifo_read(&usart_x->tx_fifo, usart_x->dmatx_buf, usart_x->dmatx_buf_size);
  if (trans_size) {
    usart_x->status = USART_DMA_BUSY;   // DMA 发送状态
    switch (usart_id) {
      case DEV_USART1:
        bsp_usart1_dmatx_config(usart_x->dmatx_buf, trans_size);
        break;

      case DEV_USART2:
        bsp_usart2_dmatx_config(usart_x->dmatx_buf, trans_size);
        break;

      default:
        break;
    }
  }
}

static int usart_dmatx_check_busy(uint8_t usart_id) {
  switch (usart_id) {
    case DEV_USART1:
      return (s_usart_dev[usart_id].status || !LL_USART_IsActiveFlag_TC(USART1));
      break;

    case DEV_USART2:
      return (s_usart_dev[usart_id].status || !LL_USART_IsActiveFlag_TC(USART2));
      break;

    default:
      return (1U);

  }
}

