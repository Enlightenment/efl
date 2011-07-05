#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# undef WIN32_LEAN_AND_MEAN
# ifndef USER_TIMER_MINIMUM
#  define USER_TIMER_MINIMUM 0x0a
# endif
#endif

#ifdef __SUNPRO_C
# include <ieeefp.h>
# include <string.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#ifndef _MSC_VER
# include <sys/time.h>
# include <unistd.h>
#else
# include <float.h>
#endif

#ifdef HAVE_ISFINITE
# define ECORE_FINITE(t) isfinite(t)
#else
# ifdef _MSC_VER
#  define ECORE_FINITE(t) _finite(t)
# else
#  define ECORE_FINITE(t) finite(t)
# endif
#endif

#define FIX_HZ 1

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

#ifdef HAVE_SYS_EPOLL_H
# define HAVE_EPOLL
# include <sys/epoll.h>
#endif

#ifdef USE_G_MAIN_LOOP
# include <glib.h>
#endif

struct _Ecore_Fd_Handler
{
   EINA_INLIST;
   ECORE_MAGIC;
   Ecore_Fd_Handler        *next_ready;
   int                      fd;
   Ecore_Fd_Handler_Flags   flags;
   Ecore_Fd_Cb              func;
   void                    *data;
   Ecore_Fd_Cb              buf_func;
   void                    *buf_data;
   Ecore_Fd_Prep_Cb         prep_func;
   void                    *prep_data;
   int                      references;
   Eina_Bool                read_active : 1;
   Eina_Bool                write_active : 1;
   Eina_Bool                error_active : 1;
   Eina_Bool                delete_me : 1;
#if defined(USE_G_MAIN_LOOP) && !defined(HAVE_EPOLL)
   GPollFD                  gfd;
#endif
};

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
#endif


#ifndef USE_G_MAIN_LOOP
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
static int _ecore_main_win32_select(int nfds, fd_set *readfds, fd_set *writefds,
                                    fd_set *exceptfds, struct timeval *timeout);
static void _ecore_main_win32_handlers_cleanup(void);
#endif

static int               in_main_loop = 0;
#ifndef USE_G_MAIN_LOOP
static int               do_quit = 0;
#endif
static Ecore_Fd_Handler *fd_handlers = NULL;
static Ecore_Fd_Handler *fd_handler_current = NULL;
static Eina_List        *fd_handlers_with_prep = NULL;
static Eina_List        *fd_handlers_with_buffer = NULL;
static Eina_List        *fd_handlers_to_delete = NULL;

/* single linked list of ready fdhs, terminated by loop to self */
static Ecore_Fd_Handler    *fd_handlers_to_call;
static Ecore_Fd_Handler    *fd_handlers_to_call_current;

#ifdef _WIN32
static Ecore_Win32_Handler *win32_handlers = NULL;
static Ecore_Win32_Handler *win32_handler_current = NULL;
static Eina_Bool            win32_handlers_delete_me = EINA_FALSE;
#endif

#ifdef _WIN32
Ecore_Select_Function main_loop_select = _ecore_main_win32_select;
#else
Ecore_Select_Function main_loop_select = select;
#endif

#ifndef USE_G_MAIN_LOOP
static double            t1 = 0.0;
static double            t2 = 0.0;
#endif

#ifdef HAVE_EPOLL
static int epoll_fd = -1;
static pid_t epoll_pid;
#endif

#ifdef USE_G_MAIN_LOOP
#ifdef HAVE_EPOLL
static GPollFD ecore_epoll_fd;
#endif
static GSource *ecore_glib_source;
static guint ecore_glib_source_id;
static GMainLoop* ecore_main_loop;
static gboolean ecore_idling;
static gboolean ecore_fds_ready;
#endif

void
_ecore_fd_valid(void)
{
#ifdef HAVE_EPOLL
   if (fcntl(epoll_fd, F_GETFD) < 0)
     {
        ERR("arghhh you caught me! report a backtrace to edevel!");
        pause();
     }
#endif
}

static inline void
_ecore_try_add_to_call_list(Ecore_Fd_Handler *fdh)
{
   /* check if this fdh is already in the list */
   if (fdh->next_ready)
     return;
   if (fdh->read_active || fdh->write_active || fdh->error_active)
     {
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
   if (epoll_pid && epoll_pid != getpid())
     {
        /* forked! */
        _ecore_main_loop_shutdown();
     }
   if (epoll_pid == 0 && epoll_fd < 0)
     {
        _ecore_main_loop_init();
     }
   return epoll_fd;
}

static inline int
_ecore_epoll_add(int efd, int fd, int events, void *ptr)
{
   struct epoll_event ev;

   memset(&ev, 0, sizeof (ev));
   ev.events = events;
   ev.data.ptr = ptr;
   INF("adding poll on %d %08x", fd, events);
   return epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev);
}

static inline int
_ecore_poll_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ)  events |= EPOLLIN;
   if (fdh->flags & ECORE_FD_WRITE) events |= EPOLLOUT;
   if (fdh->flags & ECORE_FD_ERROR) events |= EPOLLERR;
   return events;
}
#else
static inline int
_ecore_poll_events_from_fdh(Ecore_Fd_Handler *fdh __UNUSED__)
{
   return 0;
}
#endif

