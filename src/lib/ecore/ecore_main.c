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
# include <ieeefp.h> // for Solaris
#endif

#ifdef HAVE_ISFINITE
# define ECORE_FINITE(t)  isfinite(t)
#else
# define ECORE_FINITE(t) finite(t)
#endif

//#define FIX_HZ 1

#ifdef FIX_HZ
# include <sys/param.h>
# ifndef HZ
#  define HZ 100
# endif
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#include "ecore_main_common.h"

#include "eina_internal.h"

#ifdef USE_G_MAIN_LOOP
# include <glib.h>
#endif

#ifdef HAVE_LIBUV
# ifdef HAVE_NODE_UV_H
#  include <node/uv.h>
# elif defined(HAVE_NODEJS_DEPS_UV_UV_H)
#  include <nodejs/deps/uv/uv.h>
# elif defined(HAVE_NODEJS_DEPS_UV_INCLUDE_UV_H)
#  include <nodejs/deps/uv/include/uv.h>
# elif defined(HAVE_NODEJS_SRC_UV_H)
#  include <nodejs/src/uv.h>
# elif defined(HAVE_UV_H)
#  include <uv.h>
# else
#  error No uv.h header found?
# endif

// XXX: FIXME: use eina_module
# if defined(HAVE_DLOPEN) && !defined(_WIN32)
#  include <dlfcn.h>
# endif

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

#define NS_PER_SEC (1000000000.0)

struct _Ecore_Fd_Handler
{
   EINA_INLIST;
   ECORE_MAGIC;
   Ecore_Fd_Handler       *next_ready;
   int                     fd;
   Ecore_Fd_Handler_Flags  flags;
   Eo                     *handler;
   Eo                     *loop;
   Efl_Loop_Data          *loop_data;
   Ecore_Fd_Cb             func;
   void                   *data;
   Ecore_Fd_Cb             buf_func;
   void                   *buf_data;
   Ecore_Fd_Prep_Cb        prep_func;
   void                   *prep_data;
   int                     references;
#if defined(USE_G_MAIN_LOOP)
   GPollFD                 gfd;
#endif
#ifdef HAVE_LIBUV
   uv_poll_t               uv_handle;
#endif
   Eina_Bool               read_active : 1;
   Eina_Bool               write_active : 1;
   Eina_Bool               error_active : 1;
   Eina_Bool               delete_me : 1;
   Eina_Bool               file : 1;
   Eina_Bool               legacy : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Fd_Handler);

#ifdef _WIN32
struct _Ecore_Win32_Handler
{
   EINA_INLIST;
   ECORE_MAGIC;
   HANDLE                  h;
   Eo                     *handler;
   Eo                     *loop;
   Efl_Loop_Data          *loop_data;
   Ecore_Win32_Handle_Cb   func;
   void                   *data;
   int                     references;
   Eina_Bool               delete_me : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Win32_Handler);
#endif

#if !defined(USE_G_MAIN_LOOP) && !defined(HAVE_LIBUV)
static int  _ecore_main_select(Eo *obj, Efl_Loop_Data *pd, double timeout);
#endif
static void _ecore_main_prepare_handlers(Eo *obj, Efl_Loop_Data *pd);
static void _ecore_main_fd_handlers_cleanup(Eo *obj, Efl_Loop_Data *pd);
#ifndef _WIN32
# ifndef USE_G_MAIN_LOOP
static void _ecore_main_fd_handlers_bads_rem(Eo *obj, Efl_Loop_Data *pd);
# endif
#endif
static void _ecore_main_fd_handlers_call(Eo *obj, Efl_Loop_Data *pd);
static int  _ecore_main_fd_handlers_buf_call(Eo *obj, Efl_Loop_Data *pd);
#ifndef USE_G_MAIN_LOOP
static void _ecore_main_loop_iterate_internal(Eo *obj, Efl_Loop_Data *pd, int once_only);
#endif

#ifdef _WIN32
static int _ecore_main_win32_select(int             nfds,
                                    fd_set         *readfds,
                                    fd_set         *writefds,
                                    fd_set         *exceptfds,
                                    struct timeval *timeout);
static void _ecore_main_win32_handlers_cleanup(Eo *obj, Efl_Loop_Data *pd);
#endif

static void _ecore_main_loop_setup(Eo *obj, Efl_Loop_Data *pd);
static void _ecore_main_loop_clear(Eo *obj, Efl_Loop_Data *pd);

// for legacy mainloop only and not other loops
int in_main_loop = 0;
#ifndef USE_G_MAIN_LOOP
static double t1 = 0.0;
static double t2 = 0.0;
#endif

#ifdef _WIN32
Ecore_Select_Function        main_loop_select    = _ecore_main_win32_select;
static Ecore_Select_Function general_loop_select = _ecore_main_win32_select;
#else
# include <sys/select.h>
Ecore_Select_Function        main_loop_select    = select;
static Ecore_Select_Function general_loop_select = select;
#endif

#ifdef USE_G_MAIN_LOOP
static GPollFD    ecore_epoll_fd;
static GPollFD    ecore_timer_fd;
static GSource   *ecore_glib_source;
static guint      ecore_glib_source_id;
static GMainLoop *ecore_main_loop;
static gboolean   ecore_idling;
static gboolean   _ecore_glib_idle_enterer_called;
static gboolean   ecore_fds_ready;
#endif

#ifdef EFL_EXTRA_SANITY_CHECKS
static inline void
_ecore_fd_valid(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd EINA_UNUSED)
{
# ifdef HAVE_SYS_EPOLL_H
   if ((pd->epoll_fd >= 0) &&
       (fcntl(pd->epoll_fd, F_GETFD) < 0))
     {
        ERR("arghhh you caught me! report a backtrace to edevel!");
#  ifdef HAVE_PAUSE
        pause();
#  else
        sleep(60);
#  endif
     }
# endif
}
#endif

static inline void
_ecore_try_add_to_call_list(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd, Ecore_Fd_Handler *fdh)
{
   // check if this fdh is already in the list
   if (fdh->next_ready)
     {
        DBG("next_ready");
        return;
     }
   if (fdh->read_active || fdh->write_active || fdh->error_active)
     {
        DBG("added");
        // make sure next_ready is non-null by pointing to ourselves
        // use that to indicate this fdh is in the ready list
        // insert at the head of the list to avoid trouble
        fdh->next_ready = pd->fd_handlers_to_call ? pd->fd_handlers_to_call : fdh;
        pd->fd_handlers_to_call = fdh;
     }
}

static inline void
_throttle_do(Efl_Loop_Data *pd)
{
   if (pd->throttle == 0) return
   eina_evlog("+throttle", NULL, 0.0, NULL);
   usleep(pd->throttle);
   eina_evlog("-throttle", NULL, 0.0, NULL);
}

#ifdef HAVE_SYS_EPOLL_H
static inline int
_ecore_get_epoll_fd(Eo *obj, Efl_Loop_Data *pd)
{
   if (pd->epoll_pid && (pd->epoll_pid != getpid())) // forked!
     _ecore_main_loop_clear(obj, pd);
   if ((pd->epoll_pid == 0) && (pd->epoll_fd < 0))
     _ecore_main_loop_setup(obj, pd);
   return pd->epoll_fd;
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
   if (fdh->flags & ECORE_FD_READ)  events |= EPOLLIN | EPOLLHUP;
   if (fdh->flags & ECORE_FD_WRITE) events |= EPOLLOUT | EPOLLHUP;
   if (fdh->flags & ECORE_FD_ERROR) events |= EPOLLERR | EPOLLPRI | EPOLLHUP;
   return events;
}
#endif

#ifdef USE_G_MAIN_LOOP
static inline int
_gfd_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ)  events |= G_IO_IN | G_IO_HUP;
   if (fdh->flags & ECORE_FD_WRITE) events |= G_IO_OUT | G_IO_HUP;
   if (fdh->flags & ECORE_FD_ERROR) events |= G_IO_ERR | G_IO_HUP;
   return events;
}
#endif

#ifdef HAVE_LIBUV
static void
_ecore_main_uv_poll_cb(uv_poll_t *handle, int status, int events)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;

   DBG("_ecore_main_uv_poll_cb %p status %d events %d",
       (void *)handle->data, status, events);
   Ecore_Fd_Handler *fdh = handle->data;

   if (_ecore_main_uv_idling)
     {
        DBG("not IDLE anymore");
        _ecore_main_uv_idling = EINA_FALSE;
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_EXIT, NULL);
        _ecore_animator_run_reset();
     }

  if (status)               fdh->error_active = EINA_TRUE;
  if (events & UV_READABLE) fdh->read_active  = EINA_TRUE;
  if (events & UV_WRITABLE) fdh->write_active = EINA_TRUE;

  if (events & UV_DISCONNECT)
     {
        fdh->read_active  = EINA_TRUE;
        fdh->write_active = EINA_TRUE;
        fdh->error_active = EINA_TRUE;
     }
  _ecore_try_add_to_call_list(obj, pd, fdh);

  _ecore_main_fd_handlers_call(obj, pd);
  if (pd->fd_handlers_with_buffer) _ecore_main_fd_handlers_buf_call(obj, pd);
  _ecore_signal_received_process(obj, pd);
  efl_loop_message_process(obj);
  _ecore_main_fd_handlers_cleanup(obj, pd);
  _efl_loop_timer_expired_timers_call(obj, pd, pd->loop_time);
}

