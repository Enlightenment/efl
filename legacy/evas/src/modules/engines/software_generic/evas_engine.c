#include "evas_common.h"
#include "evas_private.h"

/*
 *****
 **
 ** ENGINE ROUTINES
 **
 *****
 */

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
   evas_common_rectangle_draw(surface, context, x, y, w, h);
   evas_common_cpu_end_opt();
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   evas_common_line_draw(surface, context, x1, y1, x2, y2);
   evas_common_cpu_end_opt();
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
   evas_common_polygon_draw(surface, context, polygon);
   evas_common_cpu_end_opt();
}

static void *
eng_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance)
{
   if (!gradient) gradient = evas_common_gradient_new();
   evas_common_gradient_color_add(gradient, r, g, b, a, distance);
   return gradient;
}

static void *
eng_gradient_colors_clear(void *data, void *context, void *gradient)
{
   evas_common_gradient_colors_clear(gradient);
   return gradient;
}

static void *
eng_gradient_data_set(void *data, void *context, void *gradient, void *map, int len, int has_alpha)
{
   if (!gradient)
     gradient = evas_common_gradient_new();
   evas_common_gradient_data_set(gradient, map, len, has_alpha);
   return gradient;
}

static void *
eng_gradient_data_unset(void *data, void *context, void *gradient)
{
   evas_common_gradient_data_unset(gradient);
   return gradient;
}

