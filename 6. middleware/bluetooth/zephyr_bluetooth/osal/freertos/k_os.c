#include <zephyr.h>
#include <FreeRTOS.h>
#include <semphr.h>

#include <common/log.h>

int k_sem_init(struct k_sem *sem, unsigned int initial_count,
               unsigned int limit)
{
    sem->sem = xSemaphoreCreateCounting(limit, initial_count);
    BT_INFO("sem:%p, name:%s", sem->sem, sem->name);
    sys_dlist_init(&sem->poll_events);
    return 0;
}

int k_sem_take(struct k_sem *sem, k_timeout_t timeout)
{
    BaseType_t ret;
    unsigned int t = timeout;

    __ASSERT(sem && sem->sem, "sem :%s is NULL\r\n", sem->name);

    if (timeout == K_FOREVER) {
        t = portMAX_DELAY;
    } else if (timeout == K_NO_WAIT) {
        t = 0;
    }

    ret = xSemaphoreTake(sem->sem, pdMS_TO_TICKS(t));
    return ret == pdPASS ? 0 : -1;
}

void k_sem_give(struct k_sem *sem)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int ret;

    __ASSERT(sem && sem->sem, "sem :%s is NULL\r\n", sem->name);

    if (xPortIsInsideInterrupt()) {
        ret = xSemaphoreGiveFromISR((SemaphoreHandle_t)sem->sem, &xHigherPriorityTaskWoken);
        if (ret == pdPASS) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    } else {
        xSemaphoreGive((SemaphoreHandle_t)sem->sem);
    }
}

void k_sem_reset(struct k_sem *sem)
{
    xQueueReset((QueueHandle_t)sem->sem);
}

unsigned int k_sem_count_get(struct k_sem *sem)
{
    if (NULL == sem || NULL == sem->sem) {
        return 0;
    }

    return uxQueueMessagesWaiting((QueueHandle_t)sem->sem);
}

k_tid_t k_thread_create(const char *name,
                        k_thread_stack_t *stack,
                        size_t stack_size,
                        k_thread_entry_t entry,
                        void *args,
                        int prio)
{
    TaskHandle_t htask = NULL;
    stack_size /= sizeof(StackType_t);
    xTaskCreate(entry, name, stack_size, args, prio, &htask);
    return (k_tid_t)htask;
}

void k_thread_abort(k_tid_t thread)
{
    vTaskDelete(thread);
}

void k_yield(void)
{
    taskYIELD();
}

k_tid_t k_current_get(void)
{
    return xTaskGetCurrentTaskHandle();
}

unsigned int irq_lock(void)
{
    taskENTER_CRITICAL();
    return 1;
}

void irq_unlock(unsigned int key)
{
    taskEXIT_CRITICAL();
}

int64_t k_uptime_get(void)
{
    return (int64_t)xTaskGetTickCount() * 1000 / configTICK_RATE_HZ;
}

bool k_is_in_isr(void)
{
    return xPortIsInsideInterrupt();
}

void k_sleep(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}