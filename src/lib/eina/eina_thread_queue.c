#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>
#include "Eina.h"
#include "eina_thread_queue.h"
#include "eina_safety_checks.h"
#include "eina_log.h"

#include "eina_private.h"

#ifdef __ATOMIC_RELAXED
#define ATOMIC 1
#endif

// use spinlocks for read/write locks as they lead to more throughput and
// these locks are meant to be held very temporarily, if there is any
// contention at all
#define RW_SPINLOCK 1

#ifdef RW_SPINLOCK
#define RWLOCK           Eina_Spinlock
#define RWLOCK_NEW(x)    eina_spinlock_new(x)
#define RWLOCK_FREE(x)   eina_spinlock_free(x)
#define RWLOCK_LOCK(x)   eina_spinlock_take(x)
#define RWLOCK_UNLOCK(x) eina_spinlock_release(x)
#else
#define RWLOCK           Eina_Lock
#define RWLOCK_NEW(x)    eina_lock_new(x)
#define RWLOCK_FREE(x)   eina_lock_free(x)
#define RWLOCK_LOCK(x)   eina_lock_take(x)
#define RWLOCK_UNLOCK(x) eina_lock_release(x)
#endif

typedef struct _Eina_Thread_Queue_Msg_Block Eina_Thread_Queue_Msg_Block;

struct _Eina_Thread_Queue
{
   Eina_Thread_Queue_Msg_Block  *data; // all the data being written to
   Eina_Thread_Queue_Msg_Block  *last; // the last block where new data goes
   Eina_Thread_Queue_Msg_Block  *read; // block when reading starts from data
   Eina_Thread_Queue            *parent; // parent queue to wake on send
   RWLOCK                        lock_read; // a lock for when doing reads
   RWLOCK                        lock_write; // a lock for doing writes
   Eina_Semaphore                sem; // signalling - one per message
#ifndef ATOMIC
   Eina_Spinlock                 lock_pending; // lock for pending field
#endif
   int                           pending; // how many messages left to read
   int                           fd; // optional fd to write byte to on msg
};

struct _Eina_Thread_Queue_Msg_Block
{
   Eina_Thread_Queue_Msg_Block  *next; // next block in the list
   Eina_Lock                     lock_non_0_ref; // block non-0 ref state
#ifndef ATOMIC
   Eina_Spinlock                 lock_ref; // lock for ref field
#endif
   int                           ref; // the number of open reads/writes
   int                           size; // the total allocated bytes of data[]
   int                           first; // the byte pos of the first msg
   int                           last; // the byte pos just after the last msg
   Eina_Bool                     full : 1; // is this block full yet?
   Eina_Thread_Queue_Msg         data[1]; // data in memory beyond struct end
};

// the minimum size of any message block holding 1 or more messages
#define MIN_SIZE ((int)(4096 - sizeof(Eina_Thread_Queue_Msg_Block) + sizeof(Eina_Thread_Queue_Msg)))

// a pool of spare message blocks that are only of the minimum size so we
// avoid reallocation via malloc/free etc. to avoid free memory pages and
// pressure on the malloc subsystem
static int _eina_thread_queue_log_dom = -1;
static int _eina_thread_queue_block_pool_count = 0;
static Eina_Spinlock _eina_thread_queue_block_pool_lock;
static Eina_Thread_Queue_Msg_Block *_eina_thread_queue_block_pool = NULL;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_thread_queue_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_thread_queue_log_dom, __VA_ARGS__)

// api's to get message blocks from the pool or put them back in
static Eina_Thread_Queue_Msg_Block *
_eina_thread_queue_msg_block_new(int size)
{
   Eina_Thread_Queue_Msg_Block *blk;

   eina_spinlock_take(&(_eina_thread_queue_block_pool_lock));
   if (_eina_thread_queue_block_pool)
     {
        blk = _eina_thread_queue_block_pool;
        if (blk->size >= size)
          {
             blk->first = 0;
             blk->last = 0;
             blk->ref = 0;
             blk->full = 0;
             _eina_thread_queue_block_pool = blk->next;
             blk->next = NULL;
             _eina_thread_queue_block_pool_count--;
             eina_spinlock_release(&(_eina_thread_queue_block_pool_lock));
             return blk;
          }
        blk = NULL;
     }
   eina_spinlock_release(&(_eina_thread_queue_block_pool_lock));

   blk = malloc(sizeof(Eina_Thread_Queue_Msg_Block) -
                sizeof(Eina_Thread_Queue_Msg) +
                size);
   if (!blk)
     {
        ERR("Thread queue block buffer of size %i allocation failed", size);
        return NULL;
     }
   blk->next = NULL;
#ifndef ATOMIC
   eina_spinlock_new(&(blk->lock_ref));
#endif
   eina_lock_new(&(blk->lock_non_0_ref));
   blk->size = size;
   blk->first = 0;
   blk->last = 0;
   blk->ref = 0;
   blk->full = 0;
   return blk;
}

