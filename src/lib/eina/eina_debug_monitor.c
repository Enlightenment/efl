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
#include "eina_types.h"
#include "eina_evlog.h"
#include "eina_util.h"

#ifdef EINA_HAVE_DEBUG

#define DEBUG_SERVER ".ecore/efl_debug/0"

extern pthread_t            _eina_debug_thread_mainloop;
extern volatile pthread_t  *_eina_debug_thread_active;
extern volatile int         _eina_debug_thread_active_num;

int                    _eina_debug_monitor_service_fd = -1;
Eina_Semaphore         _eina_debug_monitor_return_sem;

static Eina_Bool       _monitor_thread_runs = EINA_FALSE;
static pthread_t       _monitor_thread;

// _bt_buf[0] is always for mainloop, 1 + is for extra threads
static void             ***_bt_buf;
static int                *_bt_buf_len;
static struct timespec    *_bt_ts;
static int                *_bt_cpu;

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

static inline void
_bt_cpu_set(int slot)
{
#if HAVE_SCHED_GETCPU
   _bt_cpu[slot] = sched_getcpu();
#else
   _bt_cpu[slot] = -1;
#endif
}

static inline void
_bt_ts_set(int slot, pthread_t self)
{
#if defined(__clockid_t_defined)
   clockid_t cid;
   pthread_getcpuclockid(self, &cid);
   clock_gettime(cid, &(_bt_ts[slot]));
#else
   (void) self;
   memset(&(_bt_ts[slot]), 0, sizeof(struct timespec));
#endif
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

   // find which slot in the array of threads we have so we store info
   // in the correct slot for us
   if (self != _eina_debug_thread_mainloop)
     {
        for (i = 0; i < _eina_debug_thread_active_num; i++)
          {
             if (self == _eina_debug_thread_active[i])
               {
                  slot = i + 1;
                  goto found;
               }
          }
        // we couldn't find out thread reference! help!
        fprintf(stderr, "EINA DEBUG ERROR: can't find thread slot!\n");
        eina_semaphore_release(&_eina_debug_monitor_return_sem, 1);
        return;
     }
found:
   // store thread info like what cpu core we are on now (not reliable
   // but hey - better than nothing), the amount of cpu time total
   // we have consumed (it's cumulative so subtracing deltas can give
   // you an average amount of cpu time consumed between now and the
   // previous time we looked) and also a full backtrace
   _bt_cpu_set(slot);
   _bt_ts_set(slot, self);
   _bt_buf_len[slot] = _eina_debug_unwind_bt(_bt_buf[slot], EINA_MAX_BT);
   // now wake up the monitor to let them know we are done collecting our
   // backtrace info
   eina_semaphore_release(&_eina_debug_monitor_return_sem, 1);
}

// we shall sue SIGPROF as out signal for pausing threads and having them
// dump a backtrace for polling based profiling
#define SIG SIGPROF

// a quick and dirty local time point getter func - not portable
static inline double
get_time(void)
{
   struct timeval timev;
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000.0);
}

static void
_eina_debug_collect_bt(pthread_t pth)
{
   // this async signals the thread to switch to the deebug signal handler
   // and collect a backtrace and other info from inside the thread
   pthread_kill(pth, SIG);
}

