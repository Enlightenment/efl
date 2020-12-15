/* EINA - EFL data type library
 * Copyright (C) 2020 Expertise Solutions Cons em Inf
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_types.h"
#include "eina_config.h"
#include "eina_array.h"
#include "eina_thread.h"
#include "eina_main.h"
#include "eina_debug_private.h"
#include "eina_log.h"

#include <assert.h>
#include <evil_private.h>
#include <process.h>

#define RTNICENESS 1
#define NICENESS 5

/*
 * The underlying type of Eina_Thread
 */
struct Thread
{
   CRITICAL_SECTION cancel_lock; /* mutex to protect the cancel handle */
   char name[16]; /* the thread name */
   HANDLE handle; /* thread handle */
   void *data; /* on entry, the thread function argument, on exit, the return value */
   Eina_Thread_Cb fn; /* the thread function */
   Eina_Array *cleanup_fn;
   Eina_Array *cleanup_arg;
   unsigned id; /* thread id */
   Eina_Bool free_on_exit; /* free the structure when thread exit */
   volatile Eina_Bool cancel; /* the cancel event handle */
   volatile Eina_Bool cancellable; /* is cancel enabled? */
};

typedef struct Thread Thread_t;

/*
 * This TLS stores the Eina_Thread for the current thread
 */
static DWORD tls_thread_self = 0;

static Thread_t main_thread = { 0 };

/*
 * If we alloc'ed the Thread_t in eina_thread_self, we set
 * free_on_exit flag to true, we then free it here
 */
void
free_thread(void)
{
   Thread_t *t = TlsGetValue(tls_thread_self);
   if (t && t->free_on_exit)
     {
        if (t) eina_array_free(t->cleanup_fn);
        if (t) eina_array_free(t->cleanup_arg);
        free(t);
     }
}

static unsigned
thread_fn(void *arg)
{
   Thread_t *thr = arg;
   TlsSetValue(tls_thread_self, thr);
   _eina_debug_thread_add(&thr);
   EINA_THREAD_CLEANUP_PUSH(_eina_debug_thread_del, &thr);
   thr->data = thr->fn(thr->data, (Eina_Thread) thr);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   return 0;
}

EINA_API Eina_Thread
eina_thread_self(void)
{
    Thread_t *self = TlsGetValue(tls_thread_self);
    /*
     * If self is NULL this means
     * 1) This function was called before eina_thread_init
     * 2) This thread wasn't created by eina_thread_create
     *
     * In either case we alloc a new Thread struct and return
     * it.
     */
    if (!self)
      {
         self = calloc(1, sizeof(*self));
         self->handle = GetCurrentThread();
         self->id = GetCurrentThreadId();
         self->free_on_exit = EINA_TRUE;
         self->cleanup_fn = eina_array_new(4);
         self->cleanup_arg = eina_array_new(4);
         if (tls_thread_self)
            TlsSetValue(tls_thread_self, self);
      }
    return (Eina_Thread) self;
}

EINA_API Eina_Bool
eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return ((Thread_t *) t1)->id == ((Thread_t *) t2)->id;
}

EINA_API Eina_Bool
eina_thread_create(Eina_Thread *t, Eina_Thread_Priority prio,
                   int affinity, Eina_Thread_Cb func, const void *data)
{
   Thread_t *thr = calloc(1, sizeof(Thread_t));
   if (!thr)
      return EINA_FALSE;

   thr->data = (void *) data;
   thr->fn = func;

   thr->handle = (HANDLE) _beginthreadex(NULL, 0, thread_fn, thr, CREATE_SUSPENDED, &thr->id);
   if (!thr->handle)
      goto fail;

   int priority;
   switch (prio)
     {
        case EINA_THREAD_URGENT:
           priority = THREAD_PRIORITY_HIGHEST;
           break;
        case EINA_THREAD_BACKGROUND:
           priority = THREAD_PRIORITY_BELOW_NORMAL;
           break;
        case EINA_THREAD_IDLE:
           priority = THREAD_PRIORITY_IDLE;
           break;
        default:
           priority = THREAD_PRIORITY_NORMAL;
     }

   if (!SetThreadPriority(thr->handle, priority))
      goto fail;

   if ((affinity >= 0) && (!SetThreadAffinityMask(thr->handle, 1 << affinity)))
      goto fail;

   thr->id = GetThreadId(thr->handle);
   if (!thr->id)
      goto fail;

   thr->cleanup_fn = eina_array_new(4);
   thr->cleanup_arg = eina_array_new(4);
   if ((!thr->cleanup_fn) || (!thr->cleanup_arg))
      goto fail;

   InitializeCriticalSection(&thr->cancel_lock);

   if (!ResumeThread(thr->handle))
      goto cs_fail;

   GetModuleFileNameA(NULL, thr->name, sizeof(thr->name));
   *t = (Eina_Thread) thr;
   return EINA_TRUE;

cs_fail:
   DeleteCriticalSection(&thr->cancel_lock);
fail:
   if (thr)
     {
        if (thr->handle) CloseHandle(thr->handle);
        if (thr->cleanup_fn) eina_array_free(thr->cleanup_fn);
        if (thr->cleanup_arg) eina_array_free(thr->cleanup_arg);
        free(thr);
     }
   return EINA_FALSE;
}

EINA_API void *
eina_thread_join(Eina_Thread t)
{
   void *data;
   Thread_t *thr = (Thread_t *) t;

   if (WAIT_OBJECT_0 == WaitForSingleObject(thr->handle, INFINITE))
      data = thr->data;
   else
      data = NULL;

   DeleteCriticalSection(&thr->cancel_lock);
   CloseHandle(thr->handle);
   eina_array_free(thr->cleanup_fn);
   eina_array_free(thr->cleanup_arg);
   free(thr);

   return data;
}

