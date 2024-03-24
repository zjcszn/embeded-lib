#ifndef _MSG_Q_H_
#define _MSG_Q_H_
#include "api.h"

typedef OS_EVENT *msg_q_t;

int msgget(msg_q_t *msg_q, int q_size);
int msgsnd(msg_q_t msgid, void *msgbuf);
int msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout);
int msgfree(msg_q_t msg_q);

#endif
