#include <zephyr.h>
#include <FreeRTOS.h>
#include <semphr.h>

void k_queue_init(struct k_queue *queue)
{
    sys_slist_init(&queue->data_q);
    sys_dlist_init(&queue->poll_events);

    k_sem_init(&queue->sem, 0, 10);
}

void k_queue_append(struct k_queue *queue, void *data)
{
    unsigned int key;

    key = irq_lock();
    sys_slist_append(&queue->data_q, data);
    irq_unlock(key);

    k_sem_give(&queue->sem);
}

void k_queue_prepend(struct k_queue *queue, void *data)
{
    unsigned int key;

    key = irq_lock();
    sys_slist_prepend(&queue->data_q, data);
    irq_unlock(key);

    k_sem_give(&queue->sem);
}

int k_queue_append_list(struct k_queue *queue, void *head, void *tail)
{
    unsigned int key;

    key = irq_lock();
    sys_slist_append_list(&queue->data_q, head, tail);
    irq_unlock(key);

    k_sem_give(&queue->sem);
    return 0;
}

void *k_queue_get(struct k_queue *queue, k_timeout_t timeout)
{
    void *buf = NULL;
    unsigned int key;
    int ret;

    ret = k_sem_take(&queue->sem, timeout);
    if (ret < 0) {
        return NULL;
    }

    key = irq_lock();
    buf = sys_slist_get(&queue->data_q);
    irq_unlock(key);
    return buf;
}

int k_queue_is_empty(struct k_queue *queue)
{
    return (int)sys_slist_is_empty(&queue->data_q);
}