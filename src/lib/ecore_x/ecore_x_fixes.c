#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>

#include "ecore_x_private.h"
#include "Ecore_X.h"

#include <X11/Xregion.h>

static int _fixes_available;
#ifdef ECORE_XFIXES
static int _fixes_major, _fixes_minor;
static int _cursor_visible = 1;
#endif /* ifdef ECORE_XFIXES */

void
_ecore_x_fixes_init(void)
{
#ifdef ECORE_XFIXES
   _fixes_major = 3;
   _fixes_minor = 0;

   LOGFN;

   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);

   if (XFixesQueryVersion(_ecore_x_disp, &_fixes_major, &_fixes_minor))
     {
        _fixes_available = 1;

        ECORE_X_EVENT_FIXES_SELECTION_NOTIFY = ecore_event_type_new();
     }
   else
     _fixes_available = 0;

#else /* ifdef ECORE_XFIXES */
   _fixes_available = 0;
#endif /* ifdef ECORE_XFIXES */
}

#ifdef ECORE_XFIXES
/* I don't know what to call this function. */
static XRectangle *
_ecore_x_rectangle_ecore_to_x(Ecore_X_Rectangle *rects,
                              int num)
{
   XRectangle *xrect;
   int i;

   if (num == 0)
     return NULL;

   xrect = malloc(sizeof(XRectangle) * num);
   if (!xrect)
     return NULL;

   for (i = 0; i < num; i++)
     {
        xrect[i].x = rects[i].x;
        xrect[i].y = rects[i].y;
        xrect[i].width = rects[i].width;
        xrect[i].height = rects[i].height;
     }
   return xrect;
}

static Ecore_X_Rectangle *
_ecore_x_rectangle_x_to_ecore(XRectangle *xrect,
                              int num)
{
   Ecore_X_Rectangle *rects;
   int i;

   if (num == 0)
     return NULL;

   rects = malloc(sizeof(Ecore_X_Rectangle) * num);
   if (!rects)
     return NULL;

   for (i = 0; i < num; i++)
     {
        rects[i].x = xrect[i].x;
        rects[i].y = xrect[i].y;
        rects[i].width = xrect[i].width;
        rects[i].height = xrect[i].height;
     }
   return rects;
}

#endif /* ifdef ECORE_XFIXES */

EAPI Eina_Bool
ecore_x_fixes_selection_notification_request(Ecore_X_Atom selection)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, EINA_FALSE);

#ifdef ECORE_XFIXES
   if (_fixes_available)
     {
        XFixesSelectSelectionInput (_ecore_x_disp,
                                    DefaultRootWindow(_ecore_x_disp),
                                    selection,
                                    XFixesSetSelectionOwnerNotifyMask |
                                    XFixesSelectionWindowDestroyNotifyMask |
                                    XFixesSelectionClientCloseNotifyMask);
        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_fixes_window_selection_notification_request(Ecore_X_Window window, Ecore_X_Atom selection)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, EINA_FALSE);

#ifdef ECORE_XFIXES
   if (_fixes_available)
     {
        XFixesSelectSelectionInput (_ecore_x_disp,
                                    window,
                                    selection,
                                    XFixesSetSelectionOwnerNotifyMask |
                                    XFixesSelectionWindowDestroyNotifyMask |
                                    XFixesSelectionClientCloseNotifyMask);
        return EINA_TRUE;
     }
#endif
   return EINA_FALSE;
}

EAPI Ecore_X_Region
ecore_x_region_new(Ecore_X_Rectangle *rects,
                   int num)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, 0);

#ifdef ECORE_XFIXES
   Ecore_X_Region region;
   XRectangle *xrect;

   LOGFN;
   xrect = _ecore_x_rectangle_ecore_to_x(rects, num);
   region = XFixesCreateRegion(_ecore_x_disp, xrect, num);
   free(xrect);
   return region;
#else /* ifdef ECORE_XFIXES */
   return 0;
#endif /* ifdef ECORE_XFIXES */
}

EAPI Ecore_X_Region
ecore_x_region_new_from_bitmap(Ecore_X_Pixmap bitmap)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, 0);
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   LOGFN;
   region = XFixesCreateRegionFromBitmap(_ecore_x_disp, bitmap);
   if (_ecore_xlib_sync) ecore_x_sync();
   return region;
#else /* ifdef ECORE_XFIXES */
   return 0;
#endif /* ifdef ECORE_XFIXES */
}

