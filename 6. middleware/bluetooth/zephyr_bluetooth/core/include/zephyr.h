#ifndef ZEPHYR_INCLUDE_H_
#define ZEPHYR_INCLUDE_H_

#define CONFIG_RISCV
#define CONFIG_ASSERT_VERBOSE
#define __ASSERT_ON 2

#include <bt_config.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/errno.h>

#include <toolchain.h>
#include <sys/__assert.h>
#include <sys/util.h>
#include <sys/byteorder.h>
#include <sys/slist.h>
#include <sys/dlist.h>
#include <sys/check.h>
#include <sys/util_macro.h>
#include <sys/atomic.h>
#include <linker/sections.h>

// clang-format off
#define SYS_PORT_TRACING_FUNC(type, func, ...) do { } while (false)
#define SYS_PORT_TRACING_FUNC_ENTER(type, func, ...) do { } while (false)
#define SYS_PORT_TRACING_FUNC_BLOCKING(type, func, ...) do { } while (false)
#define SYS_PORT_TRACING_FUNC_EXIT(type, func, ...) do { } while (false)
#define SYS_PORT_TRACING_OBJ_INIT(obj_type, obj, ...) do { } while (false)
#define SYS_PORT_TRACING_OBJ_FUNC(obj_type, func, obj, ...) do { } while (false)
#define SYS_PORT_TRACING_OBJ_FUNC_ENTER(obj_type, func, obj, ...) do { } while (false)
#define SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(obj_type, func, obj, ...) do { } while (false)
#define SYS_PORT_TRACING_OBJ_FUNC_EXIT(obj_type, func, obj, ...) do { } while (false)
// clang-format on

#define K_TIMEOUT_EQ(a, b)       ((a) == (b))
#define Z_TIMEOUT_TICKS(t)       (t)
#define K_PRIO_COOP(x)           (x)
#define K_PRIO_PREEMPT(x)        (x)
#define k_ticks_to_ms_floor32(x) x

#define K_FOREVER                0xffffffff
#define K_NO_WAIT                0

#define MSEC_PER_SEC             1000
#define K_MSEC(ms)               (ms)
#define K_SECONDS(s)             K_MSEC((s)*MSEC_PER_SEC)
#define K_MINUTES(m)             K_SECONDS((m)*60)
#define K_HOURS(h)               K_MINUTES((h)*60)

#define printk                   printf

typedef int32_t k_ticks_t;
typedef uint32_t k_timeout_t;
typedef unsigned int k_spinlock_key_t;
typedef uint8_t k_thread_stack_t;

struct k_spinlock {
    uint32_t data;
};

#include <k_os.h>
#include <k_queue.h>
#include <k_work_q.h>
#include <k_poll.h>
#include <k_mem_slab.h>
#include <net/buf.h>
#include <settings/settings.h>
#include <drivers/hci_driver.h>

#endif