#ifdef USE_G_MAIN_LOOP
static inline int
_gfd_events_from_fdh(Ecore_Fd_Handler *fdh)
{
   int events = 0;
   if (fdh->flags & ECORE_FD_READ)  events |= G_IO_IN;
   if (fdh->flags & ECORE_FD_WRITE) events |= G_IO_OUT;
   if (fdh->flags & ECORE_FD_ERROR) events |= G_IO_ERR;
   return events;
}
#endif

static inline int
_ecore_main_fdh_poll_add(Ecore_Fd_Handler *fdh)
{
   int r = 0;
#ifdef HAVE_EPOLL
   r = _ecore_epoll_add(_ecore_get_epoll_fd(), fdh->fd,
                        _ecore_poll_events_from_fdh(fdh), fdh);
#elif USE_G_MAIN_LOOP
   fdh->gfd.fd = fdh->fd;
   fdh->gfd.events = _gfd_events_from_fdh(fdh);
   fdh->gfd.revents = 0;
   INF("adding gpoll on %d %08x", fdh->fd, fdh->gfd.events);
   g_source_add_poll(ecore_glib_source, &fdh->gfd);
#else
   if (!ECORE_MAGIC_CHECK(fdh, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fdh, ECORE_MAGIC_FD_HANDLER,
                         "_ecore_main_fdh_poll_add");
     }
#endif
   return r;
}

static inline void
_ecore_main_fdh_poll_del(Ecore_Fd_Handler *fdh)
{
#ifdef HAVE_EPOLL
   struct epoll_event ev;
   int efd = _ecore_get_epoll_fd();

   memset(&ev, 0, sizeof (ev));
   INF("removing poll on %d", fdh->fd);
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
#elif USE_G_MAIN_LOOP
   fdh->gfd.fd = fdh->fd;
   fdh->gfd.events = _gfd_events_from_fdh(fdh);
   fdh->gfd.revents = 0;
   INF("adding gpoll on %d %08x", fdh->fd, fdh->gfd.events);
   g_source_add_poll(ecore_glib_source, &fdh->gfd);
#else
   if (!ECORE_MAGIC_CHECK(fdh, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fdh, ECORE_MAGIC_FD_HANDLER,
                         "_ecore_main_fdh_poll_del");
     }
#endif
}

static inline int
_ecore_main_fdh_poll_modify(Ecore_Fd_Handler *fdh)
{
   int r = 0;
#ifdef HAVE_EPOLL
   struct epoll_event ev;
   int efd = _ecore_get_epoll_fd();

   memset(&ev, 0, sizeof (ev));
   ev.events = _ecore_poll_events_from_fdh(fdh);
   ev.data.ptr = fdh;
   INF("modifing epoll on %d to %08x", fdh->fd, ev.events);
   r = epoll_ctl(efd, EPOLL_CTL_MOD, fdh->fd, &ev);
#elif USE_G_MAIN_LOOP
   fdh->gfd.fd = fdh->fd;
   fdh->gfd.events = _gfd_events_from_fdh(fdh);
   fdh->gfd.revents = 0;
   INF("modifing gpoll on %d to %08x", fdh->fd, fdh->gfd.events);
#else
   if (!ECORE_MAGIC_CHECK(fdh, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fdh, ECORE_MAGIC_FD_HANDLER,
                         "_ecore_main_fdh_poll_modify");
     }
#endif
   return r;
}

