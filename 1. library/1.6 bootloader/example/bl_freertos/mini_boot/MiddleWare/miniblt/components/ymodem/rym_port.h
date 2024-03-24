#ifndef __RYM_PORT_H__
#define __RYM_PORT_H__


#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"


typedef  void*   rym_semp_t;

/* macro wrapper of rtos interface */
#define  rym_ticks_from_ms(ms)      pdMS_TO_TICKS(ms)
#define  rym_delayms(ms)            vTaskDelay(pdMS_TO_TICKS(ms))
#define  rym_get_ticks()            xTaskGetTickCount()
#define  rym_malloc(size)           pvPortMalloc(size)
#define  rym_calloc(num,size)       pvPortCalloc(num,size)
#define  rym_free(ptr)              vPortFree(ptr)
#define  rym_enter_critical()       taskENTER_CRITICAL()
#define  rym_exit_critical()        taskEXIT_CRITICAL()

/* communication interface of serial device */
uint32_t rym_serial_write(uint8_t dev_id, const void* src, uint32_t len);
uint32_t rym_serial_read(uint8_t dev_id, void* dst, uint32_t len);
bool     rym_serial_lock(uint8_t dev_id);
bool     rym_serial_unlock(uint8_t dev_id);
void     rym_serial_clear(uint8_t dev_id);


/* wrapper of rtos semaphore */
bool  rym_semp_take(rym_semp_t semp, uint32_t block_time);
bool  rym_semp_give(rym_semp_t semp);
bool  rym_semp_create(rym_semp_t *semp);
void  rym_semp_delete(rym_semp_t *semp);

/* notify rym thread when serial device recive data */
void  rym_rx_notify(void);

#endif