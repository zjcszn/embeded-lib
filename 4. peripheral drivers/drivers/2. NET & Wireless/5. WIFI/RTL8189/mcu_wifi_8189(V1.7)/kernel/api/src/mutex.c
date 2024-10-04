#include "api.h"
#include "debug.h"
#include "mutex.h"


mutex_t mutex_init(const char *name)
{
	INT8U err;
	mutex_t ret;
	ret =  (mutex_t)OSSemCreate(1);
	if(ret){
		OSEventNameSet(ret, (INT8U*)name, &err);
	}
	return ret;
}

int mutex_lock(mutex_t mutex)
{
	INT8U perr;

	OSSemPend((OS_EVENT*)mutex, 0, &perr);
	if (perr == OS_ERR_NONE)
		return 0;

	p_err("mutex_lock err %d", perr);
	return  - 1;
}

int mutex_unlock(mutex_t mutex)
{
	INT8U perr;

	perr = OSSemPost((OS_EVENT*)mutex);
	if (perr == OS_ERR_NONE)
		return 0;

	p_err("mutex_unlock err %d", perr);
	return  - 1;
}

void mutex_destory(mutex_t mutex)
{
	INT8U perr;
	OSSemDel((OS_EVENT*)mutex, OS_DEL_ALWAYS, &perr);

	if (perr != OS_ERR_NONE)
		p_err("mutex_destory err %d", perr);
}
