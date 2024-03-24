#include "at_fifo.h"
#include "at_osal.h"

#include <string.h>

static inline unsigned int fifo_queue_used(fifo_queue_t fifo);
static inline unsigned int fifo_queue_unused(fifo_queue_t fifo);
static inline unsigned int min(unsigned int x, unsigned int y);
static inline unsigned int roundup_pow_of_two(unsigned int x);


fifo_queue_t fifo_queue_create(unsigned int bufsz) 
{
  fifo_queue_t fifo;

  if (bufsz == 0) return NULL;

  fifo = (fifo_queue_t) rtos_calloc(1, sizeof(struct _fifo));
  if (fifo != NULL)
  {
    bufsz = roundup_pow_of_two(bufsz);
    fifo->data = rtos_malloc(bufsz);
    if (fifo->data != NULL) 
    {
      fifo->size = bufsz;
      fifo->mask = bufsz - 1;
      return fifo;
    }
    rtos_free(fifo);
  }
  return NULL;
}


unsigned int fifo_queue_write(fifo_queue_t fifo, const void* src, unsigned int len) 
{
  if ((len = min(len, fifo_queue_unused(fifo))) != 0)
  {
    unsigned int off, btw;

    off = fifo->in & fifo->mask;
    btw = min(len, fifo->size - off);

    memcpy(fifo->data + off, src, btw);
    memcpy(fifo->data, src + btw, len - btw);

    fifo->in += len;  
  }
  return len;
}

unsigned int fifo_queue_getc(fifo_queue_t fifo, void *buf) 
{
  if (fifo_queue_used(fifo) == 0) return 0;

  *((char *)buf) = ((char *)(fifo->data))[fifo->out & fifo->mask];

  fifo->out++;
  return 1;
}

unsigned int fifo_queue_putc(fifo_queue_t fifo, char c) 
{
  if (fifo_queue_unused(fifo) == 0) return 0;

  ((char *)fifo->data)[fifo->in & fifo->mask] = c;

  fifo->in++;
  return 1;
}

unsigned int fifo_queue_read(fifo_queue_t fifo, void* buf, unsigned int len) 
{
  if ((len = min(len, fifo_queue_used(fifo))) != 0)
  {
    unsigned int off, btr;

    off = fifo->out & fifo->mask;
    btr = min(len, fifo->size - off);

    memcpy(buf, fifo->data + off, btr);
    memcpy(buf + btr, fifo->data, len - btr);

    fifo->out += len;
  }
  return len;
}

inline void fifo_queue_clear(fifo_queue_t fifo) 
{
  fifo->out = fifo->in;
}

static inline unsigned int fifo_queue_used(fifo_queue_t fifo) 
{
  return (fifo->in - fifo->out);
}

static inline unsigned int fifo_queue_unused(fifo_queue_t fifo)
{
  return (fifo->size - fifo_queue_used(fifo));
}

static inline unsigned int min(unsigned int x, unsigned int y) 
{
  return (x < y ? x : y);
}

static inline unsigned int roundup_pow_of_two(unsigned int x)
{
  if (x & (x-1)) 
  {
    x--;
    x |= x >> 1;  x |= x >> 2;
    x |= x >> 4;  x |= x >> 8;
    x |= x >> 16;
    x++;
  }
  return x;
}


