#include "eina_debug.h"

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

static inline int
_eina_debug_unwind_bt(void **bt, int max)
{
   unw_cursor_t cursor;
   unw_context_t uc;
   unw_word_t p;
   int total;

   unw_getcontext(&uc);
   unw_init_local(&cursor, &uc);
   for (total = 0; (unw_step(&cursor) > 0) && (total < max); total++)
     {
        unw_get_reg(&cursor, UNW_REG_IP, &p);
        bt[total] = (void *)p;
     }
   return total;
}

static void
_eina_debug_signal(int sig EINA_UNUSED,
                   siginfo_t *si EINA_UNUSED,
                   void *foo EINA_UNUSED)
{
   int i, slot = 0;
   pthread_t self = pthread_self();
   clockid_t cid;
   // XXX: use pthread_getcpuclockid() to get cpu time used since last poll
   // 
   // clockid_t cid;
   // struct timespec ts ts;
   // pthread_getcpuclockid(pthread_self(), &cid);
   // clock_gettime(cid, &ts);
   // printf("%4ld.%03ld\n", ts.tv_sec, ts.tv_nsec / 1000000);
   //
   // also get current cpu with:
   // getcpu()
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
        fprintf(stderr, "EINA DEBUG ERROR: can't find thread slot!\n");
        eina_semaphore_release(&_eina_debug_monitor_return_sem, 1);
        return;
     }
found:
//   printf("dump into slot %i for %p\n", slot, (void *)self);
   _bt_cpu[slot] = sched_getcpu();
   pthread_getcpuclockid(self, &cid);
   clock_gettime(cid, &(_bt_ts[slot]));
//   _bt_buf_len[slot] = backtrace(_bt_buf[slot], EINA_MAX_BT);
   _bt_buf_len[slot] = _eina_debug_unwind_bt(_bt_buf[slot], EINA_MAX_BT);
   eina_semaphore_release(&_eina_debug_monitor_return_sem, 1);
}

#define SIG SIGPROF
//#define SIG ((SIGRTMIN + SIGRTMAX) / 2)

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
_eina_debug_monitor(void *data EINA_UNUSED)
{
   int bts = 0, ret, max_fd;
   double t0, t;
   fd_set rfds, wfds, exfds;
   struct timeval tv = { 0 };
   unsigned int poll_time = 1000;
   Eina_Bool poll_on = EINA_FALSE;

   t0 = get_time();
   for (;;)
     {
        int i;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&exfds);
        FD_SET(_eina_debug_monitor_service_fd, &rfds);
        max_fd = _eina_debug_monitor_service_fd;
        if (poll_on)
          {
             if ((tv.tv_sec == 0) && (tv.tv_usec == 0))
               {
                  tv.tv_sec = 0;
                  tv.tv_usec = poll_time;
               }
             ret = select(max_fd + 1, &rfds, &wfds, &exfds, &tv);
          }
        else ret = select(max_fd + 1, &rfds, &wfds, &exfds, NULL);
        if ((ret == 1) && (FD_ISSET(_eina_debug_monitor_service_fd, &rfds)))
          {
             unsigned int size;
             int rret;

             // XXX: handle protocol
             rret = read(_eina_debug_monitor_service_fd, &size, 4);
             if ((rret == 4) && (size > 0) && (size < 63356))
               {
                  char *buf = alloca(size);

                  rret = read(_eina_debug_monitor_service_fd, buf, size);
                  if ((rret == (int)size) && (size >= 4))
                    {
                       if (!strncmp(buf, "PLON", 4))
                         {
                            if (size >= 8) memcpy(&poll_time, buf + 4, 4);
                            poll_on = EINA_TRUE;
                         }
                       else if (!strncmp(buf, "PLOF", 4))
                         {
                            poll_time = 1000;
                            poll_on = EINA_FALSE;
                         }
                       else
                         fprintf(stderr, "EINA DEBUG ERROR: Uunknown command\n");
                    }
                  else
                    {
                       if (rret <= 0)
                         {
                            fprintf(stderr, "EINA DEBUG ERROR: Lost debug daemon!\n");
                            goto fail;
                         }
                       else
                         {
                         }
                    }
               }
             else
               {
                  if (rret <= 0)
                    {
                       fprintf(stderr, "EINA_DEBUG ERROR: Lost debug daemon!\n");
                       goto fail;
                    }
                  else
                    {
                       fprintf(stderr, "EINA DEBUG ERROR: Invalid message size %i\n", size);
                       goto fail;
                    }
               }
          }

        if (poll_on)
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
//             for (i = 0; i < (_eina_debug_thread_active_num + 1); i++)
//               {
//                  _eina_debug_dump_fhandle_bt(stderr, _bt_buf[i], _bt_buf_len[i]);
//               }
err:
             eina_spinlock_release(&_eina_debug_thread_lock);
             bts++;
             if (bts >= 10000)
               {
                  t = get_time();
                  fprintf(stderr, "%1.5f bt's per sec\n", (double)bts / (t - t0));
                  t0 = t;
                  bts = 0;
               }
          }
     }
fail:
   close(_eina_debug_monitor_service_fd);
   _eina_debug_monitor_service_fd = -1;
   return NULL;
}

// start up the debug monitor if we haven't already
void
_eina_debug_monitor_thread_start(void)
{
   int err;

   if (_monitor_thread_runs) return;
   // XXX: set up socket conn to debug daemon and then have thread deal with
   // it from there on
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

   sa.sa_sigaction = _eina_debug_signal;
   sa.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&sa.sa_mask);
   if (sigaction(SIG, &sa, NULL) != 0)
     fprintf(stderr, "EINA DEBUG ERROR: Can't set up sig %i handler!\n", SIG);
}

static const char *
_socket_home_get()
{
   const char *dir = getenv("XDG_RUNTIME_DIR");
   if (!dir) dir = getenv("HOME");
   if (!dir) dir = getenv("TMPDIR");
   if (!dir) dir = "/tmp";
   return dir;
}

void
_eina_debug_monitor_service_connect(void)
{
   char buf[4096];
   int fd, socket_unix_len, curstate = 0;
   struct sockaddr_un socket_unix;

   snprintf(buf, sizeof(buf), "%s/%s", _socket_home_get(), DEBUG_SERVER);
   fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (fd < 0) goto err;
   if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) goto err;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate,
                  sizeof(curstate)) < 0)
     goto err;
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
#define LENGTH_OF_SOCKADDR_UN(s) \
   (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (connect(fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     goto err;
   _eina_debug_monitor_service_fd = fd;
   return;
err:
   close(fd);
   return;
}
#endif
