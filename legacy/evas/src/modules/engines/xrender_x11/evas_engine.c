#include "evas_common.h"
#include "evas_private.h"

#include "Evas_Engine_XRender_X11.h"

#include "evas_engine.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine        Render_Engine;
typedef struct _Render_Engine_Update Render_Engine_Update;

struct _Render_Engine_Update
{
   int x, y, w, h;
   Xrender_Surface *surface;
};

struct _Render_Engine
{
   struct {
      void         *connection;
      void         *screen;
      unsigned int  window;
      unsigned int  mask;
      void         *visual;
      void        (*sync) (Render_Engine *re);
   } x11;
   unsigned char    destination_alpha : 1;

   Ximage_Info     *xinf;
   Xrender_Surface *output;
   Xrender_Surface *mask_output;

   Tilebuf         *tb;
   Tilebuf_Rect    *rects;
   Eina_Inlist     *cur_rect;
   int              end : 1;

   Eina_List       *updates;

   void *(*font_surface_new)(void *xinf, RGBA_Font_Glyph *fg);
   void (*font_surface_free)(void *fs);
   void (*font_surface_draw)(void *xinf, void *surface, void *dc, RGBA_Font_Glyph *fg, int x, int y);

   XR_Gradient *(*gradient_new)(Ximage_Info *xinf);
   void (*gradient_free)(XR_Gradient *gr);
   void (*gradient_color_stop_add)(XR_Gradient *gr, int r, int g, int b, int a, int delta);
   void (*gradient_alpha_stop_add)(XR_Gradient *gr, int a, int delta);
   void (*gradient_color_data_set)(XR_Gradient *gr, void *map, int len, int has_alpha);
   void (*gradient_alpha_data_set)(XR_Gradient *gr, void *alpha_map, int len);
   void (*gradient_clear)(XR_Gradient *gr);
   void (*gradient_fill_set)(XR_Gradient *gr, int x, int y, int w, int h);
   void (*gradient_fill_angle_set)(XR_Gradient *gr, double angle);
   void (*gradient_fill_spread_set)(XR_Gradient *gr, int spread);
   void (*gradient_angle_set)(XR_Gradient *gr, double angle);
   void (*gradient_offset_set)(XR_Gradient *gr, float offset);
   void (*gradient_direction_set)(XR_Gradient *gr, int direction);
   void (*gradient_type_set)(XR_Gradient *gr, char *name, char *params);
   void (*gradient_draw)(Xrender_Surface *rs, RGBA_Draw_Context *dc, XR_Gradient *gr, int x, int y, int w, int h);

   XR_Image *(*image_load)(Ximage_Info *xinf, const char *file, const char *key, Evas_Image_Load_Opts *lo);
   XR_Image *(*image_new_from_data)(Ximage_Info *xinf, int w, int h, void *data, int alpha, int cspace);
   XR_Image *(*image_new_from_copied_data)(Ximage_Info *xinf, int w, int h, void *data, int alpha, int cspace);
   XR_Image *(*image_new)(Ximage_Info *xinf, int w, int h);
   void (*image_resize)(XR_Image *im, int w, int h);
   void (*image_free)(XR_Image *im);
   void (*image_region_dirty)(XR_Image *im, int x, int y, int w, int h);
   void (*image_dirty)(XR_Image *im);
   XR_Image *(*image_copy)(XR_Image *im);
   void *(*image_data_get)(XR_Image *im);
   XR_Image *(*image_data_find)(void *data);
   void (*image_data_put)(XR_Image *im, void *data);
   void (*image_alpha_set)(XR_Image *im, int alpha);
   int  (*image_alpha_get)(XR_Image *im);
   void (*image_border_set)(XR_Image *im, int l, int r, int t, int b);
   void (*image_border_get)(XR_Image *im, int *l, int *r, int *t, int *b);
   void (*image_surface_gen)(XR_Image *im);
   void (*image_cache_set)(int size);
   int  (*image_cache_get)(void);

   Ximage_Info *(*ximage_info_get)(void *connection, unsigned int draw, void *vis);
   void (*ximage_info_free)(Ximage_Info *xinf);
   void (*ximage_info_pool_flush)(Ximage_Info *xinf, int max_num, int max_mem);
   Ximage_Image *(*ximage_new)(Ximage_Info *xinf, int w, int h, int depth);
   void (*ximage_free)(Ximage_Image *xim);
   void (*ximage_put)(Ximage_Image *xim, Drawable draw, int x, int y, int w, int h);

