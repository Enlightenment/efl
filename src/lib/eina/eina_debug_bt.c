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

static void             ***_bt_buf;
static int                *_bt_buf_len;
static struct timespec    *_bt_ts;
static int                *_bt_cpu;
static int                 _bt_threads_nb;

/* Used by trace timer */
static double _trace_t0 = 0.0;
static Eina_Debug_Timer *_timer = NULL;

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
        // we have little choice but to hope/assume dladdr() doesn't alloc
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

static Eina_Debug_Error
_prof_get_cb(Eina_Debug_Session *session EINA_UNUSED, int srcid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   clockid_t cid;
   int slot = eina_debug_thread_id_get();
   if (slot >= _bt_threads_nb) return EINA_DEBUG_OK;
   _bt_cpu[slot] = sched_getcpu();
   pthread_getcpuclockid(pthread_self(), &cid);
   clock_gettime(cid, &(_bt_ts[slot]));
   _bt_buf_len[slot] = _eina_debug_unwind_bt(_bt_buf[slot], EINA_MAX_BT);
   return EINA_DEBUG_OK;
}

static Eina_Bool
_trace_cb(void *data)
{
   static Eina_Debug_Packet_Header *hdr = NULL;
   static int bts = 0;
   int i;

   if (!hdr)
     {
        hdr = calloc(1, sizeof(*hdr));
        hdr->size = sizeof(Eina_Debug_Packet_Header);
        hdr->thread_id = 0xFFFFFFFF;
        hdr->opcode = _prof_get_op;
     }

   if (!_trace_t0) _trace_t0 = get_time();

   _bt_threads_nb = _eina_debug_thread_active_num;
   // reset our "stack" of memory se use to dump thread info into
   _eina_debug_chunk_tmp_reset();
   // get an array of pointers for the backtrace array for main + th
   _bt_buf = _eina_debug_chunk_tmp_push(_bt_threads_nb * sizeof(void *));
   if (!_bt_buf) goto err;
   // get an array of pointers for the timespec array for mainloop + th
   _bt_ts = _eina_debug_chunk_tmp_push(_bt_threads_nb * sizeof(struct timespec));
   if (!_bt_ts) goto err;
   // get an array of pointers for the cpuid array for mainloop + th
   _bt_cpu = _eina_debug_chunk_tmp_push(_bt_threads_nb * sizeof(int));
   if (!_bt_cpu) goto err;
   // get an array of void ptrs for each thread we know about for bt
   for (i = 0; i < _bt_threads_nb; i++)
     {
        _bt_buf[i] = _eina_debug_chunk_tmp_push(EINA_MAX_BT * sizeof(void *));
        if (!_bt_buf[i]) goto err;
     }
   // get an array of ints to stor the bt len for mainloop + threads
   _bt_buf_len = _eina_debug_chunk_tmp_push(_bt_threads_nb * sizeof(int));

   // now collect per thread
   eina_debug_dispatch(data, (void *)hdr);

   // we now have gotten all the data from all threads
   // we can process it now as we see fit, so release thread lock
   for (i = 0; i < _eina_debug_thread_active_num; i++)
     {
        _eina_debug_dump_fhandle_bt(stderr, _bt_buf[i], _bt_buf_len[i]);
     }
err:
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
static Eina_Debug_Error
_prof_on_cb(Eina_Debug_Session *session, int cid EINA_UNUSED, void *buffer, int size)
{
   unsigned int time;
   if (size >= 4)
     {
        memcpy(&time, buffer, 4);
        _trace_t0 = 0.0;
        _timer = eina_debug_timer_add(time, _trace_cb, session);
     }
   return EINA_DEBUG_OK;
}

static Eina_Debug_Error
_prof_off_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   eina_debug_timer_del(_timer);
   _timer = NULL;
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
