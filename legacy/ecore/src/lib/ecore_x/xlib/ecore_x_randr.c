/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_x_private.h"

EAPI int
ecore_x_randr_query()
{
#ifdef ECORE_XRANDR
   int randr_base = 0;
   int randr_err_base = 0;

   if (XRRQueryExtension(_ecore_x_disp, &randr_base, &randr_err_base))
     return 1;
   else
     return 0;
#else
   return 0;
#endif
}

EAPI int
ecore_x_randr_events_select(Ecore_X_Window win, int on)
{
#ifdef ECORE_XRANDR
   if (on)
     XRRSelectInput(_ecore_x_disp, win, RRScreenChangeNotifyMask);
   else
     XRRSelectInput(_ecore_x_disp, win, 0);

   return 1;
#else
   return 0;
#endif
}

EAPI Ecore_X_Randr_Rotation
ecore_x_randr_screen_rotations_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation rot, crot;
   
   rot = XRRRotations(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &crot);
   return rot;
#else
   return 0;
#endif   
}

EAPI Ecore_X_Randr_Rotation
ecore_x_randr_screen_rotation_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation rot, crot = 0;
   
   rot = XRRRotations(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &crot);
   return crot;
#else
   return 0;
#endif   
}

EAPI void
ecore_x_randr_screen_rotation_set(Ecore_X_Window root, Ecore_X_Randr_Rotation rot)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *xrrcfg;
   SizeID sizeid;
   Rotation crot;
   
   xrrcfg = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!xrrcfg) return;
   sizeid = XRRConfigCurrentConfiguration(xrrcfg, &crot);
   XRRSetScreenConfig(_ecore_x_disp, xrrcfg, root, sizeid, rot, CurrentTime);
   XRRFreeScreenConfigInfo(xrrcfg);
#endif   
}

EAPI Ecore_X_Screen_Size *
ecore_x_randr_screen_sizes_get(Ecore_X_Window root, int *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Screen_Size *ret;
   XRRScreenSize *sizes;
   int i, n;

   if (num) *num = 0;

   /* we don't have to free sizes, no idea why not */
   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   ret = calloc(n, sizeof(Ecore_X_Screen_Size));
   if (!ret) return NULL;

   if (num) *num = n;
   for (i = 0; i < n; i++)
     {
	ret[i].width = sizes[i].width;
	ret[i].height = sizes[i].height;
     }
   return ret;
#else
   if (num) *num = 0;
   return NULL;
#endif
}

EAPI Ecore_X_Screen_Size
ecore_x_randr_current_screen_size_get(Ecore_X_Window root)
{
   Ecore_X_Screen_Size ret = { -1, -1 };
#ifdef ECORE_XRANDR
   XRRScreenSize *sizes;
   XRRScreenConfiguration *sc;
   SizeID size_index;
   Rotation rotation;
   int n;

   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!sc)
     {
	printf("ERROR: Couldn't get screen information for %d\n", root);
	return ret;
     }
   size_index = XRRConfigCurrentConfiguration(sc, &rotation);

   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   if (size_index < n)
     {
	ret.width = sizes[size_index].width;
	ret.height = sizes[size_index].height;
     }
   XRRFreeScreenConfigInfo(sc);
#endif
   return ret;
}

EAPI int
ecore_x_randr_screen_size_set(Ecore_X_Window root, Ecore_X_Screen_Size size)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *sc;
   XRRScreenSize *sizes;
   int i, n, size_index = -1;

   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   for (i = 0; i < n; i++)
     {
	if ((sizes[i].width == size.width) && (sizes[i].height == size.height))
	  {
	     size_index = i;
	     break;
	  }
     }
   if (size_index == -1) return 0;
   
   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (XRRSetScreenConfig(_ecore_x_disp, sc,
			  root, size_index,
			  RR_Rotate_0, CurrentTime))
     {
	printf("ERROR: Can't set new screen size!\n");
	XRRFreeScreenConfigInfo(sc);
	return 0;
     }
   XRRFreeScreenConfigInfo(sc);
   return 1;
#else
   return 0;
#endif
}

EAPI Ecore_X_Screen_Refresh_Rate
ecore_x_randr_current_screen_refresh_rate_get(Ecore_X_Window root)
{
   Ecore_X_Screen_Refresh_Rate ret = { -1 };
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *sc;

   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!sc)
     {
	printf("ERROR: Couldn't get screen information for %d\n", root);
	return ret;
     }
   ret.rate = XRRConfigCurrentRate(sc);
   XRRFreeScreenConfigInfo(sc);   
#endif
   return ret;
}

EAPI Ecore_X_Screen_Refresh_Rate *
ecore_x_randr_screen_refresh_rates_get(Ecore_X_Window root, int size_id, int *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Screen_Refresh_Rate *ret = NULL;
   XRRScreenConfiguration *sc;   
   short *rates;
   int i, n;

   if (num) *num = 0;

   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!sc)
     {
	printf("ERROR: Couldn't get screen information for %d\n", root);
	return ret;
     }
   
   rates = XRRRates(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), size_id, &n);
   ret = calloc(n, sizeof(Ecore_X_Screen_Refresh_Rate));
   if (!ret) 
     { 
	XRRFreeScreenConfigInfo(sc);
	return NULL;
     }
   
   if (num) *num = n;
   for (i = 0; i < n; i++)
     {
	ret[i].rate = rates[i];
     }
   XRRFreeScreenConfigInfo(sc);   
   return ret;
#else
   if (num) *num = 0;   
   return NULL;
#endif
}

EAPI int
ecore_x_randr_screen_refresh_rate_set(Ecore_X_Window root, Ecore_X_Screen_Size size, Ecore_X_Screen_Refresh_Rate rate)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *sc;
   XRRScreenSize *sizes;
   int i, n, size_index = -1;

   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   for (i = 0; i < n; i++)
     {
	if ((sizes[i].width == size.width) && (sizes[i].height == size.height))
	  {
	     size_index = i;
	     break;
	  }
     }
   if (size_index == -1) return 0;
   
   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (XRRSetScreenConfigAndRate(_ecore_x_disp, sc,
				 root, size_index,
				 RR_Rotate_0, rate.rate, CurrentTime))
     {
	printf("ERROR: Can't set new screen size and refresh rate!\n");
	XRRFreeScreenConfigInfo(sc);
	return 0;
     }
   XRRFreeScreenConfigInfo(sc);
   return 1;
#else
   return 1;
#endif
}
