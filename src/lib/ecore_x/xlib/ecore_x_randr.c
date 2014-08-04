 #ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/* NB: FIXME:
 * 
 * A lot of this code uses XRRGetScreenInfo and then calls 
 * XRRFreeScreenConfigInfo. Typically this is not an Unheard of thing to do, 
 * however this process of getting config and freeing config does force a 
 * round-trip to the X server */

/* local variables */
static Eina_Bool _randr_avail = EINA_FALSE;

static Ecore_X_Atom connector_type = 0;
static Ecore_X_Atom connector_number = 0;

#ifdef ECORE_XRANDR

# define RANDR_VERSION_1_1 ((1 << 16) | 1)
# define RANDR_VERSION_1_2 ((1 << 16) | 2)
# define RANDR_VERSION_1_3 ((1 << 16) | 3)
# define RANDR_VERSION_1_4 ((1 << 16) | 4)

# define RANDR_EDID_VERSION_1_3 ((1 << 8) | 3)
# define RANDR_EDID_VERSION_MAJOR 0x12
# define RANDR_EDID_VERSION_MINOR 0x13

# define RANDR_EDID_MANUFACTURER 0x08
# define RANDR_EDID_BLOCK 0x36

typedef enum _Ecore_X_Randr_Edid_Aspect_Ratio_Preferred 
{
   RANDR_EDID_ASPECT_RATIO_PREFERRED_4_3 = 0x00,
     RANDR_EDID_ASPECT_RATIO_PREFERRED_16_9 = 0x01,
     RANDR_EDID_ASPECT_RATIO_PREFERRED_16_10 = 0x02,
     RANDR_EDID_ASPECT_RATIO_PREFERRED_5_4 = 0x03,
     RANDR_EDID_ASPECT_RATIO_PREFERRED_15_9 = 0x04
} Ecore_X_Randr_Edid_Aspect_Ratio_Preferred;

static int _randr_major, _randr_minor, _randr_version;

XRRScreenResources *(*_ecore_x_randr_screen_resources_get)(Display *disp, Window win);

#endif

/* local functions */
void 
_ecore_x_randr_init(void)
{
#ifdef ECORE_XRANDR
   _randr_major = 1;
   _randr_minor = 4;
   _randr_version = 0;

   /* try to query the randr extenstion version */
   if (XRRQueryVersion(_ecore_x_disp, &_randr_major, &_randr_minor))
     {
        _randr_version = (_randr_major << 16) | _randr_minor;

        if (_randr_version >= RANDR_VERSION_1_3)
          _ecore_x_randr_screen_resources_get = XRRGetScreenResourcesCurrent;
        else if (_randr_version == RANDR_VERSION_1_2)
          _ecore_x_randr_screen_resources_get = XRRGetScreenResources;

        _randr_avail = EINA_TRUE;

        connector_type = ecore_x_atom_get(RR_PROPERTY_CONNECTOR_TYPE);
        connector_number = ecore_x_atom_get(RR_PROPERTY_CONNECTOR_NUMBER);
     }
#endif
}

/* public functions */
EAPI int 
ecore_x_randr_version_get(void)
{
#ifdef ECORE_XRANDR
   if (_randr_avail) return _randr_version;
#endif
   return -1;
}

EAPI Eina_Bool 
ecore_x_randr_query(void)
{
   return _randr_avail;
}

/**
 * @brief This function returns the current config timestamp from 
 * XRRScreenConfiguration.
 * 
 * @param root root window to query screen configuration from
 * 
 * @returns The screen configuration timestamp
 * 
 * @since 1.8
 */
EAPI Ecore_X_Time 
ecore_x_randr_config_timestamp_get(Ecore_X_Window root)
{
   Ecore_X_Time timestamp = 0;

#ifdef ECORE_XRANDR
   XRRScreenConfiguration *cfg;

   /* try to get the screen configuration from Xrandr */
   if ((cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
     {
        Time tm;

        XRRConfigTimes(cfg, &tm);

        timestamp = (Ecore_X_Time)tm;

        /* free any returned screen config */
        if (cfg) XRRFreeScreenConfigInfo(cfg);
     }
#endif

   return timestamp;
}

/***************************************
 * API Functions for RandR version 1.1 *
 ***************************************/

/*
 * @param root window which's primary output will be queried
 */
EAPI Ecore_X_Randr_Orientation 
ecore_x_randr_screen_primary_output_orientations_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation ret = 0, crot = 0;

   /* get the rotations available from XRandr */
   ret = XRRRotations(_ecore_x_disp, 
                      XRRRootToScreen(_ecore_x_disp, root), &crot);

   return ret;
#else
   return 0;
#endif
}

/*
 * @param root window which's primary output will be queried
 * @return the current orientation of the root window's screen primary output
 */
EAPI Ecore_X_Randr_Orientation 
ecore_x_randr_screen_primary_output_orientation_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   Rotation ret = 0;

   /* get the current rotation available from XRandr */
   XRRRotations(_ecore_x_disp, 
                XRRRootToScreen(_ecore_x_disp, root), &ret);

   return ret;
#else
   return 0;
#endif
}

/*
 * @brief Sets a given screen's primary output's orientation.
 *
 * @param root Window which's screen's primary output will be queried.
 * @param orientation orientation which should be set for the root window's
 * screen primary output.
 * @return @c EINA_TRUE if the primary output's orientation could be
 * successfully altered.
 */
