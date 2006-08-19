#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_GL_X11.h"
#include "evas_gl_common.h"

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
static void *eng_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance);
static void *eng_gradient_colors_clear(void *data, void *context, void *gradient);
static void *eng_gradient_data_set(void *data, void *context, void *gradient, void *map, int len, int has_alpha);
static void *eng_gradient_data_unset(void *data, void *context, void *gradient);
static void eng_gradient_free(void *data, void *gradient);
static void eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h);
static void eng_gradient_range_offset_set(void *data, void *gradient, float offset);
static void eng_gradient_type_set(void *data, void *gradient, char *name);
static void eng_gradient_type_params_set(void *data, void *gradient, char *params);
static void *eng_gradient_geometry_init(void *data, void *gradient, int spread);
static int  eng_gradient_alpha_get(void *data, void *gradient, int spread, int op);
static void eng_gradient_map(void *data, void *context, void *gradient, int spread);
static void eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle, int spread);
static void *eng_image_load(void *data, char *file, char *key, int *error, Evas_Image_Load_Opts *lo);
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
static void *eng_image_border_set(void *data, void *image, int l, int r, int t, int b);
static void eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b);
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


static Visual *eng_best_visual_get(Display *disp, int screen);
static Colormap eng_best_colormap_get(Display *disp, int screen);
static int eng_best_depth_get(Display *disp, int screen);

static void eng_font_hinting_set(void *data, void *font, int hinting);
static int eng_font_hinting_can_hint(void *data, int hinting);

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_GL_X11_Window *win;
   int                 end;
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
     eng_image_border_set,
     eng_image_border_get,
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
   Evas_Engine_Info_GL_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   printf("GL: create info...\n");
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_GL_X11 *in;

   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_GL_X11 *info;

   info = (Evas_Engine_Info_GL_X11 *)in;
   printf("GL: setup info...\n");
   if (!e->engine.data.output)
     e->engine.data.output =
     eng_output_setup(e->output.w,
		      e->output.h,
		      info->info.display,
		      info->info.drawable,
		      info->info.visual,
		      info->info.colormap,
		      info->info.depth);
   else
     {
	re = e->engine.data.output;
	eng_window_free(re->win);
	re->win = eng_window_new(info->info.display,
				 info->info.drawable,
				 0,/* FIXME: screen 0 assumption */
				 info->info.visual,
				 info->info.colormap,
				 info->info.depth,
				 e->output.w,
				 e->output.h);
     }
   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
}

static void *
eng_output_setup(int w, int h, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth)
{
   Render_Engine *re;
   int eb, evb;

   if (!glXQueryExtension(disp, &eb, &evb)) return NULL;
   re = calloc(1, sizeof(Render_Engine));

   re->win = eng_window_new(disp, draw,
			    0 /* FIXME: screen 0 assumption */,
			    vis, cmap, depth, w, h);
   if (!re->win)
     {
	free(re);
	return NULL;
     }
   printf("GL: gl window setup done.\n");
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
   evas_gl_common_context_resize(re->win->gl_context, w, h);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* smple bounding box */
   if (!re->win->draw.redraw)
     {
#if 0
	re->win->draw.x1 = x;
	re->win->draw.y1 = y;
	re->win->draw.x2 = x + w - 1;
	re->win->draw.y2 = y + h - 1;
#else
	re->win->draw.x1 = 0;
	re->win->draw.y1 = 0;
	re->win->draw.x2 = re->win->w - 1;
	re->win->draw.y2 = re->win->h - 1;
#endif
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
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
//   printf("GL: finish update cycle!\n");
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* get the upate rect surface - return engine data as dummy */
   if (!re->win->draw.redraw)
     {
//	printf("GL: NO updates!\n");
	return NULL;
     }
//   printf("GL: update....!\n");
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
   /* put back update surface.. in this case just unflag redraw */
//   printf("GL: update done.\n");
   re->win->draw.redraw = 0;
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
//   printf("GL: flush your mush!\n");
   eng_window_use(re->win);

/* SLOW AS ALL HELL! */
#if 0
   evas_gl_common_swap_rect(re->win->gl_context,
			    re->win->draw.x1, re->win->draw.y1,
			    re->win->draw.x2 - re->win->draw.x1 + 1,
			    re->win->draw.y2 - re->win->draw.y1 + 1);
#else
#if 0
   glFlush();
     {
	unsigned int rc;
   
	glXGetVideoSyncSGI(&rc);
	glXWaitVideoSyncSGI(2, (rc + 1) % 2, &rc);
     }
#endif   
   glXSwapBuffers(re->win->disp, re->win->win);
#endif
//   glFlush();
//   glXWaitGL();
//   XSync(re->win->disp, False);
//   printf("SYNC! %i\n", fr++);
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
   /* not used in gl engine */
}

static void
eng_context_cutout_clear(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* not used in gl engine */
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
eng_context_render_op_set(void *data, void *context, int op)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_draw_context_set_render_op(context, op);
}

static int
eng_context_render_op_get(void *data, void *context)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return ((RGBA_Draw_Context *)context)->render_op;
}






