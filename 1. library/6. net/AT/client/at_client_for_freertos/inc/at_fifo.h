#ifndef __AT_FIFO_H__
#define __AT_FIFO_H__


#ifdef __cplusplus
extern "C" {
#endif


struct _fifo{
  unsigned int in;
  unsigned int out;
  unsigned int size;
  unsigned int mask;
  void*        data;
};

typedef struct _fifo * fifo_queue_t;



fifo_queue_t fifo_queue_create(unsigned int bufsz);
inline void  fifo_queue_clear(fifo_queue_t fifo);
unsigned int fifo_queue_write(fifo_queue_t fifo, const void* src, unsigned int len);
unsigned int fifo_queue_read(fifo_queue_t fifo, void* buf, unsigned int len);
unsigned int fifo_queue_getc(fifo_queue_t fifo, void *buf);
unsigned int fifo_queue_putc(fifo_queue_t fifo, char c);

#ifdef __cplusplus
}
#endif

#endif