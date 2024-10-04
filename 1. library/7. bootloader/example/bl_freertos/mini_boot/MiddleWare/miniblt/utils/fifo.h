#ifndef __FIFO_H__
#define __FIFO_H__

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _IO volatile

enum _fifo_err
{
    QUEUE_EOK,
    QUEUE_ENOMEM,
    QUEUE_EBUFSZ,
    QUEUE_EPARAM,
};
typedef enum _fifo_err fifo_err_t;


struct _fifo
{
_IO unsigned int in;
_IO unsigned int out;
    unsigned int size;
    unsigned int mask;
    void*        data;
};
typedef struct _fifo* fifo_queue_t;


static inline void fifo_queue_clear(fifo_queue_t fifo) 
{
    fifo->out = fifo->in;
}

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


extern fifo_queue_t fifo_queue_create(unsigned int bufsz);
extern int          fifo_queue_init(fifo_queue_t fifo, void *buf, unsigned int bufsz);
extern void         fifo_queue_delete(fifo_queue_t *fifo);
extern inline  void fifo_queue_clear(fifo_queue_t fifo);
extern unsigned int fifo_queue_write(fifo_queue_t fifo, const void* src, unsigned int len);
extern unsigned int fifo_queue_read(fifo_queue_t fifo, void *buf, unsigned int len);
extern unsigned int fifo_queue_getc(fifo_queue_t fifo, void *buf);
extern unsigned int fifo_queue_putc(fifo_queue_t fifo, char c);

#ifdef __cplusplus
}
#endif

#endif