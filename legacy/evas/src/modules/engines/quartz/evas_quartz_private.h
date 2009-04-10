#ifndef _EVAS_QUARTZ_PRIVATE_H_
#define _EVAS_QUARTZ_PRIVATE_H_

#include "evas_engine.h"

CG_EXTERN void CGContextResetClip (CGContextRef); // undocumented CoreGraphics function to clear clip rect/*

static inline void flip_pixels(int *y, int *h, void *re);

static void *eng_info(Evas *e);
static void eng_info_free(Evas *e, void *info);

static void *eng_output_setup(CGContextRef context, int w, int h);
static int eng_setup(Evas *e, void *in);

static void eng_output_free(void *data);
static void eng_output_resize(void *data, int w, int h);
static void eng_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void eng_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void eng_output_redraws_clear(void *data);
static void *eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h);
static void eng_output_flush(void *data);

static void *eng_context_new(void *data);
static void eng_context_free(void *data, void *context);
static void eng_context_clip_set(void *data, void *context, int x, int y, int w, int h);
static void eng_context_clip_unset(void *data, void *context);
static int eng_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h);
static void eng_context_color_set(void *data, void *context, int r, int g, int b, int a);
static int eng_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a);
static void eng_context_multiplier_set(void *data, void *context, int r, int g, int b, int a);
static void eng_context_multiplier_unset(void *data, void *context);
static int eng_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a);
static void eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h);
static void eng_context_cutout_clear(void *data, void *context);
static void eng_context_anti_alias_set(void *data, void *context, unsigned char aa);
static unsigned char eng_context_anti_alias_get(void *data, void *context);

static void eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h);

static void eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2);

static void *eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y);
static void *eng_polygon_points_clear(void *data, void *context, void *polygon);
static void eng_polygon_draw(void *data, void *context, void *surface, void *polygon);

static void *eng_gradient_new(void *data);
static void eng_gradient_free(void *data, void *gradient);
static void eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta);
static void eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta);
static void eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha);
static void eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len);
static void eng_gradient_clear(void *data, void *gradient);
static void eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h);
static void eng_gradient_fill_angle_set(void *data, void *gradient, double angle);
static void eng_gradient_fill_spread_set(void *data, void *gradient, int spread);
static void eng_gradient_angle_set(void *data, void *gradient, double angle);
static void eng_gradient_offset_set(void *data, void *gradient, float offset);
static void eng_gradient_direction_set(void *data, void *gradient, int direction);
static void eng_gradient_type_set(void *data, void *gradient, char *name, char *params);
static int eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h);
static int eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h);
static void eng_gradient_render_pre(void *data, void *context, void *gradient);
static void eng_gradient_render_post(void *data, void *gradient);
static void eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h);
static void *eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo);

static void *eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace);
static void *eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace);
static void eng_image_free(void *data, void *image);
static void eng_image_size_get(void *data, void *image, int *w, int *h);
static void *eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h);
static void *eng_image_alpha_set(void *data, void *image, int has_alpha);
static int eng_image_alpha_get(void *data, void *image);
static char *eng_image_comment_get(void *data, void *image, char *key);
static char *eng_image_format_get(void *data, void *image);
static void eng_image_colorspace_set(void *data, void *image, int cspace);
static int eng_image_colorspace_get(void *data, void *image);
static void eng_image_native_set(void *data, void *image, void *native);
static void *eng_image_native_get(void *data, void *image);
static void *eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data);
static void *eng_image_data_put(void *data, void *image, DATA32 *image_data);
static void *eng_image_size_set(void *data, void *image, int w, int h);
static void eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);

static void *eng_font_add(void *data, void *font, const char *name, int size);
static Evas_Quartz_Font *quartz_font_from_ats(ATSFontContainerRef container, int size);
static void *eng_font_load(void *data, const char *name, int size);
static void *eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size);
static void eng_font_free(void *data, void *font);
static int eng_font_ascent_get(void *data, void *font);
static int eng_font_descent_get(void *data, void *font);
static int eng_font_max_ascent_get(void *data, void *font);
static int eng_font_max_descent_get(void *data, void *font);
static void eng_font_string_size_get(void *data, void *font, const char *text, int *w, int *h);
static int eng_font_inset_get(void *data, void *font, const char *text);
static int eng_font_h_advance_get(void *data, void *font, const char *text);
static int eng_font_v_advance_get(void *data, void *font, const char *text);
static int eng_font_char_coords_get(void *data, void *font, const char *text, int pos, int *cx, int *cy, int *cw, int *ch);
static int eng_font_char_at_coords_get(void *data, void *font, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
static void eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text);
static void eng_font_hinting_set(void *data, void *font, int hinting);
static int eng_font_hinting_can_hint(void *data, int hinting);

EAPI int module_open(Evas_Module *em);
EAPI void module_close(void);

#endif /* _EVAS_QUARTZ_PRIVATE_H_ */
