#include "CO_platform.h"

#include "n32a455.h"
#include "perf_counter.h"


/**
 * @brief Get the current system time in milliseconds.
 * 
 * @return CO_time_t The current system time in milliseconds.
 */
CO_time_t CO_Platform_GetMs(void) 
{
    return (CO_time_t)get_system_ms();
}



/**
 * @brief Lock the platform's interrupt mask to prevent interrupts.
 * 
 * @return uint32_t The previous interrupt mask value.
 */
uint32_t CO_Platform_Lock(void) 
{
    
    /* Save interrupt mask and disable interrupts */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    return primask;
}


/**
 * @brief Unlock the platform's interrupt mask, restoring previous interrupt state.
 * 
 * @param primask The previous interrupt mask value.
 */
void CO_Platform_Unlock(uint32_t primask) 
{
    /* Restore interrupt mask */
    __set_PRIMASK(primask);
}




void CO_Platform_SleepMs(CO_time_t ms) 
{
    delay_ms(ms);
}


/* if CO_USE_GLOBALS not defined, need to implement CO_Platform_Calloc and CO_Platform_Free */
#ifndef CO_USE_GLOBALS
/**
 * @brief Allocate memory using the standard C library function calloc.
 * 
 * @param num Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @return void* Pointer to the allocated memory.
 */
void *CO_Platform_Calloc(uint32_t num, uint32_t size) {
    return calloc(num, size);
}


/**
 * @brief Free memory allocated by CO_Platform_Calloc.
 * 
 * @param ptr Pointer to the memory to be freed.
 */
void CO_Platform_Free(void *ptr) {
    free(ptr);
}
#endif   // CO_USE_GLOBALS


