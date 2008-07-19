/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_GRADIENT_H
#define _EVAS_GRADIENT_H


EAPI void           evas_common_gradient_init              (void);

EAPI RGBA_Gradient *evas_common_gradient_new               (void);
EAPI void           evas_common_gradient_free              (RGBA_Gradient *gr);
EAPI void           evas_common_gradient_clear             (RGBA_Gradient *gr);
EAPI void           evas_common_gradient_color_stop_add    (RGBA_Gradient *gr, int r, int g, int b, int a, int dist);
EAPI void           evas_common_gradient_alpha_stop_add    (RGBA_Gradient *gr, int a, int dist);
EAPI void           evas_common_gradient_color_data_set    (RGBA_Gradient *gr, DATA32 *data, int len, int alpha_flags);
EAPI void           evas_common_gradient_alpha_data_set    (RGBA_Gradient *gr, DATA8 *adata, int len);
EAPI void           evas_common_gradient_type_set          (RGBA_Gradient *gr, const char *name, char *params);
EAPI void           evas_common_gradient_fill_set          (RGBA_Gradient *gr, int x, int y, int w, int h);
EAPI void           evas_common_gradient_fill_angle_set    (RGBA_Gradient *gr, float angle);
EAPI void           evas_common_gradient_fill_spread_set   (RGBA_Gradient *gr, int spread);
EAPI void           evas_common_gradient_map_angle_set     (RGBA_Gradient *gr, float angle);
EAPI void           evas_common_gradient_map_offset_set    (RGBA_Gradient *gr, float offset);
EAPI void           evas_common_gradient_map_direction_set (RGBA_Gradient *gr, int direction);
EAPI void           evas_common_gradient_map               (RGBA_Draw_Context *dc, RGBA_Gradient *gr, int len);
EAPI void           evas_common_gradient_draw              (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr);

EAPI RGBA_Gradient_Type *evas_common_gradient_geometer_get (const char *name);


#endif /* _EVAS_GRADIENT_H */
