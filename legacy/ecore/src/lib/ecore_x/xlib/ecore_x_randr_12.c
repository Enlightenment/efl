/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_x_private.h"
#include "ecore_x_randr.h"

#define Ecore_X_Randr_None   (Ecore_X_Randr_Crtc)0
#define Ecore_X_Randr_Unset  (Ecore_X_Randr_Crtc)-1

#ifdef ECORE_XRANDR

#define RANDR_1_2 ((1 << 16) | 2)

#define RANDR_VALIDATE_ROOT(screen, root) \
   ((screen = XRRRootToScreen(_ecore_x_disp, root)) != -1)

#define RANDR_CHECK_1_2_RET(ret) if(_randr_version < RANDR_1_2) return ret

#define RANDR_PROPERTY_EDID                     "EDID"
#define RANDR_PROPERTY_BACKLIGHT                "Backlight"
#define RANDR_PROPERTY_SIGNAL_FORMAT            "SignalFormat"
#define RANDR_PROPERTY_SIGNAL_PROPERTIES        "SignalProperties"
#define RANDR_PROPERTY_CONNECTOR_TYPE           "ConnectorType"
#define RANDR_PROPERTY_CONNECTOR_NUMBER         "ConnectorNumber"
#define RANDR_PROPERTY_COMPATIBILITY_LIST       "CompatibilityList"
#define RANDR_PROPERTY_CLONE_LIST               "CloneList"

extern XRRScreenResources * (*_ecore_x_randr_get_screen_resources)(Display *
                                                                   dpy,
                                                                   Window
                                                                   window);
extern int _randr_version;
#endif

/**
 * @brief enable event selection. This enables basic interaction with
 * output/crtc events and requires RRandR >= 1.2.
 * @param win select this window's properties for RandRR events
 * @param on enable/disable selecting
 */
EAPI void
ecore_x_randr_events_select(Ecore_X_Window win, Eina_Bool on)
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
#endif
}

/**
 * @brief validates a CRTC for a given root window's screen.
 * @param root the window which's default display will be queried
 * @param crtc the CRTC to be validated.
 * @return in case it is found EINA_TRUE will be returned. Else EINA_FALSE is returned.
 */