   Xrender_Surface *(*render_surface_new)(Ximage_Info *xinf, int w, int h, XRenderPictFormat *fmt, int alpha);
   Xrender_Surface *(*render_surface_adopt)(Ximage_Info *xinf, unsigned int draw, int w, int h, int alpha);
   Xrender_Surface *(*render_surface_format_adopt)(Ximage_Info *xinf, unsigned int draw, int w, int h, void *fmt, int alpha);
   void (*render_surface_free)(Xrender_Surface *rs);
   void (*render_surface_repeat_set)(Xrender_Surface *rs, int repeat);
   void (*render_surface_solid_rectangle_set)(Xrender_Surface *rs, int r, int g, int b, int a, int x, int y, int w, int h);
   void (*render_surface_argb_pixels_fill)(Xrender_Surface *rs, int sw, int sh, void *pixels, int x, int y, int w, int h, int ox, int oy);
   void (*render_surface_rgb_pixels_fill)(Xrender_Surface *rs, int sw, int sh __UNUSED__, void *pixels, int x, int y, int w, int h, int ox, int oy);
   void (*render_surface_clips_set)(Xrender_Surface *rs, RGBA_Draw_Context *dc, int rx, int ry, int rw, int rh);
   void (*render_surface_composite)(Xrender_Surface *srs, Xrender_Surface *drs, RGBA_Draw_Context *dc, int sx, int sy, int sw, int sh, int x, int y, int w, int h, int smooth);
   void (*render_surface_copy)(Xrender_Surface *srs, Xrender_Surface *drs, int sx, int sy, int x, int y, int w, int h);
   void (*render_surface_rectangle_draw)(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x, int y, int w, int h);
   void (*render_surface_line_draw)(Xrender_Surface *rs, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
   void (*render_surface_polygon_draw)(Xrender_Surface *rs, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);
};

/* internal engine routines */

#ifdef BUILD_ENGINE_XRENDER_X11

static void
_xlib_sync(Render_Engine *re)
{
   XSync((Display *)re->x11.connection, False);
}

static Render_Engine *
_output_xlib_setup(int           width,
                   int           height,
                   unsigned char destination_alpha,
                   void         *connection,
                   unsigned int  draw,
                   unsigned int  mask,
                   void         *visual)
{
   Render_Engine *re;

   re = calloc(1, sizeof(Render_Engine));
   if (!re)
     return NULL;

   re->x11.connection = connection;
   re->x11.screen = NULL;
   re->x11.window = draw;
   re->x11.mask = mask;
   re->x11.visual = visual;

   if (re->xinf) _xr_xlib_image_info_free(re->xinf);
   re->xinf = _xr_xlib_image_info_get((Display *)re->x11.connection, (Drawable)re->x11.window, (Visual *)re->x11.visual);

   if (!re->x11.mask)
     re->output = _xr_xlib_render_surface_adopt(re->xinf, (Drawable)re->x11.window, width, height, destination_alpha);
   else
     re->output = _xr_xlib_render_surface_adopt(re->xinf, (Drawable)re->x11.window, width, height, 0);
   if (re->x11.mask)
     re->mask_output = _xr_xlib_render_surface_format_adopt(re->xinf,
                                                            (Drawable)re->x11.mask,
                                                            width, height,
                                                            re->xinf->x11.fmt1, 1);
   else
     re->mask_output = NULL;

//   if (re->output) _xr_xlib_render_surface_free(re->output);
//   if (re->mask_output) _xr_xlib_render_surface_free(re->mask_output);

   re->x11.sync = _xlib_sync;

   re->font_surface_new = _xre_xlib_font_surface_new;
   re->font_surface_free = _xre_xlib_font_surface_free;
   re->font_surface_draw = _xre_xlib_font_surface_draw;

   re->gradient_new = _xre_xlib_gradient_new;
   re->gradient_free = _xre_xlib_gradient_free;
   re->gradient_color_stop_add = _xre_xlib_gradient_color_stop_add;
   re->gradient_alpha_stop_add = _xre_xlib_gradient_alpha_stop_add;
   re->gradient_color_data_set = _xre_xlib_gradient_color_data_set;
   re->gradient_alpha_data_set = _xre_xlib_gradient_alpha_data_set;
   re->gradient_clear = _xre_xlib_gradient_clear;
   re->gradient_fill_set = _xre_xlib_gradient_fill_set;
   re->gradient_fill_angle_set = _xre_xlib_gradient_fill_angle_set;
   re->gradient_fill_spread_set = _xre_xlib_gradient_fill_spread_set;
   re->gradient_angle_set = _xre_xlib_gradient_angle_set;
   re->gradient_offset_set = _xre_xlib_gradient_offset_set;
   re->gradient_direction_set = _xre_xlib_gradient_direction_set;
   re->gradient_type_set = _xre_xlib_gradient_type_set;
   re->gradient_draw = _xre_xlib_gradient_draw;

   re->image_load = _xre_xlib_image_load;
   re->image_new_from_data = _xre_xlib_image_new_from_data;
   re->image_new_from_copied_data = _xre_xlib_image_new_from_copied_data;
   re->image_new = _xre_xlib_image_new;
   re->image_resize = _xre_xlib_image_resize;
   re->image_free = _xre_xlib_image_free;
   re->image_region_dirty = _xre_xlib_image_region_dirty;
   re->image_dirty = _xre_xlib_image_dirty;
   re->image_copy = _xre_xlib_image_copy;
   re->image_data_get = _xre_xlib_image_data_get;
   re->image_data_find = _xre_xlib_image_data_find;
   re->image_data_put = _xre_xlib_image_data_put;
   re->image_alpha_set = _xre_xlib_image_alpha_set;
   re->image_alpha_get = _xre_xlib_image_alpha_get;
   re->image_border_set = _xre_xlib_image_border_set;
   re->image_border_get = _xre_xlib_image_border_get;
   re->image_surface_gen = _xre_xlib_image_surface_gen;
   re->image_cache_set = _xre_xlib_image_cache_set;
   re->image_cache_get = _xre_xlib_image_cache_get;

   re->ximage_info_get = _xr_xlib_image_info_get;
   re->ximage_info_free = _xr_xlib_image_info_free;
   re->ximage_info_pool_flush = _xr_xlib_image_info_pool_flush;
   re->ximage_new = _xr_xlib_image_new;
   re->ximage_free = _xr_xlib_image_free;
   re->ximage_put = _xr_xlib_image_put;

   re->render_surface_new = _xr_xlib_render_surface_new;
   re->render_surface_adopt = _xr_xlib_render_surface_adopt;
   re->render_surface_format_adopt = _xr_xlib_render_surface_format_adopt;
   re->render_surface_free = _xr_xlib_render_surface_free;
   re->render_surface_repeat_set = _xr_xlib_render_surface_repeat_set;
   re->render_surface_solid_rectangle_set = _xr_xlib_render_surface_solid_rectangle_set;
   re->render_surface_argb_pixels_fill = _xr_xlib_render_surface_argb_pixels_fill;
   re->render_surface_rgb_pixels_fill = _xr_xlib_render_surface_rgb_pixels_fill;
   re->render_surface_clips_set = _xr_xlib_render_surface_clips_set;
   re->render_surface_composite = _xr_xlib_render_surface_composite;
   re->render_surface_copy = _xr_xlib_render_surface_copy;
   re->render_surface_rectangle_draw = _xr_xlib_render_surface_rectangle_draw;
   re->render_surface_line_draw = _xr_xlib_render_surface_line_draw;
   re->render_surface_polygon_draw = _xr_xlib_render_surface_polygon_draw;

   return re;
}

#endif /* BUILD_ENGINE_XRENDER_X11 */

#ifdef BUILD_ENGINE_XRENDER_XCB

static void
_xcb_sync(Render_Engine *re)
{
   xcb_get_input_focus_reply_t *reply;

   reply = xcb_get_input_focus_reply(re->x11.connection,
                                     xcb_get_input_focus_unchecked(re->x11.connection),
                                     NULL);
   if (reply)
     free(reply);
}

static Render_Engine *
_output_xcb_setup(int           width,
                  int           height,
                  unsigned char destination_alpha,
                  void         *connection,
                  void         *screen,
                  unsigned int  draw,
                  unsigned int  mask,
                  void         *visual)
{
   Render_Engine *re;

   re = calloc(1, sizeof(Render_Engine));
   if (!re)
     return NULL;

   re->x11.connection = connection;
   re->x11.screen = screen;
   re->x11.window = draw;
   re->x11.mask = mask;
   re->x11.visual = visual;

   if (re->xinf) _xr_xcb_image_info_free(re->xinf);
   re->xinf = _xr_xcb_image_info_get((xcb_connection_t *)re->x11.connection, (xcb_screen_t *)re->x11.screen, (xcb_drawable_t)re->x11.window, (xcb_visualtype_t *)re->x11.visual);

   if (!re->x11.mask)
     re->output = _xr_xcb_render_surface_adopt(re->xinf, (Drawable)re->x11.window, width, height, destination_alpha);
   else
     re->output = _xr_xcb_render_surface_adopt(re->xinf, (Drawable)re->x11.window, width, height, 0);
   if (re->x11.mask)
     re->mask_output = _xr_xcb_render_surface_format_adopt(re->xinf,
                                                            (Drawable)re->x11.mask,
                                                            width, height,
                                                            re->xinf->x11.fmt1, 1);
   else
     re->mask_output = NULL;

   if (re->output) _xr_xcb_render_surface_free(re->output);
   if (re->mask_output) _xr_xcb_render_surface_free(re->mask_output);

   re->x11.sync = _xcb_sync;

   re->font_surface_new = _xre_xcb_font_surface_new;
   re->font_surface_free = _xre_xcb_font_surface_free;
   re->font_surface_draw = _xre_xcb_font_surface_draw;

   re->gradient_new = _xre_xcb_gradient_new;
   re->gradient_free = _xre_xcb_gradient_free;
   re->gradient_color_stop_add = _xre_xcb_gradient_color_stop_add;
   re->gradient_alpha_stop_add = _xre_xcb_gradient_alpha_stop_add;
   re->gradient_color_data_set = _xre_xcb_gradient_color_data_set;
   re->gradient_alpha_data_set = _xre_xcb_gradient_alpha_data_set;
   re->gradient_clear = _xre_xcb_gradient_clear;
   re->gradient_fill_set = _xre_xcb_gradient_fill_set;
   re->gradient_fill_angle_set = _xre_xcb_gradient_fill_angle_set;
   re->gradient_fill_spread_set = _xre_xcb_gradient_fill_spread_set;
   re->gradient_angle_set = _xre_xcb_gradient_angle_set;
   re->gradient_offset_set = _xre_xcb_gradient_offset_set;
   re->gradient_direction_set = _xre_xcb_gradient_direction_set;
   re->gradient_type_set = _xre_xcb_gradient_type_set;
   re->gradient_draw = _xre_xcb_gradient_draw;

   re->image_load = _xre_xcb_image_load;
   re->image_new_from_data = _xre_xcb_image_new_from_data;
   re->image_new_from_copied_data = _xre_xcb_image_new_from_copied_data;
   re->image_new = _xre_xcb_image_new;
   re->image_resize = _xre_xcb_image_resize;
   re->image_free = _xre_xcb_image_free;
   re->image_region_dirty = _xre_xcb_image_region_dirty;
   re->image_dirty = _xre_xcb_image_dirty;
   re->image_copy = _xre_xcb_image_copy;
   re->image_data_get = _xre_xcb_image_data_get;
   re->image_data_find = _xre_xcb_image_data_find;
   re->image_data_put = _xre_xcb_image_data_put;
   re->image_alpha_set = _xre_xcb_image_alpha_set;
   re->image_alpha_get = _xre_xcb_image_alpha_get;
   re->image_border_set = _xre_xcb_image_border_set;
   re->image_border_get = _xre_xcb_image_border_get;
   re->image_surface_gen = _xre_xcb_image_surface_gen;
   re->image_cache_set = _xre_xcb_image_cache_set;
   re->image_cache_get = _xre_xcb_image_cache_get;

   re->ximage_info_get = _xr_xcb_image_info_get;
   re->ximage_info_free = _xr_xcb_image_info_free;
   re->ximage_info_pool_flush = _xr_xcb_image_info_pool_flush;
   re->ximage_new = _xr_xcb_image_new;
   re->ximage_free = _xr_xcb_image_free;
   re->ximage_put = _xr_xcb_image_put;

   re->render_surface_new = _xr_xcb_render_surface_new;
   re->render_surface_adopt = _xr_xcb_render_surface_adopt;
   re->render_surface_format_adopt = _xr_xcb_render_surface_format_adopt;
   re->render_surface_free = _xr_xcb_render_surface_free;
   re->render_surface_repeat_set = _xr_xcb_render_surface_repeat_set;
   re->render_surface_solid_rectangle_set = _xr_xcb_render_surface_solid_rectangle_set;
   re->render_surface_argb_pixels_fill = _xr_xcb_render_surface_argb_pixels_fill;
   re->render_surface_rgb_pixels_fill = _xr_xcb_render_surface_rgb_pixels_fill;
   re->render_surface_clips_set = _xr_xcb_render_surface_clips_set;
   re->render_surface_composite = _xr_xcb_render_surface_composite;
   re->render_surface_copy = _xr_xcb_render_surface_copy;
   re->render_surface_rectangle_draw = _xr_xcb_render_surface_rectangle_draw;
   re->render_surface_line_draw = _xr_xcb_render_surface_line_draw;
   re->render_surface_polygon_draw = _xr_xcb_render_surface_polygon_draw;

   return re;
}

#endif /* BUILD_ENGINE_XRENDER_XCB */

/* engine api this module provides */
static void *
eng_info(Evas *e __UNUSED__)
{
   Evas_Engine_Info_XRender_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_XRender_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
}

static void
eng_info_free(Evas *e __UNUSED__, void *info)
{
   Evas_Engine_Info_XRender_X11 *in;

   in = (Evas_Engine_Info_XRender_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_XRender_X11 *info;
   int resize = 1;

   info = (Evas_Engine_Info_XRender_X11 *)in;
   if (!e->engine.data.output)
     {
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

#ifdef BUILD_ENGINE_XRENDER_X11
        if (info->info.backend == 0)
          {
             re = _output_xlib_setup(e->output.w,
                                     e->output.h,
                                     info->info.destination_alpha,
                                     info->info.connection,
                                     info->info.drawable,
                                     info->info.mask,
                                     info->info.visual);
          }
#endif /* BUILD_ENGINE_XRENDER_X11 */

#ifdef BUILD_ENGINE_XRENDER_XCB
        if (info->info.backend == 1)
          {
             re = _output_xcb_setup(e->output.w,
                                    e->output.h,
                                    info->info.destination_alpha,
                                    info->info.connection,
                                    info->info.screen,
                                    info->info.drawable,
                                    info->info.mask,
                                    info->info.visual);
          }
#endif /* BUILD_ENGINE_XRENDER_XCB */

        if (!re)
          return;

	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
	e->engine.data.output = re;
	resize = 0;
     }
   re = e->engine.data.output;
   if (!re) return;

   if (!e->engine.data.context) e->engine.data.context = e->engine.func->context_new(e->engine.data.output);

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
	re->updates = eina_list_remove_list(re->updates, re->updates);
	re->render_surface_free(reu->surface);
	free(reu);
     }
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->output) re->render_surface_free(re->output);
   if (re->mask_output) re->render_surface_free(re->mask_output);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   if (re->xinf) re->ximage_info_free(re->xinf);
   free(re);
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->output)
     {
	if ((re->output->width == w) && (re->output->height == h)) return;
	if (re->output) re->render_surface_free(re->output);
     }
   re->output = re->render_surface_adopt(re->xinf, re->x11.window, w, h, 0);
   if (re->mask_output)
     {
	if (re->mask_output) re->render_surface_free(re->mask_output);
	re->mask_output = re->render_surface_format_adopt(re->xinf,
                                                          re->x11.mask,
                                                          w, h,
                                                          re->xinf->x11.fmt1, 1);
     }
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
	re->cur_rect = EINA_INLIST_GET(re->rects);
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
   if ((re->destination_alpha) || (re->x11.mask))
     {
	Xrender_Surface *surface;

	surface = re->render_surface_new(re->xinf, uw, uh, re->xinf->x11.fmt32, 1);
	re->render_surface_solid_rectangle_set(surface, 0, 0, 0, 0, 0, 0, uw, uh);
	return surface;
     }