static int
_ecore_main_uv_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ)  events |= UV_READABLE;
   if (fdh->flags & ECORE_FD_WRITE) events |= UV_WRITABLE;
   DBG("events is %d", (int)events);
   return events;
}
#endif

static inline int
_ecore_main_fdh_poll_add(Efl_Loop_Data *pd EINA_UNUSED, Ecore_Fd_Handler *fdh)
{
   DBG("_ecore_main_fdh_poll_add");
   int r = 0;

#ifdef HAVE_SYS_EPOLL_H
# ifdef HAVE_LIBUV
   if (!_dl_uv_run)
# endif
     {
        if ((!fdh->file) && (pd->epoll_fd >= 0))
          r = _ecore_epoll_add(_ecore_get_epoll_fd(fdh->loop, pd), fdh->fd,
                               _ecore_poll_events_from_fdh(fdh), fdh);
     }
# ifdef HAVE_LIBUV
   else
# endif
#endif
     {
#ifdef HAVE_LIBUV
        if (!fdh->file)
          {
             DBG("_ecore_main_fdh_poll_add libuv socket %p", fdh);
             fdh->uv_handle.data = fdh;
             DBG("_ecore_main_fdh_poll_add2 %p", fdh);
             _dl_uv_poll_init_socket(_dl_uv_default_loop(),
                                     &fdh->uv_handle, fdh->fd);
             DBG("_ecore_main_fdh_poll_add3 %p", fdh->uv_handle.data);
             _dl_uv_poll_start(&fdh->uv_handle,
                               _ecore_main_uv_events_from_fdh(fdh)
                               , _ecore_main_uv_poll_cb);
             DBG("_ecore_main_fdh_poll_add libuv DONE");
          }
        else
          {
             DBG("_ecore_main_fdh_poll_add libuv file");
             fdh->uv_handle.data = fdh;
             DBG("_ecore_main_fdh_poll_add2 %p", fdh);
             _dl_uv_poll_init(_dl_uv_default_loop(),
                              &fdh->uv_handle, fdh->fd);
             DBG("_ecore_main_fdh_poll_add3 %p", fdh->uv_handle.data);
             _dl_uv_poll_start(&fdh->uv_handle,
                               _ecore_main_uv_events_from_fdh(fdh)
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
_ecore_main_fdh_poll_del(Efl_Loop_Data *pd, Ecore_Fd_Handler *fdh)
{
#ifdef HAVE_SYS_EPOLL_H
# ifdef HAVE_LIBUV
   if (!_dl_uv_run)
# endif
     {
        if (!pd)
          {
             WRN("Efl_Loop_Data is NULL!");
             return;
          }

        if ((!fdh->file) && (pd->epoll_fd >= 0))
          {
             struct epoll_event ev;
             int efd = _ecore_get_epoll_fd(fdh->loop, pd);

             memset(&ev, 0, sizeof (ev));
             DBG("removing poll on %d", fdh->fd);
             // could get an EBADF if somebody closed the FD before removing
             if ((epoll_ctl(efd, EPOLL_CTL_DEL, fdh->fd, &ev) < 0))
               {
                  if (errno == EBADF)
                    {
                       WRN("fd %d closed, can't remove from epoll - reinit!",
                           fdh->fd);
                       _ecore_main_loop_clear(fdh->loop, pd);
                       _ecore_main_loop_setup(fdh->loop, pd);
                    }
                  else ERR("Failed to delete epoll fd %d! (errno=%d)",
                           fdh->fd, errno);
               }
          }
     }
# ifdef HAVE_LIBUV
   else
# endif
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
_ecore_main_fdh_poll_modify(Efl_Loop_Data *pd EINA_UNUSED, Ecore_Fd_Handler *fdh)
{
   DBG("_ecore_main_fdh_poll_modify %p", fdh);
   int r = 0;
#ifdef HAVE_SYS_EPOLL_H
# ifdef HAVE_LIBUV
   if (!_dl_uv_run)
# endif
     {
        if ((!fdh->file) && (pd->epoll_fd >= 0))
          {
             struct epoll_event ev;
             int efd = _ecore_get_epoll_fd(fdh->loop, pd);

             memset(&ev, 0, sizeof (ev));
             ev.events = _ecore_poll_events_from_fdh(fdh);
             ev.data.ptr = fdh;
             DBG("modifing epoll on %d to %08x", fdh->fd, ev.events);
             r = epoll_ctl(efd, EPOLL_CTL_MOD, fdh->fd, &ev);
          }
     }
# ifdef HAVE_LIBUV
   else
# endif
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

static Eina_Bool
_ecore_main_idlers_exist(Efl_Loop_Data *pd)
{
   return pd->idlers || eina_freeq_ptr_pending(eina_freeq_main_get());
}

static void
_ecore_main_idler_all_call(Eo *loop)
{
   efl_event_callback_call(loop, EFL_LOOP_EVENT_IDLE, NULL);
   // just spin in an idler until the free queue is empty freeing 84 items
   // from the free queue each time.for now this seems like an ok balance
   // between going in and out of a reduce func with mutexes around it
   // vs blocking mainloop for too long. this number is up for discussion
   eina_freeq_reduce(eina_freeq_main_get(), 84);
}

#ifdef HAVE_SYS_EPOLL_H
static inline int
_ecore_main_fdh_epoll_mark_active(Eo *obj, Efl_Loop_Data *pd)
{
   DBG("_ecore_main_fdh_epoll_mark_active");
   struct epoll_event ev[32];
   int i, ret;
   int efd = _ecore_get_epoll_fd(obj, pd);

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

        if (ev[i].events & EPOLLIN)  fdh->read_active  = EINA_TRUE;
        if (ev[i].events & EPOLLOUT) fdh->write_active = EINA_TRUE;
        if (ev[i].events & EPOLLERR) fdh->error_active = EINA_TRUE;

        if (ev[i].events & EPOLLHUP)
          {
             fdh->read_active  = EINA_TRUE;
             fdh->write_active = EINA_TRUE;
             fdh->error_active = EINA_TRUE;
          }

        _ecore_try_add_to_call_list(obj, pd, fdh);
     }
   return ret;
}
#endif

#ifdef USE_G_MAIN_LOOP
static inline int
_ecore_main_fdh_glib_mark_active(Eo *obj, Efl_Loop_Data *pd)
{
   Ecore_Fd_Handler *fdh;
   int ret = 0;

   // call the prepare callback for all handlers
   EINA_INLIST_FOREACH(pd->fd_handlers, fdh)
     {
        if (fdh->delete_me) continue;

        if (fdh->gfd.revents & G_IO_IN)  fdh->read_active  = EINA_TRUE;
        if (fdh->gfd.revents & G_IO_OUT) fdh->write_active = EINA_TRUE;
        if (fdh->gfd.revents & G_IO_ERR) fdh->error_active = EINA_TRUE;

        if (fdh->gfd.revents & G_IO_HUP)
          {
             fdh->read_active  = EINA_TRUE;
             fdh->write_active = EINA_TRUE;
             fdh->error_active = EINA_TRUE;
          }

        _ecore_try_add_to_call_list(obj, pd, fdh);

        if (fdh->gfd.revents & (G_IO_IN | G_IO_OUT | G_IO_ERR)) ret++;
     }

   return ret;
}

// like we are about to enter main_loop_select in  _ecore_main_select
static gboolean
_ecore_main_gsource_prepare(GSource *source EINA_UNUSED,
                            gint    *next_time)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;
   gboolean ready = FALSE;

   in_main_loop++;
   pd->in_loop = in_main_loop;

   if ((!ecore_idling) && (!_ecore_glib_idle_enterer_called))
     {
        pd->loop_time = ecore_time_get();
        _efl_loop_timer_expired_timers_call(obj, pd, pd->loop_time);

        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
        _ecore_glib_idle_enterer_called = FALSE;

        if (pd->fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call(obj, pd);
     }

   _ecore_signal_received_process(obj, pd);

   // don't check fds if somebody quit
   if (g_main_loop_is_running(ecore_main_loop))
     {
        // only set idling state in dispatch
        if (ecore_idling && (!_ecore_main_idlers_exist(pd)) &&
            (!pd->message_queue))
          {
             if (_efl_loop_timers_exists(obj, pd))
               {
                  int r = -1;
                  double t = _efl_loop_timer_next_get(obj, pd);

                  if ((pd->timer_fd >= 0) && (t > 0.0))
                    {
                       struct itimerspec ts;

                       ts.it_interval.tv_sec = 0;
                       ts.it_interval.tv_nsec = 0;
                       ts.it_value.tv_sec = t;
                       ts.it_value.tv_nsec = fmod(t * NS_PER_SEC, NS_PER_SEC);

                       // timerfd cannot sleep for 0 time
                       if (ts.it_value.tv_sec || ts.it_value.tv_nsec)
                         {
                            r = timerfd_settime(pd->timer_fd, 0, &ts, NULL);
                            if (r < 0)
                              {
                                 ERR("timer set returned %d (errno=%d)",
                                     r, errno);
                                 close(pd->timer_fd);
                                 pd->timer_fd = -1;
                              }
                            else INF("sleeping for %ld s %06ldus",
                                     ts.it_value.tv_sec,
                                     ts.it_value.tv_nsec / 1000);
                         }
                    }
                  if (r == -1)
                    {
                       *next_time = ceil(t * 1000.0);
                       if (t == 0.0) ready = TRUE;
                    }
               }
             else *next_time = -1;
          }
        else
          {
             *next_time = 0;
             if (pd->message_queue) ready = TRUE;
          }

        if (pd->fd_handlers_with_prep) _ecore_main_prepare_handlers(obj, pd);
     }
   else ready = TRUE;

   in_main_loop--;
   pd->in_loop = in_main_loop;
   DBG("leave, timeout = %d", *next_time);

   // ready if we're not running (about to quit)
   return ready;
}

static gboolean
_ecore_main_gsource_check(GSource *source EINA_UNUSED)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;
   gboolean ret = FALSE;

   in_main_loop++;
   pd->in_loop = in_main_loop;
   // check if old timers expired
   if (ecore_idling && (!_ecore_main_idlers_exist(pd)) &&
       (!pd->message_queue))
     {
        if (pd->timer_fd >= 0)
          {
             uint64_t count = 0;
             int r = read(pd->timer_fd, &count, sizeof count);
             if ((r == -1) && (errno == EAGAIN))
               {
               }
             else if (r == sizeof count) ret = TRUE;
             else
               {
                  // unexpected things happened... fail back to old way
                  ERR("timer read returned %d (errno=%d)", r, errno);
                  close(pd->timer_fd);
                  pd->timer_fd = -1;
               }
          }
     }
   else ret = TRUE;

   // check if fds are ready
#ifdef HAVE_SYS_EPOLL_H
   if (pd->epoll_fd >= 0)
     ecore_fds_ready = (_ecore_main_fdh_epoll_mark_active(obj, pd) > 0);
   else
#endif
     ecore_fds_ready = (_ecore_main_fdh_glib_mark_active(obj, pd) > 0);
   _ecore_main_fd_handlers_cleanup(obj, pd);
   if (ecore_fds_ready) ret = TRUE;

   // check timers after updating loop time
   if (!ret && _efl_loop_timers_exists(obj, pd))
     ret = (0.0 == _efl_loop_timer_next_get(obj, pd));

   in_main_loop--;
   pd->in_loop = in_main_loop;
   return ret;
}

// like we just came out of main_loop_select in  _ecore_main_select
static gboolean
_ecore_main_gsource_dispatch(GSource    *source EINA_UNUSED,
                             GSourceFunc callback EINA_UNUSED,
                             gpointer    user_data EINA_UNUSED)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;
   gboolean events_ready, timers_ready, idlers_ready;
   double next_time;

   pd->loop_time = ecore_time_get();
   _efl_loop_timer_enable_new(obj, pd);
   next_time = _efl_loop_timer_next_get(obj, pd);

   events_ready = pd->message_queue ? 1 : 0;
   timers_ready = _efl_loop_timers_exists(obj, pd) && (0.0 == next_time);
   idlers_ready = _ecore_main_idlers_exist(pd);

   in_main_loop++;
   pd->in_loop = in_main_loop;
   DBG("enter idling=%d fds=%d events=%d timers=%d (next=%.2f) idlers=%d",
       ecore_idling, ecore_fds_ready, events_ready,
       timers_ready, next_time, idlers_ready);

   if (ecore_idling && events_ready)
     {
        _ecore_animator_run_reset();
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_EXIT, NULL);
        ecore_idling = 0;
     }
   else if (!ecore_idling && !events_ready) ecore_idling = 1;

   if (ecore_idling)
     {
        _ecore_main_idler_all_call(obj);

        events_ready = pd->message_queue ? 1 : 0;

        if (ecore_fds_ready || events_ready || timers_ready)
          {
             _ecore_animator_run_reset();
             efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_EXIT, NULL);
             ecore_idling = 0;
          }
     }

   // process events
   if (!ecore_idling)
     {
        _ecore_main_fd_handlers_call(obj, pd);
        if (pd->fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call(obj, pd);
        _ecore_signal_received_process(obj, pd);
        efl_loop_message_process(obj);
        _ecore_main_fd_handlers_cleanup(obj, pd);

        _efl_loop_timer_expired_timers_call(obj, pd, pd->loop_time);

        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
        _ecore_glib_idle_enterer_called = TRUE;

        if (pd->fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call(obj, pd);
     }

   in_main_loop--;
   pd->in_loop = in_main_loop;

   return TRUE; // what should be returned here?
}

static void
_ecore_main_gsource_finalize(GSource *source EINA_UNUSED)
{
}

static GSourceFuncs ecore_gsource_funcs =
{
   .prepare  = _ecore_main_gsource_prepare,
   .check    = _ecore_main_gsource_check,
   .dispatch = _ecore_main_gsource_dispatch,
   .finalize = _ecore_main_gsource_finalize,
};
#endif

#ifdef HAVE_LIBUV
static inline void _ecore_main_loop_uv_check(uv_check_t *handle);
static void _ecore_main_loop_uv_prepare(uv_prepare_t *handle);

static void
_ecore_main_loop_timer_run(uv_timer_t *timer EINA_UNUSED)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;

   if (_ecore_main_uv_idling)
     {
        _ecore_main_uv_idling = EINA_FALSE;
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_EXIT, NULL);
        _ecore_animator_run_reset();
     }
   pd->loop_time = ecore_time_get();
   _ecore_main_loop_uv_check(NULL);
   _ecore_main_loop_uv_prepare(NULL);
}

static inline void
_ecore_main_loop_uv_check(uv_check_t *handle EINA_UNUSED)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;

   DBG("_ecore_main_loop_uv_check idling? %d", (int)_ecore_main_uv_idling);
   in_main_loop++;
   pd->in_loop = in_main_loop;

   if (pd->do_quit) goto quit;

   do
     {
        _ecore_main_fd_handlers_call(obj, pd);
        if (pd->fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call(obj, pd);
        _ecore_signal_received_process(obj, pd);
        efl_loop_message_process(obj);
        _ecore_main_fd_handlers_cleanup(obj, pd);
        _efl_loop_timer_expired_timers_call(obj, pd, pd->loop_time);
     }
   while (pd->fd_handlers_to_call);
quit:
   in_main_loop--;
   pd->in_loop = in_main_loop;
}
#endif

static void
_ecore_main_loop_setup(Eo *obj, Efl_Loop_Data *pd)
{
   // Please note that this function is being also called in case of a bad
   // fd to reset the main loop.
#ifdef HAVE_SYS_EPOLL_H
   pd->epoll_fd = epoll_create(1);
   if (pd->epoll_fd < 0) WRN("Failed to create epoll fd!");
   else
     {
        eina_file_close_on_exec(pd->epoll_fd, EINA_TRUE);

        pd->epoll_pid = getpid();

        // add polls on all our file descriptors
        Ecore_Fd_Handler *fdh;
        EINA_INLIST_FOREACH(pd->fd_handlers, fdh)
          {
             if (fdh->delete_me) continue;
             _ecore_epoll_add(pd->epoll_fd, fdh->fd,
                              _ecore_poll_events_from_fdh(fdh), fdh);
             _ecore_main_fdh_poll_add(pd, fdh);
          }
     }
#endif

   if (obj == ML_OBJ)
     {
#ifdef HAVE_LIBUV
        // XXX: FIXME: the below uv init should not assert but gracefully
        // fail with errors
        DBG("loading lib uv");
# ifdef HAVE_NODEJS
        void *lib = dlopen(NULL, RTLD_LAZY);
# else
        void *lib = dlopen("libuv.so.1", RTLD_GLOBAL | RTLD_LAZY);
# endif

        if (lib && dlsym(lib, "uv_run"))
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
        // else
        //   DBG("did not load uv");
        DBG("loaded dlsyms uv");
#endif

        // setup for the g_main_loop only integration
#ifdef USE_G_MAIN_LOOP
        ecore_glib_source = g_source_new(&ecore_gsource_funcs,
                                         sizeof(GSource));
        if (!ecore_glib_source) CRI("Failed to create glib source for epoll!");
        else
          {
             g_source_set_priority(ecore_glib_source,
                                   G_PRIORITY_HIGH_IDLE + 20);
# ifdef HAVE_SYS_EPOLL_H
             if (pd->epoll_fd >= 0)
               {
                  // epoll multiplexes fds into the g_main_loop
                  ecore_epoll_fd.fd = pd->epoll_fd;
                  ecore_epoll_fd.events = G_IO_IN;
                  ecore_epoll_fd.revents = 0;
                  g_source_add_poll(ecore_glib_source, &ecore_epoll_fd);
               }
# endif
             // timerfd gives us better than millisecond accuracy
             pd->timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
             if (pd->timer_fd < 0) WRN("failed to create timer fd!");
             else
               {
                  eina_file_close_on_exec(pd->timer_fd, EINA_TRUE);
                  ecore_timer_fd.fd = pd->timer_fd;
                  ecore_timer_fd.events = G_IO_IN;
                  ecore_timer_fd.revents = 0;
                  g_source_add_poll(ecore_glib_source, &ecore_timer_fd);
               }

             ecore_glib_source_id = g_source_attach(ecore_glib_source, NULL);
             if (ecore_glib_source_id <= 0)
               CRI("Failed to attach glib source to default context");
          }
#endif
     }
   _ecore_main_timechanges_start(obj);
}

static void
_ecore_main_loop_clear(Eo *obj, Efl_Loop_Data *pd)
{
   if (!pd) return;
   // Please note that _ecore_main_loop_shutdown is called in cycle to
   // restart the main loop in case of a bad fd
   if (obj == ML_OBJ)
     {
        _ecore_main_timechanges_stop(obj);
#ifdef USE_G_MAIN_LOOP
        if (ecore_glib_source)
          {
             g_source_destroy(ecore_glib_source);
             ecore_glib_source = NULL;
          }
#endif
#ifdef HAVE_LIBUV
        if (_dl_uv_run)
          {
             DBG("_ecore_main_loop_shutdown");
             _dl_uv_timer_stop(&_ecore_main_uv_handle_timers);
             _dl_uv_close((uv_handle_t*)&_ecore_main_uv_handle_timers, 0);
          }
#endif
     }
# ifdef HAVE_SYS_EPOLL_H
   if (pd->epoll_fd >= 0)
     {
        close(pd->epoll_fd);
        pd->epoll_fd = -1;
     }
#endif
   if (pd->timer_fd >= 0)
     {
        close(pd->timer_fd);
        pd->timer_fd = -1;
     }
}

void
_ecore_main_loop_init(void)
{
   DBG("_ecore_main_loop_init");
   if (!efl_main_loop_get()) ERR("Cannot create main loop object");
   _ecore_main_loop_setup(ML_OBJ, ML_DAT);
}

void
_ecore_main_loop_shutdown(void)
{
   if (!ML_OBJ) return;
   _ecore_main_loop_clear(ML_OBJ, ML_DAT);
// XXX: this seemingly closes fd's it shouldn't.... :( fd 0?
   efl_replace(&ML_OBJ, NULL);
   ML_DAT = NULL;
}

void
_ecore_main_loop_iterate(Eo *obj, Efl_Loop_Data *pd)
{
   if (obj == ML_OBJ)
     {
#ifdef HAVE_LIBUV
        if (!_dl_uv_run)
          {
#endif
#ifndef USE_G_MAIN_LOOP
             pd->loop_time = ecore_time_get();
             _ecore_main_loop_iterate_internal(obj, pd, 1);
#else
             g_main_context_iteration(NULL, 0);
#endif
#ifdef HAVE_LIBUV
          }
        else
          _dl_uv_run(_dl_uv_default_loop(), UV_RUN_ONCE | UV_RUN_NOWAIT);
#endif
     }
   else
     {
#ifndef USE_G_MAIN_LOOP
        pd->loop_time = ecore_time_get();
        _ecore_main_loop_iterate_internal(obj, pd, 1);
#else
             g_main_context_iteration(NULL, 0);
#endif
     }
}

int
_ecore_main_loop_iterate_may_block(Eo *obj, Efl_Loop_Data *pd, int may_block)
{
   if (obj == ML_OBJ)
     {
#ifdef HAVE_LIBUV
        if (!_dl_uv_run)
          {
#endif
#ifndef USE_G_MAIN_LOOP
             in_main_loop++;
             pd->in_loop = in_main_loop;
             pd->loop_time = ecore_time_get();
             _ecore_main_loop_iterate_internal(obj, pd, !may_block);
             in_main_loop--;
             pd->in_loop = in_main_loop;
             return pd->message_queue ? 1 : 0;
#else
             return g_main_context_iteration(NULL, may_block);
#endif
#ifdef HAVE_LIBUV
          }
        else
          _dl_uv_run(_dl_uv_default_loop(),
                     may_block ? (UV_RUN_ONCE | UV_RUN_NOWAIT) : UV_RUN_ONCE);
#endif
     }
   else
     {
#ifndef USE_G_MAIN_LOOP
        pd->in_loop++;
        pd->loop_time = ecore_time_get();
        _ecore_main_loop_iterate_internal(obj, pd, !may_block);
        pd->in_loop--;
        return pd->message_queue ? 1 : 0;
#else
        return g_main_context_iteration(NULL, may_block);
#endif
     }
   return 0;
}

void
_ecore_main_loop_begin(Eo *obj, Efl_Loop_Data *pd)
{
   if (obj == ML_OBJ)
     {
#ifdef HAVE_SYSTEMD
        sd_notify(0, "READY=1");
#endif
#ifdef HAVE_LIBUV
        if (!_dl_uv_run)
          {
#endif
#ifndef USE_G_MAIN_LOOP
             in_main_loop++;
             pd->in_loop = in_main_loop;
             pd->loop_time = ecore_time_get();
             while (!pd->do_quit)
               _ecore_main_loop_iterate_internal(obj, pd, 0);
             pd->do_quit = 0;
             in_main_loop--;
             pd->in_loop = in_main_loop;
#else
             if (!pd->do_quit)
               {
                  if (!ecore_main_loop)
                    ecore_main_loop = g_main_loop_new(NULL, FALSE);
                  g_main_loop_run(ecore_main_loop);
               }
             pd->do_quit = 0;
#endif
#ifdef HAVE_LIBUV
          }
        else
          {
             DBG("uv_run");
             in_main_loop++;
             pd->in_loop = in_main_loop;
             pd->loop_time = ecore_time_get();
             while (!pd->do_quit)
               _dl_uv_run(_dl_uv_default_loop(), UV_RUN_DEFAULT);
             in_main_loop--;
             pd->in_loop = in_main_loop;
             pd->do_quit = 0;
             DBG("quit");
          }
#endif
     }
   else
     {
#ifndef USE_G_MAIN_LOOP
        pd->in_loop++;
        pd->loop_time = ecore_time_get();
        while (!pd->do_quit)
          _ecore_main_loop_iterate_internal(obj, pd, 0);
        pd->do_quit = 0;
        pd->in_loop--;
#else
        if (!pd->do_quit)
          {
             if (!ecore_main_loop)
               ecore_main_loop = g_main_loop_new(NULL, 1);
             g_main_loop_run(ecore_main_loop);
          }
        pd->do_quit = 0;
#endif
     }
}

void
_ecore_main_loop_quit(Eo *obj, Efl_Loop_Data *pd)
{
   pd->do_quit = 1;
   if (obj != ML_OBJ) return;
#ifdef USE_G_MAIN_LOOP
   if (ecore_main_loop) g_main_loop_quit(ecore_main_loop);
#elif defined(HAVE_LIBUV)
   if (_dl_uv_run) _dl_uv_stop(_dl_uv_default_loop());
#endif
}

EAPI void
ecore_main_loop_iterate(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   efl_loop_iterate(ML_OBJ);
}

EAPI int
ecore_main_loop_iterate_may_block(int may_block)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return efl_loop_iterate_may_block(ML_OBJ, may_block);
}

