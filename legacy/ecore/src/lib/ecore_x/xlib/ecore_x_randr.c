/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "ecore_x_private.h"

static int _randr_available = 0;
#ifdef ECORE_XRANDR
static int _randr_major, _randr_minor, _randr_version;
#define RANDR_1_2 ((1 << 16) | 2)
#define RANDR_1_3 ((1 << 16) | 3)
#endif

void
_ecore_x_randr_init(void)
{
#ifdef ECORE_XRANDR
   _randr_major = 1;
   _randr_minor = 3;
   _randr_version = 0;

   if (XRRQueryVersion(_ecore_x_disp, &_randr_major, &_randr_minor))
     {
        _randr_version = (_randr_major << 16) | _randr_minor;
        _randr_available = 1;
     }
   else
      _randr_available = 0;

#else
   _randr_available = 0;
#endif
}

EAPI int
ecore_x_randr_query(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return _randr_available;
}

EAPI int
ecore_x_randr_events_select(Ecore_X_Window win, int on)
{
#ifdef ECORE_XRANDR
   int mask;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!on)
      mask = 0;
   else
     {
        mask = RRScreenChangeNotifyMask;
        if (_randr_version >= RANDR_1_2)
           mask |= (RRCrtcChangeNotifyMask |
                    RROutputChangeNotifyMask |
                    RROutputPropertyNotifyMask);
     }

   XRRSelectInput(_ecore_x_disp, win, mask);

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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   rot = XRRRotations(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                                     root), &crot);
   return rot;
#else
   return 0;
#endif
}

EAPI Ecore_X_Randr_Rotation
ecore_x_randr_screen_rotation_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation crot = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XRRRotations(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &crot);
   return crot;
#else
   return 0;
#endif
}

EAPI void
ecore_x_randr_screen_rotation_set(Ecore_X_Window root,
                                  Ecore_X_Randr_Rotation rot)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *xrrcfg;
   SizeID sizeid;
   Rotation crot;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xrrcfg = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!xrrcfg)
      return;

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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (num)
      *num = 0;

   /* we don't have to free sizes, no idea why not */
   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   ret = calloc(n, sizeof(Ecore_X_Screen_Size));
   if (!ret)
      return NULL;

   if (num)
      *num = n;

   for (i = 0; i < n; i++)
     {
        ret[i].width = sizes[i].width;
        ret[i].height = sizes[i].height;
     }
   return ret;
#else
   if (num)
      *num = 0;

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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!sc)
     {
        ERR("Couldn't get screen information for %d", root);
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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   for (i = 0; i < n; i++)
     {
        if ((sizes[i].width == size.width) && (sizes[i].height == size.height))
          {
             size_index = i;
             break;
          }
     }
   if (size_index == -1)
      return 0;

   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (XRRSetScreenConfig(_ecore_x_disp, sc,
                          root, size_index,
                          RR_Rotate_0, CurrentTime))
     {
        ERR("Can't set new screen size!");
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

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!sc)
     {
        ERR("Couldn't get screen information for %d", root);
        return ret;
     }

   ret.rate = XRRConfigCurrentRate(sc);
   XRRFreeScreenConfigInfo(sc);
#endif
   return ret;
}

EAPI Ecore_X_Screen_Refresh_Rate *
ecore_x_randr_screen_refresh_rates_get(Ecore_X_Window root,
                                       int size_id,
                                       int *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Screen_Refresh_Rate *ret = NULL;
   XRRScreenConfiguration *sc;
   short *rates;
   int i, n;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (num)
      *num = 0;

   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (!sc)
     {
        ERR("Couldn't get screen information for %d", root);
        return ret;
     }

   rates = XRRRates(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                                   root), size_id, &n);
   ret = calloc(n, sizeof(Ecore_X_Screen_Refresh_Rate));
   if (!ret)
     {
        XRRFreeScreenConfigInfo(sc);
        return NULL;
     }

   if (num)
      *num = n;

   for (i = 0; i < n; i++)
     {
        ret[i].rate = rates[i];
     }
        XRRFreeScreenConfigInfo(sc);
   return ret;
#else
   if (num)
      *num = 0;

   return NULL;
#endif
}

EAPI int
ecore_x_randr_screen_refresh_rate_set(Ecore_X_Window root,
                                      Ecore_X_Screen_Size size,
                                      Ecore_X_Screen_Refresh_Rate rate)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *sc;
   XRRScreenSize *sizes;
   int i, n, size_index = -1;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   for (i = 0; i < n; i++)
     {
        if ((sizes[i].width == size.width) && (sizes[i].height == size.height))
          {
             size_index = i;
             break;
          }
     }
   if (size_index == -1)
      return 0;

   sc = XRRGetScreenInfo(_ecore_x_disp, root);
   if (XRRSetScreenConfigAndRate(_ecore_x_disp, sc,
                                 root, size_index,
                                 RR_Rotate_0, rate.rate, CurrentTime))
     {
        ERR("Can't set new screen size and refresh rate!");
        XRRFreeScreenConfigInfo(sc);
        return 0;
     }

        XRRFreeScreenConfigInfo(sc);
   return 1;
#else
   return 1;
#endif
}