EAPI Eina_Bool 
ecore_x_randr_screen_primary_output_orientation_set(Ecore_X_Window root, Ecore_X_Randr_Orientation orient)
{
#ifdef ECORE_XRANDR
   Eina_Bool ret = EINA_FALSE;
   Rotation crot = 0;
   XRRScreenConfiguration *cfg = NULL;
   int id = 0;

   /* try to get the screen config from XRandr */
   if (!(cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
     return EINA_FALSE;

   /* get the screen's current size id */
   id = XRRConfigCurrentConfiguration(cfg, &crot);

   /* attempt to set the new orientation */
   if (!XRRSetScreenConfig(_ecore_x_disp, cfg, root, id, orient, CurrentTime))
     ret = EINA_TRUE;

   /* free any returned screen config */
   if (cfg) XRRFreeScreenConfigInfo(cfg);

   return ret;
#else
   return EINA_FALSE;
#endif
}

/*
 * @brief gets a screen's primary output's possible sizes
 * @param root window which's primary output will be queried
 * @param num number of sizes reported as supported by the screen's primary output
 * @return an array of sizes reported as supported by the screen's primary output or - if query failed - NULL
 */
EAPI Ecore_X_Randr_Screen_Size_MM *
ecore_x_randr_screen_primary_output_sizes_get(Ecore_X_Window root, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Screen_Size_MM *ret = NULL;
   XRRScreenSize *sizes;
   int n = 0, i = 0;

   /* retrieve the number of sizes from X, and the sizes themselves.
    * 
    * NB: don't have to free the returned sizes */
   sizes = XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
   if (!sizes) return NULL;
   if (n <= 0) return NULL;

   /* try to allocate our structure for these sizes */
   if (!(ret = calloc(n, sizeof(Ecore_X_Randr_Screen_Size_MM))))
     return NULL;

   if (num) *num = n;

   /* fill in our allocated structure with the screen sizes */
   for (i = 0; i < n; i++)
     {
        ret[i].width = sizes[i].width;
        ret[i].height = sizes[i].height;
        ret[i].width_mm = sizes[i].mwidth;
        ret[i].height_mm = sizes[i].mheight;
     }

   return ret;
#else
   return NULL;
#endif
}

EAPI void 
ecore_x_randr_screen_primary_output_current_size_get(Ecore_X_Window root, int *w, int *h, int *w_mm, int *h_mm, int *size_index)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *cfg = NULL;

   /* try to get the screen config from XRandr */
   if ((cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
     {
        XRRScreenSize *sizes;
        Rotation crot = 0;
        int n = 0;

        /* retrieve the number of sizes from X, and the sizes themselves.
         * 
         * NB: don't have to free the returned sizes */
        sizes = 
          XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);
        if ((sizes) && (n > 0))
          {
             int idx = 0;

             /* get the index of the current configuration */
             idx = XRRConfigCurrentConfiguration(cfg, &crot);

             /* if the index is valid, then fill in the return variables with 
              * the size information for this index */
             if ((idx < n) && (idx >= 0))
               {
                  if (w) *w = sizes[idx].width;
                  if (h) *h = sizes[idx].height;
                  if (w_mm) *w_mm = sizes[idx].mwidth;
                  if (h_mm) *h_mm = sizes[idx].mheight;
                  if (size_index) *size_index = idx;
               }
          }

        /* free the returned screen config */
        XRRFreeScreenConfigInfo(cfg);
     }
#endif
}

/*
 * @brief Sets a given screen's primary output size, but disables all other
 * outputs at the same time.
 *
 * @param root Window which's primary output will be queried.
 * @param size_index Within the list of sizes reported as supported by the root
 * window's screen primary output.
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure due to e.g.
 * invalid times.
 */
EAPI Eina_Bool 
ecore_x_randr_screen_primary_output_size_set(Ecore_X_Window root, int size_index)
{
#ifdef ECORE_XRANDR
   Eina_Bool ret = EINA_FALSE;
   int n = 0;

   /* check for valid size index first */
   if (size_index < 0) return EINA_FALSE;

   /* get the number of sizes from XRandr */
   XRRSizes(_ecore_x_disp, XRRRootToScreen(_ecore_x_disp, root), &n);

   /* make sure the requested index is below the number returned from randr */
   if (size_index < n)
     {
        XRRScreenConfiguration *cfg = NULL;

        /* try to get the screen config from XRandr */
        if ((cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
          {
             /* try to set the new screen config
              * 
              * NB: Returns Success (0) if it works */
             if (!XRRSetScreenConfig(_ecore_x_disp, cfg, root, size_index, 
                                      ECORE_X_RANDR_ORIENTATION_ROT_0, 
                                      CurrentTime))
               {
                  ret = EINA_TRUE;
               }

             /* free the returned screen config */
             XRRFreeScreenConfigInfo(cfg);
          }
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

/*
 * @param root window which's primary output will be queried
 * @return currently used refresh rate or - if request failed or RandRR is not available - 0.0
 */
EAPI Ecore_X_Randr_Refresh_Rate 
ecore_x_randr_screen_primary_output_current_refresh_rate_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   XRRScreenConfiguration *cfg = NULL;

   /* try to get the screen config from XRandr */
   if ((cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
     {
        Ecore_X_Randr_Refresh_Rate ret = 0.0;

        /* try to get the current refresh rate */
        ret = XRRConfigCurrentRate(cfg);

        /* free the returned screen config */
        XRRFreeScreenConfigInfo(cfg);

        return ret;
     }
#endif
   return 0.0;
}

/*
 * @param root window which's primary output will be queried
 * @param size_index referencing the size to query valid refresh rates for
 * @return currently used refresh rate or - if request failed or RandRR is not available - NULL
 */
EAPI Ecore_X_Randr_Refresh_Rate *
ecore_x_randr_screen_primary_output_refresh_rates_get(Ecore_X_Window root, int size_index, int *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Refresh_Rate *rates = NULL;
   int n = 0;

   /* try to get the refresh rates for this screen */
   if ((rates = XRRRates(_ecore_x_disp, 
                         XRRRootToScreen(_ecore_x_disp, root), size_index, &n)))
     {
        Ecore_X_Randr_Refresh_Rate *ret = NULL;

        if (n == 0) return NULL;

        /* try to allocate space for the return */
        if ((ret = malloc(n * sizeof(Ecore_X_Randr_Refresh_Rate))))
          {
             int i = 0;

             /* fill in our return values */
             for (i = 0; i < n; i++)
               ret[i] = rates[i];

             if (num) *num = n;

             return ret;
          }
     }
#endif
   return NULL;
}

/*
 * @brief Sets the current primary output's refresh rate.
 *
 * @param root Window which's primary output will be queried.
 * @param size_index Referencing the size to be set.
 * @param rate The refresh rate to be set.
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool 
ecore_x_randr_screen_primary_output_refresh_rate_set(Ecore_X_Window root, int size_index, Ecore_X_Randr_Refresh_Rate rate)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_1)
     {
        XRRScreenConfiguration *cfg = NULL;

        /* try to get the screen config from XRandr */
        if ((cfg = XRRGetScreenInfo(_ecore_x_disp, root)))
          {
             Eina_Bool ret = EINA_FALSE;
             Rotation rot = 0;

             /* get the current rotation */
             XRRConfigRotations(cfg, &rot);
 
             /* try to set the new screen config
              * 
              * NB: Returns Success (0) if it works */
             if (!XRRSetScreenConfigAndRate(_ecore_x_disp, cfg, root, 
                                            size_index, rot, rate, CurrentTime))
               {
                  ret = EINA_TRUE;
               }

             /* free the returned screen config */
             XRRFreeScreenConfigInfo(cfg);

             return ret;
          }
     }
#endif
   return EINA_FALSE;
}

/***************************************
 * API Functions for RandR version 1.2 *
 ***************************************/

/**
 * @brief Enable event selection. This enables basic interaction with
 * output/crtc events and requires RandR >= 1.2.
 *
 * @param win Select this window's properties for RandR events.
 * @param on Enable/disable selecting.
 */
EAPI void 
ecore_x_randr_events_select(Ecore_X_Window win, Eina_Bool on)
{
#ifdef ECORE_XRANDR
   int mask = 0;

   if (on)
     {
        mask = RRScreenChangeNotifyMask;
        if (_randr_version >= RANDR_VERSION_1_2)
          mask |= (RRCrtcChangeNotifyMask | RROutputChangeNotifyMask | 
                   RROutputPropertyNotifyMask);
     }

   /* tell randr what events we want to listen to for this window */
   XRRSelectInput(_ecore_x_disp, win, mask);
#endif
}

/*
 * @param w width of screen in px
 * @param h height of screen in px
 */
EAPI void 
ecore_x_randr_screen_current_size_get(Ecore_X_Window root, int *w, int *h, int *w_mm, int *h_mm)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     {
        int scr = 0;

        /* get the screen number */
        scr = XRRRootToScreen(_ecore_x_disp, root);

        if (w) *w = DisplayWidth(_ecore_x_disp, scr);
        if (h) *h = DisplayHeight(_ecore_x_disp, scr);
        if (w_mm) *w_mm = DisplayWidthMM(_ecore_x_disp, scr);
        if (h_mm) *h_mm = DisplayHeightMM(_ecore_x_disp, scr);
     }
#endif
}

/*
 * @param root window which's screen will be queried
 * @param wmin minimum width the screen can be set to
 * @param hmin minimum height the screen can be set to
 * @param wmax maximum width the screen can be set to
 * @param hmax maximum height the screen can be set to
 */
EAPI void 
ecore_x_randr_screen_size_range_get(Ecore_X_Window root, int *wmin, int *hmin, int *wmax, int *hmax)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     {
        int swmin = 0, shmin = 0, swmax = 0, shmax = 0;

        /* try to get the screen size range from XRandr
         * 
         * NB: returns 1 on success */
        if ((XRRGetScreenSizeRange(_ecore_x_disp, root, &swmin, &shmin, 
                                   &swmax, &shmax)))
          {
             /* fill in the return variables */
             if (wmin) *wmin = swmin;
             if (hmin) *hmin = shmin;
             if (wmax) *wmax = swmax;
             if (hmax) *hmax = shmax;
          }
     }
#endif
}

/**
 * @brief removes unused screen space. The most upper left CRTC is set to 0x0
 * and all other CRTCs dx,dy respectively.
 * @param root the window's screen which will be reset.
 */
EAPI void 
ecore_x_randr_screen_reset(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        if (res->ncrtc > 0)
          {
             Ecore_X_Randr_Crtc crtcs[res->ncrtc];
             int i = 0, nenabled = 0;
             int nw = 0, nh = 0;
             int dx = 100000, dy = 100000;

             for (i = 0; i < res->ncrtc; i++)
               {
                  XRRCrtcInfo *info = NULL;

                  /* try to get the crtc info from Xrandr */
                  if (!(info = XRRGetCrtcInfo(_ecore_x_disp, res, res->crtcs[i])))
                    continue;

                  /* safety check */
                  if ((info->mode <= 0) || (info->noutput == 0))
                    {
                       /* free the crtc info */
                       XRRFreeCrtcInfo(info);

                       continue;
                    }

                  crtcs[nenabled++] = res->crtcs[i];

                  if ((int)(info->x + info->width) > nw)
                    nw = (info->x + info->width);

                  if ((int)(info->y + info->height) > nh)
                    nh = (info->y + info->height);

                  if (info->x < dx) dx = info->x;
                  if (info->y < dy) dy = info->y;

                  /* free the crtc info */
                  XRRFreeCrtcInfo(info);
               }

             /* free the resources */
             XRRFreeScreenResources(res);

             if ((dx > 0) || (dy > 0))
               {
                  if (ecore_x_randr_move_crtcs(root, crtcs, nenabled, -dx, -dy))
                    {
                       nw -= dx;
                       nh -= dy;
                    }
               }

             ecore_x_randr_screen_current_size_set(root, nw, nh, -1, -1);
          }
     }
#endif
}

/*
 * @param root Window which's screen's size should be set. If invalid (e.g. 
 * @c NULL) no action is taken.
 * @param w Width in px the screen should be set to. If out of valid
 * boundaries, current value is assumed.
 * @param h Height in px the screen should be set to. If out of valid
 * boundaries, current value is assumed.
 * @param w_mm Width in mm the screen should be set to. If @c 0, current
 * aspect is assumed.
 * @param h_mm Height in mm the screen should be set to. If @c 0, current
 * aspect is assumed.
 * @return @c EINA_TRUE if request was successfully sent or screen is already
 * in requested size, @c EINA_FALSE if parameters are invalid.
 */
EAPI Eina_Bool 
ecore_x_randr_screen_current_size_set(Ecore_X_Window root, int w, int h, int w_mm, int h_mm)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     {
        int cw = 0, ch = 0, cwmm = 0, chmm = 0;
        int wmin = 0, hmin = 0, wmax = 0, hmax = 0;

        /* get the current screen size */
        ecore_x_randr_screen_current_size_get(root, &cw, &ch, &cwmm, &chmm);

        /* compare to the values passed in. if there are no changes, get out */
        if ((w == cw) && (h == ch) &&
            ((w_mm == -1) || (w_mm == cwmm)) &&
            ((h_mm == -1) || (h_mm == chmm)))
          return EINA_TRUE;

        /* get the current size range */
        ecore_x_randr_screen_size_range_get(root, &wmin, &hmin, &wmax, &hmax);

        /* compare to the values passed in. make sure they are within range */
        if ((w != 0) && ((w < wmin) || (w > wmax))) return EINA_FALSE;
        if ((h != 0) && ((h < hmin) || (h > hmax))) return EINA_FALSE;

        /* safety check some values */
        if (w <= 0) w = cw;
        if (h <= 0) h = ch;
        if (w_mm <= 0)
          {
             if ((double)cw > 0.0)
               w_mm = (int)(((double)(cwmm / (double)cw)) * (double)w);
             else
               w_mm = (int)(((double)(cwmm)) * (double)w);
          }
        if (h_mm <= 0)
          {
             if ((double)ch > 0.0)
               h_mm = (int)(((double)(chmm / (double)ch)) * (double)h);
             else
               h_mm = (int)(((double)(chmm)) * (double)h);
          }

        /* tell XRandr to set screen size */
        XRRSetScreenSize(_ecore_x_disp, root, w, h, w_mm, h_mm);

        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

/*
 * @brief get detailed information for all modes related to a root window's screen
 * @param root window which's screen's resources are queried
 * @param num number of modes returned
 * @return modes' information
 */
EAPI Ecore_X_Randr_Mode_Info **
ecore_x_randr_modes_info_get(Ecore_X_Window root, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        Ecore_X_Randr_Mode_Info **ret = NULL;

        /* set the returned number of modes */
        if (num) *num = res->nmode;

        /* if we did not get any modes from X, then cleanup and return */
        if (res->nmode == 0)
          {
             /* free the resources */
             XRRFreeScreenResources(res);

             return NULL;
          }

        /* try to allocate space for our return variable */
        if ((ret = (Ecore_X_Randr_Mode_Info **)
             malloc(res->nmode * sizeof(Ecore_X_Randr_Mode_Info *))))
          {
             int i = 0;

             /* loop through all the modes and assign to our return var */
             for (i = 0; i < res->nmode; i++)
               {
                  if ((ret[i] = malloc(sizeof(Ecore_X_Randr_Mode_Info))))
                    {
                       ret[i]->xid = res->modes[i].id;
                       ret[i]->width = res->modes[i].width;
                       ret[i]->height = res->modes[i].height;
                       ret[i]->dotClock = res->modes[i].dotClock;
                       ret[i]->hSyncStart = res->modes[i].hSyncStart;
                       ret[i]->hSyncEnd = res->modes[i].hSyncEnd;
                       ret[i]->hTotal = res->modes[i].hTotal;
                       ret[i]->hSkew = res->modes[i].hSkew;
                       ret[i]->vSyncStart = res->modes[i].vSyncStart;
                       ret[i]->vSyncEnd = res->modes[i].vSyncEnd;
                       ret[i]->vTotal = res->modes[i].vTotal;
                       if (res->modes[i].nameLength > 0)
                         {
                            if ((ret[i]->name = 
                                 (malloc(res->modes[i].nameLength + 1))))
                              strncpy(ret[i]->name, res->modes[i].name,
                                      (res->modes[i].nameLength + 1));
                            else
                              ret[i]->name = NULL;
                         }
                       else
                         ret[i]->name = NULL;

                       ret[i]->nameLength = res->modes[i].nameLength;
                       ret[i]->modeFlags = res->modes[i].modeFlags;
                    }
                  else
                    {
                       while (i > 0)
                         free(ret[--i]);
                       free(ret);
                       ret = NULL;
                       break;
                    }
               }
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @brief Add a mode to a display.
 *
 * @param root Window to which's screen's ressources are added.
 * @param mode_info
 * @return Ecore_X_Randr_Mode of the added mode. Ecore_X_Randr_None if mode
 * adding failed.
 * @since 1.2.0
 */
EAPI Ecore_X_Randr_Mode 
ecore_x_randr_mode_info_add(Ecore_X_Window root, Ecore_X_Randr_Mode_Info *mode_info)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     {
        Ecore_X_Randr_Mode mode = 0;

        /* if we have valid mode_info from the user, then ask XRandr to 
         * create the new mode using that as base */
        if (mode_info)
          mode = XRRCreateMode(_ecore_x_disp, root, (XRRModeInfo *)mode_info);

        return mode;
     }
#endif
   return 0;
}

/*
 * @brief Delete a mode from the display.
 *
 * @param mode_info
 * @since 1.2.0
 */
EAPI void 
ecore_x_randr_mode_del(Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     XRRDestroyMode(_ecore_x_disp, mode);
#endif
}

/*
 * @brief get detailed information for a given mode id
 * @param root window which's screen's ressources are queried
 * @param mode the XID which identifies the mode of interest
 * @return mode's detailed information
 */
EAPI Ecore_X_Randr_Mode_Info *
ecore_x_randr_mode_info_get(Ecore_X_Window root, Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        Ecore_X_Randr_Mode_Info *ret = NULL;
        int i = 0;

        if (res->nmode == 0)
          {
             /* free the resources */
             XRRFreeScreenResources(res);

             return NULL;
          }

        /* loop the mode informations and find the one we want */
        for (i = 0; i < res->nmode; i++)
          {
             /* compare mode ids */
             if (res->modes[i].id != mode) continue;

             /* try to allocate our return mode information structure */
             if (!(ret = malloc(sizeof(Ecore_X_Randr_Mode_Info))))
               break;

             /* copy the mode information into our return structure */
             ret->xid = res->modes[i].id;
             ret->width = res->modes[i].width;
             ret->height = res->modes[i].height;
             ret->dotClock = res->modes[i].dotClock;
             ret->hSyncStart = res->modes[i].hSyncStart;
             ret->hSyncEnd = res->modes[i].hSyncEnd;
             ret->hTotal = res->modes[i].hTotal;
             ret->hSkew = res->modes[i].hSkew;
             ret->vSyncStart = res->modes[i].vSyncStart;
             ret->vSyncEnd = res->modes[i].vSyncEnd;
             ret->vTotal = res->modes[i].vTotal;
             ret->modeFlags = res->modes[i].modeFlags;
             ret->name = NULL;
             ret->nameLength = 0;
             if (res->modes[i].nameLength > 0)
               {
                  ret->nameLength = res->modes[i].nameLength;
                  if ((ret->name = malloc(res->modes[i].nameLength + 1)))
                    strncpy(ret->name, res->modes[i].name,
                            (res->modes[i].nameLength + 1));
               }
             break;
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @brief Free detailed mode information. The pointer handed in will be set to
 * @c NULL after freeing the memory.
 *
 * @param mode_info The mode information that should be freed.
 */
EAPI void 
ecore_x_randr_mode_info_free(Ecore_X_Randr_Mode_Info *mode_info)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     {
        if (mode_info)
          {
             if (mode_info->name) free(mode_info->name);
             free(mode_info);
             mode_info = NULL;
          }
     }
#endif
}

/*
 * @brief Get all known CRTCs related to a root window's screen.
 *
 * @param root Window which's screen's ressources are queried.
 * @param num Number of CRTCs returned.
 * @return CRTC IDs.
 */
EAPI Ecore_X_Randr_Crtc *
ecore_x_randr_crtcs_get(Ecore_X_Window root, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        Ecore_X_Randr_Crtc *ret = NULL;

        if (res->ncrtc == 0)
          {
             /* free the resources */
             XRRFreeScreenResources(res);

             return NULL;
          }

        /* try to allocate space for our return variable */
        if ((ret = malloc(res->ncrtc * sizeof(Ecore_X_Randr_Crtc))))
          {
             int i = 0;

             if (num) *num = res->ncrtc;

             /* copy the crtc information into our return variable */
             for (i = 0; i < res->ncrtc; i++)
               ret[i] = res->crtcs[i];
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

EAPI Ecore_X_Randr_Output *
ecore_x_randr_outputs_get(Ecore_X_Window root, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        Ecore_X_Randr_Output *ret = NULL;

        if (res->noutput == 0)
          {
             /* free the resources */
             XRRFreeScreenResources(res);

             return NULL;
          }

        /* try to allocate space for our return variable */
        if ((ret = malloc(res->noutput * sizeof(Ecore_X_Randr_Output))))
          {
             int i = 0;

             if (num) *num = res->noutput;

             /* copy the output information into our return variable */
             for (i = 0; i < res->noutput; i++)
               ret[i] = res->outputs[i];
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @brief Get the outputs, which display a certain window.
 *
 * @param window Window the displaying outputs shall be found for
 * @param num The number of outputs displaying the window
 * @return Array of outputs that display a certain window. @c NULL if no
 * outputs was found that displays the specified window.
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_window_outputs_get(Ecore_X_Window window, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   Ecore_X_Window root;
   Ecore_X_Randr_Crtc *crtcs = NULL;
   int ncrtcs = 0;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* grab this windows root window */
   root = ecore_x_window_root_get(window);

   /* get the crtcs from xrandr */
   if ((crtcs = ecore_x_randr_crtcs_get(root, &ncrtcs)))
     {
        XRRScreenResources *res = NULL;
        Ecore_X_Randr_Output *ret = NULL;

        /* try to get the screen resources from Xrandr
         * 
         * NB: We do this ONCE here as we reuse it for every crtc.
         * NB: The old code used to loop and fetch the screen resources on 
         *     every crtc */
        if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
          {
             Ecore_X_Randr_Output *tret = NULL;
             int i = 0, nret = 0;

             /* for each crtc, get it's outputs */
             for (i = 0, nret = 0; i < ncrtcs; i++)
               {
                  XRRCrtcInfo *crtc = NULL;

                  /* try to get the crtc info for this crtc */
                  if (!(crtc = XRRGetCrtcInfo(_ecore_x_disp, res, crtcs[i])))
                    continue;
                  
                  if (crtc->noutput > 0)
                    {
                       /* try to reallocate our return variable */
                       if ((tret = realloc(ret, ((nret + crtc->noutput) * 
                                                 sizeof(Ecore_X_Randr_Output)))))
                         {
                            ret = tret;
                            memcpy(&ret[nret], crtc->outputs, 
                                   (crtc->noutput * sizeof(Ecore_X_Randr_Output)));
                            nret += crtc->noutput;
                         }
                    }
                  /* free the crtc info */
                  XRRFreeCrtcInfo(crtc);
               }

             if (num) *num = nret;

             /* free the resources */
             XRRFreeScreenResources(res);
          }

        /* free any allocated crtcs from the get function */
        free(crtcs);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @deprecated bad naming. Use ecore_x_randr_window_outputs_get instead.
 * @brief Get the outputs, which display a certain window.
 *
 * @param window Window the displaying outputs shall be found for.
 * @param num The number of outputs displaying the window.
 * @return Array of outputs that display a certain window. @c NULL if no
 * outputs was found that displays the specified window.
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_current_output_get(Ecore_X_Window window, int *num)
{
   return ecore_x_randr_window_outputs_get(window, num);
}

/*
 * @brief get the CRTCs, which display a certain window
 * @param window window the displaying crtcs shall be found for
 * @param num the number of crtcs displaying the window
 * @return Array of crtcs that display a certain window. @c NULL if no crtcs
 * was found that displays the specified window.
 * @since 1.2.0
 */
EAPI Ecore_X_Randr_Crtc *
ecore_x_randr_window_crtcs_get(Ecore_X_Window window, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   Ecore_X_Window root;
   Ecore_X_Randr_Crtc *crtcs = NULL;
   int ncrtcs = 0;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* grab this windows root window */
   root = ecore_x_window_root_get(window);

   /* get the crtcs from xrandr */
   if ((crtcs = ecore_x_randr_crtcs_get(root, &ncrtcs)))
     {
        XRRScreenResources *res = NULL;
        Ecore_X_Randr_Crtc *ret = NULL;

        if (ncrtcs < 1)
          {
             free(crtcs);
             return NULL;
          }

        /* make sure we can allocate our return variable */
        if (!(ret = calloc(1, ncrtcs * sizeof(Ecore_X_Randr_Crtc))))
          {
             free(crtcs);
             return NULL;
          }

        /* try to get the screen resources from Xrandr
         * 
         * NB: We do this ONCE here as we reuse it for every crtc.
         * NB: The old code used to loop and fetch the screen resources on 
         *     every crtc */
        if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
          {
             Window tw;
             Eina_Rectangle wrect, crect;
             int i = 0, nret = 0;

             /* get the geometry of this window */
             ecore_x_window_geometry_get(window, &wrect.x, &wrect.y, 
                                         &wrect.w, &wrect.h);

             /* translate coordinates relative to root window */
             XTranslateCoordinates(_ecore_x_disp, window, root, 
                                   0, 0, &wrect.x, &wrect.y, &tw);

             for (i = 0; i < ncrtcs; i++)
               {
                  XRRCrtcInfo *info = NULL;

                  /* try to get crtc info */
                  if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtcs[i])))
                    {
                       /* check if crtc is enabled */
                       if (info->mode != 0)
                         {
                            /* enabled. get geometry */
                            crect.x = info->x;
                            crect.y = info->y;
                            crect.w = info->width;
                            crect.h = info->height;

                            /* check intersection with window */
                            if (eina_rectangles_intersect(&wrect, &crect))
                              {
                                 /* add if intersect */
                                 ret[nret] = crtcs[i];
                                 nret++;
                              }
                         }

                       /* free the crtc info */
                       XRRFreeCrtcInfo(info);
                    }
               }

             /* free the resources */
             XRRFreeScreenResources(res);

             if (num) *num = nret;
          }

        free(crtcs);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @deprecated bad naming. Use ecore_x_randr_window_crtcs_get instead.
 * @brief get the CRTCs, which display a certain window
 * @param window window the displaying crtcs shall be found for
 * @param num the number of crtcs displaying the window
 * @return Array of crtcs that display a certain window. @c NULL if no crtcs
 * was found that displays the specified window.
 */
EAPI Ecore_X_Randr_Crtc *
ecore_x_randr_current_crtc_get(Ecore_X_Window window, int *num)
{
   return ecore_x_randr_window_crtcs_get(window, num);
}

/*
 * @brief get a CRTC's outputs.
 * @param root the root window which's screen will be queried
 * @param num number of outputs referenced by given CRTC
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_crtc_outputs_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Output *ret = NULL;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             /* if we have no outputs, return NULL */
             if (info->noutput == 0)
               {
                  /* free the crtc info */
                  XRRFreeCrtcInfo(info);

                  /* free the resources */
                  XRRFreeScreenResources(res);

                  return NULL;
               }

             /* try to allocate our return struct */
             if ((ret = malloc(info->noutput * sizeof(Ecore_X_Randr_Output))))
               {
                  int i = 0;

                  /* loop the outputs on this crtc */
                  for (i = 0; i < info->noutput; i++)
                    ret[i] = info->outputs[i];

                  if (num) *num = info->noutput;
               }

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @brief get a CRTC's possible outputs.
 * @param root the root window which's screen will be queried
 * @param num number of possible outputs referenced by given CRTC
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_crtc_possible_outputs_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *num)
{
   if (num) *num = 0;
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Output *ret = NULL;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             if (info->npossible == 0)
               {
                  /* free the crtc info */
                  XRRFreeCrtcInfo(info);

                  /* free the resources */
                  XRRFreeScreenResources(res);

                  return NULL;
               }

             /* try to allocate our return struct */
             if ((ret = malloc(info->npossible * sizeof(Ecore_X_Randr_Output))))
               {
                  int i = 0;

                  /* loop the outputs on this crtc */
                  for (i = 0; i < info->npossible; i++)
                    ret[i] = info->possible[i];

                  if (num) *num = info->npossible;
               }

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

EAPI void 
ecore_x_randr_crtc_geometry_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             if (x) *x = info->x;
             if (y) *y = info->y;
             if (w) *w = info->width;
             if (h) *h = info->height;

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);
     }
#endif
}

EAPI void 
ecore_x_randr_crtc_pos_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return;

   ecore_x_randr_crtc_geometry_get(root, crtc, x, y, NULL, NULL);
#endif
}

/*
 * @brief Sets the position of given CRTC within root window's screen.
 *
 * @param root The window's screen to be queried.
 * @param crtc The CRTC which's position within the mentioned screen is to be
 * altered.
 * @param x Position on the x-axis (0 == left) of the screen. if x < 0 current
 * value will be kept.
 * @param y Position on the y-ayis (0 == top) of the screen. if y < 0, current
 * value will be kept.
 * @return @c EINA_TRUE if position could successfully be altered.
 */
EAPI Eina_Bool 
ecore_x_randr_crtc_pos_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int x, int y)
{
#ifdef ECORE_XRANDR
   int cx = 0, cy = 0, cw = 0, ch = 0;
   int sw = 0, sh = 0, nw = 0, nh = 0;

   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   /* grab the current crtc geometry */
   ecore_x_randr_crtc_geometry_get(root, crtc, &cx, &cy, &cw, &ch);

   /* grab the current screen geometry */
   ecore_x_randr_screen_current_size_get(root, &sw, &sh, NULL, NULL);

   /* safety check some values */
   if (x < 0) x = cx;
   if (y < 0) y = cy;
   if ((x + cw) > sw) nw = (x + cw);
   if ((y + ch) > sh) nh = (y + ch);

   if ((nw > 0) && (nh > 0))
     {
        /* try to update the current screen geometry */
        if (!ecore_x_randr_screen_current_size_set(root, nw, nh, 0, 0))
          return EINA_FALSE;
     }

   /* try to set the new crtc position */
   return ecore_x_randr_crtc_settings_set(root, crtc, NULL, -1, x, y, -1, -1);
#endif
   return EINA_FALSE;
}

/**
 * @brief Get the current set mode of a given CRTC
 * @param root the window's screen to be queried
 * @param crtc the CRTC which's should be queried
 * @return currently set mode or - in case parameters are invalid -
 * Ecore_X_Randr_Unset
 */
EAPI Ecore_X_Randr_Mode 
ecore_x_randr_crtc_mode_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return -1;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Mode ret = -1;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             /* get the mode */
             ret = info->mode;

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return -1;
}

/**
 * @brief Sets a mode for a CRTC and the outputs attached to it.
 *
 * @param root The window's screen to be queried.
 * @param crtc The CRTC which shall be set.
 * @param outputs Array of outputs which have to be compatible with the mode.
 * If @c NULL, CRTC will be disabled.
 * @param noutputs Number of outputs in array to be used. Use
 * Ecore_X_Randr_Unset (or @c -1) to use currently used outputs.
 * @param mode XID of the mode to be set. If set to @c 0 the CRTC will be
 * disabled. If set to @c -1 the call will fail.
 * @return @c EINA_TRUE if mode setting was successful, @c EINA_FALSE
 * otherwise.
 */
EAPI Eina_Bool 
ecore_x_randr_crtc_mode_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, Ecore_X_Randr_Output *outputs, int noutputs, Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   return ecore_x_randr_crtc_settings_set(root, crtc, outputs, noutputs, 
                                          -1, -1, mode, -1);
#endif
   return EINA_FALSE;
}

EAPI void 
ecore_x_randr_crtc_size_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *w, int *h)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return;
   ecore_x_randr_crtc_geometry_get(root, crtc, NULL, NULL, w, h);
#endif
}

EAPI Ecore_X_Randr_Refresh_Rate 
ecore_x_randr_crtc_refresh_rate_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc EINA_UNUSED, Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return 0.0;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        int i = 0;
        double ret = 0.0;

        for (i = 0; i < res->nmode; i++)
          {
             if (res->modes[i].id == mode)
               {
                  if ((res->modes[i].hTotal) && (res->modes[i].vTotal))
                    {
                       ret = ((double)res->modes[i].dotClock /
                              ((double)res->modes[i].hTotal *
                                  (double)res->modes[i].vTotal));
                    }
                  break;
               }
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return 0.0;
}

EAPI Ecore_X_Randr_Orientation 
ecore_x_randr_crtc_orientations_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return 0;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Orientation ret = 0;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             /* get the mode */
             ret = info->rotations;

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return 0;
}

