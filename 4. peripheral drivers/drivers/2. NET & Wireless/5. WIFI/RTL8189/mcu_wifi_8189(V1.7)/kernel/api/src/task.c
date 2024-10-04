//#define DEBUG
#include "api.h"
#include "debug.h"
#include "task.h"

static struct workqueue_struct *timer_tasklet = NULL;
static struct workqueue_struct *tasklet = NULL;

static void os_workqueue_thread(struct workqueue_struct *workqueue);


int thread_create(void(*task)(void *p_arg), void *p_arg, unsigned int prio, unsigned int *pbos, unsigned int stk_size, char *name)
{
	INT8U ret, need_free_stack = 0;
	if (pbos == 0)
	{
		need_free_stack = 1;
		pbos = (OS_STK*)mem_calloc(stk_size, sizeof(OS_STK));
		if (!pbos)
			return OS_ERR;
	}

	ret = OSTaskCreateExt(task, p_arg, &pbos[stk_size - 1], prio, prio, pbos, stk_size, NULL, (INT16U)(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR), need_free_stack);

	if (ret == OS_ERR_NONE)
	{
		OSTaskNameSet(prio, (INT8U*)name, (INT8U*) &ret);
		return prio;
	}else{
		if(need_free_stack && pbos)
			mem_free(pbos);
	}

	return  - 1;
}

int thread_exit(int thread_id)
{
	int ret;
	ret = OSTaskDel(thread_id);
	return ret;
}

//返回任务自身的ID号
int thread_myself()
{
	return OSTCBCur->OSTCBId;
}


void initList(PLIST_HEADER pList)
{
	pList->pHead = pList->pTail = NULL;
	pList->size = 0;
	return ;
}

void insertTailList(PLIST_HEADER pList, PLIST_ENTRY pEntry)
{
	uint32_t cpu_sr;
	assert(pList);
    	cpu_sr = local_irq_save();
	pEntry->pNext = NULL;
	if (pList->pTail)
		pList->pTail->pNext = pEntry;
	else
		pList->pHead = pEntry;
	pList->pTail = pEntry;
	pList->size++;
	local_irq_restore(cpu_sr);
	return ;
}

PLIST_ENTRY removeHeadList(PLIST_HEADER pList)
{
	PLIST_ENTRY pNext;
	PLIST_ENTRY pEntry = 0;
	uint32_t cpu_sr;
	assert(pList);
    	cpu_sr = local_irq_save();
	pEntry = pList->pHead;
	if (pList->pHead != NULL)
	{
		pNext = pList->pHead->pNext;
		pList->pHead = pNext;
		if (pNext == NULL)
			pList->pTail = NULL;
		pList->size--;
	}else{
		
		pList->size = 0;
	}
	local_irq_restore(cpu_sr);
	return pEntry;
}

int getListSize(PLIST_HEADER pList)
{
	return pList->size;
}

PLIST_ENTRY delEntryList(PLIST_HEADER pList, PLIST_ENTRY pEntry)
{
	PLIST_ENTRY pCurEntry;
	PLIST_ENTRY pPrvEntry;

	if (pList->pHead == NULL)
		return NULL;

	if (pEntry == pList->pHead)
	{
		pCurEntry = pList->pHead;
		pList->pHead = pCurEntry->pNext;

		if (pList->pHead == NULL)
			pList->pTail = NULL;

		pList->size--;
		return pCurEntry;
	}

	pPrvEntry = pList->pHead;
	pCurEntry = pPrvEntry->pNext;
	while (pCurEntry != NULL)
	{
		if (pEntry == pCurEntry)
		{
			pPrvEntry->pNext = pCurEntry->pNext;

			if (pEntry == pList->pTail)
				pList->pTail = pPrvEntry;

			pList->size--;
			break;
		}
		pPrvEntry = pCurEntry;
		pCurEntry = pPrvEntry->pNext;
	}

	return pCurEntry;
}

struct workqueue_struct *create_workqueue(const char *name, uint32_t stack_size, int prio)
{
	struct workqueue_struct *new_queue = (struct workqueue_struct *)mem_calloc(sizeof(struct workqueue_struct ), 1);
	if(!new_queue)
		return NULL;

	new_queue->thread_event = init_event();
	if(!new_queue->thread_event)
		goto faild1;