#ifdef HAVE_EPOLL
static inline int _ecore_main_fdh_poll_mark_active(void)
{
   struct epoll_event ev[32];
   int i, ret;
   int efd = _ecore_get_epoll_fd();

   memset(&ev, 0, sizeof (ev));
   ret = epoll_wait(efd, ev, sizeof(ev) / sizeof(struct epoll_event), 0);
   if (ret < 0)
     {
        if (errno == EINTR) return -1;
        ERR("epoll_wait failed %d", errno);
        return -1;
     }

   for (i = 0; i < ret; i++)
     {
        Ecore_Fd_Handler *fdh;

        fdh = ev[i].data.ptr;
        if (!ECORE_MAGIC_CHECK(fdh, ECORE_MAGIC_FD_HANDLER))
          {
             ECORE_MAGIC_FAIL(fdh, ECORE_MAGIC_FD_HANDLER,
                              "_ecore_main_fdh_poll_mark_active");
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

#elif USE_G_MAIN_LOOP

static inline int _ecore_main_fdh_poll_mark_active(void)
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

        if (fdh->gfd.revents & (G_IO_IN|G_IO_OUT|G_IO_ERR)) ret++;
     }

   INF("found %d active fds", ret);

   return ret;
}

#endif

#ifdef USE_G_MAIN_LOOP

/* like we are about to enter main_loop_select in  _ecore_main_select */
static gboolean
_ecore_main_gsource_prepare(GSource *source __UNUSED__, gint *next_time)
{
   double t = _ecore_timer_next_get();
   gboolean running;

   INF("enter, next timeout in %.1f", t);
   in_main_loop++;

   if (!ecore_idling)
     {
         while (_ecore_timer_call(_ecore_time_loop_time));
         _ecore_timer_cleanup();

         /* when idling, busy loop checking the fds only */
         if (!ecore_idling)
           {
              _ecore_idle_enterer_call();
              _ecore_throttle();
           }
     }

   /* don't check fds if somebody quit */
   running = g_main_loop_is_running(ecore_main_loop);
   if (running)
     {
        /* only set idling state in dispatch */
        if (ecore_idling && !_ecore_idler_exist())
          {
             if (_ecore_timers_exists())
               {
                  double t = _ecore_timer_next_get();
                  *next_time = ceil(t * 1000.0);
               }
             else
               *next_time = -1;
          }
        else
          *next_time = 0;

        if (fd_handlers_with_prep)
          _ecore_main_prepare_handlers();
     }

   in_main_loop--;
   INF("leave, timeout = %d", *next_time);

   /* ready if we're not running (about to quit) */
   return !running;
}

static gboolean
_ecore_main_gsource_check(GSource *source __UNUSED__)
{
   in_main_loop++;

   ecore_fds_ready = (_ecore_main_fdh_poll_mark_active() > 0);
   _ecore_main_fd_handlers_cleanup();

   _ecore_time_loop_time = ecore_time_get();
   _ecore_timer_enable_new();

   in_main_loop--;

   return TRUE; /* always dispatch */
}

/* like we just came out of main_loop_select in  _ecore_main_select */
static gboolean
_ecore_main_gsource_dispatch(GSource *source __UNUSED__, GSourceFunc callback __UNUSED__, gpointer user_data __UNUSED__)
{
   gboolean events_ready, timers_ready, idlers_ready, signals_ready;
   double next_time = _ecore_timer_next_get();

   events_ready = _ecore_event_exist();
   timers_ready = _ecore_timers_exists() && (0.0 >= next_time);
   idlers_ready = _ecore_idler_exist();
   signals_ready = (_ecore_signal_count_get() > 0);

   in_main_loop++;
   INF("enter idling=%d fds=%d events=%d signals=%d timers=%d (next=%.2f) idlers=%d",
       ecore_idling, ecore_fds_ready, events_ready, signals_ready,
       timers_ready, next_time, idlers_ready);

   if (ecore_idling && events_ready)
     {
        _ecore_idle_exiter_call();
        ecore_idling = 0;
     }
   else if (!ecore_idling && !events_ready)
     {
        ecore_idling = 1;
     }

   if (ecore_idling)
     {
        _ecore_idler_call();

        events_ready = _ecore_event_exist();
        timers_ready = _ecore_timers_exists() && (0.0 >= next_time);
        idlers_ready = _ecore_idler_exist();

        if ((ecore_fds_ready || events_ready || timers_ready || idlers_ready || signals_ready))
          {
             _ecore_idle_exiter_call();
             ecore_idling = 0;
          }
     }

   /* process events */
   if (!ecore_idling)
     {
        _ecore_main_fd_handlers_call();
        if (fd_handlers_with_buffer)
          _ecore_main_fd_handlers_buf_call();
        while (_ecore_signal_count_get()) _ecore_signal_call();
        _ecore_event_call();
        _ecore_main_fd_handlers_cleanup();
     }

   in_main_loop--;

   return TRUE; /* what should be returned here? */
}

static void
_ecore_main_gsource_finalize(GSource *source __UNUSED__)
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

void
_ecore_main_loop_init(void)
{
   INF("enter");
#ifdef HAVE_EPOLL
   epoll_fd = epoll_create(1);
   if (epoll_fd < 0)
      CRIT("Failed to create epoll fd!");
   epoll_pid = getpid();

   /* add polls on all our file descriptors */
   Ecore_Fd_Handler *fdh;
   EINA_INLIST_FOREACH(fd_handlers, fdh)
     {
        if (fdh->delete_me)
           continue;
        _ecore_epoll_add(epoll_fd, fdh->fd,
                         _ecore_poll_events_from_fdh(fdh), fdh);
        _ecore_main_fdh_poll_add(fdh);
     }

#endif

#ifdef USE_G_MAIN_LOOP
   ecore_glib_source = g_source_new(&ecore_gsource_funcs, sizeof (GSource));
   if (!ecore_glib_source)
      CRIT("Failed to create glib source for epoll!");
   else
     {
#ifdef HAVE_EPOLL
        ecore_epoll_fd.fd = epoll_fd;
        ecore_epoll_fd.events = G_IO_IN;
        ecore_epoll_fd.revents = 0;
        g_source_add_poll(ecore_glib_source, &ecore_epoll_fd);
#endif
        ecore_glib_source_id = g_source_attach(ecore_glib_source, NULL);
        if (ecore_glib_source_id <= 0)
           CRIT("Failed to attach glib source to default context");
     }
#endif
}

void
_ecore_main_loop_shutdown(void)
{
#ifdef USE_G_MAIN_LOOP
   if (ecore_glib_source)
     {
        g_source_destroy(ecore_glib_source);
        ecore_glib_source = NULL;
     }
#endif

#ifdef HAVE_EPOLL
   if (epoll_fd >= 0)
     {
        close(epoll_fd);
        epoll_fd = -1;
     }

    epoll_pid = 0;
#endif
}

/**
 * @addtogroup Ecore_Group Ecore - Main Loop and Job Functions.
 *
 * @{
 */

/**
 * @addtogroup Ecore_Main_Loop_Group Ecore Main Loop functions
 *
 * These functions control the Ecore event handling loop.  This loop is
 * designed to work on embedded systems all the way to large and
 * powerful mutli-cpu workstations.
 *
 * It serialises all system signals and events into a single event
 * queue, that can be easily processed without needing to worry about
 * concurrency.  A properly written, event-driven program using this
 * kind of programming does not need threads.  It makes the program very
 * robust and easy to follow.
 *
 * Here is an example of simple program and its basic event loop flow:
 * @image html prog_flow.png
 *
 * For examples of setting up and using a main loop, see @ref
 * Ecore_Main_Loop_Page.
 *
 * @{
 */

/**
 * Runs a single iteration of the main loop to process everything on the
 * queue.
 *
 * It does everything that is already done inside an @c Ecore main loop, like
 * checking for expired timers, idlers, etc. But it will do it only once and
 * return, instead of keep watching for new events.
 */
EAPI void
ecore_main_loop_iterate(void)
{
#ifndef USE_G_MAIN_LOOP
   _ecore_main_loop_iterate_internal(1);
#else
    g_main_context_iteration(NULL, 1);
#endif
}

/**
 * Runs the application main loop.
 *
 * This function will not return until @ref ecore_main_loop_quit is called. It
 * will check for expired timers, idlers, file descriptors being watched by fd
 * handlers, etc. Once everything is done, before entering again on idle state,
 * any callback set as @c Idle_Enterer will be called.
 *
 * Each main loop iteration is done by calling ecore_main_loop_iterate()
 * internally.
 *
 * The polling (select) function used can be changed with
 * ecore_main_loop_select_func_set().
 *
 * The function used to check for file descriptors, events, and that has a
 * timeout for the timers can be changed using
 * ecore_main_loop_select_func_set().
 */
EAPI void
ecore_main_loop_begin(void)
{
#ifndef USE_G_MAIN_LOOP
   in_main_loop++;
   while (do_quit == 0) _ecore_main_loop_iterate_internal(0);
   do_quit = 0;
   in_main_loop--;
#else
   ecore_main_loop = g_main_loop_new(NULL, FALSE);
   g_main_loop_run(ecore_main_loop);
#endif
}

/**
 * Quits the main loop once all the events currently on the queue have
 * been processed.
 *
 * This function returns immediately, but will mark the ecore_main_loop_begin()
 * function to return at the end of the current main loop iteration.
 */
EAPI void
ecore_main_loop_quit(void)
{
#ifndef USE_G_MAIN_LOOP
   do_quit = 1;
#else
   g_main_loop_quit(ecore_main_loop);
#endif
}

/**
 * Sets the function to use when monitoring multiple file descriptors,
 * and waiting until one of more of the file descriptors before ready
 * for some class of I/O operation.
 *
 * This function will be used instead of the system call select and
 * could possible be used to integrate the Ecore event loop with an
 * external event loop.
 *
 * @warning you don't know how to use, don't even try to use it.
 *
 */
EAPI void
ecore_main_loop_select_func_set(Ecore_Select_Function func)
{
   main_loop_select = func;
}

/**
 * Gets the select function set by ecore_select_func_set(),
 * or the native select function if none was set.
 *
 */
EAPI Ecore_Select_Function
ecore_main_loop_select_func_get(void)
{
   return main_loop_select;
}

/**
 * @defgroup Ecore_FD_Handler_Group File Event Handling Functions
 *
 * Functions that deal with file descriptor handlers.
 *
 * The @ref Ecore_Fd_Handler can be used to watch a file descriptor for data
 * available for reading, for the availability to write without blocking, and
 * for errors on the file descriptor.
 *
 * ecore_main_fd_handler_add() is used to setup a handler for a given file
 * descriptor. This file descriptor can be the standard input, a network socket,
 * a stream received through some driver of a hardware decoder, etc. Thus it can
 * contain errors, like a disconnection, a broken pipe, and so, and that's why
 * it's possible to check for these errors with the @ref ECORE_FD_ERROR flag.
 *
 * An @ref Ecore_Fd_Handler can be used to watch on a file descriptor without
 * blocking, still being able to receive events, expire timers, and other watch
 * for other things that happen in the Ecore main loop.
 *
 * Example of use of a file descriptor handler:
 * @li @ref ecore_fd_handler_example_c
 */

/**
 * Adds a callback for activity on the given file descriptor.
 *
 * @p func will be called during the execution of @ref ecore_main_loop_begin
 * when the file descriptor is available for reading, or writing, or both.
 *
 * Normally the return value from the @p func is "zero means this handler is
 * finished and can be deleted" as is usual for handler callbacks.  However,
 * if the @p buf_func is supplied, then the return value from the @p func is
 * "non zero means the handler should be called again in a tight loop".
 *
 * @p buf_func is called during event loop handling to check if data that has
 * been read from the file descriptor is in a buffer and is available to
 * read.  Some systems (notably xlib) handle their own buffering, and would
 * otherwise not work with select().  These systems should use a @p buf_func.
 * This is a most annoying hack, only ecore_x uses it, so refer to that for
 * an example.  NOTE - @p func should probably return "one" always if
 * @p buf_func is used, to avoid confusion with the other return value
 * semantics.
 *
 * @param   fd       The file descriptor to watch.
 * @param   flags    To watch it for read (@c ECORE_FD_READ) and/or
 *                   (@c ECORE_FD_WRITE) write ability.  @c ECORE_FD_ERROR
 *
 * @param   func     The callback function.
 * @param   data     The data to pass to the callback.
 * @param   buf_func The function to call to check if any data has been
 *                   buffered and already read from the fd.  Can be @c NULL.
 * @param   buf_data The data to pass to the @p buf_func function.
 * @return  A fd handler handle if successful.  @c NULL otherwise.
 * @ingroup Ecore_FD_Handler_Group
 */
EAPI Ecore_Fd_Handler *
ecore_main_fd_handler_add(int fd, Ecore_Fd_Handler_Flags flags, Ecore_Fd_Cb func, const void *data,
                          Ecore_Fd_Cb buf_func, const void *buf_data)
{
   Ecore_Fd_Handler *fdh;

   if ((fd < 0) || (flags == 0) || (!func)) return NULL;

   fdh = calloc(1, sizeof(Ecore_Fd_Handler));
   if (!fdh) return NULL;
   ECORE_MAGIC_SET(fdh, ECORE_MAGIC_FD_HANDLER);
   fdh->next_ready = NULL;
   fdh->fd = fd;
   fdh->flags = flags;
   if (_ecore_main_fdh_poll_add(fdh) < 0)
     {
        int err = errno;
        ERR("Failed to add poll on fd %d (errno = %d: %s)!", fd, err, strerror(err));
        free(fdh);
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
   fd_handlers = (Ecore_Fd_Handler *)
      eina_inlist_append(EINA_INLIST_GET(fd_handlers),
                         EINA_INLIST_GET(fdh));
   return fdh;
}

#ifdef _WIN32
EAPI Ecore_Win32_Handler *
ecore_main_win32_handler_add(void *h, Ecore_Win32_Handle_Cb func, const void *data)
{
   Ecore_Win32_Handler *wh;

   if (!h || !func) return NULL;

   wh = calloc(1, sizeof(Ecore_Win32_Handler));
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
ecore_main_win32_handler_add(void *h __UNUSED__, Ecore_Win32_Handle_Cb func __UNUSED__,
                             const void *data __UNUSED__)
{
   return NULL;
}
#endif

/**
 * Deletes the given FD handler.
 * @param   fd_handler The given FD handler.
 * @return  The data pointer set using @ref ecore_main_fd_handler_add,
 *          for @p fd_handler on success.  @c NULL otherwise.
 * @ingroup Ecore_FD_Handler_Group
 *
 * Beware that if the fd is already closed, ecore may complain if it uses
 * epoll internally, and that in some rare cases this may be able to cause
 * crashes and instability. Remember to delete your fd handlers before the
 * fd's they listen to are closed.
 */
EAPI void *
ecore_main_fd_handler_del(Ecore_Fd_Handler *fd_handler)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_del");
        return NULL;
     }
   if (fd_handler->delete_me)
     {
        ERR("fdh %p deleted twice", fd_handler);
        return NULL;
     }

   _ecore_main_fdh_poll_del(fd_handler);
   fd_handler->delete_me = EINA_TRUE;
   fd_handlers_to_delete = eina_list_append(fd_handlers_to_delete, fd_handler);
   if (fd_handler->prep_func && fd_handlers_with_prep)
     fd_handlers_with_prep = eina_list_remove(fd_handlers_with_prep, fd_handler);
   if (fd_handler->buf_func && fd_handlers_with_buffer)
     fd_handlers_with_buffer = eina_list_remove(fd_handlers_with_buffer, fd_handler);
   return fd_handler->data;
}

#ifdef _WIN32
EAPI void *
ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler)
{
   if (!ECORE_MAGIC_CHECK(win32_handler, ECORE_MAGIC_WIN32_HANDLER))
     {
        ECORE_MAGIC_FAIL(win32_handler, ECORE_MAGIC_WIN32_HANDLER,
                         "ecore_main_win32_handler_del");
        return NULL;
     }
   win32_handler->delete_me = EINA_TRUE;
   win32_handlers_delete_me = EINA_TRUE;
   return win32_handler->data;
}
#else
EAPI void *
ecore_main_win32_handler_del(Ecore_Win32_Handler *win32_handler __UNUSED__)
{
   return NULL;
}
#endif