EAPI Ecore_X_Randr_Orientation 
ecore_x_randr_crtc_orientation_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return 0;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Orientation ret = 0;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             /* get the mode */
             ret = info->rotation;

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return 0;
}

EAPI Eina_Bool 
ecore_x_randr_crtc_orientation_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const Ecore_X_Randr_Orientation orientation)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   if (orientation != 0)
     return ecore_x_randr_crtc_settings_set(root, crtc, NULL, 
                                            -1, -1, -1, -1, orientation);
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_randr_crtc_clone_set(Ecore_X_Window root, Ecore_X_Randr_Crtc original, Ecore_X_Randr_Crtc cln)
{
#ifdef ECORE_XRANDR
   Eina_Bool ret = EINA_FALSE;
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Orientation orig_orient = 0;
        Ecore_X_Randr_Mode orig_mode = -1;
        int ox = 0, oy = 0;

        /* try to get crtc info for original crtc */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, original)))
          {
             ox = info->x;
             oy = info->y;
             orig_orient = info->rotation;
             orig_mode = info->mode;

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        ret = ecore_x_randr_crtc_settings_set(root, cln, NULL, -1, ox, oy, 
                                              orig_mode, orig_orient);

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return EINA_FALSE;
}

/**
 * @brief Sets the demanded parameters for a given CRTC. Note that the CRTC is
 * auto enabled in it's preferred mode, when it was disabled before.
 *
 * @param root The root window which's default display will be queried.
 * @param crtc The CRTC which's configuration should be altered.
 * @param outputs An array of outputs, that should display this CRTC's content.
 * @param noutputs Number of outputs in the array of outputs. If set to
 * Ecore_X_Randr_Unset, current outputs and number of outputs will be used.
 * If set to Ecore_X_Randr_None, CRTC will be disabled.
 * @param x New x coordinate. If <0 (e.g. Ecore_X_Randr_Unset) the current x
 * corrdinate will be assumed.
 * @param y New y coordinate. If <0 (e.g. Ecore_X_Randr_Unset) the current y
 * corrdinate will be assumed.
 * @param mode The new mode to be set. If Ecore_X_Randr_None is passed, the
 * CRTC will be disabled. If Ecore_X_Randr_Unset is passed, the current mode is
 * assumed.
 * @param orientation The new orientation to be set. If Ecore_X_Randr_Unset is
 * used, the current mode is assumed.
 * @return @c EINA_TRUE if the configuration alteration was successful,
 * @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool 
ecore_x_randr_crtc_settings_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, Ecore_X_Randr_Output *outputs, int noutputs, int x, int y, Ecore_X_Randr_Mode mode, Ecore_X_Randr_Orientation orientation)
{
#ifdef ECORE_XRANDR
   Eina_Bool ret = EINA_FALSE;
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        RRCrtc rcrtc;
        RROutput *routputs = NULL;
        XRRCrtcInfo *info = NULL;
        Eina_Bool need_free = EINA_FALSE;
        int i = 0;

        rcrtc = (RRCrtc)crtc;

        /* try to get crtc info for original crtc */
        if (!(info = XRRGetCrtcInfo(_ecore_x_disp, res, rcrtc)))
          {
             /* free the resources */
             XRRFreeScreenResources(res);
             return EINA_FALSE;
          }

        if ((int)mode == -1) mode = info->mode;
        if ((int)orientation == -1) orientation = info->rotation;
        if (x < 0) x = info->x;
        if (y < 0) y = info->y;

        if (noutputs < 0)
          {
             noutputs = info->noutput;
             if (noutputs > 0)
               {
                  routputs = malloc(noutputs * sizeof(RROutput));
                  for (i = 0; i < noutputs; i++)
                    routputs[i] = info->outputs[i];
                  need_free = EINA_TRUE;
               }
          }
        else if (noutputs > 0)
          {
             routputs = malloc(noutputs * sizeof(RROutput));
             for (i = 0; i < noutputs; i++)
               routputs[i] = (RROutput)outputs[i];
             need_free = EINA_TRUE;
          }

        /* try to set the crtc config */
        if (!XRRSetCrtcConfig(_ecore_x_disp, res, rcrtc, CurrentTime, 
                              x, y, mode, orientation, 
                              routputs, noutputs))
          ret = EINA_TRUE;

        if (need_free) free(routputs);

        /* free the crtc info */
        XRRFreeCrtcInfo(info);

        /* free the resources */
        XRRFreeScreenResources(res);
     }

   return ret;
