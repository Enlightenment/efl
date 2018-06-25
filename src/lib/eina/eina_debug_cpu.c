# ifndef _GNU_SOURCE
#  define _GNU_SOURCE 1
# endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eina_debug.h"
#include "eina_types.h"
#include "eina_list.h"
#include "eina_mempool.h"
#include "eina_util.h"
#include "eina_evlog.h"
#include "eina_debug_private.h"

#ifndef _WIN32
volatile int           _eina_debug_sysmon_reset = 0;
volatile int           _eina_debug_sysmon_active = 0;
volatile int           _eina_debug_evlog_active = 0;
volatile int           _eina_debug_cpu_active = 0;

Eina_Lock       _sysmon_lock;

static Eina_Thread       _sysmon_thread;

// this is a DEDICATED thread tojust collect system info and to have the
// least impact it can on a cpu core or system. all this does right now
// is sleep and wait for a command to begin polling for the cpu state.
// right now that means iterating through cpu's and getting their cpu
// frequency to match up with event logs.
static void *
_sysmon(void *data EINA_UNUSED, Eina_Thread thr EINA_UNUSED)
{
   static int cpufreqs[64] = { 0 };
   int i, fd, freq;
   char buf[256], path[256];
   ssize_t red;
#if defined(__clockid_t_defined)
   static struct timespec t_last = { 0, 0 };
   static Eina_Debug_Thread *prev_threads = NULL;
   static int prev_threads_num = 0;
   int j, cpu;
   Eina_Bool prev_threads_redo;
   clockid_t cid;
   struct timespec t, t_now;
   unsigned long long tim_span, tim1, tim2;
#endif

   // set a name for this thread for system debugging
   eina_thread_name_set(eina_thread_self(), "Edbg-sys");
   for (;;)
     {
        // wait on a mutex that will be locked for as long as this
        // threead is not meant to go running
        eina_lock_take(&_sysmon_lock);
        if (!_eina_debug_cpu_active) break;
        // if we need to reset as we just started polling system stats...
        if (_eina_debug_sysmon_reset)
          {
             _eina_debug_sysmon_reset = 0;
             // clear out all the clocks for threads
#if defined(__clockid_t_defined)
             // reset the last clock timestamp when we checked to "now"
             clock_gettime(CLOCK_MONOTONIC, &t);
             t_last = t;
             // walk over all threads
             eina_spinlock_take(&_eina_debug_thread_lock);
             for (i = 0; i < _eina_debug_thread_active_num; i++)
               {
                  // get the correct clock id to use for the target thread
                  pthread_getcpuclockid
                     (_eina_debug_thread_active[i].thread, &cid);
                  // get the clock cpu time accumulation for that threas
                  clock_gettime(cid, &t);
                  _eina_debug_thread_active[i].clok = t;
               }
             eina_spinlock_release(&_eina_debug_thread_lock);
#endif
             // clear all the cpu freq (up to 64 cores) to 0
             for (i = 0; i < 64; i++) cpufreqs[i] = 0;
          }
        eina_lock_release(&_sysmon_lock);

#if defined(__clockid_t_defined)
        // get the current time
        clock_gettime(CLOCK_MONOTONIC, &t_now);
        tim1 = (t_last.tv_sec * 1000000000LL) + t_last.tv_nsec;
        // the time span between now and last time we checked
        tim_span = ((t_now.tv_sec * 1000000000LL) + t_now.tv_nsec) - tim1;
        // if the time span is non-zero we might get sensible results
        if (tim_span > 0)
          {
             prev_threads_redo = EINA_FALSE;
             eina_spinlock_take(&_eina_debug_thread_lock);
             // figure out if the list of thread id's has changed since
             // our last poll. this is imporant as we need to set the
             // thread cpu usage to 0 for threads that have disappeared
             if (prev_threads_num != _eina_debug_thread_active_num)
               prev_threads_redo = EINA_TRUE;
             else
               {
                  // XXX: isolate this out of hot path
                  for (i = 0; i < _eina_debug_thread_active_num; i++)
                    {
                       if (_eina_debug_thread_active[i].thread !=
                           prev_threads[i].thread)
                         {
                            prev_threads_redo = EINA_TRUE;
                            break;
                         }
                    }
               }
             for (i = 0; i < _eina_debug_thread_active_num; i++)
               {
                  Eina_Thread thread = _eina_debug_thread_active[i].thread;
                  // get the clock for the thread and its cpu time usage
                  pthread_getcpuclockid(thread, &cid);
                  clock_gettime(cid, &t);
                  // calculate a long timestamp (64bits)
                  tim1 = (_eina_debug_thread_active[i].clok.tv_sec * 1000000000LL) +
                          _eina_debug_thread_active[i].clok.tv_nsec;
                  // and get the difference in clock time in NS
                  tim2 = ((t.tv_sec * 1000000000LL) + t.tv_nsec) - tim1;
                  // and that as percentage of the timespan
                  cpu = (int)((100 * (int)tim2) / (int)tim_span);
                  // round to the nearest 10 percent - rough anyway
                  cpu = ((cpu + 5) / 10) * 10;
                  if (cpu > 100) cpu = 100;
                  // if the usage changed since last time we checked...
                  if (cpu != _eina_debug_thread_active[i].val)
                    {
                       // log this change
                       snprintf(buf, sizeof(buf), "*CPUUSED %llu",
                                (unsigned long long)thread);
                       snprintf(path, sizeof(path), "%i", _eina_debug_thread_active[i].val);
                       eina_evlog(buf, NULL, 0.0, path);
                       snprintf(path, sizeof(path), "%i", cpu);
                       eina_evlog(buf, NULL, 0.0, path);
                       // store the clock time + cpu we got for next poll
                       _eina_debug_thread_active[i].val = cpu;
                    }
                  _eina_debug_thread_active[i].clok = t;
               }
             // so threads changed between this poll and last so we need
             // to redo our mapping/storage of them
             if (prev_threads_redo)
               {
                  prev_threads_redo = EINA_FALSE;
                  // find any threads from our last run that do not
                  // exist now in our new list of threads
                  for (j = 0; j < prev_threads_num; j++)
                    {
                       for (i = 0; i < _eina_debug_thread_active_num; i++)
                         {
                            if (prev_threads[j].thread ==
                                _eina_debug_thread_active[i].thread) break;
                         }
                       // thread was there before and not now
                       if (i == _eina_debug_thread_active_num)
                         {
                            // log it finishing - ie 0
                            snprintf(buf, sizeof(buf), "*CPUUSED %llu",
                                     (unsigned long long)
                                     prev_threads[i].thread);
                            eina_evlog(buf, NULL, 0.0, "0");
                         }
                    }
                  // if the thread count changed then allocate a new shadow
                  // buffer of thread id's etc.
                  if (prev_threads_num != _eina_debug_thread_active_num)
                    {
                       if (prev_threads) free(prev_threads);
                       prev_threads_num = _eina_debug_thread_active_num;
                       prev_threads = malloc(prev_threads_num *
                                             sizeof(Eina_Debug_Thread));
                    }
                  // store the thread handles/id's
                  for (i = 0; i < prev_threads_num; i++)
                    prev_threads[i].thread =
                    _eina_debug_thread_active[i].thread;
               }
             eina_spinlock_release(&_eina_debug_thread_lock);
             t_last = t_now;
          }
#endif
        // poll up to 64 cpu cores for cpufreq info to log alongside
        // the evlog call data
        for (i = 0; i < 64; i++)
          {
             // look at sysfs nodes per cpu
             snprintf
               (buf, sizeof(buf),
                "/sys/devices/system/cpu/cpu%i/cpufreq/scaling_cur_freq", i);
             fd = open(buf, O_RDONLY);
             freq = 0;
             // if the node is there, read it
             if (fd >= 0)
               {
                  // really low overhead read from cpufreq node (just an int)
                  red = read(fd, buf, sizeof(buf) - 1);
                  if (red > 0)
                    {
                       // read something - it should be an int with whitespace
                       buf[red] = 0;
                       freq = atoi(buf);
                       // move to mhz
                       freq = (freq + 500) / 1000;
                       // round mhz to the nearest 100mhz - to have less noise
                       freq = ((freq + 50) / 100) * 100;
                    }
                  // close the fd so we can freshly poll next time around
                  close(fd);
               }
             // node not there - ran out of cpu's to poll?
             else break;
             // if the current frequency changed vs previous poll, then log
             if (freq != cpufreqs[i])
               {
                  snprintf(buf, sizeof(buf), "*CPUFREQ %i", i);
                  snprintf(path, sizeof(path), "%i", freq);
                  eina_evlog(buf, NULL, 0.0, path);
                  cpufreqs[i] = freq;
               }
          }
        usleep(1000); // 1ms sleep
     }
   _eina_debug_cpu_active = -1;
   eina_lock_release(&_sysmon_lock);
   return NULL;
}

