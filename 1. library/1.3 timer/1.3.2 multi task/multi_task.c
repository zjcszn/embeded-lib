#include "multi_task.h"
#include <string.h>

/************************ 宏定义 ***********************/

#define __IO volatile

#define TASK_MAX_NUM    (10U)   // 最大任务数量

/********************* 静态全局变量 *********************/

// 任务堆空间
static task_t  task_heap[TASK_MAX_NUM] = {0};

// 任务调度队列
static task_t* task_list = NULL;

// 任务Ticks
static __IO uint64_t task_ticks = 0;

/********************* 私有函数声明 *********************/

static void    task_insert(task_t *new_task);
static task_t* task_malloc(void);
static void    task_free(task_t *task);

/*********************** 函数定义 **********************/

/**
 * @brief Ticks更新函数，定时器内运行
 * 
 */
void task_ticks_update(void) {
  task_ticks++;
}


/**
 * @brief 添加一个新任务,并插入任务调度队列
 * 
 * @param entry 任务入口函数
 * @param args 传入参数
 * @param delay 延时时间
 * @param period 循环周期
 * @return int 添加失败返回-1，添加成功返回0
 */
int task_add(void(*entry)(void *args), void *args, uint16_t delay, uint16_t period) {
  task_t *new_task = task_malloc();
  if (new_task == NULL) return -1;
  new_task->entry   = entry;
  new_task->args    = args;
  new_task->delay   = delay;
  new_task->period  = period;
  new_task->timeout = task_ticks + delay;
  task_insert(new_task);
  return 0;
}

/**
 * @brief 将一个任务按序插入任务调度队列中
 * 
 * @param task 
 */
static void task_insert(task_t *task) {
  task_t **target;
  for (target = &task_list;; target = &(*target)->next) {
    if (!*target) {
        task->next = NULL;
        *target = task;
        break;
    }
    if (task->timeout < (*target)->timeout) {
        task->next = *target;
        *target = task;
        break;
    }
  }
}

/**
 * @brief 从任务调度队列中删除一个任务
 * 
 * @param entry 待删除的任务函数指针
 * @return int 删除失败返回-1，删除成功返回0
 */
int task_del(void(*entry)(void *args)) {
  task_t** target = &task_list;
  for (; *target; target = &(*target)->next) {
    if ((*target)->entry == entry) {
      task_t *del_task = *target;
      *target = (*target)->next;
      task_free(del_task);
      return 0;
    }
  }
  return -1;
}

/**
 * @brief 在任务调度队列中查找指定任务
 * 
 * @param entry 
 * @return task_t* 任务空间首地址
 */
task_t* task_find(void(*entry)(void *args)) {
  task_t** target = &task_list;
  for (; *target; target = &(*target)->next) {
    if ((*target)->entry == entry) {
      return *target;
    }
  }
  return NULL;
}

/**
 * @brief 更新任务循环周期
 * 
 * @param task 
 * @param period 
 */
void task_period_update(task_t *task, uint16_t period) {
  task->period = period;
}

/**
 * @brief 任务调度处理程序
 * 
 */
void task_poll(void) {
  task_t *target = task_list;
  for (; target; target = target->next) {
    if(task_ticks < target->timeout) {
      return;
    }
    task_list = target->next;
    if (target->period) {
      target->timeout = task_ticks + target->period;
      task_insert(target);
    }
    target->entry(target->args);
  }
}


/**
 * @brief 任务堆分配函数，从定义的堆数组内寻找可用空间，并分配给新任务
 * 
 * @return task_t* 任务空间首地址
 */
static task_t* task_malloc(void) {
  for (int i = 0; i < TASK_MAX_NUM; i++) {
    if(task_heap[i].entry == NULL) {
      return &task_heap[i];
    }
  }
  return NULL;
}

/**
 * @brief 任务堆释放函数，释放不使用的任务空间
 * 
 * @param task 
 */
static void task_free(task_t *task) {
  if (task != NULL) {
    memset(task, 0x00, sizeof(task_t));
  }
}