/**
 * @brief Set the prepare callback with data for a given #Ecore_Fd_Handler
 *
 * @param fd_handler The fd handler
 * @param func The prep function
 * @param data The data to pass to the prep function
 *
 * This function will be called prior to any fd handler's callback function
 * (even the other fd handlers), before entering the main loop select function.
 *
 * @note Once a prepare callback is set for a fd handler, it cannot be changed.
 * You need to delete the fd handler and create a new one, to set another
 * callback.
 * @note You probably don't need this function. It is only necessary for very
 * uncommon cases that need special behavior.
 *
 * @ingroup Ecore_FD_Handler_Group
 */
EAPI void
ecore_main_fd_handler_prepare_callback_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Prep_Cb func, const void *data)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_prepare_callback_set");
        return;
     }
   fd_handler->prep_func = func;
   fd_handler->prep_data = (void *)data;
   if ((!fd_handlers_with_prep) ||
      (fd_handlers_with_prep && (!eina_list_data_find(fd_handlers_with_prep, fd_handler))))
     /* FIXME: THIS WILL NOT SCALE WITH LOTS OF PREP FUNCTIONS!!! */
     fd_handlers_with_prep = eina_list_append(fd_handlers_with_prep, fd_handler);
}

/**
 * Retrieves the file descriptor that the given handler is handling.
 * @param   fd_handler The given FD handler.
 * @return  The file descriptor the handler is watching.
 * @ingroup Ecore_FD_Handler_Group
 */