EAPI Ecore_X_Region
ecore_x_region_new_from_window(Ecore_X_Window win,
                               Ecore_X_Region_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, 0);
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   LOGFN;
   region = XFixesCreateRegionFromWindow(_ecore_x_disp, win, type);
   if (_ecore_xlib_sync) ecore_x_sync();
   return region;
#else /* ifdef ECORE_XFIXES */
   return 0;
#endif /* ifdef ECORE_XFIXES */
}

EAPI Ecore_X_Region
ecore_x_region_new_from_gc(Ecore_X_GC gc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, 0);
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   LOGFN;
   region = XFixesCreateRegionFromGC(_ecore_x_disp, gc);
   if (_ecore_xlib_sync) ecore_x_sync();
   return region;
#else /* ifdef ECORE_XFIXES */
   return 0;
#endif /* ifdef ECORE_XFIXES */
}

EAPI Ecore_X_Region
ecore_x_region_new_from_picture(Ecore_X_Picture picture)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, 0);
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   LOGFN;
   region = XFixesCreateRegionFromPicture(_ecore_x_disp, picture);
   if (_ecore_xlib_sync) ecore_x_sync();
   return region;
#else /* ifdef ECORE_XFIXES */
   return 0;
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_free(Ecore_X_Region region)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesDestroyRegion(_ecore_x_disp, region);
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_set(Ecore_X_Region region,
                   Ecore_X_Rectangle *rects,
                   int num)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   XRectangle *xrect = _ecore_x_rectangle_ecore_to_x(rects, num);
   LOGFN;
   XFixesSetRegion(_ecore_x_disp, region, xrect, num);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_copy(Ecore_X_Region dest,
                    Ecore_X_Region source)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesCopyRegion(_ecore_x_disp, dest, source);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_combine(Ecore_X_Region dest,
                       Ecore_X_Region source1,
                       Ecore_X_Region source2)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesUnionRegion(_ecore_x_disp, dest, source1, source2);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_intersect(Ecore_X_Region dest,
                         Ecore_X_Region source1,
                         Ecore_X_Region source2)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesIntersectRegion(_ecore_x_disp, dest, source1, source2);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_subtract(Ecore_X_Region dest,
                        Ecore_X_Region source1,
                        Ecore_X_Region source2)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesSubtractRegion(_ecore_x_disp, dest, source1, source2);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_invert(Ecore_X_Region dest,
                      Ecore_X_Rectangle *bounds,
                      Ecore_X_Region source)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   XRectangle *xbound;
   int num = 1;

   LOGFN;
   xbound = _ecore_x_rectangle_ecore_to_x(bounds, num);

   XFixesInvertRegion(_ecore_x_disp, dest, xbound, source);
   if (_ecore_xlib_sync) ecore_x_sync();
   free(xbound);
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_translate(Ecore_X_Region region,
                         int dx,
                         int dy)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesTranslateRegion(_ecore_x_disp, region, dx, dy);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_extents(Ecore_X_Region dest,
                       Ecore_X_Region source)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesRegionExtents(_ecore_x_disp, dest, source);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI Ecore_X_Rectangle *
ecore_x_region_fetch(Ecore_X_Region region,
                     int *num,
                     Ecore_X_Rectangle *bounds)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_x_disp, NULL);
#ifdef ECORE_XFIXES
   Ecore_X_Rectangle *rects;
   XRectangle *xrect, xbound;

   LOGFN;
   xrect = XFixesFetchRegionAndBounds(_ecore_x_disp, region, num, &xbound);
   if (_ecore_xlib_sync) ecore_x_sync();
   rects = _ecore_x_rectangle_x_to_ecore(xrect, *num);
   (*bounds).x = xbound.x;
   (*bounds).y = xbound.y;
   (*bounds).width = xbound.width;
   (*bounds).height = xbound.height;
   XFree(xrect);
   return rects;
