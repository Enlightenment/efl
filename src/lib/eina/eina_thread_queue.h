#ifndef EINA_THREAD_QUEUE_H_
#define EINA_THREAD_QUEUE_H_

/**
 * @addtogroup Eina_Thread_Queue_Group Thread Queue Group
 * @ingroup Eina
 *
 * @brief These functions provide simple zero-copy message queues for threads
 *
 * @{
 *
 * @since 1.11
 */

/**
 * @typedef Eina_Thread_Queue
 *
 * This is a uni-directional zero-copy thread message queue specifically
 * designed with the idea of sending large volumes of messages with no
 * copies from one thread to another (or from/to the mainloop). The ides
 * is that a thread queue is created and then one or more threads send
 * messages in one end and fetch messages on the other end. If you set a
 * parent message queue to 1 or more queues, then this parent will wake up
 * with a sub queue message, indicating which child queue woke up. This can
 * be used to implement the ability to listen to multiple queues at once.
 * 
 * @since 1.11
 */
typedef struct _Eina_Thread_Queue Eina_Thread_Queue;

/**
 * @typedef Eina_Thread_Queue_Msg
 *
 * This is the minimal header of every message to be put into an Eina
 * Thread Queue. Every message has at least this header at the start of the
 * message data, with payload following. You would put this structure as
 * the first struct member of every message type you have, like
 * Eina_Thread_Queue_Msg_Sub does. Messages are always 8 byte aligned within
 * message memory to ensure alignment of all types.
 * 
 * @since 1.11
 */
typedef struct _Eina_Thread_Queue_Msg Eina_Thread_Queue_Msg;
/**
 * @typedef Eina_Thread_Queue_Msg_Sub
 *
 * This is a special message type for Eina Thread Queues that have child
 * queues. This is the only Message type for a parent message queue and
 * it indicates which child queue was woken up with a new message to read.
 * When this message is retrieved, the caller should then also fetch the
 * message from the inidcated child queue too.
 *
 * @since 1.11
 */
typedef struct _Eina_Thread_Queue_Msg_Sub Eina_Thread_Queue_Msg_Sub;

struct _Eina_Thread_Queue_Msg
{
   int size; /*< The size of the message in bytes, including this header */
};

struct _Eina_Thread_Queue_Msg_Sub
{
   Eina_Thread_Queue_Msg  head; /*< Standard header on all messages */
   Eina_Thread_Queue     *queue; /*< The child queue that woke up and needs a message fetched from it */
};

/**
 * @brief Create a new thread queue
 *
 * @return A valid new thread queue, or NULL on failure
 * 
 * @since 1.11
 */
EAPI Eina_Thread_Queue *
eina_thread_queue_new(void);

/**
 * @brief Free a thread queue
 *
 * This frees a thread queue. It must no longer be in use by anything waiting
 * on messages or sending them. Any pending messages will be freed without
 * being processed by a listener.
 * 
 * @param thq The thread queue to free
 * 
 * @since 1.11
 */
EAPI void
eina_thread_queue_free(Eina_Thread_Queue *thq) EINA_ARG_NONNULL(1);

/**
 * @brief Allocate a message to send down a thread queue
 *
 * @param thq The thred queue to allocate the message on
 * @param size The size, in bytes, of the message, including standard header
 * @param allocref A pointer to store a general reference handle for the message
 * @return A pointer to the message data to fill in
 *
 * This allocates space for a new message on the message queue, but does not
 * actually trigger the send. For that you will need to call
 * eina_thread_queue_send_done() to complete the send and trigger the other
 * side.
 * 
 * @since 1.11
 */
EAPI void *
eina_thread_queue_send(Eina_Thread_Queue *thq, int size, void **allocref) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Finish sending the allocated message
 *
 * @param thq The thread queue the message was placed on
 * @param allocref The allocref returned by eina_thread_queue_send()
 *
 * This completes the send and triggers the thread queue to wake up any
 * listeners.
 * 
 * @since 1.11
 */
EAPI void
eina_thread_queue_send_done(Eina_Thread_Queue *thq, void *allocref) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Fetch a message from a thread queue
 *
 * @param thq The thread queue to fetch the message from
 * @param allocref A pointer to store a general reference handle for the message
 * @return A pointer to the message data
 *
 * This will fetch the next message to read from the thread queue and return
 * a pointer to it. The message is guaranteed to have an initial
 * Eina_Thread_Queue_Msg member that will indicate size of the message as
 * a whole. This function will wait, if no messages are available to read
 * and block until a new message comes in, then return. When the message is
 * finished with, the caller must use eina_thread_queue_wait_done() to indicate
 * they are done.
 * 
 * @since 1.11
 */