EAPI int
ecore_main_fd_handler_fd_get(Ecore_Fd_Handler *fd_handler)
{
   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_fd_get");
        return -1;
     }
   return fd_handler->fd;
}

/**
 * Return if read, write or error, or a combination thereof, is active on the
 * file descriptor of the given FD handler.
 * @param   fd_handler The given FD handler.
 * @param   flags      The flags, @c ECORE_FD_READ, @c ECORE_FD_WRITE or
 *                     @c ECORE_FD_ERROR to query.
 * @return  #EINA_TRUE if any of the given flags are active. #EINA_FALSE otherwise.
 * @ingroup Ecore_FD_Handler_Group
 */
EAPI Eina_Bool
ecore_main_fd_handler_active_get(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags)
{
   int ret = EINA_FALSE;

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

/**
 * Set what active streams the given FD handler should be monitoring.
 * @param   fd_handler The given FD handler.
 * @param   flags      The flags to be watching.
 * @ingroup Ecore_FD_Handler_Group
 */
EAPI void
ecore_main_fd_handler_active_set(Ecore_Fd_Handler *fd_handler, Ecore_Fd_Handler_Flags flags)
{
   int ret;

   if (!ECORE_MAGIC_CHECK(fd_handler, ECORE_MAGIC_FD_HANDLER))
     {
        ECORE_MAGIC_FAIL(fd_handler, ECORE_MAGIC_FD_HANDLER,
                         "ecore_main_fd_handler_active_set");
        return;
     }
   fd_handler->flags = flags;
   ret = _ecore_main_fdh_poll_modify(fd_handler);
   if (ret < 0)
     {
        ERR("Failed to mod epoll fd %d: %s!", fd_handler->fd, strerror(ret));
     }
}

/**
 * @}
 */

/**
 * @}
 */

void
_ecore_main_shutdown(void)
{
   if (in_main_loop)
     {
       ERR("\n"
           "*** ECORE WARINING: Calling ecore_shutdown() while still in the main loop.\n"
           "***                 Program may crash or behave strangely now.");
        return;
     }
   while (fd_handlers)
     {
        Ecore_Fd_Handler *fdh;

        fdh = fd_handlers;
        fd_handlers = (Ecore_Fd_Handler *) eina_inlist_remove(EINA_INLIST_GET(fd_handlers),
                                                              EINA_INLIST_GET(fdh));
        ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
        free(fdh);
     }
   if (fd_handlers_with_buffer)
     fd_handlers_with_buffer = eina_list_free(fd_handlers_with_buffer);
   if (fd_handlers_with_prep)
     fd_handlers_with_prep = eina_list_free(fd_handlers_with_prep);
   if (fd_handlers_to_delete)
     fd_handlers_to_delete = eina_list_free(fd_handlers_to_delete);

   fd_handlers_to_call = NULL;
   fd_handlers_to_call_current = NULL;
   fd_handlers_to_delete = NULL;
   fd_handler_current = NULL;

#ifdef _WIN32
   while (win32_handlers)
     {
        Ecore_Win32_Handler *wh;

        wh = win32_handlers;
        win32_handlers = (Ecore_Win32_Handler *) eina_inlist_remove(EINA_INLIST_GET(win32_handlers),
                                                                    EINA_INLIST_GET(wh));
        ECORE_MAGIC_SET(wh, ECORE_MAGIC_NONE);
        free(wh);
     }
   win32_handlers_delete_me = EINA_FALSE;
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
             fdh->prep_func(fdh->prep_data, fdh);
             fdh->references--;
          }
        else
          fd_handlers_with_prep = eina_list_remove_list(fd_handlers_with_prep, l);
     }
}