EAPI void
ecore_main_loop_begin(void)
{
   DBG("ecore_main_loop_begin");
   EINA_MAIN_LOOP_CHECK_RETURN;
   eina_evlog("+mainloop", NULL, 0.0, NULL);
   efl_loop_begin(ML_OBJ);
   eina_evlog("-mainloop", NULL, 0.0, NULL);
}

EAPI void
ecore_main_loop_quit(void)
{
   Eina_Value v = EINA_VALUE_EMPTY;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);
   eina_value_set(&v, 0);
   EINA_MAIN_LOOP_CHECK_RETURN;
   efl_loop_quit(ML_OBJ, v);
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
_ecore_main_fd_handler_add(Eo                    *obj,
                           Efl_Loop_Data         *pd,
                           Eo                    *handler,
                           int                    fd,
                           Ecore_Fd_Handler_Flags flags,
                           Ecore_Fd_Cb            func,
                           const void            *data,
                           Ecore_Fd_Cb            buf_func,
                           const void            *buf_data,
                           Eina_Bool              is_file)
{
   DBG("_ecore_main_fd_handler_add");
   Ecore_Fd_Handler *fdh = NULL;

   if ((fd < 0) || (flags == 0) || (!func)) return NULL;

   fdh = ecore_fd_handler_calloc(1);
   if (!fdh) return NULL;
   ECORE_MAGIC_SET(fdh, ECORE_MAGIC_FD_HANDLER);
   fdh->loop = obj;
   fdh->loop_data = pd;
   fdh->handler = handler;
   fdh->fd = fd;
   fdh->flags = flags;
   fdh->file = is_file;
   if (_ecore_main_fdh_poll_add(pd, fdh) < 0)
     {
        int err = errno;
        ERR("Failed to add poll on fd %d (errno = %d: %s)!",
            fd, err, strerror(err));
        ecore_fd_handler_mp_free(fdh);
        return NULL;
     }
   fdh->func = func;
   fdh->data = (void *)data;
   fdh->buf_func = buf_func;
   if (buf_func)
     pd->fd_handlers_with_buffer = eina_list_append
       (pd->fd_handlers_with_buffer, fdh);
   fdh->buf_data = (void *)buf_data;
   if (is_file)
     pd->file_fd_handlers = eina_list_append
       (pd->file_fd_handlers, fdh);
   pd->fd_handlers = (Ecore_Fd_Handler *)
     eina_inlist_append(EINA_INLIST_GET(pd->fd_handlers),
                        EINA_INLIST_GET(fdh));
   return fdh;
}

