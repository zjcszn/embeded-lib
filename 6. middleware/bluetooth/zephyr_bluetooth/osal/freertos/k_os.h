#ifndef K_OS_H_
#define K_OS_H_

struct k_sem {
	void *sem;
	const char *name;
	sys_dlist_t poll_events;
};

#define Z_SEM_INITIALIZER(obj, initial_count, count_limit) { .name = #obj }

/**
 * @defgroup semaphore_apis Semaphore APIs
 * @ingroup kernel_apis
 * @{
 */

/**
 * @brief Maximum limit value allowed for a semaphore.
 *
 * This is intended for use when a semaphore does not have
 * an explicit maximum limit, and instead is just used for
 * counting purposes.
 *
 */
#define K_SEM_MAX_LIMIT UINT_MAX

/**
 * @brief Initialize a semaphore.
 *
 * This routine initializes a semaphore object, prior to its first use.
 *
 * @param sem Address of the semaphore.
 * @param initial_count Initial semaphore count.
 * @param limit Maximum permitted semaphore count.
 *
 * @see K_SEM_MAX_LIMIT
 *
 * @retval 0 Semaphore created successfully
 * @retval -EINVAL Invalid values
 *
 */
int k_sem_init(struct k_sem *sem, unsigned int initial_count,
			  unsigned int limit);

/**
 * @brief Take a semaphore.
 *
 * This routine takes @a sem.
 *
 * @note @a timeout must be set to K_NO_WAIT if called from ISR.
 *
 * @funcprops \isr_ok
 *
 * @param sem Address of the semaphore.
 * @param timeout Waiting period to take the semaphore,
 *                or one of the special values K_NO_WAIT and K_FOREVER.
 *
 * @retval 0 Semaphore taken.
 * @retval -EBUSY Returned without waiting.
 * @retval -EAGAIN Waiting period timed out,
 *			or the semaphore was reset during the waiting period.
 */
int k_sem_take(struct k_sem *sem, k_timeout_t timeout);

/**
 * @brief Give a semaphore.
 *
 * This routine gives @a sem, unless the semaphore is already at its maximum
 * permitted count.
 *
 * @funcprops \isr_ok
 *
 * @param sem Address of the semaphore.
 *
 * @return N/A
 */
void k_sem_give(struct k_sem *sem);

/**
 * @brief Resets a semaphore's count to zero.
 *
 * This routine sets the count of @a sem to zero.
 * Any outstanding semaphore takes will be aborted
 * with -EAGAIN.
 *
 * @param sem Address of the semaphore.
 *
 * @return N/A
 */
void k_sem_reset(struct k_sem *sem);

/**
 * @brief Get a semaphore's count.
 *
 * This routine returns the current count of @a sem.
 *
 * @param sem Address of the semaphore.
 *
 * @return Current semaphore count.
 */
unsigned int k_sem_count_get(struct k_sem *sem);

/**
 * @brief Statically define and initialize a semaphore.
 *
 * The semaphore can be accessed outside the module where it is defined using:
 *
 * @code extern struct k_sem <name>; @endcode
 *
 * @param name Name of the semaphore.
 * @param initial_count Initial semaphore count.
 * @param count_limit Maximum permitted semaphore count.
 */
#define K_SEM_DEFINE(name, initial_count, count_limit) \
	STRUCT_SECTION_ITERABLE(k_sem, name) = \
		Z_SEM_INITIALIZER(name, initial_count, count_limit); \
	BUILD_ASSERT(((count_limit) != 0) && \
		     ((initial_count) <= (count_limit)) && \
			 ((count_limit) <= K_SEM_MAX_LIMIT));

typedef void (*k_thread_entry_t)(void *arg);

typedef void *k_tid_t;

#define K_KERNEL_STACK_DEFINE(sym, size) uint8_t __aligned(8) sym[size];
#define K_KERNEL_STACK_SIZEOF(sym) (sizeof(sym))