// use target format to avoid conversion to depth when copying to screen
//   return _xr_render_surface_new(re->xinf, uw, uh, re->xinf->fmtdef, 0);
// use 24/32bpp for tmp buf for better quality. rendering in 24/32bpp
   return re->render_surface_new(re->xinf, uw, uh, re->xinf->x11.fmt24, 0);
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
   reu->surface = (Xrender_Surface *)surface;
   re->updates = eina_list_append(re->updates, reu);
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
	re->updates = eina_list_remove_list(re->updates, re->updates);
	if (re->mask_output)
	  {
	     Xrender_Surface *tsurf;

	     re->render_surface_copy(reu->surface, re->output, 0, 0,
				     reu->x, reu->y, reu->w, reu->h);
	     tsurf = re->render_surface_new(re->xinf, reu->w, reu->h, re->xinf->x11.fmt1, 1);
	     if (tsurf)
	       {
		  re->render_surface_copy(reu->surface, tsurf, 0, 0,
					  0, 0, reu->w, reu->h);
		  re->render_surface_copy(tsurf, re->mask_output, 0, 0,
					  reu->x, reu->y, reu->w, reu->h);
		  re->render_surface_free(tsurf);
	       }
	  }
	else
	  {
	     re->render_surface_copy(reu->surface, re->output, 0, 0,
				     reu->x, reu->y, reu->w, reu->h);
	  }
	re->render_surface_free(reu->surface);
	free(reu);
     }
   re->x11.sync(re);
   re->ximage_info_pool_flush(re->xinf, 0, 0);
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
   Render_Engine *re;

   re = (Render_Engine *)data;

   re->render_surface_rectangle_draw((Xrender_Surface *)surface,
                                     (RGBA_Draw_Context *)context,
                                     x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   re->render_surface_line_draw((Xrender_Surface *)surface, (RGBA_Draw_Context *)context, x1, y1, x2, y2);
}

