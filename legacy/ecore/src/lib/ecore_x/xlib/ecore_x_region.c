/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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
   return (Ecore_X_XRegion *)XCreateRegion();
}

EAPI void
ecore_x_xregion_free(Ecore_X_XRegion *region)
{
   if (!region)
     return;

   XDestroyRegion((Region)region);
}

EAPI int
ecore_x_xregion_set(Ecore_X_XRegion *region, Ecore_X_GC gc)
{
   return XSetRegion(_ecore_x_disp, gc, (Region)region);
}

EAPI void
ecore_x_xregion_translate(Ecore_X_XRegion *region, int x, int y)
{
   if (!region)
     return;

   /* return value not used */
   XOffsetRegion((Region)region, x, y);
}

EAPI int
ecore_x_xregion_intersect(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2)
{
   return XIntersectRegion((Region)r1, (Region)r2, (Region)dst);
}

int
ecore_x_xregion_union(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2)
{
   return XUnionRegion((Region)r1, (Region)r2, (Region)dst);
}

EAPI int
ecore_x_xregion_union_rect(Ecore_X_XRegion *dst, Ecore_X_XRegion *src, Ecore_X_Rectangle *rect)
{
   XRectangle xr;

   xr.x = rect->x;
   xr.y = rect->y;
   xr.width = rect->width;
   xr.height = rect->height;

   return XUnionRectWithRegion(&xr, (Region)src, (Region)dst);
}

EAPI int
ecore_x_xregion_subtract(Ecore_X_XRegion *dst, Ecore_X_XRegion *rm, Ecore_X_XRegion *rs)
{
   return XSubtractRegion((Region)rm, (Region)rs, (Region)dst);
}

EAPI int
ecore_x_xregion_is_empty(Ecore_X_XRegion *region)
{
   if (!region)
     return 1;

   return !XEmptyRegion((Region)region);
}

EAPI int
ecore_x_xregion_is_equal(Ecore_X_XRegion *r1, Ecore_X_XRegion *r2)
{
   if (!r1 || !r2)
     return 0;

   return XEqualRegion((Region)r1, (Region)r1);
}

EAPI int
ecore_x_xregion_point_contain(Ecore_X_XRegion *region, int x, int y)
{
   if (!region)
     return 0;

   return XPointInRegion((Region)region, x, y);
}

EAPI int
ecore_x_xregion_rect_contain(Ecore_X_XRegion *region, Ecore_X_Rectangle *rect)
{
   if (!region || !rect)
     return 0;

   return XRectInRegion((Region)region, rect->x, rect->y, rect->width, rect->height);
}
