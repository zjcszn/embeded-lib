#include "zephyr.h"
#include "host/hci_core.h"

extern uint32_t _net_buf_pool_list_start;
extern uint32_t _net_buf_pool_list_end;

extern struct k_sem g_poll_sem;
extern struct k_fifo free_tx;

extern int init_mem_slab_module(void);
extern void k_poll_init(void);
extern void k_sys_work_q_init(void);

void zephyr_init(void)
{
    struct net_buf_pool *pool = NULL;

    for (pool = (struct net_buf_pool *)&_net_buf_pool_list_start; pool < (struct net_buf_pool *)&_net_buf_pool_list_end; pool++) {
        k_lifo_init(&pool->free);
    }

    init_mem_slab_module();

    k_sem_init(&g_poll_sem, 0, 1);

#if !defined(CONFIG_BT_WAIT_NOP)
    k_sem_init(&bt_dev.ncmd_sem, 1, 1);
#else
    k_sem_init(&bt_dev.ncmd_sem, 0, 1);
#endif

    k_fifo_init(&bt_dev.cmd_tx_queue);
#if !defined(CONFIG_BT_RECV_IS_RX_THREAD)
    k_fifo_init(&bt_dev.rx_queue);
#endif
    k_fifo_init(&free_tx);

    k_sys_work_q_init();
}