/**
 * @brief Create a thread.
 *
 * This routine initializes a thread, then schedules it for execution.
 *
 * The new thread may be scheduled for immediate execution or a delayed start.
 * If the newly spawned thread does not have a delayed start the kernel
 * scheduler may preempt the current thread to allow the new thread to
 * execute.
 *
 * Thread options are architecture-specific, and can include K_ESSENTIAL,
 * K_FP_REGS, and K_SSE_REGS. Multiple options may be specified by separating
 * them using "|" (the logical OR operator).
 *
 * Stack objects passed to this function must be originally defined with
 * either of these macros in order to be portable:
 *
 * - K_THREAD_STACK_DEFINE() - For stacks that may support either user or
 *   supervisor threads.
 * - K_KERNEL_STACK_DEFINE() - For stacks that may support supervisor
 *   threads only. These stacks use less memory if CONFIG_USERSPACE is
 *   enabled.
 *
 * The stack_size parameter has constraints. It must either be:
 *
 * - The original size value passed to K_THREAD_STACK_DEFINE() or
 *   K_KERNEL_STACK_DEFINE()
 * - The return value of K_THREAD_STACK_SIZEOF(stack) if the stack was
 *   defined with K_THREAD_STACK_DEFINE()
 * - The return value of K_KERNEL_STACK_SIZEOF(stack) if the stack was
 *   defined with K_KERNEL_STACK_DEFINE().
 *
 * Using other values, or sizeof(stack) may produce undefined behavior.
 *
 * @param new_thread Pointer to uninitialized struct k_thread
 * @param stack Pointer to the stack space.
 * @param stack_size Stack size in bytes.
 * @param entry Thread entry function.
 * @param p1 1st entry point parameter.
 * @param p2 2nd entry point parameter.
 * @param p3 3rd entry point parameter.
 * @param prio Thread priority.
 * @param options Thread options.
 * @param delay Scheduling delay, or K_NO_WAIT (for no delay).
 *
 * @return ID of new thread.
 *
 */
k_tid_t k_thread_create(const char *name,
				  k_thread_stack_t *stack,
				  size_t stack_size,
				  k_thread_entry_t entry,
				  void *args,
				  int prio);

/**
 * @brief Abort a thread.
 *
 * This routine permanently stops execution of @a thread. The thread is taken
 * off all kernel queues it is part of (i.e. the ready queue, the timeout
 * queue, or a kernel object wait queue). However, any kernel resources the
 * thread might currently own (such as mutexes or memory blocks) are not
 * released. It is the responsibility of the caller of this routine to ensure
 * all necessary cleanup is performed.
 *
 * After k_thread_abort() returns, the thread is guaranteed not to be
 * running or to become runnable anywhere on the system.  Normally
 * this is done via blocking the caller (in the same manner as
 * k_thread_join()), but in interrupt context on SMP systems the
 * implementation is required to spin for threads that are running on
 * other CPUs.  Note that as specified, this means that on SMP
 * platforms it is possible for application code to create a deadlock
 * condition by simultaneously aborting a cycle of threads using at
 * least one termination from interrupt context.  Zephyr cannot detect
 * all such conditions.
 *
 * @param thread ID of thread to abort.
 *
 * @return N/A
 */
void k_thread_abort(k_tid_t thread);

/**
 * @brief Yield the current thread.
 *
 * This routine causes the current thread to yield execution to another
 * thread of the same or higher priority. If there are no other ready threads
 * of the same or higher priority, the routine returns immediately.
 *
 * @return N/A
 */
void k_yield(void);

/**
 * @brief Get thread ID of the current thread.
 *
 * @return ID of current thread.
 *
 */
k_tid_t k_current_get(void);

/**
 * @brief Get system uptime.
 *
 * This routine returns the elapsed time since the system booted,
 * in milliseconds.
 *
 * @note
 *    While this function returns time in milliseconds, it does
 *    not mean it has millisecond resolution. The actual resolution depends on
 *    @kconfig{CONFIG_SYS_CLOCK_TICKS_PER_SEC} config option.
 *
 * @return Current uptime in milliseconds.
 */
int64_t k_uptime_get(void);

#define Z_TICK_ABS(t) (-1 - 1 - (t))

/* Returns the uptime expiration (relative to an unlocked "now"!) of a
 * timeout object.  When used correctly, this should be called once,
 * synchronously with the user passing a new timeout value.  It should
 * not be used iteratively to adjust a timeout.
 */
static inline uint64_t sys_clock_timeout_end_calc(k_timeout_t timeout)
{
	k_ticks_t dt;

	if (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		return UINT64_MAX;
	} else if (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		return k_uptime_get();
	} else {

		dt = timeout;

		if (IS_ENABLED(CONFIG_TIMEOUT_64BIT) && Z_TICK_ABS(dt) >= 0) {
			return Z_TICK_ABS(dt);
		}
		return k_uptime_get() + MAX(1, dt);
	}
}

unsigned int irq_lock(void);

void irq_unlock(unsigned int key);

bool k_is_in_isr(void);

void k_sleep(uint32_t ms);

static inline k_spinlock_key_t k_spin_lock(struct k_spinlock *lock)
{
	return irq_lock();
}

static inline void k_spin_unlock(struct k_spinlock *lock, k_spinlock_key_t key)
{
	irq_unlock(key);
}

#endif