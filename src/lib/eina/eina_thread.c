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
#include "eina_lock.h"  //in case of a POSIX build, it will include pthread.h with proper flags
#include "eina_thread.h"
#include "eina_sched.h"
#include "eina_cpu.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_debug_private.h"

#include <errno.h>
#ifndef _WIN32
# include <signal.h>
# include "eina_inline_thread_posix.x"
#else
# include "eina_inline_thread_win32.x"
#endif
# include <string.h>

EINA_API const void *EINA_THREAD_JOIN_CANCELED = EINA_THREAD_CANCELED;

EINA_API Eina_Thread
eina_thread_self(void)
{
   return _eina_thread_self();
}

EINA_API Eina_Bool
eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return _eina_thread_equal(t1, t2);
}
static void *
_eina_internal_call(void *context)
{
   Eina_Thread_Call *c = context;
   void *r;
   Eina_Thread self;

   // Default this thread to not cancellable as per Eina documentation
   eina_thread_cancellable_set(EINA_FALSE, NULL);

   EINA_THREAD_CLEANUP_PUSH(free, c);

   if (c->prio == EINA_THREAD_BACKGROUND || c->prio == EINA_THREAD_IDLE)
     eina_sched_prio_drop();

   self = eina_thread_self();
   _eina_debug_thread_add(&self);
   EINA_THREAD_CLEANUP_PUSH(_eina_debug_thread_del, &self);
   r = c->func((void*) c->data, eina_thread_self());
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);

   return r;
}
EINA_API Eina_Bool
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

EINA_API void *
eina_thread_join(Eina_Thread t)
{
   return _eina_thread_join(t);
}

EINA_API Eina_Bool
eina_thread_name_set(Eina_Thread t, const char *name)
{
#if defined(EINA_HAVE_PTHREAD_SETNAME) || defined(EINA_HAVE_WIN32_THREAD_SETNAME)
   char buf[16];
   if (name)
     {
        strncpy(buf, name, 15);
        buf[15] = 0;
     }
   else buf[0] = 0;
   if (_eina_thread_name_set(t, buf) == 0) return EINA_TRUE;
#else
   (void)t;
   (void)name;
#endif
   return EINA_FALSE;
}

EINA_API Eina_Bool
eina_thread_cancel(Eina_Thread t)
{
   if (!t) return EINA_FALSE;
   return _eina_thread_cancel(t) == 0;
}

EINA_API Eina_Bool
eina_thread_cancellable_set(Eina_Bool cancellable, Eina_Bool *was_cancellable)
{
   return _eina_thread_cancellable_set(cancellable,was_cancellable);
}

EINA_API void
eina_thread_cancel_checkpoint(void)
{
   _eina_thread_cancel_checkpoint();
}

EINA_API void *
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

