#ifndef _HARD_TIMER_H
#define _HARD_TIMER_H

void start_timer(uint32_t freq, void *call_back);
void pause_timer(void);
void continue_timer(void);
#endif
