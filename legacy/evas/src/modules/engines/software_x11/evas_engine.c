#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_X11.h"

static void *eng_info(Evas *e);
static void eng_info_free(Evas *e, void *info);
static void eng_setup(Evas *e, void *info);
static void *eng_output_setup(int w, int h, int rot, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth, int debug, int grayscale, int max_colors, Pixmap mask, int shape_dither);
static void eng_output_free(void *data);
static void eng_output_resize(void *data, int w, int h);
static void eng_output_tile_size_set(void *data, int w, int h);
static void eng_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void eng_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void eng_output_redraws_clear(void *data);
static void *eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h);
static void eng_output_flush(void *data);
static void *eng_context_new(void *data);
static void eng_context_free(void *data, void *context);
static void eng_context_clip_set(void *data, void *context, int x, int y, int w, int h);
static void eng_context_clip_clip(void *data, void *context, int x, int y, int w, int h);
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
static void eng_context_color_interpolation_set(void *data, void *context, int color_space);
static int eng_context_color_interpolation_get(void *data, void *context);
static void eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h);
static void eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2);
static void *eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y);
static void *eng_polygon_points_clear(void *data, void *context, void *polygon);
static void eng_polygon_draw(void *data, void *context, void *surface, void *polygon);
static void *eng_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance);
static void *eng_gradient_colors_clear(void *data, void *context, void *gradient);
static void eng_gradient_free(void *data, void *gradient);
static void eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h);
static void eng_gradient_type_set(void *data, void *gradient, char *name);
static void eng_gradient_type_params_set(void *data, void *gradient, char *params);
static void *eng_gradient_geometry_init(void *data, void *gradient, int spread);
static int  eng_gradient_alpha_get(void *data, void *gradient, int spread);
static void eng_gradient_map(void *data, void *context, void *gradient, int spread);
static void eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle, int spread);
static void *eng_image_load(void *data, char *file, char *key, int *error);
static void *eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data);
static void *eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data);
static void eng_image_free(void *data, void *image);
static void eng_image_size_get(void *data, void *image, int *w, int *h);
static void *eng_image_size_set(void *data, void *image, int w, int h);
static void *eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h);
static void *eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data);
static void *eng_image_data_put(void *data, void *image, DATA32 *image_data);
static void *eng_image_alpha_set(void *data, void *image, int has_alpha);
static int eng_image_alpha_get(void *data, void *image);
static void eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
static char *eng_image_comment_get(void *data, void *image, char *key);
static char *eng_image_format_get(void *data, void *image);
static void eng_image_cache_flush(void *data);
static void eng_image_cache_set(void *data, int bytes);
static int eng_image_cache_get(void *data);
static void *eng_font_load(void *data, char *name, int size);
static void *eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size);
static void *eng_font_add(void *data, void *font, char *name, int size);
static void *eng_font_memory_add(void *data, void *font, char *name, int size, const void *fdata, int fdata_size);
static void eng_font_free(void *data, void *font);
static int eng_font_ascent_get(void *data, void *font);
static int eng_font_descent_get(void *data, void *font);
static int eng_font_max_ascent_get(void *data, void *font);
static int eng_font_max_descent_get(void *data, void *font);
static void eng_font_string_size_get(void *data, void *font, char *text, int *w, int *h);
static int eng_font_inset_get(void *data, void *font, char *text);
static int eng_font_h_advance_get(void *data, void *font, char *text);
static int eng_font_v_advance_get(void *data, void *font, char *text);
static int eng_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch);
static int eng_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
static void eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text);
static void eng_font_cache_flush(void *data);
static void eng_font_cache_set(void *data, int bytes);
static int eng_font_cache_get(void *data);

static void eng_font_hinting_set(void *data, void *font, int hinting);
static int eng_font_hinting_can_hint(void *data, int hinting);

static Visual *eng_best_visual_get(Display *disp, int screen);
static Colormap eng_best_colormap_get(Display *disp, int screen);
static int eng_best_depth_get(Display *disp, int screen);
static Evas_Performance *eng_output_perf_new(Evas *e, Display *disp, Visual *vis, Colormap cmap, Drawable draw, int depth);
static Evas_Performance *eng_output_perf_test(Evas *e, Display *disp, Visual *vis, Colormap cmap, Drawable draw, int depth);
static char *eng_output_perf_data(Evas_Performance *perf);
static char *eng_output_perf_key(Evas_Performance *perf);
static void eng_output_perf_free(Evas_Performance *perf);
static void eng_output_perf_build(Evas_Performance *perf, const char *data);
static void eng_output_perf_device_store(Evas_Performance *perf);

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Tilebuf          *tb;
   Outbuf           *ob;
   Tilebuf_Rect     *rects;
   Evas_Object_List *cur_rect;
   int               end : 1;
};

