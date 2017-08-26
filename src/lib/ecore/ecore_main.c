#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EINA_SLSTR_INTERNAL

#ifdef _WIN32
# ifndef USER_TIMER_MINIMUM
#  define USER_TIMER_MINIMUM 0x0a
# endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#ifdef HAVE_SYSTEMD
# include <systemd/sd-daemon.h>
#endif

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h> /* for Solaris */
#endif

#ifdef _MSC_VER
# include <float.h>
#endif

#ifdef HAVE_ISFINITE
# define ECORE_FINITE(t)  isfinite(t)
#else
# ifdef _MSC_VER
#  define ECORE_FINITE(t) _finite(t)
# else
#  define ECORE_FINITE(t) finite(t)
# endif
#endif

//#define FIX_HZ 1

#ifdef FIX_HZ
# ifndef _MSC_VER
#  include <sys/param.h>
# endif
# ifndef HZ
#  define HZ 100
# endif
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#if defined(HAVE_SYS_EPOLL_H) && !defined(HAVE_LIBUV)
# define HAVE_EPOLL   1
# include <sys/epoll.h>
#else

# define HAVE_EPOLL   0
# define EPOLLIN      1
# define EPOLLPRI     2
# define EPOLLOUT     4
# define EPOLLERR     8

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

typedef union epoll_data {
   void    *ptr;
   int      fd;
   uint32_t u32;
   uint64_t u64;
} epoll_data_t;

struct epoll_event
{
   uint32_t     events;
   epoll_data_t data;
};

static inline int
epoll_create(int size EINA_UNUSED)
{
   return -1;
}

static inline int
epoll_wait(int                 epfd EINA_UNUSED,
           struct epoll_event *events EINA_UNUSED,
           int                 maxevents EINA_UNUSED,
           int                 timeout EINA_UNUSED)
{
   return -1;
}

static inline int
epoll_ctl(int                 epfd EINA_UNUSED,
          int                 op EINA_UNUSED,
          int                 fd EINA_UNUSED,
          struct epoll_event *event EINA_UNUSED)
{
   return -1;
}

#endif

#ifdef HAVE_SYS_TIMERFD_H
# include <sys/timerfd.h>
#else
/* fallback code if we don't have real timerfd - reduces number of ifdefs  */
# ifndef CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC 0 /* bogus value */
# endif
# ifndef TFD_NONBLOCK
#  define TFD_NONBLOCK    0 /* bogus value */
# endif
#endif /* HAVE_SYS_TIMERFD_H */

#ifndef TFD_TIMER_ABSTIME
# define TFD_TIMER_ABSTIME (1 << 0)
#endif
#ifndef TFD_TIMER_CANCELON_SET
# define TFD_TIMER_CANCELON_SET (1 << 1)
#endif

#ifndef HAVE_TIMERFD_CREATE
static inline int
timerfd_create(int clockid EINA_UNUSED,
               int flags EINA_UNUSED)
{
   return -1;
}

static inline int
timerfd_settime(int                      fd EINA_UNUSED,
                int                      flags EINA_UNUSED,
                const struct itimerspec *new_value EINA_UNUSED,
                struct itimerspec       *old_value EINA_UNUSED)
{
   return -1;
}

#endif /* HAVE_TIMERFD_CREATE */

#ifdef USE_G_MAIN_LOOP
# include <glib.h>
#endif

#ifdef HAVE_LIBUV
#ifdef HAVE_NODE_UV_H
#include <node/uv.h>
#elif defined(HAVE_NODEJS_DEPS_UV_UV_H)
#include <nodejs/deps/uv/uv.h>
#elif defined(HAVE_NODEJS_DEPS_UV_INCLUDE_UV_H)
#include <nodejs/deps/uv/include/uv.h>
#elif defined(HAVE_NODEJS_SRC_UV_H)
#include <nodejs/src/uv.h>
#elif defined(HAVE_UV_H)
#include <uv.h>
#else
#error No uv.h header found?
#endif

#if defined HAVE_DLOPEN && ! defined _WIN32
# include <dlfcn.h>
#endif

static uv_prepare_t _ecore_main_uv_prepare;
static uv_check_t _ecore_main_uv_check;
static uv_timer_t _ecore_main_uv_handle_timers;
static Eina_Bool _ecore_main_uv_idling;

static int (*_dl_uv_loop_alive)(uv_loop_t*) = 0;
static int (*_dl_uv_run)(uv_loop_t*, uv_run_mode mode) = 0;
static int (*_dl_uv_stop)(uv_loop_t*) = 0;
static uv_loop_t* (*_dl_uv_default_loop)() = 0;
static int (*_dl_uv_poll_init_socket)(uv_loop_t* loop, uv_poll_t* handle, uv_os_sock_t fd) = 0;
static int (*_dl_uv_poll_init)(uv_loop_t* loop, uv_poll_t* handle, int fd) = 0;
static int (*_dl_uv_poll_start)(uv_poll_t* handle, int events, uv_poll_cb cb) = 0;
static int (*_dl_uv_poll_stop)(uv_poll_t* handle) = 0;
static int (*_dl_uv_timer_init)(uv_loop_t*, uv_timer_t* handle);
static int (*_dl_uv_timer_start)(uv_timer_t* handle,
                                 uv_timer_cb cb,
                                 uint64_t timeout,
                                 uint64_t repeat);
static int (*_dl_uv_timer_stop)(uv_timer_t* handle);
static int (*_dl_uv_prepare_init)(uv_loop_t*, uv_prepare_t* prepare);
static int (*_dl_uv_prepare_start)(uv_prepare_t* prepare, uv_prepare_cb cb);
static int (*_dl_uv_prepare_stop)(uv_prepare_t* prepare);
static int (*_dl_uv_check_init)(uv_loop_t*, uv_check_t* prepare);
static int (*_dl_uv_check_start)(uv_check_t* prepare, uv_check_cb cb);
static int (*_dl_uv_check_stop)(uv_check_t* prepare);
static int (*_dl_uv_close)(uv_handle_t* handle, uv_close_cb close_cb);
#endif

#define NS_PER_SEC (1000.0 * 1000.0 * 1000.0)

struct _Ecore_Fd_Handler
{
   EINA_INLIST;
                          ECORE_MAGIC;
   Ecore_Fd_Handler      *next_ready;
   int                    fd;
   Ecore_Fd_Handler_Flags flags;
   Ecore_Fd_Cb            func;
   void                  *data;
   Ecore_Fd_Cb            buf_func;
   void                  *buf_data;
   Ecore_Fd_Prep_Cb       prep_func;
   void                  *prep_data;
   int                    references;
   Eina_Bool              read_active : 1;
   Eina_Bool              write_active : 1;
   Eina_Bool              error_active : 1;
   Eina_Bool              delete_me : 1;
   Eina_Bool              file : 1;
#if defined(USE_G_MAIN_LOOP)
   GPollFD                gfd;
#endif
#ifdef HAVE_LIBUV
   uv_poll_t              uv_handle;
#endif
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Fd_Handler);

typedef struct _Efl_Loop_Promise_Simple_Data {
   union {
      Ecore_Timer *timer;
      Ecore_Idler *idler;
   };
   Eina_Promise *promise;
} Efl_Loop_Promise_Simple_Data;
GENERIC_ALLOC_SIZE_DECLARE(Efl_Loop_Promise_Simple_Data);

#ifdef _WIN32
struct _Ecore_Win32_Handler
{
   EINA_INLIST;
                         ECORE_MAGIC;
   HANDLE                h;
   Ecore_Win32_Handle_Cb func;
   void                 *data;
   int                   references;
   Eina_Bool             delete_me : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Win32_Handler);
#endif

#if !defined(USE_G_MAIN_LOOP) && !defined(HAVE_LIBUV)
static int  _ecore_main_select(double timeout);
#endif
static void _ecore_main_prepare_handlers(void);
static void _ecore_main_fd_handlers_cleanup(void);
#ifndef _WIN32
# ifndef USE_G_MAIN_LOOP
static void _ecore_main_fd_handlers_bads_rem(void);
# endif
#endif
static void _ecore_main_fd_handlers_call(void);
static int  _ecore_main_fd_handlers_buf_call(void);
#ifndef USE_G_MAIN_LOOP
static void _ecore_main_loop_iterate_internal(int once_only);
#endif

#ifdef _WIN32
static int _ecore_main_win32_select(int             nfds,
                                    fd_set         *readfds,
                                    fd_set         *writefds,
                                    fd_set         *exceptfds,
                                    struct timeval *timeout);
static void _ecore_main_win32_handlers_cleanup(void);
#endif

int in_main_loop = 0;

static Eina_List *_pending_futures = NULL;
static Eina_List *_pending_promises = NULL;
static unsigned char _ecore_exit_code = 0;
static int do_quit = 0;
static Ecore_Fd_Handler *fd_handlers = NULL;
static Ecore_Fd_Handler *fd_handler_current = NULL;
static Eina_List *fd_handlers_with_prep = NULL;
static Eina_List *file_fd_handlers = NULL;
static Eina_List *fd_handlers_with_buffer = NULL;
static Eina_List *fd_handlers_to_delete = NULL;

/* single linked list of ready fdhs, terminated by loop to self */
static Ecore_Fd_Handler *fd_handlers_to_call;
static Ecore_Fd_Handler *fd_handlers_to_call_current;

#ifdef _WIN32
static Ecore_Win32_Handler *win32_handlers = NULL;
static Ecore_Win32_Handler *win32_handler_current = NULL;
static Eina_List *win32_handlers_to_delete = NULL;
#endif

#ifdef _WIN32
Ecore_Select_Function main_loop_select = _ecore_main_win32_select;
#else
# if !defined EXOTIC_NO_SELECT
#   include <sys/select.h>
Ecore_Select_Function main_loop_select = select;
# else
Ecore_Select_Function main_loop_select = NULL;
# endif
#endif

#ifndef USE_G_MAIN_LOOP
static double t1 = 0.0;
static double t2 = 0.0;
#endif

#ifdef HAVE_EPOLL
static int epoll_fd = -1;
static pid_t epoll_pid;
#endif
static int timer_fd = -1;

#ifdef USE_G_MAIN_LOOP
static GPollFD ecore_epoll_fd;
static GPollFD ecore_timer_fd;
static GSource *ecore_glib_source;
static guint ecore_glib_source_id;
static GMainLoop *ecore_main_loop;
static gboolean ecore_idling;
static gboolean _ecore_glib_idle_enterer_called;
static gboolean ecore_fds_ready;
#endif

#ifdef EFL_EXTRA_SANITY_CHECKS
static inline void
_ecore_fd_valid(void)
{
#ifdef HAVE_EPOLL
   if (epoll_fd >= 0)
     {
        if (fcntl(epoll_fd, F_GETFD) < 0)
          {
             ERR("arghhh you caught me! report a backtrace to edevel!");
#ifdef HAVE_PAUSE
             pause();
#else
             sleep(60);
#endif
          }
     }
#endif
}
#endif

static inline void
_ecore_try_add_to_call_list(Ecore_Fd_Handler *fdh)
{
   /* check if this fdh is already in the list */
   if (fdh->next_ready)
     {
       DBG("next_ready");
       return;
     }
   if (fdh->read_active || fdh->write_active || fdh->error_active)
     {
       DBG("added");
        /*
         * make sure next_ready is non-null by pointing to ourselves
         * use that to indicate this fdh is in the ready list
         * insert at the head of the list to avoid trouble
         */
        fdh->next_ready = fd_handlers_to_call ? fd_handlers_to_call : fdh;
        fd_handlers_to_call = fdh;
     }
}

#ifdef HAVE_EPOLL
static inline int
_ecore_get_epoll_fd(void)
{
   if (epoll_pid && (epoll_pid != getpid()))
     {
        /* forked! */
        _ecore_main_loop_shutdown();
     }
   if ((epoll_pid == 0) && (epoll_fd < 0))
     {
        _ecore_main_loop_init();
     }
   return epoll_fd;
}

static inline int
_ecore_epoll_add(int   efd,
                 int   fd,
                 int   events,
                 void *ptr)
{
   struct epoll_event ev;

   memset(&ev, 0, sizeof (ev));
   ev.events = events;
   ev.data.ptr = ptr;
   DBG("adding poll on %d %08x", fd, events);
   return epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
}

static inline int
_ecore_poll_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ) events |= EPOLLIN;
   if (fdh->flags & ECORE_FD_WRITE) events |= EPOLLOUT;
   if (fdh->flags & ECORE_FD_ERROR) events |= EPOLLERR | EPOLLPRI;
   return events;
}
#endif

#ifdef USE_G_MAIN_LOOP
static inline int
_gfd_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ) events |= G_IO_IN;
   if (fdh->flags & ECORE_FD_WRITE) events |= G_IO_OUT;
   if (fdh->flags & ECORE_FD_ERROR) events |= G_IO_ERR;
   return events;
}
#endif

