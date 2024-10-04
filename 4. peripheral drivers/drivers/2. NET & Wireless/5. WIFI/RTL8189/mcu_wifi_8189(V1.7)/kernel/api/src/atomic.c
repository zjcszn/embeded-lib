#include "api.h"
#include "atomic.h"


int atomic_test_set(atomic *at, int val)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();
	if (at->val)
	{
		OS_EXIT_CRITICAL();
		return 1;
	}
	at->val = val;
	OS_EXIT_CRITICAL();
	return 0;
}

void atomic_set(atomic *at, int val)
{
	OS_CPU_SR cpu_sr;
	OS_ENTER_CRITICAL();
	at->val = val;
	OS_EXIT_CRITICAL();
}


int atomic_read(volatile atomic_t *v)
{
	int val;
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	val = v->val;
	local_irq_restore(cpu_sr);
	return val;
}

void atomic_add(int i, volatile atomic_t *v)
{
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	v->val += i;
	local_irq_restore(cpu_sr);
}

void atomic_sub(int i, volatile atomic_t *v)
{
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	v->val -= i;
	local_irq_restore(cpu_sr);
}

int atomic_add_return(int i, atomic_t *v)
{
	int temp;
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	temp = v->val;
	temp += i;
	v->val = temp;
	local_irq_restore(cpu_sr);

	return temp;
}

int  atomic_sub_return(int i, atomic_t *v)
{
	unsigned long temp;
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	temp = v->val;
	temp -= i;
	v->val = temp;
	local_irq_restore(cpu_sr);
	
	return temp;
}