#endif
   return EINA_FALSE;
}

/**
 * @brief Sets a CRTC relative to another one.
 *
 * @param root The root window which's default display will be set.
 * @param crtc_r1 The CRTC to be positioned.
 * @param crtc_r2 The CRTC the position should be relative to.
 * @param policy The relation between the crtcs.
 * @param alignment In case CRTCs size differ, aligns CRTC1 accordingly at
 * CRTC2's borders.
 * @return @c EINA_TRUE if crtc could be successfully positioned, @c EINA_FALSE
 * if repositioning failed or if position of new crtc would be out of given
 * screen's min/max bounds.
 */
EAPI Eina_Bool 
ecore_x_randr_crtc_pos_relative_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc_r1, Ecore_X_Randr_Crtc crtc_r2, Ecore_X_Randr_Output_Policy policy, Ecore_X_Randr_Relative_Alignment alignment)
{
#ifdef ECORE_XRANDR
   Eina_Rectangle r1, r2;
   int mw = 0, mh = 0, sw = 0, sh = 0;
   int nx = 0, ny = 0;

   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   /* check each crtc has a valid mode */
   if (ecore_x_randr_crtc_mode_get(root, crtc_r1) == 0) return EINA_FALSE;
   if (ecore_x_randr_crtc_mode_get(root, crtc_r2) == 0) return EINA_FALSE;

   /* get the geometry of each crtc */
   ecore_x_randr_crtc_geometry_get(root, crtc_r1, &r1.x, &r1.y, &r1.w, &r1.h);
   ecore_x_randr_crtc_geometry_get(root, crtc_r2, &r2.x, &r2.y, &r2.w, &r2.h);

   /* get the geometry of the screen */
   ecore_x_randr_screen_size_range_get(root, NULL, NULL, &mw, &mh);
   ecore_x_randr_screen_current_size_get(root, &sw, &sh, NULL, NULL);

   switch (policy)
     {
      case ECORE_X_RANDR_OUTPUT_POLICY_RIGHT:
        nx = (r2.x + r2.w);

        switch (alignment)
          {
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
             ny = -1;
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
             ny = ((int)(((double)r2.h / 2.0) + r2.y - ((double)r1.h / 2.0)));
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
             ny = ((int)((double)sh / 2.0) - ((double)r1.h / 2.0));
             break;
          }

        break;
      case ECORE_X_RANDR_OUTPUT_POLICY_LEFT:
        nx = (r2.x - r1.w);

        switch (alignment)
          {
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
             ny = -1;
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
             ny = ((int)(((double)r2.h / 2.0) + r2.y - ((double)r1.h / 2.0)));
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
             ny = ((int)((double)sh / 2.0) - ((double)r1.h / 2.0));
             break;
          }

        break;
      case ECORE_X_RANDR_OUTPUT_POLICY_BELOW:
        ny = (r2.y + r2.h);

        switch (alignment)
          {
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
             nx = -1;
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
             nx = ((int)((((double)r2.x + (double)r2.w) / 2.0) - 
                         ((double)r1.w / 2.0)));
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
             nx = ((int)((double)sw / 2.0));
             break;
          }

        break;
      case ECORE_X_RANDR_OUTPUT_POLICY_ABOVE:
        ny = (r2.y - r1.h);

        switch (alignment)
          {
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
             nx = -1;
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
             nx = ((int)((((double)r2.x + (double)r2.w) / 2.0) - 
                         ((double)r1.w / 2.0)));
             break;
           case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
             nx = ((int)((double)sw / 2.0));
             break;
          }

        break;
      case ECORE_X_RANDR_OUTPUT_POLICY_CLONE:
        return ecore_x_randr_crtc_pos_set(root, crtc_r1, r2.x, r2.y);
        break;
      case ECORE_X_RANDR_OUTPUT_POLICY_NONE:
        break;
      default:
        return EINA_FALSE;
     }

   if ((nx == r1.x) && (ny == r1.y)) return EINA_TRUE;
   if (((ny + r1.h) > mh) || ((nx + r1.w) > mw)) return EINA_FALSE;

   return ecore_x_randr_crtc_pos_set(root, crtc_r1, nx, ny);
#else
   return EINA_FALSE;
#endif
}