	initList(&new_queue->list);
	new_queue->name = name;
	new_queue->task_id = prio; //thread_create调用的时候可能马上会切换到新线程，这里预先赋好值
	new_queue->task_id = thread_create((void (*)(void*))os_workqueue_thread,
                        new_queue,
                        prio,
                        NULL,
                        stack_size,
                        (char*)name);

	if(new_queue->task_id <= 0)
		goto faild;
	

	return new_queue;
faild:
	del_event(new_queue->thread_event);;
faild1:
	mem_free(new_queue);
	OSSchedUnlock();
	return NULL;
}

void destroy_workqueue(struct workqueue_struct *workqueue)
{
	if(!workqueue)
		return;
	p_dbg("destroy %s", workqueue->name);
	del_event(workqueue->thread_event);
	thread_exit(workqueue->task_id);
	mem_free(workqueue);
}

/*
*工作线程
*参考linux的workqueue机制
*为异步事件的处理提供一种很好的方式
*/
static void os_workqueue_thread(struct workqueue_struct *workqueue)
{
	int ret;
	uint32_t irq_flag;
	struct work_struct *work;
	p_dbg("start workqueue %s id:%d", workqueue->name, workqueue->task_id);
	while (1)
	{
		ret = wait_event_timeout(workqueue->thread_event, 5000);
		if (ret != 0)
		{
			if (ret != WAIT_EVENT_TIMEOUT)
			{
				p_err("os_tasklet_thread wait err:%d, %x\n", ret, workqueue->thread_event);
				sleep(2000);
			}
			continue;
		}

		clear_wait_event(workqueue->thread_event);
		if (workqueue->list.size > 0)
		{
			while (workqueue->list.size)
			{
				PLIST_ENTRY entry = NULL;
				irq_flag = local_irq_save();
				if(workqueue->list.size)
					entry = removeHeadList(&workqueue->list);
				local_irq_restore(irq_flag);
					
				if (entry)
				{
					work = (struct work_struct*)((char*)entry - offsetof(struct work_struct, entry));
					if (work->fun)
						work->fun(work->data);
				}
				else
				{
					p_err("no entry:%d\n", workqueue->list.size);
					break;
				} 
			}
		}

	}
}

/*
*定时器服务线程
*因ucos自带的定时器函数不允许使用信号等待操作
*这里单独建一个线程
*注意:已经对ucos内核做了一点修改
*/
int init_timer_tasklet()
{
	timer_tasklet = create_workqueue("timer tasklet", TIMER_TASKLET_STACK_SIZE, TASK_TIMER_TASKLET_PRIO);
	if(timer_tasklet)
		return 0;
	else
		return -1;
}

int init_sys_tasklet()
{
	tasklet = create_workqueue("system tasklet", SYS_TASKLET_STACK_SIZE, TASK_SYS_TASKLET_PRIO);
	if(tasklet)
		return 0;
	else
		return -1;
}


void init_work(struct work_struct *work, void *fun, void *data)
{
	work->fun = (void (*)(void *))fun;
	work->data = data;
}

int schedule_work(struct workqueue_struct *queue, struct work_struct *work)
{
	uint32_t irq_flg;
	LIST_ENTRY *entry;
	irq_flg = local_irq_save();
	for(entry = queue->list.pHead; entry != NULL; entry = entry->pNext)
	{
		if(entry == &work->entry)
		{
			break;
		}
	}
	if(entry == NULL)
		insertTailList(&queue->list, &work->entry);
	local_irq_restore(irq_flg);
	
	wake_up(queue->thread_event);
	return 0;
}

void init_tasklet(struct tasklet_struct *work, void *fun, void *data)
{
	work->fun = (void (*)(void *))fun;
	work->data = data;
}


int schedule_tasklet(struct tasklet_struct *work)
{
	int ret = -1;
	if(tasklet)
		schedule_work(tasklet, (struct work_struct *)work);

	return ret;
}

int schedule_timer_tasklet(struct work_struct *work)
{
	int ret = -1;
	if(timer_tasklet)
		ret = schedule_work(timer_tasklet,  work);
	return ret;
}

void timer_tasklet_callback(struct work_struct *work)
{
	schedule_timer_tasklet(work);
}

void init_work_thread(void)
{
	init_timer_tasklet();
    init_sys_tasklet();
}