#ifdef HAVE_LIBUV
static void
_ecore_main_uv_poll_cb(uv_poll_t* handle, int status, int events)
{
   DBG("_ecore_main_uv_poll_cb %p status %d events %d", (void*)handle->data, status, events);
   Ecore_Fd_Handler* fdh = handle->data;

   if(_ecore_main_uv_idling)
     {
       DBG("not IDLE anymore");
       _ecore_main_uv_idling = EINA_FALSE;
       _ecore_idle_exiter_call(_mainloop_singleton);
       _ecore_animator_run_reset();
     }
  
  if (status)
    fdh->error_active = EINA_TRUE;
  if (events & UV_READABLE)
    fdh->read_active = EINA_TRUE;
  if (events & UV_WRITABLE)
    fdh->write_active = EINA_TRUE;

  _ecore_try_add_to_call_list(fdh);

  _ecore_main_fd_handlers_call();
  if (fd_handlers_with_buffer)
    _ecore_main_fd_handlers_buf_call();
  _ecore_signal_received_process();
  _ecore_event_call();
  _ecore_main_fd_handlers_cleanup();
  _efl_loop_timer_expired_timers_call(_ecore_time_loop_time);
}

static int
_ecore_main_uv_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ) events |= UV_READABLE;
   if (fdh->flags & ECORE_FD_WRITE) events |= UV_WRITABLE;
   DBG("events is %d", (int)events);
   return events;
}
#endif

static inline int
_ecore_main_fdh_poll_add(Ecore_Fd_Handler *fdh)
{
   DBG("_ecore_main_fdh_poll_add");
   int r = 0;

#ifdef HAVE_EPOLL
#ifdef HAVE_LIBUV
   if(!_dl_uv_run)
#endif
     {
        if ((!fdh->file) && (epoll_fd >= 0))
         {
           r = _ecore_epoll_add(_ecore_get_epoll_fd(), fdh->fd,
                                _ecore_poll_events_from_fdh(fdh), fdh);
         }
     }
#ifdef HAVE_LIBUV
   else
#endif
#endif
     {
#ifdef HAVE_LIBUV
       if(!fdh->file)
         {
           DBG("_ecore_main_fdh_poll_add libuv socket %p", fdh);
           fdh->uv_handle.data = fdh;
           DBG("_ecore_main_fdh_poll_add2 %p", fdh);
           _dl_uv_poll_init_socket(_dl_uv_default_loop(), &fdh->uv_handle, fdh->fd);
           DBG("_ecore_main_fdh_poll_add3 %p", fdh->uv_handle.data);
           _dl_uv_poll_start(&fdh->uv_handle, _ecore_main_uv_events_from_fdh(fdh)
                             , _ecore_main_uv_poll_cb);
           DBG("_ecore_main_fdh_poll_add libuv DONE");
         }
       else
         {
           DBG("_ecore_main_fdh_poll_add libuv file");
           fdh->uv_handle.data = fdh;
           DBG("_ecore_main_fdh_poll_add2 %p", fdh);
           _dl_uv_poll_init(_dl_uv_default_loop(), &fdh->uv_handle, fdh->fd);
           DBG("_ecore_main_fdh_poll_add3 %p", fdh->uv_handle.data);
           _dl_uv_poll_start(&fdh->uv_handle, _ecore_main_uv_events_from_fdh(fdh)
                             , _ecore_main_uv_poll_cb);
           DBG("_ecore_main_fdh_poll_add libuv DONE");
         }
#elif defined(USE_G_MAIN_LOOP)
        fdh->gfd.fd = fdh->fd;
        fdh->gfd.events = _gfd_events_from_fdh(fdh);
        fdh->gfd.revents = 0;
        DBG("adding gpoll on %d %08x", fdh->fd, fdh->gfd.events);
        g_source_add_poll(ecore_glib_source, &fdh->gfd);
#endif
     }
   return r;
}

static inline void
_ecore_main_fdh_poll_del(Ecore_Fd_Handler *fdh)
{
#ifdef HAVE_EPOLL
#ifdef HAVE_LIBUV
   if(!_dl_uv_run)
#endif
     {
       if ((!fdh->file) && (epoll_fd >= 0))
         {
           struct epoll_event ev;
           int efd = _ecore_get_epoll_fd();

           memset(&ev, 0, sizeof (ev));
           DBG("removing poll on %d", fdh->fd);
           /* could get an EBADF if somebody closed the FD before removing it */
           if ((epoll_ctl(efd, EPOLL_CTL_DEL, fdh->fd, &ev) < 0))
             {
               if (errno == EBADF)
                 {
                   WRN("fd %d was closed, can't remove from epoll - reinit!",
                       fdh->fd);
                   _ecore_main_loop_shutdown();
                   _ecore_main_loop_init();
                 }
               else
                 {
                   ERR("Failed to delete epoll fd %d! (errno=%d)", fdh->fd, errno);
                 }
             }
         }
     }
#ifdef HAVE_LIBUV
   else
#endif
#endif
     {
#ifdef HAVE_LIBUV
       DBG("_ecore_main_fdh_poll_del libuv %p", fdh);
       uv_handle_t* h = (uv_handle_t*)&fdh->uv_handle;
       _dl_uv_close(h, 0);
       DBG("_ecore_main_fdh_poll_del libuv DONE");
#elif USE_G_MAIN_LOOP
        fdh->gfd.fd = fdh->fd;
        fdh->gfd.events = _gfd_events_from_fdh(fdh);
        fdh->gfd.revents = 0;
        DBG("removing gpoll on %d %08x", fdh->fd, fdh->gfd.events);
        g_source_remove_poll(ecore_glib_source, &fdh->gfd);
#endif
     }
}

static inline int
_ecore_main_fdh_poll_modify(Ecore_Fd_Handler *fdh)
{
   DBG("_ecore_main_fdh_poll_modify %p", fdh);
   int r = 0;
#ifdef HAVE_EPOLL
#ifdef HAVE_LIBUV
   if(!_dl_uv_run)
#endif
     {
       if ((!fdh->file) && (epoll_fd >= 0))
         {
           struct epoll_event ev;
           int efd = _ecore_get_epoll_fd();

           memset(&ev, 0, sizeof (ev));
           ev.events = _ecore_poll_events_from_fdh(fdh);
           ev.data.ptr = fdh;
           DBG("modifing epoll on %d to %08x", fdh->fd, ev.events);
           r = epoll_ctl(efd, EPOLL_CTL_MOD, fdh->fd, &ev);
         }
     }
#ifdef HAVE_LIBUV
   else
#endif
#endif
     {
#ifdef HAVE_LIBUV
       _dl_uv_poll_start(&fdh->uv_handle, _ecore_main_uv_events_from_fdh(fdh)
                         , _ecore_main_uv_poll_cb);
#elif defined(USE_G_MAIN_LOOP)
        fdh->gfd.fd = fdh->fd;
        fdh->gfd.events = _gfd_events_from_fdh(fdh);
        fdh->gfd.revents = 0;
        DBG("modifing gpoll on %d to %08x", fdh->fd, fdh->gfd.events);
#endif
     }
   return r;
}

#ifdef HAVE_EPOLL
static inline int
_ecore_main_fdh_epoll_mark_active(void)
{
   DBG("_ecore_main_fdh_epoll_mark_active");
   struct epoll_event ev[32];
   int i, ret;
   int efd = _ecore_get_epoll_fd();

   memset(&ev, 0, sizeof (ev));
   ret = epoll_wait(efd, ev, sizeof(ev) / sizeof(struct epoll_event), 0);
   if (ret < 0)
     {
        if (errno == EINTR) return -1;
        ERR("epoll_wait failed on fd: %d %s", efd, strerror(errno));
        return -1;
     }

   for (i = 0; i < ret; i++)
     {
        Ecore_Fd_Handler *fdh;

        fdh = ev[i].data.ptr;
        if (!ECORE_MAGIC_CHECK(fdh, ECORE_MAGIC_FD_HANDLER))
          {
             ECORE_MAGIC_FAIL(fdh, ECORE_MAGIC_FD_HANDLER,
                              "_ecore_main_fdh_epoll_mark_active");
             continue;
          }
        if (fdh->delete_me)
          {
             ERR("deleted fd in epoll");
             continue;
          }

        if (ev[i].events & EPOLLIN)
          fdh->read_active = EINA_TRUE;
        if (ev[i].events & EPOLLOUT)
          fdh->write_active = EINA_TRUE;
        if (ev[i].events & EPOLLERR)
          fdh->error_active = EINA_TRUE;

        _ecore_try_add_to_call_list(fdh);
     }

   return ret;
}
#endif

#ifdef USE_G_MAIN_LOOP

static inline int
_ecore_main_fdh_glib_mark_active(void)
{
   Ecore_Fd_Handler *fdh;
   int ret = 0;

   /* call the prepare callback for all handlers */
   EINA_INLIST_FOREACH(fd_handlers, fdh)
     {
        if (fdh->delete_me)
          continue;

        if (fdh->gfd.revents & G_IO_IN)
          fdh->read_active = EINA_TRUE;
        if (fdh->gfd.revents & G_IO_OUT)
          fdh->write_active = EINA_TRUE;
        if (fdh->gfd.revents & G_IO_ERR)
          fdh->error_active = EINA_TRUE;

        _ecore_try_add_to_call_list(fdh);

        if (fdh->gfd.revents & (G_IO_IN | G_IO_OUT | G_IO_ERR)) ret++;
     }

   return ret;
}

