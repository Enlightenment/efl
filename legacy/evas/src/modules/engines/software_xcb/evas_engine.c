#include <xcb/xcb.h>
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_Xcb.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Tilebuf          *tb;
   Outbuf           *ob;
   Tilebuf_Rect     *rects;
   Evas_Object_List *cur_rect;
   int               end : 1;
};

/* prototypes we will use here */
static void *_output_setup(int w, int h, int rot, xcb_connection_t *conn, xcb_drawable_t draw, xcb_visualtype_t *vis, xcb_colormap_t cmap, int depth, int debug, int grayscale, int max_colors, xcb_drawable_t mask, int shape_dither, int destination_alpha);
static xcb_visualtype_t *_best_visual_get(xcb_connection_t *conn, int screen);
static xcb_colormap_t _best_colormap_get(xcb_connection_t *conn, int screen);
static int _best_depth_get(xcb_connection_t *conn, int screen);
static Evas_Performance *_output_perf_new(Evas *e, xcb_connection_t *conn, xcb_visualtype_t *vis, xcb_colormap_t cmap, xcb_drawable_t draw, int depth);
static Evas_Performance *_output_perf_test(Evas *e, xcb_connection_t *conn, xcb_visualtype_t *vis, xcb_colormap_t cmap, xcb_drawable_t draw, int depth);
static char *_output_perf_data(Evas_Performance *perf);
static char *_output_perf_key(Evas_Performance *perf);
static void _output_perf_free(Evas_Performance *perf);
static void _output_perf_build(Evas_Performance *perf, const char *data);
static void _output_perf_device_store(Evas_Performance *perf);

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

static void *
_output_setup(int               w,
	      int               h,
	      int               rot,
	      xcb_connection_t *conn,
	      xcb_drawable_t    draw,
	      xcb_visualtype_t *vis,
	      xcb_colormap_t    cmap,
	      int               depth,
	      int               debug,
	      int               grayscale,
	      int               max_colors,
	      xcb_drawable_t    mask,
	      int               shape_dither,
              int               destination_alpha)
{
   Render_Engine *re;
   Outbuf_Perf   *perf;

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

   evas_software_xcb_x_init();
   evas_software_xcb_x_color_init();
   evas_software_xcb_outbuf_init();

   /* get any stored performance metrics from device (xserver) */
   perf = evas_software_xcb_outbuf_perf_restore_x(conn, draw, vis, cmap, depth);
   re->ob = evas_software_xcb_outbuf_setup_x(w, h, rot,
					     OUTBUF_DEPTH_INHERIT,
					     conn,
					     draw,
					     vis,
					     cmap,
					     depth,
					     perf,
					     grayscale,
					     max_colors,
					     mask,
                                             shape_dither,
                                             destination_alpha);
   if (!re->ob)
     {
        evas_software_xcb_outbuf_perf_free(perf);
	free(re);
	return NULL;
     }
   evas_software_xcb_outbuf_debug_set(re->ob, debug);
   re->tb = evas_common_tilebuf_new(w, h);
   if (!re->tb)
     {
	evas_software_xcb_outbuf_free(re->ob);
	free(re);
	return NULL;
     }
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   return re;
}

static xcb_visualtype_t *
_best_visual_get(xcb_connection_t *conn, int screen)
{
   xcb_screen_iterator_t iter_screen;
   xcb_depth_iterator_t  iter_depth;
   xcb_screen_t         *scr = NULL;

   if (!conn) return NULL;

   iter_screen = xcb_setup_roots_iterator (xcb_get_setup (conn));
   for (; iter_screen.rem; --screen, xcb_screen_next (&iter_screen))
      if (screen == 0)
        {
           scr = iter_screen.data;
           break;
        }
   if (!scr) return NULL;

   iter_depth = xcb_screen_allowed_depths_iterator(scr);
   for (; iter_depth.rem; xcb_depth_next (&iter_depth))
     {
        xcb_visualtype_iterator_t iter_vis;

        iter_vis = xcb_depth_visuals_iterator(iter_depth.data);
        for (; iter_vis.rem; --screen, xcb_visualtype_next (&iter_vis))
          {
            if (scr->root_visual == iter_vis.data->visual_id)
              return iter_vis.data;
          }
     }

   return NULL;
}

static xcb_colormap_t
_best_colormap_get(xcb_connection_t *conn, int screen)
{
   xcb_screen_iterator_t iter;
   xcb_colormap_t        c = { 0 };

   if (!conn) return c;

   iter = xcb_setup_roots_iterator (xcb_get_setup (conn));
   for (; iter.rem; --screen, xcb_screen_next (&iter))
      if (screen == 0)
         return iter.data->default_colormap;

   return c;
}

static int
_best_depth_get(xcb_connection_t *conn, int screen)
{
   xcb_screen_iterator_t iter;

   if (!conn) return 0;

   iter = xcb_setup_roots_iterator (xcb_get_setup (conn));
   for (; iter.rem; --screen, xcb_screen_next (&iter))
      if (screen == 0)
         return iter.data->root_depth;

   return 0;
}

