#include "ecore_xcb_private.h"
#include <pixman.h>

/*
 * [ ] XPolygonRegion
 * [ ] XShrinkRegion
 * [ ] XClipBox
 * [ ] XXorRegion
 */

EAPI Ecore_X_XRegion *
ecore_x_xregion_new()
{
   pixman_region16_t *region;

   region = (pixman_region16_t *)malloc(sizeof(pixman_region16_t));
   if (!region) return NULL;

   pixman_region_init(region);

   return (Ecore_X_XRegion *)region;
}

EAPI void
ecore_x_xregion_free(Ecore_X_XRegion *region)
{
   if (!region) return;

   pixman_region_fini(region);
   free(region);
}

EAPI Eina_Bool
ecore_x_xregion_set(Ecore_X_XRegion *region, Ecore_X_GC gc)
{
   xcb_rectangle_t *rects;
   pixman_box16_t *boxes;
   int num = 0, i = 0;

   if (!region) return EINA_FALSE;

   boxes = pixman_region_rectangles((pixman_region16_t *)region, &num);
   if ((!boxes) || (num == 0)) return EINA_FALSE;

   rects = (xcb_rectangle_t *)malloc(sizeof(xcb_rectangle_t) * num);
   if (!rects) return EINA_FALSE;

   for (i = 0; i < num; i++)
     {
        rects[i].x = boxes[i].x1;
        rects[i].y = boxes[i].y1;
        rects[i].width = boxes[i].x2 - boxes[i].x1 + 1;
        rects[i].height = boxes[i].y2 - boxes[i].y1 + 1;
     }

   xcb_set_clip_rectangles(_ecore_xcb_conn, XCB_CLIP_ORDERING_YX_BANDED,
                           gc, 0, 0, num, rects);

   ecore_x_flush();
   return EINA_TRUE;
}

EAPI void
ecore_x_xregion_translate(Ecore_X_XRegion *region, int x, int y)
{
   if (!region) return;

   pixman_region_translate((pixman_region16_t *)region, x, y);
}

EAPI Eina_Bool
ecore_x_xregion_intersect(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2)
{
   return pixman_region_intersect((pixman_region16_t *)dst, 
                                  (pixman_region16_t *)r1, 
                                  (pixman_region16_t *)r2);
}

EAPI Eina_Bool
ecore_x_xregion_union(Ecore_X_XRegion *dst, Ecore_X_XRegion *r1, Ecore_X_XRegion *r2)
{
   return pixman_region_union((pixman_region16_t *)dst, 
                              (pixman_region16_t *)r1, 
                              (pixman_region16_t *)r2);
}

EAPI Eina_Bool
ecore_x_xregion_union_rect(Ecore_X_XRegion *dst, Ecore_X_XRegion *src, Ecore_X_Rectangle *rect)
{
   return pixman_region_union_rect((pixman_region16_t *)dst, 
                                   (pixman_region16_t *)src,
                                   rect->x, rect->y, rect->width, rect->height);
}

EAPI Eina_Bool
ecore_x_xregion_subtract(Ecore_X_XRegion *dst, Ecore_X_XRegion *rm, Ecore_X_XRegion *rs)
{
   return pixman_region_subtract((pixman_region16_t *)dst, 
                                 (pixman_region16_t *)rm, 
                                 (pixman_region16_t *)rs);
}

EAPI Eina_Bool
ecore_x_xregion_is_empty(Ecore_X_XRegion *region)
{
   if (!region) return EINA_TRUE;

   return !pixman_region_not_empty((pixman_region16_t *)region);
}

EAPI Eina_Bool
ecore_x_xregion_is_equal(Ecore_X_XRegion *r1, Ecore_X_XRegion *r2)
{
   if ((!r1) || (!r2)) return EINA_FALSE;

   return pixman_region_equal((pixman_region16_t *)r1, 
                              (pixman_region16_t *)r2);
}

EAPI Eina_Bool
ecore_x_xregion_point_contain(Ecore_X_XRegion *region, int x, int y)
{
   if (!region) return EINA_FALSE;

   return pixman_region_contains_point((pixman_region16_t *)region, x, y, NULL);
}

EAPI Eina_Bool
ecore_x_xregion_rect_contain(Ecore_X_XRegion *region, Ecore_X_Rectangle *rect)
{
   pixman_box16_t box;

   if ((!region) || (!rect)) return EINA_FALSE;

   box.x1 = rect->x;
   box.y1 = rect->y;
   box.x2 = rect->x + rect->width - 1;
   box.y2 = rect->y + rect->height - 1;

   return pixman_region_contains_rectangle((pixman_region16_t *)region, &box);
}