/* like we are about to enter main_loop_select in  _ecore_main_select */
static gboolean
_ecore_main_gsource_prepare(GSource *source EINA_UNUSED,
                            gint    *next_time)
{
   gboolean ready = FALSE;

   in_main_loop++;

   if (!ecore_idling && !_ecore_glib_idle_enterer_called)
     {
        _ecore_time_loop_time = ecore_time_get();
        _efl_loop_timer_expired_timers_call(_ecore_time_loop_time);

        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
        _ecore_glib_idle_enterer_called = FALSE;

        if (fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call();
     }

   _ecore_signal_received_process();

   /* don't check fds if somebody quit */
   if (g_main_loop_is_running(ecore_main_loop))
     {
        /* only set idling state in dispatch */
         if (ecore_idling && !_ecore_idler_exist(_mainloop_singleton) && !_ecore_event_exist())
           {
              if (_efl_loop_timers_exists())
                {
                   int r = -1;
                   double t = _efl_loop_timer_next_get();

                   if ((timer_fd >= 0) && (t > 0.0))
                     {
                        struct itimerspec ts;

                        ts.it_interval.tv_sec = 0;
                        ts.it_interval.tv_nsec = 0;
                        ts.it_value.tv_sec = t;
                        ts.it_value.tv_nsec = fmod(t * NS_PER_SEC, NS_PER_SEC);

     /* timerfd cannot sleep for 0 time */
                        if (ts.it_value.tv_sec || ts.it_value.tv_nsec)
                          {
                             r = timerfd_settime(timer_fd, 0, &ts, NULL);
                             if (r < 0)
                               {
                                  ERR("timer set returned %d (errno=%d)", r, errno);
                                  close(timer_fd);
                                  timer_fd = -1;
                               }
                             else
                               INF("sleeping for %ld s %06ldus",
                                   ts.it_value.tv_sec,
                                   ts.it_value.tv_nsec / 1000);
                          }
                     }
                   if (r == -1)
                     {
                        *next_time = ceil(t * 1000.0);
                        if (t == 0.0)
                          ready = TRUE;
                     }
                }
              else
                *next_time = -1;
           }
         else
           {
              *next_time = 0;
              if (_ecore_event_exist())
                ready = TRUE;
           }

         if (fd_handlers_with_prep)
           _ecore_main_prepare_handlers();
     }
   else
     ready = TRUE;

   in_main_loop--;
   DBG("leave, timeout = %d", *next_time);

   /* ready if we're not running (about to quit) */
   return ready;
}

static gboolean
_ecore_main_gsource_check(GSource *source EINA_UNUSED)
{
   gboolean ret = FALSE;

   in_main_loop++;

   /* check if old timers expired */
   if (ecore_idling && !_ecore_idler_exist(_mainloop_singleton) && !_ecore_event_exist())
     {
        if (timer_fd >= 0)
          {
             uint64_t count = 0;
             int r = read(timer_fd, &count, sizeof count);
             if ((r == -1) && (errno == EAGAIN))
               ;
             else if (r == sizeof count)
               ret = TRUE;
             else
               {
                  /* unexpected things happened... fail back to old way */
                   ERR("timer read returned %d (errno=%d)", r, errno);
                   close(timer_fd);
                   timer_fd = -1;
               }
          }
     }
   else
     ret = TRUE;

   /* check if fds are ready */
#ifdef HAVE_EPOLL
   if (epoll_fd >= 0)
     ecore_fds_ready = (_ecore_main_fdh_epoll_mark_active() > 0);
   else
#endif
     ecore_fds_ready = (_ecore_main_fdh_glib_mark_active() > 0);
   _ecore_main_fd_handlers_cleanup();
   if (ecore_fds_ready)
     ret = TRUE;

   /* check timers after updating loop time */
   if (!ret && _efl_loop_timers_exists())
     ret = (0.0 == _efl_loop_timer_next_get());

   in_main_loop--;

   return ret;
}

/* like we just came out of main_loop_select in  _ecore_main_select */
static gboolean
_ecore_main_gsource_dispatch(GSource    *source EINA_UNUSED,
                             GSourceFunc callback EINA_UNUSED,
                             gpointer    user_data EINA_UNUSED)
{
   gboolean events_ready, timers_ready, idlers_ready;
   double next_time;

   _ecore_time_loop_time = ecore_time_get();
   _efl_loop_timer_enable_new();
   next_time = _efl_loop_timer_next_get();

   events_ready = _ecore_event_exist();
   timers_ready = _efl_loop_timers_exists() && (0.0 == next_time);
   idlers_ready = _ecore_idler_exist(_mainloop_singleton);

   in_main_loop++;
   DBG("enter idling=%d fds=%d events=%d timers=%d (next=%.2f) idlers=%d",
       ecore_idling, ecore_fds_ready, events_ready,
       timers_ready, next_time, idlers_ready);

   if (ecore_idling && events_ready)
     {
        _ecore_animator_run_reset();
        _ecore_idle_exiter_call(_mainloop_singleton);
        ecore_idling = 0;
     }
   else if (!ecore_idling && !events_ready)
     {
        ecore_idling = 1;
     }

   if (ecore_idling)
     {
        _ecore_idler_all_call(_mainloop_singleton);

        events_ready = _ecore_event_exist();

        if (ecore_fds_ready || events_ready || timers_ready)
          {
             _ecore_animator_run_reset();
             _ecore_idle_exiter_call(_mainloop_singleton);
             ecore_idling = 0;
          }
     }

   /* process events */
   if (!ecore_idling)
     {
        _ecore_main_fd_handlers_call();
        if (fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call();
        _ecore_signal_received_process();
        _ecore_event_call();
        _ecore_main_fd_handlers_cleanup();

        _efl_loop_timer_expired_timers_call(_ecore_time_loop_time);

        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
        _ecore_glib_idle_enterer_called = TRUE;

        if (fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call();
     }

   in_main_loop--;

   return TRUE; /* what should be returned here? */
}

static void
_ecore_main_gsource_finalize(GSource *source EINA_UNUSED)
{
}

static GSourceFuncs ecore_gsource_funcs =
{
   .prepare = _ecore_main_gsource_prepare,
   .check = _ecore_main_gsource_check,
   .dispatch = _ecore_main_gsource_dispatch,
   .finalize = _ecore_main_gsource_finalize,
};

#endif

#ifdef HAVE_SYS_TIMERFD_H
static int realtime_fd = -1;

static void detect_time_changes_start(void);
static Eina_Bool
_realtime_update(void *data EINA_UNUSED, Ecore_Fd_Handler *fdh EINA_UNUSED)
{
   char buf[64];

   if (read(realtime_fd, buf, sizeof(buf)) >= 0) return EINA_TRUE;

   DBG("system clock changed");
   ecore_event_add(ECORE_EVENT_SYSTEM_TIMEDATE_CHANGED, NULL, NULL, NULL);

   close(realtime_fd);
   realtime_fd = -1;
   detect_time_changes_start();
   return EINA_FALSE;
}
#endif

static void
detect_time_changes_start(void)
{
#ifdef HAVE_SYS_TIMERFD_H
   struct itimerspec its;

   if (realtime_fd >= 0) return;

   realtime_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
   if (realtime_fd < 0) return;

   memset(&its, 0, sizeof(its));
   its.it_value.tv_sec = 0x7ffffff0; // end of time - 0xf
   if (timerfd_settime(realtime_fd,
                       TFD_TIMER_ABSTIME | TFD_TIMER_CANCELON_SET,
                       &its, NULL) < 0)
     {
        WRN("Couldn't arm timerfd to detect clock changes: %s",
            strerror(errno));
        close(realtime_fd);
        realtime_fd = -1;
        return;
     }

   ecore_main_fd_handler_add(realtime_fd, ECORE_FD_READ,
                             _realtime_update, NULL, NULL, NULL);
#endif
}

static void
detect_time_changes_stop(void)
{
#ifdef HAVE_SYS_TIMERFD_H
   if (realtime_fd > 0)
     {
        close(realtime_fd);
        realtime_fd = -1;
     }
#endif
}


#ifdef HAVE_LIBUV
static inline
void
_ecore_main_loop_uv_check(uv_check_t* handle EINA_UNUSED);
static void _ecore_main_loop_uv_prepare(uv_prepare_t* handle);

static
void _ecore_main_loop_timer_run(uv_timer_t* timer EINA_UNUSED)
{
  if(_ecore_main_uv_idling)
    {
      _ecore_main_uv_idling = EINA_FALSE;
      _ecore_idle_exiter_call(_mainloop_singleton);
      _ecore_animator_run_reset();
    }
  _ecore_time_loop_time = ecore_time_get();
  _ecore_main_loop_uv_check(NULL);
  
  _ecore_main_loop_uv_prepare(NULL);
}
static void _ecore_main_loop_uv_prepare(uv_prepare_t* handle);

static inline
void
_ecore_main_loop_uv_check(uv_check_t* handle EINA_UNUSED)
{
   DBG("_ecore_main_loop_uv_check idling? %d", (int)_ecore_main_uv_idling);
   in_main_loop++;

   if(do_quit)
     goto quit;

   do
     {
       _ecore_main_fd_handlers_call();
       if (fd_handlers_with_buffer)
         _ecore_main_fd_handlers_buf_call();
       _ecore_signal_received_process();
       _ecore_event_call();
       _ecore_main_fd_handlers_cleanup();
       _efl_loop_timer_expired_timers_call(_ecore_time_loop_time);
     }
   while(fd_handlers_to_call);
quit:
   in_main_loop--;
}
#endif

void
_ecore_main_loop_init(void)
{
   // Please note that this function is being also called in case of a bad fd to reset the main loop.

   DBG("_ecore_main_loop_init");
#ifdef HAVE_EPOLL
   epoll_fd = epoll_create(1);
   if (epoll_fd < 0)
     WRN("Failed to create epoll fd!");
   else
     {
        eina_file_close_on_exec(epoll_fd, EINA_TRUE);

        epoll_pid = getpid();

        /* add polls on all our file descriptors */
        Ecore_Fd_Handler *fdh;
        EINA_INLIST_FOREACH(fd_handlers, fdh)
          {
             if (fdh->delete_me) continue;
             _ecore_epoll_add(epoll_fd, fdh->fd,
                              _ecore_poll_events_from_fdh(fdh), fdh);
             _ecore_main_fdh_poll_add(fdh);
          }
     }
#endif

#ifdef HAVE_LIBUV
   {
     DBG("loading lib uv");
#ifdef HAVE_NODEJS
     void* lib = dlopen(NULL, RTLD_LAZY);
#else
     void* lib = dlopen("libuv.so", RTLD_GLOBAL | RTLD_LAZY);
#endif

     if(lib && dlsym(lib, "uv_run"))
       {
         DBG("loaded lib uv");
         _dl_uv_run = dlsym(lib, "uv_run");
         assert(!!_dl_uv_run);
         _dl_uv_stop = dlsym(lib, "uv_stop");
         assert(!!_dl_uv_stop);
         _dl_uv_default_loop = dlsym(lib, "uv_default_loop");
         assert(!!_dl_uv_default_loop);
         _dl_uv_poll_init_socket = dlsym(lib, "uv_poll_init_socket");
         assert(!!_dl_uv_poll_init_socket);
         _dl_uv_poll_init = dlsym(lib, "uv_poll_init");
         assert(!!_dl_uv_poll_init);
         _dl_uv_poll_start = dlsym(lib, "uv_poll_start");
         assert(!!_dl_uv_poll_start);
         _dl_uv_poll_stop = dlsym(lib, "uv_poll_stop");
         assert(!!_dl_uv_poll_stop);
         _dl_uv_timer_init = dlsym(lib, "uv_timer_init");
         assert(!!_dl_uv_timer_init);
         _dl_uv_timer_start = dlsym(lib, "uv_timer_start");
         assert(!!_dl_uv_timer_start);
         _dl_uv_timer_stop = dlsym(lib, "uv_timer_stop");
         assert(!!_dl_uv_timer_stop);
         _dl_uv_prepare_init = dlsym(lib, "uv_prepare_init");
         assert(!!_dl_uv_prepare_init);
         _dl_uv_prepare_start = dlsym(lib, "uv_prepare_start");
         assert(!!_dl_uv_prepare_start);
         _dl_uv_prepare_stop = dlsym(lib, "uv_prepare_stop");
         assert(!!_dl_uv_prepare_stop);
         _dl_uv_check_init = dlsym(lib, "uv_check_init");
         assert(!!_dl_uv_check_init);
         _dl_uv_check_start = dlsym(lib, "uv_check_start");
         assert(!!_dl_uv_check_start);
         _dl_uv_check_stop = dlsym(lib, "uv_check_stop");
         assert(!!_dl_uv_check_stop);
         _dl_uv_close = dlsym(lib, "uv_close");
         assert(!!_dl_uv_close);
         _dl_uv_loop_alive = dlsym(lib, "uv_loop_alive");
         assert(!!_dl_uv_loop_alive);

  //dlclose(lib);

         DBG("_dl_uv_prepare_init");
         _dl_uv_prepare_init(_dl_uv_default_loop(), &_ecore_main_uv_prepare);
         DBG("_dl_uv_prepare_start");
         _dl_uv_prepare_start(&_ecore_main_uv_prepare, &_ecore_main_loop_uv_prepare);
         DBG("_dl_uv_prepare_started");

         DBG("_dl_uv_check_init");
         _dl_uv_check_init(_dl_uv_default_loop(), &_ecore_main_uv_check);
         DBG("_dl_uv_check_start");
         _dl_uv_check_start(&_ecore_main_uv_check, &_ecore_main_loop_uv_check);
         DBG("_dl_uv_check_started");

         _dl_uv_timer_init(_dl_uv_default_loop(),  &_ecore_main_uv_handle_timers);
       }
     /* else */
     /*   DBG("did not load uv"); */
     DBG("loaded dlsyms uv");
   }
#endif

   /* setup for the g_main_loop only integration */
#ifdef USE_G_MAIN_LOOP
   ecore_glib_source = g_source_new(&ecore_gsource_funcs, sizeof (GSource));
   if (!ecore_glib_source)
     CRI("Failed to create glib source for epoll!");
   else
     {
        g_source_set_priority(ecore_glib_source, G_PRIORITY_HIGH_IDLE + 20);
#ifdef HAVE_EPOLL
        if (epoll_fd >= 0)
          {
             /* epoll multiplexes fds into the g_main_loop */
              ecore_epoll_fd.fd = epoll_fd;
              ecore_epoll_fd.events = G_IO_IN;
              ecore_epoll_fd.revents = 0;
              g_source_add_poll(ecore_glib_source, &ecore_epoll_fd);
          }
#endif
        /* timerfd gives us better than millisecond accuracy in g_main_loop */
        timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if (timer_fd < 0)
          WRN("failed to create timer fd!");
        else
          {
             eina_file_close_on_exec(timer_fd, EINA_TRUE);
             ecore_timer_fd.fd = timer_fd;
             ecore_timer_fd.events = G_IO_IN;
             ecore_timer_fd.revents = 0;
             g_source_add_poll(ecore_glib_source, &ecore_timer_fd);
          }

        ecore_glib_source_id = g_source_attach(ecore_glib_source, NULL);
        if (ecore_glib_source_id <= 0)
          CRI("Failed to attach glib source to default context");
     }
#endif

   detect_time_changes_start();
}

void
_ecore_main_loop_shutdown(void)
{
   // Please note that _ecore_main_loop_shutdown is called in cycle to restart the main loop in case of a bad fd

#ifdef USE_G_MAIN_LOOP
   if (ecore_glib_source)
     {
        g_source_destroy(ecore_glib_source);
        ecore_glib_source = NULL;
     }
#endif

   detect_time_changes_stop();

#ifdef HAVE_EPOLL
   if (epoll_fd >= 0)
     {
        close(epoll_fd);
        epoll_fd = -1;
     }
   epoll_pid = 0;
#endif

   if (timer_fd >= 0)
     {
        close(timer_fd);
        timer_fd = -1;
     }

#ifdef HAVE_LIBUV
   if(_dl_uv_run)
     {
       DBG("_ecore_main_loop_shutdown");
       _dl_uv_timer_stop(&_ecore_main_uv_handle_timers);
       _dl_uv_close((uv_handle_t*)&_ecore_main_uv_handle_timers, 0);
     }
#endif
}

void *
_ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler)
{
   DBG("_ecore_main_fd_handler_del %p", fd_handler);
   if (fd_handler->delete_me)
     {
        ERR("fdh %p deleted twice", fd_handler);
        return NULL;
     }

   fd_handler->delete_me = EINA_TRUE;
   _ecore_main_fdh_poll_del(fd_handler);
   fd_handlers_to_delete = eina_list_append(fd_handlers_to_delete, fd_handler);
   if (fd_handler->prep_func && fd_handlers_with_prep)
     fd_handlers_with_prep = eina_list_remove(fd_handlers_with_prep, fd_handler);
   if (fd_handler->buf_func && fd_handlers_with_buffer)
     fd_handlers_with_buffer = eina_list_remove(fd_handlers_with_buffer, fd_handler);
   return fd_handler->data;
}

EAPI void
ecore_main_loop_iterate(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
#ifdef HAVE_LIBUV
   if(!_dl_uv_run) {
#endif
#ifndef USE_G_MAIN_LOOP
   _ecore_time_loop_time = ecore_time_get();
   _ecore_main_loop_iterate_internal(1);
#else
   g_main_context_iteration(NULL, 0);
#endif
#ifdef HAVE_LIBUV
   }
   else
     _dl_uv_run(_dl_uv_default_loop(), UV_RUN_ONCE | UV_RUN_NOWAIT);
#endif
}

EAPI int
ecore_main_loop_iterate_may_block(int may_block)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
#ifdef HAVE_LIBUV
   if(!_dl_uv_run) {
#endif
#ifndef USE_G_MAIN_LOOP
   _ecore_time_loop_time = ecore_time_get();
in_main_loop++;
   _ecore_main_loop_iterate_internal(!may_block);
in_main_loop--;
   return _ecore_event_exist();
#else
   return g_main_context_iteration(NULL, may_block);
#endif
#ifdef HAVE_LIBUV
   }
   else
     _dl_uv_run(_dl_uv_default_loop(), may_block ? UV_RUN_ONCE | UV_RUN_NOWAIT : UV_RUN_ONCE);
#endif
   return 0;
}

EAPI void
ecore_main_loop_begin(void)
{
   DBG("ecore_main_loop_begin");
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (in_main_loop > 0)
     {
        ERR("Running ecore_main_loop_begin() inside an existing main loop");
        return;
     }
   eina_evlog("+mainloop", NULL, 0.0, NULL);
#ifdef HAVE_SYSTEMD
   sd_notify(0, "READY=1");
#endif
#ifdef HAVE_LIBUV
   if(!_dl_uv_run) {
#endif
#ifndef USE_G_MAIN_LOOP
   in_main_loop++;
   _ecore_time_loop_time = ecore_time_get();
   while (do_quit == 0) _ecore_main_loop_iterate_internal(0);
   do_quit = 0;
   in_main_loop--;
#else
   if (!do_quit)
     {
        if (!ecore_main_loop)
          ecore_main_loop = g_main_loop_new(NULL, FALSE);
        g_main_loop_run(ecore_main_loop);
     }
   do_quit = 0;
#endif
#ifdef HAVE_LIBUV
   }
   else
     {
       DBG("uv_run");
       _ecore_time_loop_time = ecore_time_get();
       in_main_loop++;
       while(!do_quit)
         _dl_uv_run(_dl_uv_default_loop(), UV_RUN_DEFAULT);
       in_main_loop--;
       do_quit = 0;
       DBG("quit");
     }
#endif
   eina_evlog("-mainloop", NULL, 0.0, NULL);
}

EAPI void
ecore_main_loop_quit(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   do_quit = 1;
#ifdef USE_G_MAIN_LOOP
   if (ecore_main_loop)
     g_main_loop_quit(ecore_main_loop);
#elif defined(HAVE_LIBUV)
   if (_dl_uv_run)
     _dl_uv_stop(_dl_uv_default_loop());
#endif
}

EAPI int 
ecore_main_loop_nested_get(void)
{
   return in_main_loop;
}

EAPI Eina_Bool
ecore_main_loop_animator_ticked_get(void)
{
   DBG("ecore_main_loop_animator_ticked_get");
   return _ecore_animator_run_get();
}

EAPI void
ecore_main_loop_select_func_set(Ecore_Select_Function func)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   main_loop_select = func;
}

