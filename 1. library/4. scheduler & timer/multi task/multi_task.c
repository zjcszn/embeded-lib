#include "multi_task.h"
#include <string.h>


/*============================== Macro Definitions =================================*/
#define TICKS_LT(a, b)   ((int32_t)((uint32_t)(a) - (uint32_t)(b)) <  0)    // Less than
#define TICKS_GE(a, b)   ((int32_t)((uint32_t)(a) - (uint32_t)(b)) >= 0)    // Greater than or equal to


/*============================== Type Definitions =================================*/
typedef struct task_cb {
    task_func_t func;     
    void* arg;             
    uint32_t    period;   
    uint32_t    deadline;  
    struct task_cb* next;       
} task_cb_t;


/*============================== Private Variables =================================*/
static task_cb_t  s_task_pool[TASK_MAX_NUM];
static task_cb_t* s_task_list_head = NULL;
static volatile uint32_t s_system_tick = 0;




/*============================== Private Functions =================================*/

/**
 * @brief add a task to the task list
 * @note  must be called in critical section
 * 
 * @param node the task to be inserted
 */
static void task_insert(task_cb_t *node) {
    task_cb_t **curr_ptr = &s_task_list_head;

    while (*curr_ptr != NULL) {
        task_cb_t *entry = *curr_ptr;
        
        if (TICKS_LT(node->deadline, entry->deadline)) {
            break; 
        }
        curr_ptr = &entry->next;
    }

    node->next = *curr_ptr;
    *curr_ptr = node;
}


/**
 * @brief initialize the task system
 * @note  must be called in system startup, initialize task pool and ready list
 */
void task_system_init(void) {
    TASK_ENTER_CRITICAL();
    memset(s_task_pool, 0, sizeof(s_task_pool));
    s_task_list_head = NULL;
    s_system_tick = 0;
    TASK_EXIT_CRITICAL();
}



/**
 * @brief update the system tick
 * @note  must be called in timer interrupt, 1ms period
 */
void task_tick_update(void) {
    s_system_tick++;
}



/**
 * @brief get the current system tick
 * @return uint32_t the current system tick
 */
uint32_t task_get_tick(void) {
    uint32_t tick;
    TASK_ENTER_CRITICAL();
    tick = s_system_tick;
    TASK_EXIT_CRITICAL();
    return tick;
}


/**
 * @brief create a new task
 * @note  must be called in critical section
 * 
 * @param func the task function
 * @param arg the argument passed to the task function
 * @param delay_ms the delay in ms before the first execution
 * @param period_ms the period in ms for periodic execution
 * @return task_status_t the status of the task creation
 */
task_status_t task_create(task_func_t func, void *arg, uint32_t delay_ms, uint32_t period_ms) {
    task_cb_t *new_task = NULL;
    task_status_t status = TASK_OK;

    if (func == NULL) {
        return TASK_ERR_PARAM;
    }

    /* delete existing task with the same function pointer */
    task_delete(func);

    TASK_ENTER_CRITICAL();

    /* find an empty slot in task pool */
    for (uint32_t i = 0; i < TASK_MAX_NUM; i++) {
        if (s_task_pool[i].func == NULL) {
            new_task = &s_task_pool[i];
            break;
        }
    }

    if (new_task == NULL) {
        /* if no empty slot found, return error */
        status = TASK_ERR_FULL;
    } else {
        new_task->func = func;
        new_task->arg = arg;
        new_task->period = period_ms; 
        new_task->deadline = s_system_tick + delay_ms;
        task_insert(new_task);
    }

    TASK_EXIT_CRITICAL();
    return status;
}




/**
 * @brief delete a task
 * @note  must be called in critical section
 * 
 * @param func the task function to be deleted
 * @return task_status_t the status of the task deletion
 */
task_status_t task_delete(task_func_t func) {
    task_cb_t **curr_ptr;
    task_cb_t *target = NULL;
    task_status_t status = TASK_ERR_NOT_FOUND;

    if (func == NULL) {
        return TASK_ERR_PARAM;
    }

    TASK_ENTER_CRITICAL();
    
    curr_ptr = &s_task_list_head;
    while (*curr_ptr != NULL) {
        if ((*curr_ptr)->func == func) {
            target = *curr_ptr;
            *curr_ptr = target->next;
            memset(target, 0, sizeof(task_cb_t));
            status = TASK_OK;
            break; 
        }
        curr_ptr = &((*curr_ptr)->next);
    }

    TASK_EXIT_CRITICAL();
    return status;
}


void task_scheduler(void) {
    task_cb_t *current_task;
    uint32_t now;
    int task_result;

    do {
        current_task = NULL;

        /* STEP1: Try to get a task to execute (Atomic Pop) */
        TASK_ENTER_CRITICAL();

        if (s_task_list_head != NULL) {
            now = s_system_tick;

            if (TICKS_GE(now, s_task_list_head->deadline)) {
                current_task = s_task_list_head;
                s_task_list_head = current_task->next;
                current_task->next = NULL;
            }
        }

        TASK_EXIT_CRITICAL();

        if (current_task == NULL) {
            break;
        }


        /* STEP2: Execute the task (No Lock) */
        if (current_task->func != NULL) {
            task_result = current_task->func(current_task->arg);
        } else {
            task_result = 1;
        }

        /* STEP3: Post-process the task result (Atomic) */
        TASK_ENTER_CRITICAL();

        now = s_system_tick;
        
        if ((current_task->period > 0) && (task_result == 0)) {

            current_task->deadline += current_task->period;

            if (TICKS_LT(current_task->deadline, now)) {
                
                //LOG("Warning: Task overload/latency detected! Task Addr: %p\n", current_task->func); 
                current_task->deadline = now + current_task->period;
            }     

            task_insert(current_task);

        } else {
            memset(current_task, 0, sizeof(task_cb_t));
        }

        TASK_EXIT_CRITICAL();
        
    } while (1);
}