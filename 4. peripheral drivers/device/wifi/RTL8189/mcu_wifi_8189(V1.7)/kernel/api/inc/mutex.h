#ifndef _MUTEX_H_
#define _MUTEX_H_
#include "api.h"

typedef OS_EVENT *mutex_t;

mutex_t mutex_init(const char *name);

int mutex_lock(mutex_t mutex);

int mutex_unlock(mutex_t mutex);

void mutex_destory(mutex_t mutex);

#define mutex_free mutex_destory

#endif
