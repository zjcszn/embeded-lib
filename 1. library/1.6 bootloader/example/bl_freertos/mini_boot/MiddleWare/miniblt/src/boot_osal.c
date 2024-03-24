#include "boot_osal.h"

#include "FreeRTOS.h"
#include "portmacro.h"
#include "semphr.h"
#include "task.h"


/**
 * @brief Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @param size is the minimum size of the requested block in bytes.
 *
 * @return the pointer to allocated memory or NULL if no free memory was found.
 */
void *rtos_malloc(uint32_t size) {
    return pvPortMalloc(size);
}

/**
 * @brief  This function will contiguously allocate enough space for count objects
 *         that are size bytes of memory each and returns a pointer to the allocated
 *         memory.
 *
 * @note   The allocated memory is filled with bytes of value zero.
 *
 * @param  count is the number of objects to allocate.
 *
 * @param  size is the size of one object to allocate.
 *
 * @return pointer to allocated memory / NULL pointer if there is an error.
 */
void *rtos_calloc(uint32_t count, uint32_t size) {
    return pvPortCalloc(count, size);
}

/**
 * @brief This function will change the size of previously allocated memory block.
 *
 * @param rmem is the pointer to memory allocated by rt_malloc.
 *
 * @param newsize is the required new size.
 *
 * @return the changed memory block address.
 */
void *rtos_realloc(void *rmem, uint32_t newsize) {
    return pvPortRealloc(rmem, newsize);
}

/**
 * @brief This function will release the previously allocated memory block by
 *        rt_malloc. The released memory block is taken back to system heap.
 *
 * @param rmem the address of memory which will be released.
 */
void rtos_free(void *rmem) {
    vPortFree(rmem);
}

/**
 * @brief    This function will create a mutex object.
 *
 * @note     For the mutex object, its memory space is automatically allocated.
 *           By contrast, the rt_mutex_init() function will initialize a static mutex object.
 * *
 * @return   Return a pointer to the mutex object. When the return value is RT_NULL, it means the creation failed.
 *
 * @warning  This function can ONLY be called from threads.
 */
rtos_mutex_t rtos_mutex_create(void) {
    return (rtos_mutex_t) xSemaphoreCreateMutex();
}

/**
 * @brief    This function will delete a mutex object and release this memory space.
 *
 * @note     This function is used to delete a mutex object which is created by the rt_mutex_create() function.
 *           When the mutex is successfully deleted, it will resume all suspended threads in the mutex list.
 *
 * @param    mutex is a pointer to a mutex object to be deleted.
 *
 * @return   None.
 * 
 */
void rtos_mutex_delete(rtos_mutex_t mutex) {
    vSemaphoreDelete(mutex);
}

/**
 * @brief    This function will take a mutex, if the mutex is unavailable, the thread shall wait for
 *           the mutex up to a specified time.
 *
 * @note     When this function is called, the count value of the mutex->value will decrease 1 until it is equal to 0.
 *           When the mutex->value is 0, it means that the mutex is unavailable. At this time, it will suspend the
 *           thread preparing to take the mutex.
 *           On the contrary, the rtos_mutex_give() function will increase the count value of mutex->value by 1 each time.
 *
 * @param    mutex is a pointer to a mutex object.
 *
 * @param    timeout is a timeout period (unit: an OS tick). If the mutex is unavailable, the thread will wait for
 *           the mutex up to the amount of time specified by the argument.
 *           NOTE:
 *           If use Macro RTOS_WAITING_FOREVER to set this parameter, which means that when the
 *           message is unavailable in the queue, the thread will be waiting forever.
 *           If use macro RTOS_WAITING_NO to set this parameter, which means that this
 *           function is non-blocking and will return immediately.
 *  
 * @return   Return the operation status. ONLY When the return value is RTOS_EOK, the operation is successful.
 *           If the return value is any other values, it means that the mutex take failed.
 */
int rtos_mutex_take(rtos_mutex_t mutex, uint32_t timeout) {
    return (xSemaphoreTake(mutex, pdMS_TO_TICKS(timeout)) == pdPASS) ? RTOS_EOK : RTOS_ETIMEOUT;
}

/**
 * @brief    This function will release a mutex. If there is thread suspended on the mutex, the thread will be resumed.
 *
 * @note     If there are threads suspended on this mutex, the first thread in the list of this mutex object
 *           will be resumed, and a thread scheduling will be executed.
 *           If no threads are suspended on this mutex, the count value mutex->value of this mutex will increase by 1.
 *
 * @param    mutex is a pointer to a mutex object.
 *
 * @param    call_flag is a caller flag. value can be RTOS_FROM_THREAD | RTOS_FROM_ISR.
 * 
 * @return   Return the operation status. When the return value is RTOS_EOK, the operation is successful.
 *           If the return value is any other values, it means that the mutex release failed.
 */
int rtos_mutex_give(rtos_mutex_t mutex) {
    return (xSemaphoreGive(mutex) == pdPASS) ? RTOS_EOK : RTOS_ETIMEOUT;
}

/**
 * @brief    Creating a semaphore object.
 *
 * @note     For the semaphore object, its memory space is allocated automatically.
 *
 * @param    value is the initial value for the semaphore.
 *           If used to share resources, you should initialize the value as the number of available resources.
 *           If used to signal the occurrence of an event, you should initialize the value as 0.
 * 
 * @return   Return a pointer to the semaphore object. When the return value is RT_NULL, it means the creation failed.
 *
 * @warning  This function can NOT be called in interrupt context.
 */
