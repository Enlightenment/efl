#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_DDraw.h"

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Tilebuf          *tb;
   Outbuf           *ob;
   Tilebuf_Rect     *rects;
   Eina_Inlist      *cur_rect;
   int               end : 1;
};


static void *
_output_setup(int  width,
              int  height,
              int  rot,
              HWND window,
              int  depth,
              int  fullscreen)
{
   Render_Engine *re;

   re = calloc(1, sizeof(Render_Engine));
   if (!re)
     return NULL;

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

   evas_software_ddraw_outbuf_init();

   re->ob = evas_software_ddraw_outbuf_setup(width, height, rot,
                                             OUTBUF_DEPTH_INHERIT,
                                             window, depth, fullscreen);
   if (!re->ob)
     {
	free(re);
	return NULL;
     }

   /* for updates return 1 big buffer, but only use portions of it, also cache
    it and keep it around until an idle_flush */
   /* disable for now - i am hunting down why some expedite tests are slower,
    * as well as shaped stuff is broken and probable non-32bpp is broken as
    * convert funcs dont do the right thing
    *
   re->ob->onebuf = 1;
    */

   re->tb = evas_common_tilebuf_new(width, height);
   if (!re->tb)
     {
	evas_software_ddraw_outbuf_free(re->ob);
	free(re);
	return NULL;
     }
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   return re;
}


/* engine api this module provides */

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_Software_DDraw *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Software_DDraw));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_Software_DDraw *in;

   in = (Evas_Engine_Info_Software_DDraw *)info;
   free(in);
}

static void
eng_setup(Evas *e, void *in)
{
   Render_Engine                   *re;
   Evas_Engine_Info_Software_DDraw *info;

   info = (Evas_Engine_Info_Software_DDraw *)in;
   if (!e->engine.data.output)
     e->engine.data.output = _output_setup(e->output.w,
                                           e->output.h,
                                           info->info.rotation,
                                           info->info.window,
                                           info->info.depth,
                                           info->info.fullscreen);
   else
     {
	int ponebuf = 0;

	re = e->engine.data.output;
	ponebuf = re->ob->onebuf;
	evas_software_ddraw_outbuf_free(re->ob);
	re->ob = evas_software_ddraw_outbuf_setup(e->output.w,
                                                  e->output.h,
                                                  info->info.rotation,
                                                  OUTBUF_DEPTH_INHERIT,
                                                  info->info.window,
                                                  info->info.depth,
                                                  info->info.fullscreen);
	re->ob->onebuf = ponebuf;
     }
   if (!e->engine.data.output) return;
   if (!e->engine.data.context)
     e->engine.data.context = e->engine.func->context_new(e->engine.data.output);

   re = e->engine.data.output;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   if (!data) return;

   re = (Render_Engine *)data;
   evas_software_ddraw_outbuf_free(re->ob);
   evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int width, int height)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_ddraw_outbuf_reconfigure(re->ob,
					  width,
					  height,
                                          evas_software_ddraw_outbuf_rot_get(re->ob),
                                          OUTBUF_DEPTH_INHERIT);
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(width, height);
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
eng_output_redraws_next_update_get(void *data,
                                   int  *x,
                                   int  *y,
                                   int  *w,
                                   int  *h,
                                   int  *cx,
                                   int  *cy,
                                   int  *cw,
                                   int  *ch)
{
   Render_Engine *re;
   RGBA_Image    *surface;
   Tilebuf_Rect  *rect;
   int            ux;
   int            uy;
   int            uw;
   int            uh;

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
   ux = rect->x;
   uy = rect->y;
   uw = rect->w;
   uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }

   surface = evas_software_ddraw_outbuf_new_region_for_update(re->ob,
                                                              ux,
                                                              uy,
                                                              uw,
                                                              uh,
                                                              cx,
                                                              cy,
                                                              cw,
                                                              ch);

   *x = ux;
   *y = uy;
   *w = uw;
   *h = uh;

   return surface;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_pipe_begin(surface);
   evas_common_pipe_flush(surface);
   evas_software_ddraw_outbuf_push_updated_region(re->ob, surface, x, y, w, h);
   evas_software_ddraw_outbuf_free_region_for_update(re->ob, surface);
   evas_common_cpu_end_opt();
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_ddraw_outbuf_flush(re->ob);
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_ddraw_outbuf_idle_flush(re->ob);
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
   "software_ddraw",
   "none"
};
