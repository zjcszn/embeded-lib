#ifndef __FIFO_H__
#define __FIFO_H__


#ifdef __cplusplus
extern "C" {
#endif


#define QUEUE_EOK              0
#define QUEUE_ENOMEM           1
#define QUEUE_EBUFSZ           2
#define QUEUE_EPARAM           3



struct _fifo{
  unsigned int in;
  unsigned int out;
  unsigned int size;
  unsigned int mask;
  void*        data;
};
typedef struct _fifo * fifo_queue_t;



static inline void fifo_queue_reset(fifo_queue_t fifo) 
{
  fifo->in = fifo->out = 0;
}

static inline int fifo_queue_is_full(fifo_queue_t fifo) 
{
  return (fifo->size == (fifo->in - fifo->out));
}

static inline int fifo_queue_is_empty(fifo_queue_t fifo)
{
  return (fifo->in == fifo->out);
}


fifo_queue_t fifo_queue_create(unsigned int bufsz);
int          fifo_queue_init(fifo_queue_t fifo, void *buf, unsigned int bufsz);
void         fifo_queue_delete(fifo_queue_t *fifo);
inline  void fifo_queue_clear(fifo_queue_t fifo);
unsigned int fifo_queue_write(fifo_queue_t fifo, const void* src, unsigned int len);
unsigned int fifo_queue_read(fifo_queue_t fifo, void *buf, unsigned int len);
unsigned int fifo_queue_getc(fifo_queue_t fifo, void *buf);
unsigned int fifo_queue_putc(fifo_queue_t fifo, char c);

#ifdef __cplusplus
}
#endif

#endif