/*
 * @since 1.8
 */
EAPI Ecore_X_Randr_Crtc_Info *
ecore_x_randr_crtc_info_get(Ecore_X_Window root, const Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRCrtcInfo *info = NULL;
        Ecore_X_Randr_Crtc_Info *ret = NULL;

        /* try to get crtc info */
        if ((info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             if ((ret = malloc(sizeof(Ecore_X_Randr_Crtc_Info))))
               {
                  /* copy the mode information into our return structure */
                  ret->timestamp = info->timestamp;
                  ret->x = info->x;
                  ret->y = info->y;
                  ret->width = info->width;
                  ret->height = info->height;
                  ret->mode = info->mode;
                  ret->rotation = info->rotation;
                  ret->rotations = info->rotations;
                  ret->noutput = info->noutput;
                  ret->npossible = info->npossible;

                  ret->outputs = NULL;
                  ret->possible = NULL;

                  if (info->noutput > 0)
                    {
                       if ((ret->outputs = 
                            malloc(info->noutput * sizeof(Ecore_X_Randr_Output))))
                         {
                            int i = 0;

                            /* loop the outputs on this crtc */
                            for (i = 0; i < info->noutput; i++)
                              ret->outputs[i] = info->outputs[i];
                         }
                    }

                  if (info->npossible > 0)
                    {
                       if ((ret->possible = 
                            malloc(info->npossible * sizeof(Ecore_X_Randr_Output))))
                         {
                            int i = 0;

                            /* loop the outputs on this crtc */
                            for (i = 0; i < info->npossible; i++)
                              ret->possible[i] = info->possible[i];
                         }
                    }
               }

             /* free the crtc info */
             XRRFreeCrtcInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @since 1.8
 */
EAPI void 
ecore_x_randr_crtc_info_free(Ecore_X_Randr_Crtc_Info *info)
{
#ifdef ECORE_XRANDR
   if (_randr_version >= RANDR_VERSION_1_2)
     {
        if (info)
          {
             if (info->outputs) free(info->outputs);
             if (info->possible) free(info->possible);
             free(info);
             info = NULL;
          }
     }
#endif
}

/*
 * @brief Add given mode to given output.
 *
 * @param output The output the mode is added to.
 * @param mode The mode added to the output.
 * @return @c EINA_FALSE if output or mode equal Ecore_X_Randr_None, else
 * @c EINA_TRUE.
 * Additionally, if xcb backend is used, the success of the addition is
 * reported back directly.
 * @since 1.2.0
 */
EAPI Eina_Bool 
ecore_x_randr_output_mode_add(Ecore_X_Randr_Output output, Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   if ((output) && (mode))
     {
        /* add this mode to output
         * 
         * NB: This XRR function returns void so we have to assume it worked */
        XRRAddOutputMode(_ecore_x_disp, output, mode);

        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

/*
 * @brief delete given mode from given output
 * @param output the output the mode is removed from
 * @param mode the mode removed from the output
 * @since 1.2.0
 */
EAPI void 
ecore_x_randr_output_mode_del(Ecore_X_Randr_Output output, Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return;

   if ((!output) || (!mode)) return;

   XRRDeleteOutputMode(_ecore_x_disp, output, mode);
#endif
}

EAPI Ecore_X_Randr_Mode *
ecore_x_randr_output_modes_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num, int *npreferred)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        Ecore_X_Randr_Mode *modes = NULL;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             if (num) *num = info->nmode;
             if (npreferred) *npreferred = info->npreferred;

             if (info->nmode > 0)
               {
                  if ((modes = malloc(info->nmode * sizeof(Ecore_X_Randr_Mode))))
                    {
                       int i = 0;

                       for (i = 0; i < info->nmode; i++)
                         modes[i] = info->modes[i];
                    }
               }

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return modes;
     }
#endif
   return NULL;
}

/**
 * @brief gets the the outputs which might be used simultenously on the same
 * CRTC.
 * @param root window that this information should be queried for.
 * @param output the output which's clones we concern
 * @param num number of possible clones
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_output_clones_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        Ecore_X_Randr_Output *outputs = NULL;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             if (num) *num = info->nclone;

             if (info->nclone > 0)
               {
                  /* try to allocate space for output return */
                  if ((outputs = malloc(info->nclone * sizeof(Ecore_X_Randr_Output))))
                    {
                       int i = 0;

                       for (i = 0; i < info->nclone; i++)
                         outputs[i] = info->clones[i];
                    }
               }

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return outputs;
     }
#endif
   return NULL;
}

EAPI Ecore_X_Randr_Crtc *
ecore_x_randr_output_possible_crtcs_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return 0;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        Ecore_X_Randr_Crtc *crtcs = NULL;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             if (num) *num = info->ncrtc;

             if (info->ncrtc > 0)
               {
                  /* try to allocate space for the return crtcs */
                  if ((crtcs = malloc(info->ncrtc * sizeof(Ecore_X_Randr_Crtc))))
                    {
                       int i = 0;

                       for (i = 0; i < info->ncrtc; i++)
                         crtcs[i] = info->crtcs[i];
                    }
               }

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return crtcs;
     }
#endif
   return NULL;
}

EAPI Ecore_X_Randr_Crtc 
ecore_x_randr_output_crtc_get(Ecore_X_Window root, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return 0;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        Ecore_X_Randr_Crtc ret = 0;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             ret = info->crtc;

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return 0;
}

/**
 * @brief gets the given output's name as reported by X
 * @param root the window which's screen will be queried
 * @param output The output for which the name will be reported.
 * @param len length of returned c-string.
 * @return name of the output as reported by X
 */
EAPI char *
ecore_x_randr_output_name_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *len)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return 0;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        char *ret = NULL;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             if (info->name)
               {
                  size_t s;
#ifdef XRANDR_GOOD
                  s = info->nameLen;
#else
                  /* pre 1.4.0 does not fill in info->nameLen */
                  s = strlen(info->name);
#endif
                  ret = malloc(s + 1);
                  memcpy(ret, info->name, s);
                  ret[s] = 0;
                  if (len) *len = s;
               }

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return NULL;
}

/*
 * @deprecated use ecore_x_randr_crtc_gamma_size_get()
 */
EINA_DEPRECATED EAPI int 
ecore_x_randr_crtc_gamma_ramp_size_get(Ecore_X_Randr_Crtc crtc EINA_UNUSED)
{
   return 0;
}

/*
 * @deprecated use ecore_x_randr_crtc_gamma_get()
 */
EINA_DEPRECATED EAPI Ecore_X_Randr_Crtc_Gamma **
ecore_x_randr_crtc_gamma_ramps_get(Ecore_X_Randr_Crtc crtc EINA_UNUSED)
{
   return NULL;
}

/*
 * @deprecated use ecore_x_randr_crtc_gamma_set()
 */
EINA_DEPRECATED EAPI Eina_Bool 
ecore_x_randr_crtc_gamma_ramps_set(Ecore_X_Randr_Crtc crtc EINA_UNUSED, const Ecore_X_Randr_Crtc_Gamma *red EINA_UNUSED, const Ecore_X_Randr_Crtc_Gamma *green EINA_UNUSED, const Ecore_X_Randr_Crtc_Gamma *blue EINA_UNUSED)
{
   return EINA_FALSE;
}

/*
 * @since 1.8
 */
EAPI int 
ecore_x_randr_crtc_gamma_size_get(Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return 0;
   return XRRGetCrtcGammaSize(_ecore_x_disp, crtc);
#else
   (void)crtc;
#endif
   return 0;
}

/*
 * @since 1.8
 */
EAPI Ecore_X_Randr_Crtc_Gamma_Info *
ecore_x_randr_crtc_gamma_get(Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Crtc_Gamma_Info *info = NULL;
   XRRCrtcGamma *xgamma = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the gamma for this crtc from Xrandr */
   if (!(xgamma = XRRGetCrtcGamma(_ecore_x_disp, crtc)))
     return NULL;

   /* try to allocate space for the return struct and copy the results in */
   if ((info = malloc(sizeof(Ecore_X_Randr_Crtc_Gamma_Info))))
     memcpy(info, xgamma, sizeof(Ecore_X_Randr_Crtc_Gamma_Info));

   /* free the returned gamma resource */
   XRRFreeGamma(xgamma);

   return info;
#else
   (void)crtc;
#endif
   return NULL;
}

/*
 * @since 1.8
 */
EAPI Eina_Bool 
ecore_x_randr_crtc_gamma_set(Ecore_X_Randr_Crtc crtc, const Ecore_X_Randr_Crtc_Gamma_Info *gamma)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   /* try to set the gamma
    * 
    * NB: XRRSetCrtcGamma returns void
    */
   XRRSetCrtcGamma(_ecore_x_disp, crtc, (XRRCrtcGamma *)gamma);
   return EINA_TRUE;
#else
   (void)crtc;
   (void)gamma;
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_randr_move_all_crtcs_but(Ecore_X_Window root, const Ecore_X_Randr_Crtc *not_moved, int nnot_moved, int dx, int dy)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;
   Eina_Bool ret = EINA_FALSE;

   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   if ((nnot_moved <= 0) || (!not_moved)) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        Ecore_X_Randr_Crtc *crtcs = NULL;
        int n = 0;

        n = (res->ncrtc - nnot_moved);
        if (n > 0)
          {
             /* try to allocate space for a list of crtcs */
             if ((crtcs = malloc(n * sizeof(Ecore_X_Randr_Crtc))))
               {
                  int i = 0, j = 0, k = 0;

                  for (i = 0, k = 0; (i < res->ncrtc) && (k < n); i++)
                    {
                       for (j = 0; j < nnot_moved; j++)
                         {
                            if (res->crtcs[i] == not_moved[j])
                              break;
                         }

                       if (j == nnot_moved) crtcs[k++] = res->crtcs[i];
                    }
               }
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        /* actually move the crtcs */
        if (crtcs)
          {
             ret = ecore_x_randr_move_crtcs(root, crtcs, n, dx, dy);
             free(crtcs);
          }

        return ret;
     }
#endif
   return EINA_FALSE;
}

/*
 * @brief Move given CRTCs belonging to the given root window's screen dx/dy
 * pixels relative to their current position. The screen size will be
 * automatically adjusted if necessary and possible.
 *
 * @param root Window which's screen's resources are used.
 * @param crtcs List of CRTCs to be moved.
 * @param ncrtc Number of CRTCs in array.
 * @param dx Amount of pixels the CRTCs should be moved in x direction.
 * @param dy Amount of pixels the CRTCs should be moved in y direction.
 * @return @c EINA_TRUE if all crtcs could be moved successfully.
 */