static Evas_Func eng_func =
{
   eng_info,
     eng_info_free,
     eng_setup,
     eng_output_free,
     eng_output_resize,
     eng_output_tile_size_set,
     eng_output_redraws_rect_add,
     eng_output_redraws_rect_del,
     eng_output_redraws_clear,
     eng_output_redraws_next_update_get,
     eng_output_redraws_next_update_push,
     eng_output_flush,
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
     /* rectangle draw funcs */
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
     eng_gradient_free,
     eng_gradient_fill_set,
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

     eng_font_hinting_set,
     eng_font_hinting_can_hint
};

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Software_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Software_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->info.debug = 0;
   info->info.alloc_grayscale = 0;
   info->info.alloc_colors_max = 216;
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   info->func.performance_test = eng_output_perf_test;
   info->func.performance_free = eng_output_perf_free;
   info->func.performance_data_get = eng_output_perf_data;
   info->func.performance_key_get = eng_output_perf_key;
   info->func.performance_new = eng_output_perf_new;
   info->func.performance_build = eng_output_perf_build;
   info->func.performance_device_store = eng_output_perf_device_store;
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Software_X11 *in;

   in = (Evas_Engine_Info_Software_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Software_X11 *info;

   info = (Evas_Engine_Info_Software_X11 *)in;
   if (!e->engine.data.output)
     e->engine.data.output =
     eng_output_setup(e->output.w,
					   e->output.h,
					   info->info.rotation,
					   info->info.display,
					   info->info.drawable,
					   info->info.visual,
					   info->info.colormap,
					   info->info.depth,
					   info->info.debug,
					   info->info.alloc_grayscale,
					   info->info.alloc_colors_max,
					   info->info.mask,
					   info->info.shape_dither);
   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);

   re = e->engine.data.output;
   evas_software_x11_outbuf_drawable_set(re->ob, info->info.drawable);
   evas_software_x11_outbuf_mask_set(re->ob, info->info.mask);
   evas_software_x11_outbuf_rotation_set(re->ob, info->info.rotation);
}

static void *
eng_output_setup(int w, int h, int rot, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth, int debug, int grayscale, int max_colors, Pixmap mask, int shape_dither)
{
   Render_Engine *re;
   Outbuf_Perf *perf;

   re = calloc(1, sizeof(Render_Engine));
   /* if we haven't initialized - init (automatic abort if already done) */
   evas_common_cpu_init();

   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
   evas_common_rectangle_init();
   evas_common_gradient_init();
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();

   evas_software_x11_x_init();
   evas_software_x11_x_color_init();
   evas_software_x11_outbuf_init();

   /* get any stored performance metrics from device (xserver) */
   perf = evas_software_x11_outbuf_perf_restore_x(disp, draw, vis, cmap, depth);
   re->ob = evas_software_x11_outbuf_setup_x(w, h, rot, OUTBUF_DEPTH_INHERIT, disp, draw, vis, cmap, depth, perf, grayscale, max_colors, mask, shape_dither);
   if (!re->ob)
     {
	evas_software_x11_outbuf_perf_free(perf);
	free(re);
	return NULL;
     }
   evas_software_x11_outbuf_debug_set(re->ob, debug);
   re->tb = evas_common_tilebuf_new(w, h);
   if (!re->tb)
     {
	evas_software_x11_outbuf_free(re->ob);
	free(re);
	return NULL;
     }
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   return re;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_x11_outbuf_free(re->ob);
   evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_x11_outbuf_reconfigure(re->ob, w, h,
				   evas_software_x11_outbuf_get_rot(re->ob),
				   OUTBUF_DEPTH_INHERIT);
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   RGBA_Image *surface;
   Tilebuf_Rect *rect;
   int ux, uy, uw, uh;

   re = (Render_Engine *)data;
   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = (Evas_Object_List *)re->rects;
     }
   if (!re->cur_rect) return NULL;
   rect = (Tilebuf_Rect *)re->cur_rect;
   ux = rect->x; uy = rect->y; uw = rect->w; uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }

   surface = evas_software_x11_outbuf_new_region_for_update
     (re->ob, ux, uy, uw, uh, cx, cy, cw, ch);
   *x = ux; *y = uy; *w = uw; *h = uh;
   return surface;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_x11_outbuf_push_updated_region(re->ob, surface, x, y, w, h);
   evas_software_x11_outbuf_free_region_for_update(re->ob, surface);
   evas_common_cpu_end_opt();
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_x11_outbuf_flush(re->ob);
}