EAPI void *
eina_thread_queue_wait(Eina_Thread_Queue *thq, void **allocref) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Finish fetching a message from a thread queue
 *
 * @param thq The thread queue the message was fetched from
 * @param allocref The allocref returned by eina_thread_queue_wait()
 *
 * This should be used after eina_thread_queue_wait() or
 * eina_thread_queue_poll() to indicate the caller is done with the message.
 * 
 * @since 1.11
 */
EAPI void
eina_thread_queue_wait_done(Eina_Thread_Queue *thq, void *allocref) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Fetch a message from a thread queue, but return immediately if there is none with NULL
 *
 * @param thq The thread queue to fetch the message from
 * @param allocref A pointer to store a general reference handle for the message
 * @return A pointer to the message data
 *
 * This is the same as eina_thread_queue_wait(), but if no messages are
 * available for reading, it immediately returns NULL tot he caller, without
 * waiting for a new message to arrive.
 * 
 * @see eina_thread_queue_wait()
 * 
 * @since 1.11
 */
EAPI void *
eina_thread_queue_poll(Eina_Thread_Queue *thq, void **allocref) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the number of messages on a queue as yet unfetched
 *
 * @param thq The thread queue to query for pending count
 * @return The number of messages waiting to be fetched
 *
 * This returns the number of messages waiting to be fetched with
 * eina_thread_queue_wait() or eina_thread_queue_poll().
 * 
 * @since 1.11
 */
EAPI int
eina_thread_queue_pending_get(const Eina_Thread_Queue *thq) EINA_ARG_NONNULL(1);

/**
 * @brief Set the parent of a thread queue (make this one a child)
 *
 * @param thq The thread queue to alter the parent of
 * @param thq_parent The new parent to set
 *
 * This sets the parent queue where messages will be reported to. This is
 * how you can listen to multiple queues at once - set multiple queues to
 * have the same parent and then just wait on that one parent. This should
 * be done before any messages are read from or written to the queue. To unset
 * a parent, just set the parent to NULL.
 * 
 * @since 1.11
 */
EAPI void
eina_thread_queue_parent_set(Eina_Thread_Queue *thq, Eina_Thread_Queue *thq_parent) EINA_ARG_NONNULL(1);

/**
 * @brief Get the parent of a thread queue
 *
 * @param thq The thread queue to get the parent of
 * @return The parent thread queue
 *
 * This gets the paren set by eina_thread_queue_parent_get(). If no parent
 * is set, NULL is returned.
 * 
 * @see eina_thread_queue_parent_set()
 *
 * @since 1.11
 */
EAPI Eina_Thread_Queue *
eina_thread_queue_parent_get(const Eina_Thread_Queue *thq) EINA_ARG_NONNULL(1);

/**
 * @brief Set a file descriptor to write a byte to on a message send
 *
 * @param thq The thread queue to set the file descriptor of
 * @param fd The fd to set, or -1 to unset it
 *
 * This sets a file descriptor to write to when a message is written to the
 * thread queue. This can be used to glue a thread queue to something like
 * an Ecore_Pipe that can wake up the mainloop and call a callbck whenever
 * data is available on the pipe. The number of bytes available will be
 * the number of messages to fetch from the associated thread queue.
 *
 * You should set this up before anything writes to or reads from this
 * thread queue.
 * 
 * @since 1.11
 */
EAPI void
eina_thread_queue_fd_set(Eina_Thread_Queue *thq, int fd) EINA_ARG_NONNULL(1);

/**
 * @brief Get the file descriptor written to on message sends
 *
 * @param thq The thread queue to get the file descriptor of
 * @return The file descriptor set (or -1 if none is set).
 *
 * This returns the file descriptor set by eina_thread_queue_fd_set() and
 * by default returns -1 (no fd set).
 *
 * @see eina_thread_queue_fd_set()
 * 
 * @since 1.11
 */
EAPI int
eina_thread_queue_fd_get(const Eina_Thread_Queue *thq) EINA_ARG_NONNULL(1);

/**
 * @}
 */
#endif
