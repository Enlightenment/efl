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

static int _screensaver_available = -1;

EAPI int
ecore_x_screensaver_event_available_get(void)
{
   if (_screensaver_available >= 0) return _screensaver_available;
#ifdef ECORE_XSS
   int _screensaver_major, _screensaver_minor;

   _screensaver_major = 1;
   _screensaver_minor = 0;

   if (XScreenSaverQueryVersion(_ecore_x_disp, &_screensaver_major, &_screensaver_minor))
     _screensaver_available = 1;
   else
     _screensaver_available = 0;
#else
   _screensaver_available = 0;
#endif
   return _screensaver_available;
}

EAPI int 
ecore_x_screensaver_idle_time_get(void)
{
#ifdef ECORE_XSS   
   XScreenSaverInfo *xss;
   int idle;

   xss = XScreenSaverAllocInfo();
   XScreenSaverQueryInfo(_ecore_x_disp, RootWindow(_ecore_x_disp, DefaultScreen(_ecore_x_disp)), xss); 
   idle = xss->idle / 1000;
   XFree(xss);

   return idle;
#endif

   return 0;
}

EAPI void
ecore_x_screensaver_set(int timeout, int interval, int blank, int expose)
{
   XSetScreenSaver(_ecore_x_disp, timeout, interval, blank, expose); 
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
ecore_x_screensaver_blank_set(double blank)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, pto, 
		   pint, (int)blank, pexpo);
}

EAPI double
ecore_x_screensaver_blank_get(void)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pblank;
}

EAPI void
ecore_x_screensaver_expose_set(double expose)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, pto, 
		   pint, pblank, (int)expose);
}

EAPI double
ecore_x_screensaver_expose_get(void)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pexpo;
}

EAPI void
ecore_x_screensaver_interval_set(double interval)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);  
   XSetScreenSaver(_ecore_x_disp, pto, 
		   (int)interval, pblank, pexpo);
}

EAPI double
ecore_x_screensaver_interval_get(void)
{
   int pto, pint, pblank, pexpo;
   
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return (double)pint;
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