static void
eng_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   re->render_surface_polygon_draw((Xrender_Surface *)surface, (RGBA_Draw_Context *)context, (RGBA_Polygon_Point *)polygon);
}


static void
eng_gradient2_color_np_stop_insert(void *data __UNUSED__, void *gradient __UNUSED__, int r __UNUSED__, int g __UNUSED__, int b __UNUSED__, int a __UNUSED__, float pos __UNUSED__)
{
}

static void
eng_gradient2_clear(void *data __UNUSED__, void *gradient __UNUSED__)
{
}

static void
eng_gradient2_fill_transform_set(void *data __UNUSED__, void *gradient __UNUSED__, void *transform __UNUSED__)
{
}

static void
eng_gradient2_fill_spread_set(void *data __UNUSED__, void *gradient __UNUSED__, int spread __UNUSED__)
{
}

static void *
eng_gradient2_linear_new(void *data __UNUSED__)
{
   return NULL;
}

static void
eng_gradient2_linear_free(void *data __UNUSED__, void *linear_gradient __UNUSED__)
{
}

static void
eng_gradient2_linear_fill_set(void *data __UNUSED__, void *linear_gradient __UNUSED__, int x0 __UNUSED__, int y0 __UNUSED__, int x1 __UNUSED__, int y1 __UNUSED__)
{
}