static inline Eina_Bool
_ecore_x_randr_crtc_validate(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   XRRScreenResources *res = NULL;
   int i;
   Eina_Bool ret = EINA_FALSE;

   if ((crtc == Ecore_X_Randr_None) ||
       (crtc == Ecore_X_Randr_Unset))
      return ret;

   if (_ecore_x_randr_root_validate(root) && crtc &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        for (i = 0; i < res->ncrtc; i++)
          {
             if (res->crtcs[i] == crtc)
               {
                  ret = EINA_TRUE;
                  break;
               }
          }
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

Eina_Bool
_ecore_x_randr_output_validate(Ecore_X_Window root, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   XRRScreenResources *res = NULL;
   int i;

   if (_ecore_x_randr_root_validate(root) && output &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        for (i = 0; i < res->noutput; i++)
          {
             if (res->outputs[i] == output)
               {
                  ret = EINA_TRUE;
                  break;
               }
          }
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

static inline Eina_Bool
_ecore_x_randr_mode_validate(Ecore_X_Window root, Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   Eina_Bool ret = EINA_FALSE;
   XRRScreenResources *res = NULL;
   int i;

   if (_ecore_x_randr_root_validate(root) && mode &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        for (i = 0; i < res->nmode; i++)
          {
             if (res->modes[i].id == mode)
               {
                  ret = EINA_TRUE;
                  break;
               }
          }
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

/*
 * @param w width of screen in px
 * @param h height of screen in px
 */
EAPI void
ecore_x_randr_screen_current_size_get(Ecore_X_Window root,
                                      int *w,
                                      int *h,
                                      int *w_mm,
                                      int *h_mm)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   Ecore_X_Randr_Screen scr;

   if (!RANDR_VALIDATE_ROOT(scr, root))
      return;

   if (w)
      *w = DisplayWidth(_ecore_x_disp, scr);

   if (h)
      *h = DisplayHeight(_ecore_x_disp, scr);

   if (w_mm)
      *w_mm = DisplayWidthMM(_ecore_x_disp, scr);

   if (h_mm)
      *h_mm = DisplayHeightMM(_ecore_x_disp, scr);

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
ecore_x_randr_screen_size_range_get(Ecore_X_Window root,
                                    int *wmin,
                                    int *hmin,
                                    int *wmax,
                                    int *hmax)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   int twmin, thmin, twmax, thmax;
   if (XRRGetScreenSizeRange (_ecore_x_disp, root, &twmin, &thmin, &twmax,
                              &thmax))
     {
        if (wmin)
           *wmin = twmin;

        if (hmin)
           *hmin = thmin;

        if (wmax)
           *wmax = twmax;

        if (hmax)
           *hmax = thmax;
     }

#endif
}

/*
 * @param root window which's screen's size should be set. If invalid (e.g. NULL) no action is taken.
 * @param w width in px the screen should be set to. If out of valid boundaries, current value is assumed.
 * @param h height in px the screen should be set to. If out of valid boundaries, current value is assumed.
 * @param w_mm width in mm the screen should be set to. If 0, current aspect is assumed.
 * @param h_mm height in mm the screen should be set to. If 0, current aspect is assumed.
 * @return EINA_TRUE if request was successfully sent or screen is already in
 * requested size, EINA_FALSE if parameters are invalid
 */
EAPI Eina_Bool
ecore_x_randr_screen_current_size_set(Ecore_X_Window root,
                                      int w,
                                      int h,
                                      int w_mm,
                                      int h_mm)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   Ecore_X_Randr_Screen scr;
   int w_c, h_c, w_mm_c, h_mm_c, twmin, thmin, twmax, thmax;

   if (!RANDR_VALIDATE_ROOT(scr, root))
      return EINA_FALSE;

   ecore_x_randr_screen_current_size_get(root, &w_c, &h_c, &w_mm_c, &h_mm_c);
   if ((w == w_c) && (h == h_c) && (w_mm_c == w_mm) && (h_mm_c == h_mm))
      return EINA_TRUE;

   ecore_x_randr_screen_size_range_get(root, &twmin, &thmin, &twmax, &thmax);

   if (((w != Ecore_X_Randr_None) &&
        ((w < twmin) ||
         (w > twmax))) ||
       ((h != Ecore_X_Randr_None) && ((h < thmin) || (h > thmax))))
      return EINA_FALSE;

   if (w <= 0)
      w = DisplayWidth(_ecore_x_disp, scr);

   if (h <= 0)
      h = DisplayHeight(_ecore_x_disp, scr);

   if(w_mm <= 0)
      w_mm =
         (int)(((double)(DisplayWidthMM(_ecore_x_disp,
                                        scr) /
                         (double)DisplayWidth(_ecore_x_disp,
                                              scr))) * (double)w);

   if(h_mm <= 0)
      h_mm =
         (int)(((double)(DisplayHeightMM(_ecore_x_disp,
                                         scr) /
                         (double)DisplayHeight(_ecore_x_disp,
                                               scr))) * (double)h);

   XRRSetScreenSize (_ecore_x_disp, root, w, h, w_mm, h_mm);
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

/*
 * @brief get detailed information for all modes related to a root window's screen
 * @param root window which's screen's ressources are queried
 * @param num number of modes returned
 * @return modes' information
 */
EAPI Ecore_X_Randr_Mode_Info **
ecore_x_randr_modes_info_get(Ecore_X_Window root, int *num)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Mode_Info **ret = NULL;
   int i;

   if (_ecore_x_randr_root_validate(root) &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        if ((ret =
                (Ecore_X_Randr_Mode_Info **)malloc(sizeof(
                                                      Ecore_X_Randr_Mode_Info *)
                                                   *
                                                   res->nmode)))
          {
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
                       if ((ret[i]->name = (malloc(res->modes[i].nameLength))))
                          strncpy(ret[i]->name, res->modes[i].name,
                                  res->modes[i].nameLength);
                       else
                          ret[i]->name = NULL;

                       ret[i]->nameLength = res->modes[i].nameLength;
                       ret[i]->modeFlags = res->modes[i].modeFlags;
                    }
                  else
                    {
                       while(i > 0)
                          free(ret[--i]);
                          free(ret);
                       ret = NULL;
                       break;
                    }
               }
          }

        if (ret && num)
           *num = res->nmode;

        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return NULL;
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
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Mode_Info *ret = NULL;
   int i;

   if (_ecore_x_randr_root_validate(root) &&
       (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
     {
        for (i = 0; i < res->nmode; i++)
          {
             if ((res->modes[i].id == mode) &&
                 (ret = malloc(sizeof(Ecore_X_Randr_Mode_Info))))
               {
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
                  ret->name = NULL;
                  ret->nameLength = 0;
                  if (res->modes[i].nameLength > 0)
                    {
                       ret->nameLength = res->modes[i].nameLength;
                       ret->name = malloc(res->modes[i].nameLength + 1);
                       if (ret->name)
                         memcpy(ret->name, res->modes[i].name,
                                res->modes[i].nameLength + 1);
                    }
                  ret->modeFlags = res->modes[i].modeFlags;
                  break;
               }
          }
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return NULL;
#endif
}

/*
 * @brief free detailed mode information. The pointer handed in will be set to
 * NULL after freeing the memory.
 * @param mode_info the mode information that should be freed
 */
EAPI void
ecore_x_randr_mode_info_free(Ecore_X_Randr_Mode_Info *mode_info)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   if (!mode_info)
     return;

   if (mode_info->name)
     free(mode_info->name);

   free(mode_info);
   mode_info = NULL;
#endif
}

/*
 * @brief get all known CRTCs related to a root window's screen
 * @param root window which's screen's ressources are queried
 * @param num number of CRTCs returned
 * @return CRTC IDs
 */
EAPI Ecore_X_Randr_Crtc *
ecore_x_randr_crtcs_get(Ecore_X_Window root, int *num)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Crtc *ret = NULL;

   if (num && root &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        if ((ret = malloc(sizeof(Ecore_X_Randr_Crtc) * res->ncrtc)))
          {
             memcpy(ret, res->crtcs, (sizeof(Ecore_X_Randr_Crtc) * res->ncrtc));
             *num = res->ncrtc;
          }

        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return NULL;
#endif
}

EAPI Ecore_X_Randr_Output *
ecore_x_randr_outputs_get(Ecore_X_Window root, int *num)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Output *ret = NULL;

   if (num && root &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        if ((ret = malloc(sizeof(Ecore_X_Randr_Output) * res->noutput)))
          {
             memcpy(ret, res->outputs,
                    (sizeof(Ecore_X_Randr_Output) * res->noutput));
             if (num)
                *num = res->noutput;
          }

        if (res)
           XRRFreeScreenResources(res);
     }

   return ret;
#else
   return NULL;
#endif
}

//Per Crtc
/*
 * @brief get a CRTC's outputs.
 * @param root the root window which's screen will be queried
 * @param num number of outputs referenced by given CRTC
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_crtc_outputs_get(Ecore_X_Window root,
                               Ecore_X_Randr_Crtc crtc,
                               int *num)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Output *ret = NULL;
   XRRCrtcInfo *crtc_info = NULL;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res =
           _ecore_x_randr_get_screen_resources (_ecore_x_disp,
                                                root)) &&
       (crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
     {
        if ((ret = malloc(sizeof(Ecore_X_Randr_Output) * crtc_info->noutput)))
          {
             memcpy(ret, crtc_info->outputs,
                    (sizeof(Ecore_X_Randr_Output) * crtc_info->noutput));
             if (num)
                *num = crtc_info->noutput;
          }

        if (crtc_info)
           XRRFreeCrtcInfo(crtc_info);

        if (res)
           XRRFreeScreenResources(res);
     }

   return ret;
#else
   return NULL;
#endif
}

/*
 * @brief get a CRTC's possible outputs.
 * @param root the root window which's screen will be queried
 * @param num number of possible outputs referenced by given CRTC
 */
EAPI Ecore_X_Randr_Output *
ecore_x_randr_crtc_possible_outputs_get(Ecore_X_Window root,
                                        Ecore_X_Randr_Crtc crtc,
                                        int *num)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Output *ret = NULL;
   XRRCrtcInfo *crtc_info = NULL;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        if((crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             if ((ret =
                     malloc(sizeof(Ecore_X_Randr_Output) * crtc_info->npossible)))
               {
                  memcpy(ret, crtc_info->possible,
                         (sizeof(Ecore_X_Randr_Output) * crtc_info->npossible));
                  if (num)
                     *num = res->ncrtc;
               }

             XRRFreeCrtcInfo(crtc_info);
          }

        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return NULL;
#endif
}

EAPI void
ecore_x_randr_crtc_geometry_get(Ecore_X_Window root,
                                Ecore_X_Randr_Crtc crtc,
                                int *x,
                                int *y,
                                int *w,
                                int *h)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   XRRScreenResources *res = NULL;
   XRRCrtcInfo *crtc_info = NULL;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res =
           _ecore_x_randr_get_screen_resources (_ecore_x_disp,
                                                root)) &&
       (crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
     {
        if (x)
           *x = crtc_info->x;

        if (y)
           *y = crtc_info->y;

        if (w)
           *w = crtc_info->width;

        if (h)
           *h = crtc_info->height;

        XRRFreeCrtcInfo(crtc_info);
        XRRFreeScreenResources(res);
     }

#endif
}

/*
 * @brief sets the position of given CRTC within root window's screen
 * @param root the window's screen to be queried
 * @param crtc the CRTC which's position within the mentioned screen is to be altered
 * @param x position on the x-axis (0 == left) of the screen. if x < 0 current value will be kept.
 * @param y position on the y-ayis (0 == top) of the screen. if y < 0, current value will be kept.
 * @return EINA_TRUE if position could be successfully be altered.
 */
EAPI Eina_Bool
ecore_x_randr_crtc_pos_set(Ecore_X_Window root,
                           Ecore_X_Randr_Crtc crtc,
                           int x,
                           int y)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);
   int w_c, h_c, w_new = 0, h_new = 0;
   Eina_Rectangle crtc_geo;

   ecore_x_randr_crtc_geometry_get(root,
                                   crtc,
                                   &crtc_geo.x,
                                   &crtc_geo.y,
                                   &crtc_geo.w,
                                   &crtc_geo.h);
   ecore_x_randr_screen_current_size_get(root, &w_c, &h_c, NULL, NULL);
   if (x < 0)
      x = crtc_geo.x;

   if (y < 0)
      y = crtc_geo.y;

   if ((x + crtc_geo.w) > w_c)
      w_new = x + crtc_geo.w;

   if ((y + crtc_geo.h) > h_c)
      h_new = y + crtc_geo.h;

   if ((w_new != 0) || (h_new != 0))
      if (!ecore_x_randr_screen_current_size_set(root, w_new, h_new, 0, 0))
         return EINA_FALSE;

   return ecore_x_randr_crtc_settings_set(root,
                                          crtc,
                                          NULL,
                                          Ecore_X_Randr_Unset,
                                          x,
                                          y,
                                          Ecore_X_Randr_Unset,
                                          Ecore_X_Randr_Unset);
#else
   return EINA_FALSE;
#endif
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
   RANDR_CHECK_1_2_RET(Ecore_X_Randr_Unset);
   XRRScreenResources *res = NULL;
   XRRCrtcInfo *crtc_info = NULL;
   Ecore_X_Randr_Mode ret = Ecore_X_Randr_Unset;
   if (_ecore_x_randr_root_validate(root) &&
       _ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res =
           _ecore_x_randr_get_screen_resources(_ecore_x_disp,
                                               root)) &&
       (crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
     {
        ret = crtc_info->mode;
        XRRFreeCrtcInfo(crtc_info);
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return Ecore_X_Randr_Unset;
#endif
}

/**
 * @brief sets a mode for a CRTC and the outputs attached to it
 * @param root the window's screen to be queried
 * @param crtc the CRTC which shall be set
 * @param outputs array of outputs which have to be compatible with the mode. If
 * NULL CRTC will be disabled.
 * @param noutputs number of outputs in array to be used. Use
 * Ecore_X_Randr_Unset (or -1) to use currently used outputs.
 * @param mode XID of the mode to be set. If set to 0 the CRTC will be disabled.
 * If set to -1 the call will fail.
 * @return EINA_TRUE if mode setting was successful. Else EINA_FALSE
 */
EAPI Eina_Bool
ecore_x_randr_crtc_mode_set(Ecore_X_Window root,
                            Ecore_X_Randr_Crtc crtc,
                            Ecore_X_Randr_Output *outputs,
                            int noutputs,
                            Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   if (mode == Ecore_X_Randr_Unset)
      return EINA_FALSE;

   return ecore_x_randr_crtc_settings_set(root,
                                          crtc,
                                          outputs,
                                          noutputs,
                                          Ecore_X_Randr_Unset,
                                          Ecore_X_Randr_Unset,
                                          mode,
                                          Ecore_X_Randr_Unset);
#else
   return EINA_FALSE;
#endif
}

EAPI void
ecore_x_randr_crtc_size_get(Ecore_X_Window root,
                            Ecore_X_Randr_Crtc crtc,
                            int *w,
                            int *h)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   ecore_x_randr_crtc_geometry_get(root, crtc, NULL, NULL, w, h);
#endif
}

EAPI Ecore_X_Randr_Refresh_Rate
ecore_x_randr_crtc_refresh_rate_get(Ecore_X_Window root,
                                    Ecore_X_Randr_Crtc crtc,
                                    Ecore_X_Randr_Mode mode)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(0.0);
   XRRScreenResources *res = NULL;
   XRRCrtcInfo *crtc_info = NULL;
   Ecore_X_Randr_Refresh_Rate ret = 0.0;
   int i;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        for (i = 0; i < res->nmode; i++)
           if (res->modes[i].id == mode)
             {
                if (res->modes[i].hTotal && res->modes[i].vTotal)
                   ret = ((double)res->modes[i].dotClock /
                          ((double)res->modes[i].hTotal *
                           (double)res->modes[i].vTotal));

                break;
             }

     }

   if (crtc_info)
      XRRFreeCrtcInfo(crtc_info);

   if (res)
      XRRFreeScreenResources(res);

   return ret;
#else
   return 0.0;
#endif
}

