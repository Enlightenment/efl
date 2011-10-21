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
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return (Ecore_X_XRegion *)XCreateRegion();
} /* ecore_x_xregion_new */

EAPI void
ecore_x_xregion_free(Ecore_X_XRegion *region)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!region)
     return;

   XDestroyRegion((Region)region);
} /* ecore_x_xregion_free */

EAPI Eina_Bool
ecore_x_xregion_set(Ecore_X_XRegion *region,
                    Ecore_X_GC       gc)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XSetRegion(_ecore_x_disp, gc, (Region)region) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_set */

EAPI void
ecore_x_xregion_translate(Ecore_X_XRegion *region,
                          int              x,
                          int              y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!region)
     return;

   /* return value not used */
   XOffsetRegion((Region)region, x, y);
} /* ecore_x_xregion_translate */

EAPI Eina_Bool
ecore_x_xregion_intersect(Ecore_X_XRegion *dst,
                          Ecore_X_XRegion *r1,
                          Ecore_X_XRegion *r2)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XIntersectRegion((Region)r1, (Region)r2, (Region)dst) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_intersect */

EAPI Eina_Bool
ecore_x_xregion_union(Ecore_X_XRegion *dst,
                      Ecore_X_XRegion *r1,
                      Ecore_X_XRegion *r2)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XUnionRegion((Region)r1, (Region)r2, (Region)dst) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_union */

EAPI Eina_Bool
ecore_x_xregion_union_rect(Ecore_X_XRegion   *dst,
                           Ecore_X_XRegion   *src,
                           Ecore_X_Rectangle *rect)
{
   XRectangle xr;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xr.x = rect->x;
   xr.y = rect->y;
   xr.width = rect->width;
   xr.height = rect->height;

   return XUnionRectWithRegion(&xr, (Region)src, (Region)dst) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_union_rect */

EAPI Eina_Bool
ecore_x_xregion_subtract(Ecore_X_XRegion *dst,
                         Ecore_X_XRegion *rm,
                         Ecore_X_XRegion *rs)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XSubtractRegion((Region)rm, (Region)rs, (Region)dst) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_subtract */

EAPI Eina_Bool
ecore_x_xregion_is_empty(Ecore_X_XRegion *region)
{
   if (!region)
     return EINA_TRUE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XEmptyRegion((Region)region) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_is_empty */

EAPI Eina_Bool
ecore_x_xregion_is_equal(Ecore_X_XRegion *r1,
                         Ecore_X_XRegion *r2)
{
   if (!r1 || !r2)
     return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XEqualRegion((Region)r1, (Region)r1) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_is_equal */

EAPI Eina_Bool
ecore_x_xregion_point_contain(Ecore_X_XRegion *region,
                              int              x,
                              int              y)
{
   if (!region)
     return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XPointInRegion((Region)region, x, y) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_point_contain */

EAPI Eina_Bool
ecore_x_xregion_rect_contain(Ecore_X_XRegion   *region,
                             Ecore_X_Rectangle *rect)
{
   if (!region || !rect)
     return EINA_FALSE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   return XRectInRegion((Region)region,
                        rect->x,
                        rect->y,
                        rect->width,
                        rect->height) ? EINA_TRUE : EINA_FALSE;
} /* ecore_x_xregion_rect_contain */