static Eina_Bool
_cpufreq_on_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   Eina_Bool err;
   if (!_eina_debug_evlog_active)
     {
        _eina_debug_evlog_active = 1;
        eina_evlog_start();
     }
   if (_eina_debug_sysmon_active) return EINA_TRUE;

   eina_lock_take(&_sysmon_lock);

   err = eina_thread_create(&_sysmon_thread, EINA_THREAD_NORMAL, -1, _sysmon, NULL);

   if (!err)
     {
        e_debug("EINA DEBUG ERROR: Can't create debug sysmon thread!");
        eina_lock_release(&_sysmon_lock);
        return EINA_FALSE;
     }
   _eina_debug_cpu_active = 1;
   _eina_debug_sysmon_reset = 1;
   _eina_debug_sysmon_active = 1;
   eina_lock_release(&_sysmon_lock);
   return EINA_TRUE;
}

static void
_stop_cpu_thread(void)
{
   extern Eina_Bool fork_resetting;
   eina_lock_take(&_sysmon_lock);
   _eina_debug_cpu_active = 0;
   eina_lock_release(&_sysmon_lock);
   /* wait for thread to exit */
   while (!fork_resetting)
     {
        usleep(1000);
        eina_lock_take(&_sysmon_lock);
        if (_eina_debug_cpu_active == -1) break;
        eina_lock_release(&_sysmon_lock);
     }
   _eina_debug_cpu_active = 0;
   eina_lock_release(&_sysmon_lock);
}

static Eina_Bool
_cpufreq_off_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   if (!_eina_debug_sysmon_active) return EINA_TRUE;
   _stop_cpu_thread();
   if (_eina_debug_evlog_active)
     {
        eina_evlog_stop();
        _eina_debug_evlog_active = 0;
     }
   return EINA_TRUE;
}

EINA_DEBUG_OPCODES_ARRAY_DEFINE(_OPS,
      {"CPU/Freq/on", NULL, &_cpufreq_on_cb},
      {"CPU/Freq/off", NULL, &_cpufreq_off_cb},
      {NULL, NULL, NULL}
);
#endif

Eina_Bool
_eina_debug_cpu_init(void)
{
#ifndef _WIN32
   eina_lock_new(&_sysmon_lock);
   eina_debug_opcodes_register(NULL, _OPS(), NULL, NULL);
#endif
   return EINA_TRUE;
}

Eina_Bool
_eina_debug_cpu_shutdown(void)
{
#ifndef _WIN32
   if (_eina_debug_sysmon_active)
     _stop_cpu_thread();
   eina_lock_free(&_sysmon_lock);
   _eina_debug_sysmon_reset = _eina_debug_sysmon_active = _eina_debug_evlog_active = 0;
#endif
   return EINA_TRUE;
}