rtos_sem_t rtos_sem_create(uint32_t max_count, uint32_t init_val) {
    rtos_sem_t sem = NULL;

    if (max_count > 1) {
        sem = xSemaphoreCreateCounting(max_count, init_val);
    } else if (max_count == 1) {
        sem = xSemaphoreCreateBinary();
        if (sem && init_val) xSemaphoreGive(sem);
    }

    return sem;
}

/**
 * @brief    This function will delete a semaphore object and release the memory space.
 *
 * @note     This function is used to delete a semaphore object which is created by the rtos_sem_create() function.
 *           When the semaphore is successfully deleted, it will resume all suspended threads in the semaphore list.
 *
 * @param    sem is a pointer to a semaphore object to be deleted.
 *
 * @return   Return the operation status. When the return value is RTOS_EOK, the operation is successful.
 *           If the return value is any other values, it means that the semaphore detach failed.
 *
 */
void rtos_sem_delete(rtos_sem_t sem) {
    vSemaphoreDelete(sem);
}

/**
 * @brief    This function will take a semaphore, if the semaphore is unavailable, the thread shall wait for
 *           the semaphore up to a specified time.
 *
 * @note     When this function is called, the count value of the sem->value will decrease 1 until it is equal to 0.
 *           When the sem->value is 0, it means that the semaphore is unavailable. At this time, it will suspend the
 *           thread preparing to take the semaphore.
 *           On the contrary, the rtos_sem_give() function will increase the count value of sem->value by 1 each time.
 *
 * @param    sem is a pointer to a semaphore object.
 *
 * @param    timeout is a timeout period (unit: an OS tick). If the semaphore is unavailable, the thread will wait for
 *           the semaphore up to the amount of time specified by this parameter.
 *
 * @param    call_flag is a caller flag. value can be RTOS_FROM_THREAD | RTOS_FROM_ISR.
 *
 *           NOTE:
 *           If use Macro RTOS_WAITING_FOREVER to set this parameter, which means that when the
 *           message is unavailable in the queue, the thread will be waiting forever.
 *           If use macro RTOS_WAITING_NO to set this parameter, which means that this
 *           function is non-blocking and will return immediately.
 *
 * @return   Return the operation status. ONLY When the return value is RT_EOK, the operation is successful.
 *           If the return value is any other values, it means that the semaphore take failed.
 *
 * @warning  This function can ONLY be called in the thread context. It MUST NOT BE called in interrupt context.
 */
int rtos_sem_take(rtos_sem_t sem, uint32_t timeout) {
    if (xPortIsInsideInterrupt()) {
        return ((xSemaphoreTakeFromISR((SemaphoreHandle_t) sem, NULL) == pdPASS) ? RTOS_EOK : RTOS_ETIMEOUT);
    } else {
        return ((xSemaphoreTake((SemaphoreHandle_t) sem, pdMS_TO_TICKS(timeout)) == pdPASS) ? RTOS_EOK : RTOS_ETIMEOUT);
    }
}


/**
 * @brief    This function will release a semaphore. If there is thread suspended on the semaphore, it will get resumed.
 *
 * @note     If there are threads suspended on this semaphore, the first thread in the list of this semaphore object
 *           will be resumed, and a thread scheduling will be executed.
 *           If no threads are suspended on this semaphore, the count value sem->value of this semaphore will increase by 1.
 *
 * @param    sem is a pointer to a semaphore object.
 *
 * @param    call_flag is a caller flag. value can be RTOS_FROM_THREAD | RTOS_FROM_ISR.
 *
 * @return   Return the operation status. When the return value is RTOS_EOK, the operation is successful.
 *           If the return value is any other values, it means that the semaphore release failed.
 */
int rtos_sem_give(rtos_sem_t sem) {
    if (xPortIsInsideInterrupt()) {
        return (xSemaphoreGiveFromISR((SemaphoreHandle_t) sem, NULL) == pdPASS) ? RTOS_EOK : RTOS_ETIMEOUT;
    } else {
        return (xSemaphoreGive((SemaphoreHandle_t) sem) == pdPASS) ? RTOS_EOK : RTOS_ETIMEOUT;
    }
}

/**
 * @brief    This function will return current tick from operating system startup.
 *
 * @return   Return current tick.
 */
uint32_t rtos_tick_get(void) {
    /* return the global tick */
    return xTaskGetTickCount();
}

/**
 * @brief   This function will create a thread object and allocate thread object memory.
 *          and stack.
 *
 * @param   name is the name of thread, which shall be unique.
 *
 * @param   entry is the entry function of thread.
 *
 * @param   args is the parameter of thread enter function.
 *
 * @param   stk_size is the size of thread stack.
 *
 * @param   prio is the priority of thread.
 *
 * @return  If the return value is a rt_thread structure pointer, the function is successfully executed.
 *          If the return value is NULL, it means this operation failed.
 */
rtos_thread_t rtos_thread_create(const char *name, uint32_t stk_size, uint32_t prio, rtos_entry_t entry, void *args) {
    TaskHandle_t htask = NULL;
    stk_size /= sizeof(StackType_t);
    xTaskCreate((TaskFunction_t) entry, name, stk_size, args, prio, &htask);
    return (rtos_thread_t) htask;
}

void rtos_thread_delete(rtos_thread_t task_handle) {
    vTaskDelete(task_handle);
}

void rtos_thread_start(void) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        vTaskStartScheduler();
    }
}

void rtos_delayms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

