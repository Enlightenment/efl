/*
 * vim:ts=8:sw=3:sts=8:expandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"
#include "ecore_x_randr.h"

#define Ecore_X_Randr_None 0
#ifdef ECORE_XRANDR

#define RANDR_1_1          ((1 << 16) | 1)

#define RANDR_VALIDATE_ROOT(screen,                                  \
                            root) ((screen =                         \
                                      XRRRootToScreen(_ecore_x_disp, \
                                                      root)) != -1)
#define RANDR_CHECK_1_1_RET(ret)  if(_randr_version < RANDR_1_1) return ret

extern XRRScreenResources *(*_ecore_x_randr_get_screen_resources)(Display *
                                                                  dpy,
                                                                  Window
                                                                  window);
extern int _randr_version;
#endif /* ifdef ECORE_XRANDR */

/*
 * @param root window which's primary output will be queried
 */
EAPI Ecore_X_Randr_Orientation
ecore_x_randr_screen_primary_output_orientations_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation rot = Ecore_X_Randr_None, crot;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   rot =
     XRRRotations(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                                 root), &crot);
   return rot;
#else /* ifdef ECORE_XRANDR */
   return Ecore_X_Randr_None;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_orientations_get */

/*
 * @param root window which's primary output will be queried
 * @return the current orientation of the root window's screen primary output
 */
EAPI Ecore_X_Randr_Orientation
ecore_x_randr_screen_primary_output_orientation_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation crot = Ecore_X_Randr_None;
   XRRRotations(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                               root), &crot);
   return crot;
#else /* ifdef ECORE_XRANDR */
   return Ecore_X_Randr_None;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_orientation_get */

/*
 * @brief sets a given screen's primary output's orientation
 * @param root window which's screen's primary output will be queried
 * @param orientation orientation which should be set for the root window's screen primary output
 * @return EINA_TRUE if the primary output's orientation could be successfully altered
 */
EAPI Eina_Bool
ecore_x_randr_screen_primary_output_orientation_set(
  Ecore_X_Window root,
  Ecore_X_Randr_Orientation
  orientation)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *xrr_screen_cfg = NULL;
   int sizeid;
   Rotation crot;
   Eina_Bool ret = EINA_FALSE;
   if (!(xrr_screen_cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
     return EINA_FALSE;

   sizeid = XRRConfigCurrentConfiguration(xrr_screen_cfg, &crot);
   if (!XRRSetScreenConfig(_ecore_x_disp, xrr_screen_cfg, root, sizeid,
                           orientation, CurrentTime))
     ret = EINA_TRUE;

   if (xrr_screen_cfg)
     XRRFreeScreenConfigInfo(xrr_screen_cfg);

   return ret;
#else /* ifdef ECORE_XRANDR */
   return EINA_FALSE;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_orientation_set */

/*
 * @brief gets a screen's primary output's possible sizes
 * @param root window which's primary output will be queried
 * @param num number of sizes reported as supported by the screen's primary output
 * @return an array of sizes reported as supported by the screen's primary output or - if query failed - NULL
 */
EAPI Ecore_X_Randr_Screen_Size_MM *
ecore_x_randr_screen_primary_output_sizes_get(Ecore_X_Window root,
                                              int           *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Screen_Size_MM *ret = NULL;
   XRRScreenSize *sizes;
   int i, n;

   /* we don't have to free sizes, because they're hold in a cache inside X*/
   sizes =
     XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                             root), &n);
   if ((!sizes) || (n <= 0)) return NULL;
   ret = calloc(n, sizeof(Ecore_X_Randr_Screen_Size_MM));
   if (!ret)
     return NULL;

   if (num)
     *num = n;

   for (i = 0; i < n; i++)
     {
        ret[i].width = sizes[i].width;
        ret[i].height = sizes[i].height;
        ret[i].width_mm = sizes[i].mwidth;
        ret[i].height_mm = sizes[i].mheight;
     }
   return ret;
#else /* ifdef ECORE_XRANDR */
   return NULL;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_sizes_get */

/*
 * @brief get the current set size of a given screen's primary output
 * @param root window which's primary output will be queried
 * @param w the current size's width
 * @param h the current size's height
 * @param w_mm the current size's width in mm
 * @param h_mm the current size's height in mm
 * @param size_index of current set size to be used with ecore_x_randr_primary_output_size_set()
 */
EAPI void
ecore_x_randr_screen_primary_output_current_size_get(Ecore_X_Window root,
                                                     int           *w,
                                                     int           *h,
                                                     int           *w_mm,
                                                     int           *h_mm,
                                                     int           *size_index)
{
#ifdef ECORE_XRANDR
   XRRScreenSize *sizes;
   XRRScreenConfiguration *sc = NULL;
   int idx;
   Rotation orientation;
   int n;

   if (!(sc = XRRGetScreenInfo(_ecore_x_disp, root)))
     {
        ERR("Couldn't get screen information for %d", root);
        return;
     }

   idx = XRRConfigCurrentConfiguration(sc, &orientation);

   sizes =
     XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                             root), &n);
   if ((idx < n) && (idx >= 0))
     {
        if (w)
          *w = sizes[idx].width;

        if (h)
          *h = sizes[idx].height;

        if (w_mm)
          *w_mm = sizes[idx].mwidth;

        if (h_mm)
          *h_mm = sizes[idx].mheight;

        if (size_index)
          *size_index = idx;
     }

   XRRFreeScreenConfigInfo(sc);
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_current_size_get */

