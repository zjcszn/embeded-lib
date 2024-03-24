#ifndef WORK_Q_H_
#define WORK_Q_H_

struct k_work;
struct k_work_q;

struct k_timer {
    void *timer;
};

extern struct k_work_q k_sys_work_q;

/**
 * @cond INTERNAL_HIDDEN
 */

struct k_work_delayable;
struct k_work_sync;

/**
 * INTERNAL_HIDDEN @endcond
 */

/**
 * @defgroup workqueue_apis Work Queue APIs
 * @ingroup kernel_apis
 * @{
 */

/** @brief The signature for a work item handler function.
 *
 * The function will be invoked by the thread animating a work queue.
 *
 * @param work the work item that provided the handler.
 */
typedef void (*k_work_handler_t)(struct k_work *work);

/** @brief Initialize a (non-delayable) work structure.
 *
 * This must be invoked before submitting a work structure for the first time.
 * It need not be invoked again on the same work structure.  It can be
 * re-invoked to change the associated handler, but this must be done when the
 * work item is idle.
 *
 * @funcprops \isr_ok
 *
 * @param work the work structure to be initialized.
 *
 * @param handler the handler to be invoked by the work item.
 */
void k_work_init(struct k_work *work,
		  k_work_handler_t handler);

/** @brief Submit a work item to the system queue.
 *
 * @funcprops \isr_ok
 *
 * @param work pointer to the work item.
 *
 * @return as with k_work_submit_to_queue().
 */
int k_work_submit(struct k_work *work);

/** @brief Cancel a work item.
 *
 * This attempts to prevent a pending (non-delayable) work item from being
 * processed by removing it from the work queue.  If the item is being
 * processed, the work item will continue to be processed, but resubmissions
 * are rejected until cancellation completes.
 *
 * If this returns zero cancellation is complete, otherwise something
 * (probably a work queue thread) is still referencing the item.
 *
 * See also k_work_cancel_sync().
 *
 * @funcprops \isr_ok
 *
 * @param work pointer to the work item.
 *
 * @return the k_work_busy_get() status indicating the state of the item after all
 * cancellation steps performed by this call are completed.
 */
int k_work_cancel(struct k_work *work);

/** @brief Cancel a work item and wait for it to complete.
 *
 * Same as k_work_cancel() but does not return until cancellation is complete.
 * This can be invoked by a thread after k_work_cancel() to synchronize with a
 * previous cancellation.
 *
 * On return the work structure will be idle unless something submits it after
 * the cancellation was complete.
 *
 * @note Be careful of caller and work queue thread relative priority.  If
 * this function sleeps it will not return until the work queue thread
 * completes the tasks that allow this thread to resume.
 *
 * @note Behavior is undefined if this function is invoked on @p work from a
 * work queue running @p work.
 *
 * @param work pointer to the work item.
 *
 * @param sync pointer to an opaque item containing state related to the
 * pending cancellation.  The object must persist until the call returns, and
 * be accessible from both the caller thread and the work queue thread.  The
 * object must not be used for any other flush or cancel operation until this
 * one completes.  On architectures with CONFIG_KERNEL_COHERENCE the object
 * must be allocated in coherent memory.
 *
 * @retval true if work was pending (call had to wait for cancellation of a
 * running handler to complete, or scheduled or submitted operations were
 * cancelled);
 * @retval false otherwise
 */
bool k_work_cancel_sync(struct k_work *work, struct k_work_sync *sync);

/** @brief Initialize a delayable work structure.
 *
 * This must be invoked before scheduling a delayable work structure for the
 * first time.  It need not be invoked again on the same work structure.  It
 * can be re-invoked to change the associated handler, but this must be done
 * when the work item is idle.
 *
 * @funcprops \isr_ok
 *
 * @param dwork the delayable work structure to be initialized.
 *
 * @param handler the handler to be invoked by the work item.
 */
void k_work_init_delayable(struct k_work_delayable *dwork,
			   k_work_handler_t handler);

/**
 * @brief Get the parent delayable work structure from a work pointer.
 *
 * This function is necessary when a @c k_work_handler_t function is passed to
 * k_work_schedule_for_queue() and the handler needs to access data from the
 * container of the containing `k_work_delayable`.
 *
 * @param work Address passed to the work handler
 *
 * @return Address of the containing @c k_work_delayable structure.
 */
struct k_work_delayable *k_work_delayable_from_work(struct k_work *work);

/** @brief Get the number of ticks until a scheduled delayable work will be
 * submitted.
 *
 * @note This is a live snapshot of state, which may change before the result
 * can be inspected.  Use locks where appropriate.
 *
 * @funcprops \isr_ok
 *
 * @param dwork pointer to the delayable work item.
 *
 * @return the number of ticks until the timer that will schedule the work
 * item will expire, or zero if the item is not scheduled.
 */
