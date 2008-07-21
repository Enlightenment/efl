#ifndef _EVAS_DRAW_H
#define _EVAS_DRAW_H


EAPI void               evas_common_draw_init                            (void);

EAPI RGBA_Draw_Context *evas_common_draw_context_new                     (void);
EAPI void               evas_common_draw_context_free                    (RGBA_Draw_Context *dc);
EAPI void               evas_common_draw_context_font_ext_set            (RGBA_Draw_Context *dc,
                                                                          void *data,
                                                                          void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg),
                                                                          void  (*gl_free) (void *ext_dat),
                                                                          void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y));
EAPI void               evas_common_draw_context_clip_clip               (RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI void               evas_common_draw_context_set_clip                (RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI void               evas_common_draw_context_unset_clip              (RGBA_Draw_Context *dc);
EAPI void               evas_common_draw_context_set_color               (RGBA_Draw_Context *dc, int r, int g, int b, int a);
EAPI void               evas_common_draw_context_set_multiplier          (RGBA_Draw_Context *dc, int r, int g, int b, int a);
EAPI void               evas_common_draw_context_unset_multiplier        (RGBA_Draw_Context *dc);
EAPI Cutout_Rects      *evas_common_draw_context_cutouts_new             (void);
EAPI void               evas_common_draw_context_cutouts_free            (Cutout_Rects* rects);
EAPI void               evas_common_draw_context_cutouts_del             (Cutout_Rects* rects, int index);
EAPI void               evas_common_draw_context_add_cutout              (RGBA_Draw_Context *dc, int x, int y, int w, int h);
EAPI void               evas_common_draw_context_clear_cutouts           (RGBA_Draw_Context *dc);
EAPI Cutout_Rects      *evas_common_draw_context_apply_cutouts           (RGBA_Draw_Context *dc);
EAPI void               evas_common_draw_context_apply_clear_cutouts     (Cutout_Rects* rects);
EAPI void               evas_common_draw_context_apply_clean_cutouts     (Cutout_Rects* rects);
EAPI void               evas_common_draw_context_set_anti_alias          (RGBA_Draw_Context *dc, unsigned char aa);
EAPI void               evas_common_draw_context_set_color_interpolation (RGBA_Draw_Context *dc, int color_space);
EAPI void               evas_common_draw_context_set_render_op           (RGBA_Draw_Context *dc, int op);
EAPI void               evas_common_draw_context_set_sli                 (RGBA_Draw_Context *dc, int y, int h);


#endif /* _EVAS_DRAW_H */
