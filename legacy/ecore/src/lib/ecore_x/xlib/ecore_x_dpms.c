/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_x_private.h"

static int _dpms_available;

void
_ecore_x_dpms_init(void)
{
#ifdef ECORE_XDPMS
   int _dpms_major, _dpms_minor;

   _dpms_major = 1;
   _dpms_minor = 0;

   if (DPMSGetVersion(_ecore_x_disp, &_dpms_major, &_dpms_minor))
     _dpms_available = 1;
   else
     _dpms_available = 0;
#else
   _dpms_available = 0;
#endif
}

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
EAPI int
ecore_x_dpms_query(void)
{
   return _dpms_available;
}

/**
 * Checks if the X server is capable of DPMS.
 * @return @c 1 if the X server is capable of DPMS, @c 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_capable_get(void)
{
#ifdef ECORE_XDPMS
   return DPMSCapable(_ecore_x_disp);
#else
   return 0;
#endif
}

/**
 * Checks the DPMS state of the display.
 * @return @c 1 if DPMS is enabled, @c 0 otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_enabled_get(void)
{
#ifdef ECORE_XDPMS
   unsigned char state;
   unsigned short power_lvl;

   DPMSInfo(_ecore_x_disp, &power_lvl, &state);
   return state;
#else
   return 0;
#endif
}

/**
 * Sets the DPMS state of the display.
 * @param enabled @c 0 to disable DPMS characteristics of the server, enable it otherwise.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI void
ecore_x_dpms_enabled_set(int enabled)
{
#ifdef ECORE_XDPMS
   if (enabled)
     DPMSEnable(_ecore_x_disp);
   else
     DPMSDisable(_ecore_x_disp);
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
ecore_x_dpms_timeouts_get(unsigned int *standby, unsigned int *suspend, unsigned int *off)
{
#ifdef ECORE_XDPMS
   DPMSGetTimeouts(_ecore_x_disp, (unsigned short *)standby, 
		   (unsigned short *)suspend, (unsigned short *)off);
#endif
}

/**
 * Sets the timeouts. The values are in unit of seconds.
 * @param standby Amount of time of inactivity before standby mode will be invoked.
 * @param suspend Amount of time of inactivity before the screen is placed into suspend mode.
 * @param off     Amount of time of inactivity before the monitor is shut off.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI int
ecore_x_dpms_timeouts_set(unsigned int standby, unsigned int suspend, unsigned int off)
{
#ifdef ECORE_XDPMS
   return DPMSSetTimeouts(_ecore_x_disp, standby, suspend, off);
#else
   return 0;
#endif
}

/**
 * Returns the amount of time of inactivity before standby mode is invoked.
 * @return The standby timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_standby_get()
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return standby;
#else
   return 0;
#endif
}

/**
 * Returns the amount of time of inactivity before the second level of
 * power saving is invoked.
 * @return The suspend timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_suspend_get()
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return suspend;
#else
   return 0;
#endif
}

/**
 * Returns the amount of time of inactivity before the third and final
 * level of power saving is invoked.
 * @return The off timeout value.
 * @ingroup Ecore_X_DPMS_Group
 */
EAPI unsigned int
ecore_x_dpms_timeout_off_get()
{
#ifdef ECORE_XDPMS
   unsigned short standby, suspend, off;

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return off;
#else
   return 0;
#endif
}

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

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, new_timeout, suspend, off);
#endif
}

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

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, standby, new_timeout, off);
#endif
}

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

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, standby, suspend, new_timeout);
#endif
}