EAPI Ecore_X_Randr_Orientation
ecore_x_randr_crtc_orientations_get(Ecore_X_Window root,
                                    Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(Ecore_X_Randr_None);
   XRRCrtcInfo *crtc_info = NULL;
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Orientation ret = Ecore_X_Randr_None;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res =
           _ecore_x_randr_get_screen_resources (_ecore_x_disp,
                                                root)) &&
       (crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
     {
        ret = crtc_info->rotations;
     }
   if (crtc_info)
      XRRFreeCrtcInfo(crtc_info);

   if (res)
      XRRFreeScreenResources(res);

   return ret;
#else
   return Ecore_X_Randr_None;
#endif
}

EAPI Ecore_X_Randr_Orientation
ecore_x_randr_crtc_orientation_get(Ecore_X_Window root, Ecore_X_Randr_Crtc crtc)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(Ecore_X_Randr_None);
   XRRCrtcInfo *crtc_info = NULL;
   XRRScreenResources *res = NULL;
   Ecore_X_Randr_Orientation ret = Ecore_X_Randr_None;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res =
           _ecore_x_randr_get_screen_resources (_ecore_x_disp,
                                                root)) &&
       (crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
     {
        ret = crtc_info->rotation;
     }
   if (crtc_info)
      XRRFreeCrtcInfo(crtc_info);

   if (res)
      XRRFreeScreenResources(res);

   return ret;
#else
   return Ecore_X_Randr_None;
#endif
}

EAPI Eina_Bool
ecore_x_randr_crtc_orientation_set(Ecore_X_Window root,
                                   Ecore_X_Randr_Crtc crtc,
                                   Ecore_X_Randr_Orientation orientation)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);
   Eina_Bool ret = EINA_FALSE;

   if (orientation != Ecore_X_Randr_None)
     {
        ret = ecore_x_randr_crtc_settings_set(root,
                                              crtc,
                                              NULL,
                                              Ecore_X_Randr_Unset,
                                              Ecore_X_Randr_Unset,
                                              Ecore_X_Randr_Unset,
                                              Ecore_X_Randr_Unset,
                                              orientation);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

EAPI void
ecore_x_randr_crtc_pos_get(Ecore_X_Window root,
                           Ecore_X_Randr_Crtc crtc,
                           int *x,
                           int *y)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();

   ecore_x_randr_crtc_geometry_get(root, crtc, x, y, NULL, NULL);
#endif
}

