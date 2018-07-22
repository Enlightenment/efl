/* EINA - EFL data type library
 * Copyright (C) 2012 Cedric Bail
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

#include <stdlib.h>

#include "eina_config.h"
#include "eina_lock.h" /* it will include pthread.h with proper flags */
#include "eina_thread.h"
#include "eina_sched.h"
#include "eina_cpu.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"

#include "eina_debug_private.h"

#include <pthread.h>
#include <errno.h>
#ifndef _WIN32
# include <signal.h>
#endif
# include <string.h>

#if defined(EINA_HAVE_PTHREAD_AFFINITY) || defined(EINA_HAVE_PTHREAD_SETNAME)
#ifndef __linux__
#include <pthread_np.h>
#define cpu_set_t cpuset_t
#endif
#endif

static inline void *
_eina_thread_join(Eina_Thread t)
{
   void *ret = NULL;
   int err = pthread_join((pthread_t)t, &ret);

   if (err == 0) return ret;
   return NULL;
}

static inline Eina_Bool
_eina_thread_create(Eina_Thread *t, int affinity, void *(*func)(void *data), void *data)
{
   int err;
   pthread_attr_t attr;
#ifndef _WIN32
   sigset_t oldset, newset;
#endif
   
   pthread_attr_init(&attr);
   if (affinity >= 0)
     {
#ifdef EINA_HAVE_PTHREAD_AFFINITY
        cpu_set_t cpu;

        CPU_ZERO(&cpu);
        CPU_SET(affinity, &cpu);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
#endif
     }

   /* setup initial locks */
#ifndef _WIN32
   sigemptyset(&newset);
   sigaddset(&newset, SIGPIPE);
   sigaddset(&newset, SIGALRM);
   sigaddset(&newset, SIGCHLD);
   sigaddset(&newset, SIGUSR1);
   sigaddset(&newset, SIGUSR2);
   sigaddset(&newset, SIGHUP);
   sigaddset(&newset, SIGQUIT);
   sigaddset(&newset, SIGINT);
   sigaddset(&newset, SIGTERM);
# ifdef SIGPWR
   sigaddset(&newset, SIGPWR);
# endif
   pthread_sigmask(SIG_BLOCK, &newset, &oldset);
#endif
   err = pthread_create((pthread_t *)t, &attr, func, data);
#ifndef _WIN32
   pthread_sigmask(SIG_SETMASK, &oldset, NULL);
#endif
   pthread_attr_destroy(&attr);

   if (err == 0) return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
_eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return pthread_equal((pthread_t)t1, (pthread_t)t2);
}

static inline Eina_Thread
_eina_thread_self(void)
{
   return (Eina_Thread)pthread_self();
}


typedef struct _Eina_Thread_Call Eina_Thread_Call;
struct _Eina_Thread_Call
{
   Eina_Thread_Cb func;
   const void *data;

   Eina_Thread_Priority prio;
   int affinity;
};

static void *
_eina_internal_call(void *context)
{
   Eina_Thread_Call *c = context;
   void *r;
   pthread_t self;

   // Default this thread to not cancellable as per Eina documentation
   eina_thread_cancellable_set(EINA_FALSE, NULL);

   EINA_THREAD_CLEANUP_PUSH(free, c);

   if (c->prio == EINA_THREAD_BACKGROUND ||
       c->prio == EINA_THREAD_IDLE)
     eina_sched_prio_drop();

   self = pthread_self();
   _eina_debug_thread_add(&self);
   EINA_THREAD_CLEANUP_PUSH(_eina_debug_thread_del, &self);
   r = c->func((void*) c->data, eina_thread_self());
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);

   return r;
}

EAPI Eina_Thread
eina_thread_self(void)
{
   return _eina_thread_self();
}

EAPI Eina_Bool
eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return !!_eina_thread_equal(t1, t2);
}

EAPI Eina_Bool
eina_thread_create(Eina_Thread *t,
                   Eina_Thread_Priority prio, int affinity,
                   Eina_Thread_Cb func, const void *data)
{
   Eina_Thread_Call *c;

   EINA_SAFETY_ON_NULL_RETURN_VAL(t, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, EINA_FALSE);

   c = malloc(sizeof (Eina_Thread_Call));
   if (!c) return EINA_FALSE;

   c->func = func;
   c->data = data;
   c->prio = prio;
   c->affinity = affinity;

   // valgrind complains c is lost - but it's not - it is handed to the
   // child thread to be freed when c->func returns in _eina_internal_call().
   if (_eina_thread_create(t, affinity, _eina_internal_call, c))
     return EINA_TRUE;

   free(c);
   return EINA_FALSE;
}

EAPI void *
eina_thread_join(Eina_Thread t)
{
   return _eina_thread_join(t);
}

EAPI Eina_Bool
eina_thread_name_set(Eina_Thread t, const char *name)
{
#ifdef EINA_HAVE_PTHREAD_SETNAME
   char buf[16];
   if (name)
     {
        strncpy(buf, name, 15);
        buf[15] = 0;
     }
   else buf[0] = 0;
#ifndef __linux__
   pthread_set_name_np((pthread_t)t, buf);
   return EINA_TRUE;
#else
   if (pthread_setname_np((pthread_t)t, buf) == 0) return EINA_TRUE;
#endif
#else
   (void)t;
   (void)name;
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool
eina_thread_cancel(Eina_Thread t)
{
   if (!t) return EINA_FALSE;
   return pthread_cancel((pthread_t)t) == 0;
}

EAPI Eina_Bool
eina_thread_cancellable_set(Eina_Bool cancellable, Eina_Bool *was_cancellable)
{
   int state = cancellable ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE;
   int old = 0;
   int r;

   /* enforce deferred in case users changed to asynchronous themselves */
   pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old);

   r = pthread_setcancelstate(state, &old);
   if (was_cancellable && r == 0)
     *was_cancellable = (old == PTHREAD_CANCEL_ENABLE);

   return r == 0;
}

EAPI void
eina_thread_cancel_checkpoint(void)
{
   pthread_testcancel();
}

EAPI void *
eina_thread_cancellable_run(Eina_Thread_Cancellable_Run_Cb cb, Eina_Free_Cb cleanup_cb, void *data)
{
   Eina_Bool old = EINA_FALSE;
   void *ret;

   EINA_THREAD_CLEANUP_PUSH(cleanup_cb, data);
   eina_thread_cancellable_set(EINA_TRUE, &old); // is a cancellation point
   ret = cb(data); // may not run if was previously canceled
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   eina_thread_cancellable_set(old, NULL);
   return ret;
}

EAPI const void *EINA_THREAD_JOIN_CANCELED = PTHREAD_CANCELED;

Eina_Bool
eina_thread_init(void)
{
   return EINA_TRUE;
}

Eina_Bool
eina_thread_shutdown(void)
{
   return EINA_TRUE;
}