static void *
eng_context_new(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_draw_context_new();
}

static void
eng_context_free(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_free(context);
}

static void
eng_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

static void
eng_context_clip_clip(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

static void
eng_context_clip_unset(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_unset_clip(context);
}

static int
eng_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *x = ((RGBA_Draw_Context *)context)->clip.x;
   *y = ((RGBA_Draw_Context *)context)->clip.y;
   *w = ((RGBA_Draw_Context *)context)->clip.w;
   *h = ((RGBA_Draw_Context *)context)->clip.h;
   return ((RGBA_Draw_Context *)context)->clip.use;
}

static void
eng_context_color_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_set_color(context, r, g, b, a);
}

static int
eng_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->col.col));
   return 1;
}

static void
eng_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

static void
eng_context_multiplier_unset(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_unset_multiplier(context);
}

static int
eng_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   return ((RGBA_Draw_Context *)context)->mul.use;
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_context_anti_alias_set(void *data, void *context, unsigned char aa)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_set_anti_alias(context, aa);
}

static unsigned char
eng_context_anti_alias_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return ((RGBA_Draw_Context *)context)->anti_alias;
}

static void
eng_context_color_interpolation_set(void *data, void *context, int color_space)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_set_color_interpolation(context, color_space);
}

static int
eng_context_color_interpolation_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return ((RGBA_Draw_Context *)context)->interpolation.color_space;
}






static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifdef IMGONLY
   return;
#endif
   evas_common_rectangle_draw(surface, context, x, y, w, h);
   evas_common_cpu_end_opt();
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifdef IMGONLY
   return;
#endif
   evas_common_line_draw(surface, context, x1, y1, x2, y2);
   evas_common_cpu_end_opt();
}

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_polygon_point_add(polygon, x, y);
   context = NULL;
}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_polygon_points_clear(polygon);
   context = NULL;
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifdef IMGONLY
   return;
#endif
   evas_common_polygon_draw(surface, context, polygon);
   evas_common_cpu_end_opt();
}

static void *
eng_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!gradient)
     gradient = evas_common_gradient_new();
   evas_common_gradient_color_add(gradient, r, g, b, a, distance);
   return gradient;
   context = NULL;
}

static void *
eng_gradient_colors_clear(void *data, void *context, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_gradient_colors_clear(gradient);
   return gradient;
   context = NULL;
}

static void
eng_gradient_free(void *data, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_gradient_free(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_gradient_type_set(gradient, name);
}

static void
eng_gradient_type_params_set(void *data, void *gradient, char *params)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_gradient_type_params_set(gradient, params);
}

static void *
eng_gradient_geometry_init(void *data, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   gradient = evas_common_gradient_geometry_init(gradient, spread);
   return gradient;
}

static int
eng_gradient_alpha_get(void *data, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_gradient_has_alpha(gradient, spread);
}

static void
eng_gradient_map(void *data, void *context, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_gradient_map(context, gradient, spread);
   evas_common_cpu_end_opt();
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifdef IMGONLY
   return;
#endif
   evas_common_gradient_draw(surface, context, x, y, w, h, gradient, angle, spread);
   evas_common_cpu_end_opt();
}

static void *
eng_image_load(void *data, char *file, char *key, int *error)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = 0;
   return evas_common_load_image_from_file(file, key);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
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
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
   im = evas_common_image_create(w, h);
   if (!im) return NULL;
   if (image_data)
     memcpy(im->image->data, image_data, w * h * sizeof(DATA32));
   return im;
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_unref(image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
   im = image;
   if (w) *w = im->image->w;
   if (h) *h = im->image->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   RGBA_Image *im, *im_old;

   re = (Render_Engine *)data;
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
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_dirty(image);
   return image;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
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
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
   im = image;
   if (image_data != im->image->data)
     {
	int w, h;

	w = im->image->w;
	h = im->image->h;
	evas_common_image_unref(im);
	return eng_image_new_from_data(data, w, h, image_data);
     }
   /* hmmm - but if we wrote... why bother? */
/*   evas_common_image_dirty(im); */
   return im;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
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
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
   im = image;
   if (im->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
   return 0;
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
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
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
   im = image;
   return im->info.comment;
   key = 0;
}

static char *
eng_image_format_get(void *data, void *image)
{
   Render_Engine *re;
   RGBA_Image *im;

   re = (Render_Engine *)data;
   im = image;
//   if (im->info.format == 1) return "png";
   return NULL;
}

static void
eng_image_cache_flush(void *data)
{
   Render_Engine *re;
   int tmp_size;

   re = (Render_Engine *)data;
   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_set_cache(bytes);
}

static int
eng_image_cache_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_image_get_cache();
}

