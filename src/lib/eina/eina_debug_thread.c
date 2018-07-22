/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
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

#include "eina_debug.h"
#include "eina_debug_private.h"

// a really simple store of currently known active threads. the mainloop is
// special and inittied at debug init time - assuming eina inits in the
// mainloop thread (whihc is expected). also a growable array of thread
// id's for other threads is held here so we can loop over them and do things
// like get them to stop and dump a backtrace for us
Eina_Spinlock         _eina_debug_thread_lock;

Eina_Thread             _eina_debug_thread_mainloop = 0;
Eina_Debug_Thread    *_eina_debug_thread_active = NULL;
int                   _eina_debug_thread_active_num = 0;

static int            _thread_active_size = 0;
static int            _thread_id_counter = 1;

// add a thread id to our tracking array - very simple. add to end, and
// if array to small, reallocate it to be bigger by 16 slots AND double that
// size (so grows should slow down FAST). we will never shrink this array
void
_eina_debug_thread_add(void *th)
{
   Eina_Thread *pth = th;
   // take thread tracking lock
   eina_spinlock_take(&_eina_debug_thread_lock);
   // if we don't have enough space to store thread id's - make some more
   if (_thread_active_size < (_eina_debug_thread_active_num + 1))
     {
        Eina_Debug_Thread *threads = realloc
          (_eina_debug_thread_active,
           ((_eina_debug_thread_active_num + 16) * 2) *
            sizeof(Eina_Debug_Thread));
        if (threads)
          {
             _eina_debug_thread_active = threads;
             _thread_active_size = (_eina_debug_thread_active_num + 16) * 2;
          }
     }
   // add new thread id to the end
   _eina_debug_thread_active[_eina_debug_thread_active_num].thread = *pth;
#if defined(__clockid_t_defined)
   _eina_debug_thread_active[_eina_debug_thread_active_num].clok.tv_sec = 0;
   _eina_debug_thread_active[_eina_debug_thread_active_num].clok.tv_nsec = 0;
   _eina_debug_thread_active[_eina_debug_thread_active_num].val = -1;
#endif
   _eina_debug_thread_active[_eina_debug_thread_active_num].thread_id = _thread_id_counter++;
   _eina_debug_thread_active_num++;
   // release our lock cleanly
   eina_spinlock_release(&_eina_debug_thread_lock);
}

// remove a thread id from our tracking array - simply find and shuffle all
// later elements down. this array should be small almsot all the time and
// shouldn't bew changing THAT often for this to matter
void
_eina_debug_thread_del(void *th)
{
   Eina_Thread *pth = th;
   int i;
   // take a thread tracking lock
   eina_spinlock_take(&_eina_debug_thread_lock);
   // find the thread id to remove
   for (i = 0; i < _eina_debug_thread_active_num; i++)
     {
        if (_eina_debug_thread_active[i].thread == *pth)
          {
             // found it - now shuffle down all further thread id's in array
             for (; i < (_eina_debug_thread_active_num - 1); i++)
               _eina_debug_thread_active[i] = _eina_debug_thread_active[i + 1];
             // reduce our counter and get out of loop
             _eina_debug_thread_active_num--;
             break;
          }
     }
   // release lock cleanly
   eina_spinlock_release(&_eina_debug_thread_lock);
}

// register the thread that is the mainloop - always there
void
_eina_debug_thread_mainloop_set(void *th)
{
   Eina_Thread *pth = th;
   _eina_debug_thread_mainloop = *pth;
}

