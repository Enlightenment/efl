#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ecore.h>

#include "Ecore_Avahi.h"

#ifdef HAVE_AVAHI
#include <avahi-common/watch.h>

typedef struct _Ecore_Avahi_Watch Ecore_Avahi_Watch;
typedef struct _Ecore_Avahi_Timeout Ecore_Avahi_Timeout;

struct _Ecore_Avahi_Watch
{
   Ecore_Fd_Handler  *handler;
   Ecore_Avahi       *parent;

   AvahiWatchCallback callback;
   void              *callback_data;
};

struct _Ecore_Avahi_Timeout
{
   Ecore_Timer         *timer;
   Ecore_Avahi         *parent;

   AvahiTimeoutCallback callback;
   void                *callback_data;
};

struct _Ecore_Avahi
{
   AvahiPoll  api;

   Eina_List *watches;
   Eina_List *timeouts;
};

static Ecore_Fd_Handler_Flags
_ecore_avahi_events2ecore(AvahiWatchEvent events)
{
   return (events & AVAHI_WATCH_IN ? ECORE_FD_READ : 0) |
     (events & AVAHI_WATCH_OUT ? ECORE_FD_WRITE : 0) |
     ECORE_FD_ERROR;
}

static Eina_Bool
_ecore_avahi_watch_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Avahi_Watch *watch = data;
   AvahiWatchEvent flags = 0;

   flags = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ) ? AVAHI_WATCH_IN : 0;
   flags |= ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE) ? AVAHI_WATCH_OUT : 0;
   flags |= ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR) ? AVAHI_WATCH_ERR : 0;

   watch->callback((AvahiWatch*) watch, ecore_main_fd_handler_fd_get(fd_handler), flags, watch->callback_data);

   return ECORE_CALLBACK_RENEW;
}

static AvahiWatch *
_ecore_avahi_watch_new(const AvahiPoll *api,
                       int fd, AvahiWatchEvent events,
                       AvahiWatchCallback callback, void *userdata)
{
   Ecore_Avahi_Watch *watch;
   Ecore_Avahi *ea;

   ea = api->userdata;
   watch = calloc(1, sizeof (Ecore_Avahi_Watch));
   if (!watch) return NULL;

   watch->handler = ecore_main_fd_handler_add(fd, _ecore_avahi_events2ecore(events),
                                              _ecore_avahi_watch_cb, watch, NULL, NULL);
   watch->callback = callback;
   watch->callback_data = userdata;
   watch->parent = ea;

   ea->watches = eina_list_append(ea->watches, watch);

   return (AvahiWatch*) watch;
}

static void
_ecore_avahi_watch_update(AvahiWatch *w, AvahiWatchEvent events)
{
   Ecore_Avahi_Watch *watch = (Ecore_Avahi_Watch *) w;

   ecore_main_fd_handler_active_set(watch->handler, _ecore_avahi_events2ecore(events));
}

static void
_ecore_avahi_watch_free(AvahiWatch *w)
{
   Ecore_Avahi_Watch *watch = (Ecore_Avahi_Watch *) w;

   ecore_main_fd_handler_del(watch->handler);
   watch->parent->watches = eina_list_remove(watch->parent->watches, watch);
   free(watch);
}

static AvahiWatchEvent
_ecore_avahi_watch_get_events(AvahiWatch *w)
{
   Ecore_Avahi_Watch *watch = (Ecore_Avahi_Watch *) w;
   AvahiWatchEvent flags = 0;

   flags = ecore_main_fd_handler_active_get(watch->handler, ECORE_FD_READ) ? AVAHI_WATCH_IN : 0;
   flags |= ecore_main_fd_handler_active_get(watch->handler, ECORE_FD_WRITE) ? AVAHI_WATCH_OUT : 0;
   flags |= ecore_main_fd_handler_active_get(watch->handler, ECORE_FD_ERROR) ? AVAHI_WATCH_ERR : 0;

   return flags;
}

