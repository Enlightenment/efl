#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "evas_engine_api_gl_x11.h"
#include "Evas.h"
#include "Evas_Engine_GL_X11.h"

static void *evas_engine_gl_x11_info(Evas *e);
static void evas_engine_gl_x11_info_free(Evas *e, void *info);
static void evas_engine_gl_x11_setup(Evas *e, void *info);
static void *evas_engine_gl_x11_output_setup(int w, int h, int rot, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth);
static void evas_engine_gl_x11_output_free(void *data);
static void evas_engine_gl_x11_output_resize(void *data, int w, int h);
static void evas_engine_gl_x11_output_tile_size_set(void *data, int w, int h);
static void evas_engine_gl_x11_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void evas_engine_gl_x11_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void evas_engine_gl_x11_output_redraws_clear(void *data);
static void *evas_engine_gl_x11_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void evas_engine_gl_x11_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h);
static void evas_engine_gl_x11_output_flush(void *data);
static void *evas_engine_gl_x11_context_new(void *data);
static void evas_engine_gl_x11_context_free(void *data, void *context);
static void evas_engine_gl_x11_context_clip_set(void *data, void *context, int x, int y, int w, int h);
static void evas_engine_gl_x11_context_clip_clip(void *data, void *context, int x, int y, int w, int h);
static void evas_engine_gl_x11_context_clip_unset(void *data, void *context);
static int evas_engine_gl_x11_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h);
static void evas_engine_gl_x11_context_color_set(void *data, void *context, int r, int g, int b, int a);
static int evas_engine_gl_x11_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a);
static void evas_engine_gl_x11_context_multiplier_set(void *data, void *context, int r, int g, int b, int a);
static void evas_engine_gl_x11_context_multiplier_unset(void *data, void *context);
static int evas_engine_gl_x11_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a);
static void evas_engine_gl_x11_context_cutout_add(void *data, void *context, int x, int y, int w, int h);
static void evas_engine_gl_x11_context_cutout_clear(void *data, void *context);
static void evas_engine_gl_x11_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h);
static void evas_engine_gl_x11_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2);
static void *evas_engine_gl_x11_polygon_point_add(void *data, void *context, void *polygon, int x, int y);
static void *evas_engine_gl_x11_polygon_points_clear(void *data, void *context, void *polygon);
static void evas_engine_gl_x11_polygon_draw(void *data, void *context, void *surface, void *polygon);
static void *evas_engine_gl_x11_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance);
static void *evas_engine_gl_x11_gradient_colors_clear(void *data, void *context, void *gradient);
static void evas_engine_gl_x11_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle);
static void *evas_engine_gl_x11_image_load(void *data, char *file, char *key, int *error);
static void *evas_engine_gl_x11_image_new_from_data(void *data, int w, int h, DATA32 *image_data);
static void *evas_engine_gl_x11_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data);
static void evas_engine_gl_x11_image_free(void *data, void *image);
static void evas_engine_gl_x11_image_size_get(void *data, void *image, int *w, int *h);
static void *evas_engine_gl_x11_image_size_set(void *data, void *image, int w, int h);
static void *evas_engine_gl_x11_image_dirty_region(void *data, void *image, int x, int y, int w, int h);
static void *evas_engine_gl_x11_image_data_get(void *data, void *image, int to_write, DATA32 **image_data);
static void *evas_engine_gl_x11_image_data_put(void *data, void *image, DATA32 *image_data);
static void *evas_engine_gl_x11_image_alpha_set(void *data, void *image, int has_alpha);
static int evas_engine_gl_x11_image_alpha_get(void *data, void *image);
static void evas_engine_gl_x11_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth);
static char *evas_engine_gl_x11_image_comment_get(void *data, void *image, char *key);
static char *evas_engine_gl_x11_image_format_get(void *data, void *image);
static void evas_engine_gl_x11_image_cache_flush(void *data);
static void evas_engine_gl_x11_image_cache_set(void *data, int bytes);
static int evas_engine_gl_x11_image_cache_get(void *data);
static void *evas_engine_gl_x11_font_load(void *data, char *name, int size);
static void evas_engine_gl_x11_font_free(void *data, void *font);
static int evas_engine_gl_x11_font_ascent_get(void *data, void *font);
static int evas_engine_gl_x11_font_descent_get(void *data, void *font);
static int evas_engine_gl_x11_font_max_ascent_get(void *data, void *font);
static int evas_engine_gl_x11_font_max_descent_get(void *data, void *font);
static void evas_engine_gl_x11_font_string_size_get(void *data, void *font, char *text, int *w, int *h);
static int evas_engine_gl_x11_font_inset_get(void *data, void *font, char *text);
static int evas_engine_gl_x11_font_h_advance_get(void *data, void *font, char *text);
static int evas_engine_gl_x11_font_v_advance_get(void *data, void *font, char *text);
static int evas_engine_gl_x11_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch);
static int evas_engine_gl_x11_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
static void evas_engine_gl_x11_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text);
static void evas_engine_gl_x11_font_cache_flush(void *data);
static void evas_engine_gl_x11_font_cache_set(void *data, int bytes);
static int evas_engine_gl_x11_font_cache_get(void *data);


