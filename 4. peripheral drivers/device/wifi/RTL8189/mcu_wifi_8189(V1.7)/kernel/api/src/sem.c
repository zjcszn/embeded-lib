#include "api.h"
#include "debug.h"
#include "sem.h"

void sem_init(sem_t *sem, int pshard, int value)
{
	*sem = (sem_t)OSSemCreate(value);
	if (*sem == 0)
		p_err("sem_init err");
}

int sem_wait(sem_t *sem, unsigned int timeout)
{
	INT8U perr;

	if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;

	OSSemPend((OS_EVENT*) * sem, timeout, &perr);
	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_TIMEOUT)
		return 1;

	p_err("sem_wait err %d", perr);
	return  - 1;
}

int sem_post(sem_t *sem)
{
	INT8U perr;
	perr = OSSemPost((OS_EVENT*) * sem);
	if (perr == OS_ERR_NONE)
		return 0;

	p_err("sem_post err %d", perr);
	return  - 1;

}

void sem_destory(sem_t *sem)
{
	INT8U perr;
	OSSemDel((OS_EVENT*) * sem, OS_DEL_ALWAYS, &perr);

	if (perr != OS_ERR_NONE)
		p_err("sem_destory err %d", perr);
}
