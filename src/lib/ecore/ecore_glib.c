#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "Ecore.h"
#include "ecore_private.h"

#ifdef HAVE_GLIB
# include <glib.h>

static Eina_Bool _ecore_glib_active = EINA_FALSE;
static Ecore_Select_Function _ecore_glib_select_original;
static GPollFD *_ecore_glib_fds = NULL;
static size_t _ecore_glib_fds_size = 0;
static const size_t ECORE_GLIB_FDS_INITIAL = 128;
static const size_t ECORE_GLIB_FDS_STEP = 8;
static const size_t ECORE_GLIB_FDS_MAX_FREE = 256;
static GMutex *_ecore_glib_select_lock;
static GCond *_ecore_glib_select_cond;

static Eina_Bool
_ecore_glib_fds_resize(size_t size)
{
   void *tmp = realloc(_ecore_glib_fds, sizeof(GPollFD) * size);

   if (!tmp)
     {
        ERR("Could not realloc from %zu to %zu buckets.",
            _ecore_glib_fds_size, size);
        return EINA_FALSE;
     }

   _ecore_glib_fds = tmp;
   _ecore_glib_fds_size = size;
   return EINA_TRUE;
}

static int
_ecore_glib_context_query(GMainContext *ctx,
                          int           priority,
                          int          *p_timer)
{
   int reqfds;

   if (_ecore_glib_fds_size == 0)
     {
        if (!_ecore_glib_fds_resize(ECORE_GLIB_FDS_INITIAL)) return -1;
     }

   while (1)
     {
        size_t size;

        reqfds = g_main_context_query
            (ctx, priority, p_timer, _ecore_glib_fds, _ecore_glib_fds_size);
        if (reqfds <= (int)_ecore_glib_fds_size) break;

        size = (1 + reqfds / ECORE_GLIB_FDS_STEP) * ECORE_GLIB_FDS_STEP;
        if (!_ecore_glib_fds_resize(size)) return -1;
     }

   if (reqfds + ECORE_GLIB_FDS_MAX_FREE < _ecore_glib_fds_size)
     {
        size_t size;

        size = (1 + reqfds / ECORE_GLIB_FDS_MAX_FREE) * ECORE_GLIB_FDS_MAX_FREE;
        _ecore_glib_fds_resize(size);
     }

   return reqfds;
}

static int
_ecore_glib_context_poll_from(const GPollFD *pfds,
                              int            count,
                              fd_set        *rfds,
                              fd_set        *wfds,
                              fd_set        *efds)
{
   const GPollFD *itr = pfds, *itr_end = pfds + count;
   int glib_fds = -1;

   for (; itr < itr_end; itr++)
     {
        if (glib_fds < itr->fd)
          glib_fds = itr->fd;

        if (itr->events & G_IO_IN)
          FD_SET(itr->fd, rfds);
        if (itr->events & G_IO_OUT)
          FD_SET(itr->fd, wfds);
        if (itr->events & (G_IO_HUP | G_IO_ERR))
          FD_SET(itr->fd, efds);
     }

   return glib_fds + 1;
}

static int
_ecore_glib_context_poll_to(GPollFD      *pfds,
                            int           count,
                            const fd_set *rfds,
                            const fd_set *wfds,
                            const fd_set *efds,
                            int           ready)
{
   GPollFD *itr = pfds, *itr_end = pfds + count;

   for (; (itr < itr_end) && (ready > 0); itr++)
     {
        itr->revents = 0;
        if (FD_ISSET(itr->fd, rfds) && (itr->events & G_IO_IN))
          {
             itr->revents |= G_IO_IN;
             ready--;
          }
        if (FD_ISSET(itr->fd, wfds) && (itr->events & G_IO_OUT))
          {
             itr->revents |= G_IO_OUT;
             ready--;
          }
        if (FD_ISSET(itr->fd, efds) && (itr->events & (G_IO_HUP | G_IO_ERR)))
          {
             itr->revents |= G_IO_ERR;
             ready--;
          }
     }
   return ready;
}

static int
_ecore_glib_select__locked(GMainContext   *ctx,
                           int             ecore_fds,
                           fd_set         *rfds,
                           fd_set         *wfds,
                           fd_set         *efds,
                           struct timeval *ecore_timeout)
{
   int priority, maxfds, glib_fds, reqfds, reqtimeout, ret;
   struct timeval *timeout, glib_timeout;

   g_main_context_prepare(ctx, &priority);
   reqfds = _ecore_glib_context_query(ctx, priority, &reqtimeout);
   if (reqfds < 0) goto error;

