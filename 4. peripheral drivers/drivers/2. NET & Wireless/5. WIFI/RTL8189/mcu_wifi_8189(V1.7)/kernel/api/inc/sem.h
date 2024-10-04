#ifndef _SEM_H_
#define _SEM_H_
#include "api.h"

#include "api.h"

typedef OS_EVENT *sem_t;

void sem_init(sem_t *sem, int pshard, int value);

int sem_wait(sem_t *sem, unsigned int timeout);

int sem_post(sem_t *sem);

void sem_destory(sem_t *sem);

#endif