static int
eng_gradient2_linear_is_opaque(void *data __UNUSED__, void *context __UNUSED__, void *linear_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return 1;
}

static int
eng_gradient2_linear_is_visible(void *data __UNUSED__, void *context __UNUSED__, void *linear_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return 1;
}

static void
eng_gradient2_linear_render_pre(void *data __UNUSED__, void *context __UNUSED__, void *linear_gradient __UNUSED__)
{
}

static void
eng_gradient2_linear_render_post(void *data __UNUSED__, void *linear_gradient __UNUSED__)
{
}

static void
eng_gradient2_linear_draw(void *data __UNUSED__, void *context __UNUSED__, void *surface __UNUSED__, void *linear_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
}

static void *
eng_gradient2_radial_new(void *data __UNUSED__)
{
   return NULL;
}

static void
eng_gradient2_radial_free(void *data __UNUSED__, void *radial_gradient __UNUSED__)
{
}

static void
eng_gradient2_radial_fill_set(void *data __UNUSED__, void *radial_gradient __UNUSED__, float cx __UNUSED__, float cy __UNUSED__, float rx __UNUSED__, float ry __UNUSED__)
{
}

static int
eng_gradient2_radial_is_opaque(void *data __UNUSED__, void *context __UNUSED__, void *radial_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return 1;
}

