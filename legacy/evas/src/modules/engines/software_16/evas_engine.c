#include "evas_common.h"
#include "evas_private.h"
#include "evas_common_soft16.h"

/*
 *****
 **
 ** ENGINE ROUTINES
 **
 *****
 */

#define NOT_IMPLEMENTED()                                               \
   fprintf(stderr, "NOT_IMPLEMENTED: %s() at %s:%d\n",                  \
           __FUNCTION__, __FILE__, __LINE__)

static void *
eng_context_new(void *data)
{
   return evas_common_draw_context_new();
}

static void
eng_context_free(void *data, void *context)
{
   evas_common_draw_context_free(context);
}

static void
eng_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

static void
eng_context_clip_clip(void *data, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

static void
eng_context_clip_unset(void *data, void *context)
{
   evas_common_draw_context_unset_clip(context);
}

static int
eng_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
{
   *x = ((RGBA_Draw_Context *)context)->clip.x;
   *y = ((RGBA_Draw_Context *)context)->clip.y;
   *w = ((RGBA_Draw_Context *)context)->clip.w;
   *h = ((RGBA_Draw_Context *)context)->clip.h;
   return ((RGBA_Draw_Context *)context)->clip.use;
}

static void
eng_context_color_set(void *data, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_color(context, r, g, b, a);
}

static int
eng_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->col.col));
   return 1;
}

static void
eng_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

static void
eng_context_multiplier_unset(void *data, void *context)
{
   evas_common_draw_context_unset_multiplier(context);
}

static int
eng_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   return ((RGBA_Draw_Context *)context)->mul.use;
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_context_anti_alias_set(void *data, void *context, unsigned char aa)
{
   evas_common_draw_context_set_anti_alias(context, aa);
}

static unsigned char
eng_context_anti_alias_get(void *data, void *context)
{
   return ((RGBA_Draw_Context *)context)->anti_alias;
}

static void
eng_context_color_interpolation_set(void *data, void *context, int color_space)
{
   evas_common_draw_context_set_color_interpolation(context, color_space);
}

static int
eng_context_color_interpolation_get(void *data, void *context)
{
   return ((RGBA_Draw_Context *)context)->interpolation.color_space;
}

static void
eng_context_render_op_set(void *data, void *context, int op)
{
   evas_common_draw_context_set_render_op(context, op);
}

static int
eng_context_render_op_get(void *data, void *context)
{
   return ((RGBA_Draw_Context *)context)->render_op;
}



static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   soft16_rectangle_draw(surface, context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   soft16_line_draw(surface, context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   return evas_common_polygon_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   return evas_common_polygon_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   soft16_polygon_draw(surface, context, polygon);
}

static void *
eng_gradient_new(void *data)
{
   NOT_IMPLEMENTED();
   return NULL;
//   return evas_common_gradient_new();
}

static void
eng_gradient_free(void *data, void *gradient)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_free(gradient);
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_clear(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_map_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_map_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_map_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   NOT_IMPLEMENTED();
//   evas_common_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   NOT_IMPLEMENTED();
   return 0;
//   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
//   RGBA_Gradient *gr = (RGBA_Gradient *)gradient;
//
//   if (!dc || !gr || !gr->type.geometer)  return 0;
//   return !(gr->type.geometer->has_alpha(gr, dc->render_op) |
//              gr->type.geometer->has_mask(gr, dc->render_op));
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   NOT_IMPLEMENTED();
   return 0;
//   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
//
//   if (!dc || !gradient)  return 0;
//   return 1;
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
//   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;
//   RGBA_Gradient *gr = (RGBA_Gradient *)gradient;
//   int  len;
//
//   if (!dc || !gr || !gr->type.geometer)  return;
//   gr->type.geometer->geom_set(gr);
//   len = gr->type.geometer->get_map_len(gr);
//   evas_common_gradient_map(dc, gr, len);
   NOT_IMPLEMENTED();
}

static void
eng_gradient_render_post(void *data, void *gradient)
{
   NOT_IMPLEMENTED();
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
//   evas_common_gradient_draw(surface, context, x, y, w, h, gradient);
//   evas_common_cpu_end_opt();
   NOT_IMPLEMENTED();
}

static int
eng_image_alpha_get(void *data, void *image)
{
   Soft16_Image *im;

   if (!image) return 0;
   im = image;
   return im->cache_entry.flags.alpha;
}

static int
eng_image_colorspace_get(void *data, void *image)
{
   return EVAS_COLORSPACE_RGB565_A5P;
}

static void *
eng_image_alpha_set(void *data, void *image, int have_alpha)
{
   if (!image) return NULL;
   have_alpha = !!have_alpha;
   image = soft16_image_alpha_set(image, have_alpha);
   return image;
}

static void *
eng_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   return image;
}

static void
eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   return NULL;
}

static char *
eng_image_format_get(void *data, void *image)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   NOT_IMPLEMENTED();
}

static void
eng_image_native_set(void *data, void *image, void *native)
{
   NOT_IMPLEMENTED();
}

