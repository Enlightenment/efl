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
# ifdef _WIN32
#  include <Evil.h>
# else
#  include <dlfcn.h>
# endif
#endif

#ifdef HAVE_UNWIND
#include <libunwind.h>
#endif

#include "eina_debug.h"
#include "eina_debug_private.h"

#ifndef _WIN32
# include <signal.h>
// realtime signals guarantee a minimum of 8, so SIGRTMIN + 7 would be valid
// at a minimum, so let's choose + 6 ... second last of the minimum set.
// SIGRTMAX of course is defined too... note the manual pages for sigation say
// that it calls rt_sigaction transparently for us so... no need for anything
// else special
# define SIG (SIGRTMIN + 6)
#endif

static Eina_Semaphore _wait_for_bts_sem;

// _bt_buf[0] is always for mainloop, 1 + is for extra threads
static void             ***_bt_buf;
static int                *_bt_buf_len;
static struct timespec    *_bt_ts;
static int                *_bt_cpu;

/* Used by trace timer */
static double _trace_t0 = 0.0;
static Eina_Debug_Timer *_timer = NULL;

void
_eina_debug_dump_fhandle_bt(FILE *f, void **bt, int btlen)
{
#ifndef _WIN32
   int i;
   Dl_info info;
   const char *file;
   unsigned long long offset, base;

   for (i = 0; i < btlen; i++)
     {
        file = NULL;
        offset = base = 0;
        // we have little choice but to hope/assume dladdr() doesn't alloc
        // anything here
        if ((dladdr(bt[i], &info)) && (info.dli_fname) && (info.dli_fname[0]))
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
#else
   (void)f;
   (void)bt;
   (void)btlen;
#endif
}

// a backtracer that uses libunwind to do the job
static inline int
_eina_debug_unwind_bt(void **bt, int max)
{
#ifdef HAVE_UNWIND
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
#else
   (void)bt;
   (void)max;
   return 0;
#endif
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

#ifndef _WIN32
static void
_signal_handler(int sig EINA_UNUSED,
      siginfo_t *si EINA_UNUSED, void *foo EINA_UNUSED)
{
   int i, slot = 0;
   pthread_t self = pthread_self();
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_SCHED_GETCPU) && defined(__clockid_t_defined)
   clockid_t cid;
#endif

   // find which slot in the array of threads we have so we store info
   // in the correct slot for us
   for (i = 0; i < _eina_debug_thread_active_num; i++)
     {
        if (self == _eina_debug_thread_active[i].thread)
          {
             slot = i;
             goto found;
          }
     }
   // we couldn't find out thread reference! help!
   e_debug("EINA DEBUG ERROR: can't find thread slot!");
   eina_semaphore_release(&_wait_for_bts_sem, 1);
   return;
found:
   /*
    * Below is very non-portable code!
    *
    * - clock_gettime() is not implemented on macOS < 10.12
    * - sched_getcpu() is not implemented on macOS
    * - pthread_getcpuclockid() is not implemented on macOS
    * - CLOCK_THREAD_CPUTIME_ID should be identical to pthread_getcpuclockid(),
    *   but it requires POSIX thingies to be defined.
    */
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_SCHED_GETCPU) && defined(__clockid_t_defined)
   // store thread info like what cpu core we are on now (not reliable
   // but hey - better than nothing), the amount of cpu time total
   // we have consumed (it's cumulative so subtracing deltas can give
   // you an average amount of cpu time consumed between now and the
   // previous time we looked) and also a full backtrace
   _bt_cpu[slot] = sched_getcpu();
# ifdef HAVE_PTHREAD_GETCPUCLOCKID
   /* Try pthread_getcpuclockid() first */
   pthread_getcpuclockid(self, &cid);
# elif defined(_POSIX_THREAD_CPUTIME)
   /* Fallback to POSIX clock id. */
   cid = CLOCK_THREAD_CPUTIME_ID;
# else
   /* Boom, we lost */
#  error Cannot determine the clock id for clock_gettime()
# endif
   clock_gettime(cid, &(_bt_ts[slot]));
   _bt_buf_len[slot] = _eina_debug_unwind_bt(_bt_buf[slot], EINA_MAX_BT);
#endif /* HAVE_CLOCK_GETTIME && HAVE_SCHED_GETCPU */
   // now wake up the monitor to let them know we are done collecting our
   // backtrace info
   eina_semaphore_release(&_wait_for_bts_sem, 1);
}
#endif