static void
_eina_thread_queue_msg_block_real_free(Eina_Thread_Queue_Msg_Block *blk)
{
   eina_lock_take(&(blk->lock_non_0_ref));
   eina_lock_release(&(blk->lock_non_0_ref));
   eina_lock_free(&(blk->lock_non_0_ref));
#ifndef ATOMIC
   eina_spinlock_take(&(blk->lock_ref));
   eina_spinlock_release(&(blk->lock_ref));
   eina_spinlock_free(&(blk->lock_ref));
#endif
   free(blk);
}

static void
_eina_thread_queue_msg_block_free(Eina_Thread_Queue_Msg_Block *blk)
{
   if (blk->size == MIN_SIZE)
     {
        eina_spinlock_take(&(_eina_thread_queue_block_pool_lock));
        if (_eina_thread_queue_block_pool_count < 20)
          {
             _eina_thread_queue_block_pool_count++;
             blk->next = _eina_thread_queue_block_pool;
             _eina_thread_queue_block_pool = blk;
             eina_spinlock_release(&(_eina_thread_queue_block_pool_lock));
          }
        else
          {
             eina_spinlock_release(&(_eina_thread_queue_block_pool_lock));
             _eina_thread_queue_msg_block_real_free(blk);
          }
     }
   else _eina_thread_queue_msg_block_real_free(blk);
}

static Eina_Bool
_eina_thread_queue_msg_block_pool_init(void)
{
   return eina_spinlock_new(&_eina_thread_queue_block_pool_lock);
}

static void
_eina_thread_queue_msg_block_pool_shutdown(void)
{
   eina_spinlock_take(&(_eina_thread_queue_block_pool_lock));
   while (_eina_thread_queue_block_pool)
     {
        Eina_Thread_Queue_Msg_Block *blk, *blknext;

        for (;;)
          {
             blk = _eina_thread_queue_block_pool;
             if (!blk) break;
             blknext = blk->next;
             _eina_thread_queue_msg_block_real_free(blk);
             _eina_thread_queue_block_pool = blknext;
          }
     }
   eina_spinlock_release(&(_eina_thread_queue_block_pool_lock));
   eina_spinlock_free(&_eina_thread_queue_block_pool_lock);
}

// utility functions for waiting/waking threads
static void
_eina_thread_queue_wait(Eina_Thread_Queue *thq)
{
   if (!eina_semaphore_lock(&(thq->sem)))
     ERR("Thread queue semaphore lock/wait failed - bad things will happen");
}

static void
_eina_thread_queue_wake(Eina_Thread_Queue *thq)
{
   if (!eina_semaphore_release(&(thq->sem), 1))
     ERR("Thread queue semaphore release/wakeup faile - bad things will happen");
}

// how to allocate or release memory within one of the message blocks for
// an arbitrary sized bit of message data. the size always includes the
// message header which tells you the size of that message
static Eina_Thread_Queue_Msg *
_eina_thread_queue_msg_alloc(Eina_Thread_Queue *thq, int size, Eina_Thread_Queue_Msg_Block **blkret)
{
   Eina_Thread_Queue_Msg_Block *blk;
   Eina_Thread_Queue_Msg *msg = NULL;
   int ref;

   // round up to nearest 8
   size = ((size + 7) >> 3) << 3;
   if (!thq->data)
     {
        if (size < MIN_SIZE)
          thq->data = _eina_thread_queue_msg_block_new(MIN_SIZE);
        else
          thq->data = _eina_thread_queue_msg_block_new(size);
        thq->last = thq->data;
     }
   blk = thq->last;
   if (blk->full)
     {
        if (size < MIN_SIZE)
          blk->next = _eina_thread_queue_msg_block_new(MIN_SIZE);
        else
          blk->next = _eina_thread_queue_msg_block_new(size);
        blk = blk->next;
        thq->last = blk;
     }
   if ((blk->size - blk->last) >= size)
     {
        blk->last += size;
        if (blk->last == blk->size) blk->full = 1;
        msg = (Eina_Thread_Queue_Msg *)((char *)(&(blk->data[0])) + (blk->last - size));
     }
   else
     {
        if (size < MIN_SIZE)
          blk->next = _eina_thread_queue_msg_block_new(MIN_SIZE);
        else
          blk->next = _eina_thread_queue_msg_block_new(size);
        blk = blk->next;
        thq->last = blk;
        blk->last += size;
        if (blk->last == blk->size) blk->full = 1;
        msg = (Eina_Thread_Queue_Msg *)(&(blk->data[0]));
     }
   msg->size = size;
#ifdef ATOMIC
   ref = __atomic_add_fetch(&(blk->ref), 1, __ATOMIC_RELAXED);
#else
   eina_spinlock_take(&(blk->lock_ref));
   blk->ref++;
   ref = blk->ref;
   eina_spinlock_release(&(blk->lock_ref));
#endif
   if (ref == 1) eina_lock_take(&(blk->lock_non_0_ref));
   *blkret = blk;
   return msg;
}