static void *
eng_image_native_get(void *data, void *image)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   return evas_cache_image_request(evas_common_soft16_image_cache_get(), file, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   if ((image_data) && (cspace != EVAS_COLORSPACE_RGB565_A5P))
     {
	fprintf(stderr, "Unsupported colorspace %d in %s() (%s:%d)\n",
		cspace, __FUNCTION__, __FILE__, __LINE__);
	return NULL;
     }
   return evas_cache_image_data(evas_common_soft16_image_cache_get(), w, h, image_data, alpha, EVAS_COLORSPACE_RGB565_A5P);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   if ((image_data) && (cspace != EVAS_COLORSPACE_RGB565_A5P))
     {
	fprintf(stderr, "Unsupported colorspace %d in %s() (%s:%d)\n",
		cspace, __FUNCTION__, __FILE__, __LINE__);
	return NULL;
     }
   return evas_cache_image_copied_data(evas_common_soft16_image_cache_get(), w, h, image_data, alpha, EVAS_COLORSPACE_RGB565_A5P);
}

static void
eng_image_free(void *data, void *image)
{
   evas_cache_image_drop((Image_Entry *) image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Soft16_Image *im;

   if (w) *w = 0;
   if (h) *h = 0;
   if (!image) return;
   im = image;
   if (w) *w = im->cache_entry.w;
   if (h) *h = im->cache_entry.h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   if (!image) return NULL;
   if ((w <= 0) || (h <= 0))
     {
        evas_cache_image_drop((Image_Entry *) image);
	return NULL;
     }
   return evas_cache_image_size_set((Image_Entry *) image, w, h);
}

static void
eng_image_stride_get(void *data, void *image, int *stride)
{
   Soft16_Image *im;

   if (stride) *stride = 0;
   if (!image) return;
   im = image;
   if (stride) *stride = im->stride;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   /* FIXME: is this required? */
   //NOT_IMPLEMENTED();
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Soft16_Image *im;

   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }

   im = image;
   evas_cache_image_load_data(&im->cache_entry);

   if (to_write)
     im = (Soft16_Image *) evas_cache_image_alone(&im->cache_entry);

   if (image_data) *image_data = (DATA32 *) im->pixels;

   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Soft16_Image *old_im, *new_im;

   if (!image) return NULL;

   old_im = image;
   if ((DATA16 *)image_data == old_im->pixels) return old_im;

   new_im = (Soft16_Image *) evas_cache_image_copied_data(evas_common_soft16_image_cache_get(), old_im->cache_entry.w, old_im->cache_entry.h, image_data, old_im->cache_entry.flags.alpha, EVAS_COLORSPACE_RGB565_A5P);
   evas_cache_image_drop(&old_im->cache_entry);
   return new_im;
}

#include <assert.h>

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Soft16_Image *im;

   im = (Soft16_Image *) image;

   evas_cache_image_load_data(&im->cache_entry);
   soft16_image_draw(im, surface, context,
		     src_x, src_y, src_w, src_h,
		     dst_x, dst_y, dst_w, dst_h,
		     smooth);
}

static void
eng_image_cache_flush(void *data)
{
   evas_cache_image_flush(evas_common_soft16_image_cache_get());
}

static void
eng_image_cache_set(void *data, int bytes)
{
   evas_cache_image_set(evas_common_soft16_image_cache_get(), bytes);
}

static int
eng_image_cache_get(void *data)
{
   return evas_cache_image_get(evas_common_soft16_image_cache_get());
}

static void *
eng_font_load(void *data, const char *name, int size)
{
   return evas_common_font_load(name, size);
}

static void *
eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_load(name, size, fdata, fdata_size);
}

static void *
eng_font_add(void *data, void *font, const char *name, int size)
{
   return evas_common_font_add(font, name, size);
}

static void *
eng_font_memory_add(void *data, void *font, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_add(font, name, size, fdata, fdata_size);
}

static void
eng_font_free(void *data, void *font)
{
   evas_common_font_free(font);
}

static int
eng_font_ascent_get(void *data, void *font)
{
   return evas_common_font_ascent_get(font);
}

static int
eng_font_descent_get(void *data, void *font)
{
   return evas_common_font_descent_get(font);
}

static int
eng_font_max_ascent_get(void *data, void *font)
{
   return evas_common_font_max_ascent_get(font);
}

static int
eng_font_max_descent_get(void *data, void *font)
{
   return evas_common_font_max_descent_get(font);
}

static void
eng_font_string_size_get(void *data, void *font, const char *text, int *w, int *h)
{
   evas_common_font_query_size(font, text, w, h);
}

static int
eng_font_inset_get(void *data, void *font, const char *text)
{
   return evas_common_font_query_inset(font, text);
}

static int
eng_font_h_advance_get(void *data, void *font, const char *text)
{
   int h, v;

   evas_common_font_query_advance(font, text, &h, &v);
   return h;
}

static int
eng_font_v_advance_get(void *data, void *font, const char *text)
{
   int h, v;

   evas_common_font_query_advance(font, text, &h, &v);
   return v;
}

