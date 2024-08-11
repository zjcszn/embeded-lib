#include "multi_task.h"
#include <string.h>


#define __IO          volatile

typedef struct task_handle task_handle_t;

struct task_handle{
    entry_t entry;       
    void* args;        
    uint16_t  delay;       
    uint16_t  period;      
    uint32_t  deadline;    
    task_handle_t *next;         
};

static task_handle_t  tcb[TASK_MAX_NUM];
static task_handle_t *task_list;
static __IO uint32_t  task_ticks;


static void task_insert(task_handle_t *task) {
    task_handle_t  **p;
    uint32_t  now = task_ticks;

    for (p = &task_list; ; p = &(*p)->next) {
        if (!(*p)) {
            task->next = NULL;
            *p = task;
            break;
        }
        if ((int)(task->deadline - now) < (int)((*p)->deadline - now)) {
            task->next = *p;
            *p = task;
            break;
        }
    }
}

void task_ticks_update(void) {
    task_ticks++;
}


uint32_t get_system_ticks(void) {
    return task_ticks;
}

/**
 * @brief 删除任务
 * 
 * @param entry 任务入口
 * @return int 0表示删除失败，1表示删除失败（未找到该任务）
 */
int task_delete(entry_t entry) {
    task_handle_t** p;
    task_handle_t*  del;
    
    for (p = &task_list; *p; p = &((*p)->next)) {
        if ((*p)->entry == entry) {
            del = *p;
            *p = (*p)->next;
            memset(del, 0x00, sizeof(task_handle_t));
            return 0;
        }
    }

    return 1;
}

/**
 * @brief 创建任务
 * 
 * @param entry 任务入口
 * @param args 传入参数
 * @param delay 延迟时间 ms
 * @param period 调度周期
 * @return int 0表示创建成功， 1表示创建失败
 */
int task_create(entry_t entry, void *args, uint16_t delay, uint16_t period) {
    task_handle_t *new = NULL;

    task_delete(entry); /* 如果任务已在列表中，则删除旧任务 */

    for (int i = 0; i < TASK_MAX_NUM; i++) {
        if(tcb[i].entry == NULL) {
            new = &tcb[i];
            break;
        }
    }

    if (new != NULL) {
        new->entry    = entry;
        new->args     = args;
        new->delay    = delay;
        new->period   = period;
        new->deadline = task_ticks + delay;

        task_insert(new);
        return 0;
    }

    return 1;
}




void task_scheduler(void) {
    task_handle_t  *p;
    uint32_t now = task_ticks;

    for (p = task_list; p; p = p->next) {
        if((int)(p->deadline - now) > 0) {
            return;
        }

        task_list = p->next;
        now = task_ticks;

        if ((p->entry(p->args) == 0) && (p->period)) {
            p->deadline = now + p->period;
            task_insert(p);
        } else {
            memset(p, 0x00, sizeof(task_handle_t));
        }
    }
}


