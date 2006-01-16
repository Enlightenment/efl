/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "evas_engine_api_cairo_x11.h"
#include "Evas_Engine_Cairo_X11.h"
#include "evas_cairo_common.h"

static void *evas_engine_cairo_x11_info(Evas *e);
static void evas_engine_cairo_x11_info_free(Evas *e, void *info);
static void evas_engine_cairo_x11_setup(Evas *e, void *info);
static void *evas_engine_cairo_x11_output_setup(int w, int h, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth);
static void evas_engine_cairo_x11_output_free(void *data);
static void evas_engine_cairo_x11_output_resize(void *data, int w, int h);
static void evas_engine_cairo_x11_output_tile_size_set(void *data, int w, int h);
static void evas_engine_cairo_x11_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void evas_engine_cairo_x11_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void evas_engine_cairo_x11_output_redraws_clear(void *data);
static void *evas_engine_cairo_x11_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void evas_engine_cairo_x11_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h);
static void evas_engine_cairo_x11_output_flush(void *data);
static void *evas_engine_cairo_x11_context_new(void *data);
static void evas_engine_cairo_x11_context_free(void *data, void *context);
static void evas_engine_cairo_x11_context_clip_set(void *data, void *context, int x, int y, int w, int h);
static void evas_engine_cairo_x11_context_clip_clip(void *data, void *context, int x, int y, int w, int h);
static void evas_engine_cairo_x11_context_clip_unset(void *data, void *context);
static int evas_engine_cairo_x11_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h);
static void evas_engine_cairo_x11_context_color_set(void *data, void *context, int r, int g, int b, int a);
static int evas_engine_cairo_x11_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a);
static void evas_engine_cairo_x11_context_multiplier_set(void *data, void *context, int r, int g, int b, int a);
static void evas_engine_cairo_x11_context_multiplier_unset(void *data, void *context);
static int evas_engine_cairo_x11_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a);
static void evas_engine_cairo_x11_context_cutout_add(void *data, void *context, int x, int y, int w, int h);
static void evas_engine_cairo_x11_context_cutout_clear(void *data, void *context);
static void evas_engine_cairo_x11_context_anti_alias_set(void *data, void *context, unsigned char aa);
static unsigned char evas_engine_cairo_x11_context_anti_alias_get(void *data, void *context);
static void evas_engine_cairo_x11_context_color_interpolation_set(void *data, void *context, int color_space);
static int evas_engine_cairo_x11_context_color_interpolation_get(void *data, void *context);
static void evas_engine_cairo_x11_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h);
static void evas_engine_cairo_x11_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2);
static void *evas_engine_cairo_x11_polygon_point_add(void *data, void *context, void *polygon, int x, int y);
static void *evas_engine_cairo_x11_polygon_points_clear(void *data, void *context, void *polygon);
static void evas_engine_cairo_x11_polygon_draw(void *data, void *context, void *surface, void *polygon);
static void *evas_engine_cairo_x11_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance);
static void *evas_engine_cairo_x11_gradient_colors_clear(void *data, void *context, void *gradient);
static void evas_engine_cairo_x11_gradient_free(void *data, void *gradient);
static void evas_engine_cairo_x11_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h);
static void evas_engine_cairo_x11_gradient_type_set(void *data, void *gradient, char *name);
static void evas_engine_cairo_x11_gradient_type_params_set(void *data, void *gradient, char *params);
static void *evas_engine_cairo_x11_gradient_geometry_init(void *data, void *gradient, int spread);
static int  evas_engine_cairo_x11_gradient_alpha_get(void *data, void *gradient, int spread);
static void evas_engine_cairo_x11_gradient_map(void *data, void *context, void *gradient, int spread);
static void evas_engine_cairo_x11_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle, int spread);
static void *evas_engine_cairo_x11_image_load(void *data, char *file, char *key, int *error);
static void *evas_engine_cairo_x11_image_new_from_data(void *data, int w, int h, DATA32 *image_data);
static void *evas_engine_cairo_x11_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data);
static void evas_engine_cairo_x11_image_free(void *data, void *image);
static void evas_engine_cairo_x11_image_size_get(void *data, void *image, int *w, int *h);
static void *evas_engine_cairo_x11_image_size_set(void *data, void *image, int w, int h);
static void *evas_engine_cairo_x11_image_dirty_region(void *data, void *image, int x, int y, int w, int h);
static void *evas_engine_cairo_x11_image_data_get(void *data, void *image, int to_write, DATA32 **image_data);
static void *evas_engine_cairo_x11_image_data_put(void *data, void *image, DATA32 *image_data);
static void *evas_engine_cairo_x11_image_alpha_set(void *data, void *image, int has_alpha);
static int evas_engine_cairo_x11_image_alpha_get(void *data, void *image);
static void evas_engine_cairo_x11_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
static char *evas_engine_cairo_x11_image_comment_get(void *data, void *image, char *key);
static char *evas_engine_cairo_x11_image_format_get(void *data, void *image);
static void evas_engine_cairo_x11_image_cache_flush(void *data);
static void evas_engine_cairo_x11_image_cache_set(void *data, int bytes);
static int evas_engine_cairo_x11_image_cache_get(void *data);
static void *evas_engine_cairo_x11_font_load(void *data, char *name, int size);
static void *evas_engine_cairo_x11_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size);
static void *evas_engine_cairo_x11_font_add(void *data, void *font, char *name, int size);
static void *evas_engine_cairo_x11_font_memory_add(void *data, void *font, char *name, int size, const void *fdata, int fdata_size);
static void evas_engine_cairo_x11_font_free(void *data, void *font);
static int evas_engine_cairo_x11_font_ascent_get(void *data, void *font);
static int evas_engine_cairo_x11_font_descent_get(void *data, void *font);
static int evas_engine_cairo_x11_font_max_ascent_get(void *data, void *font);
static int evas_engine_cairo_x11_font_max_descent_get(void *data, void *font);
static void evas_engine_cairo_x11_font_string_size_get(void *data, void *font, char *text, int *w, int *h);
static int evas_engine_cairo_x11_font_inset_get(void *data, void *font, char *text);
static int evas_engine_cairo_x11_font_h_advance_get(void *data, void *font, char *text);
static int evas_engine_cairo_x11_font_v_advance_get(void *data, void *font, char *text);
static int evas_engine_cairo_x11_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch);
static int evas_engine_cairo_x11_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
static void evas_engine_cairo_x11_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text);
static void evas_engine_cairo_x11_font_cache_flush(void *data);
static void evas_engine_cairo_x11_font_cache_set(void *data, int bytes);
static int evas_engine_cairo_x11_font_cache_get(void *data);

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_Cairo_X11_Window *win;
};

