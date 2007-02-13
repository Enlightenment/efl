/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*
 * DPMS code
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

/*  Are we capable */
EAPI int 
ecore_x_dpms_capable(void)
{
   return DPMSCapable(_ecore_x_disp);   
}

/* Are we enabled */
EAPI int
ecore_x_dpms_enabled_get(void)
{
   unsigned char state;
   unsigned short power_lvl;
   
   DPMSInfo(_ecore_x_disp, &power_lvl, &state);
   return state;
}

/* Enable/Disable DPMS */
EAPI void
ecore_x_dpms_enabled_set(int enabled)
{
   if (enabled)
      DPMSEnable(_ecore_x_disp);
   else
      DPMSDisable(_ecore_x_disp);
}

/* Set all timeouts to known values */
EAPI int
ecore_x_dpms_timeouts_set(int standby, int suspend, int off)
{
   return DPMSSetTimeouts(_ecore_x_disp, 
		        standby, 
			suspend,
			off);
}

/* Get/Set Timeouts */
EAPI int 
ecore_x_dpms_timeout_standby_get()
{
   unsigned short standby, suspend, off;
   
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return standby;
}

EAPI int 
ecore_x_dpms_timeout_suspend_get()
{
   unsigned short standby, suspend, off;
   
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return suspend;
}

EAPI int 
ecore_x_dpms_timeout_off_get()
{
   unsigned short standby, suspend, off;

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   return off;
}

EAPI void 
ecore_x_dpms_timeout_standby_set(int new_timeout)
{
   unsigned short standby, suspend, off;

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, 
                   new_timeout, 
                   suspend,
                   off);
}

EAPI void 
ecore_x_dpms_timeout_suspend_set(int new_timeout)
{
   unsigned short standby, suspend, off;
        
   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, 
		standby,
		new_timeout, 
		off);
}

EAPI void 
ecore_x_dpms_timeout_off_set(int new_timeout)
{
   unsigned short standby, suspend, off;

   DPMSGetTimeouts(_ecore_x_disp, &standby, &suspend, &off);
   DPMSSetTimeouts(_ecore_x_disp, 
			standby,
			suspend,
			new_timeout);
}
