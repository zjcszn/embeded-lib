#ifndef _TIMER_H_
#define _TIMER_H_
#include "api.h"

typedef OS_TMR timer_t;
typedef void(*timer_callback_func)(void *parg);

timer_t *timer_setup(int time_val, int type, timer_callback_func callback, void *callback_arg);

int timer_pending(timer_t *tmr);

int mod_timer(timer_t *tmr, unsigned int expires);

int add_timer(timer_t *tmr);

int del_timer(timer_t *tmr);

int timer_free(timer_t *tmr);

void sleep(uint32_t ms);


unsigned int os_time_get(void);

extern volatile uint32_t jiffies, tick_ms;


#endif