static void
_eina_thread_queue_msg_alloc_done(Eina_Thread_Queue_Msg_Block *blk)
{
   int ref;
#ifdef ATOMIC
   ref = __atomic_sub_fetch(&(blk->ref), 1, __ATOMIC_RELAXED);
#else
   eina_spinlock_take(&(blk->lock_ref));
   blk->ref--;
   ref = blk->ref;
   eina_spinlock_release(&(blk->lock_ref));
#endif
   if (ref == 0) eina_lock_release(&(blk->lock_non_0_ref));
}

static Eina_Thread_Queue_Msg *
_eina_thread_queue_msg_fetch(Eina_Thread_Queue *thq, Eina_Thread_Queue_Msg_Block **blkret)
{
   Eina_Thread_Queue_Msg_Block *blk;
   Eina_Thread_Queue_Msg *msg;
   int ref;

   if (!thq->read)
     {
        RWLOCK_LOCK(&(thq->lock_write));
        blk = thq->data;
        if (!blk)
          {
             RWLOCK_UNLOCK(&(thq->lock_write));
             return NULL;
          }
#ifdef ATOMIC
        __atomic_load(&(blk->ref), &ref, __ATOMIC_RELAXED);
#else
        eina_spinlock_take(&(blk->lock_ref));
        ref = blk->ref;
        eina_spinlock_release(&(blk->lock_ref));
#endif
        if (ref > 0) eina_lock_take(&(blk->lock_non_0_ref));
        thq->read = blk;
        if (thq->last == blk) thq->last = blk->next;
        thq->data = blk->next;
        blk->next = NULL;
        if (ref > 0) eina_lock_release(&(blk->lock_non_0_ref));
        RWLOCK_UNLOCK(&(thq->lock_write));
     }
   blk = thq->read;
   msg = (Eina_Thread_Queue_Msg *)((char *)(&(blk->data[0])) + blk->first);
   blk->first += msg->size;
   if (blk->first >= blk->last) thq->read = NULL;
   *blkret = blk;
#ifdef ATOMIC
   __atomic_add_fetch(&(blk->ref), 1, __ATOMIC_RELAXED);
#else
   eina_spinlock_take(&(blk->lock_ref));
   blk->ref++;
   eina_spinlock_release(&(blk->lock_ref));
#endif
   return msg;
}

static void
_eina_thread_queue_msg_fetch_done(Eina_Thread_Queue_Msg_Block *blk)
{
   int ref;

#ifdef ATOMIC
   ref = __atomic_sub_fetch(&(blk->ref), 1, __ATOMIC_RELAXED);
#else
   eina_spinlock_take(&(blk->lock_ref));
   blk->ref--;
   ref = blk->ref;
   eina_spinlock_release(&(blk->lock_ref));
#endif
   if ((blk->first >= blk->last) && (ref == 0))
     _eina_thread_queue_msg_block_free(blk);
}


