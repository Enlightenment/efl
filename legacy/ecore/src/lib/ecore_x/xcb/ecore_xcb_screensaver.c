/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_ScrenSaver_Group X Shape extension
 *
 * These functions use the shape extension of the X server to change
 * shape of given windows.
 */


/**
 * Return whether the X server supports the ScrenSaver Extension.
 * @return 1 if the X ScrenSaver Extension is available, 0 otherwise.
 *
 * Return 1 if the X server supports the ScrenSaver Extension version 1.0,
 * 0 otherwise.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI int
ecore_x_screensaver_event_available_get(void)
{
   return 1;
}


/**
 * Sends the QueryInfo request.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_idle_time_prefetch(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_query_info_cookie_t cookie;

   cookie = xcb_screensaver_query_info_unchecked(_ecore_xcb_conn, ((xcb_screen_t *)_ecore_xcb_screen)->root);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_SCREENSAVER */
}


/**
 * Gets the reply of the QueryInfo request sent by ecore_x_get_screensaver_prefetch().
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_idle_time_fetch(void)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_query_info_cookie_t cookie;
   xcb_screensaver_query_info_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_screensaver_query_info_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_SCREENSAVER */
}


/**
 * Get the number of seconds since the last input was received.
 * @return The number of seconds.
 *
 * Get the number of milliseconds since the last input was received
 * from the user on any of the input devices.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI int
ecore_x_screensaver_idle_time_get(void)
{
  int idle = 0;
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_query_info_reply_t *reply;

   reply = _ecore_xcb_reply_get();

   if (!reply) return 0;

   /* FIXME: check if it is ms_since_user_input or ms_until_server */
   idle = reply->ms_since_user_input / 1000;
#endif /* ECORE_XCB_SCREENSAVER */

   return idle;
}


/**
 * Set the parameters of the screen saver.
 * @param timeout  The timeout, in second.
 * @param interval The interval, in seconds.
 * @param blank    0 to disable screen blanking, otherwise enable it.
 * @param expose   Allow Expose generation event or not.
 *
 * Set the parameters of the screen saver. @p timeout is the timeout,
 * in seconds, until the screen saver turns on. @p interval is the
 * interval, in seconds, between screen saver alterations. @p blank
 * specifies how to enable screen blanking. @p expose specifies the
 * screen save control values.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_set(int timeout,
                        int interval,
                        int blank,
                        int expose)
{
   xcb_set_screen_saver(_ecore_xcb_conn,
                        (int16_t)timeout,
                        (int16_t)interval,
                        (uint8_t)blank,
                        (uint8_t)expose);
}


/**
 * Sends the GetScreenSaver request.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_get_screensaver_prefetch(void)
{
   xcb_get_screen_saver_cookie_t cookie;

   cookie = xcb_get_screen_saver_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetScreenSaver request sent by ecore_x_get_screensaver_prefetch().
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_get_screensaver_fetch(void)
{
   xcb_get_screen_saver_cookie_t cookie;
   xcb_get_screen_saver_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_screen_saver_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}


/**
 * Set the timeout of the screen saver.
 * @param  timeout The timeout to set.
 *
 * Set the @p timeout, in seconds, until the screen saver turns on.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_timeout_set(int timeout)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_set_screen_saver(_ecore_xcb_conn,
                        (int16_t)timeout,
                        reply->interval,
                        reply->prefer_blanking,
                        reply->allow_exposures);
}


/**
 * Get the timeout of the screen saver.
 * @return The timeout.
 *
 * Get the @p timeout, in seconds, until the screen saver turns on.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI int
ecore_x_screensaver_timeout_get(void)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0.0;

   return (int)reply->timeout;
}


/**
 * Set the interval of the screen saver.
 * @param  interval The interval to set.
 *
 * Set the @p interval, in seconds, between screen saver alterations.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_interval_set(int interval)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_set_screen_saver(_ecore_xcb_conn,
                        reply->timeout,
                        (int16_t)interval,
                        reply->prefer_blanking,
                        reply->allow_exposures);
}


/**
 * Get the interval of the screen saver.
 * @return The interval.
 *
 * Get the @p interval, in seconds, between screen saver alterations.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI int
ecore_x_screensaver_interval_get(void)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0.0;

   return (int)reply->interval;
}


/**
 * Set the screen blanking.
 * @param  blank The blank to set.
 *
 * @p blank specifies how to enable screen blanking.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_blank_set(int blank)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_set_screen_saver(_ecore_xcb_conn,
                        reply->timeout,
                        reply->interval,
                        (uint8_t)blank,
                        reply->allow_exposures);
}


/**
 * Get the screen blanking.
 * @return The blanking.
 *
 * Get the screen blanking.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI int
ecore_x_screensaver_blank_get(void)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0.0;

   return (int)reply->prefer_blanking;
}


/**
 * Set the screen save control values.
 * @param  expose The expose to set.
 *
 * Set the screen save control values.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_expose_set(int expose)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_set_screen_saver(_ecore_xcb_conn,
                        reply->timeout,
                        reply->interval,
                        reply->prefer_blanking,
                        (uint8_t)expose);
}


/**
 * Get the screen save control values.
 * @return The expose.
 *
 * Get the screen save control values.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_get_screensaver_prefetch(), which sends the GetScreenSaver request,
 * then ecore_x_get_screensaver_fetch(), which gets the reply.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI int
ecore_x_screensaver_expose_get(void)
{
   xcb_get_screen_saver_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0.0;

   return (int)reply->allow_exposures;
}


/**
 * Specifies if the Screen Saver NotifyMask event should be generated.
 * @param on 0 to disable the generation of the event, otherwise enable it.
 *
 * Specifies if the Screen Saver NotifyMask event on the screen
 * associated with drawable should be generated for this client. If
 * @p on is set to @c 0, the generation is disabled, otherwise, it is
 * enabled.
 * @ingroup Ecore_X_ScrenSaver_Group
 */
EAPI void
ecore_x_screensaver_event_listen_set(int on)
{
#ifdef ECORE_XCB_SCREENSAVER
   xcb_screensaver_select_input(_ecore_xcb_conn,
                                ((xcb_screen_t *)_ecore_xcb_screen)->root,
                                on ? XCB_SCREENSAVER_EVENT_NOTIFY_MASK : 0);
#endif /* ECORE_XCB_SCREENSAVER */
}