static void
eng_gradient_free(void *data, void *gradient)
{
   evas_common_gradient_free(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   evas_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_range_offset_set(void *data, void *gradient, float offset)
{
   evas_common_gradient_range_offset_set(gradient, offset);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name)
{
   evas_common_gradient_type_set(gradient, name);
}

static void
eng_gradient_type_params_set(void *data, void *gradient, char *params)
{
   evas_common_gradient_type_params_set(gradient, params);
}

static void *
eng_gradient_geometry_init(void *data, void *gradient, int spread)
{
   gradient = evas_common_gradient_geometry_init(gradient, spread);
   return gradient;
}

static int
eng_gradient_alpha_get(void *data, void *gradient, int spread, int op)
{
   return evas_common_gradient_has_alpha(gradient, spread, op);
}

static void
eng_gradient_map(void *data, void *context, void *gradient, int spread)
{
   evas_common_gradient_map(context, gradient, spread);
   evas_common_cpu_end_opt();
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle, int spread)
{
   evas_common_gradient_draw(surface, context, x, y, w, h, gradient, angle, spread);
   evas_common_cpu_end_opt();
}

static void *
eng_image_load(void *data, char *file, char *key, int *error)
{
   *error = 0;
   return evas_common_load_image_from_file(file, key);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   RGBA_Image *im;

   im = evas_common_image_new();
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	evas_common_image_free(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;
   im->image->data = image_data;
   im->image->no_free = 1;
   return im;
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
{
   RGBA_Image *im;

   im = evas_common_image_create(w, h);
   if (!im) return NULL;
   if (image_data)
     memcpy(im->image->data, image_data, w * h * sizeof(DATA32));
   return im;
}

static void
eng_image_free(void *data, void *image)
{
   evas_common_image_unref(image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   RGBA_Image *im;

   im = image;
   if (w) *w = im->image->w;
   if (h) *h = im->image->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   RGBA_Image *im, *im_old;

   im_old = image;
   im = evas_common_image_create(w, h);
   if (!im) return im_old;
   if (im_old)
     {
	evas_common_load_image_data_from_file(im_old);
	if (im_old->image->data)
	  {
	     evas_common_blit_rectangle(im_old, im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
	evas_common_image_unref(im_old);
     }
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   evas_common_image_dirty(image);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   RGBA_Image *im;

   im = image;
   evas_common_load_image_data_from_file(im);
   if (to_write)
     {
	if (im->references > 1)
	  {
	     RGBA_Image *im_new;

	     im_new = evas_common_image_create(im->image->w, im->image->h);
	     if (!im_new) return im;
	     evas_common_blit_rectangle(im, im_new, 0, 0, im->image->w, im->image->h, 0, 0);
	     evas_common_cpu_end_opt();
	     evas_common_image_unref(im);
	     im = im_new;
	  }
	else
	  evas_common_image_dirty(im);
     }
   *image_data = im->image->data;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   RGBA_Image *im;

   im = image;
   if (image_data != im->image->data)
     {
	int w, h;

	w = im->image->w;
	h = im->image->h;
	evas_common_image_unref(im);
	return eng_image_new_from_data(data, w, h, image_data);
     }
   return im;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   RGBA_Image *im;

   im = image;
   if (im->references > 1)
     {
	RGBA_Image *im_new;

	im_new = evas_common_image_create(im->image->w, im->image->h);
	if (!im_new) return im;
	evas_common_load_image_data_from_file(im);
	evas_common_blit_rectangle(im, im_new, 0, 0, im->image->w, im->image->h, 0, 0);
	evas_common_cpu_end_opt();
	evas_common_image_unref(im);
	im = im_new;
     }
   else
     evas_common_image_dirty(im);
   if (has_alpha)
     im->flags |= RGBA_IMAGE_HAS_ALPHA;
   else
     im->flags &= ~RGBA_IMAGE_HAS_ALPHA;
   return im;
}


static int
eng_image_alpha_get(void *data, void *image)
{
   RGBA_Image *im;

   im = image;
   if (im->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
   return 0;
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   evas_common_load_image_data_from_file(image);
   if (smooth)
     evas_common_scale_rgba_in_to_out_clip_smooth(image, surface, context,
				      src_x, src_y, src_w, src_h,
				      dst_x, dst_y, dst_w, dst_h);
   else
     evas_common_scale_rgba_in_to_out_clip_sample(image, surface, context,
				      src_x, src_y, src_w, src_h,
				      dst_x, dst_y, dst_w, dst_h);
   evas_common_cpu_end_opt();
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   RGBA_Image *im;

   im = image;
   return im->info.comment;
}

static char *
eng_image_format_get(void *data, void *image)
{
   return NULL;
}

static void
eng_image_cache_flush(void *data)
{
   int tmp_size;

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   evas_common_image_set_cache(bytes);
}

static int
eng_image_cache_get(void *data)
{
   return evas_common_image_get_cache();
}

static void *
eng_font_load(void *data, char *name, int size)
{
   return evas_common_font_load(name, size);
}

static void *
eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_load(name, size, fdata, fdata_size);
}

static void *
eng_font_add(void *data, void *font, char *name, int size)
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
eng_font_string_size_get(void *data, void *font, char *text, int *w, int *h)
{
   evas_common_font_query_size(font, text, w, h);
}

static int
eng_font_inset_get(void *data, void *font, char *text)
{
   return evas_common_font_query_inset(font, text);
}

static int
eng_font_h_advance_get(void *data, void *font, char *text)
{
   int h, v;

   evas_common_font_query_advance(font, text, &h, &v);
   return h;
}

static int
eng_font_v_advance_get(void *data, void *font, char *text)
{
   int h, v;

   evas_common_font_query_advance(font, text, &h, &v);
   return v;
}

static int
eng_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_coords(font, text, pos, cx, cy, cw, ch);
}

static int
eng_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_text_at_pos(font, text, x, y, cx, cy, cw, ch);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   if ((w == ow) && (h == oh))
     evas_common_font_draw(surface, context, font, x, y, text);
   else
     {
	/* create output surface size ow x oh and scale to w x h */
	RGBA_Draw_Context *dc, *dc_in;

	dc_in = context;
	dc = evas_common_draw_context_new();
	if (dc)
	  {
	     RGBA_Image *im;
	     int inset;

	     dc->col.col = dc_in->col.col;
	     inset = evas_common_font_query_inset( font, text);
	     im = evas_common_image_create(ow+inset, oh);
	     if (im)
	       {
		  int max_ascent;
		  int j;

		  im->flags |= RGBA_IMAGE_HAS_ALPHA;
		  j = (ow + inset) * oh;
		  memset(im->image->data, 0, j * sizeof(DATA32));

		  max_ascent = evas_common_font_max_ascent_get(font);

		  evas_common_font_draw(im, dc, font, 0, max_ascent, text);
		  evas_common_cpu_end_opt();
		  evas_common_scale_rgba_in_to_out_clip_smooth
		    (im, surface, context, inset, 0, ow, oh,
		     x + ((inset * w) / ow), y - ((max_ascent * h) / oh),
		     w, h);
		  evas_common_image_free(im);
	       }
	     evas_common_draw_context_free(dc);
	  }
     }
   evas_common_cpu_end_opt();
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
     eng_gradient_color_add,
     eng_gradient_colors_clear,
     eng_gradient_data_set,
     eng_gradient_data_unset,
     eng_gradient_free,
     eng_gradient_fill_set,
     eng_gradient_range_offset_set,
     eng_gradient_type_set,
     eng_gradient_type_params_set,
     eng_gradient_geometry_init,
     eng_gradient_alpha_get,
     eng_gradient_map,
     eng_gradient_draw,
     /* image draw funcs */
     eng_image_load,
     eng_image_new_from_data,
     eng_image_new_from_copied_data,
     eng_image_free,
     eng_image_size_get,
     eng_image_size_set,
     eng_image_dirty_region,
     eng_image_data_get,
     eng_image_data_put,
     eng_image_alpha_set,
     eng_image_alpha_get,
     eng_image_draw,
     eng_image_comment_get,
     eng_image_format_get,
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

int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&func);
   return 1;
}

void
module_close(void)
{
}

Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, 
     EVAS_MODULE_TYPE_ENGINE,
     "software_generic",
     "none"
};
