#include "ecore_xcb_private.h"
# ifdef ECORE_XCB_SCREENSAVER
#  include <xcb/screensaver.h>
# endif

/* local variables */
static Eina_Bool _screensaver_avail = EINA_FALSE;

/* external variables */
int _ecore_xcb_event_screensaver = -1;

void
_ecore_xcb_screensaver_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SCREENSAVER
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_screensaver_id);
#endif
}

void
_ecore_xcb_screensaver_finalize(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SCREENSAVER
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_screensaver_id);
   if ((ext_reply) && (ext_reply->present))
     {
        xcb_screensaver_query_version_cookie_t cookie;
        xcb_screensaver_query_version_reply_t *reply;

        cookie =
          xcb_screensaver_query_version_unchecked(_ecore_xcb_conn,
                                                  XCB_SCREENSAVER_MAJOR_VERSION,
                                                  XCB_SCREENSAVER_MINOR_VERSION);
        reply =
          xcb_screensaver_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
             if ((reply->server_major_version >= XCB_SCREENSAVER_MAJOR_VERSION) &&
                 (reply->server_minor_version >= XCB_SCREENSAVER_MINOR_VERSION))
               _screensaver_avail = EINA_TRUE;

             free(reply);
          }

        if (_screensaver_avail)
          _ecore_xcb_event_screensaver = ext_reply->first_event;
     }
#endif
}

EAPI int
ecore_x_screensaver_idle_time_get(void)
{
   int ret = 0;
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_query_info_cookie_t cookie;
   xcb_screensaver_query_info_reply_t *reply;
   Ecore_X_Window root;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return 0;

#ifdef ECORE_XCB_SCREENSAVER
   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   cookie = xcb_screensaver_query_info_unchecked(_ecore_xcb_conn, root);
   reply = xcb_screensaver_query_info_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   ret = (reply->ms_until_server / 1000);
   free(reply);
#endif

   return ret;
}

EAPI void
ecore_x_screensaver_set(int timeout,
                        int interval,
                        int prefer_blanking,
                        int allow_exposures)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return;

#ifdef ECORE_XCB_SCREENSAVER
   xcb_set_screen_saver(_ecore_xcb_conn,
                        timeout, interval, prefer_blanking, allow_exposures);
#endif
}

EAPI void
ecore_x_screensaver_timeout_set(int timeout)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
   uint16_t pint;
   uint8_t pblank, pexpo;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   pint = reply->interval;
   pblank = reply->prefer_blanking;
   pexpo = reply->allow_exposures;
   free(reply);
   xcb_set_screen_saver(_ecore_xcb_conn, timeout, pint, pblank, pexpo);
#endif
}

EAPI int
ecore_x_screensaver_timeout_get(void)
{
   int timeout = 0;
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return 0;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   timeout = reply->timeout;
   free(reply);
#endif

   return timeout;
}

EAPI void
ecore_x_screensaver_blank_set(int blank)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
   uint16_t pint, pto;
   uint8_t pexpo;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   pto = reply->timeout;
   pint = reply->interval;
   pexpo = reply->allow_exposures;
   free(reply);
   xcb_set_screen_saver(_ecore_xcb_conn, pto, pint, blank, pexpo);
#endif
}

EAPI int
ecore_x_screensaver_blank_get(void)
{
   int blank = 0;
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return 0;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   blank = reply->prefer_blanking;
   free(reply);
#endif

   return blank;
}

EAPI void
ecore_x_screensaver_expose_set(int expose)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
   uint16_t pint, pto;
   uint8_t pblank;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   pto = reply->timeout;
   pint = reply->interval;
   pblank = reply->prefer_blanking;
   free(reply);
   xcb_set_screen_saver(_ecore_xcb_conn, pto, pint, pblank, expose);
#endif
}

EAPI int
ecore_x_screensaver_expose_get(void)
{
   int expose = 0;
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return 0;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   expose = reply->allow_exposures;
   free(reply);
#endif

   return expose;
}

EAPI void
ecore_x_screensaver_interval_set(int interval)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
   uint16_t pto;
   uint8_t pblank, pexpose;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   pto = reply->timeout;
   pblank = reply->prefer_blanking;
   pexpose = reply->allow_exposures;
   free(reply);
   xcb_set_screen_saver(_ecore_xcb_conn, pto, interval, pblank, pexpose);
#endif
}

EAPI int
ecore_x_screensaver_interval_get(void)
{
   int interval = 0;
#ifdef ECORE_XCB_SCREENSAVER
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return 0;

#ifdef ECORE_XCB_SCREENSAVER
   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   interval = reply->interval;
   free(reply);
#endif

   return interval;
}

EAPI void
ecore_x_screensaver_event_listen_set(Eina_Bool on)
{
#ifdef ECORE_XCB_SCREENSAVER
   Ecore_X_Window root;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_screensaver_avail) return;

#ifdef ECORE_XCB_SCREENSAVER
   root = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   if (on)
     xcb_screensaver_select_input(_ecore_xcb_conn, root,
                                  XCB_SCREENSAVER_EVENT_NOTIFY_MASK | 
                                  XCB_SCREENSAVER_EVENT_CYCLE_MASK);
   else
     xcb_screensaver_select_input(_ecore_xcb_conn, root, 0);
#endif
}

EAPI Eina_Bool
ecore_x_screensaver_event_available_get(void)
{
   return _screensaver_avail;
}

EAPI Eina_Bool
ecore_x_screensaver_custom_blanking_enable(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   uint32_t mask_list[9];
   
   xcb_screensaver_set_attributes_checked
     (_ecore_xcb_conn,
         ((xcb_screen_t *)_ecore_xcb_screen)->root,
         -9999, -9999, 1, 1, 0,
         XCB_WINDOW_CLASS_INPUT_ONLY,
         XCB_COPY_FROM_PARENT, XCB_COPY_FROM_PARENT,
         0, mask_list);
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
ecore_x_screensaver_custom_blanking_disable(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_unset_attributes_checked
     (_ecore_xcb_conn,
         ((xcb_screen_t *)_ecore_xcb_screen)->root);
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}


EAPI void
ecore_x_screensaver_supend(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_suspend(_ecore_xcb_conn, 1);
#endif
}

EAPI void
ecore_x_screensaver_resume(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_suspend(_ecore_xcb_conn, 0);
#endif
}
