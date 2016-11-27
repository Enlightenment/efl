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

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#ifdef HAVE_DLADDR
# include <dlfcn.h>
#endif
#include <libunwind.h>

#include "eina_debug.h"
#include "eina_debug_private.h"

static Eina_Semaphore _wait_for_bts_sem;

// _bt_buf[0] is always for mainloop, 1 + is for extra threads
static void             ***_bt_buf;
static int                *_bt_buf_len;
static struct timespec    *_bt_ts;
static int                *_bt_cpu;

/* Used by trace timer */
static double _trace_t0 = 0.0;

static int _prof_get_op = EINA_DEBUG_OPCODE_INVALID;

void
_eina_debug_dump_fhandle_bt(FILE *f, void **bt, int btlen)
{
   int i;
   Dl_info info;
   const char *file;
   unsigned long long offset, base;

   for (i = 0; i < btlen; i++)
     {
        file = NULL;
        offset = base = 0;
        // we have little choice but to hgope/assume dladdr() doesn't alloc
        // anything here
        if ((dladdr(bt[i], &info)) && (info.dli_fname[0]))
          {
             offset = (unsigned long long)(uintptr_t)bt[i];
             base = (unsigned long long)(uintptr_t)info.dli_fbase;
             file = _eina_debug_file_get(info.dli_fname);
          }
        // rely on normal libc buffering for file ops to avoid syscalls.
        // may or may not be a good idea. good enough for now.
        if (file) fprintf(f, "%s\t 0x%llx 0x%llx\n", file, offset, base);
        else fprintf(f, "??\t -\n");
     }
}

// a backtracer that uses libunwind to do the job
static inline int
_eina_debug_unwind_bt(void **bt, int max)
{
   unw_cursor_t cursor;
   unw_context_t uc;
   unw_word_t p;
   int total;

   // create a context for unwinding
   unw_getcontext(&uc);
   // begin our work
   unw_init_local(&cursor, &uc);
   // walk up each stack frame until there is no more, storing it
   for (total = 0; (unw_step(&cursor) > 0) && (total < max); total++)
     {
        unw_get_reg(&cursor, UNW_REG_IP, &p);
        bt[total] = (void *)p;
     }
   // return our total backtrace stack size
   return total;
}

// this signal handler is called inside each and every thread when the
// thread gets a signal via pthread_kill(). this causes the thread to
// stop here inside this handler and "do something" then when this returns
// resume whatever it was doing like any signal handler
static void
_eina_debug_signal(int sig EINA_UNUSED,
                   siginfo_t *si EINA_UNUSED,
                   void *foo EINA_UNUSED)
{
   int i, slot = 0;
   pthread_t self = pthread_self();
   clockid_t cid;

   // find which slot in the array of threads we have so we store info
   // in the correct slot for us
   if (self != _eina_debug_thread_mainloop)
     {
        for (i = 0; i < _eina_debug_thread_active_num; i++)
          {
             if (self == _eina_debug_thread_active[i].thread)
               {
                  slot = i + 1;
                  goto found;
               }
          }
        // we couldn't find out thread reference! help!
        e_debug("EINA DEBUG ERROR: can't find thread slot!");
        eina_semaphore_release(&_wait_for_bts_sem, 1);
        return;
     }
found:
   // store thread info like what cpu core we are on now (not reliable
   // but hey - better than nothing), the amount of cpu time total
   // we have consumed (it's cumulative so subtracing deltas can give
   // you an average amount of cpu time consumed between now and the
   // previous time we looked) and also a full backtrace
   _bt_cpu[slot] = sched_getcpu();
   pthread_getcpuclockid(self, &cid);
   clock_gettime(cid, &(_bt_ts[slot]));
   _bt_buf_len[slot] = _eina_debug_unwind_bt(_bt_buf[slot], EINA_MAX_BT);
   // now wake up the monitor to let them know we are done collecting our
   // backtrace info
   eina_semaphore_release(&_wait_for_bts_sem, 1);
}


// a quick and dirty local time point getter func - not portable
static inline double
get_time(void)
{
#if defined(__clockid_t_defined)
   struct timespec t;
   clock_gettime(CLOCK_MONOTONIC, &t);
   return (double)t.tv_sec + (((double)t.tv_nsec) / 1000000000.0);
#else
   struct timeval timev;
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000.0);
#endif
}

static void
_eina_debug_collect_bt(pthread_t pth EINA_UNUSED)
{
   // this async signals the thread to switch to the deebug signal handler
   // and collect a backtrace and other info from inside the thread
   //pthread_kill(pth, SIG);
}

