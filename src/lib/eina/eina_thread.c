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
#ifdef _WIN32
# include "eina_list.h"
# include "eina_lock.h"
#endif

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_thread.h"

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN

typedef struct _Eina_TLS_Cbs_Win32 Eina_TLS_Cbs_Win32;
struct _Eina_TLS_Cbs_Win32
{
   Eina_TLS key;
   Eina_TLS_Delete_Cb cb;
};
typedef struct _Eina_Thread_Win32 Eina_Thread_Win32;
struct _Eina_Thread_Win32
{
   HANDLE thread;
   void *(*func)(void *data);
   void *data;
   void *ret;
   Eina_List *tls_keys;

   Eina_Thread index;
};

/* FIXME: For the moment Eina_Thread is considered not
   thread safe, wondering if it's worth it */
static unsigned long int _current_index = 1; /* start from one as the main loop == 0 */
static Eina_List *_thread_pool = NULL;
static Eina_List *_thread_running = NULL;
static Eina_List *_tls_keys_cbs = NULL;

static inline Eina_TLS_Cbs_Win32 *
_eina_thread_tls_cb_find(Eina_TLS key)
{
   Eina_TLS_Cbs_Win32 *cb;
   Eina_List *l;

   EINA_LIST_FOREACH(_tls_keys_cbs, l, cb)
      if (cb->key == key)
        return cb;

   return NULL;
}

static inline void
_eina_thread_tls_keys_clean(Eina_Thread_Win32 *tw)
{
   void *data;
   Eina_TLS_Cbs_Win32 *cb;

   EINA_LIST_FREE(tw->tls_keys, data)
     {
        Eina_TLS key = (Eina_TLS)(uintptr_t)data;
        cb = _eina_thread_tls_cb_find(key);
        if (cb)
          cb->cb(eina_tls_get(key));
     }
   tw->tls_keys = NULL;
}

EAPI Eina_Bool
_eina_thread_tls_cb_register(Eina_TLS key, Eina_TLS_Delete_Cb cb)
{
   Eina_TLS_Cbs_Win32 *tls_cb = malloc(sizeof(Eina_TLS_Cbs_Win32));
   if (!cb) return EINA_FALSE;

   tls_cb->key = key;
   tls_cb->cb = cb;
   _tls_keys_cbs = eina_list_append(_tls_keys_cbs, tls_cb);

   return EINA_TRUE;
}

EAPI Eina_Bool
_eina_thread_tls_cb_unregister(Eina_TLS key)
{
   Eina_TLS_Cbs_Win32 *cb = _eina_thread_tls_cb_find(key);
   if (!cb) return EINA_FALSE;

   _tls_keys_cbs = eina_list_remove(_tls_keys_cbs, cb);
   free(cb);

   return EINA_TRUE;
}

EAPI Eina_Bool
_eina_thread_tls_key_add(Eina_TLS key)
{
   HANDLE t;
   Eina_Thread_Win32 *tw;
   Eina_List *l;

   t = GetCurrentThread();
   EINA_LIST_FOREACH(_thread_running, l, tw)
      if (tw->thread == t)
        {
           void *data = (void *)(uintptr_t)key;
           if (!eina_list_data_find(tw->tls_keys, data))
             tw->tls_keys = eina_list_append(tw->tls_keys, data);
           return EINA_TRUE;
        }

   return EINA_FALSE;
}

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

static inline Eina_Thread
_eina_thread_self(void)
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

static inline Eina_Bool
_eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
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

static inline Eina_Bool
_eina_thread_create(Eina_Thread *t,
                          int affinity,
                          void *(*func)(void *data),
                          const void *data)
{
   Eina_Thread_Win32 *tw;

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
   tw->tls_keys = NULL;

   tw->thread = CreateThread(NULL, 0, _eina_thread_win32_cb, tw, 0, NULL);
   if (!tw->thread) goto on_error;

   /* affinity is an hint, if we fail, we continue without */
   if (affinity >= 0)
     SetThreadAffinityMask(tw->thread, 1 << affinity);

   _thread_running = eina_list_append(_thread_running, tw);

   *t = tw->index;
   return EINA_TRUE;

 on_error:
   _thread_pool = eina_list_append(_thread_pool, tw);
   return EINA_FALSE;
}

static inline void *
_eina_thread_join(Eina_Thread t)
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
   _eina_thread_tls_keys_clean(tw);

   _thread_running = eina_list_remove(_thread_running, tw);
   _thread_pool = eina_list_append(_thread_pool, _thread_pool);

   return ret;
}

#elif defined(EFL_HAVE_POSIX_THREADS)
# include <pthread.h>
# include <errno.h>

static inline void *
_eina_thread_join(Eina_Thread t)
{
   void *ret = NULL;
   int err = pthread_join(t, &ret);

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
   err = pthread_create(t, &attr, func, data);
   pthread_attr_destroy(&attr);

   if (err == 0) return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
_eina_thread_equal(Eina_Thread t1, Eina_Thread t2)
{
   return pthread_equal(t1, t2);
}

static inline Eina_Thread
_eina_thread_self(void)
{
   return pthread_self();
}

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
