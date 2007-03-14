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
ecore_x_screensaver_event_available_get(void)
{
#ifdef ECORE_XSS
   return 1;
#else
   return 0;
#endif
}

EAPI void
ecore_x_screensaver_set(int timeout, int interval, int blank, int expose)
{
#ifdef ECORE_XSS
   XSetScreenSaver(_ecore_x_disp, timeout, interval, blank, expose); 
#endif
}

EAPI void
ecore_x_screensaver_timeout_set(double timeout)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, (int)timeout, 
		   pint, pblank, pexpo);
#endif
}

EAPI double
ecore_x_screensaver_timeout_get(void)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pto;
#else
   return 0;
#endif
}

EAPI void
ecore_x_screensaver_blank_set(double blank)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, pto, 
		   pint, (int)blank, pexpo);
#endif
}

EAPI double
ecore_x_screensaver_blank_get(void)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pblank;
#else
   return 0;
#endif
}

EAPI void
ecore_x_screensaver_expose_set(double expose)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, pto, 
		   pint, pblank, (int)expose);
#endif
}

EAPI double
ecore_x_screensaver_expose_get(void)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pexpo;
#else
   return 0;
#endif
}

EAPI void
ecore_x_screensaver_interval_set(double interval)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, pto, 
		   (int)interval, pblank, pexpo);
#endif
}

EAPI double
ecore_x_screensaver_interval_get(void)
{
#ifdef ECORE_XSS
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pint;
#else
   return 0;
#endif
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
#else
   on = 0;
#endif   
}
