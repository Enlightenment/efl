#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Software_Gdi.h"

int _evas_engine_soft_gdi_log_dom = -1;
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
_output_setup(int          width,
              int          height,
              int          rot,
              HWND         window,
              int          depth,
              unsigned int borderless,
              unsigned int fullscreen,
              unsigned int region)
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
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();

   evas_software_gdi_outbuf_init();

   if (width <= 0)
     width = 1;
   if (height <= 0)
     height = 1;

   re->ob = evas_software_gdi_outbuf_setup(width, height, rot,
                                           OUTBUF_DEPTH_INHERIT,
                                           window, depth, borderless, fullscreen, region,
                                           0, 0);
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
	evas_software_gdi_outbuf_free(re->ob);
	free(re);
	return NULL;
     }
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   return re;
}


/* engine api this module provides */

static void *
eng_info(Evas *e EINA_UNUSED)
{
   Evas_Engine_Info_Software_Gdi *info;
   info = calloc(1, sizeof(Evas_Engine_Info_Software_Gdi));
   if (!info) return NULL;
   info->magic.magic = rand();
   return info;
}

static void
eng_info_free(Evas *e EINA_UNUSED, void *info)
{
   Evas_Engine_Info_Software_Gdi *in;
   in = (Evas_Engine_Info_Software_Gdi *)info;
   free(in);
}

static int
eng_setup(Evas *eo_e, void *in)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Render_Engine                 *re;
   Evas_Engine_Info_Software_Gdi *info;

   info = (Evas_Engine_Info_Software_Gdi *)in;
   if (!e->engine.data.output)
     e->engine.data.output = _output_setup(e->output.w,
                                           e->output.h,
                                           info->info.rotation,
                                           info->info.window,
                                           info->info.depth,
                                           info->info.borderless,
                                           info->info.fullscreen,
                                           info->info.region);
   else
     {
	int ponebuf = 0;

	re = e->engine.data.output;
	ponebuf = re->ob->onebuf;
	evas_software_gdi_outbuf_free(re->ob);
	re->ob = evas_software_gdi_outbuf_setup(e->output.w,
                                                e->output.h,
                                                info->info.rotation,
                                                OUTBUF_DEPTH_INHERIT,
                                                info->info.window,
                                                info->info.depth,
                                                info->info.borderless,
                                                info->info.fullscreen,
                                                info->info.region,
                                                0, 0);
	re->ob->onebuf = ponebuf;
     }
   if (!e->engine.data.output) return 0;
   if (!e->engine.data.context)
     e->engine.data.context = e->engine.func->context_new(e->engine.data.output);

   re = e->engine.data.output;

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   if (!data) return;

   re = (Render_Engine *)data;
   evas_software_gdi_outbuf_free(re->ob);
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
   evas_software_gdi_outbuf_reconfigure(re->ob,
                                        width,
                                        height,
                                        evas_software_gdi_outbuf_rot_get(re->ob),
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

   surface = evas_software_gdi_outbuf_new_region_for_update(re->ob,
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
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   re = (Render_Engine *)data;
#ifdef BUILD_PIPE_RENDER
   evas_common_pipe_map_begin(surface);
#endif
   evas_software_gdi_outbuf_push_updated_region(re->ob, surface, x, y, w, h);
   evas_software_gdi_outbuf_free_region_for_update(re->ob, surface);
   evas_common_cpu_end_opt();
}

static void
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{
   Render_Engine *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   re = (Render_Engine *)data;
   evas_software_gdi_outbuf_flush(re->ob);
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_software_gdi_outbuf_idle_flush(re->ob);
}

static Eina_Bool
eng_canvas_alpha_get(void *data EINA_UNUSED, void *context EINA_UNUSED)
{
   return EINA_FALSE;
}

/* module advertising code */
static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;

   _evas_engine_soft_gdi_log_dom = eina_log_domain_register
     ("evas-software_gdi", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_soft_gdi_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(canvas_alpha_get);
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

static void
module_close(Evas_Module *em EINA_UNUSED)
{
  eina_log_domain_unregister(_evas_engine_soft_gdi_log_dom);
  _evas_engine_soft_gdi_log_dom = -1;
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "software_gdi",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, software_gdi);

#ifndef EVAS_STATIC_BUILD_SOFTWARE_GDI
EVAS_EINA_MODULE_DEFINE(engine, software_gdi);
#endif