static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_rect_draw(re->win->gl_context, context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_line_draw(re->win->gl_context, context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data, void *context, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_point_add(polygon, x, y);

}

static void *
eng_polygon_points_clear(void *data, void *context, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_poly_draw(re->win->gl_context, context, polygon);
}

static void *
eng_gradient_color_add(void *data, void *context, void *gradient, int r, int g, int b, int a, int distance)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_gradient_color_add(gradient, r, g, b, a, distance);
}

static void *
eng_gradient_colors_clear(void *data, void *context, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_gradient_colors_clear(gradient);
}

static void *
eng_gradient_data_set(void *data, void *context, void *gradient, void *map, int len, int has_alpha)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_data_set(gradient, map, len, has_alpha);
   return gradient;
   context = NULL;
}

static void *
eng_gradient_data_unset(void *data, void *context, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_data_unset(gradient);
   return gradient;
   context = NULL;
}

static void
eng_gradient_free(void *data, void *gradient)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_free(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_range_offset_set(void *data, void *gradient, float offset)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_range_offset_set(gradient, offset);

}

static void
eng_gradient_type_set(void *data, void *gradient, char *name)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_type_set(gradient, name);
}

static void
eng_gradient_type_params_set(void *data, void *gradient, char *params)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_type_params_set(gradient, params);
}

static void *
eng_gradient_geometry_init(void *data, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_gradient_geometry_init(gradient, spread);
}

static int
eng_gradient_alpha_get(void *data, void *gradient, int spread, int op)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_gradient_alpha_get(gradient, spread, op);
}

static void
eng_gradient_map(void *data, void *context, void *gradient, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_gradient_map(context, gradient, spread);
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h, double angle, int spread)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_gradient_draw(re->win->gl_context, context, gradient, x, y, w, h, angle, spread);
}

