/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_x_private.h"
#include "Ecore_X.h"

static int _fixes_available;
#ifdef ECORE_XFIXES
static int _fixes_major, _fixes_minor;
#endif

void
_ecore_x_fixes_init(void)
{
#ifdef ECORE_XFIXES
   _fixes_major = 3;
   _fixes_minor = 0;

   if (XFixesQueryVersion(_ecore_x_disp, &_fixes_major, &_fixes_minor))
     _fixes_available = 1;
   else
     _fixes_available = 0;
#else
   _fixes_available = 0;
#endif
}

#ifdef ECORE_XFIXES
/* I don't know what to call this function. */
static XRectangle *
_ecore_x_rectangle_ecore_to_x(Ecore_X_Rectangle *rects, int num)
{
   XRectangle *xrect;
   int i;

   if (num == 0) return NULL;

   xrect = malloc(sizeof(XRectangle) * num);
   if (!xrect) return NULL;
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
_ecore_x_rectangle_x_to_ecore(XRectangle *xrect, int num)
{
   Ecore_X_Rectangle *rects;
   int i;

   if (num == 0) return NULL;
   rects = malloc(sizeof(Ecore_X_Rectangle) * num);
   if (!rects) return NULL;
   for (i = 0; i < num; i++)
     {
	rects[i].x = xrect[i].x;
	rects[i].y = xrect[i].y;
	rects[i].width = xrect[i].width;
	rects[i].height = xrect[i].height;
     }
   return rects;
}
#endif

EAPI Ecore_X_Region
ecore_x_region_new(Ecore_X_Rectangle *rects, int num)
{
#ifdef ECORE_XFIXES
   Ecore_X_Region region;
   XRectangle *xrect;

   xrect = _ecore_x_rectangle_ecore_to_x(rects, num);
   region = XFixesCreateRegion(_ecore_x_disp, xrect, num);
   free(xrect);
   return region;
#else
   return 0;
#endif
}

EAPI Ecore_X_Region
ecore_x_region_new_from_bitmap(Ecore_X_Pixmap bitmap)
{
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   region = XFixesCreateRegionFromBitmap(_ecore_x_disp, bitmap);
   return region;
#else
   return 0;
#endif
}

EAPI Ecore_X_Region
ecore_x_region_new_from_window(Ecore_X_Window win, Ecore_X_Region_Type type)
{
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   region = XFixesCreateRegionFromWindow(_ecore_x_disp, win, type);
   return region;
#else
   return 0;
#endif
}

EAPI Ecore_X_Region
ecore_x_region_new_from_gc(Ecore_X_GC gc)
{
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   region = XFixesCreateRegionFromGC(_ecore_x_disp, gc);
   return region;
#else
   return 0;
#endif
}

EAPI Ecore_X_Region
ecore_x_region_new_from_picture(Ecore_X_Picture picture)
{
#ifdef ECORE_XFIXES
   Ecore_X_Region region;

   region = XFixesCreateRegionFromPicture(_ecore_x_disp, picture);
   return region;
#else
   return 0;
#endif
}

EAPI void
ecore_x_region_del(Ecore_X_Region region)
{
#ifdef ECORE_XFIXES
   XFixesDestroyRegion(_ecore_x_disp, region);
#endif
}

EAPI void
ecore_x_region_set(Ecore_X_Region region, Ecore_X_Rectangle *rects, int num)
{
#ifdef ECORE_XFIXES
   XRectangle *xrect = _ecore_x_rectangle_ecore_to_x(rects, num);
   XFixesSetRegion(_ecore_x_disp, region, xrect, num);
#endif
}

EAPI void
ecore_x_region_copy(Ecore_X_Region dest, Ecore_X_Region source)
{
#ifdef ECORE_XFIXES
   XFixesCopyRegion(_ecore_x_disp, dest, source);
#endif
}

EAPI void
ecore_x_region_combine(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2)
{
#ifdef ECORE_XFIXES
   XFixesUnionRegion(_ecore_x_disp, dest, source1, source2);
#endif
}

EAPI void
ecore_x_region_intersect(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2)
{
#ifdef ECORE_XFIXES
   XFixesIntersectRegion(_ecore_x_disp, dest, source1, source2);
#endif
}

EAPI void
ecore_x_region_subtract(Ecore_X_Region dest, Ecore_X_Region source1, Ecore_X_Region source2)
{
#ifdef ECORE_XFIXES
   XFixesSubtractRegion(_ecore_x_disp, dest, source1, source2);
#endif
}

EAPI void
ecore_x_region_invert(Ecore_X_Region dest, Ecore_X_Rectangle *bounds, Ecore_X_Region source)
{
#ifdef ECORE_XFIXES
   XRectangle *xbound;
   int num = 0;

   while(bounds + num) num++;
   xbound = _ecore_x_rectangle_ecore_to_x(bounds, num);

   XFixesInvertRegion(_ecore_x_disp, dest, xbound, source);
#endif
}

EAPI void
ecore_x_region_translate(Ecore_X_Region region, int dx, int dy)
{
#ifdef ECORE_XFIXES
   XFixesTranslateRegion(_ecore_x_disp, region, dx, dy);
#endif
}

EAPI void
ecore_x_region_extents(Ecore_X_Region dest, Ecore_X_Region source)
{
#ifdef ECORE_XFIXES
   XFixesRegionExtents(_ecore_x_disp, dest, source);
#endif
}

EAPI Ecore_X_Rectangle *
ecore_x_region_fetch(Ecore_X_Region region, int *num, Ecore_X_Rectangle *bounds){
#ifdef ECORE_XFIXES
     Ecore_X_Rectangle *rects;
     XRectangle *xrect, xbound;

     xrect = XFixesFetchRegionAndBounds(_ecore_x_disp, region, num, &xbound);
     rects = _ecore_x_rectangle_x_to_ecore(xrect, *num);
     (*bounds).x = xbound.x;
     (*bounds).y = xbound.y;
     (*bounds).width = xbound.width;
     (*bounds).height = xbound.height;
     return rects;
#else
     return NULL;
#endif
}

EAPI void
ecore_x_region_expand(Ecore_X_Region dest, Ecore_X_Region source, unsigned int left, unsigned int right, unsigned int top, unsigned int bottom)
{
#ifdef ECORE_XFIXES
   XFixesExpandRegion(_ecore_x_disp, dest, source, left, right, top, bottom);
#endif
}

EAPI void
ecore_x_region_gc_clip_set(Ecore_X_Region region, Ecore_X_GC gc, int x_origin, int y_origin)
{
#ifdef ECORE_XFIXES
   XFixesSetGCClipRegion(_ecore_x_disp, gc, x_origin, y_origin, region);
#endif
}

EAPI void
ecore_x_region_window_shape_set(Ecore_X_Region region, Ecore_X_Window win, Ecore_X_Shape_Type type, int x_offset, int y_offset)
{
#ifdef ECORE_XFIXES
   XFixesSetWindowShapeRegion(_ecore_x_disp, win, type, x_offset, y_offset, region);
#endif
}

EAPI void
ecore_x_region_picture_clip_set(Ecore_X_Region region, Ecore_X_Picture picture, int x_origin, int y_origin)
{
#ifdef ECORE_XFIXES
   XFixesSetPictureClipRegion(_ecore_x_disp, picture, x_origin, y_origin, region);
#endif
}