EAPI Ecore_Select_Function
ecore_main_loop_select_func_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   return main_loop_select;
}

Ecore_Fd_Handler *
_ecore_main_fd_handler_add(int                    fd,
                           Ecore_Fd_Handler_Flags flags,
                           Ecore_Fd_Cb            func,
                           const void            *data,
                           Ecore_Fd_Cb            buf_func,
                           const void            *buf_data,
                           Eina_Bool              is_file)
{
   DBG("_ecore_main_fd_handler_add");
   Ecore_Fd_Handler *fdh = NULL;

   if ((fd < 0) || (!func)) return NULL;

   fdh = ecore_fd_handler_calloc(1);
   if (!fdh) return NULL;
   ECORE_MAGIC_SET(fdh, ECORE_MAGIC_FD_HANDLER);
   fdh->next_ready = NULL;
   fdh->fd = fd;
   fdh->flags = flags;
   fdh->file = is_file;
   if (_ecore_main_fdh_poll_add(fdh) < 0)
     {
        int err = errno;
        ERR("Failed to add poll on fd %d (errno = %d: %s)!", fd, err, strerror(err));
        ecore_fd_handler_mp_free(fdh);
        return NULL;
     }
   fdh->read_active = EINA_FALSE;
   fdh->write_active = EINA_FALSE;
   fdh->error_active = EINA_FALSE;
   fdh->delete_me = EINA_FALSE;
   fdh->func = func;
   fdh->data = (void *)data;
   fdh->buf_func = buf_func;
   if (buf_func)
     fd_handlers_with_buffer = eina_list_append(fd_handlers_with_buffer, fdh);
   fdh->buf_data = (void *)buf_data;
   if (is_file)
     file_fd_handlers = eina_list_append(file_fd_handlers, fdh);
   fd_handlers = (Ecore_Fd_Handler *)
     eina_inlist_append(EINA_INLIST_GET(fd_handlers),
                        EINA_INLIST_GET(fdh));

   return fdh;
}

EAPI Ecore_Fd_Handler *
ecore_main_fd_handler_add(int                    fd,
                          Ecore_Fd_Handler_Flags flags,
                          Ecore_Fd_Cb            func,
                          const void            *data,
                          Ecore_Fd_Cb            buf_func,
                          const void            *buf_data)
{
   Ecore_Fd_Handler *fdh = NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   fdh = _ecore_main_fd_handler_add(fd, flags, func, data, buf_func, buf_data, EINA_FALSE);
   return fdh;
}

EAPI Ecore_Fd_Handler *
ecore_main_fd_handler_file_add(int                    fd,
                               Ecore_Fd_Handler_Flags flags,
                               Ecore_Fd_Cb            func,
                               const void            *data,
                               Ecore_Fd_Cb            buf_func,
                               const void            *buf_data)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   return _ecore_main_fd_handler_add(fd, flags, func, data, buf_func, buf_data, EINA_TRUE);
}

#ifdef _WIN32
EAPI Ecore_Win32_Handler *
ecore_main_win32_handler_add(void                 *h,
                             Ecore_Win32_Handle_Cb func,
                             const void           *data)
{
   Ecore_Win32_Handler *wh;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!h || !func) return NULL;

   wh = ecore_win32_handler_calloc(1);
   if (!wh) return NULL;
   ECORE_MAGIC_SET(wh, ECORE_MAGIC_WIN32_HANDLER);
   wh->h = (HANDLE)h;
   wh->delete_me = EINA_FALSE;
   wh->func = func;
   wh->data = (void *)data;
   win32_handlers = (Ecore_Win32_Handler *)
     eina_inlist_append(EINA_INLIST_GET(win32_handlers),
                        EINA_INLIST_GET(wh));
   return wh;
}

#else
EAPI Ecore_Win32_Handler *
ecore_main_win32_handler_add(void                 *h EINA_UNUSED,
                             Ecore_Win32_Handle_Cb func EINA_UNUSED,
                             const void           *data EINA_UNUSED)
{
   return NULL;
}

#endif

EAPI void *
ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler)
{
   if (!fd_handler) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_del");
        return NULL;
     }
   return _ecore_main_fd_handler_del(fd_handler);
}

#ifdef _WIN32
static void *
_ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler)
{
   if (win32_handler->delete_me)
     {
        ERR("win32 handler %p deleted twice", win32_handler);
        return NULL;
     }

   win32_handler->delete_me = EINA_TRUE;
   win32_handlers_to_delete = eina_list_append(win32_handlers_to_delete, win32_handler);
   return win32_handler->data;
}

EAPI void *
ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler)
{
   void *ret = NULL;

   if (!win32_handler) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!ECORE_MAGIC_CHECK(win32_handler, ECORE_MAGIC_WIN32_HANDLER))
     {
        ECORE_MAGIC_FAIL(win32_handler, ECORE_MAGIC_WIN32_HANDLER,
                         "ecore_main_win32_handler_del");
        return NULL;
     }
   ret = _ecore_main_win32_handler_del(win32_handler);
   return ret;
}

#else
EAPI void *
ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler EINA_UNUSED)
{
   return NULL;
}

#endif

EAPI void
ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler,
                                           Ecore_Fd_Prep_Cb  func,
                                           const void       *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_prepare_callback_set");
        return ;
     }
   fd_handler->prep_func = func;
   fd_handler->prep_data = (void *)data;
   if ((!fd_handlers_with_prep) ||
       (fd_handlers_with_prep && (!eina_list_data_find(fd_handlers_with_prep, fd_handler))))
     /* FIXME: THIS WILL NOT SCALE WITH LOTS OF PREP FUNCTIONS!!! */
     fd_handlers_with_prep = eina_list_append(fd_handlers_with_prep, fd_handler);
}

EAPI int
ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(-1);

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_fd_get");
        return -1;
     }
   return fd_handler->fd;
}

EAPI Eina_Bool
ecore_main_fd_handler_active_get(Ecore_Fd_Handler      *fd_handler,
                                 Ecore_Fd_Handler_Flags flags)
{
   int ret = EINA_FALSE;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(EINA_FALSE);

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_active_get");
        return EINA_FALSE;
     }
   if ((flags & ECORE_FD_READ) && (fd_handler->read_active)) ret = EINA_TRUE;
   if ((flags & ECORE_FD_WRITE) && (fd_handler->write_active)) ret = EINA_TRUE;
   if ((flags & ECORE_FD_ERROR) && (fd_handler->error_active)) ret = EINA_TRUE;
   return ret;
}

EAPI void
ecore_main_fd_handler_active_set(Ecore_Fd_Handler      *fd_handler,
                                 Ecore_Fd_Handler_Flags flags)
{
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN;

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_active_set");
        return ;
     }
   fd_handler->flags = flags;
   ret = _ecore_main_fdh_poll_modify(fd_handler);
   if (ret < 0)
     {
        ERR("Failed to mod epoll fd %d: %s!", fd_handler->fd, strerror(errno));
     }
}

void
_ecore_main_shutdown(void)
{
   Efl_Promise *promise;
   Efl_Future *future;

   if (in_main_loop)
     {
        ERR("\n"
            "*** ECORE WARNING: Calling ecore_shutdown() while still in the main loop.\n"
            "***                 Program may crash or behave strangely now.");
        return;
     }

   EINA_LIST_FREE(_pending_futures, future)
     efl_del(future);

   EINA_LIST_FREE(_pending_promises, promise)
     ecore_loop_promise_fulfill(promise);

   while (fd_handlers)
     {
        Ecore_Fd_Handler *fdh;

        fdh = fd_handlers;
        fd_handlers = (Ecore_Fd_Handler *)eina_inlist_remove(EINA_INLIST_GET(fd_handlers),
                                                             EINA_INLIST_GET(fdh));
        ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
        ecore_fd_handler_mp_free(fdh);
     }
   if (fd_handlers_with_buffer)
     fd_handlers_with_buffer = eina_list_free(fd_handlers_with_buffer);
   if (fd_handlers_with_prep)
     fd_handlers_with_prep = eina_list_free(fd_handlers_with_prep);
   if (fd_handlers_to_delete)
     fd_handlers_to_delete = eina_list_free(fd_handlers_to_delete);
   if (file_fd_handlers)
     file_fd_handlers = eina_list_free(file_fd_handlers);

   fd_handlers_to_call = NULL;
   fd_handlers_to_call_current = NULL;
   fd_handlers_to_delete = NULL;
   fd_handler_current = NULL;

#ifdef _WIN32
   while (win32_handlers)
     {
        Ecore_Win32_Handler *wh;

        wh = win32_handlers;
        win32_handlers = (Ecore_Win32_Handler *)eina_inlist_remove(EINA_INLIST_GET(win32_handlers),
                                                                   EINA_INLIST_GET(wh));
        ECORE_MAGIC_SET(wh, ECORE_MAGIC_NONE);
        ecore_win32_handler_mp_free(wh);
     }
   if (win32_handlers_to_delete)
     win32_handlers_to_delete = eina_list_free(win32_handlers_to_delete);
   win32_handlers_to_delete = NULL;
   win32_handler_current = NULL;
#endif
}

