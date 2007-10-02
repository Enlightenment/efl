#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_X11.h"

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
static void *_output_setup(int w, int h, int rot, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth, int debug, int grayscale, int max_colors, Pixmap mask, int shape_dither, int destination_alpha);
static Visual *_best_visual_get(Display *disp, int screen);
static Colormap _best_colormap_get(Display *disp, int screen);
static int _best_depth_get(Display *disp, int screen);

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

/* internal engine routines */
static void *
_output_setup(int w, int h, int rot, Display *disp, Drawable draw, Visual *vis, Colormap cmap, int depth, int debug, int grayscale, int max_colors, Pixmap mask, int shape_dither, int destination_alpha)
{
   Render_Engine *re;

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

   re->ob = evas_software_x11_outbuf_setup_x(w, h, rot, OUTBUF_DEPTH_INHERIT,
					     disp, draw, vis, cmap, depth,
					     grayscale, max_colors, mask,
					     shape_dither, destination_alpha);
   if (!re->ob)
     {
	free(re);
	return NULL;
     }
   
   /* for updates return 1 big buffer, but only use portions of it, also cache
    it and keepit around until an idle_flush */
   /* disable for now - i am hunting down why some expedite tests are slower,
    * as well as shaped stuff is broken and probable non-32bpp is broken as
    * convert funcs dont do the right thing
    *
   re->ob->onebuf = 1;
    */
   
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

static Visual *
_best_visual_get(Display *disp, int screen)
{
   if (!disp) return NULL;
   return DefaultVisual(disp, screen);
}

static Colormap
_best_colormap_get(Display *disp, int screen)
{
   if (!disp) return 0;
   return DefaultColormap(disp, screen);
}

static int
_best_depth_get(Display *disp, int screen)
{
   if (!disp) return 0;
   return DefaultDepth(disp, screen);
}

/* engine api this module provides */
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
   info->func.best_visual_get = _best_visual_get;
   info->func.best_colormap_get = _best_colormap_get;
   info->func.best_depth_get = _best_depth_get;
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
     _output_setup(e->output.w,
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
		   info->info.shape_dither,
		   info->info.destination_alpha);
   else
     {
	int ponebuf = 0;
	
	re = e->engine.data.output;
	ponebuf = re->ob->onebuf;
	evas_software_x11_outbuf_free(re->ob);
	re->ob = evas_software_x11_outbuf_setup_x(e->output.w,
						  e->output.h,
						  info->info.rotation,
						  OUTBUF_DEPTH_INHERIT,
						  info->info.display,
						  info->info.drawable,
						  info->info.visual,
						  info->info.colormap,
						  info->info.depth,
						  info->info.alloc_grayscale,
						  info->info.alloc_colors_max,
						  info->info.mask,
						  info->info.shape_dither,
						  info->info.destination_alpha);
	evas_software_x11_outbuf_debug_set(re->ob, info->info.debug);
	re->ob->onebuf = ponebuf;
     }
   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);

   re = e->engine.data.output;
   evas_software_x11_outbuf_drawable_set(re->ob, info->info.drawable);
   evas_software_x11_outbuf_mask_set(re->ob, info->info.mask);
   evas_software_x11_outbuf_rotation_set(re->ob, info->info.rotation);
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   if (!data) return;

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
   evas_common_pipe_begin(surface);
   evas_common_pipe_flush(surface);
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

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_x11_outbuf_idle_flush(re->ob);
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
     "software_x11",
     "none"
};