static void *
eng_font_load(void *data, char *name, int size)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_load(name, size);
}

static void *
eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_memory_load(name, size, fdata, fdata_size);
}

static void *
eng_font_add(void *data, void *font, char *name, int size)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_add(font, name, size);
}

static void *
eng_font_memory_add(void *data, void *font, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_memory_add(font, name, size, fdata, fdata_size);
}

static void
eng_font_free(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_free(font);
}

static int
eng_font_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_ascent_get(font);
}

static int
eng_font_descent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_descent_get(font);
}

static int
eng_font_max_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_max_ascent_get(font);
}

static int
eng_font_max_descent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_max_descent_get(font);
}

static void
eng_font_string_size_get(void *data, void *font, char *text, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_query_size(font, text, w, h);
}

static int
eng_font_inset_get(void *data, void *font, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_query_inset(font, text);
}

static int
eng_font_h_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;

   re = (Render_Engine *)data;
   evas_common_font_query_advance(font, text, &h, &v);
   return h;
}

static int
eng_font_v_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;

   re = (Render_Engine *)data;
   evas_common_font_query_advance(font, text, &h, &v);
   return v;
}

static int
eng_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_query_char_coords(font, text, pos, cx, cy, cw, ch);
}

static int
eng_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_query_text_at_pos(font, text, x, y, cx, cy, cw, ch);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
#ifdef IMGONLY
   return;
#endif
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
		  j = (ow+inset) * oh;
		  memset(im->image->data, 0, j * sizeof(DATA32));

		  max_ascent = evas_common_font_max_ascent_get(font);

		  evas_common_font_draw(im, dc, font, 0, max_ascent, text);
		  evas_common_cpu_end_opt();
		  evas_common_scale_rgba_in_to_out_clip_smooth(im, surface, context,
						   inset, 0, ow, oh,
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
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_flush();
}

static void
eng_font_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_cache_set(bytes);
}

static int
eng_font_cache_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_cache_get();
}

static void
eng_font_hinting_set(void *data, void *font, int hinting)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_font_hinting_set(font, hinting);
}

static int
eng_font_hinting_can_hint(void *data, int hinting)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return evas_common_hinting_available(hinting);
}









/* private engine functions the calling prog can use */

static Visual *
eng_best_visual_get(Display *disp, int screen)
{
   if (!disp) return NULL;
   return DefaultVisual(disp, screen);
}

static Colormap
eng_best_colormap_get(Display *disp, int screen)
{
   if (!disp) return 0;
   return DefaultColormap(disp, screen);
}

static int
eng_best_depth_get(Display *disp, int screen)
{
   if (!disp) return 0;
   return DefaultDepth(disp, screen);
}

static Evas_Performance *
eng_output_perf_new(Evas *e, Display *disp, Visual *vis, Colormap cmap, Drawable draw, int depth)
{
   return evas_software_x11_outbuf_perf_new_x(disp, draw, vis, cmap, depth);
   e = NULL;
}

static Evas_Performance *
eng_output_perf_test(Evas *e, Display *disp, Visual *vis, Colormap cmap, Drawable draw, int depth)
{
   return evas_software_x11_outbuf_perf_x(disp, draw, vis, cmap, depth);
   e = NULL;
}

static char *
eng_output_perf_data(Evas_Performance *perf)
{
   return evas_software_x11_outbuf_perf_serialize_x(perf);
}

static char *
eng_output_perf_key(Evas_Performance *perf)
{
   return evas_software_x11_outbuf_perf_serialize_info_x(perf);
}

static void
eng_output_perf_free(Evas_Performance *perf)
{
   evas_software_x11_outbuf_perf_free(perf);
}

static void
eng_output_perf_build(Evas_Performance *perf, const char *data)
{
   evas_software_x11_outbuf_perf_deserialize_x(perf, data);
}

static void
eng_output_perf_device_store(Evas_Performance *perf)
{
   evas_software_x11_outbuf_perf_store_x(perf);
}

int module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&eng_func);
   return 1;
}

void module_close(void)
{

}

Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, 
     EVAS_MODULE_TYPE_ENGINE,
     "software_x11",
     "none"
};

