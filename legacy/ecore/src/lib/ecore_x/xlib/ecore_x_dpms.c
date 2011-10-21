#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"

static Eina_Bool _dpms_available = EINA_FALSE;

void
_ecore_x_dpms_init(void)
{
#ifdef ECORE_XDPMS
   int _dpms_major, _dpms_minor;

   _dpms_major = 1;
   _dpms_minor = 0;

   if (DPMSGetVersion(_ecore_x_disp, &_dpms_major, &_dpms_minor))
     _dpms_available = EINA_TRUE;
   else
     _dpms_available = EINA_FALSE;

#else /* ifdef ECORE_XDPMS */
   _dpms_available = EINA_FALSE;
#endif /* ifdef ECORE_XDPMS */
} /* _ecore_x_dpms_init */

/**
 * @defgroup Ecore_X_DPMS_Group X DPMS Extension Functions
 *
 * Functions related to the X DPMS extension.
 */

/**
 * Checks if the X DPMS extension is available on the server.
 * @return @c 1 if the X DPMS extension is available, @c 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_query(void)
{
   return _dpms_available;
} /* ecore_x_dpms_query */

/**
 * Checks if the X server is capable of DPMS.
 * @return @c 1 if the X server is capable of DPMS, @c 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_capable_get(void)
{
#ifdef ECORE_XDPMS
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return DPMSCapable(_ecore_x_disp) ? EINA_TRUE : EINA_FALSE;
#else /* ifdef ECORE_XDPMS */
   return EINA_FALSE;
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_capable_get */

/**
 * Checks the DPMS state of the display.
 * @return @c 1 if DPMS is enabled, @c 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI Eina_Bool
ecore_x_dpms_enabled_get(void)
{
#ifdef ECORE_XDPMS
   unsigned char state;
   unsigned short power_lvl;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSInfo(_ecore_x_disp, &power_lvl, &state);
   return state ? EINA_TRUE : EINA_FALSE;
#else /* ifdef ECORE_XDPMS */
   return EINA_FALSE;
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_enabled_get */

/**
 * Sets the DPMS state of the display.
 * @param enabled @c 0 to disable DPMS characteristics of the server, enable it otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_enabled_set(int enabled)
{
#ifdef ECORE_XDPMS
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (enabled)
     DPMSEnable(_ecore_x_disp);
   else
     DPMSDisable(_ecore_x_disp);

#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_enabled_set */

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
#ifdef ECORE_XDPMS
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, (unsigned short *)standby,
                   (unsigned short *)suspend, (unsigned short *)off);
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeouts_get */

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
#ifdef ECORE_XDPMS
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return DPMSSetTimeouts(_ecore_x_disp, standby, suspend, off) ? EINA_TRUE : EINA_FALSE;
#else /* ifdef ECORE_XDPMS */
   return EINA_FALSE;
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeouts_set */

/**
 * Returns the amount of time of inactivity before standby mode is invoked.
 * @return The standby timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_standby_get(void)
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return standby;
#else /* ifdef ECORE_XDPMS */
   return 0;
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeout_standby_get */

/**
 * Returns the amount of time of inactivity before the second level of
 * power saving is invoked.
 * @return The suspend timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_suspend_get(void)
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return suspend;
#else /* ifdef ECORE_XDPMS */
   return 0;
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeout_suspend_get */

/**
 * Returns the amount of time of inactivity before the third and final
 * level of power saving is invoked.
 * @return The off timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_off_get(void)
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return off;
#else /* ifdef ECORE_XDPMS */
   return 0;
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeout_off_get */

/**
 * Sets the standby timeout (in unit of seconds).
 * @param new_standby Amount of time of inactivity before standby mode will be invoked.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_standby_set(unsigned int new_timeout)
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, new_timeout, suspend, off);
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeout_standby_set */

/**
 * Sets the suspend timeout (in unit of seconds).
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_suspend_set(unsigned int new_timeout)
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, standby, new_timeout, off);
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeout_suspend_set */

/**
 * Sets the off timeout (in unit of seconds).
 * @param off     Amount of time of inactivity before the monitor is shut off.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_timeout_off_set(unsigned int new_timeout)
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, standby, suspend, new_timeout);
#endif /* ifdef ECORE_XDPMS */
} /* ecore_x_dpms_timeout_off_set */