#ifndef USE_G_MAIN_LOOP
static int
_ecore_main_select(double timeout)
{
   struct timeval tv, *t;
   fd_set         rfds, wfds, exfds;
   int            max_fd;
   int            ret;
#ifndef HAVE_EPOLL
   Ecore_Fd_Handler *fdh;
#endif

   t = NULL;
   if ((!ECORE_FINITE(timeout)) || (timeout == 0.0)) /* finite() tests for NaN, too big, too small, and infinity.  */
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
#ifndef HAVE_EPOLL
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
#else /* HAVE_EPOLL */
   /* polling on the epoll fd will wake when an fd in the epoll set is active */
   max_fd = _ecore_get_epoll_fd();
   FD_SET(max_fd, &rfds);
#endif /* HAVE_EPOLL */

   if (_ecore_signal_count_get()) return -1;

   ret = main_loop_select(max_fd + 1, &rfds, &wfds, &exfds, t);
   
   _ecore_time_loop_time = ecore_time_get();
   if (ret < 0)
     {
#ifndef _WIN32
        if (errno == EINTR) return -1;
        else if (errno == EBADF) _ecore_main_fd_handlers_bads_rem();
#endif
     }
   if (ret > 0)
     {
#ifdef HAVE_EPOLL
        _ecore_main_fdh_poll_mark_active();
#else /* HAVE_EPOLL */
        Ecore_Fd_Handler *fdh;

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
#endif /* HAVE_EPOLL */
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
        fdh = (Ecore_Fd_Handler *) l;
        l = l->next;
        errno = 0;

        if ((fcntl(fdh->fd, F_GETFD) < 0) && (errno == EBADF))
          {
             ERR("Found bad fd at index %d", fdh->fd);
             if (fdh->flags & ECORE_FD_ERROR)
               {
                  ERR("Fd set for error! calling user");
                  fdh->references++;
                  if (!fdh->func(fdh->data, fdh))
                    {
                       ERR("Fd function err returned 0, remove it");
                       if (!fdh->delete_me)
                         {
                            fdh->delete_me = EINA_TRUE;
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
        ECORE_MAGIC_SET(fdh, ECORE_MAGIC_NONE);
        free(fdh);
        fd_handlers_to_delete = eina_list_remove_list(fd_handlers_to_delete, l);
     }
}

#ifdef _WIN32
static void
_ecore_main_win32_handlers_cleanup(void)
{
   Ecore_Win32_Handler *wh;
   Eina_Inlist *l;
   int deleted_in_use = 0;

   if (!win32_handlers_delete_me) return;
   for (l = EINA_INLIST_GET(win32_handlers); l; )
     {
        wh = (Ecore_Win32_Handler *)l;

        l = l->next;
        if (wh->delete_me)
          {
             if (wh->references)
               {
                  deleted_in_use++;
                  continue;
               }

             win32_handlers = (Ecore_Win32_Handler *)
                eina_inlist_remove(EINA_INLIST_GET(win32_handlers),
                                   EINA_INLIST_GET(wh));
             ECORE_MAGIC_SET(wh, ECORE_MAGIC_NONE);
             free(wh);
          }
     }
   if (!deleted_in_use) win32_handlers_delete_me = EINA_FALSE;
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
                  if (!fdh->func(fdh->data, fdh))
                    {
                       if (!fdh->delete_me)
                         {
                            fdh->delete_me = EINA_TRUE;
                            fd_handlers_to_delete = eina_list_append(fd_handlers_to_delete, fdh);
                         }

                    }
                  fdh->references--;
                  _ecore_fd_valid();

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
}

static int
_ecore_main_fd_handlers_buf_call(void)
{
   Ecore_Fd_Handler *fdh;
   Eina_List *l, *l2;
   int ret;

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
             if (fdh->buf_func(fdh->buf_data, fdh))
               {
                  ret |= fdh->func(fdh->data, fdh);
                  fdh->read_active = EINA_TRUE;
                  _ecore_try_add_to_call_list(fdh);
               }
             fdh->references--;
          }
        else
          fd_handlers_with_buffer = eina_list_remove_list(fd_handlers_with_buffer, l);
     }
   return ret;
}

#ifndef USE_G_MAIN_LOOP
static void
_ecore_main_loop_iterate_internal(int once_only)
{
   double next_time = -1.0;
   int    have_event = 0;
   int    have_signal;

   in_main_loop++;
   /* expire any timers */
   while (_ecore_timer_call(_ecore_time_loop_time));
   _ecore_timer_cleanup();

   /* process signals into events .... */
   while (_ecore_signal_count_get()) _ecore_signal_call();
   if (_ecore_event_exist())
     {
        _ecore_idle_enterer_call();
        _ecore_throttle();
        have_event = 1;
        _ecore_main_select(0.0);
        _ecore_timer_enable_new();
        goto process_events;
     }
   /* call idle enterers ... */
   if (!once_only)
     {
        _ecore_idle_enterer_call();
        _ecore_throttle();
     }
   else
     {
        have_event = have_signal = 0;

        if (_ecore_main_select(0.0) > 0) have_event = 1;
        if (_ecore_signal_count_get() > 0) have_signal = 1;
        if (have_signal || have_event)
          {
             _ecore_timer_enable_new();
             goto process_events;
          }
     }

   /* if these calls caused any buffered events to appear - deal with them */
   if (fd_handlers_with_buffer)
     _ecore_main_fd_handlers_buf_call();

   /* if there are any - jump to processing them */
   if (_ecore_event_exist())
     {
        have_event = 1;
        _ecore_main_select(0.0);
        _ecore_timer_enable_new();
        goto process_events;
     }
   if (once_only)
     {
        _ecore_idle_enterer_call();
        _ecore_throttle();
        in_main_loop--;
        _ecore_timer_enable_new();
        return;
     }

   if (_ecore_fps_debug)
     {
        t2 = ecore_time_get();
        if ((t1 > 0.0) && (t2 > 0.0))
           _ecore_fps_debug_runtime_add(t2 - t1);
     }
   start_loop:
   /* any timers re-added as a result of these are allowed to go */
   _ecore_timer_enable_new();
   if (do_quit)
     {
        in_main_loop--;
        _ecore_timer_enable_new();
        return;
     }
   if (!_ecore_event_exist())
     {
        /* init flags */
        have_event = have_signal = 0;
        next_time = _ecore_timer_next_get();
        /* no timers */
        if (next_time < 0)
          {
             /* no idlers */
             if (!_ecore_idler_exist())
               {
                  if (_ecore_main_select(-1.0) > 0) have_event = 1;
               }
             /* idlers */
             else
               {
                  for (;;)
                    {
                       _ecore_time_loop_time = ecore_time_get();
                       if (!_ecore_idler_call()) goto start_loop;
                       if (_ecore_main_select(0.0) > 0) break;
                       if (_ecore_event_exist()) break;
                       if (_ecore_signal_count_get() > 0) break;
                       if (_ecore_timers_exists()) goto start_loop;
                       if (do_quit) break;
                    }
               }
          }
        /* timers */
        else
          {
             /* no idlers */
             if (!_ecore_idler_exist())
               {
                  if (_ecore_main_select(next_time) > 0) have_event = 1;
               }
             /* idlers */
             else
               {
                  for (;;)
                    {
                       _ecore_time_loop_time = ecore_time_get();
                       if (!_ecore_idler_call()) goto start_loop;
                       if (_ecore_main_select(0.0) > 0) break;
                       if (_ecore_event_exist()) break;
                       if (_ecore_signal_count_get() > 0) break;
                       if (have_event || have_signal) break;
                       next_time = _ecore_timer_next_get();
                       if (next_time <= 0) break;
                       if (do_quit) break;
                    }
               }
          }
     }
   if (_ecore_fps_debug) t1 = ecore_time_get();
   /* we came out of our "wait state" so idle has exited */
   if (!once_only) _ecore_idle_exiter_call();
   /* call the fd handler per fd that became alive... */
   /* this should read or write any data to the monitored fd and then */
   /* post events onto the ecore event pipe if necessary */
   process_events:
   _ecore_main_fd_handlers_call();
   if (fd_handlers_with_buffer)
     _ecore_main_fd_handlers_buf_call();
   /* process signals into events .... */
   while (_ecore_signal_count_get()) _ecore_signal_call();
   /* handle events ... */
   _ecore_event_call();
   _ecore_main_fd_handlers_cleanup();

   if (once_only)
     {
        _ecore_idle_enterer_call();
        _ecore_throttle();
     }
   in_main_loop--;
}
#endif

#ifdef _WIN32
static int
_ecore_main_win32_select(int nfds __UNUSED__, fd_set *readfds, fd_set *writefds,
                         fd_set *exceptfds, struct timeval *tv)
{
   HANDLE       objects[MAXIMUM_WAIT_OBJECTS];
   int          sockets[MAXIMUM_WAIT_OBJECTS];
   Ecore_Fd_Handler *fdh;
   Ecore_Win32_Handler *wh;
   unsigned int objects_nbr = 0;
   unsigned int handles_nbr = 0;
   unsigned int events_nbr = 0;
   DWORD        result;
   DWORD        timeout;
   MSG          msg;
   unsigned int i;
   int          res;

   /* Create an event object per socket */
   EINA_INLIST_FOREACH(fd_handlers, fdh)
     {
        WSAEVENT event;
        long network_event;

        network_event = 0;
        if (FD_ISSET(fdh->fd, readfds))
          network_event |= FD_READ;
        if (FD_ISSET(fdh->fd, writefds))
          network_event |= FD_WRITE;
        if (FD_ISSET(fdh->fd, exceptfds))
          network_event |= FD_OOB;

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

   /* store the HANDLEs in the objects to wait for */
   EINA_INLIST_FOREACH(win32_handlers, wh)
     {
        objects[objects_nbr] = wh->h;
        handles_nbr++;
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

   if (timeout == 0) return 0;

   result = MsgWaitForMultipleObjects(objects_nbr, (const HANDLE *)objects, EINA_FALSE,
                                      timeout, QS_ALLINPUT);

   FD_ZERO(readfds);
   FD_ZERO(writefds);
   FD_ZERO(exceptfds);

   /* The result tells us the type of event we have. */
   if (result == WAIT_FAILED)
     {
        char *msg;

        msg = evil_last_error_get();
        ERR(" * %s\n", msg);
        free(msg);
        res = -1;
     }
   else if (result == WAIT_TIMEOUT)
     {
        /* ERR("time out\n"); */
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
   else if ((result >= 0) && (result < WAIT_OBJECT_0 + events_nbr))
     {
        WSANETWORKEVENTS network_event;

        WSAEnumNetworkEvents(sockets[result], objects[result], &network_event);

        if (network_event.lNetworkEvents & FD_READ)
           FD_SET(sockets[result], readfds);
        if (network_event.lNetworkEvents & FD_WRITE)
           FD_SET(sockets[result], writefds);
        if (network_event.lNetworkEvents & FD_OOB)
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

        while (win32_handler_current)
          {
             wh = win32_handler_current;

             if (objects[result - WAIT_OBJECT_0] == wh->h)
               {
                  if (!wh->delete_me)
                    {
                       wh->references++;
                       if (!wh->func(wh->data, wh))
                         {
                            wh->delete_me = EINA_TRUE;
                            win32_handlers_delete_me = EINA_TRUE;
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

   /* Remove event objects again */
   for (i = 0; i < events_nbr; i++) WSACloseEvent(objects[i]);

   return res;
}
#endif