Evas_Func evas_engine_cairo_x11_func =
{
   evas_engine_cairo_x11_info,
     evas_engine_cairo_x11_info_free,
     evas_engine_cairo_x11_setup,
     evas_engine_cairo_x11_output_free,
     evas_engine_cairo_x11_output_resize,
     evas_engine_cairo_x11_output_tile_size_set,
     evas_engine_cairo_x11_output_redraws_rect_add,
     evas_engine_cairo_x11_output_redraws_rect_del,
     evas_engine_cairo_x11_output_redraws_clear,
     evas_engine_cairo_x11_output_redraws_next_update_get,
     evas_engine_cairo_x11_output_redraws_next_update_push,
     evas_engine_cairo_x11_output_flush,
     /* draw context virtual methods */
     evas_engine_cairo_x11_context_new,
     evas_engine_cairo_x11_context_free,
     evas_engine_cairo_x11_context_clip_set,
     evas_engine_cairo_x11_context_clip_clip,
     evas_engine_cairo_x11_context_clip_unset,
     evas_engine_cairo_x11_context_clip_get,
     evas_engine_cairo_x11_context_color_set,
     evas_engine_cairo_x11_context_color_get,
     evas_engine_cairo_x11_context_multiplier_set,
     evas_engine_cairo_x11_context_multiplier_unset,
     evas_engine_cairo_x11_context_multiplier_get,
     evas_engine_cairo_x11_context_cutout_add,
     evas_engine_cairo_x11_context_cutout_clear,
     evas_engine_cairo_x11_context_anti_alias_set,
     evas_engine_cairo_x11_context_anti_alias_get,
     evas_engine_cairo_x11_context_color_interpolation_set,
     evas_engine_cairo_x11_context_color_interpolation_get,
     /* rectangle draw funcs */
     evas_engine_cairo_x11_rectangle_draw,
     /* line draw funcs */
     evas_engine_cairo_x11_line_draw,
     /* polygon draw funcs */
     evas_engine_cairo_x11_polygon_point_add,
     evas_engine_cairo_x11_polygon_points_clear,
     evas_engine_cairo_x11_polygon_draw,
     /* gradient draw funcs */
     evas_engine_cairo_x11_gradient_color_add,
     evas_engine_cairo_x11_gradient_colors_clear,
     evas_engine_cairo_x11_gradient_free,
     evas_engine_cairo_x11_gradient_fill_set,
     evas_engine_cairo_x11_gradient_type_set,
     evas_engine_cairo_x11_gradient_type_params_set,
     evas_engine_cairo_x11_gradient_geometry_init,
     evas_engine_cairo_x11_gradient_alpha_get,
     evas_engine_cairo_x11_gradient_map,
     evas_engine_cairo_x11_gradient_draw,
     /* image draw funcs */
     evas_engine_cairo_x11_image_load,
     evas_engine_cairo_x11_image_new_from_data,
     evas_engine_cairo_x11_image_new_from_copied_data,
     evas_engine_cairo_x11_image_free,
     evas_engine_cairo_x11_image_size_get,
     evas_engine_cairo_x11_image_size_set,
     evas_engine_cairo_x11_image_dirty_region,
     evas_engine_cairo_x11_image_data_get,
     evas_engine_cairo_x11_image_data_put,
     evas_engine_cairo_x11_image_alpha_set,
     evas_engine_cairo_x11_image_alpha_get,
     evas_engine_cairo_x11_image_draw,
     evas_engine_cairo_x11_image_comment_get,
     evas_engine_cairo_x11_image_format_get,
     /* image cache funcs */
     evas_engine_cairo_x11_image_cache_flush,
     evas_engine_cairo_x11_image_cache_set,
     evas_engine_cairo_x11_image_cache_get,
     /* font draw functions */
     evas_engine_cairo_x11_font_load,
     evas_engine_cairo_x11_font_memory_load,
     evas_engine_cairo_x11_font_add,
     evas_engine_cairo_x11_font_memory_add,
     evas_engine_cairo_x11_font_free,
     evas_engine_cairo_x11_font_ascent_get,
     evas_engine_cairo_x11_font_descent_get,
     evas_engine_cairo_x11_font_max_ascent_get,
     evas_engine_cairo_x11_font_max_descent_get,
     evas_engine_cairo_x11_font_string_size_get,
     evas_engine_cairo_x11_font_inset_get,
     evas_engine_cairo_x11_font_h_advance_get,
     evas_engine_cairo_x11_font_v_advance_get,
     evas_engine_cairo_x11_font_char_coords_get,
     evas_engine_cairo_x11_font_char_at_coords_get,
     evas_engine_cairo_x11_font_draw,
     /* font cache functions */
     evas_engine_cairo_x11_font_cache_flush,
     evas_engine_cairo_x11_font_cache_set,
     evas_engine_cairo_x11_font_cache_get
};