static Eina_Bool
_trace_cb(void *data)
{
   static Eina_Debug_Packet_Header *hdr = NULL;

   if (!hdr)
     {
        hdr = calloc(1, sizeof(*hdr));
        hdr->size = sizeof(Eina_Debug_Packet_Header);
        hdr->thread_id = 0xFFFFFFFF;
        hdr->opcode = _prof_get_op;
     }

   eina_debug_dispatch(data, (void *)hdr);
   return EINA_TRUE;
}

static Eina_Debug_Error
_prof_get_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   static int bts = 0;
   int i;
   if (!_trace_t0) _trace_t0 = get_time();
   // take a lock on grabbing thread debug info like backtraces
   eina_spinlock_take(&_eina_debug_thread_lock);
   // reset our "stack" of memory se use to dump thread info into
   _eina_debug_chunk_tmp_reset();
   // get an array of pointers for the backtrace array for main + th
   _bt_buf = _eina_debug_chunk_tmp_push
      ((1 + _eina_debug_thread_active_num) * sizeof(void *));
   if (!_bt_buf) goto err;
   // get an array of pointers for the timespec array for mainloop + th
   _bt_ts = _eina_debug_chunk_tmp_push
      ((1 + _eina_debug_thread_active_num) * sizeof(struct timespec));
   if (!_bt_ts) goto err;
   // get an array of pointers for the cpuid array for mainloop + th
   _bt_cpu = _eina_debug_chunk_tmp_push
      ((1 + _eina_debug_thread_active_num) * sizeof(int));
   if (!_bt_cpu) goto err;
   // now get an array of void pts for mainloop bt
   _bt_buf[0] = _eina_debug_chunk_tmp_push(EINA_MAX_BT * sizeof(void *));
   if (!_bt_buf[0]) goto err;
   // get an array of void ptrs for each thread we know about for bt
   for (i = 0; i < _eina_debug_thread_active_num; i++)
     {
        _bt_buf[i + 1] = _eina_debug_chunk_tmp_push(EINA_MAX_BT * sizeof(void *));
        if (!_bt_buf[i + 1]) goto err;
     }
   // get an array of ints to stor the bt len for mainloop + threads
   _bt_buf_len = _eina_debug_chunk_tmp_push
      ((1 + _eina_debug_thread_active_num) * sizeof(int));
   // collect bt from the mainloop - always there
   _eina_debug_collect_bt(_eina_debug_thread_mainloop);
   // now collect per thread
   for (i = 0; i < _eina_debug_thread_active_num; i++)
      _eina_debug_collect_bt(_eina_debug_thread_active[i].thread);
   // we're done probing. now collec all the "i'm done" msgs on the
   // semaphore for every thread + mainloop
   for (i = 0; i < (_eina_debug_thread_active_num + 1); i++)
      eina_semaphore_lock(&_wait_for_bts_sem);
   // we now have gotten all the data from all threadd + mainloop.
   // we can process it now as we see fit, so release thread lock
   //// XXX: some debug so we can see the bt's we collect - will go
   //                  for (i = 0; i < (_eina_debug_thread_active_num + 1); i++)
   //                    {
   //                       _eina_debug_dump_fhandle_bt(stderr, _bt_buf[i], _bt_buf_len[i]);
   //                    }
err:
   eina_spinlock_release(&_eina_debug_thread_lock);
   //// XXX: some debug just to see how well we perform - will go
   bts++;
   if (bts >= 10000)
     {
        double t;
        t = get_time();
        e_debug("%1.5f bt's per sec", (double)bts / (t - _trace_t0));
        _trace_t0 = t;
        bts = 0;
     }
   return EINA_DEBUG_OK;
}

// profiling on with poll time gap as uint payload
static Eina_Debug_Error
_prof_on_cb(Eina_Debug_Session *session, int cid EINA_UNUSED, void *buffer, int size)
{
   unsigned int time;
   if (size >= 4)
     {
        memcpy(&time, buffer, 4);
        _trace_t0 = 0.0;
        eina_debug_timer_add(time, _trace_cb, session);
     }
   return EINA_DEBUG_OK;
}

static Eina_Debug_Error
_prof_off_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   eina_debug_timer_add(0, NULL, NULL);
   return EINA_DEBUG_OK;
}

static const Eina_Debug_Opcode _OPS[] = {
       {"profiler/on", NULL, &_prof_on_cb},
       {"profiler/off", NULL, &_prof_off_cb},
       {"profiler/bt_get", &_prof_get_op, &_prof_get_cb},
       {NULL, NULL, NULL}
};

Eina_Bool
_eina_debug_bt_init(void)
{
   eina_semaphore_new(&_wait_for_bts_sem, 0);
   eina_debug_opcodes_register(NULL, _OPS, NULL);
   return EINA_TRUE;
}

Eina_Bool
_eina_debug_bt_shutdown(void)
{
   eina_semaphore_free(&_wait_for_bts_sem);
   return EINA_TRUE;
}
