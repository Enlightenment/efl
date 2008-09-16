/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <X11/Xlib.h>
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Cairo_X11.h"
#include "evas_cairo_common.h"

static void *eng_info(Evas *e);
static void eng_info_free(Evas *e, void *info);
static void eng_setup(Evas *e, void *info);
static void *eng_output_setup(int w, int h, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth);
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
static void eng_context_render_op_set(void *data, void *context, int op);
static int eng_context_render_op_get(void *data, void *context);

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

static void *eng_image_load(void *data, char *file, char *key, int *error, Evas_Image_Load_Opts *lo);
static void *eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace);
static void *eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace);
static void eng_image_free(void *data, void *image);
static void eng_image_size_get(void *data, void *image, int *w, int *h);
static void *eng_image_size_set(void *data, void *image, int w, int h);
static void *eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h);
static void *eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data);
static void *eng_image_data_put(void *data, void *image, DATA32 *image_data);
static void eng_image_data_preload_request(void *data, void *image, void *target);
static void eng_image_data_preload_cancel(void *data, void *image);
static void *eng_image_alpha_set(void *data, void *image, int has_alpha);
static int eng_image_alpha_get(void *data, void *image);
static void *eng_image_border_set(void *data, void *image, int l, int r, int t, int b);
static void eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b);
static void eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
static char *eng_image_comment_get(void *data, void *image, char *key);
static char *eng_image_format_get(void *data, void *image);
static void eng_image_colorspace_set(void *data, void *image, int cspace);
static int eng_image_colorspace_get(void *data, void *image);
static void eng_image_native_set(void *data, void *image, void *native);
static void *eng_image_native_get(void *data, void *image);
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

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_Cairo_X11_Window *win;
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
     eng_context_render_op_set,
     eng_context_render_op_get,
     /* rectangle draw funcs */
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
     NULL,
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
                              
     eng_font_hinting_set,
     eng_font_hinting_can_hint
};

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Cairo_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Cairo_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   printf("CAIRO: create info...\n");
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Cairo_X11 *in;

   in = (Evas_Engine_Info_Cairo_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Cairo_X11 *info;

   info = (Evas_Engine_Info_Cairo_X11 *)in;
   printf("CAIRO: setup info...\n");
   if (!e->engine.data.output)
     e->engine.data.output =
     eng_output_setup(e->output.w,
					e->output.h,
					info->info.display,
					info->info.drawable,
					info->info.visual,
					info->info.colormap,
					info->info.depth);
   if (!e->engine.data.output) return;

   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
   re = e->engine.data.output;
}

static void *
eng_output_setup(int w, int h, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth)
{
   Render_Engine *re;

   re = calloc(1, sizeof(Render_Engine));
   re->win = eng_window_new(disp, draw,
					      0 /* FIXME: screen 0 assumption */,
					      vis, cmap, depth, w, h);
   if (!re->win)
     {
	free(re);
	return NULL;
     }
   printf("CAIRO: cairo window setup done.\n");
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

   return re;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   eng_window_free(re->win);

   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->w = w;
   re->win->h = h;

   eng_window_size_set(re->win, w, h);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;
   /* FIXME? cairo_surface_mark_dirty_rectangle */

   re = (Render_Engine *)data;
   if (!re->win->draw.redraw)
     {
	re->win->draw.x1 = 0;
	re->win->draw.y1 = 0;
	re->win->draw.x2 = re->win->w - 1;
	re->win->draw.y2 = re->win->h - 1;
     }
   else
     {
	if (x < re->win->draw.x1) re->win->draw.x1 = x;
	if (y < re->win->draw.y1) re->win->draw.y1 = y;
	if ((x + w - 1) > re->win->draw.x2) re->win->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->win->draw.y2) re->win->draw.y2 = y + h - 1;
     }
   re->win->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re->win->draw.redraw) return NULL;

   if (x) *x = re->win->draw.x1;
   if (y) *y = re->win->draw.y1;
   if (w) *w = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (h) *h = re->win->draw.y2 - re->win->draw.y1 + 1;
   if (cx) *cx = re->win->draw.x1;
   if (cy) *cy = re->win->draw.y1;
   if (cw) *cw = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (ch) *ch = re->win->draw.y2 - re->win->draw.y1 + 1;
   return re;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   Evas_List *l;

   re = (Render_Engine *)data;
}

static void *
eng_context_new(void *data)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = calloc(1, sizeof(Evas_Cairo_Context));
   if (!ctxt) return NULL;

   ctxt->cairo = cairo_create(re->win->surface);

   return ctxt;
}

static void
eng_context_free(void *data, void *context)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = context;

   cairo_destroy(ctxt->cairo);
   free(ctxt);
}

static void
eng_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
eng_context_clip_clip(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
eng_context_clip_unset(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static int
eng_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
   return 0;
}

static void
eng_context_color_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;

   ctxt->col.r = (double)r / 255.0;
   ctxt->col.g = (double)g / 255.0;
   ctxt->col.b = (double)b / 255.0;
   ctxt->col.a = (double)a / 255.0;
}

