/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_DPMS_Group X DPMS Extension Functions
 *
 * Functions related to the X DPMS extension.
 */


#ifdef ECORE_XCB_DPMS
static int _dpms_available = 0;
static xcb_dpms_get_version_cookie_t _ecore_xcb_dpms_init_cookie;
#endif /* ECORE_XCB_DPMS */

/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_dpms_init and
   _ecore_xcb_dpms_init_finalize. The first one gets the cookies and
   the second one gets the replies. */

void
_ecore_x_dpms_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_DPMS
   if (reply && (reply->present))
      _ecore_xcb_dpms_init_cookie = xcb_dpms_get_version_unchecked(_ecore_xcb_conn, 0, 0);
#endif /* ECORE_XCB_DPMS */
}

void
_ecore_x_dpms_init_finalize(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_version_reply_t *reply;

   reply = xcb_dpms_get_version_reply(_ecore_xcb_conn,
                                      _ecore_xcb_dpms_init_cookie, NULL);

   if (reply)
     {
        if (reply->server_major_version >= 1)
          _dpms_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_DPMS */
}


/**
 * Checks if the DPMS extension is available or not.
 * @return @c 1 if the DPMS extension is available, @c 0 otherwise.
 *
 * Return 1 if the X server supports the DPMS Extension version 1.0,
 * 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_query(void)
{
#ifdef ECORE_XCB_DPMS
   return _dpms_available;
#else
   return 0;
#endif /* ECORE_XCB_DPMS */
}


/**
 * Sends the DPMSCapable request.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_capable_get_prefetch(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_capable_cookie_t cookie;

   cookie = xcb_dpms_capable_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Gets the reply of the DPMSCapable request sent by ecore_x_dpms_capable_get_prefetch().
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_capable_get_fetch(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_capable_cookie_t cookie;
   xcb_dpms_capable_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_dpms_capable_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Checks if the X server is capable of DPMS.
 * @return @c 1 if the X server is capable of DPMS, @c 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_capable_get_prefetch(), which sends the DPMSCapable request,
 * then ecore_x_dpms_capable_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_capable_get(void)
{
   int                       capable = 0;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_capable_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   capable = reply->capable;
#endif /* ECORE_XCB_DPMS */

   return capable;
}


/**
 * Sends the DPMSInfo request.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_enable_get_prefetch(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_info_cookie_t cookie;

   cookie = xcb_dpms_info_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Gets the reply of the DPMSInfo request sent by ecore_x_dpms_enable_get_prefetch().
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_enable_get_fetch(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_info_cookie_t cookie;
   xcb_dpms_info_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_dpms_info_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Checks the DPMS state of the display.
 * @return @c 1 if DPMS is enabled, @c 0 otherwise.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_enapable_get_prefetch(), which sends the DPMSInfo request,
 * then ecore_x_dpms_enapable_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_enable_get(void)
{
   int                    enable = 0;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_info_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   enable = reply->state;
#endif /* ECORE_XCB_DPMS */

   return enable;
}


/**
 * Sets the DPMS state of the display.
 * @param enabled @c 0 to disable DPMS characteristics of the server, enable it otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_enabled_set(int enabled)
{
#ifdef ECORE_XCB_DPMS
   if (enabled)
      xcb_dpms_enable(_ecore_xcb_conn);
   else
      xcb_dpms_disable(_ecore_xcb_conn);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Sets the timeouts. The values are in unit of seconds.
 * @param standby Amount of time of inactivity before standby mode will be invoked.
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 * @param off     Amount of time of inactivity before the monitor is shut off.
 * @return        Returns always 1.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_timeouts_set(unsigned int standby,
                          unsigned int suspend,
                          unsigned int off)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_set_timeouts(_ecore_xcb_conn, standby, suspend, off);
#endif /* ECORE_XCB_DPMS */

   return 1;
}


/**
 * Sends the DPMSGetTimeouts request.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeouts_get_prefetch(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_cookie_t cookie;

   cookie = xcb_dpms_get_timeouts_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Gets the reply of the DPMSGetTimeouts request sent by ecore_x_dpms_timeouts_get_prefetch().
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeouts_get_fetch(void)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_cookie_t cookie;
   xcb_dpms_get_timeouts_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_dpms_get_timeouts_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Gets the timeouts. The values are in unit of seconds.
 * @param standby Amount of time of inactivity before standby mode will be invoked.
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 * @param off     Amount of time of inactivity before the monitor is shut off.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeouts_get(unsigned int *standby,
                          unsigned int *suspend,
                          unsigned int *off)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (reply)
     {
        if (standby) *standby = reply->standby_timeout;
        if (suspend) *suspend = reply->suspend_timeout;
        if (off) *off = 0;
     }
   else
#endif /* ECORE_XCB_DPMS */
     {
        if (standby) *standby = 0;
        if (suspend) *suspend = 0;
        if (off) *off = 0;
     }
}


/**
 * Returns the amount of time of inactivity before standby mode is invoked.
 * @return The standby timeout value.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_timeouts_get_prefetch(), which sends the DPMSGetTimeouts request,
 * then ecore_x_dpms_timeouts_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_standby_get(void)
{
   int                            standby = 0;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   standby = reply->standby_timeout;
#endif /* ECORE_XCB_DPMS */

   return standby;
}


/**
 * Returns the amount of time of inactivity before the second level of
 * power saving is invoked.
 * @return The suspend timeout value.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_timeouts_get_prefetch(), which sends the DPMSGetTimeouts request,
 * then ecore_x_dpms_timeouts_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_suspend_get(void)
{
   int                            suspend = 0;;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   suspend = reply->suspend_timeout;
#endif /* ECORE_XCB_DPMS */

   return suspend;
}


/**
 * Returns the amount of time of inactivity before the third and final
 * level of power saving is invoked.
 * @return The off timeout value.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_timeouts_get_prefetch(), which sends the DPMSGetTimeouts request,
 * then ecore_x_dpms_timeouts_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_off_get(void)
{
   int                            off = 0;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   off = reply->off_timeout;
#endif /* ECORE_XCB_DPMS */

   return off;
}


/**
 * Sets the standby timeout (in unit of seconds).
 * @param new_standby Amount of time of inactivity before standby mode will be invoked.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_timeouts_get_prefetch(), which sends the DPMSGetTimeouts request,
 * then ecore_x_dpms_timeouts_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_standby_set(unsigned int new_standby)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_dpms_set_timeouts(_ecore_xcb_conn,
                         new_standby,
                         reply->suspend_timeout,
                         reply->off_timeout);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Sets the suspend timeout (in unit of seconds).
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_timeouts_get_prefetch(), which sends the DPMSGetTimeouts request,
 * then ecore_x_dpms_timeouts_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_suspend_set(unsigned int new_suspend)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_dpms_set_timeouts(_ecore_xcb_conn,
                         reply->standby_timeout,
                         new_suspend,
                         reply->off_timeout);
#endif /* ECORE_XCB_DPMS */
}


/**
 * Sets the off timeout (in unit of seconds).
 * @param off     Amount of time of inactivity before the monitor is shut off.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_dpms_timeouts_get_prefetch(), which sends the DPMSGetTimeouts request,
 * then ecore_x_dpms_timeouts_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_off_set(unsigned int new_off)
{
#ifdef ECORE_XCB_DPMS
   xcb_dpms_get_timeouts_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return;

   xcb_dpms_set_timeouts(_ecore_xcb_conn,
                         reply->standby_timeout,
                         reply->suspend_timeout,
                         new_off);
#endif /* ECORE_XCB_DPMS */
}
