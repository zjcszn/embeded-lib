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
 * @brief 插入一个任务到任务列表中，按照任务的截止时间进行排序
 * 
 * @param node 要插入的任务节点
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
 * @brief 初始化任务系统，包括任务池和就绪列表
 * @note  必须在系统启动时，创建第一个任务之前调用
 */
void task_system_init(void) {
    TASK_ENTER_CRITICAL();
    memset(s_task_pool, 0, sizeof(s_task_pool));
    s_task_list_head = NULL;
    s_system_tick = 0;
    TASK_EXIT_CRITICAL();
}



/**
 * @brief 更新系统时钟，必须在定时器中断中调用，周期为1ms
 */
void task_tick_update(void) {
    s_system_tick++;
}



/**
 * @brief 获取当前系统时钟
 * @return uint32_t 当前系统时钟值，单位为ms
 */
uint32_t task_tick_get(void) {
    uint32_t tick;
    TASK_ENTER_CRITICAL();
    tick = s_system_tick;
    TASK_EXIT_CRITICAL();
    return tick;
}


/**
 * @brief 创建一个新任务
 * 
 * @param func 任务函数指针，任务执行时调用的函数
 * @param arg 任务函数的参数指针，任务执行时传递给任务函数的参数
 * @param delay_ms 任务的初始延迟时间，单位为ms
 * @param period_ms 任务的执行周期，单位为ms
 * @return task_status_t 任务创建状态，TASK_OK表示成功，其他值表示失败
 */
task_status_t task_create(task_func_t func, void *arg, uint32_t delay_ms, uint32_t period_ms) {
    task_cb_t *new_task = NULL;
    task_status_t status = TASK_OK;

    if (func == NULL) {
        return TASK_ERR_PARAM;
    }

    /* 删除已存在的相同函数指针的任务 */
    task_delete(func);

    TASK_ENTER_CRITICAL();

    /* 查找任务池中的空槽位 */
    for (uint32_t i = 0; i < TASK_MAX_NUM; i++) {
        if (s_task_pool[i].func == NULL) {
            new_task = &s_task_pool[i];
            break;
        }
    }

    if (new_task == NULL) {
        /* 任务池已满，返回错误 */
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
 * @brief 删除一个任务
 * 
 * @param func 要删除的任务函数指针
 * @return task_status_t 任务删除状态，TASK_OK表示成功，其他值表示失败
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

        /* STEP1: 检查是否有任务就绪 */
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


        /* STEP2: 执行任务 */
        if (current_task->func != NULL) {
            task_result = current_task->func(current_task->arg);
        } else {
            task_result = 1;
        }

        /* STEP3: 更新任务的下一次执行时间 */
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