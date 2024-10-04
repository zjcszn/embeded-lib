#include "bsp_timer.h"
#include "bsp_beep.h"
#include "bsp_key.h"
#include "lwkey.h"
#include "gd32f30x_timer.h"

void bsp_timer0_init(void) {
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    timer_break_parameter_struct timer_breakpara;

    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);

    /* TIMER0 configuration freq 10khz */
    timer_initpara.prescaler         = 119;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 99;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);

   
    /* TIMER0 primary output function enable */
    timer_primary_output_config(TIMER0,DISABLE);

    /* TIMER0 channel control update interrupt enable */
    nvic_irq_enable(TIMER0_UP_TIMER9_IRQn, 0, 1);

    timer_interrupt_enable(TIMER0,TIMER_INT_UP);
    /* TIMER0 break interrupt disable */
    timer_interrupt_disable(TIMER0,TIMER_INT_BRK);

    /* TIMER0 counter enable */
    timer_enable(TIMER0);
}


void TIMER0_UP_TIMER9_IRQHandler(void) {

    static uint32_t ticks;
    if (timer_interrupt_flag_get(TIMER0, TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(TIMER0,TIMER_INT_FLAG_UP);
        bsp_beep_task();

        /* KEY Task 周期： 128 * 100us, 12.8ms */
        if (!(ticks++ & 0x7F)) {
            lwkey_task(NULL);
        }
    }

}