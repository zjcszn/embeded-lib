#ifndef __MULTI_TASK_H__
#define __MULTI_TASK_H__

#include <stdint.h>

#ifdef __cplusplus  
extern "C" {  
#endif


#define TASK_MAX_NUM    (10U)   // 最大任务数量


typedef int (*entry_t)(void *args);



uint32_t get_system_ticks(void);
void task_ticks_update(void);
int  task_create(entry_t entry, void *args, uint16_t delay, uint16_t period);
int  task_delete(entry_t entry);
void task_scheduler(void);

#ifdef __cplusplus  
}  
#endif

#endif