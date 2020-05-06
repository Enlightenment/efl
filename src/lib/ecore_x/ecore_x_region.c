#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"

/*
 * [x] XCreateRegion
 * [ ] XPolygonRegion
 * [x] XSetRegion
 * [x] XDestroyRegion
 *
 * [x] XOffsetRegion
 * [ ] XShrinkRegion
 *
 * [ ] XClipBox
 * [x] XIntersectRegion
 * [x] XUnionRegion
 * [x] XUnionRectWithRegion
 * [x] XSubtractRegion
 * [ ] XXorRegion
 *
 * [x] XEmptyRegion
 * [x] XEqualRegion
 *
 * [x] XPointInRegion
 * [x] XRectInRegion
 */

EAPI Ecore_X_XRegion *
ecore_x_xregion_new()
{
   LOGFN;
   return (Ecore_X_XRegion *)XCreateRegion();
}

EAPI void
ecore_x_xregion_free(Ecore_X_XRegion *region)
{
   LOGFN;
   if (!region)
     return;

   XDestroyRegion((Region)region);
}

EAPI Eina_Bool
ecore_x_xregion_set(Ecore_X_XRegion *region,
                    Ecore_X_GC gc)
{
   Eina_Bool ret;
   LOGFN;
   ret = !!XSetRegion(_ecore_x_disp, gc, (Region)region);
   if (_ecore_xlib_sync) ecore_x_sync();
   return ret;
}

EAPI void
ecore_x_xregion_translate(Ecore_X_XRegion *region,
                          int x,
                          int y)
{
   LOGFN;
   if (!region)
     return;

   /* return value not used */
   XOffsetRegion((Region)region, x, y);
}

EAPI Eina_Bool
ecore_x_xregion_intersect(Ecore_X_XRegion *dst,
                          Ecore_X_XRegion *r1,
                          Ecore_X_XRegion *r2)
{
   LOGFN;
   return XIntersectRegion((Region)r1, (Region)r2, (Region)dst) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_union(Ecore_X_XRegion *dst,
                      Ecore_X_XRegion *r1,
                      Ecore_X_XRegion *r2)
{
   LOGFN;
   return XUnionRegion((Region)r1, (Region)r2, (Region)dst) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_union_rect(Ecore_X_XRegion *dst,
                           Ecore_X_XRegion *src,
                           Ecore_X_Rectangle *rect)
{
   XRectangle xr;

   LOGFN;
   xr.x = rect->x;
   xr.y = rect->y;
   xr.width = rect->width;
   xr.height = rect->height;

   return XUnionRectWithRegion(&xr, (Region)src, (Region)dst) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_subtract(Ecore_X_XRegion *dst,
                         Ecore_X_XRegion *rm,
                         Ecore_X_XRegion *rs)
{
   LOGFN;
   return XSubtractRegion((Region)rm, (Region)rs, (Region)dst) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_is_empty(Ecore_X_XRegion *region)
{
   if (!region)
     return EINA_TRUE;

   LOGFN;
   return XEmptyRegion((Region)region) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_is_equal(Ecore_X_XRegion *r1,
                         Ecore_X_XRegion *r2)
{
   if (!r1 || !r2)
     return EINA_FALSE;

   LOGFN;
   return XEqualRegion((Region)r1, (Region)r1) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_point_contain(Ecore_X_XRegion *region,
                              int x,
                              int y)
{
   if (!region)
     return EINA_FALSE;

   LOGFN;
   return XPointInRegion((Region)region, x, y) ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_x_xregion_rect_contain(Ecore_X_XRegion *region,
                             Ecore_X_Rectangle *rect)
{
   if (!region || !rect)
     return EINA_FALSE;

   LOGFN;
   return XRectInRegion((Region)region,
                        rect->x,
                        rect->y,
                        rect->width,
                        rect->height) ? EINA_TRUE : EINA_FALSE;
}

