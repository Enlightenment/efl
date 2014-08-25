#ifndef _EVAS_RECTANGLE_H
#define _EVAS_RECTANGLE_H

typedef void (*Evas_Common_Rectangle_Draw_Cb)(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EAPI void	evas_common_rectangle_init	(void);
EAPI void	evas_common_rectangle_draw_cb	(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, Evas_Common_Rectangle_Draw_Cb cb);
EAPI void	evas_common_rectangle_draw	(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EAPI void evas_common_rectangle_draw_do(const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI Eina_Bool evas_common_rectangle_draw_prepare(Cutout_Rects **reuse, const RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EAPI void evas_common_rectangle_rgba_draw       (RGBA_Image *dst, DATA32 color, int render_op, int x, int y, int w, int h);

#endif /* _EVAS_RECTANGLE_H */

