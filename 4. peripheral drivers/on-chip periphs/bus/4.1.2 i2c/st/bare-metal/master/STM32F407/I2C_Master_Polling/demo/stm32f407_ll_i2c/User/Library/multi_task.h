#ifndef __MULTI_TASK_H__
#define __MULTI_TASK_H__

#include <stdint.h>

#ifdef __cplusplus  
extern "C" {  
#endif

/********************* 类型声明 *********************/

// 任务入口函数指针
typedef void (*entry_t)(void *args);
// 任务结构体类型
typedef struct _task task_t;

struct _task {
  entry_t   entry;      // 任务入口函数
  void*     args;       // 任务参数
  uint16_t  delay;      // 延迟时间
  uint16_t  period;     // 间隔周期
  task_t*   next;       // next指针    
  uint64_t  timeout;    // 超时时间  
};

/********************* 函数声明 *********************/

void task_ticks_update(void);
int  task_add(void(*entry)(void *args), void *args, uint16_t delay, uint16_t period);
int  task_del(void(*entry)(void *args));
task_t* task_find(void(*entry)(void *args));
void task_period_update(task_t *task, uint16_t period);
void task_poll(void);

#ifdef __cplusplus  
}  
#endif

#endif