static void
_ecore_main_prepare_handlers(void)
{
   Ecore_Fd_Handler *fdh;
   Eina_List *l, *l2;

   /* call the prepare callback for all handlers with prep functions */
   EINA_LIST_FOREACH_SAFE(fd_handlers_with_prep, l, l2, fdh)
     {
        if (!fdh)
          {
             fd_handlers_with_prep = eina_list_remove_list(l, fd_handlers_with_prep);
             continue;
          }
        if (!fdh->delete_me && fdh->prep_func)
          {
             fdh->references++;
             _ecore_call_prep_cb(fdh->prep_func, fdh->prep_data, fdh);
             fdh->references--;
          }
        else
          fd_handlers_with_prep = eina_list_remove_list(fd_handlers_with_prep, l);
     }
}

#if !defined(USE_G_MAIN_LOOP)
static int
_ecore_main_select(double timeout)
{
   struct timeval tv, *t;
   fd_set rfds, wfds, exfds;
   Ecore_Fd_Handler *fdh;
   Eina_List *l;
   int max_fd, ret, err_no;

   t = NULL;
   if ((!ECORE_FINITE(timeout)) || (EINA_DBL_EQ(timeout, 0.0))) /* finite() tests for NaN, too big, too small, and infinity.  */
     {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        t = &tv;
     }
   else if (timeout > 0.0)
     {
        int sec, usec;

#ifdef FIX_HZ
        timeout += (0.5 / HZ);
        sec = (int)timeout;
        usec = (int)((timeout - (double)sec) * 1000000);
#else
        sec = (int)timeout;
        usec = (int)((timeout - (double)sec) * 1000000);
#endif
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        t = &tv;
     }
   max_fd = 0;
   FD_ZERO(&rfds);
   FD_ZERO(&wfds);
   FD_ZERO(&exfds);

   /* call the prepare callback for all handlers */
   if (fd_handlers_with_prep)
     _ecore_main_prepare_handlers();

#ifdef HAVE_EPOLL
   if (epoll_fd < 0)
     {
#endif
        EINA_INLIST_FOREACH(fd_handlers, fdh)
          {
             if (!fdh->delete_me)
               {
                  if (fdh->flags & ECORE_FD_READ)
                    {
                       FD_SET(fdh->fd, &rfds);
                       if (fdh->fd > max_fd) max_fd = fdh->fd;
                    }
                  if (fdh->flags & ECORE_FD_WRITE)
                    {
                       FD_SET(fdh->fd, &wfds);
                       if (fdh->fd > max_fd) max_fd = fdh->fd;
                    }
                  if (fdh->flags & ECORE_FD_ERROR)
                    {
                       FD_SET(fdh->fd, &exfds);
                       if (fdh->fd > max_fd) max_fd = fdh->fd;
                    }
               }
          }
     }
#ifdef HAVE_EPOLL
   else
     {
        /* polling on the epoll fd will wake when an fd in the epoll set is active */
         max_fd = _ecore_get_epoll_fd();
         FD_SET(max_fd, &rfds);
     }
#endif
   EINA_LIST_FOREACH(file_fd_handlers, l, fdh)
     if (!fdh->delete_me)
       {
          if (fdh->flags & ECORE_FD_READ)
            {
               FD_SET(fdh->fd, &rfds);
               if (fdh->fd > max_fd) max_fd = fdh->fd;
            }
          if (fdh->flags & ECORE_FD_WRITE)
            {
               FD_SET(fdh->fd, &wfds);
               if (fdh->fd > max_fd) max_fd = fdh->fd;
            }
          if (fdh->flags & ECORE_FD_ERROR)
            {
               FD_SET(fdh->fd, &exfds);
               if (fdh->fd > max_fd) max_fd = fdh->fd;
            }
          if (fdh->fd > max_fd) max_fd = fdh->fd;
       }
   if (_ecore_signal_count_get()) return -1;

   eina_evlog("<RUN", NULL, 0.0, NULL);
   eina_evlog("!SLEEP", NULL, 0.0, t ? "timeout" : "forever");
   ret = main_loop_select(max_fd + 1, &rfds, &wfds, &exfds, t);
   err_no = errno;
   eina_evlog("!WAKE", NULL, 0.0, NULL);
   eina_evlog(">RUN", NULL, 0.0, NULL);

   _ecore_time_loop_time = ecore_time_get();
   if (ret < 0)
     {
#ifndef _WIN32
        if (err_no == EINTR) return -1;
        else if (err_no == EBADF)
          _ecore_main_fd_handlers_bads_rem();
#endif
     }
   if (ret > 0)
     {
#ifdef HAVE_EPOLL
        if (epoll_fd >= 0)
          _ecore_main_fdh_epoll_mark_active();
        else
#endif
          {
             EINA_INLIST_FOREACH(fd_handlers, fdh)
               {
                  if (!fdh->delete_me)
                    {
                       if (FD_ISSET(fdh->fd, &rfds))
                         fdh->read_active = EINA_TRUE;
                       if (FD_ISSET(fdh->fd, &wfds))
                         fdh->write_active = EINA_TRUE;
                       if (FD_ISSET(fdh->fd, &exfds))
                         fdh->error_active = EINA_TRUE;
                       _ecore_try_add_to_call_list(fdh);
                    }
               }
          }
        EINA_LIST_FOREACH(file_fd_handlers, l, fdh)
          {
             if (!fdh->delete_me)
               {
                  if (FD_ISSET(fdh->fd, &rfds))
                    fdh->read_active = EINA_TRUE;
                  if (FD_ISSET(fdh->fd, &wfds))
                    fdh->write_active = EINA_TRUE;
                  if (FD_ISSET(fdh->fd, &exfds))
                    fdh->error_active = EINA_TRUE;
                  _ecore_try_add_to_call_list(fdh);
               }
          }
        _ecore_main_fd_handlers_cleanup();
#ifdef _WIN32
        _ecore_main_win32_handlers_cleanup();
#endif
        return 1;
     }
   return 0;
}

#endif

#ifndef _WIN32
# ifndef USE_G_MAIN_LOOP
static void
_ecore_main_fd_handlers_bads_rem(void)
{
   Ecore_Fd_Handler *fdh;
   Eina_Inlist *l;
   int found = 0;

   ERR("Removing bad fds");
   for (l = EINA_INLIST_GET(fd_handlers); l; )
     {
        fdh = (Ecore_Fd_Handler *)l;
        l = l->next;
        errno = 0;

        if ((fcntl(fdh->fd, F_GETFD) < 0) && (errno == EBADF))
          {
             ERR("Found bad fd at index %d", fdh->fd);
             if (fdh->flags & ECORE_FD_ERROR)
               {
                  ERR("Fd set for error! calling user");
                  fdh->references++;
                  if (!_ecore_call_fd_cb(fdh->func, fdh->data, fdh))
                    {
                       ERR("Fd function err returned 0, remove it");
                       if (!fdh->delete_me)
                         {
                            fdh->delete_me = EINA_TRUE;
                            _ecore_main_fdh_poll_del(fdh);
                            fd_handlers_to_delete = eina_list_append(fd_handlers_to_delete, fdh);
                         }
                       found++;
                    }
                  fdh->references--;
               }
             else
               {
                  ERR("Problematic fd found at %d! setting it for delete", fdh->fd);
                  if (!fdh->delete_me)
                    {
                       fdh->delete_me = EINA_TRUE;
                       _ecore_main_fdh_poll_del(fdh);
                       fd_handlers_to_delete = eina_list_append(fd_handlers_to_delete, fdh);
                    }

                  found++;
               }
          }
     }
   if (found == 0)
     {
#  ifdef HAVE_GLIB
        ERR("No bad fd found. Maybe a foreign fd from glib?");
#  else
        ERR("No bad fd found. EEEK!");
#  endif
     }
   _ecore_main_fd_handlers_cleanup();
}

# endif
#endif

static void
_ecore_main_fd_handlers_cleanup(void)
{
   Ecore_Fd_Handler *fdh;
   Eina_List *l, *l2;

   if (!fd_handlers_to_delete) return;
   EINA_LIST_FOREACH_SAFE(fd_handlers_to_delete, l, l2, fdh)
     {
        if (!fdh)
          {
             fd_handlers_to_delete = eina_list_remove_list(l, fd_handlers_to_delete);
             continue;
          }
        /* fdh->delete_me should be set for all fdhs at the start of the list */
        if (fdh->references)
          continue;
        if (fdh->buf_func && fd_handlers_with_buffer)
          fd_handlers_with_buffer = eina_list_remove(fd_handlers_with_buffer, fdh);
        if (fdh->prep_func && fd_handlers_with_prep)
          fd_handlers_with_prep = eina_list_remove(fd_handlers_with_prep, fdh);
        fd_handlers = (Ecore_Fd_Handler *)
          eina_inlist_remove(EINA_INLIST_GET(fd_handlers), EINA_INLIST_GET(fdh));
        if (fdh->file)
          file_fd_handlers = eina_list_remove(file_fd_handlers, fdh);
        ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
        ecore_fd_handler_mp_free(fdh);
        fd_handlers_to_delete = eina_list_remove_list(fd_handlers_to_delete, l);
     }
}

#ifdef _WIN32
static void
_ecore_main_win32_handlers_cleanup(void)
{
   Ecore_Win32_Handler *wh;
   Eina_List *l;
   Eina_List *l2;

   if (!win32_handlers_to_delete) return;
   EINA_LIST_FOREACH_SAFE(win32_handlers_to_delete, l, l2, wh)
     {
        if (!wh)
          {
             win32_handlers_to_delete = eina_list_remove_list(l, win32_handlers_to_delete);
             continue;
          }
        /* wh->delete_me should be set for all whs at the start of the list */
        if (wh->references)
          continue;
        win32_handlers = (Ecore_Win32_Handler *)
          eina_inlist_remove(EINA_INLIST_GET(win32_handlers), EINA_INLIST_GET(wh));
        ECORE_MAGIC_SET(wh, ECORE_MAGIC_NONE);
        ecore_win32_handler_mp_free(wh);
        win32_handlers_to_delete = eina_list_remove_list(win32_handlers_to_delete, l);
     }
}

#endif

static void
_ecore_main_fd_handlers_call(void)
{
   /* grab a new list */
    if (!fd_handlers_to_call_current)
      {
         fd_handlers_to_call_current = fd_handlers_to_call;
         fd_handlers_to_call = NULL;
      }

   if (!fd_handlers_to_call_current) return;
   eina_evlog("+fd_handlers", NULL, 0.0, NULL);

    while (fd_handlers_to_call_current)
      {
         Ecore_Fd_Handler *fdh = fd_handlers_to_call_current;

         if (!fdh->delete_me)
           {
              if ((fdh->read_active) ||
                  (fdh->write_active) ||
                  (fdh->error_active))
                {
                   fdh->references++;
                   if (!_ecore_call_fd_cb(fdh->func, fdh->data, fdh))
                     {
                        if (!fdh->delete_me)
                          {
                             fdh->delete_me = EINA_TRUE;
                             _ecore_main_fdh_poll_del(fdh);
                             fd_handlers_to_delete = eina_list_append(fd_handlers_to_delete, fdh);
                          }
                     }
                   fdh->references--;
#ifdef EFL_EXTRA_SANITY_CHECKS
                   _ecore_fd_valid();
#endif

                   fdh->read_active = EINA_FALSE;
                   fdh->write_active = EINA_FALSE;
                   fdh->error_active = EINA_FALSE;
                }
           }

         /* stop when we point to ourselves */
         if (fdh->next_ready == fdh)
           {
              fdh->next_ready = NULL;
              fd_handlers_to_call_current = NULL;
              break;
           }

         fd_handlers_to_call_current = fdh->next_ready;
         fdh->next_ready = NULL;
      }
   eina_evlog("-fd_handlers", NULL, 0.0, NULL);
}

static int
_ecore_main_fd_handlers_buf_call(void)
{
   Ecore_Fd_Handler *fdh;
   Eina_List *l, *l2;
   int ret;

   if (!fd_handlers_with_buffer) return 0;
   eina_evlog("+fd_handlers_buf", NULL, 0.0, NULL);
   ret = 0;
   EINA_LIST_FOREACH_SAFE(fd_handlers_with_buffer, l, l2, fdh)
     {
        if (!fdh)
          {
             fd_handlers_with_buffer = eina_list_remove_list(l, fd_handlers_with_buffer);
             continue;
          }
        if ((!fdh->delete_me) && fdh->buf_func)
          {
             fdh->references++;
             if (_ecore_call_fd_cb(fdh->buf_func, fdh->buf_data, fdh))
               {
                  ret |= _ecore_call_fd_cb(fdh->func, fdh->data, fdh);
                  fdh->read_active = EINA_TRUE;
                  _ecore_try_add_to_call_list(fdh);
               }
             fdh->references--;
          }
        else
          fd_handlers_with_buffer = eina_list_remove_list(fd_handlers_with_buffer, l);
     }
   eina_evlog("-fd_handlers_buf", NULL, 0.0, NULL);
   return ret;
}