#else /* ifdef ECORE_XFIXES */
   return NULL;
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_expand(Ecore_X_Region dest,
                      Ecore_X_Region source,
                      unsigned int left,
                      unsigned int right,
                      unsigned int top,
                      unsigned int bottom)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesExpandRegion(_ecore_x_disp, dest, source, left, right, top, bottom);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_gc_clip_set(Ecore_X_Region region,
                           Ecore_X_GC gc,
                           int x_origin,
                           int y_origin)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesSetGCClipRegion(_ecore_x_disp, gc, x_origin, y_origin, region);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_window_shape_set(Ecore_X_Region region,
                                Ecore_X_Window win,
                                Ecore_X_Shape_Type type,
                                int x_offset,
                                int y_offset)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesSetWindowShapeRegion(_ecore_x_disp,
                              win,
                              type,
                              x_offset,
                              y_offset,
                              region);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_region_picture_clip_set(Ecore_X_Region region,
                                Ecore_X_Picture picture,
                                int x_origin,
                                int y_origin)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   LOGFN;
   XFixesSetPictureClipRegion(_ecore_x_disp,
                              picture,
                              x_origin,
                              y_origin,
                              region);
   if (_ecore_xlib_sync) ecore_x_sync();
#endif /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_cursor_show(void)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   if (!_cursor_visible)
     {
        XFixesShowCursor(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp));
        XFlush(_ecore_x_disp);
        _cursor_visible = 1;
     }
#endif  /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_cursor_hide(void)
{
   EINA_SAFETY_ON_NULL_RETURN(_ecore_x_disp);
#ifdef ECORE_XFIXES
   if (_cursor_visible)
     {
        XFixesHideCursor(_ecore_x_disp, DefaultRootWindow(_ecore_x_disp));
        XFlush(_ecore_x_disp);
        _cursor_visible = 0;
     }
#endif  /* ifdef ECORE_XFIXES */
}

EAPI void
ecore_x_root_screen_barriers_set(Ecore_X_Rectangle *screens, int num)
{
#ifdef ECORE_XFIXES
   static PointerBarrier *bar = NULL;
   static int bar_num = 0;
   static int bar_alloc = 0;
   Region reg, reg2, reg3;
   int i, j;

   // clear out old root screen barriers....
   if (bar)
     {
        for (i = 0; i < bar_num; i++)
          {
             XFixesDestroyPointerBarrier(_ecore_x_disp, bar[i]);
          }
        free(bar);
     }
   bar = NULL;
   bar_num = 0;
   bar_alloc = 0;
   if ((!screens) || (num <= 0)) return;

   // set up new ones given the screen list given
   for (i = 0; i < num; i++)
     {
        XRectangle xrect;

        reg = XCreateRegion();
        reg2 = XCreateRegion();
        xrect.x = screens[i].x - 1;
        xrect.y = screens[i].y - 1;
        xrect.width = screens[i].width + 2;
        xrect.height = screens[i].height + 2;
        XUnionRectWithRegion(&xrect, reg, reg2);
        XDestroyRegion(reg);
        reg = reg2;

        // reg == screen i + 1 pixel surrounding it
        for (j = 0; j < num; j++)
          {
             // create a region representing screen j
             reg2 = XCreateRegion();
             reg3 = XCreateRegion();
             xrect.x = screens[j].x;
             xrect.y = screens[j].y;
             xrect.width = screens[j].width;
             xrect.height = screens[j].height;
             XUnionRectWithRegion(&xrect, reg2, reg3);
             XDestroyRegion(reg2);
             reg2 = reg3;
             // reg2 == screen j

             reg3 = XCreateRegion();
             XSubtractRegion(reg, reg2, reg3);
             XDestroyRegion(reg);
             XDestroyRegion(reg2);
             reg = reg3;
             // reg now has had screen j cut out of the boundary
          }
        // reg is the result of starting with screen i and then with a
        // 1 pixel boundary around it havnig adjacent screens "cut out"
        // of that boundary leaving only extra bounds where no screens
        // are adjacent

        // walk rects and create barriers
        for (j = 0; j < reg->numRects; j++)
          {
             int x1, y1, x2, y2;

             bar_num++;
             if (bar_num > bar_alloc)
               {
                  bar_alloc += 32;
                  PointerBarrier *t = realloc(bar, bar_alloc * sizeof(PointerBarrier));
                  if (!t)
                    {
                       bar_num--;
                       return;
                    }
                  bar = t;
               }
             x1 = reg->rects[j].x1;
             y1 = reg->rects[j].y1;
             x2 = reg->rects[j].x2 - 1;
             y2 = reg->rects[j].y2 - 1;
             bar[bar_num - 1] =
               XFixesCreatePointerBarrier(_ecore_x_disp,
                                          DefaultRootWindow(_ecore_x_disp),
                                          x1, y1, x2, y2, 0, 0, NULL);
          }
        XDestroyRegion(reg);
     }
#endif
}