void *
_ecore_main_fd_handler_del(Eo *obj EINA_UNUSED,
                           Efl_Loop_Data *pd,
                           Ecore_Fd_Handler *fd_handler)
{
   void *r = fd_handler->data;

   DBG("_ecore_main_fd_handler_del %p", fd_handler);
   if (fd_handler->delete_me)
     {
        ERR("fdh %p deleted twice", fd_handler);
        return NULL;
     }

   fd_handler->handler = NULL;
   fd_handler->delete_me = EINA_TRUE;
   if (pd)
     {
        _ecore_main_fdh_poll_del(pd, fd_handler);
        pd->fd_handlers_to_delete = eina_list_append
          (pd->fd_handlers_to_delete, fd_handler);
        if (fd_handler->prep_func && pd->fd_handlers_with_prep)
          pd->fd_handlers_with_prep = eina_list_remove
            (pd->fd_handlers_with_prep, fd_handler);
        if (fd_handler->buf_func && pd->fd_handlers_with_buffer)
          pd->fd_handlers_with_buffer = eina_list_remove
            (pd->fd_handlers_with_buffer, fd_handler);
     }
   else
     {
        // The main loop is dead by now, so cleanup is required.
        ECORE_MAGIC_SET(fd_handler, ECORE_MAGIC_NONE);
        ecore_fd_handler_mp_free(fd_handler);
     }
   return r;
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
   fdh = _ecore_main_fd_handler_add(efl_main_loop_get(),
                                    ML_DAT, NULL, fd, flags, func, data,
                                    buf_func, buf_data, EINA_FALSE);
   if (fdh) fdh->legacy = EINA_TRUE;
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
   return _ecore_main_fd_handler_add(efl_main_loop_get(),
                                     ML_DAT, NULL, fd, flags, func, data,
                                     buf_func, buf_data, EINA_TRUE);
}

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
   return _ecore_main_fd_handler_del(ML_OBJ, ML_DAT, fd_handler);
}

