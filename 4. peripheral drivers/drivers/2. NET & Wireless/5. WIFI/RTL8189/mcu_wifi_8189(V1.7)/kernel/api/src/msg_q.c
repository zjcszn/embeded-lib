#include "api.h"
#include "debug.h"
#include "msg_q.h"

struct _msg_q_manage
{
	msg_q_t msg;
	void **q_start;
};

struct _msg_q_manage q_man[OS_MAX_QS];

int msgget(msg_q_t *msg_q, int q_size)
{
	OS_EVENT *msg;
	void **q_start = NULL;
	int i;

	for (i = 0; i < OS_MAX_QS; i++)
	{
		if (q_man[i].msg == 0)
			break;
	} if (i >= OS_MAX_QS)
	{
		p_err("msgget: no more msg queen");
		return  - 1;
	}

	q_start = (void **)mem_malloc(q_size *(sizeof(void*)));
	if (q_start == 0)
	{
		p_err("msgget: malloc queen err.no enough malloc");
		return  - 1;
	}

	msg = OSQCreate(q_start, (INT16U)q_size);
	if (msg == 0)
	{
		*msg_q = 0;
		mem_free(q_start);
		return  - 1;
	}

	*msg_q = (msg_q_t)msg;
	q_man[i].msg = (msg_q_t)msg;
	q_man[i].q_start = q_start;

	return 0;
}

int msgsnd(msg_q_t msgid, void *msgbuf)
{
	INT8U perr;

	if (0 == msgid)
	{
		p_err("msgsnd: msgid err");
		return  - 1;
	}
	perr = OSQPost((OS_EVENT*)msgid, (void*)msgbuf);

	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_Q_FULL)
		return 1;

	p_err("msgsnd: err %d", perr);
	return  - 1;
}

int msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout)
{
	INT8U perr;

	if (0 == msgid)
	{
		p_err("msgrcv: msgid err");
		return  - 1;
	}

	if ((timeout > 0) && (timeout < 1000uL / OS_TICKS_PER_SEC))
		timeout = 1000uL / OS_TICKS_PER_SEC;

	timeout = timeout * OS_TICKS_PER_SEC / 1000uL;

	*msgbuf = OSQPend((OS_EVENT*)msgid, (INT32U)timeout, &perr);

	if (perr == OS_ERR_NONE)
		return 0;
	else if (perr == OS_ERR_TIMEOUT)
		return 1;

	p_err("msgrcv: err %d", perr);
	return  - 1;
}

int msgfree(msg_q_t msgid)
{
	int i;
	INT8U perr;

	for (i = 0; i < OS_MAX_QS; i++)
	{
		if (q_man[i].msg == msgid)
			break;
	}
	if (i >= OS_MAX_QS)
	{
		p_err("msgfree: err no match msg_q %x", msgid);
		return  - 1;
	}

	OSQDel((OS_EVENT*)msgid, OS_DEL_ALWAYS, &perr);
	if (OS_ERR_NONE != perr)
	{
		p_err("msgfree: free err %d", perr);
		return  - 1;
	}

	if (!q_man[i].q_start)
	{
		assert(0);
	}

	mem_free(q_man[i].q_start);
	q_man[i].q_start = 0;
	q_man[i].msg = 0;

	return 0;
}

void msg_q_init(void)
{
	int i;
	for (i = 0; i < OS_MAX_QS; i++)
	{
		q_man[i].msg = 0;
		q_man[i].q_start = 0;
	}
}
