#include "lwip/arch/sys_arch.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "api.h"
#include "app.h"


struct timeoutnode 
{
	struct sys_timeouts timeouts;
	INT8U prio;
	struct timeoutnode *next;
};

struct timeoutnode nulltimeouts;

struct timeoutnode *timeoutslist;

const void * const pvNullPointer;

void sys_init(void)
{
    timeoutslist = &nulltimeouts;
    
    nulltimeouts.timeouts.next = NULL; 
    
    nulltimeouts.next = NULL;
}

u32_t sys_jiffies(void)
{
	return jiffies;
}

u32_t sys_now(void)
{
	return jiffies*10;     //返回毫秒
}

err_t sys_sem_new(sys_sem_t *sem,u8_t count)
{
	*sem = OSSemCreate((INT16U)count);
	if(*sem == 0)
	{
		p_err("sys_sem_new err");
		return ERR_VAL;
	}
	return ERR_OK;
}

int sys_sem_valid(sys_sem_t *sem)
{
	if(*sem == 0)
		return 0;
	return 1;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = 0;
}
/*
;*****************************************************************************************************
;* 函数名称 : sys_sem_free
;* 描    述 : 删除一个信号
;* 输　	 入 : sem: 信号句柄
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_sem_free(sys_sem_t *sem)
{
	INT8U err;
	
	OSSemDel(*sem, OS_DEL_ALWAYS, &err);
}


/*
;*****************************************************************************************************
;* 函数名称 : sys_sem_signal
;* 描    述 : 发送一个信号
;* 输　	 入 : sem: 信号句柄
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_sem_signal(sys_sem_t *sem)
{
	INT8U err;
	err = OSSemPost(*sem);
	if(err)
		p_err("sys_sem_signal err:%d\n", err);
}


/*
;*****************************************************************************************************
;* 函数名称 : sys_arch_sem_wait
;* 描    述 : 等待一个信号
;* 输　	 入 : sem: 信号句柄, timeout: 等待超时的微秒数
;*        
;* 输　	 出 : 等待所用的微秒数
;*****************************************************************************************************
;*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	INT8U err;
	u32_t ticks, ret;
	u32_t wait_ms = tick_ms;
	
	if (OSSemAccept(*sem))			/* 如果已经收到, 则返回0 */
	{
		ret = OS_ERR_NONE;
		goto end;
	}	
		
	if (timeout == 0)				/* timeout == 0, 则一直等待 */
	{
		OSSemPend(*sem, 0, &err);	/* 一直等待直至信号来到 */
			
		if (err == OS_ERR_NONE)		/* 收到, 返回0 */
		{
			ret = OS_ERR_NONE;
		}
		else						/* 没有收到, 返回SYS_ARCH_TIMEOUT */
		{
			p_err("unkown err:%d", err);
			ret = SYS_ARCH_TIMEOUT;
		}
//		goto end;
	}
	else
	{
		ticks = (timeout * OS_TICKS_PER_SEC) / 1000;
		
		if (ticks < 1)
		{
			ticks = 1;
		}
		else if (ticks > 65535)
		{
			ticks = 65535;
		}
		
		OSSemPend(*sem, (u16_t)ticks, &err);
		
		if (err == OS_ERR_NONE)
		{	
			ret = OS_ERR_NONE;
		}
		else
		{
//			p_err("wait %d ticks, timeout", ticks);
			ret = SYS_ARCH_TIMEOUT;
		}
	}
end:
	if(ret == OS_ERR_NONE)
		ret = tick_ms - wait_ms;
	return ret;
}


/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_new
;* 描    述 : 创建一个邮箱
;* 输　	 入 : size: 邮箱容量(实际不起作用)
;*        
;* 输　	 出 : sys_mbox_t: 邮箱句柄
;*****************************************************************************************************
;*/


err_t sys_mbox_new(sys_mbox_t *pMbox, sys_sem_t *sem, int size)
{

	err_t ret = ERR_OK;
	int real_size = min(size,MAX_MSG_IN_LWIP_MBOX);
	
	*pMbox = (sys_mbox_t)mem_calloc(sizeof(LWIP_MBOX),1);
	
	if (*pMbox == NULL)
	{
		ret = 1;
		goto end;
	}
	
	(*pMbox)->lwip_mbox_e = OSQCreate((*pMbox)->lwip_msg_q, real_size);
	
	if ((*pMbox)->lwip_mbox_e == NULL)
	{
		mem_free(*pMbox);
		*pMbox = 0;
		ret = 2;
		goto end;
	}
	
	*sem = OSSemCreate(0);
	if(*sem == 0)
	{
		ret = 3;
		p_err("sys_mbox_new OSSemCreate err\n");
	}
end:
	if(ret != ERR_OK)
		p_err("sys_mbox_new err:%d\n",ret);
	return ret;
}

int sys_mbox_valid(sys_mbox_t *pMbox)
{
	if(*pMbox == 0)
		return 0;
	return 1;
}

void sys_mbox_set_invalid(sys_mbox_t *pMbox)
{
	*pMbox = 0;
}
/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_post
;* 描    述 : 发送邮件到邮箱
;* 输　	 入 : mbox: 邮箱句柄, msg: 邮件
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_mbox_post(sys_mbox_t *mbox, sys_sem_t *sem, void *msg)
{
	INT8U err;
	if(*mbox == 0)
	{
		p_err("sys_mbox_post arg err\n");
		return;
	}
	if (msg == NULL)
		msg = (void*)&pvNullPointer;
again:
	if(OSQPost((*mbox)->lwip_mbox_e, msg) == OS_ERR_Q_FULL)
	{
		if(in_interrupt()){
			mem_free(msg);
			return;
		}
		OSSemPend(*sem, 10, &err); //超时5个tick,
		goto again;
	}
/*    	
	while (OSQPost((*mbox)->lwip_mbox_e, msg) == OS_ERR_Q_FULL)
		OSTimeDly(10);*/
}