static void *
evas_engine_cairo_x11_info(Evas *e)
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
evas_engine_cairo_x11_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Cairo_X11 *in;

   in = (Evas_Engine_Info_Cairo_X11 *)info;
   free(in);
}

static void
evas_engine_cairo_x11_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Cairo_X11 *info;

   info = (Evas_Engine_Info_Cairo_X11 *)in;
   printf("CAIRO: setup info...\n");
   if (!e->engine.data.output)
     e->engine.data.output =
     evas_engine_cairo_x11_output_setup(e->output.w,
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
evas_engine_cairo_x11_output_setup(int w, int h, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth)
{
   Render_Engine *re;

   re = calloc(1, sizeof(Render_Engine));
   re->win = evas_engine_cairo_x11_window_new(disp, draw,
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
evas_engine_cairo_x11_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   evas_engine_cairo_x11_window_free(re->win);

   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
evas_engine_cairo_x11_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->w = w;
   re->win->h = h;

   evas_engine_cairo_x11_window_size_set(re->win, w, h);
}

static void
evas_engine_cairo_x11_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
evas_engine_cairo_x11_output_redraws_rect_add(void *data, int x, int y, int w, int h)
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
evas_engine_cairo_x11_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
evas_engine_cairo_x11_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
}

static void *
evas_engine_cairo_x11_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
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
evas_engine_cairo_x11_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
}

static void
evas_engine_cairo_x11_output_flush(void *data)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;
   Evas_List *l;

   re = (Render_Engine *)data;
}

static void *
evas_engine_cairo_x11_context_new(void *data)
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
evas_engine_cairo_x11_context_free(void *data, void *context)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = context;

   cairo_destroy(ctxt->cairo);
   free(ctxt);
}

static void
evas_engine_cairo_x11_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
evas_engine_cairo_x11_context_clip_clip(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
evas_engine_cairo_x11_context_clip_unset(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static int
evas_engine_cairo_x11_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
   return 0;
}

static void
evas_engine_cairo_x11_context_color_set(void *data, void *context, int r, int g, int b, int a)
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
evas_engine_cairo_x11_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
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
evas_engine_cairo_x11_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
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
evas_engine_cairo_x11_context_multiplier_unset(void *data, void *context)
{
   Render_Engine *re;
   Evas_Cairo_Context *ctxt;

   re = (Render_Engine *)data;
   ctxt = (Evas_Cairo_Context *)context;
   ctxt->mul.set = 0;
}

static int
evas_engine_cairo_x11_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
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
evas_engine_cairo_x11_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
evas_engine_cairo_x11_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in cairo engine */
}