#ifdef _WIN32
EAPI Ecore_Win32_Handler *
_ecore_main_win32_handler_add(Eo                    *obj,
                              Efl_Loop_Data         *pd,
                              Eo                    *handler,
                              void                  *h,
                              Ecore_Win32_Handle_Cb  func,
                              const void            *data)
{
   Ecore_Win32_Handler *wh;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!h || !func) return NULL;

   wh = ecore_win32_handler_calloc(1);
   if (!wh) return NULL;
   ECORE_MAGIC_SET(wh, ECORE_MAGIC_WIN32_HANDLER);
   wh->loop = obj;
   wh->loop_data = pd;
   wh->handler = handler;
   wh->h = (HANDLE)h;
   wh->func = func;
   wh->data = (void *)data;
   pd->win32_handlers = (Ecore_Win32_Handler *)
     eina_inlist_append(EINA_INLIST_GET(pd->win32_handlers),
                        EINA_INLIST_GET(wh));
   return wh;
}

void *
_ecore_main_win32_handler_del(Eo *obj EINA_UNUSED,
                              Efl_Loop_Data *pd,
                              Ecore_Win32_Handler *win32_handler)
{
   if (win32_handler->delete_me)
     {
        ERR("win32 handler %p deleted twice", win32_handler);
        return NULL;
     }

   win32_handler->delete_me = EINA_TRUE;
   win32_handler->handler = NULL;
   pd->win32_handlers_to_delete = eina_list_append
     (pd->win32_handlers_to_delete, win32_handler);
   return win32_handler->data;
}

EAPI Ecore_Win32_Handler *
ecore_main_win32_handler_add(void                  *h,
                             Ecore_Win32_Handle_Cb  func,
                             const void            *data)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   return _ecore_main_win32_handler_add(ML_OBJ, ML_DAT, NULL, h, func, data);
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
   ret = _ecore_main_win32_handler_del(ML_OBJ, ML_DAT, win32_handler);
   return ret;
}
#else
EAPI Ecore_Win32_Handler *
_ecore_main_win32_handler_add(Eo                    *obj EINA_UNUSED,
                              Efl_Loop_Data         *pd EINA_UNUSED,
                              Eo                    *handler EINA_UNUSED,
                              void                  *h EINA_UNUSED,
                              Ecore_Win32_Handle_Cb  func EINA_UNUSED,
                              const void            *data EINA_UNUSED)
{
   return NULL;
}

void *
_ecore_main_win32_handler_del(Eo *obj EINA_UNUSED,
                              Efl_Loop_Data *pd EINA_UNUSED,
                              Ecore_Win32_Handler *win32_handler EINA_UNUSED)
{
   return NULL;
}

EAPI Ecore_Win32_Handler *
ecore_main_win32_handler_add(void                  *h EINA_UNUSED,
                             Ecore_Win32_Handle_Cb  func EINA_UNUSED,
                             const void            *data EINA_UNUSED)
{
   return NULL;
}

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
   if (!fd_handler) return;
   Efl_Loop_Data *pd = fd_handler->loop_data;
   EINA_MAIN_LOOP_CHECK_RETURN;

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_prepare_callback_set");
        return;
     }
   fd_handler->prep_func = func;
   fd_handler->prep_data = (void *)data;
   if ((!pd->fd_handlers_with_prep) ||
       (pd->fd_handlers_with_prep &&
        (!eina_list_data_find(pd->fd_handlers_with_prep, fd_handler))))
     // FIXME: THIS WILL NOT SCALE WITH LOTS OF PREP FUNCTIONS!!!
     pd->fd_handlers_with_prep = eina_list_append
       (pd->fd_handlers_with_prep, fd_handler);
}

EAPI int
ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler)
{
   if (!fd_handler) return -1;

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
   if ((flags & ECORE_FD_READ) &&  (fd_handler->read_active))  ret = EINA_TRUE;
   if ((flags & ECORE_FD_WRITE) && (fd_handler->write_active)) ret = EINA_TRUE;
   if ((flags & ECORE_FD_ERROR) && (fd_handler->error_active)) ret = EINA_TRUE;
   return ret;
}

EAPI void
ecore_main_fd_handler_active_set(Ecore_Fd_Handler      *fd_handler,
                                 Ecore_Fd_Handler_Flags flags)
{
   int ret = -1;

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_active_set");
        return;
     }
   fd_handler->flags = flags;
   if (fd_handler->loop_data)
     ret = _ecore_main_fdh_poll_modify(fd_handler->loop_data, fd_handler);
   if (ret < 0)
     ERR("Failed to mod epoll fd %d, loop data=%p: %s!",
         fd_handler->fd, fd_handler->loop_data, strerror(errno));
}

void
_ecore_main_content_clear(Eo *obj, Efl_Loop_Data *pd)
{
   __eina_promise_cancel_data(obj);

   while (pd->fd_handlers)
     {
        Ecore_Fd_Handler *fdh = pd->fd_handlers;

        pd->fd_handlers = (Ecore_Fd_Handler *)
          eina_inlist_remove(EINA_INLIST_GET(pd->fd_handlers),
                             EINA_INLIST_GET(fdh));
        if ((fdh->handler) && (fdh->legacy)) efl_del(fdh->handler);
        else
          {
// XXX: can't do this because this fd handler is legacy and might
// be cleaned up later in object destructors
//             ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
//             ecore_fd_handler_mp_free(fdh);
          }
     }
   if (pd->fd_handlers_with_buffer)
     pd->fd_handlers_with_buffer =
       eina_list_free(pd->fd_handlers_with_buffer);
   if (pd->fd_handlers_with_prep)
     pd->fd_handlers_with_prep =
       eina_list_free(pd->fd_handlers_with_prep);
   if (pd->file_fd_handlers)
     pd->file_fd_handlers =
       eina_list_free(pd->file_fd_handlers);
   if (pd->fd_handlers_to_delete)
     pd->fd_handlers_to_delete =
       eina_list_free(pd->fd_handlers_to_delete);
   pd->fd_handlers_to_call = NULL;
   pd->fd_handlers_to_call_current = NULL;

   pd->do_quit = 0;

#ifdef _WIN32
   while (pd->win32_handlers)
     {
        Ecore_Win32_Handler *wh = pd->win32_handlers;

        pd->win32_handlers = (Ecore_Win32_Handler *)
          eina_inlist_remove(EINA_INLIST_GET(pd->win32_handlers),
                             EINA_INLIST_GET(wh));
        if (wh->handler) efl_del(wh->handler);
        else
          {
             ECORE_MAGIC_SET(wh, ECORE_MAGIC_NONE);
             ecore_win32_handler_mp_free(wh);
          }
     }
   if (pd->win32_handlers_to_delete)
     pd->win32_handlers_to_delete =
       eina_list_free(pd->win32_handlers_to_delete);
   pd->win32_handler_current = NULL;
#endif
}

void
_ecore_main_shutdown(void)
{
   Efl_Loop_Data *pd = ML_DAT;

   if (pd->in_loop)
     {
        ERR("Calling ecore_shutdown() while still in the main loop!!!");
        return;
     }
}

static void
_ecore_main_prepare_handlers(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd)
{
   Ecore_Fd_Handler *fdh;
   Eina_List *l, *l2;

   // call the prepare callback for all handlers with prep functions
   EINA_LIST_FOREACH_SAFE(pd->fd_handlers_with_prep, l, l2, fdh)
     {
        if (!fdh)
          {
             pd->fd_handlers_with_prep = eina_list_remove_list
               (l, pd->fd_handlers_with_prep);
             continue;
          }
        if (!fdh->delete_me && fdh->prep_func)
          {
             fdh->references++;
             _ecore_call_prep_cb(fdh->prep_func, fdh->prep_data, fdh);
             fdh->references--;
          }
        else
          pd->fd_handlers_with_prep = eina_list_remove_list
            (pd->fd_handlers_with_prep, l);
     }
}

