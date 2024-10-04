/**
 * @File:    fifo.h
 * @Author:  zjcszn
 * @Date:    2022-09-25
 * 
 * Change logs:
 * Date        Author       Notes
 * 2022-09-25  zjcszn       First add
 * 
*/

#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USE_FIFO_LOCK         0      // 使用互斥锁
#define USE_FIFO_BARRIER      0      // 使用内存屏障
#define USE_FIFO_ASSERT       0      // 使用assert宏


#ifndef __IO
#define __IO volatile
#endif

#if (USE_FIFO_BARRIER  ==  1)
  #if defined (__CORTEX_M)
    #include "cmsis_compiler.h"
    #define barrier_fifo() __DSB()    // ARM数据同步屏障
  #elif defined (__GNUC__)
    #define barrier_fifo() __sync_synchronize()
  #else
    #define barrier_fifo() ((void)0U)
  #endif
#else
  #define barrier_fifo() ((void)0U)
#endif

typedef uint32_t fifo_t ;

typedef struct _fifo{
        uint8_t *buf;   // 环形缓冲区地址
        fifo_t  size;   // 缓冲区大小
        fifo_t  mask;   // 下标掩码
  __IO  fifo_t  in;     // 写指针
  __IO  fifo_t  out;    // 读指针
}FIFO_TypeDef;

// 重置fifo
static inline void fifo_reset(FIFO_TypeDef *fifo) {
  fifo->in = fifo->out = 0;
}

// 判断fifo缓冲区是否为空
static inline int fifo_is_empty(FIFO_TypeDef *fifo) {
  barrier_fifo();
  return (fifo->in == fifo->out);
}

// fifo缓冲区中已写入的字节长度
static inline fifo_t  fifo_len(FIFO_TypeDef *fifo) {
  barrier_fifo();
  return (fifo->in - fifo->out);
}

// 判断fifo缓冲区是否已满
static inline int fifo_is_full(FIFO_TypeDef *fifo) {
  barrier_fifo();
  return (fifo->size == (fifo->in - fifo->out));
}

int    fifo_init(FIFO_TypeDef *fifo, uint8_t *buffer_addr, fifo_t  buffer_size);
int    fifo_alloc(FIFO_TypeDef *fifo, fifo_t  buffer_size);
fifo_t fifo_write(FIFO_TypeDef *fifo, const uint8_t *src_buf, fifo_t  len);
fifo_t fifo_read(FIFO_TypeDef *fifo, uint8_t *dst_buf, fifo_t  len);
fifo_t fifo_read_peek(FIFO_TypeDef *fifo, uint8_t *dst_buf, fifo_t  len);
fifo_t fifo_write_byte(FIFO_TypeDef *fifo, uint8_t c);
fifo_t fifo_read_byte(FIFO_TypeDef *fifo, uint8_t *c);
fifo_t fifo_read_byte_peek(FIFO_TypeDef *fifo, uint8_t *c);

#ifdef __cplusplus
}
#endif

#endif