/* EINA - EFL data type library
 * Copyright (C) 2017 Carsten Haitzler
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

# ifndef _GNU_SOURCE
#  define _GNU_SOURCE 1
# endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#ifdef HAVE_SYS_EPOLL_H
# include <sys/epoll.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

#include "eina_debug.h"
#include "eina_debug_private.h"

static Eina_Spinlock _lock;

struct _Eina_Debug_Timer
{
   unsigned int rel_time;
   unsigned int timeout;
   Eina_Debug_Timer_Cb cb;
   void *data;
};

static Eina_List *_timers = NULL;

static Eina_Bool _thread_runs = EINA_FALSE;
static pthread_t _thread;

static int pipeToThread[2];

static void
_timer_append(Eina_Debug_Timer *t)
{
   Eina_Debug_Timer *t2;
   Eina_List *itr;
   unsigned int prev_time = 0;
   char c = '\0';
   EINA_LIST_FOREACH(_timers, itr, t2)
     {
        if (t2->timeout > t->timeout) goto end;
        prev_time = t2->timeout;
     }
   t2 = NULL;
end:
   t->rel_time = t->timeout - prev_time;
   if (!t2) _timers = eina_list_append(_timers, t);
   else _timers = eina_list_prepend_relative(_timers, t, t2);
   if (write(pipeToThread[1], &c, 1) != 1)
     e_debug("EINA DEBUG ERROR: Can't wake up thread for debug timer");
}

static void *
_monitor(void *_data EINA_UNUSED)
{
#ifdef HAVE_SYS_EPOLL_H
#define MAX_EVENTS   4
   struct epoll_event event;
   struct epoll_event events[MAX_EVENTS];
   int epfd = epoll_create(MAX_EVENTS), ret;

   event.data.fd = pipeToThread[0];
   event.events = EPOLLIN;
   ret = epoll_ctl(epfd, EPOLL_CTL_ADD, event.data.fd, &event);
   if (ret) perror("epoll_ctl/add");
#ifdef EINA_HAVE_PTHREAD_SETNAME
# ifndef __linux__
   pthread_set_name_np
# else
   pthread_setname_np
# endif
     (pthread_self(), "Edbg-tim");
#endif
   while (1)
     {
        int timeout = -1; //in milliseconds
        pthread_testcancel();
        eina_spinlock_take(&_lock);
        if (_timers)
          {
             Eina_Debug_Timer *t = eina_list_data_get(_timers);
             timeout = t->timeout;
          }
        eina_spinlock_release(&_lock);

        ret = epoll_wait(epfd, events, MAX_EVENTS, timeout);
        pthread_testcancel();

        /* Some timer has been add/removed or we need to exit */
        if (ret)
          {
             char c;
             if (read(pipeToThread[0], &c, 1) != 1) break;
          }
        else
          {
             Eina_List *itr, *itr2, *renew = NULL;
             Eina_Debug_Timer *t;
             eina_spinlock_take(&_lock);
             EINA_LIST_FOREACH_SAFE(_timers, itr, itr2, t)
               {
                  if (itr == _timers || t->rel_time == 0)
                    {
                       _timers = eina_list_remove(_timers, t);
                       if (t->cb(t->data)) renew = eina_list_append(renew, t);
                       else free(t);
                    }
               }
             EINA_LIST_FREE(renew, t) _timer_append(t);
             eina_spinlock_release(&_lock);
          }
     }
#endif
   _thread_runs = EINA_FALSE;
   close(pipeToThread[0]);
   close(pipeToThread[1]);
   return NULL;
}

EAPI Eina_Debug_Timer *
eina_debug_timer_add(unsigned int timeout_ms, Eina_Debug_Timer_Cb cb, void *data)
{
   if (!cb || !timeout_ms) return NULL;
   Eina_Debug_Timer *t = calloc(1, sizeof(*t));
   t->cb = cb;
   t->data = data;
   t->timeout = timeout_ms;
   eina_spinlock_take(&_lock);
   _timer_append(t);
   if (!_thread_runs)
     {
#ifndef _WIN32
        sigset_t oldset, newset;

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
        int err = pthread_create(&_thread, NULL, _monitor, NULL);
#ifndef _WIN32
        pthread_sigmask(SIG_SETMASK, &oldset, NULL);
#endif
        if (err != 0)
          {
             e_debug("EINA DEBUG ERROR: Can't create debug timer thread!");
             abort();
          }
        _thread_runs = EINA_TRUE;
     }
   eina_spinlock_release(&_lock);
   return t;
}

EAPI void
eina_debug_timer_del(Eina_Debug_Timer *t)
{
   eina_spinlock_take(&_lock);
   Eina_List *itr = eina_list_data_find_list(_timers, t);
   if (itr)
     {
        _timers = eina_list_remove_list(_timers, itr);
        free(t);
     }
   eina_spinlock_release(&_lock);
}

Eina_Bool
_eina_debug_timer_init(void)
{
   eina_spinlock_new(&_lock);
#ifndef _WIN32
   if (pipe(pipeToThread) == -1)
     return  EINA_FALSE;
#endif
   return EINA_TRUE;
}

Eina_Bool
_eina_debug_timer_shutdown(void)
{
   Eina_Debug_Timer *t;

   eina_spinlock_take(&_lock);
   EINA_LIST_FREE(_timers, t)
     free(t);
   close(pipeToThread[0]);
   close(pipeToThread[1]);
   if (_thread_runs)
     pthread_cancel(_thread);
   _thread_runs = 0;
   eina_spinlock_release(&_lock);
   eina_spinlock_free(&_lock);

   return EINA_TRUE;
}