//////////////////////////////////////////////////////////////////////////////
Eina_Bool
eina_thread_queue_init(void)
{
   _eina_thread_queue_log_dom = eina_log_domain_register("eina_thread_queue",
                                                         EINA_LOG_COLOR_DEFAULT);
   if (_eina_thread_queue_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_thread_queue");
        return EINA_FALSE;
     }
   if (!_eina_thread_queue_msg_block_pool_init())
     {
        ERR("Cannot init thread queue block pool spinlock");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
eina_thread_queue_shutdown(void)
{
   _eina_thread_queue_msg_block_pool_shutdown();
   eina_log_domain_unregister(_eina_thread_queue_log_dom);
   return EINA_TRUE;
}

EAPI Eina_Thread_Queue *
eina_thread_queue_new(void)
{
   Eina_Thread_Queue *thq;

   thq = calloc(1, sizeof(Eina_Thread_Queue));
   if (!thq)
     {
        ERR("Allocation of Thread queue structure failed");
        return NULL;
     }
   thq->fd = -1;
   if (!eina_semaphore_new(&(thq->sem), 0))
     {
        ERR("Cannot init new semaphore for eina_threadqueue");
        free(thq);
        return NULL;
     }
   RWLOCK_NEW(&(thq->lock_read));
   RWLOCK_NEW(&(thq->lock_write));
#ifndef ATOMIC
   eina_spinlock_new(&(thq->lock_pending));
#endif
   return thq;
}

EAPI void
eina_thread_queue_free(Eina_Thread_Queue *thq)
{
   if (!thq) return;

#ifndef ATOMIC
   eina_spinlock_free(&(thq->lock_pending));
#endif
   RWLOCK_FREE(&(thq->lock_read));
   RWLOCK_FREE(&(thq->lock_write));
   eina_semaphore_free(&(thq->sem));
   free(thq);
}

EAPI void *
eina_thread_queue_send(Eina_Thread_Queue *thq, int size, void **allocref)
{
   Eina_Thread_Queue_Msg *msg;
   Eina_Thread_Queue_Msg_Block *blk;

   RWLOCK_LOCK(&(thq->lock_write));
   msg = _eina_thread_queue_msg_alloc(thq, size, &blk);
   RWLOCK_UNLOCK(&(thq->lock_write));
   *allocref = blk;
#ifdef ATOMIC
  __atomic_add_fetch(&(thq->pending), 1, __ATOMIC_RELAXED);
#else
   eina_spinlock_take(&(thq->lock_pending));
   thq->pending++;
   eina_spinlock_release(&(thq->lock_pending));
#endif
   return msg;
}

EAPI void
eina_thread_queue_send_done(Eina_Thread_Queue *thq, void *allocref)
{
   _eina_thread_queue_msg_alloc_done(allocref);
   _eina_thread_queue_wake(thq);
   if (thq->parent)
     {
        void *ref;
        Eina_Thread_Queue_Msg_Sub *msg;

        msg = eina_thread_queue_send(thq->parent,
                                     sizeof(Eina_Thread_Queue_Msg_Sub), &ref);
        if (msg)
          {
             msg->queue = thq;
             eina_thread_queue_send_done(thq->parent, ref);
          }
     }
   if (thq->fd >= 0)
     {
        char dummy = 0;
        if (write(thq->fd, &dummy, 1) != 1)
          ERR("Eina Threadqueue write to fd %i failed", thq->fd);
     }
}

EAPI void *
eina_thread_queue_wait(Eina_Thread_Queue *thq, void **allocref)
{
   Eina_Thread_Queue_Msg *msg;
   Eina_Thread_Queue_Msg_Block *blk;

   _eina_thread_queue_wait(thq);
   RWLOCK_LOCK(&(thq->lock_read));
   msg = _eina_thread_queue_msg_fetch(thq, &blk);
   RWLOCK_UNLOCK(&(thq->lock_read));
   *allocref = blk;
#ifdef ATOMIC
  __atomic_sub_fetch(&(thq->pending), 1, __ATOMIC_RELAXED);
#else
   eina_spinlock_take(&(thq->lock_pending));
   thq->pending--;
   eina_spinlock_release(&(thq->lock_pending));
#endif
   return msg;
}

EAPI void
eina_thread_queue_wait_done(Eina_Thread_Queue *thq EINA_UNUSED, void *allocref)
{
   _eina_thread_queue_msg_fetch_done(allocref);
}

EAPI void *
eina_thread_queue_poll(Eina_Thread_Queue *thq, void **allocref)
{
   Eina_Thread_Queue_Msg *msg;
   Eina_Thread_Queue_Msg_Block *blk;

   RWLOCK_LOCK(&(thq->lock_read));
   msg = _eina_thread_queue_msg_fetch(thq, &blk);
   RWLOCK_UNLOCK(&(thq->lock_read));
   if (msg)
     {
        _eina_thread_queue_wait(thq);
        *allocref = blk;
#ifdef ATOMIC
        __atomic_sub_fetch(&(thq->pending), 1, __ATOMIC_RELAXED);
#else
        eina_spinlock_take(&(thq->lock_pending));
        thq->pending--;
        eina_spinlock_release(&(thq->lock_pending));
#endif
     }
   return msg;
}

EAPI int
eina_thread_queue_pending_get(const Eina_Thread_Queue *thq)
{
   int pending;

#ifdef ATOMIC
   __atomic_load(&(thq->pending), &pending, __ATOMIC_RELAXED);
#else
   eina_spinlock_take((Eina_Spinlock *)&(thq->lock_pending));
   pending = thq->pending;
   eina_spinlock_release((Eina_Spinlock *)&(thq->lock_pending));
#endif
   return pending;
}

EAPI void
eina_thread_queue_parent_set(Eina_Thread_Queue *thq, Eina_Thread_Queue *thq_parent)
{
   thq->parent = thq_parent;
}

EAPI Eina_Thread_Queue *
eina_thread_queue_parent_get(const Eina_Thread_Queue *thq)
{
   return thq->parent;
}

EAPI void
eina_thread_queue_fd_set(Eina_Thread_Queue *thq, int fd)
{
   thq->fd = fd;
}

EAPI int
eina_thread_queue_fd_get(const Eina_Thread_Queue *thq)
{
   return thq->fd;
}
