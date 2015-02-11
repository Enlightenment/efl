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
#include "eina_thread.h"
#include "eina_sched.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"

# include <pthread.h>
# include <errno.h>

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

   pthread_attr_init(&attr);
   if (affinity >= 0)
     {
#ifdef EINA_HAVE_PTHREAD_AFFINITY
        cpu_set_t cpu;
        int cpunum;

        cpunum = eina_cpu_count();

        CPU_ZERO(&cpu);
        CPU_SET(affinity % cpunum, &cpu);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
#endif
     }

   /* setup initial locks */
   err = pthread_create((pthread_t *)t, &attr, func, data);
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

   if (c->prio == EINA_THREAD_BACKGROUND ||
       c->prio == EINA_THREAD_IDLE)
     eina_sched_prio_drop();

   /* FIXME: set priority and affinity */
   r = c->func((void*) c->data, eina_thread_self());

   free(c);

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