static Visual *evas_engine_gl_x11_best_visual_get(Display *disp, int screen);
static Colormap evas_engine_gl_x11_best_colormap_get(Display *disp, int screen);
static int evas_engine_gl_x11_best_depth_get(Display *disp, int screen);

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   int               end;
};

Evas_Func evas_engine_gl_x11_func =
{
   evas_engine_gl_x11_info,
     evas_engine_gl_x11_info_free,
     evas_engine_gl_x11_setup,
     evas_engine_gl_x11_output_free,
     evas_engine_gl_x11_output_resize,
     evas_engine_gl_x11_output_tile_size_set,
     evas_engine_gl_x11_output_redraws_rect_add,
     evas_engine_gl_x11_output_redraws_rect_del,
     evas_engine_gl_x11_output_redraws_clear,
     evas_engine_gl_x11_output_redraws_next_update_get,
     evas_engine_gl_x11_output_redraws_next_update_push,
     evas_engine_gl_x11_output_flush,
     /* draw context virtual methods */
     evas_engine_gl_x11_context_new,
     evas_engine_gl_x11_context_free,
     evas_engine_gl_x11_context_clip_set,
     evas_engine_gl_x11_context_clip_clip,
     evas_engine_gl_x11_context_clip_unset,
     evas_engine_gl_x11_context_clip_get,
     evas_engine_gl_x11_context_color_set,
     evas_engine_gl_x11_context_color_get,
     evas_engine_gl_x11_context_multiplier_set,
     evas_engine_gl_x11_context_multiplier_unset,
     evas_engine_gl_x11_context_multiplier_get,
     evas_engine_gl_x11_context_cutout_add,
     evas_engine_gl_x11_context_cutout_clear,
     /* rectangle draw funcs */
     evas_engine_gl_x11_rectangle_draw,
     /* line draw funcs */
     evas_engine_gl_x11_line_draw,
     /* polygon draw funcs */
     evas_engine_gl_x11_polygon_point_add,
     evas_engine_gl_x11_polygon_points_clear,
     evas_engine_gl_x11_polygon_draw,
     /* gradient draw funcs */
     evas_engine_gl_x11_gradient_color_add,
     evas_engine_gl_x11_gradient_colors_clear,
     evas_engine_gl_x11_gradient_draw,
     /* image draw funcs */
     evas_engine_gl_x11_image_load,
     evas_engine_gl_x11_image_new_from_data,
     evas_engine_gl_x11_image_new_from_copied_data,
     evas_engine_gl_x11_image_free,
     evas_engine_gl_x11_image_size_get,
     evas_engine_gl_x11_image_size_set,
     evas_engine_gl_x11_image_dirty_region,
     evas_engine_gl_x11_image_data_get,
     evas_engine_gl_x11_image_data_put,
     evas_engine_gl_x11_image_alpha_set,
     evas_engine_gl_x11_image_alpha_get,
     evas_engine_gl_x11_image_draw,
     evas_engine_gl_x11_image_comment_get,
     evas_engine_gl_x11_image_format_get,
     /* image cache funcs */
     evas_engine_gl_x11_image_cache_flush,
     evas_engine_gl_x11_image_cache_set,
     evas_engine_gl_x11_image_cache_get,
     /* font draw functions */
     evas_engine_gl_x11_font_load,
     evas_engine_gl_x11_font_free,
     evas_engine_gl_x11_font_ascent_get,
     evas_engine_gl_x11_font_descent_get,
     evas_engine_gl_x11_font_max_ascent_get,
     evas_engine_gl_x11_font_max_descent_get,
     evas_engine_gl_x11_font_string_size_get,
     evas_engine_gl_x11_font_inset_get,
     evas_engine_gl_x11_font_h_advance_get,
     evas_engine_gl_x11_font_v_advance_get,
     evas_engine_gl_x11_font_char_coords_get,
     evas_engine_gl_x11_font_char_at_coords_get,     
     evas_engine_gl_x11_font_draw,
     /* font cache functions */
     evas_engine_gl_x11_font_cache_flush,
     evas_engine_gl_x11_font_cache_set,
     evas_engine_gl_x11_font_cache_get
};