k_ticks_t k_work_delayable_remaining_get(const struct k_work_delayable *dwork);

/** @brief Submit an idle work item to the system work queue after a
 * delay.
 *
 * This is a thin wrapper around k_work_schedule_for_queue(), with all the API
 * characteristcs of that function.
 *
 * @param dwork pointer to the delayable work item.
 *
 * @param delay the time to wait before submitting the work item.  If @c
 * K_NO_WAIT this is equivalent to k_work_submit_to_queue().
 *
 * @return as with k_work_schedule_for_queue().
 */
int k_work_schedule(struct k_work_delayable *dwork,
				   k_timeout_t delay);

/** @brief Reschedule a work item to the system work queue after a
 * delay.
 *
 * This is a thin wrapper around k_work_reschedule_for_queue(), with all the
 * API characteristcs of that function.
 *
 * @param dwork pointer to the delayable work item.
 *
 * @param delay the time to wait before submitting the work item.
 *
 * @return as with k_work_reschedule_for_queue().
 */
int k_work_reschedule(struct k_work_delayable *dwork,
				     k_timeout_t delay);

/** @brief Cancel delayable work.
 *
 * Similar to k_work_cancel() but for delayable work.  If the work is
 * scheduled or submitted it is canceled.  This function does not wait for the
 * cancellation to complete.
 *
 * @note The work may still be running when this returns.  Use
 * k_work_flush_delayable() or k_work_cancel_delayable_sync() to ensure it is
 * not running.
 *
 * @note Canceling delayable work does not prevent rescheduling it.  It does
 * prevent submitting it until the cancellation completes.
 *
 * @funcprops \isr_ok
 *
 * @param dwork pointer to the delayable work item.
 *
 * @return the k_work_delayable_busy_get() status indicating the state of the
 * item after all cancellation steps performed by this call are completed.
 */
int k_work_cancel_delayable(struct k_work_delayable *dwork);

/** @brief Cancel delayable work and wait.
 *
 * Like k_work_cancel_delayable() but waits until the work becomes idle.
 *
 * @note Canceling delayable work does not prevent rescheduling it.  It does
 * prevent submitting it until the cancellation completes.
 *
 * @note Be careful of caller and work queue thread relative priority.  If
 * this function sleeps it will not return until the work queue thread
 * completes the tasks that allow this thread to resume.
 *
 * @note Behavior is undefined if this function is invoked on @p dwork from a
 * work queue running @p dwork.
 *
 * @param dwork pointer to the delayable work item.
 *
 * @param sync pointer to an opaque item containing state related to the
 * pending cancellation.  The object must persist until the call returns, and
 * be accessible from both the caller thread and the work queue thread.  The
 * object must not be used for any other flush or cancel operation until this
 * one completes.  On architectures with CONFIG_KERNEL_COHERENCE the object
 * must be allocated in coherent memory.
 *
 * @retval true if work was not idle (call had to wait for cancellation of a
 * running handler to complete, or scheduled or submitted operations were
 * cancelled);
 * @retval false otherwise
 */
bool k_work_cancel_delayable_sync(struct k_work_delayable *dwork,
				  struct k_work_sync *sync);

enum {
/**
 * @cond INTERNAL_HIDDEN
 */

	/* The atomic API is used for all work and queue flags fields to
	 * enforce sequential consistency in SMP environments.
	 */

	/* Bits that represent the work item states.  At least nine of the
	 * combinations are distinct valid stable states.
	 */
	K_WORK_RUNNING_BIT = 0,
	K_WORK_CANCELING_BIT = 1,
	K_WORK_QUEUED_BIT = 2,
	K_WORK_DELAYED_BIT = 3,

	K_WORK_MASK = BIT(K_WORK_DELAYED_BIT) | BIT(K_WORK_QUEUED_BIT)
		| BIT(K_WORK_RUNNING_BIT) | BIT(K_WORK_CANCELING_BIT),

	/* Static work flags */
	K_WORK_DELAYABLE_BIT = 8,
	K_WORK_DELAYABLE = BIT(K_WORK_DELAYABLE_BIT),

	/* Dynamic work queue flags */
	K_WORK_QUEUE_STARTED_BIT = 0,
	K_WORK_QUEUE_STARTED = BIT(K_WORK_QUEUE_STARTED_BIT),
	K_WORK_QUEUE_BUSY_BIT = 1,
	K_WORK_QUEUE_BUSY = BIT(K_WORK_QUEUE_BUSY_BIT),
	K_WORK_QUEUE_DRAIN_BIT = 2,
	K_WORK_QUEUE_DRAIN = BIT(K_WORK_QUEUE_DRAIN_BIT),
	K_WORK_QUEUE_PLUGGED_BIT = 3,
	K_WORK_QUEUE_PLUGGED = BIT(K_WORK_QUEUE_PLUGGED_BIT),