static int
eng_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;

   if (r) *r = ctxt->col.r * 255;
   if (g) *g = ctxt->col.g * 255;
   if (b) *b = ctxt->col.b * 255;
   if (a) *a = ctxt->col.a * 255;
   return 1;
}

static void
eng_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;

   ctxt->mul.r = (double)r / 255.0;
   ctxt->mul.g = (double)g / 255.0;
   ctxt->mul.b = (double)b / 255.0;
   ctxt->mul.a = (double)a / 255.0;
   ctxt->mul.set = 1;
}

static void
eng_context_multiplier_unset(void *data, void *context)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;
   ctxt->mul.set = 0;
}

static int
eng_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;

   if (r) *r = ctxt->mul.r * 255;
   if (g) *g = ctxt->mul.g * 255;
   if (b) *b = ctxt->mul.b * 255;
   if (a) *a = ctxt->mul.a * 255;
   return ctxt->mul.set;
}

static void
eng_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
eng_context_anti_alias_set(void *data, void *context, unsigned char aa)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static unsigned char
eng_context_anti_alias_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return 1;
}

static void
eng_context_color_interpolation_set(void *data, void *context, int color_space)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
eng_context_color_interpolation_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return 0;
}

static void
eng_context_render_op_set(void *data, void *context, int op)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
eng_context_render_op_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return 0;
}


static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   double r, g, b, a;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;

   cairo_rectangle(ctxt->cairo, x, y, w, h);
   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;
   if (ctxt->mul.set)
     {
	r *= ctxt->mul.r;
	g *= ctxt->mul.g;
	b *= ctxt->mul.b;
	a *= ctxt->mul.a;
     }
   cairo_set_source_rgba(ctxt->cairo, r, g, b, a);
   cairo_fill(ctxt->cairo);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   double r, g, b, a;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;

   cairo_move_to(ctxt->cairo, x1, y1);
   cairo_line_to(ctxt->cairo, x2, y2);
   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;
   if (ctxt->mul.set)
     {
	r *= ctxt->mul.r;
	g *= ctxt->mul.g;
	b *= ctxt->mul.b;
	a *= ctxt->mul.a;
     }
   cairo_set_source_rgba(ctxt->cairo, r, g, b, a);
   cairo_stroke(ctxt->cairo);
}

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   Evas_Cairo_Polygon *poly;
   Evas_Cairo_Polygon_Point *pt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;
   poly = (Evas_Cairo_Polygon *)polygon;
   if (!poly) poly = calloc(1, sizeof(Evas_Cairo_Polygon));

   pt = malloc(sizeof(Evas_Cairo_Polygon_Point));
   if (pt)
     {
	pt->x = x;
	pt->y = y;
	poly->points = evas_list_append(poly->points, pt);
     }
   return poly;
}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   Evas_Cairo_Polygon *poly;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;
   poly = (Evas_Cairo_Polygon *)polygon;
   if (!poly) return NULL;

   while (poly->points)
     {
	free(poly->points->data);
	poly->points = evas_list_remove_list(poly->points, poly->points);
     }
   free(poly);

   return NULL;
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   Evas_Cairo_Polygon *poly;
   Evas_Cairo_Polygon_Point *pt;
   double r, g, b, a;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;
   poly = (Evas_Cairo_Polygon *)polygon;

   pt = poly->points->data;
   if (pt)
     {
	Evas_List *l;
	cairo_move_to(ctxt->cairo, pt->x, pt->y);
	for (l = poly->points->next; l; l = l->next)
	  {
	     pt = l->data;
	     cairo_line_to(ctxt->cairo, pt->x, pt->y);
	  }
     }
   r = ctxt->col.r;
   g = ctxt->col.g;
   b = ctxt->col.b;
   a = ctxt->col.a;
   if (ctxt->mul.set)
     {
	r *= ctxt->mul.r;
	g *= ctxt->mul.g;
	b *= ctxt->mul.b;
	a *= ctxt->mul.a;
     }
   cairo_set_source_rgba(ctxt->cairo, r, g, b, a);
   cairo_stroke(ctxt->cairo);
}

static void *
eng_gradient_new(void *data)
{
   return evas_common_gradient_new();
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   evas_common_gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   evas_common_gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   evas_common_gradient_clear(gradient);
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   evas_common_gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   evas_common_gradient_alpha_data_set(gradient, alpha_map, len);
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
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   evas_common_gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   evas_common_gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   evas_common_gradient_map_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   evas_common_gradient_map_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   evas_common_gradient_map_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   evas_common_gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   return 0;
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   return 0;
}

static void
eng_gradient_render_pre(void *data, void *context, void *gradient)
{
}

static void
eng_gradient_render_post(void *data, void *gradient)
{
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
}