static int
eng_gradient2_radial_is_visible(void *data __UNUSED__, void *context __UNUSED__, void *radial_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   return 1;
}

static void
eng_gradient2_radial_render_pre(void *data __UNUSED__, void *context __UNUSED__, void *radial_gradient __UNUSED__)
{
}

static void
eng_gradient2_radial_render_post(void *data __UNUSED__, void *radial_gradient __UNUSED__)
{
}

static void
eng_gradient2_radial_draw(void *data __UNUSED__, void *context __UNUSED__, void *surface __UNUSED__, void *radial_gradient __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
}

static void *
eng_gradient_new(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   return re->gradient_new(re->xinf);
}

static void
eng_gradient_free(void *data, void *gradient)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_free(gradient);
}

static void
eng_gradient_color_stop_add(void *data, void *gradient, int r, int g, int b, int a, int delta)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_color_stop_add(gradient, r, g, b, a, delta);
}

static void
eng_gradient_alpha_stop_add(void *data, void *gradient, int a, int delta)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_alpha_stop_add(gradient, a, delta);
}

static void
eng_gradient_color_data_set(void *data, void *gradient, void *map, int len, int has_alpha)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_color_data_set(gradient, map, len, has_alpha);
}

static void
eng_gradient_alpha_data_set(void *data, void *gradient, void *alpha_map, int len)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_alpha_data_set(gradient, alpha_map, len);
}

static void
eng_gradient_clear(void *data, void *gradient)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_clear(gradient);
}

static void
eng_gradient_fill_set(void *data, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_fill_set(gradient, x, y, w, h);
}

static void
eng_gradient_fill_angle_set(void *data, void *gradient, double angle)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_fill_angle_set(gradient, angle);
}

static void
eng_gradient_fill_spread_set(void *data, void *gradient, int spread)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_fill_spread_set(gradient, spread);
}

static void
eng_gradient_angle_set(void *data, void *gradient, double angle)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_angle_set(gradient, angle);
}

static void
eng_gradient_offset_set(void *data, void *gradient, float offset)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_offset_set(gradient, offset);
}

static void
eng_gradient_direction_set(void *data, void *gradient, int direction)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_direction_set(gradient, direction);
}

static void
eng_gradient_type_set(void *data, void *gradient, char *name, char *params)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_type_set(gradient, name, params);
}

static int
eng_gradient_is_opaque(void *data __UNUSED__, void *context, void *gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   RGBA_Gradient  *grad;
   RGBA_Draw_Context *dc = (RGBA_Draw_Context *)context;

   if (!dc || !gradient) return 0;
   grad = ((XR_Gradient *)gradient)->grad;
   if(!grad || !grad->type.geometer)  return 0;
   return !(grad->type.geometer->has_alpha(grad, dc->render_op) |
              grad->type.geometer->has_mask(grad, dc->render_op));
}

static int
eng_gradient_is_visible(void *data __UNUSED__, void *context, void *gradient, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   if (!context || !gradient)  return 0;
   return 1;
}

static void
eng_gradient_render_pre(void *data __UNUSED__, void *context, void *gradient)
{
   int  len;
   RGBA_Gradient  *grad;

   if (!context || !gradient) return;
   grad = ((XR_Gradient *)gradient)->grad;
   if(!grad || !grad->type.geometer)  return;
   grad->type.geometer->geom_set(grad);
   len = grad->type.geometer->get_map_len(grad);
   evas_common_gradient_map(context, grad, len);
}

