#ifndef _EVAS_LINE_H
#define _EVAS_LINE_H

typedef void (*Evas_Common_Line_Draw_Cb)(RGBA_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1);

EAPI void          evas_common_line_point_draw         (RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x, int y, RGBA_Image *mask_ie, int mask_x, int mask_y);

EAPI void          evas_common_line_init               (void);

EAPI void          evas_common_line_draw_line          (RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x1, int y1, int x2, int y2, RGBA_Image *mask_ie, int mask_x, int mask_y);
EAPI void          evas_common_line_draw_line_aa       (RGBA_Image *dst, int clip_x, int clip_y, int clip_w, int clip_h, DATA32 color, int render_op, int x1, int y1, int x2, int y2, RGBA_Image *mask_ie, int mask_x, int mask_y);
EAPI void          evas_common_line_draw_cb            (RGBA_Image *dst, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2, Evas_Common_Line_Draw_Cb cb);
EAPI void          evas_common_line_draw               (RGBA_Image *dst, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);


#endif /* _EVAS_LINE_H */