static void *
eng_image_load(void *data, char *file, char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;
   
   re = (Render_Engine *)data;
   if (error) *error = 0;

   im = calloc(1, sizeof(Evas_Cairo_Image));
   if (!im) return NULL;

   im->im = evas_common_load_image_from_file(file, key, lo);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->references = 1;
   return im;
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;

   re = (Render_Engine *)data;

   /* FIXME, cache! */
   im = calloc(1, sizeof(Evas_Cairo_Image));
   if (!im) return NULL;

   im->im = evas_common_image_new();
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->im->image = evas_common_image_surface_new(im->im);
   if (!im->im->image)
     {
	evas_common_image_free(im->im);
	free(im);
	return NULL;
     }

   im->im->image->w = w;
   im->im->image->h = h;
   im->im->image->data = image_data;
   im->im->image->no_free = 1;
   im->references = 1;
   return im;
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;

   re = (Render_Engine *)data;

   /* FIXME, cache! */
   im = calloc(1, sizeof(Evas_Cairo_Image));
   if (!im) return NULL;
   im->references = 1;
   im->im = evas_common_image_create(w, h);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   if (image_data)
     memcpy(im->im->image->data, image_data, w * h * sizeof(DATA32));
   return im;
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;

   re = (Render_Engine *)data;
   im = (Evas_Cairo_Image *)image;
   evas_common_image_unref(im->im);
   if (im->surface) cairo_surface_destroy(im->surface);
   if (im->pattern) cairo_pattern_destroy(im->pattern);
   free(im);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image)
     {
	if (w) *w = 0;
	if (h) *h = 0;
     }
   else
     {
	Evas_Cairo_Image *im;

	im = image;
	if (w) *w = im->im->image->w;
	if (h) *h = im->im->image->h;
     }
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   *image_data = NULL;
   return image;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_data_preload_request(void *data, void *image, void *target)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_data_preload_cancel(void *data, void *image)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static int
eng_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;

   re = (Render_Engine *)data;
   if (!image) return 0;

   im = image;
   if (im->im->cache_entry.flags.alpha) return 1;
   return 0;
}

static void *
eng_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return;
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   Evas_Cairo_Image *im;
   DATA32 *pix;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;
   if (!image) return;

   im = image;
   evas_common_load_image_data_from_file(im->im);
   pix = im->im->image->data;
   if (pix)
     {
	if (!im->surface)
	  {
	     im->mulpix = malloc(im->im->image->w * im->im->image->h * sizeof(DATA32));
	     if (im->mulpix)
	       {
		  int i, n;
		  DATA32 *p;

		  n = im->im->image->w * im->im->image->h;
		  p = im->mulpix;
		  for (i = 0; i < n; i++)
		    {
		       int a;

		       a = A_VAL(pix);
		       R_VAL(p) = (R_VAL(pix) * a) / 255;
		       G_VAL(p) = (G_VAL(pix) * a) / 255;
		       B_VAL(p) = (B_VAL(pix) * a) / 255;
		       A_VAL(p) = a;
		       p++;
		       pix++;
		    }
		  im->surface = cairo_image_surface_create_for_data(im->mulpix,
								    CAIRO_FORMAT_ARGB32,
								    im->im->image->w,
								    im->im->image->h,
								    0);
		  im->pattern = cairo_pattern_create_for_surface(im->surface);
	       }
	  }

	if (smooth)
	  cairo_pattern_set_filter(im->pattern, CAIRO_FILTER_BILINEAR);
	else
	  cairo_pattern_set_filter(im->pattern, CAIRO_FILTER_NEAREST);
	cairo_save(ctxt->cairo);
	cairo_translate(ctxt->cairo, dst_x, dst_y);
	cairo_scale(ctxt->cairo,
		    (double)src_w / (double)dst_w,
		    (double)src_h / (double)dst_h);
	cairo_move_to(ctxt->cairo, 0, 0);
	//     cairo_set_rgb_color(re->win->cairo,
	//			  (double)(R_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0,
	//			  (double)(R_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0,
	//			  (double)(R_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0);
	//     cairo_set_alpha(re->win->cairo,
	//		     (double)(A_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0);
	cairo_set_source_surface(ctxt->cairo,
				 im->surface,
				 im->im->image->w,
				 im->im->image->h);
	cairo_paint(ctxt->cairo);
	cairo_restore(ctxt->cairo);
     }
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return NULL;
}

static char *
eng_image_format_get(void *data, void *image)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
}

static int
eng_image_colorspace_get(void *data, void *image)
{
   return EVAS_COLORSPACE_ARGB8888;
}

static void
eng_image_native_set(void *data, void *image, void *native)
{
}

static void *
eng_image_native_get(void *data, void *image)
{
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

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void *
eng_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void *
eng_font_add(void *data, void *font, char *name, int size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void *
eng_font_memory_add(void *data, void *font, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void
eng_font_free(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
eng_font_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_descent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_max_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_max_descent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static void
eng_font_string_size_get(void *data, void *font, char *text, int *w, int *h)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
eng_font_inset_get(void *data, void *font, char *text)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_h_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_v_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
eng_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static void
eng_font_cache_flush(void *data)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static void
eng_font_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
eng_font_cache_get(void *data)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static void
eng_font_hinting_set(void *data, void *font, int hinting)
{
   Render_Engine *re;
   
   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
eng_font_hinting_can_hint(void *data, int hinting)
{
   Render_Engine *re;
   
   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&eng_func);
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
     "cairo_x11",
     "none"
};