static void
evas_engine_cairo_x11_context_anti_alias_set(void *data, void *context, unsigned char aa)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static unsigned char
evas_engine_cairo_x11_context_anti_alias_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return 1;
}

static void
evas_engine_cairo_x11_context_color_interpolation_set(void *data, void *context, int color_space)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_cairo_x11_context_color_interpolation_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return 0;
}

static void
evas_engine_cairo_x11_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
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
evas_engine_cairo_x11_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
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
evas_engine_cairo_x11_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
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
evas_engine_cairo_x11_polygon_points_clear(void *data, void *context, void *polygon)
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
evas_engine_cairo_x11_polygon_draw(void *data, void *context, void *surface, void *polygon)
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
evas_engine_cairo_x11_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return NULL;
}

static void *
evas_engine_cairo_x11_gradient_colors_clear(void *data, void *context, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return NULL;
}

static void
evas_engine_cairo_x11_gradient_free(void *data, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_gradient_type_set(void *data, void *gradient, char *name)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_gradient_type_params_set(void *data, void *gradient, char *params)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void *
evas_engine_cairo_x11_gradient_geometry_init(void *data, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return gradient;
}

static int
evas_engine_cairo_x11_gradient_alpha_get(void *data, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return 1;
}

static void
evas_engine_cairo_x11_gradient_map(void *data, void *context, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void *
evas_engine_cairo_x11_image_load(void *data, char *file, char *key, int *error)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;

   re = (Render_Engine *)data;
   if (error) *error = 0;

   im = calloc(1, sizeof(Evas_Cairo_Image));
   if (!im) return NULL;

   im->im = evas_common_load_image_from_file(file, key);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->references = 1;
   return im;
}

static void *
evas_engine_cairo_x11_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
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
evas_engine_cairo_x11_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
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
evas_engine_cairo_x11_image_free(void *data, void *image)
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
evas_engine_cairo_x11_image_size_get(void *data, void *image, int *w, int *h)
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
evas_engine_cairo_x11_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void *
evas_engine_cairo_x11_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void *
evas_engine_cairo_x11_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   *image_data = NULL;
   return image;
}

static void *
evas_engine_cairo_x11_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static void *
evas_engine_cairo_x11_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return image;
}

static int
evas_engine_cairo_x11_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_Cairo_Image *im;

   re = (Render_Engine *)data;
   if (!image) return 0;

   im = image;
   if (im->im->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
   return 0;
}

static void
evas_engine_cairo_x11_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
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
evas_engine_cairo_x11_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return NULL;
}

static char *
evas_engine_cairo_x11_image_format_get(void *data, void *image)
{
   Render_Engine *re;

   /* FIXME */
   re = (Render_Engine *)data;
   return NULL;
}

static void
evas_engine_cairo_x11_image_cache_flush(void *data)
{
   Render_Engine *re;
   int tmp_size;

   re = (Render_Engine *)data;
   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_image_set_cache(tmp_size);
}

static void
evas_engine_cairo_x11_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_set_cache(bytes);
}

static int
evas_engine_cairo_x11_image_cache_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_image_get_cache();
}

static void *
evas_engine_cairo_x11_font_load(void *data, char *name, int size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void *
evas_engine_cairo_x11_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void *
evas_engine_cairo_x11_font_add(void *data, void *font, char *name, int size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void *
evas_engine_cairo_x11_font_memory_add(void *data, void *font, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return NULL;
}

static void
evas_engine_cairo_x11_font_free(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
evas_engine_cairo_x11_font_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_descent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_max_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_max_descent_get(void *data, void *font)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static void
evas_engine_cairo_x11_font_string_size_get(void *data, void *font, char *text, int *w, int *h)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
evas_engine_cairo_x11_font_inset_get(void *data, void *font, char *text)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_h_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_v_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static int
evas_engine_cairo_x11_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

static void
evas_engine_cairo_x11_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_font_cache_flush(void *data)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_font_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
}

static int
evas_engine_cairo_x11_font_cache_get(void *data)
{
   Render_Engine *re;

   /* FIXME, use cairo font subsystem */
   re = (Render_Engine *)data;
   return 0;
}

int module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_engine_cairo_x11_func);
   return 1;
}

void module_close(void)
{
   
}

Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
     EVAS_MODULE_TYPE_ENGINE,
     "cairo_x11",
     "none"
};