#if !defined(USE_G_MAIN_LOOP)
static int
_ecore_main_select(Eo *obj, Efl_Loop_Data *pd, double timeout)
{
   struct timeval tv, *t;
   fd_set rfds, wfds, exfds;
   Ecore_Fd_Handler *fdh;
   Eina_List *l;
   int max_fd, ret;
#ifndef _WIN32
   int err_no;
#endif

   t = NULL;
   if ((!ECORE_FINITE(timeout)) || (EINA_DBL_EQ(timeout, 0.0)))
     { // finite() tests for NaN, too big, too small, and infinity.
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

   // call the prepare callback for all handlers
   if (pd->fd_handlers_with_prep) _ecore_main_prepare_handlers(obj, pd);

#ifdef HAVE_SYS_EPOLL_H
   if (pd->epoll_fd < 0)
     {
#endif
        EINA_INLIST_FOREACH(pd->fd_handlers, fdh)
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
#ifdef HAVE_SYS_EPOLL_H
     }
   else
     {
        // polling on the epoll fd will wake when fd in the epoll set is active
        max_fd = _ecore_get_epoll_fd(obj, pd);
        FD_SET(max_fd, &rfds);
     }
#endif
   EINA_LIST_FOREACH(pd->file_fd_handlers, l, fdh)
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
             if (fdh->fd > max_fd) max_fd = fdh->fd;
          }
     }
   if (_ecore_signal_count_get(obj, pd)) return -1;

   eina_evlog("<RUN", NULL, 0.0, NULL);
   eina_evlog("!SLEEP", NULL, 0.0, t ? "timeout" : "forever");
   if (obj == ML_OBJ)
     ret = main_loop_select(max_fd + 1, &rfds, &wfds, &exfds, t);
   else
     ret = general_loop_select(max_fd + 1, &rfds, &wfds, &exfds, t);
#ifndef _WIN32
   err_no = errno;
#endif
   eina_evlog("!WAKE", NULL, 0.0, NULL);
   eina_evlog(">RUN", NULL, 0.0, NULL);

   pd->loop_time = ecore_time_get();
   if (ret < 0)
     {
#ifndef _WIN32
        if (err_no == EINTR) return -1;
        else if (err_no == EBADF) _ecore_main_fd_handlers_bads_rem(obj, pd);
#endif
     }
   if (ret > 0)
     {
#ifdef HAVE_SYS_EPOLL_H
        if (pd->epoll_fd >= 0)
          _ecore_main_fdh_epoll_mark_active(obj, pd);
        else
#endif
          {
             EINA_INLIST_FOREACH(pd->fd_handlers, fdh)
               {
                  if (!fdh->delete_me)
                    {
                       if (FD_ISSET(fdh->fd, &rfds))
                         fdh->read_active  = EINA_TRUE;
                       if (FD_ISSET(fdh->fd, &wfds))
                         fdh->write_active = EINA_TRUE;
                       if (FD_ISSET(fdh->fd, &exfds))
                         fdh->error_active = EINA_TRUE;
                       _ecore_try_add_to_call_list(obj, pd, fdh);
                    }
               }
          }
        EINA_LIST_FOREACH(pd->file_fd_handlers, l, fdh)
          {
             if (!fdh->delete_me)
               {
                  if (FD_ISSET(fdh->fd, &rfds))
                    fdh->read_active  = EINA_TRUE;
                  if (FD_ISSET(fdh->fd, &wfds))
                    fdh->write_active = EINA_TRUE;
                  if (FD_ISSET(fdh->fd, &exfds))
                    fdh->error_active = EINA_TRUE;
                  _ecore_try_add_to_call_list(obj, pd, fdh);
               }
          }
        _ecore_main_fd_handlers_cleanup(obj, pd);
#ifdef _WIN32
        _ecore_main_win32_handlers_cleanup(obj, pd);
#endif
        return 1;
     }
   return 0;
}

#endif

#ifndef _WIN32
# ifndef USE_G_MAIN_LOOP
static void
_ecore_main_fd_handlers_bads_rem(Eo *obj, Efl_Loop_Data *pd)
{
   Ecore_Fd_Handler *fdh;
   Eina_Inlist *l;
   int found = 0;

   ERR("Removing bad fds");
   for (l = EINA_INLIST_GET(pd->fd_handlers); l; )
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
                            _ecore_main_fdh_poll_del(pd, fdh);
                            pd->fd_handlers_to_delete =
                              eina_list_append(pd->fd_handlers_to_delete, fdh);
                         }
                       found++;
                    }
                  fdh->references--;
               }
             else
               {
                  ERR("Problematic fd found at %d! setting it for delete",
                      fdh->fd);
                  if (!fdh->delete_me)
                    {
                       fdh->delete_me = EINA_TRUE;
                       _ecore_main_fdh_poll_del(pd, fdh);
                       pd->fd_handlers_to_delete =
                         eina_list_append(pd->fd_handlers_to_delete, fdh);
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
   _ecore_main_fd_handlers_cleanup(obj, pd);
}

# endif
#endif

static void
_ecore_main_fd_handlers_cleanup(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd)
{
   Ecore_Fd_Handler *fdh, *last;
   Eina_List *l, *l2;

   // Cleanup deleted caller from the list
   last = NULL;
   fdh = pd->fd_handlers_to_call;
   while (fdh)
     {
        if (fdh->delete_me)
          {
             if (!last)
               pd->fd_handlers_to_call = fdh == fdh->next_ready ?
                 NULL : fdh->next_ready;
             else
               last->next_ready = fdh == fdh->next_ready ?
                 last : fdh->next_ready;
          }
        else last = fdh;

        if (fdh == fdh->next_ready) break;
        fdh = fdh->next_ready;
     }

   if (!pd->fd_handlers_to_delete) return;
   EINA_LIST_FOREACH_SAFE(pd->fd_handlers_to_delete, l, l2, fdh)
     {
        if (!fdh)
          {
             pd->fd_handlers_to_delete = eina_list_remove_list
               (l, pd->fd_handlers_to_delete);
             continue;
          }
        if (fdh->references) continue;
        if (pd->fd_handlers_to_call_current == fdh)
          pd->fd_handlers_to_call_current = NULL;
        if (fdh->buf_func && pd->fd_handlers_with_buffer)
          pd->fd_handlers_with_buffer = eina_list_remove
            (pd->fd_handlers_with_buffer, fdh);
        if (fdh->prep_func && pd->fd_handlers_with_prep)
          pd->fd_handlers_with_prep = eina_list_remove
           (pd->fd_handlers_with_prep, fdh);
        pd->fd_handlers = (Ecore_Fd_Handler *)
          eina_inlist_remove(EINA_INLIST_GET(pd->fd_handlers),
                             EINA_INLIST_GET(fdh));
        if (fdh->file)
          pd->file_fd_handlers = eina_list_remove(pd->file_fd_handlers, fdh);
        ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
        ecore_fd_handler_mp_free(fdh);
        pd->fd_handlers_to_delete = eina_list_remove_list
          (pd->fd_handlers_to_delete, l);
     }
}

#ifdef _WIN32
static void
_ecore_main_win32_handlers_cleanup(Eo *obj EINA_UNUSED, Efl_Loop_Data *pd)
{
   Ecore_Win32_Handler *wh;
   Eina_List *l, *l2;

   if (!pd->win32_handlers_to_delete) return;
   EINA_LIST_FOREACH_SAFE(pd->win32_handlers_to_delete, l, l2, wh)
     {
        if (!wh)
          {
             pd->win32_handlers_to_delete = eina_list_remove_list
               (l, pd->win32_handlers_to_delete);
             continue;
          }
        // wh->delete_me should be set for all whs at the start of the list
        if (wh->references) continue;
        pd->win32_handlers = (Ecore_Win32_Handler *)
          eina_inlist_remove(EINA_INLIST_GET(pd->win32_handlers),
                             EINA_INLIST_GET(wh));
        ECORE_MAGIC_SET(wh, ECORE_MAGIC_NONE);
        ecore_win32_handler_mp_free(wh);
        pd->win32_handlers_to_delete = eina_list_remove_list
          (pd->win32_handlers_to_delete, l);
     }
}
#endif

static void
_ecore_main_fd_handlers_call(Eo *obj, Efl_Loop_Data *pd)
{
   // grab a new list
   if (!pd->fd_handlers_to_call_current)
     {
        pd->fd_handlers_to_call_current = pd->fd_handlers_to_call;
        pd->fd_handlers_to_call = NULL;
     }

   if (!pd->fd_handlers_to_call_current) return;
   eina_evlog("+fd_handlers", NULL, 0.0, NULL);

   while (pd->fd_handlers_to_call_current)
     {
        Ecore_Fd_Handler *fdh = pd->fd_handlers_to_call_current;

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
                            _ecore_main_fdh_poll_del(pd, fdh);
                            pd->fd_handlers_to_delete = eina_list_append
                              (pd->fd_handlers_to_delete, fdh);
                         }
                    }
                  fdh->references--;
#ifdef EFL_EXTRA_SANITY_CHECKS
                  _ecore_fd_valid(obj, pd);
#endif
                  fdh->read_active  = EINA_FALSE;
                  fdh->write_active = EINA_FALSE;
                  fdh->error_active = EINA_FALSE;
               }
          }

        // stop when we point to ourselves
        if (fdh->next_ready == fdh)
          {
             fdh->next_ready = NULL;
             pd->fd_handlers_to_call_current = NULL;
             break;
          }

        pd->fd_handlers_to_call_current = fdh->next_ready;
        fdh->next_ready = NULL;
      }
   eina_evlog("-fd_handlers", NULL, 0.0, NULL);
}

