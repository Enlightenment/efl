#include "evas_common.h"/* Also includes international specific stuff */
#include "evas_common_soft16.h"

/*
 *****
 **
 ** ENGINE ROUTINES
 **
 *****
 */
int _evas_soft16_log_dom = -1;
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR( _evas_soft16_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_soft16_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_soft16_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_soft16_log_dom, __VA_ARGS__)

#ifdef CRIT
#undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_soft16_log_dom, __VA_ARGS__)

#define NOT_IMPLEMENTED()                                               \
  WRN("NOT_IMPLEMENTED: %s() at %s:%d",				\
           __FUNCTION__, __FILE__, __LINE__)

static void
eng_output_dump(void *data __UNUSED__)
{
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
}

static void *
eng_context_new(void *data __UNUSED__)
{
   return evas_common_draw_context_new();
}

static void
eng_context_free(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_free(context);
}

static void
eng_context_clip_set(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

static void
eng_context_clip_clip(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

static void
eng_context_clip_unset(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_unset_clip(context);
}

static int
eng_context_clip_get(void *data __UNUSED__, void *context, int *x, int *y, int *w, int *h)
{
   *x = ((RGBA_Draw_Context *)context)->clip.x;
   *y = ((RGBA_Draw_Context *)context)->clip.y;
   *w = ((RGBA_Draw_Context *)context)->clip.w;
   *h = ((RGBA_Draw_Context *)context)->clip.h;
   return ((RGBA_Draw_Context *)context)->clip.use;
}

static void
eng_context_color_set(void *data __UNUSED__, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_color(context, r, g, b, a);
}

static int
eng_context_color_get(void *data __UNUSED__, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->col.col));
   return 1;
}

