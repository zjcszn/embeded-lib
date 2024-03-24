/**
 * @File:    fifo.c
 * @Author:  zjcszn
 * @Date:    2022-09-25
 * 
 * Change logs:
 * Date        Author       Notes
 * 2022-09-25  zjcszn       First add
 * 
*/

#include "fifo.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define min(x,y) ((x) < (y) ? (x) : (y))

#define ROUND_UP 1    // 向上调整为2的N次幂
#define ROUND_DN 0    // 向下调整为2的N次幂

#if (USE_FIFO_ASSERT == 1)
  #define assert_fifo(expr) ((expr) ? (void)0 : assert_fifo_failed((uint8_t *)__FILE__, __LINE__))
  #define assert_fifo_failed(file, line) \
          do {  \
            printf("assert failed: file %s on line %d\r\n", file, line); \
          } while(0)
#else
  #define assert_fifo(expr) ((void)0UL)
#endif

/**
 * @brief 检查fifo缓冲区大小是否是2的N次幂
 * 
 * @param size 缓冲区大小
 * @return 返回1：不是2的N次幂 ; 返回0：是2的N次幂 
 */
static inline int check_fifo_size(fifo_t size) {
  return (size & (size - 1));
}

static fifo_t round_fifo_size(fifo_t size, int mode) {
  assert_fifo(mode == ROUND_UP || mode == ROUND_DN);
  if (!size) return ((fifo_t)1UL);

  fifo_t  position = 0;
  while (size != 0) {
    position ++;
    size = size >> 1UL;    
  }
  position = mode ? position : position - 1;
  // 如果溢出，返回可表示的最大的2的N次幂 
  if (position >= (sizeof(fifo_t) << 3UL)) {
    position = position - 1;
  }
  return ((fifo_t)1UL << position);
}

/**
 * @brief fifo静态初始化：将一个缓冲区初始化为fifo队列
 * 
 * @param fifo 
 * @param buffer_addr 
 * @param buffer_size 
 * @return int 
 */
int fifo_init(FIFO_TypeDef *fifo, uint8_t *buffer_addr, fifo_t buffer_size) {
  assert_fifo(fifo);
  if (check_fifo_size(buffer_size)) {
    buffer_size = round_fifo_size(buffer_size, ROUND_DN);
  }
  if (buffer_size < 2) return -EINVAL;

  fifo->buf  = buffer_addr;
  fifo->size = buffer_size;
  fifo->mask = buffer_size - 1;

  fifo_reset(fifo);
  return 0;
}

/**
 * @brief fifo动态初始化：申请一个空间初始化为fifo队列
 * 
 * @param fifo 
 * @param buffer_size 
 * @return int 
 */
int fifo_alloc(FIFO_TypeDef *fifo, fifo_t buffer_size) {
  assert_fifo(fifo);
  if (check_fifo_size(buffer_size)) {
    buffer_size = round_fifo_size(buffer_size, ROUND_UP);
  }
  if (buffer_size < 2) return EINVAL;

  fifo->buf = (uint8_t *)malloc(sizeof(uint8_t) * buffer_size);
  if (fifo->buf == NULL) return ENOMEM;
  fifo->size = buffer_size;
  fifo->mask = buffer_size - 1;
  
  fifo_reset(fifo);
  return 0;
}

/**
 * @brief fifo入队函数：将len个字节数据入队
 * 
 * @param fifo 
 * @param src_buf 
 * @param len 
 * @return fifo_t 
 */
fifo_t fifo_write(FIFO_TypeDef *fifo, const uint8_t *src_buf, fifo_t len) {
  assert_fifo(fifo);
  if (fifo_is_full(fifo)) return 0;

  fifo_t  l;
  len = min(len, fifo->size - fifo->in + fifo->out);
  l = min(len, fifo->size - (fifo->in & fifo->mask));

  memcpy(fifo->buf + (fifo->in & fifo->mask), src_buf, l);
  memcpy(fifo->buf, src_buf + l, len - l);

  barrier_fifo();
  fifo->in += len;
  return len;
}

/**
 * @brief fifo出队函数：将len个字节数据出队
 * 
 * @param fifo 
 * @param dst_buf 
 * @param len 
 * @return fifo_t 
 */
fifo_t fifo_read(FIFO_TypeDef *fifo, uint8_t *dst_buf, fifo_t len) {
  assert_fifo(fifo);
  if (fifo_is_empty(fifo)) return 0;

  fifo_t  l;
  len = min(len, fifo->in - fifo->out);
  l = min(len, fifo->size - (fifo->out & fifo->mask));

  memcpy(dst_buf, fifo->buf + (fifo->out & fifo->mask), l);
  memcpy(dst_buf + l, fifo->buf, len - l);

  barrier_fifo();
  fifo->out += len;
  return len;
}

/**
 * @brief fifo出队函数：将len个字节数据出队，但不会更新out指针
 * 
 * @param fifo 
 * @param dst_buf 
 * @param len 
 * @return fifo_t 
 */
fifo_t fifo_read_peek(FIFO_TypeDef *fifo, uint8_t *dst_buf, fifo_t  len) {
  assert_fifo(fifo);
  if (fifo_is_empty(fifo)) return 0;

  fifo_t  l;
  len = min(len, fifo->in - fifo->out);
  l = min(len, fifo->size - (fifo->out & fifo->mask));

  memcpy(dst_buf, fifo->buf + (fifo->out & fifo->mask), l);
  memcpy(dst_buf + l, fifo->buf, len - l);

  return len;
}

/**
 * @brief fifo入队函数：将1个字节数据入队
 * 
 * @param fifo 
 * @param c 
 * @return fifo_t 
 */
fifo_t fifo_write_byte(FIFO_TypeDef *fifo, uint8_t c) {
  assert_fifo(fifo);
  if (fifo_is_full(fifo)) return 0;

  fifo->buf[fifo->in & fifo->mask] = c;
  
  barrier_fifo();
  fifo->in++;
  return 1;
}

/**
 * @brief fifo出队函数：将1个字节数据出队
 * 
 * @param fifo 
 * @param c 
 * @return fifo_t 
 */
fifo_t fifo_read_byte(FIFO_TypeDef *fifo, uint8_t *c) {
  assert_fifo(fifo);
  if (fifo_is_empty(fifo)) return 0;

  *c = fifo->buf[fifo->out & fifo->mask];
  
  barrier_fifo();
  fifo->out++;
  return 1;
}

/**
 * @brief fifo出队函数：将1个字节数据出队，但不会更新out指针
 * 
 * @param fifo 
 * @param c 
 * @return fifo_t 
 */
fifo_t fifo_read_byte_peek(FIFO_TypeDef *fifo, uint8_t *c) {
  assert_fifo(fifo);
  if (fifo_is_empty(fifo)) return 0;

  *c = fifo->buf[fifo->out & fifo->mask];
  
  return 1;
}



