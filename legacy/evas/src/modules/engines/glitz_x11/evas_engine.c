#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Glitz_X11.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine        Render_Engine;
typedef struct _Render_Engine_Update Render_Engine_Update;

struct _Render_Engine_Update
{
   int x, y, w, h;
// void *surface;
};

struct _Render_Engine
{
   Display              *disp;
   Visual               *vis;
   Drawable              win;
   Pixmap                mask;
   unsigned char         destination_alpha : 1;
   
   Tilebuf              *tb;
   Tilebuf_Rect         *rects;
   Evas_Object_List     *cur_rect;
   int                   end : 1;
   
   Evas_List            *updates;
};

/* prototypes we will use here */
static void *eng_info(Evas *e);
static void eng_info_free(Evas *e, void *info);
static void eng_setup(Evas *e, void *info);
static void eng_output_free(void *data);
static void eng_output_resize(void *data, int w, int h);
static void eng_output_tile_size_set(void *data, int w, int h);
static void eng_output_redraws_rect_add(void *data, int x, int y, int w, int h);
static void eng_output_redraws_rect_del(void *data, int x, int y, int w, int h);
static void eng_output_redraws_clear(void *data);
static void *eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch);
static void eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h);
static void eng_output_flush(void *data);
static void eng_output_idle_flush(void *data);

static void eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h);

static void eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2);

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
static const char *eng_image_comment_get(void *data, void *image, char *key);
static char *eng_image_format_get(void *data, void *image);
static void eng_image_cache_flush(void *data);
static void eng_image_cache_set(void *data, int bytes);
static int eng_image_cache_get(void *data);

static void eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text);

/* internal engine routines */

/* engine api this module provides */
static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Glitz_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Glitz_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Glitz_X11 *in;

   in = (Evas_Engine_Info_Glitz_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Glitz_X11 *info;
   int resize = 1;

   info = (Evas_Engine_Info_Glitz_X11 *)in;
   if (!e->engine.data.output)
     {
	re = calloc(1, sizeof(Render_Engine));
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
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
	e->engine.data.output = re;
	resize = 0;
     }
   re = e->engine.data.output;
   if (!re) return;
   
   if (!e->engine.data.context) e->engine.data.context = e->engine.func->context_new(e->engine.data.output);
   
   re->disp = info->info.display;
   re->vis = info->info.visual;
   re->win = info->info.drawable;
   re->mask = info->info.mask;
   re->destination_alpha = info->info.destination_alpha;
   
   if (resize)
     {
	if (re->tb) evas_common_tilebuf_free(re->tb);
	if ((e->output.w > 0) && (e->output.h > 0))
	  re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	else
	  re->tb = evas_common_tilebuf_new(1, 1);
        if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_font_shutdown();
   evas_common_image_shutdown();
   while (re->updates)
     {
	Render_Engine_Update *reu;
	
	reu = re->updates->data;
	re->updates = evas_list_remove_list(re->updates, re->updates);
//	surface_free(reu->surface);
	free(reu);
     }
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   free(re);
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb) evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
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

   *x = ux; *y = uy; *w = uw; *h = uh;
   *cx = 0; *cy = 0; *cw = uw; *ch = uh;
   return NULL;
   if ((re->destination_alpha) || (re->mask))
     {
//	Xrender_Surface *surface;
//	
//	surface = _xr_render_surface_new(re->xinf, uw, uh, re->xinf->fmt32, 1);
//	_xr_render_surface_solid_rectangle_set(surface, 0, 0, 0, 0, 0, 0, uw, uh);
//	return surface;
     }
//   return _xr_render_surface_new(re->xinf, uw, uh, re->xinf->fmt24, 0);
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   Render_Engine_Update *reu;
   
   re = (Render_Engine *)data;
   reu = malloc(sizeof(Render_Engine_Update));
   if (!reu) return;
   reu->x = x;
   reu->y = y;
   reu->w = w;
   reu->h = h;
//   reu->surface = surface;
   re->updates = evas_list_append(re->updates, reu);
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   while (re->updates)
     {
	Render_Engine_Update *reu;
	
	reu = re->updates->data;
	re->updates = evas_list_remove_list(re->updates, re->updates);
//	if (re->mask_output)
//	  {
//	     Xrender_Surface *tsurf;
//	     
//	     _xr_render_surface_copy(reu->surface, re->output, 0, 0,
//				     reu->x, reu->y, reu->w, reu->h);
//	     tsurf = _xr_render_surface_new(re->xinf, reu->w, reu->h, re->xinf->fmt1, 1);
//	     if (tsurf)
//	       {
//		  _xr_render_surface_copy(reu->surface, tsurf, 0, 0,
//					  0, 0, reu->w, reu->h);
//		  _xr_render_surface_copy(tsurf, re->mask_output, 0, 0,
//					  reu->x, reu->y, reu->w, reu->h);
//		  _xr_render_surface_free(tsurf);
//	       }
//	  }
//	else
//	  {
//	     _xr_render_surface_copy(reu->surface, re->output, 0, 0,
//				     reu->x, reu->y, reu->w, reu->h);
//	  }
//	_xr_render_surface_free(reu->surface);
	free(reu);
     }
   XSync(re->disp, False);
//   _xr_image_info_pool_flush(re->xinf, 0, 0);
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
}

static void *
eng_gradient_new(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   return NULL;
}

static void
eng_gradient_free(void *data, void *gradient)
{
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
}

static void
eng_gradient_clear(void *data, void *gradient)
{
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
}

static int
eng_gradient_is_opaque(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   return 0;
}

static int
eng_gradient_is_visible(void *data, void *context, void *gradient, int x, int y, int w, int h)
{
   if (!context || !gradient)  return 0;
   return 1;
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
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   return NULL;
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data)
{
   return NULL;
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data)
{
   return NULL;
}

static void
eng_image_free(void *data, void *image)
{
}

static void
eng_image_size_get(void *data, void *image, int *w, int *h)
{
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   return image;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   return image;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   return image;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   return image;
}

static int
eng_image_alpha_get(void *data, void *image)
{
   return 0;
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

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
}

static const char *
eng_image_comment_get(void *data, void *image, char *key)
{
   return NULL;
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

//   tmp_size = _xre_image_cache_get();
   pfunc.image_cache_flush(data);
//   _xre_image_cache_set(0);
//   _xre_image_cache_set(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   pfunc.image_cache_set(data, bytes);
//   _xre_image_cache_set(bytes);
}

static int
eng_image_cache_get(void *data)
{
   return pfunc.image_cache_get(data);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
}

/* module advertising code */
EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_draw);
   ORD(gradient_new);
   ORD(gradient_free);
   ORD(gradient_color_stop_add);
   ORD(gradient_alpha_stop_add);
   ORD(gradient_color_data_set);
   ORD(gradient_alpha_data_set);
   ORD(gradient_clear);
   ORD(gradient_fill_set);
   ORD(gradient_fill_angle_set);
   ORD(gradient_fill_spread_set);
   ORD(gradient_angle_set);
   ORD(gradient_offset_set);
   ORD(gradient_direction_set);
   ORD(gradient_type_set);
   ORD(gradient_is_opaque);
   ORD(gradient_is_visible);
   ORD(gradient_render_pre);
   ORD(gradient_render_post);
   ORD(gradient_draw);
   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(font_draw);
   /* now advertise out own api */
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
     "glitz_x11",
     "none"
};