static void
eng_gradient_render_post(void *data __UNUSED__, void *gradient __UNUSED__)
{
}

static void
eng_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = (Render_Engine *)data;

   re->gradient_draw(surface, context, gradient, x, y, w, h);
}

static int
eng_image_alpha_get(void *data, void *image)
{
   Render_Engine *re;

   if (!image) return 0;

   re = (Render_Engine *)data;
   return re->image_alpha_get((XR_Image *)image);
}

static int
eng_image_colorspace_get(void *data __UNUSED__, void *image)
{
   if (!image) return EVAS_COLORSPACE_ARGB8888;
   return ((XR_Image *)image)->cs.space;
}

static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   XR_Image *im;

   im = (XR_Image *)image;
   if (!im) return im;
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if (((im->alpha) && (has_alpha)) || ((!im->alpha) && (!has_alpha)))
     return im;
   re = (Render_Engine *)data;
   if (im->references > 1)
     {
	XR_Image *old_im;

	old_im = im;
	im = re->image_copy(old_im);
	if (im)
	  {
	     im->alpha = old_im->alpha;
	     re->image_free(old_im);
	  }
	else
	  im = old_im;
     }
   else
     re->image_dirty(im);
   re->image_alpha_set(im, has_alpha);
   return im;
}

static void *
eng_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   Render_Engine *re;

   if (!image) return image;
   re = (Render_Engine *)data;
   re->image_border_set((XR_Image *)image, l, r, t, b);
   return image;
}

static void
eng_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   Render_Engine *re;

   if (!image) return;
   re = (Render_Engine *)data;
   re->image_border_get((XR_Image *)image, l, r, t, b);
}

static char *
eng_image_comment_get(void *data __UNUSED__, void *image, char *key __UNUSED__)
{
   if (!image) return NULL;
   return strdup(((XR_Image *)image)->comment);
}

static char *
eng_image_format_get(void *data __UNUSED__, void *image)
{
   if (!image) return NULL;
   return ((XR_Image *)image)->format;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Render_Engine *re;
   XR_Image *im;

   if (!image) return;
   im = (XR_Image *)image;
   if (im->cs.space == cspace) return;

   if (im->im) evas_cache_image_drop(&im->im->cache_entry);
   im->im = NULL;

   re = (Render_Engine *)data;
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	     im->cs.data = NULL;
	     im->cs.no_free = 0;
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if ((im->free_data) && (im->data)) free(im->data);
	im->data = NULL;
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	  }
	if (im->h > 0)
	  im->cs.data = calloc(1, im->h * sizeof(unsigned char *) * 2);
	im->cs.no_free = 0;
	break;
      default:
	abort();
	break;
     }
   im->cs.space = cspace;
   re->image_dirty(im);
   re->image_region_dirty(im, 0, 0, im->w, im->h);
}

static void
eng_image_native_set(void *data __UNUSED__, void *image __UNUSED__, void *native __UNUSED__)
{
}

static void *
eng_image_native_get(void *data __UNUSED__, void *image __UNUSED__)
{
   return NULL;
}

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;
   XR_Image *im;

   re = (Render_Engine *)data;
   *error = 0;
   im = re->image_load(re->xinf, file, key, lo);
   return im;
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;
   XR_Image *im;

   re = (Render_Engine *)data;
   im = re->image_new_from_data(re->xinf, w, h, image_data, alpha, cspace);
   return im;
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;
   XR_Image *im;

   re = (Render_Engine *)data;
   im = re->image_new_from_copied_data(re->xinf, w, h, image_data, alpha, cspace);
   return im;
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   if (!image) return;
   re = (Render_Engine *)data;
   re->image_free((XR_Image *)image);
}