EINA_API Eina_Bool
eina_thread_name_set(Eina_Thread t, const char *name)
{
   Thread_t *thr = (Thread_t *) t;
   strncpy(thr->name, name, sizeof(thr->name));
   thr->name[sizeof(thr->name)-1] = '\0';
   return EINA_TRUE;
}

EINA_API Eina_Bool
eina_thread_cancel(Eina_Thread t)
{
    Eina_Bool ret = EINA_FALSE;
    Thread_t *thr = (Thread_t *) t;

    if (thr)
      {
         EnterCriticalSection(&thr->cancel_lock);
         if (thr->cancellable)
           {
              thr->cancel = EINA_TRUE;
              ret = EINA_TRUE;
           }
         LeaveCriticalSection(&thr->cancel_lock);
      }
    return ret;
}

EINA_API Eina_Bool
eina_thread_cancellable_set(Eina_Bool cancellable, Eina_Bool *was_cancellable)
{
   Thread_t *t = (Thread_t *) eina_thread_self();

   EnterCriticalSection(&t->cancel_lock);
   if (was_cancellable) *was_cancellable = t->cancellable;
   t->cancellable = cancellable;
   LeaveCriticalSection(&t->cancel_lock);

   return EINA_TRUE;
}

EINA_API void
eina_thread_cancel_checkpoint(void)
{
   Eina_Bool cancel;
   Thread_t *t = (Thread_t *) eina_thread_self();

   EnterCriticalSection(&t->cancel_lock);
   cancel = t->cancellable && t->cancel;
   LeaveCriticalSection(&t->cancel_lock);

   if (cancel)
     {
        t->data = (void *) EINA_THREAD_JOIN_CANCELED;
        while (eina_array_count(t->cleanup_fn))
          {
             Eina_Thread_Cleanup_Cb fn = (Eina_Thread_Cleanup_Cb) eina_array_pop(t->cleanup_fn);
             void *arg = eina_array_pop(t->cleanup_arg);

             if (fn)
               fn(arg);
          }

        ExitThread(0);
     }
}

EINA_API Eina_Bool
eina_thread_cleanup_push(Eina_Thread_Cleanup_Cb fn, void *data)
{
   Thread_t *t = TlsGetValue(tls_thread_self);
   assert(t);

   if (!eina_array_push(t->cleanup_fn, fn))
      return EINA_FALSE;

   if (!eina_array_push(t->cleanup_arg, data))
     {
        eina_array_pop(t->cleanup_fn);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EINA_API void
eina_thread_cleanup_pop(int execute)
{
   Thread_t *t = TlsGetValue(tls_thread_self);
   assert(t);

   if (eina_array_count(t->cleanup_fn))
     {
        Eina_Thread_Cleanup_Cb fn = (Eina_Thread_Cleanup_Cb) eina_array_pop(t->cleanup_fn);
        void *arg = eina_array_pop(t->cleanup_arg);

        if (execute && fn)
           fn(arg);
     }
}

EINA_API const void *EINA_THREAD_JOIN_CANCELED = (void *) -1L;

void
eina_sched_prio_drop(void)
{
   Thread_t *thread;
   int sched_priority;

   thread = (Thread_t *) eina_thread_self();

   sched_priority = GetThreadPriority(thread->handle);

   if (EINA_UNLIKELY(sched_priority == THREAD_PRIORITY_TIME_CRITICAL))
     {
        sched_priority -= RTNICENESS;

        /* We don't change the policy */
        if (sched_priority < 1)
          {
             EINA_LOG_INFO("RT prio < 1, setting to 1 instead");
             sched_priority = 1;
          }
        if (!SetThreadPriority(thread->handle, sched_priority))
          {
             EINA_LOG_ERR("Unable to query sched parameters");
          }
     }
   else
     {
        sched_priority += NICENESS;

        /* We don't change the policy */
        if (sched_priority > THREAD_PRIORITY_TIME_CRITICAL)
          {
             EINA_LOG_INFO("Max niceness reached; keeping max (THREAD_PRIORITY_TIME_CRITICAL)");
             sched_priority = THREAD_PRIORITY_TIME_CRITICAL;
          }
        if (!SetThreadPriority(thread->handle, sched_priority))
          {
             EINA_LOG_ERR("Unable to query sched parameters");
          }
     }
}

EINA_API Eina_Bool
eina_thread_init(void)
{
   if (!eina_main_loop_is())
      return EINA_FALSE;

   tls_thread_self = TlsAlloc();
   if (TLS_OUT_OF_INDEXES == tls_thread_self)
      return EINA_FALSE;

   if (!TlsSetValue(tls_thread_self, &main_thread))
     {
        assert(0);
        TlsFree(tls_thread_self);
        return EINA_FALSE;
     }

   main_thread.cancellable = EINA_FALSE;
   main_thread.cancel = EINA_FALSE;
   main_thread.handle = GetCurrentThread();
   main_thread.id = GetCurrentThreadId();

   InitializeCriticalSection(&main_thread.cancel_lock);
   main_thread.cleanup_fn = eina_array_new(2);
   main_thread.cleanup_arg = eina_array_new(2);

   GetModuleFileNameA(NULL, main_thread.name, sizeof(main_thread.name)/sizeof(main_thread.name[0]));

   return EINA_TRUE;
}

EINA_API Eina_Bool
eina_thread_shutdown(void)
{
   DeleteCriticalSection(&main_thread.cancel_lock);
   eina_array_free(main_thread.cleanup_fn);
   eina_array_free(main_thread.cleanup_arg);
   TlsFree(tls_thread_self);
   return EINA_TRUE;
}