/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_trypost
;* 描    述 : 尝试发送邮件到邮箱
;* 输　	 入 : mbox: 邮箱句柄, msg: 邮件
;*        
;* 输　	 出 : ERROR: ERR_MEM | OK: ERR_OK
;*****************************************************************************************************
;*/

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	err_t ret = ERR_OK;
	if(*mbox == 0)
	{
		ret = ERR_MEM;
		goto end;
	}
    if (msg == NULL)
    	msg = (void*)&pvNullPointer;

    if (OSQPost((*mbox)->lwip_mbox_e, msg) != OS_ERR_NONE)
    {
    	ret = ERR_MEM;
		goto end;
    }
end:

    return ret;
}
/*
;*****************************************************************************************************
;* 函数名称 : sys_arch_mbox_fetch
;* 描    述 : 从邮箱等待一封邮件
;* 输　	 入 : mbox: 邮箱句柄, msg: 邮件, timeout: 等待超时的微秒数
;*        
;* 输　	 出 : 等待所用的微秒数
;*****************************************************************************************************
;*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, sys_sem_t *sem, void **msg, u32_t timeout)
{
	INT8U err;
	u32_t ticks;
	u32_t wait_ms = tick_ms;
	void *Data;
	int ret = 0;
	if(*mbox == 0)
	{
		ret = OS_ERR_INVALID_OPT;
		goto end;
	}
	
	Data = OSQAccept((*mbox)->lwip_mbox_e, &err);
	
	if (err == OS_ERR_NONE)		
	{
	 	if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
			
	        }
	        else
	        {
	            *msg = Data;
	        }
		ret = OS_ERR_NONE;
		goto end;
	}	
		
	if (timeout == 0)			
	{
		Data = OSQPend((*mbox)->lwip_mbox_e, 0, &err);
			
		if (err == OS_ERR_NONE)	
		{
	        if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
	        }
	        else
	        {
	            *msg = Data;
	        }	
			ret = OS_ERR_NONE;
		}
		else					
		{
			ret = SYS_ARCH_TIMEOUT;
		}
		goto end;
	}
	else
	{
		ticks = (timeout * OS_TICKS_PER_SEC) / 1000;
		
		if (ticks < 1)
		{
			ticks = 1;
		}
		else if (ticks > 65535)
		{
			ticks = 65535;
		}
		Data = OSQPend((*mbox)->lwip_mbox_e, (u16_t)ticks, &err);
		if (err == OS_ERR_NONE)
		{
	        if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
	        }
	        else
	        {
	            *msg = Data;
	        }
				
			ret = OS_ERR_NONE;
		}
		else
		{
			ret = SYS_ARCH_TIMEOUT;
		}
	}

end:
	if(*msg && ((*sem)->OSEventCnt == 0))
	{
		OSSemPost(*sem);
	}
	if(ret == OS_ERR_NONE)
		ret = tick_ms - wait_ms;
	return ret;
}


/*
;*****************************************************************************************************
;* 函数名称 : sys_arch_mbox_tryfetch
;* 描    述 : 尝试从邮箱等待一封邮件
;* 输　	 入 : mbox: 邮箱句柄, msg: 邮件
;*        
;* 输　	 出 : ERROR: SYS_MBOX_EMPTY | OK: 0
;*****************************************************************************************************
;*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	INT8U err;
	void *Data;
	int ret;
	
	if(*mbox == 0)
	{
		ret = OS_ERR_INVALID_OPT;
		goto end;
	}
	
	Data = OSQAccept((*mbox)->lwip_mbox_e, &err);
	
	if (err == OS_ERR_NONE)			
	{
        if (Data == (void*)&pvNullPointer)
        {
            *msg = NULL;
        }
        else
        {
            *msg = Data;
        }
		ret = OS_ERR_NONE;
	}	
	else
	{
        ret = SYS_MBOX_EMPTY;
    }
end:
//	if(ret != OS_ERR_NONE)
//		p_err("sys_arch_mbox_tryfetch err:%d\n",ret);
	return ret;
}

	
/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_free
;* 描    述 : 删除一个邮箱
;* 输　	 入 : mbox: 邮箱句柄
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_mbox_free(sys_mbox_t *mbox,  sys_sem_t *sem)
{
	INT8U err;

	if(*mbox == 0)
	{
		p_err("sys_mbox_free arg err\n");
		return;
	}
	
	OSQFlush((*mbox)->lwip_mbox_e);
	
	OSQDel((*mbox)->lwip_mbox_e, OS_DEL_ALWAYS, &err);
	
	mem_free(*mbox);

/*	
	OSSemPendAbort (*sem, OS_PEND_OPT_BROADCAST, &err);
	if(err != OS_ERR_NONE)

		p_err("sys_mbox_free err1\n");
*/
	OSSemDel (*sem, OS_DEL_ALWAYS, &err);
	if(err != OS_ERR_NONE)

		p_err("sys_mbox_free err2\n");
}


sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio) 
{

	return thread_create(thread,
                        arg,
                        prio,
                        0,
                        stacksize,
                        (char*)name);
}	



/*
;*****************************************************************************************************
;*                            			End Of File
;*****************************************************************************************************
;*/


	 