static void
eng_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
{
   if (!image) return;
   if (w) *w = ((XR_Image *)image)->w;
   if (h) *h = ((XR_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   XR_Image *im, *im_old;

   if (!image) return NULL;
   re = (Render_Engine *)data;
   im_old = image;
   if ((im_old->cs.space == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (im_old->cs.space == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;
   if ((im_old) && (im_old->w == w) && (im_old->h == h))
     return image;
   if ((w <= 0) || (h <= 0))
     {
	re->image_free(im_old);
	return NULL;
     }
   if (im_old)
     {
	im = re->image_new_from_copied_data(im_old->xinf, w, h, NULL, im_old->alpha, im_old->cs.space);
	re->image_free(im_old);
	return im;
     }
   return image;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;

   if (!image) return image;
   re = (Render_Engine *)data;
   re->image_dirty((XR_Image *)image);
   re->image_region_dirty((XR_Image *)image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   XR_Image *im;

   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   re = (Render_Engine *)data;
   im = (XR_Image *)image;
   if (im->im)
     evas_cache_image_load_data(&im->im->cache_entry);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
	  {
	     if (im->references > 1)
	       {
		  XR_Image *im_old;

		  im_old = im;
		  im = re->image_copy(im_old);
		  if (im)
		    re->image_free(im_old);
		  else
		    im = im_old;
	       }
	     else
	       re->image_dirty(im);
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	break;
      default:
	abort();
	break;
     }
   if (image_data) *image_data = re->image_data_get(im);
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   XR_Image *im;

   if (!image) return image;
   re = (Render_Engine *)data;
   im = (XR_Image *)image;

   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (re->image_data_get(im) != image_data)
	  {
	     XR_Image *im_old;

	     im_old = im;
	     image = re->image_data_find(image_data);
	     if (image != im_old)
	       {
		  if (!image)
		    {
		       image = re->image_new_from_data(im_old->xinf, im_old->w, im_old->h, image_data, im_old->alpha, EVAS_COLORSPACE_ARGB8888);
		       if (image)
			 {
			    ((XR_Image *)image)->alpha = im_old->alpha;
			    re->image_free(im_old);
			 }
		       else
			 image = im_old;
		    }
		  else
		    {
		       re->image_free(im_old);
		    }
	       }
	     else
	       {
		  re->image_free(image);
		  image = im_old;
	       }
	  }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	if (re->image_data_get(im) != image_data)
	  {
	     if (im->data)
	       {
		  if (im->free_data) free(im->data);
		  im->data = NULL;
	       }
             if (im->cs.data)
	       {
		  if (!im->cs.no_free) free(im->cs.data);
	       }
	     im->cs.data = image_data;
	     re->image_dirty(im);
	  }
        break;
      default:
	abort();
	break;
     }
   return image;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   XR_Image *xim = image;
   RGBA_Image *im;

   if (!xim) return ;
   im = (RGBA_Image*) xim->im;
   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   XR_Image *xim = image;
   RGBA_Image *im;

   if (!xim) return ;
   im = (RGBA_Image*) xim->im;
   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   if ((!image) || (!surface)) return;

   re = (Render_Engine *)data;
   re->image_surface_gen((XR_Image *)image);
   if (((XR_Image *)image)->surface)
     re->render_surface_composite(((XR_Image *)image)->surface,
                                  (Xrender_Surface *)surface,
                                  (RGBA_Draw_Context *)context,
                                  src_x, src_y, src_w, src_h,
                                  dst_x, dst_y, dst_w, dst_h,
                                  smooth);
}

static void
eng_image_cache_flush(void *data)
{
   Render_Engine *re;
   int tmp_size;

   re = (Render_Engine *)data;
   tmp_size = re->image_cache_get();
   pfunc.image_cache_flush(data);
   re->image_cache_set(0);
   re->image_cache_set(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   pfunc.image_cache_set(data, bytes);
   re->image_cache_set(bytes);
}

static int
eng_image_cache_get(void *data)
{
   return pfunc.image_cache_get(data);
}

static void
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow __UNUSED__, int oh __UNUSED__, const char *text)
{
   Render_Engine        *re;
   RGBA_Image           *im;

   re = (Render_Engine *)data;

   re->render_surface_clips_set((Xrender_Surface *)surface, (RGBA_Draw_Context *)context, x, y, w, h);

   im = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                             ((Xrender_Surface *)surface)->width,
                                             ((Xrender_Surface *)surface)->height,
                                             surface,
                                             0, EVAS_COLORSPACE_ARGB8888);
   evas_common_draw_context_font_ext_set((RGBA_Draw_Context *)context,
                                         re->xinf,
                                         re->font_surface_new,
                                         re->font_surface_free,
                                         re->font_surface_draw);
   evas_common_font_draw(im, context, font, x, y, text);
   evas_common_draw_context_font_ext_set(context,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);
   evas_common_cpu_end_opt();

   evas_cache_image_drop(&im->cache_entry);
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

   ORD(gradient2_color_np_stop_insert);
   ORD(gradient2_clear);
   ORD(gradient2_fill_transform_set);
   ORD(gradient2_fill_spread_set);
   ORD(gradient2_linear_new);
   ORD(gradient2_linear_free);
   ORD(gradient2_linear_fill_set);
   ORD(gradient2_linear_is_opaque);
   ORD(gradient2_linear_is_visible);
   ORD(gradient2_linear_render_pre);
   ORD(gradient2_linear_render_post);
   ORD(gradient2_linear_draw);
   ORD(gradient2_radial_new);
   ORD(gradient2_radial_free);
   ORD(gradient2_radial_fill_set);
   ORD(gradient2_radial_is_opaque);
   ORD(gradient2_radial_is_visible);
   ORD(gradient2_radial_render_pre);
   ORD(gradient2_radial_render_post);
   ORD(gradient2_radial_draw);

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
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_native_set);
   ORD(image_native_get);
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
     "xrender_x11",
     "none"
};