/*
 * @brief sets a given screen's primary output size, but disables all other outputs at the same time
 * @param root window which's primary output will be queried
 * @param size_index within the list of sizes reported as supported by the root window's screen primary output
 * @return EINA_TRUE on success, EINA_FALSE on failure due to e.g. invalid times
 */
EAPI Eina_Bool
ecore_x_randr_screen_primary_output_size_set(Ecore_X_Window root,
                                             int            size_index)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *sc = NULL;
   XRRScreenSize *sizes;
   Eina_Bool ret = EINA_FALSE;
   int nsizes = 0;

   if (size_index >= 0 && _ecore_x_randr_root_validate(root))
     {
        sizes =
          XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp,
                                                  root), &nsizes);

        if (size_index < nsizes)
          {
             sc = XRRGetScreenInfo(_ecore_x_disp, root);
             if (!XRRSetScreenConfig(_ecore_x_disp, sc,
                                     root, size_index,
                                     ECORE_X_RANDR_ORIENTATION_ROT_0, CurrentTime))
               {
                  ret = EINA_TRUE;
               }

             if (sc)
               XRRFreeScreenConfigInfo(sc);
          }
     }

   return ret;
#else /* ifdef ECORE_XRANDR */
   return EINA_FALSE;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_size_set */

/*
 * @param root window which's primary output will be queried
 * @return currently used refresh rate or - if request failed or RandRR is not available - 0.0
 */
EAPI Ecore_X_Randr_Refresh_Rate
ecore_x_randr_screen_primary_output_current_refresh_rate_get(
  Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Refresh_Rate ret = 0.0;
   XRRScreenConfiguration *sc = NULL;

   if (!_ecore_x_randr_root_validate(root) ||
       !(sc = XRRGetScreenInfo(_ecore_x_disp, root)))
     return ret;

   ret = XRRConfigCurrentRate(sc);
   if (sc)
     XRRFreeScreenConfigInfo(sc);

   return ret;
#else /* ifdef ECORE_XRANDR */
   return 0.0;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_current_refresh_rate_get */

/*
 * @param root window which's primary output will be queried
 * @param size_index referencing the size to query valid refresh rates for
 * @return currently used refresh rate or - if request failed or RandRR is not available - NULL
 */
EAPI Ecore_X_Randr_Refresh_Rate *
ecore_x_randr_screen_primary_output_refresh_rates_get(Ecore_X_Window root,
                                                      int            size_index,
                                                      int           *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Refresh_Rate *ret = NULL, *rates = NULL;
   Ecore_X_Randr_Screen scr;
   int n;

   if (num
       && RANDR_VALIDATE_ROOT(scr, root)
       && (rates = XRRRates(_ecore_x_disp, scr, size_index, &n)))
     {
        if (rates && (ret = malloc(sizeof(Ecore_X_Randr_Refresh_Rate) * n)))
          {
             memcpy(ret, rates, (sizeof(Ecore_X_Randr_Refresh_Rate) * n));
             *num = n;
          }
     }

   return ret;
#else /* ifdef ECORE_XRANDR */
   return NULL;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_refresh_rates_get */

//>= 1.1
/*
 * @brief sets the current primary output's refresh rate
 * @param root window which's primary output will be queried
 * @param size_index referencing the size to be set
 * @param rate the refresh rate to be set
 * @return EINA_TRUE on success else EINA_FALSE
 */
EAPI Eina_Bool
ecore_x_randr_screen_primary_output_refresh_rate_set(
  Ecore_X_Window root,
  int            size_index,
  Ecore_X_Randr_Refresh_Rate
  rate)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_1_RET(EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;
   XRRScreenConfiguration *sc = NULL;

   if (!(sc = XRRGetScreenInfo(_ecore_x_disp, root)))
     return ret;

   if (!XRRSetScreenConfigAndRate(_ecore_x_disp, sc,
                                  root, size_index,
                                  RR_Rotate_0, rate, CurrentTime))
     ret = EINA_TRUE;

   XRRFreeScreenConfigInfo(sc);
   return ret;
#else /* ifdef ECORE_XRANDR */
   return EINA_FALSE;
#endif /* ifdef ECORE_XRANDR */
} /* ecore_x_randr_screen_primary_output_refresh_rate_set */