EAPI Eina_Bool 
ecore_x_randr_move_crtcs(Ecore_X_Window root, const Ecore_X_Randr_Crtc *crtcs, int ncrtc, int dx, int dy)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;
   XRRCrtcInfo **info = NULL;
   int i = 0;

   if (_randr_version < RANDR_VERSION_1_2) return EINA_FALSE;

   if (ncrtc < 1) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        Eina_Bool ret = EINA_TRUE;
        int mw = 0, mh = 0, sw = 0, sh = 0;
        int nw = 0, nh = 0;

        info = alloca(ncrtc * sizeof(XRRCrtcInfo *));
        memset(info, 0, ncrtc * sizeof(XRRCrtcInfo *));

        ecore_x_randr_screen_size_range_get(root, NULL, NULL, &mw, &mh);
        ecore_x_randr_screen_current_size_get(root, &sw, &sh, NULL, NULL);
        nw = sw;
        nh = sh;

        for (i = 0; i < ncrtc; i++)
          {
             /* try to get crtc info for original crtc */
             if ((info[i] = XRRGetCrtcInfo(_ecore_x_disp, res, crtcs[i])))
               {
                  if (((info[i]->x + dx) < 0) || ((info[i]->y + dy < 0)) || 
                      ((int)(info[i]->x + info[i]->width) > mw) || 
                      ((int)(info[i]->y + info[i]->height) > mh))
                    {
                       goto err;
                    }

                  nw = MAX(((int)(info[i]->x + info[i]->width) + dx), nw);
                  nh = MAX(((int)(info[i]->y + info[i]->height) + dy), nh);
               }
          }

        /* resize the screen if we need to */
        if (!(((nw > sw) || (nh > sh)) || 
              ecore_x_randr_screen_current_size_set(root, nw, nh, -1, -1)))
          goto err;

        /* actually move the crtcs */
        for (i = 0; ((i < ncrtc) && info[i]); i++)
          {
             if (!ecore_x_randr_crtc_settings_set(root, crtcs[i], NULL, -1, 
                                                  (info[i]->x + dx), 
                                                  (info[i]->y + dy), 
                                                  info[i]->mode, 
                                                  info[i]->rotation))
               {
                  ret = EINA_FALSE;
                  break;
               }
          }

        if (i < ncrtc)
          {
             /* something went wrong somewhere. move everything back */
             while ((--i) >= 0)
               {
                  if (info[i])
                    ecore_x_randr_crtc_settings_set(root, crtcs[i], NULL, -1, 
                                                    (info[i]->x - dx), 
                                                    (info[i]->y - dy), 
                                                    info[i]->mode, 
                                                    info[i]->rotation);
               }
          }

        /* cleanup */
        for (i = 0; i < ncrtc; i++)
          if (info[i]) XRRFreeCrtcInfo(info[i]);

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }

err:
   while (i-- > 0)
     {
        /* free the crtc info */
        if (info[i]) XRRFreeCrtcInfo(info[i]);
     }

   /* free the resources */
   if (res) XRRFreeScreenResources(res);
#endif
   return EINA_FALSE;
}

/**
 * @brief gets the width and hight of a given mode
 * @param mode the mode whose size is to be looked up
 * @param w width of given mode in px
 * @param h height of given mode in px
 */
EAPI void 
ecore_x_randr_mode_size_get(Ecore_X_Window root, Ecore_X_Randr_Mode mode, int *w, int *h)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_2) return;

   if ((mode != 0) && ((w) || (h)))
     {
        XRRScreenResources *res = NULL;

        /* try to get the screen resources from Xrandr */
        if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
          {
             int i = 0;

             for (i = 0; i < res->nmode; i++)
               {
                  if (res->modes[i].id == mode)
                    {
                       if (w) *w = res->modes[i].width;
                       if (h) *h = res->modes[i].height;
                       break;
                    }
               }

             /* free the resources */
             XRRFreeScreenResources(res);
          }
     }
#endif
}

EAPI Ecore_X_Randr_Connection_Status 
ecore_x_randr_output_connection_status_get(Ecore_X_Window root, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) 
     return ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        Ecore_X_Randr_Connection_Status ret = 
          ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             ret = info->connection;

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN;
}

EAPI void 
ecore_x_randr_output_size_mm_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *w, int *h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             if (w) *w = info->mm_width;
             if (h) *h = info->mm_height;

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);
     }
#endif
}

EAPI Eina_Bool 
ecore_x_randr_output_crtc_set(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output EINA_UNUSED, const Ecore_X_Randr_Crtc crtc EINA_UNUSED)
{
   /* TODO: !! */
   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_randr_output_backlight_available(void)
{
#ifdef ECORE_XRANDR
   Atom backlight = None;

   /* check for new backlight property */
   if ((backlight = XInternAtom(_ecore_x_disp, RR_PROPERTY_BACKLIGHT, True)))
     return EINA_TRUE;

   /* check for legacy backlight property */
   if ((backlight = XInternAtom(_ecore_x_disp, "BACKLIGHT", True)))
     return EINA_TRUE;
#endif
   return EINA_FALSE;
}

/**
 * @brief Set up the backlight level to the given level.
 *
 * @param root The window's screen which will be set.
 * @param level Of the backlight between @c 0 and @c 1.
 */
EAPI void 
ecore_x_randr_screen_backlight_level_set(Ecore_X_Window root, double level)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;
   int i = 0;

   if (_randr_version < RANDR_VERSION_1_3) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        /* set the backlight level on each output */
        for (i = 0; i < res->noutput; i++)
          ecore_x_randr_output_backlight_level_set(root, res->outputs[i], level);

        /* free the resources */
        XRRFreeScreenResources(res);
     }
#endif
}

EAPI double 
ecore_x_randr_output_backlight_level_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   XRRPropertyInfo *info = NULL;
   Atom backlight = None, type = None;
   unsigned long bytes = 0;
   unsigned long items = 0;
   unsigned char *prop = NULL;
   long val = 0;
   int format = 0;

   /* check if "new" backlight is available */
   if (_randr_version >= RANDR_VERSION_1_3)
     {
        if ((backlight = 
             XInternAtom(_ecore_x_disp, RR_PROPERTY_BACKLIGHT, True)))
          {
             XRRGetOutputProperty(_ecore_x_disp, output, backlight, 0, 4, 
                                  False, False, None, &type, &format, 
                                  &items, &bytes, &prop);
          }
     }

   if ((!prop) || (items == 0))
     {
        /* check legacy backlight property
         * 
         * FIXME: NB: Not sure what randr version we need for the legacy 
         * backlight property so skip version check */
        if ((backlight = XInternAtom(_ecore_x_disp, "BACKLIGHT", True)))
          {
             XRRGetOutputProperty(_ecore_x_disp, output, backlight, 0, 4, 
                                  False, False, None, &type, &format, 
                                  &items, &bytes, &prop);
          }
     }

   /* safety check */
   if ((!prop) || (type != XA_INTEGER) || (items != 1) || (format != 32))
     {
        WRN("Backlight property is not supported on this server or driver");
        return -1;
     }

   val = *((long *)prop);
   free(prop);

   /* try to get the backlight property value from Xrandr */
   if ((info = XRRQueryOutputProperty(_ecore_x_disp, output, backlight)))
     {
        double ret = -1;

        if ((info->range) && (info->num_values == 2))
          {
             /* convert the current value */
             ret = ((double)(val - info->values[0])) / 
               ((double)(info->values[1] - info->values[0]));
          }

        /* free the info */
        free(info);

        return ret;
     }
#endif
   return -1;
}

EAPI Eina_Bool 
ecore_x_randr_output_backlight_level_set(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output, double level)
{
#ifdef ECORE_XRANDR
   XRRPropertyInfo *info = NULL;
   Atom backlight = None;

   /* safety check some input values */
   if ((level < 0) || (level > 1))
     {
        ERR("Backlight level should be between 0 and 1");
        return EINA_FALSE;
     }

   /* check if "new" backlight is available */
   if (_randr_version >= RANDR_VERSION_1_3)
     backlight = XInternAtom(_ecore_x_disp, RR_PROPERTY_BACKLIGHT, True);

   if (!backlight)
     backlight = XInternAtom(_ecore_x_disp, "BACKLIGHT", True);

   if (!backlight)
     {
        WRN("Backlight property is not supported on this server or driver");
        return EINA_FALSE;
     }

   /* try to get the output property from Xrandr */
   if ((info = XRRQueryOutputProperty(_ecore_x_disp, output, backlight)))
     {
        Eina_Bool ret = EINA_FALSE;

        if ((info->range) && (info->num_values == 2))
          {
             double min = 0.0, max = 0.0;
             long val = 0;

             min = info->values[0];
             max = info->values[1];
             val = (level * (max - min)) + min;
             if (val > max) val = max;
             if (val < min) val = min;

             /* tell xrandr to change the backlight value */
             XRRChangeOutputProperty(_ecore_x_disp, output, backlight, 
                                     XA_INTEGER, 32, PropModeReplace, 
                                     (unsigned char *)&val, 1);

             /* send changes to X */
             ecore_x_flush();

             ret = EINA_TRUE;
          }

        /* free the info */
        free(info);

        return ret;
     }
#endif
   return EINA_FALSE;
}

/**
 * @brief gets the EDID information of an attached output if available.
 * Note that this information is not to be compared using ordinary string
 * comparison functions, since it includes 0-bytes.
 * @param root window this information should be queried from
 * @param output the XID of the output
 * @param length length of the byte-array. If NULL, request will fail.
 */
EAPI unsigned char *
ecore_x_randr_output_edid_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output, unsigned long *length)
{
#ifdef ECORE_XRANDR
   Atom edid = None, type = None;
   unsigned char *prop;
   int format = 0;
   unsigned long nitems = 0, bytes = 0;

   if (_randr_version < RANDR_VERSION_1_2) return NULL;

   /* try to get the edid atom */
   if (!(edid = XInternAtom(_ecore_x_disp, RR_PROPERTY_RANDR_EDID, False))) 
     return NULL;

   /* get the output property
    * 
    * NB: Returns 0 on success */
   if (!XRRGetOutputProperty(_ecore_x_disp, output, edid, 0, 128, False, False,
                            AnyPropertyType, &type, &format, &nitems, 
                            &bytes, &prop))
     {
        if ((type == XA_INTEGER) && (nitems >= 1) && (format == 8))
          {
             unsigned char *ret = NULL;

             if ((ret = malloc(nitems * sizeof(unsigned char))))
               {
                  if (length) *length = nitems;
                  memcpy(ret, prop, (nitems * sizeof(unsigned char)));
                  return ret;
               }
          }
     }
#endif
   return NULL;
}

EAPI Ecore_X_Render_Subpixel_Order 
ecore_x_randr_output_subpixel_order_get(Ecore_X_Window root, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_2) 
     return ECORE_X_RENDER_SUBPIXEL_ORDER_UNKNOWN;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRROutputInfo *info = NULL;
        Ecore_X_Render_Subpixel_Order ret = 0;

        /* try to get output info */
        if ((info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
          {
             ret = info->subpixel_order;

             /* free the output info */
             XRRFreeOutputInfo(info);
          }

        /* free the resources */
        XRRFreeScreenResources(res);

        return ret;
     }
#endif
   return ECORE_X_RENDER_SUBPIXEL_ORDER_UNKNOWN;
}

/***************************************
 * API Functions for RandR version 1.3 *
 ***************************************/

EAPI Ecore_X_Randr_Output *
ecore_x_randr_output_wired_clones_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output, int *num)
{
#ifdef ECORE_XRANDR
   Atom clones = None, type = None;
   unsigned char *prop;
   int format = 0;
   unsigned long nitems = 0, bytes = 0;

   /* NB: FIXME: 
    * 
    * I am not sure that this code is correct. This needs checking 
    */

   if (_randr_version < RANDR_VERSION_1_3) return NULL;

   /* try to get the edid atom */
   if (!(clones = XInternAtom(_ecore_x_disp, RR_PROPERTY_CLONE_LIST, True)))
     return NULL;

   /* get the output property
    * 
    * NB: Returns 0 on success */
   if (!XRRGetOutputProperty(_ecore_x_disp, output, clones, 0, 100, False, False, 
                            AnyPropertyType, &type, &format, &nitems, 
                            &bytes, &prop))
     {
        if ((type == XA_ATOM) && (nitems >= 1) && (format == 32))
          {
             Ecore_X_Randr_Output *ret = NULL;

             if ((ret = malloc(nitems * sizeof(Ecore_X_Randr_Output))))
               {
                  if (num) *num = nitems;
                  memcpy(ret, prop, (nitems * sizeof(Ecore_X_Randr_Output)));
                  return ret;
               }
          }
     }
#endif
   return NULL;
}