#ifdef HAVE_LIBUV
static void
_ecore_main_loop_uv_prepare(uv_prepare_t* handle EINA_UNUSED)
{
   _dl_uv_timer_stop(&_ecore_main_uv_handle_timers);
   if(in_main_loop == 0 && do_quit)
     {
       _ecore_main_fd_handlers_cleanup();

       while (fd_handlers)
         {
           Ecore_Fd_Handler *fdh;

           fdh = fd_handlers;
           fd_handlers = (Ecore_Fd_Handler *)eina_inlist_remove(EINA_INLIST_GET(fd_handlers),
                                                                EINA_INLIST_GET(fdh));
           fdh->delete_me = 1;
           _ecore_main_fdh_poll_del(fdh);
           ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
           ecore_fd_handler_mp_free(fdh);
         }
       if (fd_handlers_with_buffer)
         fd_handlers_with_buffer = eina_list_free(fd_handlers_with_buffer);
       if (fd_handlers_with_prep)
         fd_handlers_with_prep = eina_list_free(fd_handlers_with_prep);
       if (fd_handlers_to_delete)
         fd_handlers_to_delete = eina_list_free(fd_handlers_to_delete);
       if (file_fd_handlers)
         file_fd_handlers = eina_list_free(file_fd_handlers);

       fd_handlers_to_call = NULL;
       fd_handlers_to_call_current = NULL;
       fd_handlers_to_delete = NULL;
       fd_handler_current = NULL;

       _dl_uv_prepare_stop(&_ecore_main_uv_prepare);
       _dl_uv_check_stop(&_ecore_main_uv_check);
       _dl_uv_stop(_dl_uv_default_loop());

       return;
     }

   in_main_loop++;

   if(!_ecore_main_uv_idling)
     {
        _ecore_main_uv_idling = EINA_TRUE;
        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
     }

   double t = -1;
   if(_ecore_main_uv_idling)
     {
       _ecore_idler_all_call(_mainloop_singleton);
       DBG("called idles");
       if(_ecore_idler_exist(_mainloop_singleton) || _ecore_event_exist())
         t = 0.0;
     }

   if (do_quit)
     {
       DBG("do quit outside loop");

       if(_ecore_main_uv_idling)
         {
            _ecore_idle_exiter_call(_mainloop_singleton);
            _ecore_animator_run_reset();

            _ecore_main_uv_idling = EINA_FALSE;
         }

       t = -1;

       _ecore_time_loop_time = ecore_time_get();
       _efl_loop_timer_enable_new();

       goto done;
     }

   assert(!fd_handlers_to_call);

   _ecore_time_loop_time = ecore_time_get();
   _efl_loop_timer_enable_new();
   if (_efl_loop_timers_exists() || t >= 0)
     {
       double tnext = _efl_loop_timer_next_get();
       if (t < 0 || (tnext >= 0 && tnext < t)) t = tnext;
       DBG("Should awake after %f", t);
       
       if (t >= 0.0)
         {
           //_dl_uv_timer_stop(&_ecore_main_uv_handle_timers);
           _dl_uv_timer_start(&_ecore_main_uv_handle_timers, &_ecore_main_loop_timer_run, t * 1000
                              , 0);
         }
       else
         DBG("Is not going to awake with timer");
     }
   else
     DBG("Is not going to awake with timer");

 done:
   if (fd_handlers_with_prep)
     _ecore_main_prepare_handlers();

   in_main_loop--;
}
#endif

#if !defined(USE_G_MAIN_LOOP)
enum {
   SPIN_MORE,
   SPIN_RESTART,
   LOOP_CONTINUE
};

static int
_ecore_main_loop_spin_core(void)
{
   /* as we are spinning we need to update loop time per spin */
    _ecore_time_loop_time = ecore_time_get();
    /* call all idlers */
    _ecore_idler_all_call(_mainloop_singleton);
    /* which returns false if no more idelrs exist */
    if (!_ecore_idler_exist(_mainloop_singleton)) return SPIN_RESTART;
    /* sneaky - drop through or if checks - the first one to succeed
     * drops through and returns "continue" so further ones dont run */
    if ((_ecore_main_select(0.0) > 0) || (_ecore_event_exist()) ||
        (_ecore_signal_count_get() > 0) || (do_quit))
      return LOOP_CONTINUE;
    /* default - spin more */
    return SPIN_MORE;
}

static int
_ecore_main_loop_spin_no_timers(void)
{
   /* if we have idlers we HAVE to spin and handle everything
    * in a polling way - spin in a tight polling loop */
     for (;; )
       {
          int action = _ecore_main_loop_spin_core();
          if (action != SPIN_MORE) return action;
          /* if an idler has added a timer then we need to go through
           * the start of the spin cycle again to handle cases properly */
          if (_efl_loop_timers_exists()) return SPIN_RESTART;
       }
     /* just contiune handling events etc. */
     return LOOP_CONTINUE;
}

static int
_ecore_main_loop_spin_timers(void)
{
   /* if we have idlers we HAVE to spin and handle everything
    * in a polling way - spin in a tight polling loop */
     for (;; )
       {
          int action = _ecore_main_loop_spin_core();
          if (action != SPIN_MORE) return action;
          /* if next timer expires now or in the past - stop spinning and
           * continue the mainloop walk as our "select" timeout has
           * expired now */
          if (_efl_loop_timer_next_get() <= 0.0) return LOOP_CONTINUE;
       }
     /* just contiune handling events etc. */
     return LOOP_CONTINUE;
}

static void
_ecore_fps_marker_1(void)
{
   if (!_ecore_fps_debug) return;
   t2 = ecore_time_get();
   if ((t1 > 0.0) && (t2 > 0.0)) _ecore_fps_debug_runtime_add(t2 - t1);
}

static void
_ecore_fps_marker_2(void)
{
   if (!_ecore_fps_debug) return;
   t1 = ecore_time_get();
}

static void
_ecore_main_loop_iterate_internal(int once_only)
{
   double next_time = -1.0;
   Eo *f, *p;

   in_main_loop++;

   /* destroy all optional futures */
   EINA_LIST_FREE(_pending_futures, f)
     efl_del(f);

   /* and propagate all promise value */
   EINA_LIST_FREE(_pending_promises, p)
     ecore_loop_promise_fulfill(p);

   /* expire any timers */
   _efl_loop_timer_expired_timers_call(_ecore_time_loop_time);

   /* process signals into events .... */
   _ecore_signal_received_process();
   /* if as a result of timers/animators or signals we have accumulated
    * events, then instantly handle them */
   if (_ecore_event_exist())
     {
        /* but first conceptually enter an idle state */
        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
        /* now quickly poll to see which input fd's are active */
        _ecore_main_select(0.0);
        /* allow newly queued timers to expire from now on */
        _efl_loop_timer_enable_new();
        /* go straight to processing the events we had queued */
        goto process_all;
     }

   if (once_only)
     {
        /* in once_only mode we should quickly poll for inputs, signals
         * if we got any events or signals, allow new timers to process.
         * use bitwise or to force both conditions to be tested and
         * merged together */
        if (_ecore_main_select(0.0) | _ecore_signal_count_get())
          {
             _efl_loop_timer_enable_new();
             goto process_all;
          }
     }
   else
     {
        /* call idle enterers ... */
        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
     }

   /* if these calls caused any buffered events to appear - deal with them */
   if (fd_handlers_with_buffer)
     _ecore_main_fd_handlers_buf_call();

   /* if there are any (buffered fd handling may generate them)
    * then jump to processing them */
   if (_ecore_event_exist())
     {
        _ecore_main_select(0.0);
        _efl_loop_timer_enable_new();
        goto process_all;
     }

   if (once_only)
     {
        /* in once_only mode enter idle here instead and then return */
        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
        _efl_loop_timer_enable_new();
        goto done;
     }

   _ecore_fps_marker_1();

   /* start of the sleeping or looping section */
start_loop: /*-*************************************************************/
   /* We could be looping here without exiting the function and we need to
      process future and promise before the next waiting period. */
   /* destroy all optional futures */
   EINA_LIST_FREE(_pending_futures, f)
     efl_del(f);

   /* and propagate all promise value */
   EINA_LIST_FREE(_pending_promises, p)
     ecore_loop_promise_fulfill(p);

   /* any timers re-added as a result of these are allowed to go */
   _efl_loop_timer_enable_new();
   /* if we have been asked to quit the mainloop then exit at this point */
   if (do_quit)
     {
        _efl_loop_timer_enable_new();
        goto done;
     }
   if (!_ecore_event_exist())
     {
        /* init flags */
        next_time = _efl_loop_timer_next_get();
        /* no idlers */
        if (!_ecore_idler_exist(_mainloop_singleton))
          {
             /* sleep until timeout or forever (-1.0) waiting for on fds */
             _ecore_main_select(next_time);
          }
        else
          {
             int action = LOOP_CONTINUE;

             /* no timers - spin */
             if (next_time < 0) action = _ecore_main_loop_spin_no_timers();
             /* timers - spin */
             else action = _ecore_main_loop_spin_timers();
             if (action == SPIN_RESTART) goto start_loop;
          }
     }
   _ecore_fps_marker_2();

   /* actually wake up and deal with input, events etc. */
process_all: /*-*********************************************************/

   /* we came out of our "wait state" so idle has exited */
   if (!once_only)
     {
        _ecore_animator_run_reset();
        _ecore_idle_exiter_call(_mainloop_singleton);
     }
   /* call the fd handler per fd that became alive... */
   /* this should read or write any data to the monitored fd and then */
   /* post events onto the ecore event pipe if necessary */
   _ecore_main_fd_handlers_call();
   if (fd_handlers_with_buffer) _ecore_main_fd_handlers_buf_call();
   /* process signals into events .... */
   _ecore_signal_received_process();
   /* handle events ... */
   _ecore_event_call();
   _ecore_main_fd_handlers_cleanup();

   if (once_only)
     {
        /* if in once_only mode handle idle exiting */
        _ecore_idle_enterer_call(_mainloop_singleton);
        _ecore_throttle();
     }

done: /*-*****************************************************************/
   /* Agressively flush animator */
   _ecore_animator_flush();
   if (!once_only)
     {
        /* Free all short lived strings */
        eina_slstr_local_clear();
     }
   in_main_loop--;
}

#endif

#ifdef _WIN32
typedef struct
{
   DWORD objects_nbr;
   HANDLE *objects;
   DWORD timeout;
} Ecore_Main_Win32_Thread_Data;

static unsigned int __stdcall
_ecore_main_win32_objects_wait_thread(void *data)
{
   Ecore_Main_Win32_Thread_Data *td;
   DWORD result;

   td = (Ecore_Main_Win32_Thread_Data *)data;
   result = MsgWaitForMultipleObjects(td->objects_nbr,
                                      (const HANDLE *)td->objects,
                                      FALSE,
                                      td->timeout,
                                      QS_ALLINPUT);
   return result;
}

static DWORD
_ecore_main_win32_objects_wait(DWORD objects_nbr,
                               const HANDLE *objects,
                               DWORD timeout)
{
   Ecore_Main_Win32_Thread_Data *threads_data;
   HANDLE *threads_handles;
   DWORD threads_nbr;
   DWORD threads_remain;
   DWORD objects_idx;
   DWORD result;
   DWORD i;

   if (objects_nbr < MAXIMUM_WAIT_OBJECTS)
     return MsgWaitForMultipleObjects(objects_nbr,
                                      objects,
                                      EINA_FALSE,
                                      timeout, QS_ALLINPUT);
   /*
    * too much objects, so we launch a bunch of threads to
    * wait for, each one calls MsgWaitForMultipleObjects
    */

   threads_nbr = objects_nbr / (MAXIMUM_WAIT_OBJECTS - 1);
   threads_remain = objects_nbr % (MAXIMUM_WAIT_OBJECTS - 1);
   if (threads_remain > 0)
     threads_nbr++;

   if (threads_nbr > MAXIMUM_WAIT_OBJECTS)
     {
        CRI("Too much objects to wait for (%lu).", objects_nbr);
        return WAIT_FAILED;
     }

   threads_handles = (HANDLE *)malloc(threads_nbr * sizeof(HANDLE));
   if (!threads_handles)
     {
        ERR("Can not allocate memory for the waiting thread.");
        return WAIT_FAILED;
     }

   threads_data = (Ecore_Main_Win32_Thread_Data *)malloc(threads_nbr * sizeof(Ecore_Main_Win32_Thread_Data));
   if (!threads_data)
     {
        ERR("Can not allocate memory for the waiting thread.");
        goto free_threads_handles;
     }

   objects_idx = 0;
   for (i = 0; i < threads_nbr; i++)
     {
        threads_data[i].timeout = timeout;
        threads_data[i].objects = (HANDLE *)objects + objects_idx;

        if ((i == (threads_nbr - 1)) && (threads_remain != 0))
          {
             threads_data[i].objects_nbr = threads_remain;
             objects_idx += threads_remain;
          }
        else
          {
             threads_data[i].objects_nbr = (MAXIMUM_WAIT_OBJECTS - 1);
             objects_idx += (MAXIMUM_WAIT_OBJECTS - 1);
          }

        threads_handles[i] = (HANDLE)_beginthreadex(NULL,
                                                    0,
                                                    _ecore_main_win32_objects_wait_thread,
                                                    &threads_data[i],
                                                    0,
                                                    NULL);
        if (!threads_handles[i])
          {
             DWORD j;

             ERR("Can not create the waiting threads.");
             WaitForMultipleObjects(i, threads_handles, TRUE, INFINITE);
             for (j = 0; j < i; j++)
               CloseHandle(threads_handles[i]);

             goto free_threads_data;
          }
     }

   result = WaitForMultipleObjects(threads_nbr,
                                   threads_handles,
                                   FALSE, /* we wait until one thread is signaled */
                                   INFINITE);

   if (result < (WAIT_OBJECT_0 + threads_nbr))
     {
        DWORD wait_res;

        /*
         * One of the thread callback has exited so we retrieve
         * its exit status, that is the returned value of
         * MsgWaitForMultipleObjects()
         */
        if (GetExitCodeThread(threads_handles[result - WAIT_OBJECT_0],
                              &wait_res))
          {
             WaitForMultipleObjects(threads_nbr, threads_handles, TRUE, INFINITE);
             for (i = 0; i < threads_nbr; i++)
               CloseHandle(threads_handles[i]);
             free(threads_data);
             free(threads_handles);
             return wait_res;
          }
     }
   else
     {
        ERR("Error when waiting threads.");
        if (result == WAIT_FAILED)
          {
             char *str;

             str = evil_last_error_get();
             ERR("%s", str);
             free(str);
          }
        goto close_thread;
     }

 close_thread:
   WaitForMultipleObjects(threads_nbr, threads_handles, TRUE, INFINITE);
   for (i = 0; i < threads_nbr; i++)
     CloseHandle(threads_handles[i]);
 free_threads_data:
   free(threads_data);
 free_threads_handles:
   free(threads_handles);

   return WAIT_FAILED;
}

