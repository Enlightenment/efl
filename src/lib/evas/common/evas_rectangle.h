#ifndef _EVAS_RECTANGLE_H
#define _EVAS_RECTANGLE_H


EAPI void           evas_common_rectangle_init          (void);

EAPI void           evas_common_rectangle_draw          (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EAPI void evas_common_rectangle_draw_do(const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI Eina_Bool evas_common_rectangle_draw_prepare(Cutout_Rects *reuse, const RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

#endif /* _EVAS_RECTANGLE_H */