static int
_ecore_main_fd_handlers_buf_call(Eo *obj, Efl_Loop_Data *pd)
{
   Ecore_Fd_Handler *fdh;
   Eina_List *l, *l2;
   int ret;

   if (!pd->fd_handlers_with_buffer) return 0;
   eina_evlog("+fd_handlers_buf", NULL, 0.0, NULL);
   ret = 0;
   EINA_LIST_FOREACH_SAFE(pd->fd_handlers_with_buffer, l, l2, fdh)
     {
        if (!fdh)
          {
             pd->fd_handlers_with_buffer = eina_list_remove_list
               (l, pd->fd_handlers_with_buffer);
             continue;
          }
        if ((!fdh->delete_me) && fdh->buf_func)
          {
             fdh->references++;
             if (_ecore_call_fd_cb(fdh->buf_func, fdh->buf_data, fdh))
               {
                  ret |= _ecore_call_fd_cb(fdh->func, fdh->data, fdh);
                  fdh->read_active = EINA_TRUE;
                  _ecore_try_add_to_call_list(obj, pd, fdh);
               }
             fdh->references--;
          }
        else
          pd->fd_handlers_with_buffer = eina_list_remove_list
            (pd->fd_handlers_with_buffer, l);
     }
   eina_evlog("-fd_handlers_buf", NULL, 0.0, NULL);
   return ret;
}

#ifdef HAVE_LIBUV
static void
_ecore_main_loop_uv_prepare(uv_prepare_t *handle EINA_UNUSED)
{
   Eo *obj = ML_OBJ;
   Efl_Loop_Data *pd = ML_DAT;
   double t = -1;

   _dl_uv_timer_stop(&_ecore_main_uv_handle_timers);
   if ((pd->in_loop == 0) && (pd->do_quit))
     {
        _ecore_main_fd_handlers_cleanup(obj, pd);

        while (pd->fd_handlers)
          {
             Ecore_Fd_Handler *fdh = pd->fd_handlers;
             pd->fd_handlers = (Ecore_Fd_Handler *)
               eina_inlist_remove(EINA_INLIST_GET(pd->fd_handlers),
                                  EINA_INLIST_GET(fdh));
             fdh->delete_me = 1;
             _ecore_main_fdh_poll_del(pd, fdh);
             ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
             ecore_fd_handler_mp_free(fdh);
          }
       if (pd->fd_handlers_with_buffer)
         pd->fd_handlers_with_buffer = eina_list_free(pd->fd_handlers_with_buffer);
       if (pd->fd_handlers_with_prep)
         pd->fd_handlers_with_prep = eina_list_free(pd->fd_handlers_with_prep);
       if (pd->fd_handlers_to_delete)
         pd->fd_handlers_to_delete = eina_list_free(pd->fd_handlers_to_delete);
       if (pd->file_fd_handlers)
         pd->file_fd_handlers = eina_list_free(pd->file_fd_handlers);

        pd->fd_handlers_to_call = NULL;
        pd->fd_handlers_to_call_current = NULL;

        _dl_uv_prepare_stop(&_ecore_main_uv_prepare);
        _dl_uv_check_stop(&_ecore_main_uv_check);
        _dl_uv_stop(_dl_uv_default_loop());

        return;
     }

   in_main_loop++;
   pd->in_loop = in_main_loop;

   if (!_ecore_main_uv_idling)
     {
        _ecore_main_uv_idling = EINA_TRUE;
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
     }

   if (_ecore_main_uv_idling)
     {
        _ecore_main_idler_all_call(obj);
        DBG("called idles");
        if (_ecore_main_idlers_exist(pd) || (pd->message_queue)) t = 0.0;
     }

   if (pd->do_quit)
     {
        DBG("do quit outside loop");

        if (_ecore_main_uv_idling)
          {
             efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_EXIT, NULL);
             _ecore_animator_run_reset();
             _ecore_main_uv_idling = EINA_FALSE;
          }
        t = -1;
        pd->loop_time = ecore_time_get();
        _efl_loop_timer_enable_new(obj, pd);
        goto done;
     }

   assert(!pd->fd_handlers_to_call);

   pd->loop_time = ecore_time_get();
   _efl_loop_timer_enable_new(obj, pd);
   if (_efl_loop_timers_exists(obj, pd) || (t >= 0))
     {
        double tnext = _efl_loop_timer_next_get(obj, pd);

        if ((t < 0) || ((tnext >= 0) && (tnext < t))) t = tnext;
        DBG("Should awake after %f", t);

        if (t >= 0.0)
          {
             // _dl_uv_timer_stop(&_ecore_main_uv_handle_timers);
             _dl_uv_timer_start(&_ecore_main_uv_handle_timers,
                                &_ecore_main_loop_timer_run,
                                t * 1000, 0);
          }
        else DBG("Is not going to awake with timer");
     }
   else DBG("Is not going to awake with timer");

done:
   if (pd->fd_handlers_with_prep) _ecore_main_prepare_handlers(obj, pd);
   in_main_loop--;
   pd->in_loop = in_main_loop;
}
#endif

#if !defined(USE_G_MAIN_LOOP)
enum {
   SPIN_MORE,
   SPIN_RESTART,
   LOOP_CONTINUE
};

static int
_ecore_main_loop_spin_core(Eo *obj, Efl_Loop_Data *pd)
{
   // as we are spinning we need to update loop time per spin
   pd->loop_time = ecore_time_get();
   // call all idlers
   _ecore_main_idler_all_call(obj);
   // which returns false if no more idelrs exist
   if (!_ecore_main_idlers_exist(pd)) return SPIN_RESTART;
   // sneaky - drop through or if checks - the first one to succeed
   // drops through and returns "continue" so further ones dont run
   if ((_ecore_main_select(obj, pd, 0.0) > 0) || (pd->message_queue) ||
       (_ecore_signal_count_get(obj, pd) > 0) || (pd->do_quit))
     return LOOP_CONTINUE;
   // default - spin more
   return SPIN_MORE;
}

static int
_ecore_main_loop_spin_no_timers(Eo *obj, Efl_Loop_Data *pd)
{
   // if we have idlers we HAVE to spin and handle everything
   // in a polling way - spin in a tight polling loop
   for (;;)
     {
        int action = _ecore_main_loop_spin_core(obj, pd);
        if (action != SPIN_MORE) return action;
        // if an idler has added a timer then we need to go through
        // the start of the spin cycle again to handle cases properly
        if (_efl_loop_timers_exists(obj, pd)) return SPIN_RESTART;
     }
   // just contiune handling events etc.
   return LOOP_CONTINUE;
}

static int
_ecore_main_loop_spin_timers(Eo *obj, Efl_Loop_Data *pd)
{
   // if we have idlers we HAVE to spin and handle everything
   // in a polling way - spin in a tight polling loop
   for (;;)
     {
        int action = _ecore_main_loop_spin_core(obj, pd);
        if (action != SPIN_MORE) return action;
        // if next timer expires now or in the past - stop spinning and
        // continue the mainloop walk as our "select" timeout has
        // expired now
        if (_efl_loop_timer_next_get(obj, pd) <= 0.0) return LOOP_CONTINUE;
     }
   // just contiune handling events etc.
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
_ecore_main_loop_iterate_internal(Eo *obj, Efl_Loop_Data *pd, int once_only)
{
   double next_time = -1.0;

   if (obj == ML_OBJ)
     {
        in_main_loop++;
        pd->in_loop = in_main_loop;
     }
   // expire any timers
   _efl_loop_timer_expired_timers_call(obj, pd, pd->loop_time);
   // process signals into events ....
   if (obj == ML_OBJ) _ecore_signal_received_process(obj, pd);
   // if as a result of timers/animators or signals we have accumulated
   // events, then instantly handle them
   if (pd->message_queue)
     {
        // but first conceptually enter an idle state
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
        // now quickly poll to see which input fd's are active
        _ecore_main_select(obj, pd, 0.0);
        // allow newly queued timers to expire from now on
        _efl_loop_timer_enable_new(obj, pd);
        // go straight to processing the events we had queued
        goto process_all;
     }

   if (once_only)
     {
        // in once_only mode we should quickly poll for inputs, signals
        // if we got any events or signals, allow new timers to process.
        // use bitwise or to force both conditions to be tested and
        // merged together
        if (_ecore_main_select(obj, pd, 0.0) |
            _ecore_signal_count_get(obj, pd))
          {
             _efl_loop_timer_enable_new(obj, pd);
             goto process_all;
          }
     }
   else
     {
        // call idle enterers ...
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
     }

   // if these calls caused any buffered events to appear - deal with them
   if (pd->fd_handlers_with_buffer)
     _ecore_main_fd_handlers_buf_call(obj, pd);

   // if there are any (buffered fd handling may generate them)
   // then jump to processing them */
   if (pd->message_queue)
     {
        _ecore_main_select(obj, pd, 0.0);
        _efl_loop_timer_enable_new(obj, pd);
        goto process_all;
     }

   if (once_only)
     {
        // in once_only mode enter idle here instead and then return
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
        _efl_loop_timer_enable_new(obj, pd);
        goto done;
     }

   if (obj == ML_OBJ) _ecore_fps_marker_1();

   // start of the sleeping or looping section
start_loop: //-*************************************************************
   // any timers re-added as a result of these are allowed to go
   _efl_loop_timer_enable_new(obj, pd);
   // if we have been asked to quit the mainloop then exit at this point
   if (pd->do_quit)
     {
        _efl_loop_timer_enable_new(obj, pd);
        goto done;
     }
   if (!pd->message_queue)
     {
        // init flags
        next_time = _efl_loop_timer_next_get(obj, pd);
        // no idlers
        if (!_ecore_main_idlers_exist(pd))
          {
             // sleep until timeout or forever (-1.0) waiting for on fds
             _ecore_main_select(obj, pd, next_time);
          }
        else
          {
             int action = LOOP_CONTINUE;

             // no timers - spin
             if (next_time < 0) action = _ecore_main_loop_spin_no_timers(obj, pd);
             // timers - spin
             else action = _ecore_main_loop_spin_timers(obj, pd);
             if (action == SPIN_RESTART) goto start_loop;
          }
     }
   if (obj == ML_OBJ) _ecore_fps_marker_2();

   // actually wake up and deal with input, events etc.
process_all: //-*********************************************************

   // we came out of our "wait state" so idle has exited
   if (!once_only)
     {
        _ecore_animator_run_reset(); // XXX:
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_EXIT, NULL);
     }
   // call the fd handler per fd that became alive...
   // this should read or write any data to the monitored fd and then
   // post events onto the ecore event pipe if necessary
   _ecore_main_fd_handlers_call(obj, pd);
   if (pd->fd_handlers_with_buffer) _ecore_main_fd_handlers_buf_call(obj, pd);
   // process signals into events ....
   _ecore_signal_received_process(obj, pd);
   // handle events ...
   efl_loop_message_process(obj);
   _ecore_main_fd_handlers_cleanup(obj, pd);

   if (once_only)
     {
        // if in once_only mode handle idle exiting
        efl_event_callback_call(obj, EFL_LOOP_EVENT_IDLE_ENTER, NULL);
        _ecore_throttle();
        _throttle_do(pd);
     }

done: //-*****************************************************************
   // Agressively flush animator
   _ecore_animator_flush();
   if (!once_only)
     eina_slstr_local_clear(); // Free all short lived strings

   if (obj == ML_OBJ)
     {
        in_main_loop--;
        pd->in_loop = in_main_loop;
     }
}