   glib_fds = _ecore_glib_context_poll_from
       (_ecore_glib_fds, reqfds, rfds, wfds, efds);

   if (reqtimeout == -1)
     timeout = ecore_timeout;
   else
     {
        glib_timeout.tv_sec = reqtimeout / 1000;
        glib_timeout.tv_usec = (reqtimeout % 1000) * 1000;

        if (!ecore_timeout || timercmp(ecore_timeout, &glib_timeout, >))
          timeout = &glib_timeout;
        else
          timeout = ecore_timeout;
     }

   maxfds = (ecore_fds >= glib_fds) ? ecore_fds : glib_fds;
   ret = _ecore_glib_select_original(maxfds, rfds, wfds, efds, timeout);

   ret = _ecore_glib_context_poll_to
       (_ecore_glib_fds, reqfds, rfds, wfds, efds, ret);

   if (g_main_context_check(ctx, priority, _ecore_glib_fds, reqfds))
     g_main_context_dispatch(ctx);

   return ret;

error:
   return _ecore_glib_select_original
            (ecore_fds, rfds, wfds, efds, ecore_timeout);
}

static int
_ecore_glib_select(int             ecore_fds,
                   fd_set         *rfds,
                   fd_set         *wfds,
                   fd_set         *efds,
                   struct timeval *ecore_timeout)
{
   GMainContext *ctx;
   int ret;

   ctx = g_main_context_default();

   if (g_main_context_acquire(ctx))
     {
        g_mutex_lock(_ecore_glib_select_lock);
     }
   else
     {
        while (!g_main_context_wait(ctx, _ecore_glib_select_cond,
                                    _ecore_glib_select_lock))
          g_thread_yield();
     }

   ret = _ecore_glib_select__locked
       (ctx, ecore_fds, rfds, wfds, efds, ecore_timeout);

   g_mutex_unlock(_ecore_glib_select_lock);
   g_main_context_release(ctx);

   return ret;
}

#endif

void
_ecore_glib_init(void)
{
#ifdef HAVE_GLIB
#if GLIB_CHECK_VERSION(2,32,0)
   _ecore_glib_select_lock = malloc(sizeof(GMutex));
   g_mutex_init(_ecore_glib_select_lock);
   _ecore_glib_select_cond = malloc(sizeof(GCond));
   g_cond_init(_ecore_glib_select_cond);
#else
   if (!g_thread_get_initialized()) g_thread_init(NULL);
   _ecore_glib_select_lock = g_mutex_new();
   _ecore_glib_select_cond = g_cond_new();
#endif
#endif
}

void
_ecore_glib_shutdown(void)
{
#ifdef HAVE_GLIB
   if (!_ecore_glib_active) return;
   _ecore_glib_active = EINA_FALSE;

   if (ecore_main_loop_select_func_get() == _ecore_glib_select)
     ecore_main_loop_select_func_set(_ecore_glib_select_original);

   if (_ecore_glib_fds)
     {
        free(_ecore_glib_fds);
        _ecore_glib_fds = NULL;
     }
   _ecore_glib_fds_size = 0;

#if GLIB_CHECK_VERSION(2,32,0)
   g_mutex_clear(_ecore_glib_select_lock);
   free(_ecore_glib_select_lock);
   _ecore_glib_select_lock = NULL;
   g_cond_clear(_ecore_glib_select_cond);
   free(_ecore_glib_select_cond);
   _ecore_glib_select_cond = NULL;
#else
   g_mutex_free(_ecore_glib_select_lock);
   _ecore_glib_select_lock = NULL;
   g_cond_free(_ecore_glib_select_cond);
   _ecore_glib_select_cond = NULL;
#endif
#endif
}

EAPI Eina_Bool
ecore_main_loop_glib_integrate(void)
{
#ifdef HAVE_GLIB
   void *func;

   if (_ecore_glib_active) return EINA_TRUE;
   func = ecore_main_loop_select_func_get();
   if (func == _ecore_glib_select) return EINA_TRUE;
   _ecore_glib_select_original = func;
   ecore_main_loop_select_func_set(_ecore_glib_select);
   _ecore_glib_active = EINA_TRUE;
   return EINA_TRUE;
#else
   ERR("No glib support");
   return EINA_FALSE;
#endif
}

Eina_Bool _ecore_glib_always_integrate = 1;

EAPI void
ecore_main_loop_glib_always_integrate_disable(void)
{
   _ecore_glib_always_integrate = 0;
}
