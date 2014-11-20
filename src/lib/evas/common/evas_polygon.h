#ifndef _EVAS_POLYGON_H
#define _EVAS_POLYGON_H


EAPI void                evas_common_polygon_init         (void);

EAPI RGBA_Polygon_Point *evas_common_polygon_point_add    (RGBA_Polygon_Point *points, int x, int y);
EAPI RGBA_Polygon_Point *evas_common_polygon_points_clear (RGBA_Polygon_Point *points);
EAPI void                evas_common_polygon_draw         (RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points, int x, int y);

EAPI void                evas_common_polygon_rgba_draw    (RGBA_Image *dst, int ext_x, int ext_y, int ext_w, int ext_h, DATA32 col, int render_op, RGBA_Polygon_Point *points, int x, int y, RGBA_Image *mask_ie, int mask_x, int mask_y);

#endif /* _EVAS_POLYGON_H */