static void *
eng_image_load(void *data, char *file, char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = 0;
   eng_window_use(re->win);
   return evas_gl_common_image_load(re->win->gl_context, file, key, lo);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_data(re->win->gl_context, w, h, image_data);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_copied_data(re->win->gl_context, w, h, image_data);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (w) *w = ((Evas_GL_Image *)image)->im->image->w;
   if (h) *h = ((Evas_GL_Image *)image)->im->image->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im_old;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   if (!image) return NULL;
   im_old = image;
   if ((im_old) && (im_old->im->image->w == w) && (im_old->im->image->h == h))
     return image;
   im = evas_gl_common_image_new(re->win->gl_context, w, h);
   if (im_old)
     {
	evas_common_load_image_data_from_file(im_old->im);
	if (im_old->im->image->data)
	  {
	     evas_common_blit_rectangle(im_old->im, im->im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
	evas_gl_common_image_free(im_old);
     }
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_image_dirty(image);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   eng_window_use(re->win);
   evas_common_load_image_data_from_file(im->im);
   if (to_write)
     {
	if (im->references > 1)
	  {
	     Evas_GL_Image *im_new;

	     im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->image->w, im->im->image->h, im->im->image->data);
	     if (!im_new)
	       {
		  return im;
		  *image_data = NULL;
	       }
	     im = im_new;
	  }
	else
	  evas_gl_common_image_dirty(im);
     }
   *image_data = im->im->image->data;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   eng_window_use(re->win);
   if (image_data != im->im->image->data)
     {
	int w, h;

	w = im->im->image->w;
	h = im->im->image->h;
	evas_gl_common_image_free(im);
	return eng_image_new_from_data(data, w, h, image_data);
     }
   /* hmmm - but if we wrote... why bother? */
   evas_gl_common_image_dirty(im);
   return im;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   im = image;
   if ((has_alpha) && (im->im->flags & RGBA_IMAGE_HAS_ALPHA)) return image;
   else if ((!has_alpha) && (!(im->im->flags & RGBA_IMAGE_HAS_ALPHA))) return image;
   if (im->references > 1)
     {
	Evas_GL_Image *im_new;

	im_new = evas_gl_common_image_new_from_copied_data(im->gc, im->im->image->w, im->im->image->h, im->im->image->data);
	if (!im_new) return im;
	evas_gl_common_image_free(im);
	im = im_new;
     }
   else
     evas_gl_common_image_dirty(im);
   if (has_alpha)
     im->im->flags |= RGBA_IMAGE_HAS_ALPHA;
   else
     im->im->flags &= ~RGBA_IMAGE_HAS_ALPHA;
   return image;
}


static int
eng_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   eng_window_use(re->win);
   if (im->im->flags & RGBA_IMAGE_HAS_ALPHA) return 1;
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
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_image_draw(re->win->gl_context, context, image,
			     src_x, src_y, src_w, src_h,
			     dst_x, dst_y, dst_w, dst_h,
			     smooth);
}

static char *
eng_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data, void *image)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   im = image;
//   if (im->im->info.format == 1) return "png";
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
   /* text renderign takes benchmark from 40 to 27 on gf4-ti4400 */
     {
	static RGBA_Image *im = NULL;

	if (!im)
	  {
	     im = evas_common_image_new();
	     im->image = evas_common_image_surface_new(im);
	     im->image->no_free = 1;
	  }
	im->image->w = re->win->w;
	im->image->h = re->win->h;
	im->image->data = NULL;
	evas_common_draw_context_font_ext_set(context,
					      re->win->gl_context,
					      evas_gl_font_texture_new,
					      evas_gl_font_texture_free,
					      evas_gl_font_texture_draw);
	/* 40 */
	evas_common_font_draw(im, context, font, x, y, text);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }
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
   if (!_evas_gl_x11_vi)
     _evas_gl_x11_vi = glXChooseVisual(disp, screen,
				       _evas_gl_x11_configuration);
   if (!_evas_gl_x11_vi) return NULL;
   return _evas_gl_x11_vi->visual;
}

static Colormap
eng_best_colormap_get(Display *disp, int screen)
{
   if (!disp) return 0;
   if (!_evas_gl_x11_vi)
     eng_best_visual_get(disp, screen);
   if (!_evas_gl_x11_vi) return 0;
   _evas_gl_x11_cmap = XCreateColormap(disp, RootWindow(disp, screen),
				_evas_gl_x11_vi->visual, 0);
   return _evas_gl_x11_cmap;
}

static int
eng_best_depth_get(Display *disp, int screen)
{
   if (!disp) return 0;
   if (!_evas_gl_x11_vi)
     eng_best_visual_get(disp, screen);
   if (!_evas_gl_x11_vi) return 0;
   return _evas_gl_x11_vi->depth;
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
     "gl_x11",
     "none"
};
