/* EINA - EFL data type library
 * Copyright (C) 2020 Carlos Signor
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
#include "config.h"
#endif

#include <stdlib.h>

#include "eina_config.h"
#include "eina_lock.h"
#include "eina_thread.h"
#include "eina_sched.h"
#include "eina_cpu.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"

#include "eina_debug_private.h"

#include <pthread.h>
#include <errno.h>

#include <signal.h>

#if defined(EINA_HAVE_PTHREAD_AFFINITY) || defined(EINA_HAVE_PTHREAD_SETNAME)
#ifndef __linux__
#include <pthread_np.h>
#define cpu_set_t cpuset_t
#endif
#endif

inline void *
_eina_thread_join(Eina_Thread t)
{
   void *ret = NULL;
   int err = pthread_join((pthread_t)t, &ret);

   if (err == 0) return ret;
   return NULL;
}

Eina_Bool _eina_thread_name_set(Eina_Thread thread, char *buf)
{
#ifndef __linux__
   pthread_set_name_np((pthread_t)t, buf);
   return EINA_TRUE;
#else
   return pthread_setname_np((pthread_t)thread, buf);
#endif
}

inline Eina_Bool
_eina_thread_create(Eina_Thread *t, int affinity, void *(*func)(void *data), void *data)
{
   int err;
   pthread_attr_t attr;

   sigset_t oldset, newset;

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
#ifdef SIGPWR
   sigaddset(&newset, SIGPWR);
#endif
   pthread_sigmask(SIG_BLOCK, &newset, &oldset);

   err = pthread_create((pthread_t *)t, &attr, func, data);

   pthread_sigmask(SIG_SETMASK, &oldset, NULL);

   pthread_attr_destroy(&attr);

   if (err == 0) return EINA_TRUE;

   return EINA_FALSE;
}

inline Eina_Bool
_eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return pthread_equal((pthread_t)t1, (pthread_t)t2);
}

Eina_Bool _eina_thread_cancel(Eina_Thread thread)
{
   return pthread_cancel((pthread_t)thread);
}

inline Eina_Thread
_eina_thread_self(void)
{
   return (Eina_Thread)pthread_self();
}

void _eina_thread_setcanceltype(int type, int *oldtype)
{
   pthread_setcanceltype(EINA_THREAD_CANCEL_DEFERRED, &oldtype);
}
int _eina_thread_setcancelstate(int type, int *oldtype)
{
   return pthread_setcancelstate(type, &oldtype);
}
EAPI Eina_Bool
_eina_thread_cancellable_set(Eina_Bool cancellable, Eina_Bool *was_cancellable)
{
   int state = cancellable ? EINA_THREAD_CANCEL_ENABLE : EINA_THREAD_CANCEL_DISABLE;
   int old = 0;
   int r;

   /* enforce deferred in case users changed to asynchronous themselves */

   _eina_thread_setcanceltype(EINA_THREAD_CANCEL_DEFERRED, &old);

   r = _eina_thread_setcancelstate(state, &old);
   if (was_cancellable && r == 0)
      *was_cancellable = (old == EINA_THREAD_CANCEL_ENABLE);
   return r == 0;
}
void _eina_thread_cancel_checkpoint(){
   pthread_testcancel();
}