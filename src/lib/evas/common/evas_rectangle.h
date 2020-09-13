#ifndef _EVAS_RECTANGLE_H
#define _EVAS_RECTANGLE_H

typedef void (*Evas_Common_Rectangle_Draw_Cb)(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EVAS_API void	evas_common_rectangle_init	(void);
EVAS_API void	evas_common_rectangle_draw_cb	(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, Evas_Common_Rectangle_Draw_Cb cb);
EVAS_API void	evas_common_rectangle_draw	(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EVAS_API void evas_common_rectangle_draw_do(const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);
EVAS_API Eina_Bool evas_common_rectangle_draw_prepare(Cutout_Rects **reuse, const RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EVAS_API void evas_common_rectangle_rgba_draw       (RGBA_Image *dst, DATA32 color, int render_op, int x, int y, int w, int h, RGBA_Image *mask_ie, int mask_x, int mask_y);

#endif /* _EVAS_RECTANGLE_H */

