#ifndef _API_H_
#define _API_H_

//kernel
#include "ucos_ii.h"
#include "atomic.h"
#include "mbox.h"
#include "mutex.h"
#include "sem.h"
#include "wait.h"
#include "task.h"
#include "timer.h"
#include "msg_q.h"
#include "memory.h"

extern volatile uint32_t jiffies;

__inline void enter_interrupt(void)
{
	OSIntNesting++;
}

__inline void exit_interrupt(int need_sched)
{
	if (need_sched)
		OSIntExit();
	else
		OSIntNesting--;
}

#endif