static double
_ecore_avahi_timeval2double(const struct timeval *tv)
{
   struct timeval now;
   double tm;

   if (!tv) return 3600;

   gettimeofday(&now, NULL);

   tm = tv->tv_sec - now.tv_sec + (double) (tv->tv_usec - now.tv_usec) / 1000000;
   if (tm < 0) tm = 0.001;

   return tm;
}

static Eina_Bool
_ecore_avahi_timeout_cb(void *data)
{
   Ecore_Avahi_Timeout *timeout = data;

   timeout->callback((AvahiTimeout*) timeout, timeout->callback_data);

   timeout->timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static AvahiTimeout *
_ecore_avahi_timeout_new(const AvahiPoll *api, const struct timeval *tv,
                         AvahiTimeoutCallback callback, void *userdata)
{
   Ecore_Avahi_Timeout *timeout;
   Ecore_Avahi *ea;

   ea = api->userdata;
   timeout = calloc(1, sizeof (Ecore_Avahi_Timeout));
   if (!timeout) return NULL;
   if (tv) timeout->timer = ecore_timer_add(_ecore_avahi_timeval2double(tv),
                                            _ecore_avahi_timeout_cb, timeout);
   timeout->callback = callback;
   timeout->callback_data = userdata;
   timeout->parent = ea;

   ea->timeouts = eina_list_append(ea->timeouts, timeout);

   return (AvahiTimeout*) timeout;
}

static void
_ecore_avahi_timeout_update(AvahiTimeout *t, const struct timeval *tv)
{
   Ecore_Avahi_Timeout *timeout = (Ecore_Avahi_Timeout *) t;

   if (timeout->timer) ecore_timer_del(timeout->timer);
   timeout->timer = NULL;

   if (tv)
     timeout->timer = ecore_timer_add(_ecore_avahi_timeval2double(tv),
                                      _ecore_avahi_timeout_cb, timeout);
}

static void
_ecore_avahi_timeout_free(AvahiTimeout *t)
{
   Ecore_Avahi_Timeout *timeout = (Ecore_Avahi_Timeout *) t;

   if (timeout->timer) ecore_timer_del(timeout->timer);
   timeout->parent->timeouts = eina_list_remove(timeout->parent->timeouts, timeout);
   free(timeout);
}
#endif

EAPI Ecore_Avahi *
ecore_avahi_add(void)
{
#ifdef HAVE_AVAHI
   Ecore_Avahi *handler;

   handler = calloc(1, sizeof (Ecore_Avahi));
   if (!handler) return NULL;

   handler->api.userdata = handler;
   handler->api.watch_new = _ecore_avahi_watch_new;
   handler->api.watch_free = _ecore_avahi_watch_free;
   handler->api.watch_update = _ecore_avahi_watch_update;
   handler->api.watch_get_events = _ecore_avahi_watch_get_events;

   handler->api.timeout_new = _ecore_avahi_timeout_new;
   handler->api.timeout_free = _ecore_avahi_timeout_free;
   handler->api.timeout_update = _ecore_avahi_timeout_update;

   return handler;
#else
   return NULL;
#endif
}

EAPI void
ecore_avahi_del(Ecore_Avahi *handler)
{
#ifdef HAVE_AVAHI
   Ecore_Avahi_Timeout *timeout;
   Ecore_Avahi_Watch *watch;

   EINA_LIST_FREE(handler->watches, watch)
     {
        ecore_main_fd_handler_del(watch->handler);
        free(watch);
     }

   EINA_LIST_FREE(handler->timeouts, timeout)
     {
        ecore_timer_del(timeout->timer);
        free(timeout);
     }

   free(handler);
#else
   (void) handler;
#endif
}

EAPI const void *
ecore_avahi_poll_get(Ecore_Avahi *handler)
{
#ifdef HAVE_AVAHI
   if (!handler) return NULL;
   return &handler->api;
#else
   (void)handler;
   return NULL;
#endif
}