static unsigned int
_stdin_wait_thread(void *data EINA_UNUSED)
{
   int c = getc(stdin);
   ungetc(c, stdin);

   return 0;
}

static int
_ecore_main_win32_select(int             nfds EINA_UNUSED,
                         fd_set         *readfds,
                         fd_set         *writefds,
                         fd_set         *exceptfds,
                         struct timeval *tv)
{
   HANDLE *objects;
   static HANDLE stdin_wait_thread = INVALID_HANDLE_VALUE;
   Eina_Bool stdin_thread_done = EINA_FALSE;
   HANDLE stdin_handle;
   int *sockets;
   Ecore_Fd_Handler *fdh;
   Ecore_Win32_Handler *wh;
   unsigned int fds_nbr = 0;
   unsigned int objects_nbr = 0;
   unsigned int events_nbr = 0;
   DWORD result;
   DWORD timeout;
   MSG msg;
   unsigned int i;
   int res;

   fds_nbr = eina_inlist_count(EINA_INLIST_GET(fd_handlers));
   sockets = (int *)malloc(fds_nbr * sizeof(int));
   if (!sockets)
     return -1;

   objects = (HANDLE)malloc((fds_nbr + eina_inlist_count(EINA_INLIST_GET(win32_handlers))) * sizeof(HANDLE));
   if (!objects)
     {
        free(sockets);
        return -1;
     }

   /* Create an event object per socket */
   EINA_INLIST_FOREACH(fd_handlers, fdh)
     {
        if (fdh->delete_me)
          continue;

        WSAEVENT event;
        long network_event;

        network_event = 0;
        if (readfds)
          {
             if (FD_ISSET(fdh->fd, readfds))
               network_event |= FD_READ | FD_CONNECT | FD_ACCEPT;
          }
        if (writefds)
          {
             if (FD_ISSET(fdh->fd, writefds))
               network_event |= FD_WRITE | FD_CLOSE;
          }
        if (exceptfds)
          {
             if (FD_ISSET(fdh->fd, exceptfds))
               network_event |= FD_OOB;
          }

        if (network_event)
          {
             event = WSACreateEvent();
             WSAEventSelect(fdh->fd, event, network_event);
             objects[objects_nbr] = event;
             sockets[events_nbr] = fdh->fd;
             events_nbr++;
             objects_nbr++;
          }
     }
   stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
   /* store the HANDLEs in the objects to wait for */
   EINA_INLIST_FOREACH(win32_handlers, wh)
     {
        if (wh->delete_me)
          continue;

        if (wh->h == stdin_handle)
          {
             if (stdin_wait_thread == INVALID_HANDLE_VALUE)
               stdin_wait_thread = (HANDLE)_beginthreadex(NULL,
                                                          0,
                                                          _stdin_wait_thread,
                                                          NULL,
                                                          0,
                                                          NULL);
             objects[objects_nbr] = stdin_wait_thread;
          }
        else
          {
             objects[objects_nbr] = wh->h;
          }
        objects_nbr++;
     }

   /* Empty the queue before waiting */
   while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
     {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
     }

   /* Wait for any message sent or posted to this queue */
   /* or for one of the passed handles be set to signaled. */
   if (!tv)
     timeout = INFINITE;
   else
     timeout = (DWORD)((tv->tv_sec * 1000.0) + (tv->tv_usec / 1000.0));

   if (timeout == 0)
     {
        res = 0;
        goto err;
     }

   result = _ecore_main_win32_objects_wait(objects_nbr,
                                           (const HANDLE *)objects,
                                           timeout);

   if (readfds)
     FD_ZERO(readfds);
   if (writefds)
     FD_ZERO(writefds);
   if (exceptfds)
     FD_ZERO(exceptfds);

   /* The result tells us the type of event we have. */
   if (result == WAIT_FAILED)
     {
        char *m;

        m = evil_last_error_get();
        WRN("%s", m);
        free(m);
        res = -1;
     }
   else if (result == WAIT_TIMEOUT)
     {
        INF("time-out interval elapsed.");
        res = 0;
     }
   else if (result == (WAIT_OBJECT_0 + objects_nbr))
     {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
          {
             TranslateMessage(&msg);
             DispatchMessage(&msg);
          }

        res = 0;
     }
   else if (result < WAIT_OBJECT_0 + events_nbr)
     {
        WSANETWORKEVENTS network_event;

        WSAEnumNetworkEvents(sockets[result], objects[result], &network_event);

        if ((network_event.lNetworkEvents & (FD_READ | FD_CONNECT | FD_ACCEPT)) && readfds)
          FD_SET(sockets[result], readfds);
        if ((network_event.lNetworkEvents & (FD_WRITE | FD_CLOSE)) && writefds)
          FD_SET(sockets[result], writefds);
        if ((network_event.lNetworkEvents & FD_OOB) && exceptfds)
          FD_SET(sockets[result], exceptfds);

        res = 1;
     }
   else if ((result >= (WAIT_OBJECT_0 + events_nbr)) &&
            (result < (WAIT_OBJECT_0 + objects_nbr)))
     {
        if (!win32_handler_current)
          {
             /* regular main loop, start from head */
              win32_handler_current = win32_handlers;
          }
        else
          {
             /* recursive main loop, continue from where we were */
              win32_handler_current = (Ecore_Win32_Handler *)EINA_INLIST_GET(win32_handler_current)->next;
          }

        if (objects[result - WAIT_OBJECT_0] == stdin_wait_thread)
          stdin_thread_done = EINA_TRUE;

        while (win32_handler_current)
          {
             wh = win32_handler_current;

             if (objects[result - WAIT_OBJECT_0] == wh->h ||
                 (objects[result - WAIT_OBJECT_0] == stdin_wait_thread && wh->h == stdin_handle))
               {
                  if (!wh->delete_me)
                    {
                       wh->references++;
                       if (!wh->func(wh->data, wh))
                         {
                            wh->delete_me = EINA_TRUE;
                            win32_handlers_to_delete = eina_list_append(win32_handlers_to_delete, wh);
                         }
                       wh->references--;
                    }
               }
             if (win32_handler_current) /* may have changed in recursive main loops */
               win32_handler_current = (Ecore_Win32_Handler *)EINA_INLIST_GET(win32_handler_current)->next;
          }
        res = 1;
     }
   else
     {
        ERR("unknown result...\n");
        res = -1;
     }

err :
   /* Remove event objects again */
   for (i = 0; i < events_nbr; i++) WSACloseEvent(objects[i]);

   if (stdin_thread_done)
     stdin_wait_thread = INVALID_HANDLE_VALUE;

   free(objects);
   free(sockets);
   return res;
}

#endif

Eo *_mainloop_singleton = NULL;

EOLIAN static Efl_Loop *
_efl_loop_main_get(Efl_Class *klass EINA_UNUSED, void *_pd EINA_UNUSED)
{
   if (!_mainloop_singleton)
     {
        _mainloop_singleton = efl_add(EFL_LOOP_CLASS, NULL);
     }

   return _mainloop_singleton;
}

EAPI Eo *
ecore_main_loop_get(void)
{
   return efl_loop_main_get(EFL_LOOP_CLASS);
}

EOLIAN static void
_efl_loop_iterate(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED)
{
   ecore_main_loop_iterate();
}

EOLIAN static int _efl_loop_iterate_may_block(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED, int may_block)
{
   return ecore_main_loop_iterate_may_block(may_block);
}

EOLIAN static unsigned char
_efl_loop_begin(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED)
{
   ecore_main_loop_begin();
   return _ecore_exit_code;
}

EOLIAN static void
_efl_loop_quit(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED, unsigned char exit_code)
{
   ecore_main_loop_quit();
   _ecore_exit_code = exit_code;
}

EOLIAN static Efl_Object *
_efl_loop_efl_object_provider_find(Eo *obj, Efl_Loop_Data *pd, const Efl_Object *klass)
{
   Efl_Object *r;

   if (klass == EFL_LOOP_CLASS) return obj;

   r = eina_hash_find(pd->providers, &klass);
   if (r) return r;

   return efl_provider_find(efl_super(obj, EFL_LOOP_CLASS), klass);
}

static void
_poll_trigger(void *data, const Efl_Event *event)
{
   Eo *parent = efl_parent_get(event->object);

   efl_event_callback_call(parent, data, NULL);
}

static void
_check_event_catcher_add(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item *array = event->info;
   Efl_Loop_Data *pd = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_EVENT_IDLE)
          {
             ++pd->idlers;
          }
        // XXX: all the below are kind of bad. ecore_pollers were special.
        // they all woke up at the SAME time based on interval, (all pollers
        // of interval 1 woke up together, those with 2 woke up when 1 and
        // 2 woke up, 4 woke up together along with 1 and 2 etc.
        // the below means they will just go off whenever but at a pre
        // defined interval - 1/60th, 6 and 66 seconds. not really great
        // pollers probably should be less frequent that 1/60th even on poll
        // high, medium probably down to 1-2 sec and low - yes maybe 30 or 60
        // sec... still - not timed to wake up together. :(
        else if (array[i].desc == EFL_LOOP_EVENT_POLL_HIGH)
          {
             if (!pd->poll_high)
               {
                  // Would be better to have it in sync with normal wake up
                  // of the main loop for better energy efficiency, I guess.
                  pd->poll_high = efl_add(EFL_LOOP_TIMER_CLASS, event->object,
                                          efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, _poll_trigger, EFL_LOOP_EVENT_POLL_HIGH),
                                          efl_loop_timer_interval_set(efl_added, 1.0/60.0));
               }
             ++pd->pollers.high;
          }
        else if (array[i].desc == EFL_LOOP_EVENT_POLL_MEDIUM)
          {
             if (!pd->poll_medium)
               {
                  pd->poll_medium = efl_add(EFL_LOOP_TIMER_CLASS, event->object,
                                            efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, _poll_trigger, EFL_LOOP_EVENT_POLL_MEDIUM),
                                            efl_loop_timer_interval_set(efl_added, 6));
               }
             ++pd->pollers.medium;
          }
        else if (array[i].desc == EFL_LOOP_EVENT_POLL_LOW)
          {
             if (!pd->poll_low)
               {
                  pd->poll_low = efl_add(EFL_LOOP_TIMER_CLASS, event->object,
                                         efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, _poll_trigger, EFL_LOOP_EVENT_POLL_LOW),
                                         efl_loop_timer_interval_set(efl_added, 66));
               }
             ++pd->pollers.low;
          }
     }
}

static void
_check_event_catcher_del(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item *array = event->info;
   Efl_Loop_Data *pd = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_EVENT_IDLE)
          {
             --pd->idlers;
          }
        else if (array[i].desc == EFL_LOOP_EVENT_POLL_HIGH)
          {
             --pd->pollers.high;
             if (!pd->pollers.high)
               {
                  ecore_timer_del(pd->poll_high);
                  pd->poll_high = NULL;
               }
          }
        else if (array[i].desc == EFL_LOOP_EVENT_POLL_MEDIUM)
          {
             --pd->pollers.medium;
             if (!pd->pollers.medium)
               {
                  ecore_timer_del(pd->poll_medium);
                  pd->poll_medium = NULL;
               }
          }
        else if (array[i].desc == EFL_LOOP_EVENT_POLL_LOW)
          {
             --pd->pollers.low;
             if (!pd->pollers.low)
               {
                  ecore_timer_del(pd->poll_low);
                  pd->poll_low = NULL;
               }
          }
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(event_catcher_watch,
                          { EFL_EVENT_CALLBACK_ADD, _check_event_catcher_add },
                          { EFL_EVENT_CALLBACK_DEL, _check_event_catcher_del });