static Evas_Performance *
_output_perf_new(Evas *e, xcb_connection_t *conn, xcb_visualtype_t *vis, xcb_colormap_t cmap, xcb_drawable_t draw, int depth)
{
   return evas_software_xcb_outbuf_perf_x(conn, draw, vis, cmap, depth);
   e = NULL;
}

static Evas_Performance *
_output_perf_test(Evas *e, xcb_connection_t *conn, xcb_visualtype_t *vis, xcb_colormap_t cmap, xcb_drawable_t draw, int depth)
{
   return evas_software_xcb_outbuf_perf_x(conn, draw, vis, cmap, depth);
   e = NULL;
}

static char *
_output_perf_data(Evas_Performance *perf)
{
   return evas_software_xcb_outbuf_perf_serialize_x(perf);
}

static char *
_output_perf_key(Evas_Performance *perf)
{
   return evas_software_xcb_outbuf_perf_serialize_info_x(perf);
}

static void
_output_perf_free(Evas_Performance *perf)
{
   evas_software_xcb_outbuf_perf_free(perf);
}

static void
_output_perf_build(Evas_Performance *perf, const char *data)
{
   evas_software_xcb_outbuf_perf_deserialize_x(perf, data);
}

static void
_output_perf_device_store(Evas_Performance *perf)
{
   evas_software_xcb_outbuf_perf_store_x(perf);
}

/* engine api this module provides */
static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Software_Xcb *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Software_Xcb));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->info.debug = 0;
   info->info.alloc_grayscale = 0;
   info->info.alloc_colors_max = 216;
   info->func.best_visual_get = _best_visual_get;
   info->func.best_colormap_get = _best_colormap_get;
   info->func.best_depth_get = _best_depth_get;
   info->func.performance_test = _output_perf_test;
   info->func.performance_free = _output_perf_free;
   info->func.performance_data_get = _output_perf_data;
   info->func.performance_key_get = _output_perf_key;
   info->func.performance_new = _output_perf_new;
   info->func.performance_build = _output_perf_build;
   info->func.performance_device_store = _output_perf_device_store;
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Software_Xcb *in;

   in = (Evas_Engine_Info_Software_Xcb *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Software_Xcb *info;

   info = (Evas_Engine_Info_Software_Xcb *)in;
   if (!e->engine.data.output)
     e->engine.data.output =
     _output_setup(e->output.w,
		   e->output.h,
		   info->info.rotation,
		   info->info.conn,
		   info->info.drawable,
		   info->info.visual,
		   info->info.colormap,
		   info->info.depth,
		   info->info.debug,
		   info->info.alloc_grayscale,
		   info->info.alloc_colors_max,
		   info->info.mask,
		   info->info.shape_dither,
                   info->info.destination_alpha);
   else
     {
       re = e->engine.data.output;
       evas_software_xcb_outbuf_free(re->ob);
       re->ob = evas_software_xcb_outbuf_setup_x(e->output.w,
                                                 e->output.h,
                                                 info->info.rotation,
                                                 OUTBUF_DEPTH_INHERIT,
                                                 info->info.conn,
                                                 info->info.drawable,
                                                 info->info.visual,
                                                 info->info.colormap,
                                                 info->info.depth,
                                                 evas_software_xcb_outbuf_perf_restore_x(info->info.conn, info->info.drawable, info->info.visual, info->info.colormap, info->info.depth),
                                                 info->info.alloc_grayscale,
                                                 info->info.alloc_colors_max,
                                                 info->info.mask,
                                                 info->info.shape_dither,
                                                 info->info.destination_alpha);
       evas_software_xcb_outbuf_debug_set(re->ob, info->info.debug);
     }

   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);

   re = e->engine.data.output;
   evas_software_xcb_outbuf_drawable_set(re->ob, info->info.drawable);
   evas_software_xcb_outbuf_mask_set(re->ob, info->info.mask);
   evas_software_xcb_outbuf_rotation_set(re->ob, info->info.rotation);
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_xcb_outbuf_free(re->ob);
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
   evas_software_xcb_outbuf_reconfigure(re->ob, w, h,
					evas_software_xcb_outbuf_get_rot(re->ob),
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

   surface = evas_software_xcb_outbuf_new_region_for_update(re->ob,
							    ux, uy, uw, uh,
							    cx, cy, cw, ch);
   *x = ux; *y = uy; *w = uw; *h = uh;
   return surface;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_pipe_begin(surface);
   evas_common_pipe_flush(surface);
   evas_software_xcb_outbuf_push_updated_region(re->ob, surface, x, y, w, h);
   evas_software_xcb_outbuf_free_region_for_update(re->ob, surface);
   evas_common_cpu_end_opt();
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_xcb_outbuf_flush(re->ob);
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
     "software_xcb",
     "none"
};
