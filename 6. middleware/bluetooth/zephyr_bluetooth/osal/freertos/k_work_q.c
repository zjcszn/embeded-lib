#include <zephyr.h>
#include <common/log.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>

#define K_WORK_STATE_PENDING K_WORK_RUNNING

struct k_work_q k_sys_work_q;

K_KERNEL_STACK_DEFINE(k_sys_work_q_stack, CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE);

static void k_work_submit_to_queue(struct k_work_q *work_q,
                                   struct k_work *work)
{
    if (!atomic_test_and_set_bit(work->flags, K_WORK_STATE_PENDING)) {
        k_fifo_put(&work_q->fifo, &work->node);
    }
}

static int k_delayed_work_submit_to_queue(struct k_work_q *work_q,
                                          struct k_work_delayable *dwork,
                                          uint32_t delay)
{
    int err;

    /* Work cannot be active in multiple queues */
    if (dwork->work_q && dwork->work_q != work_q) {
        err = -EADDRINUSE;
        goto done;
    }

    /* Cancel if work has been submitted */
    if (dwork->work_q == work_q) {
        err = k_work_cancel_delayable(dwork);

        if (err < 0) {
            goto done;
        }
    }

    if (!delay) {
        /* Submit work if no ticks is 0 */
        k_work_submit_to_queue(work_q, &dwork->work);
        dwork->work_q = NULL;
    } else {
        /* Add timeout */
        /* Attach workqueue so the timeout callback can submit it */
        err = xTimerChangePeriod(dwork->timer.timer, pdMS_TO_TICKS(delay), 0);
        BT_ASSERT(err == pdPASS);
        err = xTimerStart(dwork->timer.timer, 0);
        BT_ASSERT(err == pdPASS);

        dwork->start_ms = (uint32_t)k_uptime_get();
        dwork->timeout = delay;
        dwork->work_q = work_q;
    }

    err = 0;

done:
    return err;
}

static void work_timeout(void *timer)
{
    int err;

    struct k_work_delayable *dwork = (struct k_work_delayable *)pvTimerGetTimerID((TimerHandle_t)timer);

    /* submit work to workqueue */
    err = xTimerStop(dwork->timer.timer, 0);
    BT_ASSERT(err == pdPASS);
    if (dwork->work_q) {
        k_work_submit_to_queue(dwork->work_q, &dwork->work);
    }
    /* detach from workqueue, for cancel to return appropriate status */
    dwork->work_q = NULL;
}

static void work_queue_main(void *p1)
{
    struct k_work *work;
    
    ARG_UNUSED(p1);

    while (1) {
        work = k_fifo_get(&k_sys_work_q.fifo, K_FOREVER);

        if (atomic_test_and_clear_bit(work->flags, K_WORK_STATE_PENDING)) {
            work->handler(work);
        }

        k_yield();
    }
}

void k_work_init(struct k_work *work, k_work_handler_t handler)
{
    BT_ASSERT(work);

    atomic_clear_bit(work->flags, K_WORK_RUNNING);
    work->handler = handler;
}

int k_work_submit(struct k_work *work)
{
    k_work_submit_to_queue(&k_sys_work_q, work);
    return 0;
}

int k_work_cancel(struct k_work *work)
{
    return 0;
}

bool k_work_cancel_sync(struct k_work *work, struct k_work_sync *sync)
{
    return true;
}

void k_work_init_delayable(struct k_work_delayable *dwork,
                           k_work_handler_t handler)
{
    BT_ASSERT(dwork);

    k_work_init(&dwork->work, handler);
    dwork->work_q = NULL;
    dwork->start_ms = 0;
    dwork->timeout = 0;
    dwork->timer.timer = (void *)xTimerCreate("workq_tim", pdMS_TO_TICKS(1000), 0, dwork, (TimerCallbackFunction_t)work_timeout);
}

struct k_work_delayable *k_work_delayable_from_work(struct k_work *work)
{
    return CONTAINER_OF(work, struct k_work_delayable, work);
}

k_ticks_t k_work_delayable_remaining_get(const struct k_work_delayable *dwork)
{
    k_ticks_t remain;

    if (dwork == NULL) {
        return 0;
    }

    remain = dwork->timeout - ((int32_t)k_uptime_get() - dwork->start_ms);
    if (remain < 0) {
        remain = 0;
    }
    return remain;
}

int k_work_schedule(struct k_work_delayable *dwork,
                    k_timeout_t delay)
{
    return k_delayed_work_submit_to_queue(&k_sys_work_q, dwork, delay);
}

int k_work_reschedule(struct k_work_delayable *dwork,
                      k_timeout_t delay)
{
    return k_work_schedule(dwork, delay);
}

int k_work_cancel_delayable(struct k_work_delayable *dwork)
{
    int err = 0;

    if (atomic_test_bit(dwork->work.flags, K_WORK_STATE_PENDING)) {
        err = -EINPROGRESS;
        goto exit;
    }

    if (!dwork->work_q) {
        err = -EINVAL;
        goto exit;
    }

    err = xTimerStop(dwork->timer.timer, 0);
    BT_ASSERT(err == pdPASS);
    dwork->work_q = NULL;
    dwork->timeout = 0;
    dwork->start_ms = 0;

exit:
    return err;
}

bool k_work_cancel_delayable_sync(struct k_work_delayable *dwork,
                                  struct k_work_sync *sync)
{
    return k_work_cancel_delayable(dwork);
}

void k_sys_work_q_init(void)
{
    k_fifo_init(&k_sys_work_q.fifo);
    k_sys_work_q.thread = k_thread_create("sys_workq", k_sys_work_q_stack, CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE,
                                          work_queue_main, NULL, CONFIG_SYSTEM_WORKQUEUE_PRIORITY);
}
