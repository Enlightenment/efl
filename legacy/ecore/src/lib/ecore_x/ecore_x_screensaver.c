/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*
 * Screensaver code
 */
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

EAPI int
ecore_x_screensaver_event_avaialable_get(void)
{
#ifdef ECORE_XSS
   return 1;
#endif
   return 0;
}

EAPI void
ecore_x_screensaver_timeout_set(double timeout)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   XSetScreenSaver(_ecore_x_disp, (int)timeout, 
		   pint, pblank, pexpo);
}

EAPI double
ecore_x_screensaver_timeout_get(void)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pto;
}

EAPI void
ecore_x_screensaver_event_listen_set(int on)
{
#ifdef ECORE_XSS
   Ecore_X_Window root;
   
   root = DefaultRootWindow(_ecore_x_disp);
   if (on)
     XScreenSaverSelectInput(_ecore_x_disp, root, ScreenSaverNotifyMask);
   else
     XScreenSaverSelectInput(_ecore_x_disp, root, 0);
#endif   
}