	/* Static work queue flags */
	K_WORK_QUEUE_NO_YIELD_BIT = 8,
	K_WORK_QUEUE_NO_YIELD = BIT(K_WORK_QUEUE_NO_YIELD_BIT),

/**
 * INTERNAL_HIDDEN @endcond
 */
	/* Transient work flags */

	/** @brief Flag indicating a work item that is running under a work
	 * queue thread.
	 *
	 * Accessed via k_work_busy_get().  May co-occur with other flags.
	 */
	K_WORK_RUNNING = BIT(K_WORK_RUNNING_BIT),

	/** @brief Flag indicating a work item that is being canceled.
	 *
	 * Accessed via k_work_busy_get().  May co-occur with other flags.
	 */
	K_WORK_CANCELING = BIT(K_WORK_CANCELING_BIT),

	/** @brief Flag indicating a work item that has been submitted to a
	 * queue but has not started running.
	 *
	 * Accessed via k_work_busy_get().  May co-occur with other flags.
	 */
	K_WORK_QUEUED = BIT(K_WORK_QUEUED_BIT),

	/** @brief Flag indicating a delayed work item that is scheduled for
	 * submission to a queue.
	 *
	 * Accessed via k_work_busy_get().  May co-occur with other flags.
	 */
	K_WORK_DELAYED = BIT(K_WORK_DELAYED_BIT),
};

/** @brief A structure used to submit work. */
struct k_work {
	sys_snode_t node;
	/* The function to be invoked by the work queue thread. */
	k_work_handler_t handler;

	/* State of the work item.
	 *
	 * The item can be DELAYED, QUEUED, and RUNNING simultaneously.
	 *
	 * It can be RUNNING and CANCELING simultaneously.
	 */
	atomic_t flags[1];
};

#define Z_WORK_INITIALIZER(work_handler) { \
	.handler = work_handler, \
}

/** @brief A structure used to submit work after a delay. */
struct k_work_delayable {
	struct k_work work;
	struct k_work_q *work_q;
	struct k_timer timer;
    uint32_t start_ms;
    uint32_t timeout;
};

#define Z_WORK_DELAYABLE_INITIALIZER(work_handler) { \
	.work = { \
		.handler = work_handler, \
		.flags = K_WORK_DELAYABLE, \
	}, \
}

/**
 * @brief Initialize a statically-defined delayable work item.
 *
 * This macro can be used to initialize a statically-defined delayable
 * work item, prior to its first use. For example,
 *
 * @code static K_WORK_DELAYABLE_DEFINE(<dwork>, <work_handler>); @endcode
 *
 * Note that if the runtime dependencies support initialization with
 * k_work_init_delayable() using that will eliminate the initialized
 * object in ROM that is produced by this macro and copied in at
 * system startup.
 *
 * @param work Symbol name for delayable work item object
 * @param work_handler Function to invoke each time work item is processed.
 */
#define K_WORK_DELAYABLE_DEFINE(work, work_handler) \
	struct k_work_delayable work \
	  = Z_WORK_DELAYABLE_INITIALIZER(work_handler)

/**
 * @cond INTERNAL_HIDDEN
 */

/* Record used to wait for work to flush.
 *
 * The work item is inserted into the queue that will process (or is
 * processing) the item, and will be processed as soon as the item
 * completes.  When the flusher is processed the semaphore will be
 * signaled, releasing the thread waiting for the flush.
 */
struct z_work_flusher {
	struct k_work work;
	struct k_sem sem;
};

/* Record used to wait for work to complete a cancellation.
 *
 * The work item is inserted into a global queue of pending cancels.
 * When a cancelling work item goes idle any matching waiters are
 * removed from pending_cancels and are woken.
 */
struct z_work_canceller {
	sys_snode_t node;
	struct k_work *work;
	struct k_sem sem;
};

/**
 * INTERNAL_HIDDEN @endcond
 */

/** @brief A structure holding internal state for a pending synchronous
 * operation on a work item or queue.
 *
 * Instances of this type are provided by the caller for invocation of
 * k_work_flush(), k_work_cancel_sync() and sibling flush and cancel APIs.  A
 * referenced object must persist until the call returns, and be accessible
 * from both the caller thread and the work queue thread.
 *
 * @note If CONFIG_KERNEL_COHERENCE is enabled the object must be allocated in
 * coherent memory; see arch_mem_coherent().  The stack on these architectures
 * is generally not coherent.  be stack-allocated.  Violations are detected by
 * runtime assertion.
 */
struct k_work_sync {
	union {
		struct z_work_flusher flusher;
		struct z_work_canceller canceller;
	};
};

/** @brief A structure used to hold work until it can be processed. */
struct k_work_q {
	/* The thread that animates the work. */
	k_tid_t thread;
	struct k_sem sem;
	struct k_fifo fifo;
};

#endif
