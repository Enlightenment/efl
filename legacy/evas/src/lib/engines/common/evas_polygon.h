#ifndef _EVAS_POLYGON_H
#define _EVAS_POLYGON_H


EAPI void                evas_common_polygon_init         (void);

EAPI RGBA_Polygon_Point *evas_common_polygon_point_add    (RGBA_Polygon_Point *points, int x, int y);
EAPI RGBA_Polygon_Point *evas_common_polygon_points_clear (RGBA_Polygon_Point *points);
EAPI void                evas_common_polygon_draw         (RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);


#endif /* _EVAS_POLYGON_H */

