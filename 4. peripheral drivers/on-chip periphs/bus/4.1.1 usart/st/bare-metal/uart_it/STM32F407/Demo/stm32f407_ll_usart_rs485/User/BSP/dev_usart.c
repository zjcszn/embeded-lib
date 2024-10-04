#include "dev_usart.h"


/******************************************** 静态全局变量  *********************************************/

#define __STATIC static

// USART1 FIFO缓冲区
__STATIC uint8_t usart1_rx_buf[USART1_RX_BUF_SIZE];
__STATIC uint8_t usart1_tx_buf[USART1_TX_BUF_SIZE];

// USART2 FIFO缓冲区
__STATIC uint8_t usart2_rx_buf[USART2_RX_BUF_SIZE];
__STATIC uint8_t usart2_tx_buf[USART2_TX_BUF_SIZE];

// 串口设备表
__STATIC UsartDevice_T s_usart_dev[DEV_USART_COUNT] = {0};

/********************************************** 函数声明  *********************************************/


/********************************************** 函数定义  *********************************************/

/**
 * @brief 串口设备初始化
 * 
 * @param usart_id 串口设备号 DEV_USART1 | DEV_USART2
 */
void usart_device_init(uint8_t usart_id) {
  switch (usart_id) {
    case DEV_USART1:
      // 软件初始化
      s_usart_dev[DEV_USART1].usart = USART1;
      fifo_init(&s_usart_dev[DEV_USART1].rx_fifo, usart1_rx_buf, USART1_RX_BUF_SIZE);
      fifo_init(&s_usart_dev[DEV_USART1].tx_fifo, usart1_tx_buf, USART1_TX_BUF_SIZE);
      // 硬件初始化
      bsp_usart1_init();
      break;

    case DEV_USART2:		
      // 软件初始化
      s_usart_dev[DEV_USART2].usart = USART2;
      fifo_init(&s_usart_dev[DEV_USART2].rx_fifo, usart2_rx_buf, USART2_RX_BUF_SIZE);
      fifo_init(&s_usart_dev[DEV_USART2].tx_fifo, usart2_tx_buf, USART2_TX_BUF_SIZE);
      // 硬件初始化
      bsp_usart2_init();
      break;

    default:
      break;
  }
	
}

/**
 * @brief 串口发送函数：将待发送数据写入发送FIFO缓冲区
 * 
 * @param usart_id 串口设备号 DEV_USART1 | DEV_USART2
 * @param src_buf 源数据地址
 * @param size 源数据字节数
 * @return uint16_t 成功写入的字节数
 */
uint16_t usart_write(uint8_t usart_id, const uint8_t *src_buf, uint16_t size) {
  uint16_t wr_cnt; 
  wr_cnt = fifo_write(&s_usart_dev[usart_id].tx_fifo, src_buf, size);
  if (wr_cnt) {
    if (usart_id == DEV_USART2) {
      USART2_RS485_TX();    // 将RS485控制器切换到发送模式
    }
    LL_USART_EnableIT_TXE(s_usart_dev[usart_id].usart);
  }
  return wr_cnt;
}

/**
 * @brief 串口字节发送函数：将一个字节数据写入发送FIFO缓冲区
 * 
 * @param usart_id 串口设备号 DEV_USART1 | DEV_USART2
 * @param src_buf 源数据地址
 * @return uint16_t 成功写入的字节数
 */
uint16_t usart_write_byte(uint8_t usart_id, const uint8_t *src_buf) {
  uint16_t wr_cnt; 
  wr_cnt = fifo_write_byte(&s_usart_dev[usart_id].tx_fifo, *src_buf);
  if (wr_cnt) {
    if (usart_id == DEV_USART2) {
      USART2_RS485_TX();    // 将RS485控制器切换到发送模式
    }
    LL_USART_EnableIT_TXE(s_usart_dev[usart_id].usart);
  }
  return wr_cnt;
}

/**
 * @brief 串口接收数据读取函数：从接收FIFO缓冲区中读取数据
 * 
 * @param usart_id 串口设备号
 * @param dst_buf 目标缓冲区
 * @param size 待读取的字节数
 * @return uint16_t 成功读取的字节数
 */
uint16_t usart_read(uint8_t usart_id, uint8_t *dst_buf, uint16_t size) {
  return fifo_read(&s_usart_dev[usart_id].rx_fifo, dst_buf, size);
}

/**
 * @brief 串口接收数据字节读取函数：从接收FIFO缓冲区中读取一个字节数据
 * 
 * @param usart_id 串口设备号
 * @param dst_buf 目标缓冲区
 * @return uint16_t 成功读取的字节数
 */
uint16_t usart_read_byte(uint8_t usart_id, uint8_t *dst_buf) {
  return fifo_read_byte(&s_usart_dev[usart_id].rx_fifo, dst_buf);
}

/************************************* USART RX: 串口RX非空中断处理  *************************************/

/**
 * @brief 串口RX非空中断处理程序
 * 
 * @param usart_id 串口设备号
 */
void usart_rxne_isr(uint8_t usart_id) {
  fifo_write_byte(&s_usart_dev[usart_id].rx_fifo, LL_USART_ReceiveData8(s_usart_dev[usart_id].usart));
}

/******************************** USART TX: 串口发送缓冲空中断 | 发送完成中断 *******************************/

/**
 * @brief 串口发送缓冲空中断
 * 
 * @param usart_id 串口设备号
 */
void usart_txe_isr(uint8_t usart_id) {
  uint8_t        tx_tmp;
  UsartDevice_T *usart_x = &s_usart_dev[usart_id];

  if (fifo_len(&usart_x->tx_fifo)) {
    fifo_read_byte(&usart_x->tx_fifo, &tx_tmp);
    LL_USART_TransmitData8(usart_x->usart, tx_tmp);
  }
  else {
    LL_USART_DisableIT_TXE(usart_x->usart);
    LL_USART_ClearFlag_TC(usart_x->usart);
    LL_USART_EnableIT_TC(usart_x->usart);
  }
}

/**
 * @brief 串口发送完成中断处理程序
 * 
 * @param usart_id 串口设备号
 */
void usart_tc_isr (uint8_t usart_id) {
  if(usart_id == DEV_USART2) {
    USART2_RS485_RX();    // 将RS485控制器切换至接收模式
  }
  LL_USART_DisableIT_TC(s_usart_dev[usart_id].usart);
}





