/**
 * @file multi_task.h
 * @brief 轻量级协作式任务调度器
 * @note 符合 MISRA-C 规范风格，增加了临界区保护
 */

#ifndef MULTI_TASK_H_
#define MULTI_TASK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef TASK_ENTER_CRITICAL
#define TASK_ENTER_CRITICAL()  
#endif

#ifndef TASK_EXIT_CRITICAL
#define TASK_EXIT_CRITICAL()   
#endif

/* 最大任务数量 */
#define TASK_MAX_NUM    (10U)

/* 状态码定义 */
typedef enum {
    TASK_OK = 0,
    TASK_ERR_FULL,      // 任务池已满
    TASK_ERR_NOT_FOUND, // 未找到任务
    TASK_ERR_PARAM      // 参数错误
} task_status_t;

/* 任务函数指针定义 */
typedef int (*task_func_t)(void *arg);

/* 核心 API */
void          task_system_init(void);
uint32_t      task_tick_get(void);
void          task_tick_update(void);
task_status_t task_create(task_func_t func, void *arg, uint32_t delay_ms, uint32_t period_ms);
task_status_t task_delete(task_func_t func);
void          task_scheduler(void);

#ifdef __cplusplus
}
#endif

#endif /* MULTI_TASK_H_ */