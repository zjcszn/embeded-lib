#include "rym_port.h"
#include "dev_uart.h"



/* ==================== serial device interface ==================== */

uint32_t rym_serial_write(uint8_t dev_id, const void* src, uint32_t len)
{
    (void)dev_id;
    return uart_write(src, len);
}

uint32_t rym_serial_read(uint8_t dev_id, void* dst, uint32_t len)
{
    (void)dev_id;
    return uart_read(dst, len);
}

bool rym_serial_lock(uint8_t dev_id)
{
    /* lock serial device */
}

bool rym_serial_unlock(uint8_t dev_id)
{
    /* unlock serial device */
}

void rym_serial_clear(uint8_t dev_id)
{
    /* clear serial device rx buffer */
}

/* =================== rtos semaphore interface ==================== */

bool rym_semp_create(rym_semp_t *semp)
{
    *semp = xSemaphoreCreateBinary();
    return (*semp != NULL);
}

void rym_semp_delete(rym_semp_t *semp)
{
    vSemaphoreDelete(*semp);
    semp = NULL;
}

bool rym_semp_take(rym_semp_t semp, uint32_t block_time)
{
    return (xSemaphoreTake(semp, pdMS_TO_TICKS(block_time)) == pdTRUE);
}

bool rym_semp_give(rym_semp_t semp)
{
    return (xSemaphoreGiveFromISR(semp, NULL) == pdTRUE);
}



