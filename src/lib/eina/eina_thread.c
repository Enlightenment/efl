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

#ifdef EINA_HAVE_THREADS
# ifdef _WIN32_WCE

# elif defined(_WIN32)

#  include "eina_list.h"

#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef WIN32_LEAN_AND_MEAN

typedef struct _Eina_Thread_Win32 Eina_Thread_Win32;
struct _Eina_Thread_Win32
{
   HANDLE thread;
   void *(*func)(void *data);
   void *data;
   void *ret;

   Eina_Thread index;
};

/* FIXME: For the moment Eina_Thread is considered not
   thread safe, wondering if it's worth it */
static unsigned long int _current_index = 1; /* start from one as the main loop == 0 */
static Eina_List *_thread_pool = NULL;
static Eina_List *_thread_running = NULL;

static Eina_Thread_Win32 *
_eina_thread_win32_find(Eina_Thread index)
{
   Eina_Thread_Win32 *tw;
   Eina_List *l;

   EINA_LIST_FOREACH(_thread_running, l, tw)
     if (tw->index == index)
       return tw;
   return NULL;
}

static Eina_Thread
_eina_thread_win32_self(void)
{
   HANDLE t;
   Eina_Thread_Win32 *tw;
   Eina_List *l;

   t = GetCurrentThread();
   EINA_LIST_FOREACH(_thread_running, l, tw)
     if (tw->thread == t)
       return tw->index;

   /* We assume main loop == 0 on Windows */
   return 0;
}

static Eina_Bool
_eina_thread_win32_equal(Eina_Thread t1, Eina_Thread t2)
{
   if (t1 == t2) return EINA_TRUE;
   return EINA_FALSE;
}

static DWORD WINAPI
_eina_thread_win32_cb(LPVOID lpParam)
{
   Eina_Thread_Win32 *tw = lpParam;

   tw->ret = tw->func(tw->data);

   return 0;
}

static Eina_Bool
_eina_thread_win32_create(Eina_Thread *t,
                          int affinity,
                          void *(*func)(void *data),
                          const void *data)
{
   Eina_Thread_Win32 *tw;
   Eina_List *l;

   tw = eina_list_data_get(_thread_pool);
   _thread_pool = eina_list_remove_list(_thread_pool, _thread_pool);

   if (!tw)
     {
        tw = malloc(sizeof (Eina_Thread_Win32));
        if (!tw) goto on_error;

        do {
           tw->index = _current_index++;
        } while (tw->index == 0); /* prevent having a "false" main loop */
     }

   tw->func = func;
   tw->data = (void *)data;

   tw->thread = CreateThread(NULL, 0, _eina_thread_win32_cb, tw, 0, NULL);
   if (!tw->thread) goto on_error;

   /* affinity is an hint, if we fail, we continue without */
   SetThreadAffinityMask(tw->thread, 1 << affinity);

   _thread_running = eina_list_append(_thread_running, tw);

   *t = tw->index;
   return EINA_TRUE;

 on_error:
   _thread_pool = eina_list_append(_thread_pool, tw);
   return EINA_FALSE;
}

static void *
_eina_thread_win32_join(Eina_Thread t)
{
   Eina_Thread_Win32 *tw;
   void *ret;

   tw = _eina_thread_win32_find(t);
   if (!tw) return NULL;

   WaitForSingleObject(tw->thread, INFINITE);
   CloseHandle(tw->thread);

   ret = tw->ret;

   tw->ret = NULL;
   tw->thread = NULL;
   tw->func = NULL;
   tw->data = NULL;

   _thread_running = eina_list_remove(_thread_running, tw);
   _thread_pool = eina_list_append(_thread_pool, _thread_pool);

   return ret;
}

#  define PHE(x, y)       _eina_thread_win32_equal(x, y)
#  define PHS()           _eina_thread_win32_self()
#  define PHC(x, a, f, d) _eina_thread_win32_create(x, a, f, d)
#  define PHJ(x)          _eina_thread_win32_join(x)
#  define PHA(a)

# else
#  include <pthread.h>

#  ifdef __linux__
#   include <sched.h>
#   include <sys/resource.h>
#   include <unistd.h>
#   include <sys/syscall.h>
#   include <errno.h>
#  endif

static void *
_eina_thread_join(Eina_Thread t)
{
   void *ret = NULL;

   if (!pthread_join(t, &ret))
     return ret;
   return NULL;
}

static Eina_Bool
_eina_thread_create(Eina_Thread *t, int affinity, void *(*func)(void *data), void *data)
{
   Eina_Bool r;
   pthread_attr_t attr;
#ifdef EINA_HAVE_PTHREAD_AFFINITY
   cpu_set_t cpu;
   int cpunum;
#endif

   pthread_attr_init(&attr);
#ifdef EINA_HAVE_PTHREAD_AFFINITY
   if (affinity >= 0)
     {
        cpunum = eina_cpu_count();

        CPU_ZERO(&cpu);
        CPU_SET(affinity % cpunum, &cpu);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu), &cpu);
     }
#else
   (void) affinity;
#endif
   /* setup initial locks */
   r = pthread_create(t, &attr, func, data) == 0;
   pthread_attr_destroy(&attr);

   return r;
}

#  define PHE(x, y)       pthread_equal(x, y)
#  define PHS()           pthread_self()
#  define PHC(x, a, f, d) _eina_thread_create(x, a, f, d)
#  define PHJ(x)          _eina_thread_join(x)

# endif
#else
# error "Not supported any more"
#endif

typedef struct _Eina_Thread_Call Eina_Thread_Call;
struct _Eina_Thread_Call
{
   Eina_Thread_Cb func;
   const void *data;

   Eina_Thread_Priority prio;
   int affinity;
};

#include "eina_thread.h"

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
   return PHS();
}

EAPI Eina_Bool
eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return !!(PHE(t1, t2));
}

EAPI Eina_Bool
eina_thread_create(Eina_Thread *t,
                   Eina_Thread_Priority prio, int affinity,
                   Eina_Thread_Cb func, const void *data)
{
   Eina_Thread_Call *c;

   c = malloc(sizeof (Eina_Thread_Call));
   if (!c) return EINA_FALSE;

   c->func = func;
   c->data = data;
   c->prio = prio;
   c->affinity = affinity;

   if (PHC(t, affinity, _eina_internal_call, c))
     return EINA_TRUE;

   free(c);

   return EINA_FALSE;
}

EAPI void *
eina_thread_join(Eina_Thread t)
{
   return PHJ(t);
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