EOLIAN static Efl_Object *
_efl_loop_efl_object_constructor(Eo *obj, Efl_Loop_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_LOOP_CLASS));
   if (!obj) return NULL;

   efl_event_callback_array_add(obj, event_catcher_watch(), pd);

   pd->providers = eina_hash_pointer_new((void*) efl_unref);

   return obj;
}

EOLIAN static void
_efl_loop_efl_object_destructor(Eo *obj, Efl_Loop_Data *pd)
{
   eina_hash_free(pd->providers);

   efl_del(pd->poll_low);
   efl_del(pd->poll_medium);
   efl_del(pd->poll_high);

   efl_destructor(efl_super(obj, EFL_LOOP_CLASS));
}

typedef struct _Efl_Internal_Promise Efl_Internal_Promise;
struct _Efl_Internal_Promise
{
   union {
      Ecore_Job *job;
      Efl_Loop_Timer *timer;
   } u;
   Efl_Promise *promise;

   const void *data;

   Eina_Bool job_is : 1;
};

static void
_efl_loop_job_cb(void *data)
{
   Efl_Internal_Promise *j = data;

   efl_promise_value_set(j->promise, (void*) j->data, NULL);

   free(j);
}

static void
_efl_loop_arguments_cleanup(Eina_Array *arga)
{
   Eina_Stringshare *s;

   while ((s = eina_array_pop(arga)))
     eina_stringshare_del(s);
   eina_array_free(arga);
}

static void
_efl_loop_arguments_send(void *data, const Efl_Event *ev EINA_UNUSED)
{
   static Eina_Bool initialization = EINA_TRUE;
   Efl_Loop_Arguments arge;
   Eina_Array *arga = data;

   arge.argv = arga;
   arge.initialization = initialization;
   initialization = EINA_FALSE;

   efl_event_callback_call(ecore_main_loop_get(), EFL_LOOP_EVENT_ARGUMENTS, &arge);

   _efl_loop_arguments_cleanup(arga);
}

static void
_efl_loop_arguments_cancel(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _efl_loop_arguments_cleanup(data);
}

// It doesn't make sense to send those argument to any other mainloop
// As it also doesn't make sense to allow anyone to override this, so
// should be internal for sure, not even protected.
EAPI void
ecore_loop_arguments_send(int argc, const char **argv)
{
   Efl_Future *job;
   Eina_Array *arga;
   int i = 0;

   arga = eina_array_new(argc);
   for (i = 0; i < argc; i++)
     eina_array_push(arga, eina_stringshare_add(argv[i]));

   job = efl_loop_job(ecore_main_loop_get(), NULL);
   efl_future_then(job, _efl_loop_arguments_send, _efl_loop_arguments_cancel, NULL, arga);
}

static void _efl_loop_timeout_force_cancel_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void _efl_loop_timeout_cb(void *data, const Efl_Event *event EINA_UNUSED);

// Only one main loop handle for now
void
ecore_loop_future_register(Efl_Loop *l EINA_UNUSED, Efl_Future *f)
{
   _pending_futures = eina_list_append(_pending_futures, f);
}

void
ecore_loop_future_unregister(Efl_Loop *l EINA_UNUSED, Efl_Future *f)
{
   _pending_futures = eina_list_remove(_pending_futures, f);
}

void
ecore_loop_promise_register(Efl_Loop *l EINA_UNUSED, Efl_Promise *p)
{
   _pending_promises = eina_list_append(_pending_promises, p);
}

void
ecore_loop_promise_unregister(Efl_Loop *l EINA_UNUSED, Efl_Promise *p)
{
   _pending_promises = eina_list_remove(_pending_promises, p);
}

EFL_CALLBACKS_ARRAY_DEFINE(timeout,
                          { EFL_LOOP_TIMER_EVENT_TICK, _efl_loop_timeout_cb },
                          { EFL_EVENT_DEL, _efl_loop_timeout_force_cancel_cb });

static Eina_Future *
_efl_loop_Eina_FutureXXX_job(Eo *obj, Efl_Loop_Data *pd EINA_UNUSED)
{
   // NOTE: Eolian should do efl_future_then() to bind future to object.
   return efl_future_Eina_FutureXXX_then(obj,
      eina_future_resolved(efl_loop_future_scheduler_get(obj),
                           EINA_VALUE_EMPTY));
}

static void
_efl_loop_Eina_FutureXXX_idle_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   Efl_Loop_Promise_Simple_Data *d = data;
   ecore_idler_del(d->idler);
   efl_loop_promise_simple_data_mp_free(d);
}

static Eina_Bool
_efl_loop_Eina_FutureXXX_idle_done(void *data)
{
   Efl_Loop_Promise_Simple_Data *d = data;
   eina_promise_resolve(d->promise, EINA_VALUE_EMPTY);
   efl_loop_promise_simple_data_mp_free(d);
   return EINA_FALSE;
}

static Eina_Future *
_efl_loop_Eina_FutureXXX_idle(Eo *obj, Efl_Loop_Data *pd EINA_UNUSED)
{
   Efl_Loop_Promise_Simple_Data *d;
   Eina_Promise *p;

   d = efl_loop_promise_simple_data_calloc(1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->idler = ecore_idler_add(_efl_loop_Eina_FutureXXX_idle_done, d);
   EINA_SAFETY_ON_NULL_GOTO(d->idler, idler_error);

   p = eina_promise_new(efl_loop_future_scheduler_get(obj),
                        _efl_loop_Eina_FutureXXX_idle_cancel, d);
   // d is dead if p is NULL
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, NULL);
   d->promise = p;

   // NOTE: Eolian should do efl_future_then() to bind future to object.
   return efl_future_Eina_FutureXXX_then(obj, eina_future_new(p));

 idler_error:
   efl_loop_promise_simple_data_mp_free(d);
   return NULL;
}

static void
_efl_loop_Eina_FutureXXX_timeout_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   Efl_Loop_Promise_Simple_Data *d = data;
   ecore_timer_del(d->timer);
   efl_loop_promise_simple_data_mp_free(d);
}

static Eina_Bool
_efl_loop_Eina_FutureXXX_timeout_done(void *data)
{
   Efl_Loop_Promise_Simple_Data *d = data;
   eina_promise_resolve(d->promise, EINA_VALUE_EMPTY);
   efl_loop_promise_simple_data_mp_free(d);
   return EINA_FALSE;
}

static Eina_Future *
_efl_loop_Eina_FutureXXX_timeout(Eo *obj, Efl_Loop_Data *pd EINA_UNUSED, double time)
{
   Efl_Loop_Promise_Simple_Data *d;
   Eina_Promise *p;

   d = efl_loop_promise_simple_data_calloc(1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->timer = ecore_timer_add(time, _efl_loop_Eina_FutureXXX_timeout_done, d);
   EINA_SAFETY_ON_NULL_GOTO(d->timer, timer_error);

   p = eina_promise_new(efl_loop_future_scheduler_get(obj),
                        _efl_loop_Eina_FutureXXX_timeout_cancel, d);
   // d is dead if p is NULL
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, NULL);
   d->promise = p;

   // NOTE: Eolian should do efl_future_then() to bind future to object.
   return efl_future_Eina_FutureXXX_then(obj, eina_future_new(p));

 timer_error:
   efl_loop_promise_simple_data_mp_free(d);
   return NULL;
}

/* This event will be triggered when the main loop is destroyed and destroy its timers along */
static void _efl_loop_internal_cancel(Efl_Internal_Promise *p);

static void
_efl_loop_timeout_force_cancel_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   _efl_loop_internal_cancel(data);
}

static void _efl_loop_job_cancel(void* data, const Efl_Event *ev EINA_UNUSED);

static void
_efl_loop_timeout_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Internal_Promise *t = data;

   efl_promise_value_set(t->promise, (void*) t->data, NULL);
   efl_del(t->promise);

   efl_event_callback_array_del(t->u.timer, timeout(), t);
   efl_del(t->u.timer);
}

static void
_efl_loop_internal_cancel(Efl_Internal_Promise *p)
{
   efl_promise_failed_set(p->promise, EINA_ERROR_FUTURE_CANCEL);
   efl_del(p->promise);
   free(p);
}

static void
_efl_loop_job_cancel(void* data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Internal_Promise *j = data;

   if (j->job_is)
     {
        ecore_job_del(j->u.job);
     }
   else
     {
        efl_event_callback_array_del(j->u.timer, timeout(), j);
        efl_del(j->u.timer);
     }

   _efl_loop_internal_cancel(j);
}

static Efl_Internal_Promise *
_efl_internal_promise_new(Efl_Promise* promise, const void *data)
{
   Efl_Internal_Promise *p;

   p = calloc(1, sizeof (Efl_Internal_Promise));
   if (!p) return NULL;

   efl_event_callback_add(promise, EFL_PROMISE_EVENT_FUTURE_NONE, _efl_loop_job_cancel, p);

   p->promise = promise;
   p->data = data;

   return p;
}

static Efl_Future *
_efl_loop_job(Eo *obj, Efl_Loop_Data *pd EINA_UNUSED, const void *data)
{
   Efl_Internal_Promise *j;
   Efl_Object *promise;

   promise = efl_add(EFL_PROMISE_CLASS, obj);
   if (!promise) return NULL;

   j = _efl_internal_promise_new(promise, data);
   if (!j) goto on_error;

   j->job_is = EINA_TRUE;
   j->u.job = ecore_job_add(_efl_loop_job_cb, j);
   if (!j->u.job) goto on_error;

   return efl_promise_future_get(promise);

 on_error:
   efl_del(promise);
   free(j);

   return NULL;
}

static Efl_Future *
_efl_loop_timeout(Eo *obj, Efl_Loop_Data *pd EINA_UNUSED, double time, const void *data)
{
   Efl_Internal_Promise *t;
   Efl_Object *promise;

   promise = efl_add(EFL_PROMISE_CLASS, obj);
   if (!promise) return NULL;

   t = _efl_internal_promise_new(promise, data);
   if (!t) goto on_error;

   t->job_is = EINA_FALSE;
   t->u.timer = efl_add(EFL_LOOP_TIMER_CLASS, obj,
                        efl_loop_timer_interval_set(efl_added, time),
                        efl_event_callback_array_add(efl_added, timeout(), t));

   if (!t->u.timer) goto on_error;

   return efl_promise_future_get(promise);

 on_error:
   efl_del(promise);
   free(t);

   return NULL;
}

static Eina_Bool
_efl_loop_register(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd, const Efl_Class *klass, const Efl_Object *provider)
{
   // The passed object does not provide that said class.
   if (!efl_isa(provider, klass)) return EINA_FALSE;

   // Note: I would prefer to use efl_xref here, but I can't figure a nice way to
   // call efl_xunref on hash destruction.
   return eina_hash_add(pd->providers, &klass, efl_ref(provider));
}

static Eina_Bool
_efl_loop_unregister(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd, const Efl_Class *klass, const Efl_Object *provider)
{
   return eina_hash_del(pd->providers, &klass, provider);
}

Efl_Version _app_efl_version = { 0, 0, 0, 0, NULL, NULL };

EWAPI void
efl_build_version_set(int vmaj, int vmin, int vmic, int revision,
                      const char *flavor, const char *build_id)
{
   // note: EFL has not been initialized yet at this point (ie. no eina call)
   _app_efl_version.major = vmaj;
   _app_efl_version.minor = vmin;
   _app_efl_version.micro = vmic;
   _app_efl_version.revision = revision;
   free((char *) _app_efl_version.flavor);
   free((char *) _app_efl_version.build_id);
   _app_efl_version.flavor = flavor ? strdup(flavor) : NULL;
   _app_efl_version.build_id = build_id ? strdup(build_id) : NULL;
}

EOLIAN static const Efl_Version *
_efl_loop_app_efl_version_get(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED)
{
   return &_app_efl_version;
}

EOLIAN static const Efl_Version *
_efl_loop_efl_version_get(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED)
{
   /* vanilla EFL: flavor = NULL */
   static const Efl_Version version = {
      .major = VMAJ,
      .minor = VMIN,
      .micro = VMIC,
      .revision = VREV,
      .build_id = EFL_BUILD_ID,
      .flavor = NULL
   };

   return &version;
}

EOLIAN static Eina_Future_Scheduler *
_efl_loop_future_scheduler_get(Eo *obj EINA_UNUSED,
                               Efl_Loop_Data *pd EINA_UNUSED)
{
   return _ecore_event_future_scheduler_get();
}


#include "efl_loop.eo.c"