static void
_signal_init(void)
{
#ifndef _WIN32
   struct sigaction sa;

   // set up signal handler for our profiling signal - eevery thread should
   // obey this (this is the case on linux - other OSs may vary)
   sa.sa_sigaction = _signal_handler;
   sa.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&sa.sa_mask);
   if (sigaction(SIG, &sa, NULL) != 0)
      e_debug("EINA DEBUG ERROR: Can't set up sig %i handler!", SIG);

   sa.sa_sigaction = NULL;
   sa.sa_handler = SIG_IGN;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   if (sigaction(SIGPIPE, &sa, 0) == -1) perror(0);
#endif
}

static void
_collect_bt(pthread_t pth)
{
   // this async signals the thread to switch to the deebug signal handler
   // and collect a backtrace and other info from inside the thread
#ifndef _WIN32
   pthread_kill(pth, SIG);
#endif
}

static Eina_Bool
_trace_cb(void *data EINA_UNUSED)
{
   static int bts = 0;
   int i;

   if (!_trace_t0) _trace_t0 = get_time();

   // take a lock on grabbing thread debug info like backtraces
   eina_spinlock_take(&_eina_debug_thread_lock);
   // too many threads (over 1 million) !!!!
   if (_eina_debug_thread_active_num > (1024 * 1024)) goto err;
   // reset our "stack" of memory se use to dump thread info into
   _eina_debug_chunk_tmp_reset();
   // get an array of pointers for the backtrace array for main + th
   _bt_buf = _eina_debug_chunk_tmp_push
      ((_eina_debug_thread_active_num) * sizeof(void *));
   if (!_bt_buf) goto err;
   // get an array of pointers for the timespec array for mainloop + th
   _bt_ts = _eina_debug_chunk_tmp_push
      ((_eina_debug_thread_active_num) * sizeof(struct timespec));
   if (!_bt_ts) goto err;
   // get an array of pointers for the cpuid array for mainloop + th
   _bt_cpu = _eina_debug_chunk_tmp_push
      ((_eina_debug_thread_active_num) * sizeof(int));
   if (!_bt_cpu) goto err;
   // get an array of void ptrs for each thread we know about for bt
   for (i = 0; i < _eina_debug_thread_active_num; i++)
     {
        _bt_buf[i] = _eina_debug_chunk_tmp_push(EINA_MAX_BT * sizeof(void *));
        if (!_bt_buf[i]) goto err;
     }
   // get an array of ints to stor the bt len for mainloop + threads
   _bt_buf_len = _eina_debug_chunk_tmp_push
      ((_eina_debug_thread_active_num) * sizeof(int));
   // now collect per thread
   for (i = 0; i < _eina_debug_thread_active_num; i++)
      _collect_bt(_eina_debug_thread_active[i].thread);
   // we're done probing. now collec all the "i'm done" msgs on the
   // semaphore for every thread + mainloop
   for (i = 0; i < (_eina_debug_thread_active_num); i++)
      eina_semaphore_lock(&_wait_for_bts_sem);
   // we now have gotten all the data from all threads
   // we can process it now as we see fit, so release thread lock
   for (i = 0; i < _eina_debug_thread_active_num; i++)
     {
        _eina_debug_dump_fhandle_bt(stderr, _bt_buf[i], _bt_buf_len[i]);
     }
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
   return EINA_TRUE;
}

// profiling on with poll time gap as uint payload
static Eina_Bool
_prof_on_cb(Eina_Debug_Session *session, int cid EINA_UNUSED, void *buffer, int size)
{
   unsigned int time;
   if (size >= 4)
     {
        memcpy(&time, buffer, 4);
        _trace_t0 = 0.0;
        if (_timer) eina_debug_timer_del(_timer);
        _timer = eina_debug_timer_add(time, _trace_cb, session);
     }
   return EINA_TRUE;
}

static Eina_Bool
_prof_off_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   eina_debug_timer_del(_timer);
   _timer = NULL;
   return EINA_TRUE;
}

EINA_DEBUG_OPCODES_ARRAY_DEFINE(_OPS,
      {"Profiler/on", NULL, &_prof_on_cb},
      {"Profiler/off", NULL, &_prof_off_cb},
      {NULL, NULL, NULL}
);

Eina_Bool
_eina_debug_bt_init(void)
{
   _signal_init();
   eina_semaphore_new(&_wait_for_bts_sem, 0);
   eina_debug_opcodes_register(NULL, _OPS(), NULL, NULL);
   return EINA_TRUE;
}

Eina_Bool
_eina_debug_bt_shutdown(void)
{
   eina_semaphore_free(&_wait_for_bts_sem);
   return EINA_TRUE;
}