EAPI Eina_Bool
ecore_x_randr_crtc_clone_set(Ecore_X_Window root,
                             Ecore_X_Randr_Crtc original,
                             Ecore_X_Randr_Crtc clone)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   XRRScreenResources *res = NULL;
   XRRCrtcInfo *clone_crtc_info = NULL;
   Ecore_X_Randr_Mode original_mode = Ecore_X_Randr_None;
   Ecore_X_Randr_Orientation original_orientation = Ecore_X_Randr_None;
   Eina_Bool ret = EINA_FALSE;
   int x, y;

   if (_ecore_x_randr_root_validate(root) &&
           _ecore_x_randr_crtc_validate(root,
                                    original) &&
           _ecore_x_randr_crtc_validate(root,
                                    clone) &&
       (res =
           _ecore_x_randr_get_screen_resources (_ecore_x_disp,
                                                root)) &&
       (clone_crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, clone)))
     {
        ecore_x_randr_crtc_geometry_get(root, original, &x, &y, NULL, NULL);
        original_mode = ecore_x_randr_crtc_mode_get(root, original);
        original_orientation = ecore_x_randr_crtc_orientation_get(root,
                                                                  original);
        ret = ecore_x_randr_crtc_settings_set(root,
                                              clone,
                                              NULL,
                                              Ecore_X_Randr_Unset,
                                              x,
                                              y,
                                              original_mode,
                                              original_orientation);
        XRRFreeCrtcInfo(clone_crtc_info);
        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

/**
 * @brief sets the demanded parameters for a given CRTC. Note that the CRTC is
 * auto enabled in it's preferred mode, when it was disabled before.
 * @param root the root window which's default display will be queried
 * @param crtc the CRTC which's configuration should be altered
 * @param outputs an array of outputs, that should display this CRTC's content.
 * @param noutputs number of outputs in the array of outputs.
 * If set to Ecore_X_Randr_Unset, current outputs and number of outputs will be used.
 * If set to Ecore_X_Randr_None, CRTC will be disabled
 * @param x new x coordinate. If <0 (e.g. Ecore_X_Randr_Unset) the current x
 * corrdinate will be assumed.
 * @param y new y coordinate. If <0 (e.g. Ecore_X_Randr_Unset) the current y
 * corrdinate will be assumed.
 * @param mode the new mode to be set. If Ecore_X_Randr_None is passed, the
 * CRTC will be disabled. If Ecore_X_Randr_Unset is passed, the current mode is assumed.
 * @param orientation the new orientation to be set. If Ecore_X_Randr_Unset is used,
 * the current mode is assumed.
 * @return EINA_TRUE if the configuration alteration was successful, else
 * EINA_FALSE
 */
EAPI Eina_Bool
ecore_x_randr_crtc_settings_set(Ecore_X_Window root,
                                Ecore_X_Randr_Crtc crtc,
                                Ecore_X_Randr_Output *outputs,
                                int noutputs,
                                int x,
                                int y,
                                Ecore_X_Randr_Mode mode,
                                Ecore_X_Randr_Orientation orientation)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);
   XRRScreenResources *res = NULL;
   XRRCrtcInfo *crtc_info = NULL;
   Eina_Bool ret = EINA_FALSE;

   if (_ecore_x_randr_crtc_validate(root,
                                    crtc) &&
       (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
     {
        if ((crtc_info = XRRGetCrtcInfo(_ecore_x_disp, res, crtc)))
          {
             if ((mode == Ecore_X_Randr_None) ||
                 (noutputs == Ecore_X_Randr_None))
               {
                  outputs = NULL;
                  noutputs = 0;
               }
             else if (noutputs == (int)Ecore_X_Randr_Unset)
               {
                  outputs = (Ecore_X_Randr_Output *)crtc_info->outputs;
                  noutputs = crtc_info->noutput;
               }

             if (mode == Ecore_X_Randr_Unset)
                mode = crtc_info->mode;

             if (x < 0)
                x = crtc_info->x;

             if (y < 0)
                y = crtc_info->y;

             if (orientation == Ecore_X_Randr_Unset)
                orientation = crtc_info->rotation;

             if (!XRRSetCrtcConfig(_ecore_x_disp, res, crtc, CurrentTime,
                                   x, y, mode, orientation, (RROutput *)outputs,
                                   noutputs))
                ret = EINA_TRUE;

             XRRFreeCrtcInfo(crtc_info);
          }

        XRRFreeScreenResources(res);
     }

   return ret;
#else
   return EINA_FALSE;
#endif
}

/**
 * @brief sets a CRTC relative to another one.
 * @param crtc_r1 the CRTC to be positioned.
 * @param crtc_r2 the CRTC the position should be relative to
 * @param position the relation between the crtcs
 * @param alignment in case CRTCs size differ, aligns CRTC1 accordingly at CRTC2's
 * borders
 * @return EINA_TRUE if crtc could be successfully positioned. EINA_FALSE if
 * repositioning failed or if position of new crtc would be out of given screen's min/max bounds.
 */
EAPI Eina_Bool
ecore_x_randr_crtc_pos_relative_set(Ecore_X_Window root,
                                    Ecore_X_Randr_Crtc crtc_r1,
                                    Ecore_X_Randr_Crtc crtc_r2,
                                    Ecore_X_Randr_Output_Policy policy,
                                    Ecore_X_Randr_Relative_Alignment alignment)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);

   Eina_Rectangle r1_geo, r2_geo;
   int w_max, h_max, cw, ch, x_n = Ecore_X_Randr_Unset, y_n =
      Ecore_X_Randr_Unset;
   /*
   int r1_noutputs, r2_noutputs, r1_nmodes, i, j, outputs_mode_found, mode_w, mode_h;
   Ecore_X_Randr_Output *r1_outputs, *r2_outputs, *r2_r1_outputs;
   Ecore_X_Randr_Mode *r1_modes, r2_mode, r1_mode;
   Eina_Bool ret;
   */

   if ((ecore_x_randr_crtc_mode_get(root, crtc_r1) == Ecore_X_Randr_None)
       || (ecore_x_randr_crtc_mode_get(root, crtc_r2) == Ecore_X_Randr_None))
      return EINA_FALSE;

   if (!_ecore_x_randr_crtc_validate(root, crtc_r1) ||
       (!(crtc_r1 != crtc_r2) &&
           !_ecore_x_randr_crtc_validate(root, crtc_r2)))
     return EINA_FALSE;

   ecore_x_randr_crtc_geometry_get(root,
                                   crtc_r1,
                                   &r1_geo.x,
                                   &r1_geo.y,
                                   &r1_geo.w,
                                   &r1_geo.h);
   ecore_x_randr_crtc_geometry_get(root,
                                   crtc_r2,
                                   &r2_geo.x,
                                   &r2_geo.y,
                                   &r2_geo.w,
                                   &r2_geo.h);
   ecore_x_randr_screen_size_range_get(root, NULL, NULL, &w_max, &h_max);
   ecore_x_randr_screen_current_size_get(root, &cw, &ch, NULL, NULL);

   switch (policy)
     {
      case ECORE_X_RANDR_OUTPUT_POLICY_RIGHT:
         //set r1 right of r2
         x_n = r2_geo.x + r2_geo.w;

         switch (alignment)
           {
            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
               y_n = Ecore_X_Randr_Unset;
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
               y_n =
                  ((int)(((double)r2_geo.h /
                          2.0) + (double)r2_geo.y - ((double)r1_geo.h / 2.0)));
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
               y_n = ((int)((double)ch / 2.0) - ((double)r1_geo.h / 2.0));
               break;
           }
         break;

      case ECORE_X_RANDR_OUTPUT_POLICY_LEFT:
         //set r1 left of r2
         x_n = r2_geo.x - r1_geo.w;

         switch (alignment)
           {
            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
               y_n = Ecore_X_Randr_Unset;
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
               y_n =
                  ((int)(((double)r2_geo.h /
                          2.0) + r2_geo.y - ((double)r1_geo.h / 2.0)));
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
               y_n = ((int)(((double)ch / 2.0) - ((double)r1_geo.h / 2.0)));
               break;
           }
         break;

      case ECORE_X_RANDR_OUTPUT_POLICY_BELOW:
         //set r1 below r2
         y_n = r2_geo.y + r2_geo.h;

         switch (alignment)
           {
            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
               x_n = Ecore_X_Randr_Unset;
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
               x_n =
                  ((int)((((double)r2_geo.x +
                           (double)r2_geo.w) / 2.0) - ((double)r1_geo.w / 2.0)));
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
               x_n = ((int)((double)cw / 2.0));
               break;
           }
         break;

      case ECORE_X_RANDR_OUTPUT_POLICY_ABOVE:
         y_n = r2_geo.y - r1_geo.h;

         //set r1 above r2
         switch (alignment)
           {
            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_NONE:
               x_n = Ecore_X_Randr_Unset;
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_REL:
               x_n =
                  ((int)((((double)r2_geo.x +
                           (double)r2_geo.w) / 2.0) - ((double)r1_geo.w / 2.0)));
               break;

            case ECORE_X_RANDR_RELATIVE_ALIGNMENT_CENTER_SCR:
               x_n = ((int)((double)cw / 2.0));
               break;
           }
	 break;

      case ECORE_X_RANDR_OUTPUT_POLICY_CLONE:
	 return  ecore_x_randr_crtc_pos_set(root, crtc_r1, r2_geo.x, r2_geo.y);

	 /* entire cloning (including modesetting)
         //all outputs of crtc1 capable of crtc2's current mode?
         r2_mode = ecore_x_randr_crtc_mode_get(root, crtc_r2);
         if (!(r1_outputs =
                  ecore_x_randr_crtc_outputs_get(root, crtc_r1,
                                                 &r1_noutputs)) ||
             (r1_noutputs == 0))
            return EINA_FALSE;

         for (i = 0, outputs_mode_found = 0; i < r1_noutputs; i++)
           {
              if (!(r1_modes =
                       ecore_x_randr_output_modes_get(root, r1_outputs[i],
                                                      &r1_nmodes, NULL)))
                {
                   free(r1_outputs);
                   return EINA_FALSE;
                }

              for (j = 0; j < r1_nmodes; j++)
                {
                   ecore_x_randr_mode_size_get(root,
                                               r1_modes[j],
                                               &mode_w,
                                               &mode_h);
                   if ((mode_w == r2_geo.w) && (mode_h == r2_geo.h))
                     {
                        r1_mode = r1_modes[j];
                        ++outputs_mode_found;
                        free(r1_modes);
                        r1_modes = NULL;
                        break;
                     }
                }
              if (r1_modes)
                        free(r1_modes);

              if (outputs_mode_found <= i)
                {
                   //an output doesn't support the set mode, cancel!
                        free(r1_outputs);
                   return EINA_FALSE;
                }
           }
                        free (r1_outputs);
         //CRTC 1's outputs support a mode of same geometry as CRTC 2.
         ret =
            (ecore_x_randr_crtc_mode_set(root, crtc_r1, Ecore_X_Randr_None,
                                         Ecore_X_Randr_None,
                                         r1_mode) &&
             ecore_x_randr_crtc_pos_set(root, crtc_r1, r2_geo.x, r2_geo.y));
         return ret;
	 */

	 /* entire cloning on same CRTC
         //all outputs of crtc1 capable of crtc2's current mode?
         r2_mode = ecore_x_randr_crtc_mode_get(root, crtc_r2);
         if (!(r1_outputs =
                  ecore_x_randr_crtc_outputs_get(root, crtc_r1,
                                                 &r1_noutputs)) ||
             (r1_noutputs == 0))
            return EINA_FALSE;

         for (i = 0, outputs_mode_found = 0; i < r1_noutputs; i++)
           {
              if (!(r1_modes =
                       ecore_x_randr_output_modes_get(root, r1_outputs[i],
                                                      &r1_nmodes, NULL)))
                {
                        free(r1_outputs);
                   return EINA_FALSE;
                }

              for (j = 0; j < r1_nmodes; j++)
                {
                   if (r1_modes[j] == r2_mode)
                     {
                        ++outputs_mode_found;
                        free(r1_modes);
                        r1_modes = NULL;
                        break;
                     }
                }
              if (r1_modes)
                        free(r1_modes);

              if (outputs_mode_found <= i)
                {
                   //an output doesn't support the set mode, cancel!
                        free(r1_outputs);
                   return EINA_FALSE;
                }
           }
         //check whether crtc r2 can use all outputs of r1.
         if (!(r2_outputs =
                  ecore_x_randr_crtc_possible_outputs_get(root, crtc_r2,
                                                          &r2_noutputs)) ||
             (r2_noutputs == 0))
           {
                   free(r1_outputs);
              return EINA_FALSE;
           }

         for (i = 0; i < r1_noutputs; i++)
           {
              for (j = 0; j < r2_noutputs; )
                {
                   if (r1_outputs[i] == r2_outputs[j])
                      break;

                   j++;
                }
              if (j == r2_noutputs)
                {
                   //didn't find the output!
                   free (r1_outputs);
                   free (r2_outputs);
                   return EINA_FALSE;
                }
           }

         //apparently crtc2 supports all outputs of r1
         //TODO: check with the compatible list of outputs (property in RR1.3)
         r2_r1_outputs =
            malloc(sizeof(Ecore_X_Randr_Output) * (r1_noutputs + r2_noutputs));
         for (i = 0; i < r1_noutputs; i++)
           {
              r2_r1_outputs[i] = r1_outputs[i];
           }
            free(r1_outputs);
         for (; i < r2_noutputs; i++)
           {
              r2_r1_outputs[i] = r2_outputs[i];
           }
            free(r2_outputs);
         ret =
            ecore_x_randr_crtc_mode_set(root, crtc_r2, r2_r1_outputs,
                                        (r1_noutputs + r1_noutputs), r2_mode);
         free (r2_r1_outputs);
         return ret;
	 */
      case ECORE_X_RANDR_OUTPUT_POLICY_NONE:
        break;
     }
   if ((x_n == r1_geo.x) && (y_n == r1_geo.x))
      return EINA_TRUE;

   //out of possible bounds?
   if (((y_n + r1_geo.h) > h_max) || ((x_n + r1_geo.w) > w_max))
      return EINA_FALSE;

   return ecore_x_randr_crtc_pos_set(root, crtc_r1, x_n, y_n);