EAPI Ecore_X_Randr_Output **
ecore_x_randr_output_compatibility_list_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output EINA_UNUSED, int *num EINA_UNUSED)
{
   /* TODO: (1.3) !! */
   //RR_PROPERTY_COMPATIBILITY_LIST
   return NULL;
}

EAPI Ecore_X_Randr_Signal_Format *
ecore_x_randr_output_signal_formats_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output, int *num)
{
#ifdef ECORE_XRANDR
   XRRPropertyInfo *info = NULL;
   Atom sig, type;
   unsigned long bytes = 0;
   unsigned long items = 0;
   unsigned char *prop = NULL;
   int format = 0;

   if (_randr_version < RANDR_VERSION_1_3) return NULL;

   /* try to get the connector number atom */
   if (!(sig = XInternAtom(_ecore_x_disp, RR_PROPERTY_SIGNAL_FORMAT, True)))
     return NULL;

   /* try to get the output property from Xrandr
    * 
    * NB: Returns 0 on success */
   if (XRRGetOutputProperty(_ecore_x_disp, output, sig, 0, 100, 
                            False, False, AnyPropertyType, &type, &format, 
                            &items, &bytes, &prop))
     {
        printf("Signal Format property not supported.\n");
        return NULL;
     }

   free(prop);

   /* safety check */
   if ((type != XA_ATOM) || (items < 1) || (format != 32))
     return NULL;

   /* try to get the output property from Xrandr */
   if ((info = XRRQueryOutputProperty(_ecore_x_disp, output, sig)))
     {
        Ecore_X_Randr_Signal_Format *formats = NULL;

        if (num) *num = info->num_values;

        if (info->num_values > 0)
          {
             if ((formats = 
                  malloc(info->num_values * sizeof(Ecore_X_Randr_Signal_Format))))
               {
                  memcpy(formats, info->values, 
                         (info->num_values * sizeof(Ecore_X_Randr_Signal_Format)));
               }
          }

        /* free the info */
        free(info);

        return formats;
     }
#endif
   return NULL;
}

EAPI Eina_Bool 
ecore_x_randr_output_signal_format_set(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output EINA_UNUSED, Ecore_X_Randr_Signal_Format *sig EINA_UNUSED)
{
   /* TODO: (1.3) !! */
   //RR_PROPERTY_SIGNAL_FORMAT
   return EINA_FALSE;
}

EAPI Ecore_X_Randr_Signal_Property *
ecore_x_randr_output_signal_properties_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output, int *num)
{
#ifdef ECORE_XRANDR
   XRRPropertyInfo *info = NULL;
   Atom sig, type;
   unsigned long bytes = 0;
   unsigned long items = 0;
   unsigned char *prop = NULL;
   int format = 0;

   if (_randr_version < RANDR_VERSION_1_3) return NULL;

   /* try to get the connector number atom */
   if (!(sig = XInternAtom(_ecore_x_disp, RR_PROPERTY_SIGNAL_PROPERTIES, True)))
     return NULL;

   /* try to get the output property from Xrandr
    * 
    * NB: Returns 0 on success */
   if (XRRGetOutputProperty(_ecore_x_disp, output, sig, 0, 100, 
                            False, False, AnyPropertyType, &type, &format, 
                            &items, &bytes, &prop))
     {
        printf("Signal Properties property not supported.\n");
        return NULL;
     }

   free(prop);

   /* safety check */
   if ((type != XA_ATOM) || (items < 1) || (format != 32))
     return NULL;

   /* try to get the output property from Xrandr */
   if ((info = XRRQueryOutputProperty(_ecore_x_disp, output, sig)))
     {
        Ecore_X_Randr_Signal_Property *props = NULL;

        if (num) *num = info->num_values;

        if (info->num_values > 0)
          {
             if ((props = 
                  malloc(info->num_values * sizeof(Ecore_X_Randr_Signal_Property))))
               {
                  memcpy(props, info->values, 
                         (info->num_values * sizeof(Ecore_X_Randr_Signal_Property)));
               }
          }

        /* free the info */
        free(info);

        return props;
     }
#endif
   return NULL;
}

/* TODO NB:
 * 
 * Document this.
 * 
 * Returns: 
 * 0 == unknown
 * 1 == primary
 * 2 == secondary
 * 3 == (typically) TV Connector but is driver/hardware dependent
 * 
 */
EAPI int 
ecore_x_randr_output_connector_number_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   XRRPropertyInfo *info = NULL;
   Atom type;
   unsigned long bytes = 0;
   unsigned long items = 0;
   unsigned char *prop = NULL;
   int val = 0, format = 0;

   if (_randr_version < RANDR_VERSION_1_3) return -1;

   /* try to get the output property from Xrandr
    * 
    * NB: Returns 0 on success */
   if (XRRGetOutputProperty(_ecore_x_disp, output, connector_number, 0, 100, 
                            False, False, AnyPropertyType, &type, &format, 
                            &items, &bytes, &prop))
     {
        printf("ConnectionNumber property not supported.\n");
        return -1;
     }

   /* safety check */
   if ((type != XA_INTEGER) || (items != 1) || (format != 32))
     return -1;

   val = *((int *)prop);
   free(prop);

   /* try to get the output property from Xrandr */
   if ((info = XRRQueryOutputProperty(_ecore_x_disp, output, connector_number)))
     {
        int ret = 0;

        /* convert the current value */
        ret = (int)(val - info->values[0]);

        /* free the info */
        free(info);

        return ret;
     }
#endif
   return -1;
}

EAPI Ecore_X_Randr_Connector_Type 
ecore_x_randr_output_connector_type_get(Ecore_X_Window root EINA_UNUSED, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   XRRPropertyInfo *info = NULL;
   Atom type;
   unsigned long bytes = 0;
   unsigned long items = 0;
   unsigned char *prop = NULL;
   int val = 0, format = 0;

   if (_randr_version < RANDR_VERSION_1_3) return -1;

   /* try to get the connector type atom */
   if (XRRGetOutputProperty(_ecore_x_disp, output, connector_type, 0, 100, 
                        False, False, AnyPropertyType, &type, &format, 
                        &items, &bytes, &prop) != Success)
     return -1;

   if ((!prop) || (items == 0))
     {
        Atom conn;

        /* NB: some butthead drivers (*cough* nouveau *cough*) do not 
         * implement randr properly. They are not using the connector type 
         * property of randr, but rather a "subconnector" property */
        if ((conn = XInternAtom(_ecore_x_disp, "subconnector", True)))
          XRRGetOutputProperty(_ecore_x_disp, output, conn, 0, 4, 
                               False, False, AnyPropertyType, &type, 
                               &format, &items, &bytes, &prop);
     }

   if ((!prop) || (items == 0))
     {
        WRN("ConnectorType Property not supported.");
        return -1;
     }

   /* safety check */
   if ((type != XA_ATOM) || (items != 1) || (format != 32))
     return -1;

   val = *((int *)prop);
   free(prop);

   /* try to get the output property from Xrandr */
   if ((info = XRRQueryOutputProperty(_ecore_x_disp, output, connector_type)))
     {
        int ret = 0;

        /* convert the current value */
        ret = (int)(val - info->values[0]);

        /* printf("\tReturn Value: %d\n", ret); */
        /* printf("\t\tActual Name: %s\n",  */
        /*        XGetAtomName(_ecore_x_disp, ((Atom)info->values[ret]))); */

        /* free the info */
        free(info);

        return ret;
     }
#endif
   return -1;
}

EAPI Ecore_X_Randr_Output 
ecore_x_randr_primary_output_get(Ecore_X_Window root)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_3) return 0;
   return XRRGetOutputPrimary(_ecore_x_disp, root);
#else
   return 0;
#endif
}

EAPI void 
ecore_x_randr_primary_output_set(Ecore_X_Window root, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   if (_randr_version < RANDR_VERSION_1_3) return;
   XRRSetOutputPrimary(_ecore_x_disp, root, output);
#endif
}

/***************************************
 * API Functions for RandR version 1.4 *
 ***************************************/

EAPI void 
ecore_x_randr_crtc_panning_area_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_4) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRPanning *xpan = NULL;

        /* get this crtc's panning */
        if ((xpan = XRRGetPanning(_ecore_x_disp, res, crtc)))
          {
             if (x) *x = xpan->left;
             if (y) *y = xpan->top;
             if (w) *w = xpan->width;
             if (h) *h = xpan->height;

             /* free the panning resource */
             XRRFreePanning(xpan);
          }
        /* free the resources */
        XRRFreeScreenResources(res);
     }
#endif
}

EAPI Eina_Bool 
ecore_x_randr_crtc_panning_area_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const int x, const int y, const int w, const int h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;
   Eina_Bool ret = EINA_FALSE;

   if (_randr_version < RANDR_VERSION_1_4) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRPanning *xpan = NULL;

        /* get this crtc's panning */
        if ((xpan = XRRGetPanning(_ecore_x_disp, res, crtc)))
          {
             xpan->left = x;
             xpan->top = y;
             xpan->width = w;
             xpan->height = h;
             xpan->timestamp = CurrentTime;

             /* set the panning value */
             if (!XRRSetPanning(_ecore_x_disp, res, crtc, xpan))
               ret = EINA_TRUE;

             /* free the panning resource */
             XRRFreePanning(xpan);
          }

        /* free the resources */
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

EAPI void 
ecore_x_randr_crtc_tracking_area_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_4) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRPanning *xpan = NULL;

        /* get this crtc's panning */
        if ((xpan = XRRGetPanning(_ecore_x_disp, res, crtc)))
          {
             if (x) *x = xpan->track_left;
             if (y) *y = xpan->track_top;
             if (w) *w = xpan->track_width;
             if (h) *h = xpan->track_height;

             /* free the panning resource */
             XRRFreePanning(xpan);
          }
        /* free the resources */
        XRRFreeScreenResources(res);
     }
#endif
}

EAPI Eina_Bool 
ecore_x_randr_crtc_tracking_area_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const int x, const int y, const int w, const int h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;
   Eina_Bool ret = EINA_FALSE;

   if (_randr_version < RANDR_VERSION_1_4) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRPanning *xpan = NULL;

        /* get this crtc's panning */
        if ((xpan = XRRGetPanning(_ecore_x_disp, res, crtc)))
          {
             xpan->track_left = x;
             xpan->track_top = y;
             xpan->track_width = w;
             xpan->track_height = h;
             xpan->timestamp = CurrentTime;

             /* set the panning value */
             if (!XRRSetPanning(_ecore_x_disp, res, crtc, xpan))
               ret = EINA_TRUE;

             /* free the panning resource */
             XRRFreePanning(xpan);
          }

        /* free the resources */
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

EAPI void 
ecore_x_randr_crtc_border_area_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, int *x, int *y, int *w, int *h)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;

   if (_randr_version < RANDR_VERSION_1_4) return;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRPanning *xpan = NULL;

        /* get this crtc's panning */
        if ((xpan = XRRGetPanning(_ecore_x_disp, res, crtc)))
          {
             if (x) *x = xpan->border_left;
             if (y) *y = xpan->border_top;
             if (w) *w = xpan->border_right;
             if (h) *h = xpan->border_bottom;

             /* free the panning resource */
             XRRFreePanning(xpan);
          }
        /* free the resources */
        XRRFreeScreenResources(res);
     }