static void *
evas_engine_gl_x11_info(Evas *e)
{
   Evas_Engine_Info_GL_X11 *info;
   
   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->func.best_visual_get = evas_engine_gl_x11_best_visual_get;
   info->func.best_colormap_get = evas_engine_gl_x11_best_colormap_get;
   info->func.best_depth_get = evas_engine_gl_x11_best_depth_get;
   return info;
   e = NULL;
}

static void
evas_engine_gl_x11_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_GL_X11 *in;
   
   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static void
evas_engine_gl_x11_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_GL_X11 *info;
   
   info = (Evas_Engine_Info_GL_X11 *)in;
   if (!e->engine.data.output)
     e->engine.data.output = 1;
   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context = 
     e->engine.func->context_new(e->engine.data.output);   
   
   re = e->engine.data.output;
}

static void *
evas_engine_gl_x11_output_setup(int w, int h, int rot, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth)
{
   Render_Engine *re;
   
   re = calloc(1, sizeof(Render_Engine));
   /* if we haven't initialized - init (automatic abort if already done) */
   cpu_init();
   
   blend_init();
   image_init();
   convert_init();
   scale_init();
   rectangle_init();
   gradient_init();
   polygon_init();
   line_init();
   font_init();
   draw_init();
   tilebuf_init();
   
   x_gl_x11_init();
   
   return re;
}

static void
evas_engine_gl_x11_output_free(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
   free(re);
}

static void
evas_engine_gl_x11_output_resize(void *data, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_output_redraws_clear(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_output_flush(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
/*   XSync(re->ob->priv.x.disp, False); */
}

static void *
evas_engine_gl_x11_context_new(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_free(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_clip_set(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_clip_clip(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_clip_unset(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_context_clip_get(void *data, void *context, int *x, int *y, int *w, int *h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_color_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_context_color_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_multiplier_set(void *data, void *context, int r, int g, int b, int a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_multiplier_unset(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_context_multiplier_get(void *data, void *context, int *r, int *g, int *b, int *a)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_cutout_add(void *data, void *context, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}






static void
evas_engine_gl_x11_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_gradient_colors_clear(void *data, void *context, void *gradient)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_load(void *data, char *file, char *key, int *error)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}


static int
evas_engine_gl_x11_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static char *
evas_engine_gl_x11_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static char *
evas_engine_gl_x11_image_format_get(void *data, void *image)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_image_cache_flush(void *data)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_image_cache_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void *
evas_engine_gl_x11_font_load(void *data, char *name, int size)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_font_free(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int 
evas_engine_gl_x11_font_descent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_max_ascent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_max_descent_get(void *data, void *font)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_font_string_size_get(void *data, void *font, char *text, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_inset_get(void *data, void *font, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_h_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;
   
   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_v_advance_get(void *data, void *font, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_char_coords_get(void *data, void *font, char *text, int pos, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_char_at_coords_get(void *data, void *font, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, char *text)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_font_cache_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
evas_engine_gl_x11_font_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static int
evas_engine_gl_x11_font_cache_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}









/* private engine functions the calling prog can use */

static Visual *
evas_engine_gl_x11_best_visual_get(Display *disp, int screen)
{
   if (!disp) return NULL;
   return DefaultVisual(disp, screen);
}

static Colormap
evas_engine_gl_x11_best_colormap_get(Display *disp, int screen)
{
   if (!disp) return 0;
   return DefaultColormap(disp, screen);
}

static int
evas_engine_gl_x11_best_depth_get(Display *disp, int screen)
{
   if (!disp) return 0;
   return DefaultDepth(disp, screen);
}