static void
eng_context_multiplier_set(void *data __UNUSED__, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

static void
eng_context_multiplier_unset(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_unset_multiplier(context);
}

static int
eng_context_multiplier_get(void *data __UNUSED__, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   return ((RGBA_Draw_Context *)context)->mul.use;
}

static void
eng_context_cutout_add(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data __UNUSED__, void *context)
{
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_context_anti_alias_set(void *data __UNUSED__, void *context, unsigned char aa)
{
   evas_common_draw_context_set_anti_alias(context, aa);
}

static unsigned char
eng_context_anti_alias_get(void *data __UNUSED__, void *context)
{
   return ((RGBA_Draw_Context *)context)->anti_alias;
}

static void
eng_context_color_interpolation_set(void *data __UNUSED__, void *context, int color_space)
{
   evas_common_draw_context_set_color_interpolation(context, color_space);
}

static int
eng_context_color_interpolation_get(void *data __UNUSED__, void *context)
{
   return ((RGBA_Draw_Context *)context)->interpolation.color_space;
}

static void
eng_context_render_op_set(void *data __UNUSED__, void *context, int op)
{
   evas_common_draw_context_set_render_op(context, op);
}

static int
eng_context_render_op_get(void *data __UNUSED__, void *context)
{
   return ((RGBA_Draw_Context *)context)->render_op;
}



static void
eng_rectangle_draw(void *data __UNUSED__, void *context, void *surface, int x, int y, int w, int h)
{
   evas_common_soft16_rectangle_draw(surface, context, x, y, w, h);
}

static void
eng_line_draw(void *data __UNUSED__, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   evas_common_soft16_line_draw(surface, context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data __UNUSED__, void *context __UNUSED__, void *polygon, int x, int y)
{
   return evas_common_polygon_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data __UNUSED__, void *context __UNUSED__, void *polygon)
{
   return evas_common_polygon_points_clear(polygon);
}

static void
eng_polygon_draw(void *data __UNUSED__, void *context, void *surface, void *polygon, int x, int y)
{
   evas_common_soft16_polygon_draw(surface, context, polygon, x, y);
}

static int
eng_image_alpha_get(void *data __UNUSED__, void *image)
{
   Soft16_Image *im;

   if (!image) return 0;
   im = image;
   return im->cache_entry.flags.alpha;
}

static int
eng_image_colorspace_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return EVAS_COLORSPACE_RGB565_A5P;
}

static void *
eng_image_alpha_set(void *data __UNUSED__, void *image, int have_alpha)
{
   if (!image) return NULL;
   have_alpha = !!have_alpha;
   image = evas_common_soft16_image_alpha_set(image, have_alpha);
   return image;
}

static void *
eng_image_border_set(void *data __UNUSED__, void *image, int l __UNUSED__, int r __UNUSED__, int t __UNUSED__, int b __UNUSED__)
{
   return image;
}

static void
eng_image_border_get(void *data __UNUSED__, void *image __UNUSED__, int *l __UNUSED__, int *r __UNUSED__, int *t __UNUSED__, int *b __UNUSED__)
{
}

static char *
eng_image_comment_get(void *data __UNUSED__, void *image __UNUSED__, char *key __UNUSED__)
{
   return NULL;
}

static char *
eng_image_format_get(void *data __UNUSED__, void *image __UNUSED__)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void
eng_image_colorspace_set(void *data __UNUSED__, void *image __UNUSED__, int cspace __UNUSED__)
{
   NOT_IMPLEMENTED();
}

static void *
eng_image_native_set(void *data __UNUSED__, void *image __UNUSED__, void *native __UNUSED__)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void *
eng_image_native_get(void *data __UNUSED__, void *image __UNUSED__)
{
   NOT_IMPLEMENTED();
   return NULL;
}

static void *
eng_image_load(void *data __UNUSED__, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   return evas_cache_image_request(evas_common_soft16_image_cache_get(), file, key, lo, error);
}

static void *
eng_image_new_from_data(void *data __UNUSED__, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   if ((image_data) && (cspace != EVAS_COLORSPACE_RGB565_A5P))
     {
	WRN("Unsupported colorspace %d in %s() (%s:%d)",
		cspace, __FUNCTION__, __FILE__, __LINE__);
	return NULL;
     }
   return evas_cache_image_data(evas_common_soft16_image_cache_get(), w, h, image_data, alpha, EVAS_COLORSPACE_RGB565_A5P);
}

static void *
eng_image_new_from_copied_data(void *data __UNUSED__, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   if ((image_data) && (cspace != EVAS_COLORSPACE_RGB565_A5P))
     {
	WRN("Unsupported colorspace %d in %s() (%s:%d)",
		cspace, __FUNCTION__, __FILE__, __LINE__);
	return NULL;
     }
   return evas_cache_image_copied_data(evas_common_soft16_image_cache_get(), w, h, image_data, alpha, EVAS_COLORSPACE_RGB565_A5P);
}

static void
eng_image_free(void *data __UNUSED__, void *image)
{
   evas_cache_image_drop((Image_Entry *) image);
}

static void
eng_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
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
eng_image_size_set(void *data __UNUSED__, void *image, int w, int h)
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
eng_image_stride_get(void *data __UNUSED__, void *image, int *stride)
{
   Soft16_Image *im;

   if (stride) *stride = 0;
   if (!image) return;
   im = image;
   if (stride) *stride = im->stride;
}

static void *
eng_image_dirty_region(void *data __UNUSED__, void *image, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   /* FIXME: is this required? */
   //NOT_IMPLEMENTED();
   return image;
}

static void *
eng_image_data_get(void *data __UNUSED__, void *image, int to_write, DATA32 **image_data)
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
eng_image_data_put(void *data __UNUSED__, void *image, DATA32 *image_data)
{
   Soft16_Image *old_im, *new_im;

   if (!image) return NULL;

   old_im = image;
   if ((DATA16 *)image_data == old_im->pixels) return old_im;

   new_im = (Soft16_Image *) evas_cache_image_data(evas_common_soft16_image_cache_get(), old_im->cache_entry.w, old_im->cache_entry.h, image_data, old_im->cache_entry.flags.alpha, EVAS_COLORSPACE_RGB565_A5P);
   evas_cache_image_drop(&old_im->cache_entry);
   return new_im;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   Soft16_Image *im = image;

   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   Soft16_Image *im = image;

   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data __UNUSED__, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Soft16_Image *im;

   im = (Soft16_Image *) image;

   evas_cache_image_load_data(&im->cache_entry);
   evas_common_soft16_image_draw(im, surface, context,
		     src_x, src_y, src_w, src_h,
		     dst_x, dst_y, dst_w, dst_h,
		     smooth);
}

static void
eng_image_scale_hint_set(void *data __UNUSED__, void *image __UNUSED__, int hint __UNUSED__)
{
}

static int
eng_image_scale_hint_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return EVAS_IMAGE_SCALE_HINT_NONE;
}

static void
eng_image_cache_flush(void *data __UNUSED__)
{
   evas_cache_image_flush(evas_common_soft16_image_cache_get());
}

static void
eng_image_cache_set(void *data __UNUSED__, int bytes)
{
   evas_cache_image_set(evas_common_soft16_image_cache_get(), bytes);
}

static int
eng_image_cache_get(void *data __UNUSED__)
{
   return evas_cache_image_get(evas_common_soft16_image_cache_get());
}

static void *
eng_font_load(void *data __UNUSED__, const char *name, int size)
{
   return evas_common_font_load(name, size);
}

static void *
eng_font_memory_load(void *data __UNUSED__, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_load(name, size, fdata, fdata_size);
}

static void *
eng_font_add(void *data __UNUSED__, void *font, const char *name, int size)
{
   return evas_common_font_add(font, name, size);
}

static void *
eng_font_memory_add(void *data __UNUSED__, void *font, char *name, int size, const void *fdata, int fdata_size)
{
   return evas_common_font_memory_add(font, name, size, fdata, fdata_size);
}

static void
eng_font_free(void *data __UNUSED__, void *font)
{
   evas_common_font_free(font);
}

static int
eng_font_ascent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_ascent_get(font);
}

static int
eng_font_descent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_descent_get(font);
}