#endif

#ifdef _WIN32
typedef struct
{
   DWORD   objects_nbr;
   HANDLE *objects;
   DWORD   timeout;
} Ecore_Main_Win32_Thread_Data;

static unsigned int __stdcall
_ecore_main_win32_objects_wait_thread(void *data)
{
   Ecore_Main_Win32_Thread_Data *td = (Ecore_Main_Win32_Thread_Data *)data;
   return MsgWaitForMultipleObjects(td->objects_nbr,
                                    (const HANDLE *)td->objects,
                                    FALSE,
                                    td->timeout,
                                    QS_ALLINPUT);
}

static DWORD
_ecore_main_win32_objects_wait(DWORD objects_nbr,
                               const HANDLE *objects,
                               DWORD timeout)
{
   Ecore_Main_Win32_Thread_Data *threads_data;
   HANDLE *threads_handles;
   DWORD threads_nbr, threads_remain, objects_idx, result, i;

   if (objects_nbr < MAXIMUM_WAIT_OBJECTS)
     return MsgWaitForMultipleObjects(objects_nbr,
                                      objects,
                                      EINA_FALSE,
                                      timeout, QS_ALLINPUT);
   // too much objects, so we launch a bunch of threads to
   // wait for, each one calls MsgWaitForMultipleObjects

   threads_nbr = objects_nbr / (MAXIMUM_WAIT_OBJECTS - 1);
   threads_remain = objects_nbr % (MAXIMUM_WAIT_OBJECTS - 1);
   if (threads_remain > 0) threads_nbr++;

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

   threads_data = (Ecore_Main_Win32_Thread_Data *)
     malloc(threads_nbr * sizeof(Ecore_Main_Win32_Thread_Data));
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

        threads_handles[i] = (HANDLE)_beginthreadex
          (NULL, 0, _ecore_main_win32_objects_wait_thread,
           &threads_data[i], 0, NULL);
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
                                   FALSE, // we wait until one thread signaled
                                   INFINITE);

   if (result < (WAIT_OBJECT_0 + threads_nbr))
     {
        DWORD wait_res;

        // One of the thread callback has exited so we retrieve
        // its exit status, that is the returned value of
        // MsgWaitForMultipleObjects()
        if (GetExitCodeThread(threads_handles[result - WAIT_OBJECT_0],
                              &wait_res))
          {
             WaitForMultipleObjects(threads_nbr, threads_handles,
                                    TRUE, INFINITE);
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
          ERR("%s", evil_last_error_get());
        goto close_thread;
     }

close_thread:
   WaitForMultipleObjects(threads_nbr, threads_handles, TRUE, INFINITE);
   for (i = 0; i < threads_nbr; i++) CloseHandle(threads_handles[i]);
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
   Efl_Loop_Data *pd = ML_DAT;
   HANDLE *objects;
   int *sockets;
   Ecore_Fd_Handler *fdh;
   Ecore_Win32_Handler *wh;
   static HANDLE stdin_wait_thread = INVALID_HANDLE_VALUE;
   HANDLE stdin_handle;
   DWORD result, timeout;
   MSG msg;
   unsigned int fds_nbr = 0;
   unsigned int objects_nbr = 0;
   unsigned int events_nbr = 0;
   unsigned int i;
   int res;
   Eina_Bool stdin_thread_done = EINA_FALSE;

   fds_nbr = eina_inlist_count(EINA_INLIST_GET(pd->fd_handlers));
   sockets = (int *)malloc(fds_nbr * sizeof(int));
   if (!sockets) return -1;

   objects = (HANDLE)malloc((fds_nbr +
                             eina_inlist_count
                             (EINA_INLIST_GET(pd->win32_handlers)))
                            * sizeof(HANDLE));
   if (!objects)
     {
        free(sockets);
        return -1;
     }

   // Create an event object per socket
   EINA_INLIST_FOREACH(pd->fd_handlers, fdh)
     {
        if (fdh->delete_me) continue;

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
   // store the HANDLEs in the objects to wait for
   EINA_INLIST_FOREACH(pd->win32_handlers, wh)
     {
        if (wh->delete_me) continue;

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
        else objects[objects_nbr] = wh->h;
        objects_nbr++;
     }

   // Empty the queue before waiting
   while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
     {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
     }

   // Wait for any message sent or posted to this queue
   // or for one of the passed handles be set to signaled.
   if (!tv) timeout = INFINITE;
   else timeout = (DWORD)((tv->tv_sec * 1000.0) + (tv->tv_usec / 1000.0));

   if (timeout == 0)
     {
        res = 0;
        goto err;
     }

   result = _ecore_main_win32_objects_wait(objects_nbr,
                                           (const HANDLE *)objects,
                                           timeout);
   if (readfds)   FD_ZERO(readfds);
   if (writefds)  FD_ZERO(writefds);
   if (exceptfds) FD_ZERO(exceptfds);
   // The result tells us the type of event we have.
   if (result == WAIT_FAILED)
     {
        WRN("%s", evil_last_error_get());
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
        if ((network_event.lNetworkEvents &
             (FD_READ | FD_CONNECT | FD_ACCEPT)) && readfds)
          FD_SET(sockets[result], readfds);
        if ((network_event.lNetworkEvents &
             (FD_WRITE | FD_CLOSE)) && writefds)
          FD_SET(sockets[result], writefds);
        if ((network_event.lNetworkEvents & FD_OOB) && exceptfds)
          FD_SET(sockets[result], exceptfds);
        res = 1;
     }
   else if ((result >= (WAIT_OBJECT_0 + events_nbr)) &&
            (result < (WAIT_OBJECT_0 + objects_nbr)))
     {
        if (!pd->win32_handler_current)
          // regular main loop, start from head
          pd->win32_handler_current = pd->win32_handlers;
        else
          // recursive main loop, continue from where we were
          pd->win32_handler_current = (Ecore_Win32_Handler *)
            EINA_INLIST_GET(pd->win32_handler_current)->next;

        if (objects[result - WAIT_OBJECT_0] == stdin_wait_thread)
          stdin_thread_done = EINA_TRUE;

        while (pd->win32_handler_current)
          {
             wh = pd->win32_handler_current;

             if ((objects[result - WAIT_OBJECT_0] == wh->h) ||
                 ((objects[result - WAIT_OBJECT_0] == stdin_wait_thread) &&
                  (wh->h == stdin_handle)))
               {
                  if (!wh->delete_me)
                    {
                       wh->references++;
                       if (!wh->func(wh->data, wh))
                         {
                            wh->delete_me = EINA_TRUE;
                            pd->win32_handlers_to_delete = eina_list_append
                              (pd->win32_handlers_to_delete, wh);
                         }
                       wh->references--;
                    }
               }
             if (pd->win32_handler_current)
               // may have changed in recursive main loops
               pd->win32_handler_current = (Ecore_Win32_Handler *)
                 EINA_INLIST_GET(pd->win32_handler_current)->next;
          }
        res = 1;
     }
   else
     {
        ERR("unknown result...\n");
        res = -1;
     }

err:
   // Remove event objects again
   for (i = 0; i < events_nbr; i++) WSACloseEvent(objects[i]);

   if (stdin_thread_done) stdin_wait_thread = INVALID_HANDLE_VALUE;

   free(objects);
   free(sockets);
   return res;
}
#endif
