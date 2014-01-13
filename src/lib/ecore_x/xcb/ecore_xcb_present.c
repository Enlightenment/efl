#include "ecore_xcb_private.h"
# ifdef ECORE_XCB_XPRESENT
#  include <xcb/present.h>
# endif

/* local variables */
static Eina_Bool _xpresent_avail = EINA_FALSE;

/* external variables */
int _ecore_xcb_event_xpresent = -1;

void
_ecore_xcb_xpresent_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XPRESENT
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_present_id);
#endif
}

void
_ecore_xcb_xpresent_finalize(void)
{
#ifdef ECORE_XCB_XPRESENT
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XPRESENT
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_present_id);
   if ((ext_reply) && (ext_reply->present))
     {
        xcb_present_query_version_cookie_t cookie;
        xcb_present_query_version_reply_t *reply;

        cookie =
          xcb_present_query_version_unchecked(_ecore_xcb_conn,
                                             XCB_PRESENT_MAJOR_VERSION,
                                             XCB_PRESENT_MINOR_VERSION);
        reply = xcb_present_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
                _xpresent_avail = EINA_TRUE;
                free(reply);
          }

        if (_xpresent_avail)
          _ecore_xcb_event_xpresent = ext_reply->first_event;
     }
#endif
}

#ifdef ECORE_XCB_XPRESENT
#define SET(X) e->X = ev->X

static void
_present_configure(xcb_present_configure_notify_event_t *ev)
{
   Ecore_X_Event_Present_Configure *e;

   e = calloc(1, sizeof(Ecore_X_Event_Present_Configure));
   if (!e) return;

   e->win = ev->window;
   SET(x), SET(y);
   SET(width), SET(height);
   SET(off_x), SET(off_y);
   SET(pixmap_width), SET(pixmap_height);
   SET(pixmap_flags);

   ecore_event_add(ECORE_X_EVENT_PRESENT_CONFIGURE, e, NULL, NULL);
}

static void
_present_complete(xcb_present_complete_notify_event_t *ev)
{
   unsigned int mode[] =
   {
    [XCB_PRESENT_COMPLETE_MODE_COPY] = ECORE_X_PRESENT_COMPLETE_MODE_COPY,
    [XCB_PRESENT_COMPLETE_MODE_FLIP] = ECORE_X_PRESENT_COMPLETE_MODE_FLIP,
    [XCB_PRESENT_COMPLETE_MODE_SKIP] = ECORE_X_PRESENT_COMPLETE_MODE_SKIP,
   };
   Ecore_X_Event_Present_Complete *e;

   e = calloc(1, sizeof(Ecore_X_Event_Present_Complete));
   if (!e) return;

   e->win = ev->window;
   SET(serial);
   SET(ust), SET(msc);
   e->kind = (ev->kind == XCB_PRESENT_COMPLETE_KIND_NOTIFY_MSC);
   e->mode = mode[ev->mode];
   ecore_event_add(ECORE_X_EVENT_PRESENT_COMPLETE, e, NULL, NULL);
}

static void
_present_idle(xcb_present_idle_notify_event_t *ev)
{
   Ecore_X_Event_Present_Idle *e;

   e = calloc(1, sizeof(Ecore_X_Event_Present_Idle));
   if (!e) return;

   e->win = ev->window;
   SET(serial);
   SET(pixmap);
   SET(idle_fence);
   ecore_event_add(ECORE_X_EVENT_PRESENT_IDLE, e, NULL, NULL);
}
#undef SET

void
_ecore_xcb_event_handle_present_event(xcb_ge_event_t *ev)
{
   switch (ev->event_type)
     {
      case XCB_PRESENT_EVENT_CONFIGURE_NOTIFY:
        _present_configure((xcb_present_configure_notify_event_t*)(long)ev->pad1);
        break;
      case XCB_PRESENT_EVENT_COMPLETE_NOTIFY:
        _present_complete((xcb_present_complete_notify_event_t*)(long)ev->pad1);
        break;
      case XCB_PRESENT_EVENT_IDLE_NOTIFY:
        _present_idle((xcb_present_idle_notify_event_t*)(long)ev->pad1);
        break;
      default: break;
     }
}
#endif


EAPI void
ecore_x_present_select_events(Ecore_X_Window win, unsigned int events)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
#ifdef ECORE_XCB_XPRESENT
   CHECK_XCB_CONN;
   xcb_present_select_input(_ecore_xcb_conn, _ecore_xcb_event_xpresent, win, events);
#else
   (void)win;
   (void)events;
#endif
}

EAPI void
ecore_x_present_notify_msc(Ecore_X_Window win, unsigned int serial, unsigned long long target_msc, unsigned long long divisor, unsigned long long remainder)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
#ifdef ECORE_XCB_XPRESENT
   CHECK_XCB_CONN;
   xcb_present_notify_msc(_ecore_xcb_conn, win, serial, target_msc, divisor, remainder);
#else
   (void)win;
   (void)serial;
   (void)target_msc;
   (void)divisor;
   (void)remainder;
#endif
}

EAPI void
ecore_x_present_pixmap(Ecore_X_Window win, Ecore_X_Pixmap pixmap, unsigned int serial, Ecore_X_Region valid,
                       Ecore_X_Region update, int x_off, int y_off, Ecore_X_Randr_Crtc target_crtc,
                       Ecore_X_Sync_Fence wait_fence, Ecore_X_Sync_Fence idle_fence, unsigned int options,
                       unsigned long long target_msc, unsigned long long divisor, unsigned long long remainder,
                       Ecore_X_Present *notifies, int num_notifies)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
#ifdef ECORE_XCB_XPRESENT
   CHECK_XCB_CONN;
   xcb_present_pixmap(_ecore_xcb_conn, win, pixmap, serial, valid, update,
                  x_off, y_off, target_crtc, wait_fence, idle_fence, options, target_msc,
                  divisor, remainder, num_notifies, (xcb_present_notify_t*)notifies);
#else
   (void)win;
   (void)pixmap;
   (void)serial;
   (void)valid;
   (void)update;
   (void)x_off;
   (void)y_off;
   (void)target_crtc;
   (void)wait_fence;
   (void)idle_fence;
   (void)options;
   (void)target_msc;
   (void)divisor;
   (void)remainder;
   (void)notifies;
   (void)num_notifies;
#endif
}

EAPI Eina_Bool
ecore_x_present_exists(void)
{
   return _xpresent_avail;
}
