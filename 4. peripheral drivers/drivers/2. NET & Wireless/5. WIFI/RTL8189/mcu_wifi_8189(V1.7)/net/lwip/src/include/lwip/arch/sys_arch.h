#ifndef __SYS_ARCH_H__
#define __SYS_ARCH_H__


#include  "cpu.h"
#include  "ucos_ii.h"
#include  "lwip/err.h"

#define MAX_MSG_IN_LWIP_MBOX 10

typedef struct tag_LWIP_MBOX
{
	OS_EVENT * lwip_mbox_e;

	void *lwip_msg_q[MAX_MSG_IN_LWIP_MBOX];

} LWIP_MBOX, *sys_mbox_t;


typedef OS_EVENT* sys_mutex_t;
typedef OS_EVENT* sys_sem_t;
typedef INT8U     sys_thread_t;


#ifndef NULL
#define NULL 0
#endif


#define SYS_MBOX_NULL  (sys_mbox_t)NULL
#define SYS_SEM_NULL   (sys_sem_t)NULL

err_t sys_sem_new(sys_sem_t *sem,u8_t count);
void sys_sem_free(sys_sem_t *sem);
void sys_sem_signal(sys_sem_t *sem);


#endif /* __SYS_ARCH_H__ */


