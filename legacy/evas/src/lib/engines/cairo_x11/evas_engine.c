#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "evas_engine_api_cairo_x11.h"
#include "Evas.h"
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
static void evas_engine_cairo_x11_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h);
static void evas_engine_cairo_x11_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2);
static void *evas_engine_cairo_x11_polygon_point_add(void *data, void *context, void *polygon, int x, int y);
static void *evas_engine_cairo_x11_polygon_points_clear(void *data, void *context, void *polygon);
static void evas_engine_cairo_x11_polygon_draw(void *data, void *context, void *surface, void *polygon);
static void *evas_engine_cairo_x11_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance);
static void *evas_engine_cairo_x11_gradient_colors_clear(void *data, void *context, void *gradient);
static void evas_engine_cairo_x11_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle);
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
   Tilebuf               *tb;
   Tilebuf_Rect          *rects;
   Evas_Object_List      *cur_rect;
   Evas_Cairo_X11_Window *win;
   unsigned char          end : 1;
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
   int eb, evb;
   
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
   
   re->tb = evas_common_tilebuf_new(w, h);
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   
   return re;
}

static void
evas_engine_cairo_x11_output_free(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;

   evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   
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
   
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void
evas_engine_cairo_x11_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
evas_engine_cairo_x11_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_cairo_x11_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_cairo_x11_output_redraws_clear(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_tilebuf_clear(re->tb);

}

typedef struct _Update Update;

struct _Update
{
   int      x, y, w, h;
   GC       gc;
   Pixmap   pm;
};

static void *
evas_engine_cairo_x11_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   Tilebuf_Rect *rect;
   int ux, uy, uw, uh;
   Update *u;
   
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
   *x = ux; *y = uy; *w = uw; *h = uh;
   *cx = 0; *cy = 0; *cw = uw; *ch = uh;
   u = malloc(sizeof(Update));
   u->pm = XCreatePixmap(re->win->disp, re->win->win, uw, uh, re->win->depth);
   u->gc = XCreateGC(re->win->disp, u->pm, 0, NULL);
   u->x = ux; u->y = uy; u->w = uw; u->h = uh;
   cairo_set_target_drawable(re->win->cairo, re->win->disp, u->pm);
   return u;
}

static void
evas_engine_cairo_x11_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   Update *u;
   
   re = (Render_Engine *)data;
   u = surface;
   XCopyArea(re->win->disp, u->pm, re->win->win, u->gc, 0, 0, w, h, x, y);
   XFreePixmap(re->win->disp, u->pm);
   XFreeGC(re->win->disp, u->gc);
   free(u);
}

static void
evas_engine_cairo_x11_output_flush(void *data)
{
   Render_Engine *re;
   static int fr = 0;
   
   re = (Render_Engine *)data;
   XSync(re->win->disp, False);
}

static void *
evas_engine_cairo_x11_context_new(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return evas_common_draw_context_new();
}

static void
evas_engine_cairo_x11_context_free(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_free(context);
}

static void
evas_engine_cairo_x11_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

static void
evas_engine_cairo_x11_context_clip_clip(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

static void
evas_engine_cairo_x11_context_clip_unset(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_unset_clip(context);
}

static int
evas_engine_cairo_x11_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
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
evas_engine_cairo_x11_context_color_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_set_color(context, r, g, b, a);
}

static int
evas_engine_cairo_x11_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   *r = (int)(R_VAL(((RGBA_Draw_Context *)context)->col.col));
   *g = (int)(G_VAL(((RGBA_Draw_Context *)context)->col.col));
   *b = (int)(B_VAL(((RGBA_Draw_Context *)context)->col.col));
   *a = (int)(A_VAL(((RGBA_Draw_Context *)context)->col.col));
   return 1;
}

static void
evas_engine_cairo_x11_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

static void
evas_engine_cairo_x11_context_multiplier_unset(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   evas_common_draw_context_unset_multiplier(context);
}

static int
evas_engine_cairo_x11_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   *r = (int)(R_VAL(((RGBA_Draw_Context *)context)->mul.col));
   *g = (int)(G_VAL(((RGBA_Draw_Context *)context)->mul.col));
   *b = (int)(B_VAL(((RGBA_Draw_Context *)context)->mul.col));
   *a = (int)(A_VAL(((RGBA_Draw_Context *)context)->mul.col));
   return ((RGBA_Draw_Context *)context)->mul.use;
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
evas_engine_cairo_x11_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_cairo_x11_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return NULL;
}

static void *
evas_engine_cairo_x11_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return NULL;
}

