#ifndef _EVAS_FONT_DRAW_H
#define _EVAS_FONT_DRAW_H

#include "../include/evas_common_private.h"

/* draw */
typedef Eina_Bool (*Evas_Common_Font_Draw_Cb)(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, int ext_h, int im_w, int im_h);

EVAS_API Eina_Bool         evas_common_font_draw_cb              (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs, Evas_Common_Font_Draw_Cb cb);
EVAS_API void              evas_common_font_draw                 (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs);
EVAS_API Eina_Bool         evas_common_font_rgba_draw            (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, int ext_h, int im_w, int im_h);
EVAS_API void              evas_common_font_draw_init            (void);
EVAS_API void              evas_common_font_draw_prepare         (Evas_Text_Props *text_props);
EVAS_API void              evas_common_font_draw_do              (const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Gfx_Func func, RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, const Evas_Text_Props *text_props);
EVAS_API Eina_Bool         evas_common_font_draw_prepare_cutout  (Cutout_Rects **reuse, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Gfx_Func *func);
EVAS_API void              evas_common_font_glyph_draw           (RGBA_Font_Glyph *fg, RGBA_Draw_Context *dc, RGBA_Image *dst, int dst_pitch, int dx, int dy, int dw, int dh, int cx, int cy, int cw, int ch);

#endif /* _EVAS_FONT_DRAW_H */