static int
eng_font_char_coords_get(void *data, void *font, const char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_coords(font, text, pos, cx, cy, cw, ch);
}

static int
eng_font_char_at_coords_get(void *data, void *font, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_text_at_pos(font, text, x, y, cx, cy, cw, ch);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
   static RGBA_Image    *im = NULL;
   Soft16_Image         *dst = surface;

   if (!im)
     im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
   evas_cache_image_surface_alloc(&im->cache_entry, dst->cache_entry.w, dst->cache_entry.h);
   evas_common_draw_context_font_ext_set(context,
					 surface,
					 soft16_font_glyph_new,
					 soft16_font_glyph_free,
					 soft16_font_glyph_draw);
   evas_common_font_draw(im, context, font, x, y, text);
   evas_common_draw_context_font_ext_set(context,
					 NULL,
					 NULL,
					 NULL,
					 NULL);
}

static void
eng_font_cache_flush(void *data)
{
   evas_common_font_flush();
}

static void
eng_font_cache_set(void *data, int bytes)
{
   evas_common_font_cache_set(bytes);
}

static int
eng_font_cache_get(void *data)
{
   return evas_common_font_cache_get();
}

static void
eng_font_hinting_set(void *data, void *font, int hinting)
{
   evas_common_font_hinting_set(font, hinting);
}

static int
eng_font_hinting_can_hint(void *data, int hinting)
{
   return evas_common_hinting_available(hinting);
}


/*
 *****
 **
 ** ENGINE API
 **
 *****
 */

static Evas_Func func =
{
   NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     /* draw context virtual methods */
     eng_context_new,
     eng_context_free,
     eng_context_clip_set,
     eng_context_clip_clip,
     eng_context_clip_unset,
     eng_context_clip_get,
     eng_context_color_set,
     eng_context_color_get,
     eng_context_multiplier_set,
     eng_context_multiplier_unset,
     eng_context_multiplier_get,
     eng_context_cutout_add,
     eng_context_cutout_clear,
     eng_context_anti_alias_set,
     eng_context_anti_alias_get,
     eng_context_color_interpolation_set,
     eng_context_color_interpolation_get,
     eng_context_render_op_set,
     eng_context_render_op_get,
     /* rect draw funcs */
     eng_rectangle_draw,
     /* line draw funcs */
     eng_line_draw,
     /* polygon draw funcs */
     eng_polygon_point_add,
     eng_polygon_points_clear,
     eng_polygon_draw,
     /* gradient draw funcs */
     eng_gradient_new,
     eng_gradient_free,
     eng_gradient_color_stop_add,
     eng_gradient_alpha_stop_add,
     eng_gradient_color_data_set,
     eng_gradient_alpha_data_set,
     eng_gradient_clear,
     eng_gradient_fill_set,
     eng_gradient_fill_angle_set,
     eng_gradient_fill_spread_set,
     eng_gradient_angle_set,
     eng_gradient_offset_set,
     eng_gradient_direction_set,
     eng_gradient_type_set,
     eng_gradient_is_opaque,
     eng_gradient_is_visible,
     eng_gradient_render_pre,
     eng_gradient_render_post,
     eng_gradient_draw,
     /* image draw funcs */
     eng_image_load,
     eng_image_new_from_data,
     eng_image_new_from_copied_data,
     eng_image_free,
     eng_image_size_get,
     eng_image_size_set,
     eng_image_stride_get,
     eng_image_dirty_region,
     eng_image_data_get,
     eng_image_data_put,
     eng_image_alpha_set,
     eng_image_alpha_get,
     eng_image_border_set,
     eng_image_border_get,
     eng_image_draw,
     eng_image_comment_get,
     eng_image_format_get,
     eng_image_colorspace_set,
     eng_image_colorspace_get,
     eng_image_native_set,
     eng_image_native_get,
     /* image cache funcs */
     eng_image_cache_flush,
     eng_image_cache_set,
     eng_image_cache_get,
     /* font draw functions */
     eng_font_load,
     eng_font_memory_load,
     eng_font_add,
     eng_font_memory_add,
     eng_font_free,
     eng_font_ascent_get,
     eng_font_descent_get,
     eng_font_max_ascent_get,
     eng_font_max_descent_get,
     eng_font_string_size_get,
     eng_font_inset_get,
     eng_font_h_advance_get,
     eng_font_v_advance_get,
     eng_font_char_coords_get,
     eng_font_char_at_coords_get,
     eng_font_draw,
     /* font cache functions */
     eng_font_cache_flush,
     eng_font_cache_set,
     eng_font_cache_get,
     /* font hinting functions */
     eng_font_hinting_set,
     eng_font_hinting_can_hint
     /* FUTURE software generic calls go here */
};

/*
 *****
 **
 ** MODULE ACCESSIBLE API API
 **
 *****
 */

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&func);
   return 1;
}

EAPI void
module_close(void)
{
}

EAPI Evas_Module_Api evas_modapi =
{
  EVAS_MODULE_API_VERSION,
  EVAS_MODULE_TYPE_ENGINE,
  "software_16",
  "none"
};
