#ifndef __BOOT_OSAL_H__
#define __BOOT_OSAL_H__

#include <stdint.h>
#include "FreeRTOS.h"

#define  RTOS_EOK                        0
#define  RTOS_ENOMEM                     1
#define  RTOS_ETIMEOUT                   2

#define  RTOS_WAITING_NO                 0          /**< Non-block. */
#define  RTOS_WAITING_FOREVER            -1         /**< Block forever until get resource. */

#define  RTOS_FROM_ISR                   0          /**< Call from interrupt. */
#define  RTOS_FROM_THREAD                1          /**< Call from thread. */

#define  RTOS_MAX_PRIOR                  (configMAX_PRIORITIES - 1)


typedef  void*  rtos_sem_t;
typedef  void*  rtos_mutex_t;
typedef  void*  rtos_thread_t;
typedef  void (*rtos_entry_t)(void *args);      



extern   void*          rtos_malloc(uint32_t size);
extern   void*          rtos_calloc(uint32_t count, uint32_t size);
extern   void*          rtos_realloc(void *rmem, uint32_t newsize);
extern   void           rtos_free(void *rmem);
extern   rtos_mutex_t   rtos_mutex_create(void);
extern   int            rtos_mutex_take(rtos_mutex_t mutex, uint32_t timeout);
extern   int            rtos_mutex_give(rtos_mutex_t mutex);
extern   void           rtos_mutex_delete(rtos_sem_t sem);
extern   rtos_sem_t     rtos_sem_create(uint32_t max_count, uint32_t init_val);
extern   int            rtos_sem_take(rtos_sem_t sem, uint32_t timeout);
extern   int            rtos_sem_give(rtos_sem_t sem);
extern   void           rtos_sem_delete(rtos_sem_t sem);
extern   rtos_thread_t  rtos_thread_create(const char* name, uint32_t stk_size, uint32_t prio, rtos_entry_t entry, void* args);
extern   void           rtos_thread_delete(rtos_thread_t task_handle);
extern   void           rtos_thread_start(void);
extern   uint32_t       rtos_tick_get(void);
extern   uint32_t       rtos_tick_from_ms(uint32_t ms);

extern   void           rtos_delayms(uint32_t ms);




#endif