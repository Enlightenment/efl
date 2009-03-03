/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_FONT_H
#define _EVAS_FONT_H


/* main */

EAPI void              evas_common_font_init                 (void);
EAPI void              evas_common_font_shutdown             (void);

EAPI int               evas_common_font_ascent_get           (RGBA_Font *fn);
EAPI int               evas_common_font_descent_get          (RGBA_Font *fn);
EAPI int               evas_common_font_max_ascent_get       (RGBA_Font *fn);
EAPI int               evas_common_font_max_descent_get      (RGBA_Font *fn);
EAPI int               evas_common_font_get_line_advance     (RGBA_Font *fn);

EAPI int               evas_common_font_utf8_get_next        (const unsigned char *buf, int *iindex);
EAPI int               evas_common_font_utf8_get_prev        (const unsigned char *buf, int *iindex);
EAPI int               evas_common_font_utf8_get_last        (const unsigned char *buf, int buflen);
EAPI int               evas_common_font_utf8_get_len         (const unsigned char *buf);

/* draw */

EAPI void              evas_common_font_draw                 (RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text);
EAPI int               evas_common_font_glyph_search         (RGBA_Font *fn, RGBA_Font_Int **fi_ret, int gl);
EAPI RGBA_Font_Glyph  *evas_common_font_int_cache_glyph_get  (RGBA_Font_Int *fi, FT_UInt index);

/* load */

EAPI RGBA_Font_Source *evas_common_font_source_memory_load   (const char *name, const void *data, int data_size);
EAPI RGBA_Font_Source *evas_common_font_source_load          (const char *name);
EAPI int               evas_common_font_source_load_complete (RGBA_Font_Source *fs);
EAPI RGBA_Font_Source *evas_common_font_source_find          (const char *name);
EAPI void              evas_common_font_source_free          (RGBA_Font_Source *fs);
EAPI void              evas_common_font_size_use             (RGBA_Font *fn);
EAPI RGBA_Font        *evas_common_font_memory_load          (const char *name, int size, const void *data, int data_size);
EAPI RGBA_Font_Int    *evas_common_font_int_load             (const char *name, int size);
EAPI RGBA_Font_Int    *evas_common_font_int_load_init        (RGBA_Font_Int *fn);
EAPI RGBA_Font_Int    *evas_common_font_int_load_complete    (RGBA_Font_Int *fi);
EAPI RGBA_Font        *evas_common_font_memory_load          (const char *name, int size, const void *data, int data_size);
EAPI RGBA_Font        *evas_common_font_load                 (const char *name, int size);
EAPI RGBA_Font        *evas_common_font_add                  (RGBA_Font *fn, const char *name, int size);
EAPI RGBA_Font        *evas_common_font_memory_add           (RGBA_Font *fn, const char *name, int size, const void *data, int data_size);
EAPI void              evas_common_font_free                 (RGBA_Font *fn);
EAPI void              evas_common_font_hinting_set          (RGBA_Font *fn, Font_Hint_Flags hinting);
EAPI Evas_Bool         evas_common_hinting_available         (Font_Hint_Flags hinting);
EAPI RGBA_Font        *evas_common_font_memory_hinting_load  (const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting);
EAPI RGBA_Font        *evas_common_font_hinting_load         (const char *name, int size, Font_Hint_Flags hinting);
EAPI RGBA_Font        *evas_common_font_hinting_add          (RGBA_Font *fn, const char *name, int size, Font_Hint_Flags hinting);
EAPI RGBA_Font        *evas_common_font_memory_hinting_add   (RGBA_Font *fn, const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting);
EAPI void              evas_common_font_int_modify_cache_by  (RGBA_Font_Int *fi, int dir);
EAPI int               evas_common_font_cache_get            (void);
EAPI void              evas_common_font_cache_set            (int size);
EAPI void              evas_common_font_flush                (void);
EAPI void              evas_common_font_flush_last           (void);
EAPI RGBA_Font_Int    *evas_common_font_int_find             (const char *name, int size);

/* query */

EAPI int               evas_common_font_query_kerning        (RGBA_Font_Int* fi, FT_UInt prev, FT_UInt index, int* kerning);
EAPI void              evas_common_font_query_size           (RGBA_Font *fn, const char *text, int *w, int *h);
EAPI int               evas_common_font_query_inset          (RGBA_Font *fn, const char *text);
EAPI void              evas_common_font_query_advance        (RGBA_Font *fn, const char *text, int *h_adv, int *v_adv);
EAPI int               evas_common_font_query_char_coords    (RGBA_Font *fn, const char *text, int pos, int *cx, int *cy, int *cw, int *ch);
EAPI int               evas_common_font_query_text_at_pos    (RGBA_Font *fn, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);


#endif /* _EVAS_FONT_H */