#endif
}

EAPI Eina_Bool 
ecore_x_randr_crtc_border_area_set(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc, const int left, const int top, const int right, const int bottom)
{
#ifdef ECORE_XRANDR
   XRRScreenResources *res = NULL;
   Eina_Bool ret = EINA_FALSE;

   if (_randr_version < RANDR_VERSION_1_4) return EINA_FALSE;

   /* try to get the screen resources from Xrandr */
   if ((res = _ecore_x_randr_screen_resources_get(_ecore_x_disp, root)))
     {
        XRRPanning *xpan = NULL;

        /* get this crtc's panning */
        if ((xpan = XRRGetPanning(_ecore_x_disp, res, crtc)))
          {
             xpan->border_left = left;
             xpan->border_top = top;
             xpan->border_right = right;
             xpan->border_bottom = bottom;
             xpan->timestamp = CurrentTime;

             /* set the panning value */
             if (!XRRSetPanning(_ecore_x_disp, res, crtc, xpan))
               ret = EINA_TRUE;

             /* free the panning resource */
             XRRFreePanning(xpan);
          }

        /* free the resources */
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

/***************************************
 * API Functions for RandR Edid
 ***************************************/

EAPI Eina_Bool 
ecore_x_randr_edid_has_valid_header(unsigned char *edid, unsigned long edid_length)
{
   const unsigned char header[] =
     { 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 };

   if (!edid) return EINA_FALSE;
   if (edid_length < 8) return EINA_FALSE;
   if (!memcmp(edid, header, 8)) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_randr_edid_info_has_valid_checksum(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   unsigned char *iter = NULL;
   char sum = 0;
   int i = 0, version = 0;

   if (edid_length < 128) return EINA_FALSE;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return EINA_FALSE;

   for (i = 0; i < 128; i++)
     sum += edid[i];

   if (sum) return EINA_FALSE;

   /* check extension blocks */
   for (iter = edid; iter < (edid + edid_length); iter += 128)
     {
        if (iter[0] == 0x02)
          {
             for (i = 0, sum = 0; i < 128; i++)
               sum += iter[i];
          }
     }

   if (sum) return EINA_FALSE;
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI int 
ecore_x_randr_edid_version_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   if ((edid_length > RANDR_EDID_VERSION_MINOR) && 
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     {
        return (edid[RANDR_EDID_VERSION_MAJOR] << 8) | 
          edid[RANDR_EDID_VERSION_MINOR];
     }

   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
#else
   return 0;
#endif
}

EAPI char *
ecore_x_randr_edid_manufacturer_name_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   if ((edid_length > RANDR_EDID_MANUFACTURER + 1) && 
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     {
        unsigned char *x;
        char *name;

        if (!(name = malloc(sizeof(char) * 4))) return NULL;

        x = (edid + RANDR_EDID_MANUFACTURER);
        name[0] = ((x[0] & 0x7c) >> 2) + '@';
        name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xe0) >> 5) + '@';
        name[2] = (x[1] & 0x1f) + '@';
        name[3] = 0;

        return name;
     }
#endif
   return NULL;
}

EAPI char *
ecore_x_randr_edid_display_name_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   unsigned char *block = NULL;
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return NULL;

   for (block = (edid + RANDR_EDID_BLOCK); 
        block <= (edid + RANDR_EDID_BLOCK + (3 * 18)); block += 18)
     {
        if ((block[0] == 0) && (block[1] == 0))
          {
             if (block[3] == 0xfc)
               {
                  char *name, *p;
                  const char *edid_name;

                  edid_name = (const char *)block + 5;
                  if (!(name = malloc(14))) return NULL;
                  strncpy(name, edid_name, 13);
                  name[13] = 0;

                  for (p = name; *p; p++)
                    if ((*p < ' ') || (*p > '~')) *p = 0;

                  return name;
               }
          }
     }
#endif
   return NULL;
}

EAPI char *
ecore_x_randr_edid_display_ascii_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   unsigned char *block = NULL;
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return NULL;

   for (block = (edid + RANDR_EDID_BLOCK); 
        block <= (edid + RANDR_EDID_BLOCK + (3 * 18)); block += 18)
     {
        if ((block[0] == 0) && (block[1] == 0))
          {
             if (block[3] == 0xfe)
               {
                  char *ascii = NULL, *p = NULL;
                  const char *edid_ascii;

                  edid_ascii = (const char *)block + 5;

                  if (!(ascii = malloc(14))) return NULL;
                  strncpy(ascii, edid_ascii, 13);
                  ascii[13] = 0;
                  for (p = ascii; *p; p++)
                    if ((*p < ' ') || (*p > '~')) *p = 0;

                  return ascii;
               }
          }
     }
#endif
   return NULL;
}

EAPI char *
ecore_x_randr_edid_display_serial_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   unsigned char *block = NULL;
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return NULL;

   for (block = (edid + RANDR_EDID_BLOCK); 
        block <= (edid + RANDR_EDID_BLOCK + (3 * 18)); block += 18)
     {
        if ((block[0] == 0) && (block[1] == 0))
          {
             if (block[3] == 0xff)
               {
                  char *serial = NULL, *p = NULL;
                  const char *edid_serial;

                  edid_serial = (const char *)block + 5;

                  if (!(serial = malloc(14))) return NULL;
                  strncpy(serial, edid_serial, 13);
                  serial[13] = 0;
                  for (p = serial; *p; p++)
                    if ((*p < ' ') || (*p > '~')) *p = 0;

                  return serial;
               }
          }
     }
#endif
   return NULL;
}

EAPI int 
ecore_x_randr_edid_model_get(unsigned char *edid, unsigned long edid_length)
{
   return ecore_x_randr_edid_manufacturer_model_get(edid, edid_length);
}

EAPI int 
ecore_x_randr_edid_manufacturer_serial_number_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   if ((edid_length > RANDR_EDID_MANUFACTURER + 1) && 
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     return (int)(edid[0x0c] + (edid[0x0d] << 8) + 
                  (edid[0x0e] << 16) + (edid[0x0f] << 24));

   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
#else
   return 0;
#endif
}

EAPI int 
ecore_x_randr_edid_manufacturer_model_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   if ((edid_length > RANDR_EDID_MANUFACTURER + 1) && 
       (ecore_x_randr_edid_has_valid_header(edid, edid_length)))
     return (int)(edid[0x0a] + (edid[0x0b] << 8));
#endif
   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
}

EAPI Eina_Bool 
ecore_x_randr_edid_dpms_available_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return EINA_FALSE;

   return !!(edid[0x18] & 0xE0);
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool 
ecore_x_randr_edid_dpms_standby_available_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return EINA_FALSE;

   if (edid[0x18] & 0xE0) return !!(edid[0x18] & 0x80);
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_randr_edid_dpms_suspend_available_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return EINA_FALSE;

   if (edid[0x18] & 0xE0) return !!(edid[0x18] & 0x40);
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool 
ecore_x_randr_edid_dpms_off_available_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return EINA_FALSE;

   if (edid[0x18] & 0xE0) return !!(edid[0x18] & 0x20);
#endif
   return EINA_FALSE;
}

EAPI Ecore_X_Randr_Edid_Aspect_Ratio 
ecore_x_randr_edid_display_aspect_ratio_preferred_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   unsigned char *block = NULL;
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) 
     return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   for (block = (edid + RANDR_EDID_BLOCK); 
        block <= (edid + RANDR_EDID_BLOCK + (3 * 18)); block += 18)
     {
        if ((block[0] == 0) && (block[1] == 0))
          {
             if ((block[3] == 0xfd) && (block[10] == 0x04))
               {
                  Ecore_X_Randr_Edid_Aspect_Ratio_Preferred ratio = 
                    (Ecore_X_Randr_Edid_Aspect_Ratio_Preferred)((block[15] & 0xe0) >> 5);

                  switch (ratio)
                    {
                     case RANDR_EDID_ASPECT_RATIO_PREFERRED_4_3:
                       return ECORE_X_RANDR_EDID_ASPECT_RATIO_4_3;
                     case RANDR_EDID_ASPECT_RATIO_PREFERRED_16_9:
                       return ECORE_X_RANDR_EDID_ASPECT_RATIO_16_9;
                     case RANDR_EDID_ASPECT_RATIO_PREFERRED_16_10:
                       return ECORE_X_RANDR_EDID_ASPECT_RATIO_16_10;
                     case RANDR_EDID_ASPECT_RATIO_PREFERRED_5_4:
                       return ECORE_X_RANDR_EDID_ASPECT_RATIO_5_4;
                     case RANDR_EDID_ASPECT_RATIO_PREFERRED_15_9:
                       return ECORE_X_RANDR_EDID_ASPECT_RATIO_15_9;
                     default:
                       return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
                    }
               }
          }
     }

   return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;
#else
   return 0;
#endif
}

EAPI Ecore_X_Randr_Edid_Aspect_Ratio 
ecore_x_randr_edid_display_aspect_ratios_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Edid_Aspect_Ratio ret;
   unsigned char *block = NULL;
   int version = 0;

   ret = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) 
     return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   for (block = (edid + RANDR_EDID_BLOCK); 
        block <= (edid + RANDR_EDID_BLOCK + (3 * 18)); block += 18)
     {
        if ((block[0] == 0) && (block[1] == 0))
          {
             if ((block[3] == 0xfd) && (block[10] == 0x04))
               {
                  if (block[14] & 0x80)
                    ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_4_3;
                  if (block[14] & 0x40)
                    ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_16_9;
                  if (block[14] & 0x20)
                    ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_16_10;
                  if (block[14] & 0x10)
                    ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_5_4;
                  if (block[14] & 0x08)
                    ret |= ECORE_X_RANDR_EDID_ASPECT_RATIO_15_9;
               }
          }
     }

   return ret;
#else
   return 0;
#endif
}

EAPI Ecore_X_Randr_Edid_Display_Colorscheme 
ecore_x_randr_edid_display_colorscheme_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Edid_Display_Colorscheme ret;
   int version = 0;

   ret = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) 
     return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   if (ecore_x_randr_edid_display_type_digital_get(edid, edid_length))
     {
        ret = ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_4_4_4;
        if (edid[0x18] & 0x10)
          ret |= ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_YCRCB_4_4_4;
        if (edid[0x18] & 0x08)
          ret |= ECORE_X_RANDR_EDID_DISPLAY_COLORSCHEME_COLOR_RGB_YCRCB_4_2_2;
     }
   else
     ret = (edid[0x18] & 0x18);

   return ret;
#else
   return 0;
#endif
}

EAPI Eina_Bool 
ecore_x_randr_edid_display_type_digital_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   int version = 0;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) return EINA_FALSE;

   return !!(edid[0x14] & 0x80);
#else
   return EINA_FALSE;
#endif
}

EAPI Ecore_X_Randr_Edid_Display_Interface_Type 
ecore_x_randr_edid_display_interface_type_get(unsigned char *edid, unsigned long edid_length)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Edid_Display_Interface_Type type;
   int version = 0;

   type = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   version = ecore_x_randr_edid_version_get(edid, edid_length);
   if (version < RANDR_EDID_VERSION_1_3) 
     return ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   type = (edid[0x14] & 0x0f);
   if (type > ECORE_X_RANDR_EDID_DISPLAY_INTERFACE_DISPLAY_PORT)
     type = ECORE_X_RANDR_EDID_UNKNOWN_VALUE;

   return type;
#else
   return 0;
#endif
}
