#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_16_X11.h"
#include "evas_soft16.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Display          *disp;
   Drawable          draw;
   GC                gc;
   int               w, h;
   Tilebuf          *tb;
   Tilebuf_Rect     *rects;
   Tilebuf_Rect     *cur_rect;
   X_Output_Buffer  *shbuf;
   unsigned char     end : 1;
   unsigned char     shm : 1;
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

/* engine api this module provides */
static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Software_16_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Software_16_X11));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Software_16_X11 *in;

   in = (Evas_Engine_Info_Software_16_X11 *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Software_16_X11 *info;
   X_Output_Buffer *xob;
   XGCValues gcv;
   
   info = (Evas_Engine_Info_Software_16_X11 *)in;
   if (!e->engine.data.output)
     {
	/* the only check - simplistic, i know, but enough for this 
	 * "special purpose" engine. Remember it is meant to be used
	 * for limited power devices that have a 16bit display mode
	 * and no real other acceleration, and high resolution so we
	 * can pre-dither into 16bpp. */
//	if (DefaultDepth(info->info.display, 
//			 DefaultScreen(info->info.display)) != 16)
//	  return;
	/* do common routine init - we wil at least use it for core
	 * image loading and font loading/glyph rendering & placement */
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
	
	/* render engine specific data */
	re = calloc(1, sizeof(Render_Engine));
	e->engine.data.output = re;
	re->disp = info->info.display;
	re->draw = info->info.drawable;
	re->gc = XCreateGC(re->disp, re->draw, 0, &gcv);
	re->w = e->output.w;
	re->h = e->output.h;
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }
   else
     {
	/* we changed the info after first init - do a re-eval where
	 * appropriate */
//	if (DefaultDepth(info->info.display, 
//			 DefaultScreen(info->info.display)) != 16)
//	  return;
	re = e->engine.data.output;
	if (re->tb) evas_common_tilebuf_free(re->tb);
	re->disp = info->info.display;
	re->draw = info->info.drawable;
	XFreeGC(re->disp, re->gc);
	re->gc = XCreateGC(re->disp, re->draw, 0, &gcv);
	re->w = e->output.w;
	re->h = e->output.h;
	re->tb = evas_common_tilebuf_new(e->output.w, e->output.h);
	if (re->tb)
	  evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
     }
   if (!e->engine.data.output) return;
   /* add a draw context if we dont have one */
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
   /* check if the display can do shm */
   re->shm = evas_software_x11_x_can_do_shm(re->disp);
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re->gc) XFreeGC(re->disp, re->gc);
   if (re->tb) evas_common_tilebuf_free(re->tb);
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
   evas_common_tilebuf_free(re->tb);
   re->w = w;
   re->h = h;
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
   Soft16_Image *surface = NULL;
   Tilebuf_Rect *rect;
   int ux, uy, uw, uh;

   re = (Render_Engine *)data;
   if (re->end)
     {
	if (re->shbuf)
	  {
	     evas_software_x11_x_output_buffer_free(re->shbuf, 0);
	     re->shbuf = NULL;
	  }
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	int mw, mh;
	
	mw = 0;
	mh = 0;
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = re->rects;
	for (rect = re->cur_rect; rect; rect = (Tilebuf_Rect *)(rect->_list_data.next))
	  {
	     if (rect->w > mw) mw = rect->w;
	     if (rect->h > mh) mh = rect->h;
	  }
	if (re->rects)
	  re->shbuf = evas_software_x11_x_output_buffer_new
	  (re->disp, DefaultVisual(re->disp, DefaultScreen(re->disp)),
	   DefaultDepth(re->disp, DefaultScreen(re->disp)), mw, mh, 1, NULL);
     }
   if ((!re->cur_rect) || (!re->shbuf))
     {
	if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	if (re->shbuf)
	  {
	     evas_software_x11_x_output_buffer_free(re->shbuf, 0);
	     re->shbuf = NULL;
	  }
	return NULL;
     }
   rect = re->cur_rect;
   ux = rect->x; uy = rect->y; uw = rect->w; uh = rect->h;
   re->cur_rect = (Tilebuf_Rect *)(re->cur_rect->_list_data.next);
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }
   surface = re->shbuf;
   surface->w = uw;
   surface->h = uh;

   *cx = 0; *cy = 0; *cw = uw; *ch = uh;
   *x = ux; *y = uy; *w = uw; *h = uh;
   return surface;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_x11_x_output_buffer_paste(surface, re->draw, re->gc,
					   x, y, w, h, 1);
//   evas_software_x11_outbuf_push_updated_region(re->ob, surface, x, y, w, h);
//   evas_software_x11_outbuf_free_region_for_update(re->ob, surface);
//   evas_common_cpu_end_opt();
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* FIXME: later on defer the eng_output_redraws_next_update_push() until
    * flush and keep a list */
//   evas_software_x11_outbuf_flush(re->ob);
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* FIXME: clean up any resources kept around between renders in case
    * we are animating a lot and want high fps */
}


/* module advertising code */
EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_16")) return 0;
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
     "software_16_x11",
     "none"
};