static void
evas_engine_cairo_x11_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
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
evas_engine_cairo_x11_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_cairo_x11_image_load(void *data, char *file, char *key, int *error)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = 0;
     {
	Evas_Cairo_Image *im;
	
	im = calloc(1, sizeof(Evas_Cairo_Image));
	im->im = evas_common_load_image_from_file(file, key);
	if (!im->im)
	  {
	     free(im);
	     return NULL;
	  }
	im->references = 1;
	return im;
     }
   return NULL;
}

static void *
evas_engine_cairo_x11_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return NULL;
}

static void *
evas_engine_cairo_x11_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return NULL;
}

static void
evas_engine_cairo_x11_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
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
	return;
     }
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
   
   re = (Render_Engine *)data;
   return image;
}

static void *
evas_engine_cairo_x11_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return image;
}

static void *
evas_engine_cairo_x11_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   *image_data = NULL;
   return image;
}

static void *
evas_engine_cairo_x11_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return image;
}

static void *
evas_engine_cairo_x11_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return image;
}

static int
evas_engine_cairo_x11_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   if (!image) return 0;
     {
	Evas_Cairo_Image *im;
	
	im = image;
	if (im->im->flags & RGBA_IMAGE_HAS_ALPHA) return 1;	
     }
   return 0;
}

static void
evas_engine_cairo_x11_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   if (!image) return;
     {
	Evas_Cairo_Image *im;
	Update *u;
	DATA32 *pix;
	
	im = image;
	u = surface;
	evas_common_load_image_data_from_file(im->im);
	pix = im->im->image->data;
	if (pix)
	  {
	     if (!im->surf)
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
		       im->surf = cairo_surface_create_for_image(im->mulpix,
								 CAIRO_FORMAT_ARGB32,
								 im->im->image->w,
								 im->im->image->h,
								 0);
		    }
	       }
	     if (smooth)
	       cairo_surface_set_filter(im->surf, CAIRO_FILTER_BILINEAR);
	     else
	       cairo_surface_set_filter(im->surf, CAIRO_FILTER_NEAREST);
	     cairo_save(re->win->cairo);
	     cairo_translate(re->win->cairo,
			     dst_x,
			     dst_y);
	     cairo_scale(re->win->cairo,
			 (double)src_w / (double)dst_w,
			 (double)src_h / (double)dst_h
			 );
	     cairo_move_to(re->win->cairo, 0, 0);
//	     cairo_set_rgb_color(re->win->cairo,
//				  (double)(R_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0,
//				  (double)(R_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0,
//				  (double)(R_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0);
//	     cairo_set_alpha(re->win->cairo,
//			     (double)(A_VAL(((RGBA_Draw_Context *)context)->col.col)) / 255.0);
	     cairo_show_surface(re->win->cairo,
				im->surf,
				im->im->image->w,
				im->im->image->h);
	     cairo_restore(re->win->cairo);
	  }
     }
}

static char *
evas_engine_cairo_x11_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return NULL;
}

static char *
evas_engine_cairo_x11_image_format_get(void *data, void *image)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   return "";
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

   re = (Render_Engine *)data;
   return evas_common_font_load(name, size);
}

static void *
evas_engine_cairo_x11_font_memory_load(void *data, char *name, int size, const void *fdata, int fdata_size)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_memory_load(name, size, fdata, fdata_size);
}

static void
evas_engine_cairo_x11_font_free(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_free(font);
}

static int
evas_engine_cairo_x11_font_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_ascent_get(font);
}

static int 
evas_engine_cairo_x11_font_descent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_descent_get(font);
}

static int
evas_engine_cairo_x11_font_max_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_max_ascent_get(font);
}

static int
evas_engine_cairo_x11_font_max_descent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_max_descent_get(font);
}

static void
evas_engine_cairo_x11_font_string_size_get(void *data, void *font, char *text, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_query_size(font, text, w, h);
}

static int
evas_engine_cairo_x11_font_inset_get(void *data, void *font, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_query_inset(font, text);
}

static int
evas_engine_cairo_x11_font_h_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;
   
   re = (Render_Engine *)data;
   evas_common_font_query_advance(font, text, &h, &v);
   return h;
}

static int
evas_engine_cairo_x11_font_v_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   int h, v;
   
   re = (Render_Engine *)data;
   evas_common_font_query_advance(font, text, &h, &v);
   return v;
}

static int
evas_engine_cairo_x11_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_query_char_coords(font, text, pos, cx, cy, cw, ch);
}

static int
evas_engine_cairo_x11_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_query_text_at_pos(font, text, x, y, cx, cy, cw, ch); 
}

static void
evas_engine_cairo_x11_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_cairo_x11_font_cache_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_flush();
}

static void
evas_engine_cairo_x11_font_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_cache_set(bytes);
}

static int
evas_engine_cairo_x11_font_cache_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_common_font_cache_get();
}