static int
eng_font_max_ascent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_max_ascent_get(font);
}

static int
eng_font_max_descent_get(void *data __UNUSED__, void *font)
{
   return evas_common_font_max_descent_get(font);
}

static void
eng_font_string_size_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props, int *w, int *h)
{
   evas_common_font_query_size(font, text_props, w, h);
}

static int
eng_font_inset_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props)
{
   return evas_common_font_query_inset(font, text_props);
}

static int
eng_font_right_inset_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props)
{
   return evas_common_font_query_right_inset(font, text_props);
}

static int
eng_font_h_advance_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props)
{
   int h, v;

   evas_common_font_query_advance(font, text_props, &h, &v);
   return h;
}

static int
eng_font_v_advance_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props)
{
   int h, v;

   evas_common_font_query_advance(font, text_props, &h, &v);
   return v;
}

static int
eng_font_pen_coords_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch)
{
   return evas_common_font_query_pen_coords(font, text_props, pos, cpen_x, cy, cadv, ch);
}

static Eina_Bool
eng_font_text_props_info_create(void *data __UNUSED__, void *font, Eina_Unicode *text, Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props, size_t pos, size_t len)
{
   (void) font;
   (void) text;
   (void) text_props;
   (void) par_props;
   (void) pos;
   (void) len;
#if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
   evas_bidi_shape_string(text, par_props, pos, len);
#endif
   return evas_common_text_props_content_create(font, text, text_props, len);
}

static int
eng_font_char_coords_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props, int pos, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_coords(font, text_props, pos, cx, cy, cw, ch);
}

static int
eng_font_char_at_coords_get(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_at_coords(font, text_props, x, y, cx, cy, cw, ch);
}

static int
eng_font_last_up_to_pos(void *data __UNUSED__, void *font, const Evas_Text_Props *text_props, int x, int y)
{
   return evas_common_font_query_last_up_to_pos(font, text_props, x, y);
}

static void
eng_font_draw(void *data __UNUSED__, void *context, void *surface, void *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const Eina_Unicode *text, const Evas_Text_Props *text_props)
{
   static RGBA_Image    *im = NULL;
   Soft16_Image         *dst = surface;

   if (!im)
     im = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
   evas_cache_image_surface_alloc(&im->cache_entry, dst->cache_entry.w, dst->cache_entry.h);
   evas_common_draw_context_font_ext_set(context,
					 surface,
					 evas_common_soft16_font_glyph_new,
					 evas_common_soft16_font_glyph_free,
					 evas_common_soft16_font_glyph_draw);
   evas_common_font_draw(im, context, font, x, y, text, text_props);
   evas_common_draw_context_font_ext_set(context,
					 NULL,
					 NULL,
					 NULL,
					 NULL);
}

static void
eng_font_cache_flush(void *data __UNUSED__)
{
   evas_common_font_flush();
}

static void
eng_font_cache_set(void *data __UNUSED__, int bytes)
{
   evas_common_font_cache_set(bytes);
}

static int
eng_font_cache_get(void *data __UNUSED__)
{
   return evas_common_font_cache_get();
}

static void
eng_font_hinting_set(void *data __UNUSED__, void *font, int hinting)
{
   evas_common_font_hinting_set(font, hinting);
}

static int
eng_font_hinting_can_hint(void *data __UNUSED__, int hinting)
{
   return evas_common_hinting_available(hinting);
}

static Eina_Bool
eng_canvas_alpha_get(void *data __UNUSED__, void *context __UNUSED__)
{
   return EINA_TRUE;
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
     eng_output_dump,
     /* draw context virtual methods */
     eng_context_new,
     eng_canvas_alpha_get,
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
     eng_image_data_preload_request,
     eng_image_data_preload_cancel,
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
     eng_font_hinting_can_hint,
     eng_image_scale_hint_set,
     eng_image_scale_hint_get,
     /* more font draw functions */
     eng_font_last_up_to_pos,
     NULL, //   ORD(image_map_draw);
     NULL, //   ORD(image_map_surface_new);
     NULL, //   ORD(image_map_surface_free);
     NULL, // eng_image_content_hint_set - software doesn't use it
     NULL, // eng_image_content_hint_get - software doesn't use it
     eng_font_pen_coords_get,
     eng_font_text_props_info_create,
     eng_font_right_inset_get
     /* FUTURE software generic calls go here */
};

/*
 *****
 **
 ** MODULE ACCESSIBLE API API
 **
 *****
 */

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_soft16_log_dom = eina_log_domain_register
     ("evas-software_16", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_soft16_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   em->functions = (void *)(&func);
   
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
   eina_log_domain_unregister(_evas_soft16_log_dom);
}

static Evas_Module_Api evas_modapi =
{
  EVAS_MODULE_API_VERSION,
  "software_16",
  "none",
  {
    module_open,
    module_close
  }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, software_16);

#ifndef EVAS_STATIC_BUILD_SOFTWARE_16
EVAS_EINA_MODULE_DEFINE(engine, software_16);
#endif