// this is a DEDICATED debug thread to monitor the application so it works
// even if the mainloop is blocked or the app otherwise deadlocked in some
// way. this is an alternative to using external debuggers so we can get
// users or developers to get useful information about an app at all times
static void *
_eina_debug_monitor(void *_data EINA_UNUSED)
{
   int bts = 0, ret, max_fd;
   double t0, t;
   fd_set rfds, wfds, exfds;
   struct timeval tv = { 0, 0 };
   // some state for debugging
   unsigned int poll_time = 1000;
   Eina_Bool poll_on = EINA_FALSE;
   Eina_Bool poll_trace = EINA_FALSE;
   Eina_Bool poll_cpu = EINA_FALSE;

   t0 = get_time();
   // sit forever processing commands or timeouts in the debug monitor
   // thread - this is separate to the rest of the app so it shouldn't
   // impact the application specifically
   for (;;)
     {
        int i;

        // set up data for select like read fd's
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&exfds);
        // the only fd we care about - out debug daemon connection
        FD_SET(_eina_debug_monitor_service_fd, &rfds);
        max_fd = _eina_debug_monitor_service_fd;
        // if we are in a polling mode then set up a timeout and wait for it
        if (poll_on)
          {
             if ((tv.tv_sec == 0) && (tv.tv_usec == 0))
               {
                  tv.tv_sec = 0;
                  tv.tv_usec = poll_time;
               }
             ret = select(max_fd + 1, &rfds, &wfds, &exfds, &tv);
          }
        // we have no timeout - so wait forever for a message from debugd
        else ret = select(max_fd + 1, &rfds, &wfds, &exfds, NULL);
        // if the fd for debug daemon says it's alive, process it
        if ((ret == 1) && (FD_ISSET(_eina_debug_monitor_service_fd, &rfds)))
          {
             // collect a single op on the debug daemon control fd
             char op[5];
             int size;
             unsigned char *data;

             // get the opcode and stor in op - guarantee its 0 byte terminated
             data = NULL;
             size = _eina_debug_monitor_service_read(op, &data);
             // if not negative - we have a real message
             if (size >= 0)
               {
                  // profiling on with poll time gap as uint payload
                  if (!strcmp(op, "PLON"))
                    {
                       if (size >= 4) memcpy(&poll_time, data, 4);
                       poll_on = EINA_TRUE;
                       poll_trace = EINA_TRUE;
                    }
                  // profiling off with no payload
                  else if (!strcmp(op, "PLOF"))
                    {
                       poll_time = 1000;
                       poll_on = EINA_FALSE;
                       poll_trace = EINA_FALSE;
                    }
                  // enable evlog
                  else if (!strcmp(op, "EVON"))
                    {
                       eina_evlog_start();
                    }
                  // stop evlog
                  else if (!strcmp(op, "EVOF"))
                    {
                       eina_evlog_stop();
                    }
                  // enable evlog
                  else if (!strcmp(op, "CPON"))
                    {
                       if (size >= 4) memcpy(&poll_time, data, 4);
                       poll_on = EINA_TRUE;
                       poll_cpu = EINA_TRUE;
                    }
                  // stop evlog
                  else if (!strcmp(op, "CPOF"))
                    {
                       poll_on = EINA_FALSE;
                       poll_cpu = EINA_FALSE;
                    }
                  // fetch the evlog
                  else if (!strcmp(op, "EVLG"))
                    {
                       Eina_Evlog_Buf *evlog = eina_evlog_steal();
                       if ((evlog) && (evlog->buf))
                         {
                            char          tmp[12];
                            unsigned int *tmpsize  = (unsigned int *)(tmp + 0);
                            char         *op2 = "EVLG";
                            unsigned int *overflow = (unsigned int *)(tmp + 8);

                            *tmpsize = (sizeof(tmp) - 4) + evlog->top;
                            memcpy(tmp + 4, op2, 4);
                            *overflow = evlog->overflow;
                            write(_eina_debug_monitor_service_fd,
                                  tmp, sizeof(tmp));
                            write(_eina_debug_monitor_service_fd,
                                  evlog->buf, evlog->top);
                         }
                    }
                  // something we don't understand
                  else fprintf(stderr,
                               "EINA DEBUG ERROR: "
                               "Uunknown command %s\n", op);
                  free(data);
               }
             // major failure on debug daemon control fd - get out of here
             else goto fail;
          }

        if (poll_on)
          {
             if (poll_trace)
               {
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
                    _eina_debug_collect_bt(_eina_debug_thread_active[i]);
                  // we're done probing. now collec all the "i'm done" msgs on the
                  // semaphore for every thread + mainloop
                  for (i = 0; i < (_eina_debug_thread_active_num + 1); i++)
                    eina_semaphore_lock(&_eina_debug_monitor_return_sem);
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
                       t = get_time();
                       fprintf(stderr, "%1.5f bt's per sec\n", (double)bts / (t - t0));
                       t0 = t;
                       bts = 0;
                    }
               }
             if (poll_cpu)
               {
                  // XXX: opendir /proc/sefl/task
//                  eina_evlog("*cpustat", NULL, 0.0, cpustat);
               }
          }
     }
fail:
   // we failed - get out of here and disconnect to debugd
   close(_eina_debug_monitor_service_fd);
   _eina_debug_monitor_service_fd = -1;
   return NULL;
}

// start up the debug monitor if we haven't already
void
_eina_debug_monitor_thread_start(void)
{
   int err;

   // if it's already running - we're good.
   if (_monitor_thread_runs) return;
   // create debug monitor thread
   err = pthread_create(&_monitor_thread, NULL, _eina_debug_monitor, NULL);
   if (err != 0)
     {
        fprintf(stderr, "EINA DEBUG ERROR: Can't create debug thread!\n");
        abort();
     }
   else _monitor_thread_runs = EINA_TRUE;
}

void
_eina_debug_monitor_signal_init(void)
{
   struct sigaction sa;

   // set up signal handler for our profiling signal - eevery thread should
   // obey this (this is the case on linux - other OSs may vary)
   sa.sa_sigaction = _eina_debug_signal;
   sa.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&sa.sa_mask);
   if (sigaction(SIG, &sa, NULL) != 0)
     fprintf(stderr, "EINA DEBUG ERROR: Can't set up sig %i handler!\n", SIG);
}

static const char *
_socket_home_get()
{
   // get possible debug daemon socket directory base
   const char *dir = getenv("XDG_RUNTIME_DIR");
   if (!dir) dir = eina_environment_home_get();
   if (!dir) dir = eina_environment_tmp_get();
   return dir;
}

// connect to efl_debugd
void
_eina_debug_monitor_service_connect(void)
{
   char buf[4096];
   int fd, socket_unix_len, curstate = 0;
   struct sockaddr_un socket_unix;

   // try this socket file - it will likely be:
   //   ~/.ecore/efl_debug/0
   // or maybe
   //   /var/run/UID/.ecore/efl_debug/0
   // either way a 4k buffer should be ebough ( if it's not we're on an
   // insane system)
   snprintf(buf, sizeof(buf), "%s/%s", _socket_home_get(), DEBUG_SERVER);
   // create the socket
   fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (fd < 0) goto err;
   // set the socket to close when we exec things so they don't inherit it
   if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) goto err;
   // set up some socket options on addr re-use
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate,
                  sizeof(curstate)) < 0)
     goto err;
   // sa that it's a unix socket and where the path is
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
#define LENGTH_OF_SOCKADDR_UN(s) \
   (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   // actually conenct to efl_debugd service
   if (connect(fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     goto err;
   // we succeeded - store fd
   _eina_debug_monitor_service_fd = fd;
   return;
err:
   // some kind of connection failure here, so close a valid socket and
   // get out of here
   if (fd >= 0) close(fd);
}
#endif
