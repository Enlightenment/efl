#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_DAMAGE
# include <xcb/dpms.h>
#endif

/* local variables */
static Eina_Bool _dpms_avail = EINA_FALSE;

void
_ecore_xcb_dpms_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DPMS
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_dpms_id);
#endif
}

void
_ecore_xcb_dpms_finalize(void)
{
#ifdef ECORE_XCB_DPMS
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_DPMS
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_dpms_id);
   if ((ext_reply) && (ext_reply->present))
     {
        xcb_dpms_get_version_cookie_t cookie;
        xcb_dpms_get_version_reply_t *reply;

        cookie =
          xcb_dpms_get_version_unchecked(_ecore_xcb_conn,
                                         XCB_DPMS_MAJOR_VERSION,
                                         XCB_DPMS_MINOR_VERSION);
        reply = xcb_dpms_get_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply)
          {
             if (reply->server_major_version >= 1)
               _dpms_avail = EINA_TRUE;
             free(reply);
          }
     }
#endif
}

/**
 * @defgroup Ecore_X_DPMS_Group X DPMS Extension Functions
 *
 * Functions related to the X DPMS Extension
 */

/**
 * Checks if the DPMS extension is available or not.
 *
 * @return @c EINA_TRUE if the DPMS extension is available,
 * @c EINA_FALSE otherwise.
 *
 * Return EINA_TRUE if the X server supports the DPMS Extension version 1.0,
 * EINA_FALSE otherwise.
 *
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_query(void)
{
//   LOGFN(__FILE__, __LINE__, __FUNCTION__);

     return _dpms_avail;
}

/**
 * Checks if the X server is capable of DPMS.
 * @return @c 1 if the X server is capable of DPMS, @c 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_capable_get(void)
{
   Eina_Bool ret = EINA_FALSE;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_capable_cookie_t cookie;
   xcb_dpms_capable_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_dpms_avail) return EINA_FALSE;

#ifdef ECORE_XCB_DPMS
   cookie = xcb_dpms_capable_unchecked(_ecore_xcb_conn);
   reply = xcb_dpms_capable_reply(_ecore_xcb_conn, cookie, NULL);
   if (reply)
     {
        ret = reply->capable;
        free(reply);
     }
#endif

   return ret;
}

/**
 * Checks the DPMS state of the display.
 * @return @c EINA_TRUE if DPMS is enabled, @c EINA_FALSE otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_enabled_get(void)
{
   Eina_Bool ret = EINA_FALSE;
#ifdef ECORE_XCB_DPMS
   xcb_dpms_info_cookie_t cookie;
   xcb_dpms_info_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_dpms_avail) return EINA_FALSE;

#ifdef ECORE_XCB_DPMS
   cookie = xcb_dpms_info_unchecked(_ecore_xcb_conn);
   reply = xcb_dpms_info_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;
   if (reply->state) ret = EINA_TRUE;
   free(reply);
#endif

   return ret;
}

/**
 * Sets the DPMS state of the display.
 * @param enabled @c 0 to disable DPMS characteristics of the server, enable it otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_enabled_set(int enabled)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_dpms_avail) return;

#ifdef ECORE_XCB_DPMS
   if (enabled)
     xcb_dpms_enable(_ecore_xcb_conn);
   else
     xcb_dpms_disable(_ecore_xcb_conn);
#endif
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
   xcb_dpms_get_timeouts_cookie_t cookie;
   xcb_dpms_get_timeouts_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (standby) *standby = 0;
   if (suspend) *suspend = 0;
   if (off) *off = 0;

   if (!_dpms_avail) return;

#ifdef ECORE_XCB_DPMS
   cookie = xcb_dpms_get_timeouts_unchecked(_ecore_xcb_conn);
   reply = xcb_dpms_get_timeouts_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   if (standby) *standby = reply->standby_timeout;
   if (suspend) *suspend = reply->suspend_timeout;
   if (off) *off = reply->off_timeout;
   free(reply);
#endif
}

/**
 * Sets the timeouts. The values are in unit of seconds.
 * @param standby Amount of time of inactivity before standby mode will be invoked.
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 * @param off     Amount of time of inactivity before the monitor is shut off.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_timeouts_set(unsigned int standby,
                          unsigned int suspend,
                          unsigned int off)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_dpms_avail) return EINA_FALSE;

#ifdef ECORE_XCB_DPMS
   // FIXME: Add request check
   xcb_dpms_set_timeouts(_ecore_xcb_conn, standby, suspend, off);
   return EINA_TRUE;
#endif

   return EINA_FALSE;
}

/**
 * Returns the amount of time of inactivity before standby mode is invoked.
 * @return The standby timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_standby_get(void)
{
   unsigned int standby = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_dpms_timeouts_get(&standby, NULL, NULL);
   return standby;
}

/**
 * Returns the amount of time of inactivity before the second level of
 * power saving is invoked.
 * @return The suspend timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_suspend_get(void)
{
   unsigned int suspend = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_dpms_timeouts_get(NULL, &suspend, NULL);
   return suspend;
}

/**
 * Returns the amount of time of inactivity before the third and final
 * level of power saving is invoked.
 * @return The off timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_off_get(void)
{
   unsigned int off = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_dpms_timeouts_get(NULL, NULL, &off);
   return off;
}

/**
 * Sets the standby timeout (in unit of seconds).
 * @param new_standby Amount of time of inactivity before standby mode will be invoked.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_standby_set(unsigned int new_timeout)
{
   unsigned int standby = 0, suspend = 0, off = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_dpms_timeouts_get(&standby, &suspend, &off);
   ecore_x_dpms_timeouts_set(new_timeout, suspend, off);
}

/**
 * Sets the suspend timeout (in unit of seconds).
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_suspend_set(unsigned int new_timeout)
{
   unsigned int standby = 0, suspend = 0, off = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_dpms_timeouts_get(&standby, &suspend, &off);
   ecore_x_dpms_timeouts_set(standby, new_timeout, off);
}

/**
 * Sets the off timeout (in unit of seconds).
 * @param off     Amount of time of inactivity before the monitor is shut off.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_off_set(unsigned int new_timeout)
{
   unsigned int standby = 0, suspend = 0, off = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   ecore_x_dpms_timeouts_get(&standby, &suspend, &off);
   ecore_x_dpms_timeouts_set(standby, suspend, new_timeout);
}