#else
   return EINA_FALSE;
#endif
}


EAPI Ecore_X_Randr_Mode *
ecore_x_randr_output_modes_get(Ecore_X_Window root,
                               Ecore_X_Randr_Output output,
                               int *num,
                               int *npreferred)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;
   Ecore_X_Randr_Mode *modes = NULL;

   if ((output != Ecore_X_Randr_None)
       && (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root))
       && (output_info =
              XRRGetOutputInfo(_ecore_x_disp, res, (RROutput)output)))
     {
        if ((modes = malloc(sizeof(Ecore_X_Randr_Mode) * output_info->nmode)))
          {
             memcpy(modes, output_info->modes,
                    (sizeof(Ecore_X_Randr_Mode) * output_info->nmode));
             if (num)
                *num = output_info->nmode;

             if (npreferred)
                *npreferred = output_info->npreferred;
          }
     }

   if (output_info)
      XRRFreeOutputInfo(output_info);

   if (res)
      XRRFreeScreenResources(res);

   return modes;
#else
   return NULL;
#endif
}

EAPI Ecore_X_Randr_Crtc *
ecore_x_randr_output_possible_crtcs_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *num)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;
   Ecore_X_Randr_Crtc *crtcs = NULL;

   if ((output != Ecore_X_Randr_None))
     {
	if ((res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
	  {
	     if ((output_info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
	       {
		  if ((crtcs = malloc(sizeof(Ecore_X_Randr_Crtc) * output_info->ncrtc)))
		    {
		       memcpy(crtcs, output_info->crtcs, (sizeof(Ecore_X_Randr_Crtc) * output_info->ncrtc));
		       if (num) *num = output_info->ncrtc;
		    }
		  XRRFreeOutputInfo(output_info);
	       }
	     XRRFreeScreenResources(res);
	  }
     }
     return crtcs;
#else
   return Ecore_X_Randr_None;
#endif
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
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;
   Ecore_X_Randr_Output *outputs = NULL;

   if ((output != Ecore_X_Randr_None))
     {
	if ((res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
	  {
	     if ((output_info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
	       {
		  if ((outputs = malloc(sizeof(Ecore_X_Randr_Output) * output_info->nclone)))
		    {
		       memcpy(outputs, output_info->clones, (sizeof(Ecore_X_Randr_Output) * output_info->nclone));
		       if (num) *num = output_info->nclone;
		    }
		  XRRFreeOutputInfo(output_info);
	       }
	     XRRFreeScreenResources(res);
	  }
     }
     return outputs;
#else
   return Ecore_X_Randr_None;
#endif
}

EAPI Ecore_X_Randr_Crtc
ecore_x_randr_output_crtc_get(Ecore_X_Window root, Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(Ecore_X_Randr_None);
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;
   Ecore_X_Randr_Crtc ret = Ecore_X_Randr_None;

   if ((output != Ecore_X_Randr_None))
     {
	if ((res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
	  {
	     if ((output_info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
	       {
		  ret = output_info->crtc;
		  XRRFreeOutputInfo(output_info);
	       }
	     XRRFreeScreenResources(res);
	  }
     }

   return ret;
#else
   return Ecore_X_Randr_None;
#endif
}

/**
 * @brief gets the given output's name as reported by X
 * @param root the window which's screen will be queried
 * @param len length of returned c-string.
 * @return name of the output as reported by X
 */
EAPI char *
ecore_x_randr_output_name_get(Ecore_X_Window root, Ecore_X_Randr_Output output, int *len)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;
   char *ret = NULL;

   if ((output != Ecore_X_Randr_None)
       && (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root))
       && (output_info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
     {
	/*
	 * Actually the below command is correct, but due to a bug in libXrandr
	 * it doesn't work. Therefore we stick with strlen().
	 * Replace the line below with the following once this bug is
	 * fixed within libXrandr.
	 *
	 *    *len = output_info->nameLen;
	 *
	 */
	if ((ret = strdup(output_info->name)) && len)
	  *len = strlen(ret);

	XRRFreeOutputInfo(output_info);
     }

   if (res)
     XRRFreeScreenResources(res);

   return ret;
#else
   return NULL;
#endif
}

/**
 * @brief gets the width and hight of a given mode
 * @param mode the mode which's size is to be looked up
 * @param w width of given mode in px
 * @param h height of given mode in px
 */
EAPI void
ecore_x_randr_mode_size_get(Ecore_X_Window root, Ecore_X_Randr_Mode mode, int *w, int *h)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   XRRScreenResources *res = NULL;
   int i;

   if ((mode != Ecore_X_Randr_None)
       && (w || h)
       && (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
     {
	for (i = 0; i < res->nmode; i++)
	  {
	     if (res->modes[i].id == mode)
	       {
		  if (w)
		    *w = res->modes[i].width;

		  if (h)
		    *h = res->modes[i].height;

		  break;
	       }
	  }
     }

   if (res)
     XRRFreeScreenResources(res);

#endif
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
ecore_x_randr_output_edid_get(Ecore_X_Window root,
                              Ecore_X_Randr_Output output,
                              unsigned long *length)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(NULL);
   Atom name = XInternAtom (_ecore_x_disp, RANDR_PROPERTY_EDID, False);
   unsigned char *prop_data, *ret = NULL;
   int actual_format;
   unsigned long nitems, bytes_after;
   Atom actual_type;

   if (!length || !_ecore_x_randr_output_validate(root, output))
      return NULL;

   if(XRRGetOutputProperty (_ecore_x_disp, output, name,
                            0, 100, False, False,
                            AnyPropertyType,
                            &actual_type, &actual_format,
                            &nitems, &bytes_after, &prop_data) == Success)
     {
        if (actual_type == XA_INTEGER && actual_format == 8)
          {
             if ((ret = malloc(nitems * sizeof(unsigned char))))
               {
                  if(length &&
                     (memcpy(ret, prop_data, (nitems * sizeof(unsigned char)))))
                     *length = nitems;

                  return ret;
               }
          }
     }

   return NULL;
#else
   return NULL;
#endif
}

EAPI Ecore_X_Randr_Connection_Status
ecore_x_randr_output_connection_status_get(Ecore_X_Window root,
                                           Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN);
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;
   Ecore_X_Randr_Connection_Status ret =
      ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN;

   if ((output != Ecore_X_Randr_None)
       && (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root))
       && (output_info = XRRGetOutputInfo(_ecore_x_disp, res, output)))
     {
        ret = output_info->connection;
     }

   if (output_info)
      XRRFreeOutputInfo(output_info);

   if (res)
      XRRFreeScreenResources(res);

   return ret;
#else
   return ECORE_X_RANDR_CONNECTION_STATUS_UNKNOWN;
#endif
}

EAPI void
ecore_x_randr_output_size_mm_get(Ecore_X_Window root,
                                 Ecore_X_Randr_Output output,
                                 int *w_mm,
                                 int *h_mm)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   XRRScreenResources *res = NULL;
   XRROutputInfo *output_info = NULL;

   if ((output != Ecore_X_Randr_None)
       && (res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
     {
        if ((output_info =
                XRRGetOutputInfo(_ecore_x_disp, res, (RROutput)output)))
          {
             if (w_mm)
                *w_mm = output_info->mm_width;

             if (h_mm)
                *h_mm = output_info->mm_height;

             XRRFreeOutputInfo(output_info);
          }

        XRRFreeScreenResources(res);
     }

#endif
}

EAPI Eina_Bool
ecore_x_randr_move_all_crtcs_but(Ecore_X_Window root,
                                 const Ecore_X_Randr_Crtc *not_moved,
                                 int nnot_moved,
                                 int dx,
                                 int dy)
{
#ifdef ECORE_XRANDR
   Ecore_X_Randr_Crtc *crtcs_to_be_moved = NULL;
   XRRScreenResources *res = NULL;
   int i, j, k, n;
   Eina_Bool ret;


   if ((nnot_moved <= 0) || (!not_moved)
       || !_ecore_x_randr_root_validate(root)
       || !(res =
               _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
      return EINA_FALSE;

   n = (res->ncrtc - nnot_moved);
   if ((crtcs_to_be_moved = malloc(sizeof(Ecore_X_Randr_Crtc) * n)))
     {
        for (i = 0, k = 0; (i < res->ncrtc) && (k < n); i++)
          {
             for (j = 0; j < nnot_moved; j++)
               {
                  if (res->crtcs[i] == not_moved[j])
                     break;
               }
             if (j == nnot_moved)
                //crtcs[i] is not in the 'not to move'-list
                crtcs_to_be_moved[k++] = res->crtcs[i];
          }
     }

   XRRFreeScreenResources(res);
   ret = ecore_x_randr_move_crtcs(root, crtcs_to_be_moved, n, dx, dy);
   free(crtcs_to_be_moved);
   return ret;
#else
   return EINA_FALSE;
#endif
}

/*
 * @brief move given CRTCs belonging to the given root window's screen dx/dy pixels relative to their current position. The screen size will be automatically adjusted if necessary and possible.
 * @param root window which's screen's resources are used
 * @param crtcs list of CRTCs to be moved
 * @param ncrtc number of CRTCs in array
 * @param dx amount of pixels the CRTCs should be moved in x direction
 * @param dy amount of pixels the CRTCs should be moved in y direction
 * @return EINA_TRUE if all crtcs could be moved successfully.
 */
EAPI Eina_Bool
ecore_x_randr_move_crtcs(Ecore_X_Window root,
                         const Ecore_X_Randr_Crtc *crtcs,
                         int ncrtc,
                         int dx,
                         int dy)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);
   XRRScreenResources *res = NULL;
   XRRCrtcInfo **crtc_info = NULL;
   Eina_Bool ret = EINA_TRUE;
   int i, cw, ch, w_max, h_max, nw, nh;

   crtc_info = alloca(sizeof(XRRCrtcInfo *) * ncrtc);
   memset(crtc_info, 0, sizeof(XRRCrtcInfo *) * ncrtc);
   if (_ecore_x_randr_root_validate(root)
       && (res = _ecore_x_randr_get_screen_resources (_ecore_x_disp, root)))
     {
        ecore_x_randr_screen_size_range_get(root, NULL, NULL, &w_max, &h_max);
        ecore_x_randr_screen_current_size_get(root, &cw, &ch, NULL, NULL);
        nw = cw;
        nh = ch;

        for (i = 0;
             (i < ncrtc) &&
             (crtc_info[i] = XRRGetCrtcInfo(_ecore_x_disp, res, crtcs[i]));
             i++)
          {
             if (((crtc_info[i]->x + dx) < 0) ||
                 ((int)(crtc_info[i]->x + crtc_info[i]->width + dx) > w_max)
                 || ((crtc_info[i]->y + dy) < 0) ||
                 ((int)(crtc_info[i]->y + crtc_info[i]->height + dy) > h_max)
                 )
                goto _ecore_x_randr_move_crtcs_fail_free_crtc_info;

             nw = MAX((int)(crtc_info[i]->x + crtc_info[i]->width + dx), nw);
             nh = MAX((int)(crtc_info[i]->y + crtc_info[i]->height + dy), nh);
          }
        //not out of bounds

        //resize if necessary
        if (!(((nw > cw) ||
               (nh > ch)) ||
              ecore_x_randr_screen_current_size_set(root, nw, nh,
                                                    Ecore_X_Randr_Unset,
                                                    Ecore_X_Randr_Unset)))
           goto _ecore_x_randr_move_crtcs_fail_free_crtc_info;

        //actually move all the crtcs, keep their rotation and mode.
        for (i = 0; (i < ncrtc) && crtc_info[i]; i++)
          {
             if ((crtc_info[i]) &&
                 (!ecore_x_randr_crtc_settings_set(root, crtcs[i], NULL,
                                                   Ecore_X_Randr_Unset,
                                                   (crtc_info[i]->x + dx),
                                                   (crtc_info[i]->y + dy),
                                                   crtc_info[i]->mode,
                                                   crtc_info[i]->rotation)))
               {
                  ret = EINA_FALSE;
                  break;
               }
          }
        if (i < ncrtc)
          {
             //something went wrong, let's try to move the already moved crtcs
             //back.
             while ((i--) >= 0)
               {
                  if (crtc_info[i])
                     ecore_x_randr_crtc_settings_set(root,
                                                     crtcs[i],
                                                     NULL,
                                                     Ecore_X_Randr_Unset,
                                                     (crtc_info[i]->x - dx),
                                                     (crtc_info[i]->y - dy),
                                                     crtc_info[i]->mode,
                                                     crtc_info[i]->rotation);
               }
          }

        for (i = 0; i < ncrtc; i++)
          {
             if (crtc_info[i]) XRRFreeCrtcInfo(crtc_info[i]);
          }
     }

   XRRFreeScreenResources(res);

   return ret;
_ecore_x_randr_move_crtcs_fail_free_crtc_info:
   while (i-- > 0)
      XRRFreeCrtcInfo(crtc_info[i]);
   XRRFreeScreenResources(res);
   return EINA_FALSE;
#else
   return EINA_FALSE;
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
   XRRCrtcInfo *crtc_info = NULL;
   XRRScreenResources *res = NULL;
   //the 100000 are just a random huge number.
   int i, dx_min = 100000, dy_min = 100000, w_n = 0, h_n = 0, nenabled_crtcs = 0;

   if (!_ecore_x_randr_root_validate(root) ||
       !(res = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root)))
      return;

   Ecore_X_Randr_Crtc enabled_crtcs[res->ncrtc];

   for (i = 0; i < res->ncrtc; i++)
     {
        if (!(crtc_info =
                 XRRGetCrtcInfo(_ecore_x_disp, res,
                                res->crtcs[i])) ||
            (crtc_info->mode == Ecore_X_Randr_None) ||
            (crtc_info->mode == Ecore_X_Randr_Unset)
            || ((crtc_info->noutput == 0)))
           continue;

	enabled_crtcs[nenabled_crtcs++] = res->crtcs[i];

        if ((int)(crtc_info->x + crtc_info->width) > w_n)
           w_n = (crtc_info->x + crtc_info->width);

        if ((int)(crtc_info->y + crtc_info->height) > h_n)
           h_n = (crtc_info->y + crtc_info->height);

	if (crtc_info->x < dx_min)
	  dx_min = crtc_info->x;
	if (crtc_info->y < dy_min)
	  dy_min = crtc_info->y;

        XRRFreeCrtcInfo(crtc_info);
     }
   fprintf(stderr, "E_RANDR: babam, resize! Trying to move all CRTCs with dx %d and dy %d", dx_min, dy_min);
   if ((dx_min > 0) || (dy_min > 0))
     {
	if (ecore_x_randr_move_crtcs(root, enabled_crtcs, nenabled_crtcs, -dx_min, -dy_min))
	  {
	     w_n -= dx_min;
	     h_n -= dy_min;
	  }
     }
   ecore_x_randr_screen_current_size_set(root,
                                         w_n,
                                         h_n,
                                         Ecore_X_Randr_Unset,
                                         Ecore_X_Randr_Unset);
#endif
}

/**
 * @brief set up the backlight level to the given level.
 * @param root the window's screen which will be set.
 * @param level of the backlight between 0 and 1
 */

EAPI void
ecore_x_randr_screen_backlight_level_set(Ecore_X_Window root,
                                         double         level)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET();
   Atom _backlight;
   XRRScreenResources *resources = NULL;
   Ecore_X_Randr_Output output;
   int o;

   if ((level < 0) || (level > 1))
     {
        ERR("Wrong value for the backlight level. It should be between 0 and 1.");
        return;
     }

   /*
    * To make sure that the _backlight atomic property still exists.
    */
   _backlight = XInternAtom(_ecore_x_disp, RANDR_PROPERTY_BACKLIGHT, True);
   if (_backlight == None)
     {
        WRN("Backlight setting is not supported on this server or driver");
        return;
     }

   /* get the ressources */
   resources = _ecore_x_randr_get_screen_resources(_ecore_x_disp, root);
   if (!resources) return;

   for (o = 0; o < resources->noutput; o++)
     {
        output = resources->outputs[o];
        if (ecore_x_randr_output_backlight_level_get(root, output) >= 0)
          {
             ecore_x_randr_output_backlight_level_set(root, output, level);
          }
     }
   XRRFreeScreenResources(resources);
#endif
}

/*
 * @brief get the backlight level of the given output
 * @param root window which's screen should be queried
 * @param output from which the backlight level should be retrieved
 * @return the backlight level
 */

EAPI double
ecore_x_randr_output_backlight_level_get(Ecore_X_Window       root,
                                         Ecore_X_Randr_Output output)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(-1);
   Atom actual_type;
   Atom _backlight;
   XRRPropertyInfo *info = NULL;
   double dvalue;
   int actual_format;
   long value, max, min;
   unsigned long nitems;
   unsigned long bytes_after;
   unsigned char *prop = NULL;

   /* set backlight variable if not already done */

   _backlight = XInternAtom(_ecore_x_disp, RANDR_PROPERTY_BACKLIGHT, True);
   if (_backlight == None)
     {
        ERR("Backlight property is not suppported on this server or driver");
        return -1;
     }

   if (!_ecore_x_randr_output_validate(root, output))
     {
        ERR("Invalid output");
        return -1;
     }

   if (XRRGetOutputProperty(_ecore_x_disp, output, _backlight,
                            0, 4, False, False, None,
                            &actual_type, &actual_format,
                            &nitems, &bytes_after, &prop) != Success)
     {
        WRN("Backlight not supported on this output");
        return -1;
     }

   if ((actual_type != XA_INTEGER) || (nitems != 1) || (actual_format != 32)) return -1;

   value = *((long *)prop);
   free (prop);

   /* I have the current value of the backlight */
   /* Now retrieve the min and max intensities of the output */
   info = XRRQueryOutputProperty(_ecore_x_disp, output, _backlight);
   if (info)
     {
        dvalue = -1;
        if ((info->range) && (info->num_values == 2))
          {
             /* finally convert the current value in the interval [0..1] */
              min = info->values[0];
              max = info->values[1];
              dvalue = ((double)(value - min)) / ((double)(max - min));
          }
        free(info);
        return dvalue;
     }
#endif
   return -1;
}

/*
 * @brief set the backlight level of a given output
 * @param root window which's screen should be queried
 * @param output that should be set
 * @param level for which the backlight should be set
 * @return EINA_TRUE in case of success
 */

EAPI Eina_Bool
ecore_x_randr_output_backlight_level_set(Ecore_X_Window       root,
                                         Ecore_X_Randr_Output output,
                                         double               level)
{
#ifdef ECORE_XRANDR
   RANDR_CHECK_1_2_RET(EINA_FALSE);
   Atom _backlight;
   XRRPropertyInfo *info = NULL;
   double min, max, tmp;
   long new;

   if ((level < 0) || (level > 1))
     {
        ERR("Backlight level should be between 0 and 1");
        return EINA_FALSE;
     }

   if (!_ecore_x_randr_output_validate(root, output))
     {
        ERR("Wrong output value");
        return EINA_FALSE;
     }

   _backlight = XInternAtom(_ecore_x_disp, RANDR_PROPERTY_BACKLIGHT, True);
   if (_backlight == None)
     {
        WRN("Backlight property is not suppported on this server or driver");
        return EINA_FALSE;
     }

   info = XRRQueryOutputProperty(_ecore_x_disp, output, _backlight);
   if (info)
     {
        if ((info->range) && (info->num_values == 2))
          {
             min = info->values[0];
             max = info->values[1];
             tmp = (level * (max - min)) + min;
             new = tmp;
             if (new > max) new = max;
             if (new < min) new = min;
             XRRChangeOutputProperty(_ecore_x_disp, output, _backlight, XA_INTEGER, 32,
                                     PropModeReplace, (unsigned char *)&new, 1);
             XFlush(_ecore_x_disp);
          }
        free(info);
        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

/*
 * @brief get the outputs, which display a certain window
 * @param window window the displaying outputs shall be found for
 * @param num the number of outputs displaying the window
 * @return array of outputs that display a certain window. NULL if no outputs
 * was found that displays the specified window.
 */

EAPI Ecore_X_Randr_Output *
ecore_x_randr_window_outputs_get(Ecore_X_Window window,
                                 int *num)
{
#ifdef ECORE_XRANDR
   Ecore_X_Window root;
   Eina_Rectangle w_geo, c_geo;
   Ecore_X_Randr_Crtc *crtcs;
   Ecore_X_Randr_Mode mode;
   Ecore_X_Randr_Output *outputs, *ret = NULL, *tret;
   Window tw;
   int ncrtcs, noutputs, i, nret = 0, rx = 0, ry = 0;

   if (_randr_version < RANDR_1_2) goto _ecore_x_randr_current_output_get_fail;

   ecore_x_window_geometry_get(window,
                               &w_geo.x, &w_geo.y,
                               &w_geo.w, &w_geo.h);

   root = ecore_x_window_root_get(window);
   crtcs = ecore_x_randr_crtcs_get(root, &ncrtcs);
   if (!crtcs) goto _ecore_x_randr_current_output_get_fail;

   /* now get window RELATIVE to root window - thats what matters. */
   XTranslateCoordinates(_ecore_x_disp, window, root, 0, 0, &rx, &ry, &tw);
   w_geo.x = rx;
   w_geo.y = ry;

   for (i = 0; i < ncrtcs; i++)
     {
        /* if crtc is not enabled, don't bother about it any further */
        mode = ecore_x_randr_crtc_mode_get(root, crtcs[i]);
        if (mode == Ecore_X_Randr_None) continue;

        ecore_x_randr_crtc_geometry_get(root, crtcs[i],
                                        &c_geo.x, &c_geo.y,
                                        &c_geo.w, &c_geo.h);
        if (eina_rectangles_intersect(&w_geo, &c_geo))
          {
             outputs = ecore_x_randr_crtc_outputs_get(root, crtcs[i],
                                                      &noutputs);
             /* The case below should be impossible, but for safety reasons
              * remains */
             if (!outputs)
               {
                  if (num) *num = 0;
                  free(ret);
                  free(crtcs);
                  return NULL;
               }
             tret = realloc(ret, ((nret + noutputs) * sizeof(Ecore_X_Randr_Output)));
             if (!tret)
               {
                  if (num) *num = 0;
                  free(outputs);
                  free(ret);
                  free(crtcs);
                  return NULL;
               }
             ret = tret;
             memcpy(&ret[nret], outputs, (noutputs * sizeof(Ecore_X_Randr_Output)));
             nret += noutputs;
             free(outputs);
          }
     }
   free(crtcs);

   if (num) *num = nret;
   return ret;

_ecore_x_randr_current_output_get_fail:
#endif
   if (num) *num = 0;
   return NULL;
}

/*
 * @depricated bad naming. Use ecore_x_randr_window_outputs_get instead.
 * @brief get the outputs, which display a certain window
 * @param window window the displaying outputs shall be found for
 * @param num the number of outputs displaying the window
 * @return array of outputs that display a certain window. NULL if no outputs
 * was found that displays the specified window.
 */

EINA_DEPRECATED EAPI Ecore_X_Randr_Output *
ecore_x_randr_current_output_get(Ecore_X_Window window,
                                 int *num)
{
   return ecore_x_randr_window_outputs_get(window, num);
}
