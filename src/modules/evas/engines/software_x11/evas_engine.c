#include "evas_common_private.h"
#include "evas_private.h"

#include "Evas_Engine_Software_X11.h"
#include "evas_engine.h"

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
# include "evas_xlib_outbuf.h"
# include "evas_xlib_swapbuf.h"
# include "evas_xlib_color.h"
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
# include "evas_xcb_outbuf.h"
# include "evas_xcb_color.h"
# include "evas_xcb_xdefaults.h"
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
#include "evas_x_egl.h"
#endif

int _evas_engine_soft_x11_log_dom = -1;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Render_Engine_Software_Generic generic;
   Eina_Bool (*outbuf_alpha_get)(Outbuf *ob);

   struct {
      void *disp;
      void *config;
      void *surface;
   } egl;
};

/* prototypes we will use here */
static void *_best_visual_get(int backend, void *connection, int screen);
static unsigned int _best_colormap_get(int backend, void *connection, int screen);
static int _best_depth_get(int backend, void *connection, int screen);

static void *eng_info(Evas *eo_e);
static void eng_info_free(Evas *eo_e, void *info);
static int eng_setup(Evas *eo_e, void *info);
static void eng_output_free(void *data);

/* internal engine routines */

#ifdef BUILD_ENGINE_SOFTWARE_XLIB

/*
static void *
_output_egl_setup(int w, int h, int rot, Display *disp, Drawable draw,
                  Visual *vis, Colormap cmap, int depth, int debug,
                  int grayscale, int max_colors, Pixmap mask,
                  int shape_dither, int destination_alpha)
{
   Render_Engine *re;
   void *ptr;
   int stride = 0;
   
   if (depth != 32) return NULL;
   if (mask) return NULL;
   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;
   re->egl.disp = _egl_x_disp_get(disp);
   if (!re->egl.disp)
     {
        free(re);
        return NULL;
     }
   re->egl.config = _egl_x_disp_choose_config(re->egl.disp);
   if (!re->egl.config)
     {
        _egl_x_disp_terminate(re->egl.disp);
        free(re);
        return NULL;
     }
   re->egl.surface = _egl_x_win_surf_new(re->egl.disp, draw, re->egl.config);
   if (!re->egl.surface)
     {
        _egl_x_disp_terminate(re->egl.disp);
        free(re);
        return NULL;
     }
   ptr = _egl_x_surf_map(re->egl.disp, re->egl.surface, &stride);
   if (!ptr)
     {
        _egl_x_win_surf_free(re->egl.disp, re->egl.surface);
        _egl_x_disp_terminate(re->egl.disp);
        free(re);
        return NULL;
     }
   _egl_x_surf_unmap(re->egl.disp, re->egl.surface);
   
   re->ob = 
     evas_software_egl_outbuf_setup_x(w, h, rot, OUTBUF_DEPTH_INHERIT, disp, 
                                       draw, vis, cmap, depth, grayscale,
                                       max_colors, mask, shape_dither,
                                       destination_alpha);
   
   re->tb = evas_common_tilebuf_new(w, h);
   if (!re->tb)
     {
	evas_software_xlib_outbuf_free(re->ob);
	free(re);
	return NULL;
     }

   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   
   return re;
   debug = 0;
}
*/

static void
_output_egl_shutdown(Render_Engine *re)
{
   if (!re->egl.disp) return;
   _egl_x_win_surf_free(re->egl.disp, re->egl.surface);
   _egl_x_disp_terminate(re->egl.disp);
}

static void *
_output_xlib_setup(int w, int h, int rot, Display *disp, Drawable draw,
                   Visual *vis, Colormap cmap, int depth, int debug,
                   int grayscale, int max_colors, Pixmap mask,
                   int shape_dither, int destination_alpha)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   evas_software_xlib_x_init();
   evas_software_xlib_x_color_init();
   evas_software_xlib_outbuf_init();

   ob =
     evas_software_xlib_outbuf_setup_x(w, h, rot, OUTBUF_DEPTH_INHERIT, disp,
                                       draw, vis, cmap, depth, grayscale,
                                       max_colors, mask, shape_dither,
                                       destination_alpha);
   if (!ob) goto on_error;

   /* for updates return 1 big buffer, but only use portions of it, also cache
    * it and keepit around until an idle_flush */

   /* disable for now - i am hunting down why some expedite tests are slower,
    * as well as shaped stuff is broken and probable non-32bpp is broken as
    * convert funcs dont do the right thing
    *
    */
//   re->ob->onebuf = 1;

   evas_software_xlib_outbuf_debug_set(ob, debug);
   if (!evas_render_engine_software_generic_init(&re->generic, ob, NULL,
                                                 evas_software_xlib_outbuf_get_rot,
                                                 evas_software_xlib_outbuf_reconfigure,
                                                 evas_software_xlib_outbuf_new_region_for_update,
                                                 evas_software_xlib_outbuf_push_updated_region,
                                                 evas_software_xlib_outbuf_free_region_for_update,
                                                 evas_software_xlib_outbuf_idle_flush,
                                                 evas_software_xlib_outbuf_flush,
                                                 evas_software_xlib_outbuf_free,
                                                 w, h))
     goto on_error;

   return re;

 on_error:
   if (ob) evas_software_xlib_outbuf_free(ob);
   free(re);
   return NULL;
}

static void *
_output_swapbuf_setup(int w, int h, int rot, Display *disp, Drawable draw,
                      Visual *vis, Colormap cmap, int depth,
                      int debug EINA_UNUSED,
                      int grayscale, int max_colors, Pixmap mask,
                      int shape_dither, int destination_alpha)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   evas_software_xlib_x_init();
   evas_software_xlib_x_color_init();
   evas_software_xlib_swapbuf_init();

   ob =
     evas_software_xlib_swapbuf_setup_x(w, h, rot, OUTBUF_DEPTH_INHERIT, disp,
                                        draw, vis, cmap, depth, grayscale,
                                        max_colors, mask, shape_dither,
                                        destination_alpha);
   if (!ob) goto on_error;

   if (!evas_render_engine_software_generic_init(&re->generic, ob,
                                                 evas_software_xlib_swapbuf_buffer_state_get,
                                                 evas_software_xlib_swapbuf_get_rot,
                                                 evas_software_xlib_swapbuf_reconfigure,
                                                 evas_software_xlib_swapbuf_new_region_for_update,
                                                 evas_software_xlib_swapbuf_push_updated_region,
                                                 evas_software_xlib_swapbuf_free_region_for_update,
                                                 evas_software_xlib_swapbuf_idle_flush,
                                                 evas_software_xlib_swapbuf_flush,
                                                 evas_software_xlib_swapbuf_free,
                                                 w, h))
     goto on_error;
   return re;

 on_error:
   if (ob) evas_software_xlib_swapbuf_free(ob);
   free(re);
   return NULL;
}
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
static void *
_output_xcb_setup(int w, int h, int rot, xcb_connection_t *conn, 
                  xcb_screen_t *screen, xcb_drawable_t draw, 
                  xcb_visualtype_t *vis, xcb_colormap_t cmap, int depth,
                  int debug, int grayscale, int max_colors, xcb_drawable_t mask,
                  int shape_dither, int destination_alpha)
{
   Render_Engine *re;
   Outbuf *ob;

   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   evas_software_xcb_init();
   evas_software_xcb_color_init();
   evas_software_xcb_outbuf_init();

   ob = evas_software_xcb_outbuf_setup(w, h, rot, OUTBUF_DEPTH_INHERIT, conn
                                    screen, draw, vis, cmap, depth,
                                    grayscale, max_colors, mask,
                                    shape_dither, destination_alpha);
   if (!ob) goto on_error;

   /* for updates return 1 big buffer, but only use portions of it, also cache
    * it and keepit around until an idle_flush */

   /* disable for now - i am hunting down why some expedite tests are slower,
    * as well as shaped stuff is broken and probable non-32bpp is broken as
    * convert funcs dont do the right thing
    *
    */
//   re->ob->onebuf = 1;

   evas_software_xcb_outbuf_debug_set(ob, debug);

   if (!evas_render_engine_software_generic_init(re, ob, NULL,
                                                 evas_software_xcb_outbuf_rotation_get,
                                                 evas_software_xcb_outbuf_reconfigure,
                                                 evas_software_xcb_outbuf_new_region_for_update,
                                                 evas_software_xcb_outbuf_push_updated_region,
                                                 evas_software_xcb_outbuf_free_region_for_update,
                                                 evas_software_xcb_outbuf_idle_flush,
                                                 evas_software_xcb_outbuf_flush,
                                                 evas_software_xcb_outbuf_free,
                                                 w, h))
     goto on_error;
   return re;

 on_error:
   if (ob) evas_software_xcb_outbuf_free(ob);
   free(re);
   return NULL;
}
#endif

static void *
_best_visual_get(int backend, void *connection, int screen)
{
   if (!connection) return NULL;

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB)
     return DefaultVisual((Display *)connection, screen);
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
   if (backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB)
     {
        xcb_screen_iterator_t iter_screen;
        xcb_depth_iterator_t iter_depth;
        xcb_screen_t *s = NULL;

        iter_screen = 
          xcb_setup_roots_iterator(xcb_get_setup((xcb_connection_t *)connection));
        for (; iter_screen.rem; --screen, xcb_screen_next(&iter_screen))
          if (screen == 0)
            {
               s = iter_screen.data;
               break;
            }

        iter_depth = xcb_screen_allowed_depths_iterator(s);
        for (; iter_depth.rem; xcb_depth_next(&iter_depth))
          {
             xcb_visualtype_iterator_t iter_vis;

             iter_vis = xcb_depth_visuals_iterator(iter_depth.data);
             for (; iter_vis.rem; xcb_visualtype_next(&iter_vis))
               {
                  if (s->root_visual == iter_vis.data->visual_id)
                    return iter_vis.data;
               }
          }
     }
#endif

   return NULL;
}

static unsigned int
_best_colormap_get(int backend, void *connection, int screen)
{
   if (!connection) return 0;

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB)
     return DefaultColormap((Display *)connection, screen);
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
   if (backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB)
     {
        xcb_screen_iterator_t iter_screen;
        xcb_screen_t *s = NULL;

        iter_screen = 
          xcb_setup_roots_iterator(xcb_get_setup((xcb_connection_t *)connection));
        for (; iter_screen.rem; --screen, xcb_screen_next(&iter_screen))
          if (screen == 0)
            {
               s = iter_screen.data;
               break;
            }

        return s->default_colormap;
     }
#endif

   return 0;
}

static int
_best_depth_get(int backend, void *connection, int screen)
{
   if (!connection) return 0;

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB)
     return DefaultDepth((Display *)connection, screen);
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
   if (backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB)
     {
        xcb_screen_iterator_t iter_screen;
        xcb_screen_t *s = NULL;

        iter_screen = 
          xcb_setup_roots_iterator(xcb_get_setup((xcb_connection_t *)connection));
        for (; iter_screen.rem; --screen, xcb_screen_next(&iter_screen))
          if (screen == 0)
            {
               s = iter_screen.data;
               break;
            }

        return s->root_depth;
     }
#endif

   return 0;
}

/* engine api this module provides */
static void *
eng_info(Evas *eo_e EINA_UNUSED)
{
   Evas_Engine_Info_Software_X11 *info;

   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Software_X11))))
     return NULL;

   info->magic.magic = rand();
   info->info.debug = 0;
   info->info.alloc_grayscale = 0;
   info->info.alloc_colors_max = 216;
   info->func.best_visual_get = _best_visual_get;
   info->func.best_colormap_get = _best_colormap_get;
   info->func.best_depth_get = _best_depth_get;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;
   return info;
}

static void
eng_info_free(Evas *eo_e EINA_UNUSED, void *info)
{
   Evas_Engine_Info_Software_X11 *in;

   in = (Evas_Engine_Info_Software_X11 *)info;
   free(in);
}

static int
eng_setup(Evas *eo_e, void *in)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Engine_Info_Software_X11 *info;
   Render_Engine *re = NULL;

   info = (Evas_Engine_Info_Software_X11 *)in;
   if (!e->engine.data.output)
     {
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

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
        if (info->info.backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB)
          {
             static int try_swapbuf = -1;

             if (try_swapbuf == -1)
               {
                  if (getenv("EVAS_NO_DRI_SWAPBUF")) try_swapbuf = 0;
                  else try_swapbuf = 1;
               }
             if (try_swapbuf)
               re = _output_swapbuf_setup(e->output.w, e->output.h,
                                          info->info.rotation, info->info.connection,
                                          info->info.drawable, info->info.visual,
                                          info->info.colormap,
                                          info->info.depth, info->info.debug,
                                          info->info.alloc_grayscale,
                                          info->info.alloc_colors_max,
                                          info->info.mask, info->info.shape_dither,
                                          info->info.destination_alpha);
             if (re) re->outbuf_alpha_get = evas_software_xlib_swapbuf_alpha_get;
             else if (!re)
               {
                  re = _output_xlib_setup(e->output.w, e->output.h,
                                          info->info.rotation, info->info.connection,
                                          info->info.drawable, info->info.visual,
                                          info->info.colormap,
                                          info->info.depth, info->info.debug,
                                          info->info.alloc_grayscale,
                                          info->info.alloc_colors_max,
                                          info->info.mask, info->info.shape_dither,
                                          info->info.destination_alpha);
                  re->outbuf_alpha_get = evas_software_xlib_outbuf_alpha_get;
               }
          }
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
        if (info->info.backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB)
          {
             re = _output_xcb_setup(e->output.w, e->output.h,
                                    info->info.rotation, info->info.connection,
                                    info->info.screen, info->info.drawable,
                                    info->info.visual, info->info.colormap,
                                    info->info.depth, info->info.debug,
                                    info->info.alloc_grayscale,
                                    info->info.alloc_colors_max,
                                    info->info.mask, info->info.shape_dither,
                                    info->info.destination_alpha);
             re->outbuf_alpha_get = evas_software_xcb_outbuf_alpha_get;
          }
#endif

        e->engine.data.output = re;
     }
   else
     {
        Outbuf *ob = NULL;
        /* int ponebuf = 0; */

        re = e->engine.data.output;
        /* if ((re) && (re->ob)) ponebuf = re->ob->onebuf; */

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
        if (info->info.backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XLIB)
          {
             if (re->generic.outbuf_free == evas_software_xlib_swapbuf_free)
               {
                  ob =
                    evas_software_xlib_swapbuf_setup_x(e->output.w, e->output.h,
                                                       info->info.rotation,
                                                       OUTBUF_DEPTH_INHERIT,
                                                       info->info.connection,
                                                       info->info.drawable,
                                                       info->info.visual,
                                                       info->info.colormap,
                                                       info->info.depth,
                                                       info->info.alloc_grayscale,
                                                       info->info.alloc_colors_max,
                                                       info->info.mask,
                                                       info->info.shape_dither,
                                                       info->info.destination_alpha);
               }
             else
               {
                  ob =
                    evas_software_xlib_outbuf_setup_x(e->output.w, e->output.h,
                                                      info->info.rotation,
                                                      OUTBUF_DEPTH_INHERIT,
                                                      info->info.connection,
                                                      info->info.drawable,
                                                      info->info.visual,
                                                      info->info.colormap,
                                                      info->info.depth,
                                                      info->info.alloc_grayscale,
                                                      info->info.alloc_colors_max,
                                                      info->info.mask,
                                                      info->info.shape_dither,
                                                      info->info.destination_alpha);
                  evas_software_xlib_outbuf_debug_set(ob, info->info.debug);
               }
          }
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XCB
        if (info->info.backend == EVAS_ENGINE_INFO_SOFTWARE_X11_BACKEND_XCB)
          {
             evas_software_xcb_outbuf_free(re->ob);
             ob =
               evas_software_xcb_outbuf_setup(e->output.w, e->output.h,
                                              info->info.rotation,
                                              OUTBUF_DEPTH_INHERIT,
                                              info->info.connection,
                                              info->info.screen,
                                              info->info.drawable,
                                              info->info.visual,
                                              info->info.colormap,
                                              info->info.depth,
                                              info->info.alloc_grayscale,
                                              info->info.alloc_colors_max,
                                              info->info.mask,
                                              info->info.shape_dither,
                                              info->info.destination_alpha);
             evas_software_xcb_outbuf_debug_set(re->ob, info->info.debug);
          }
#endif

        if (ob)
          {
             evas_render_engine_software_generic_update(&re->generic, ob);
          }

        /* if ((re) && (re->ob)) re->ob->onebuf = ponebuf; */
     }
   if (!e->engine.data.output) return 0;
   if (!e->engine.data.context)
     {
        e->engine.data.context =
          e->engine.func->context_new(e->engine.data.output);
     }

   re = e->engine.data.output;

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     {
        evas_render_engine_software_generic_clean(&re->generic);
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
        _output_egl_shutdown(re);
#endif
        free(re);
     }

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static Eina_Bool
eng_canvas_alpha_get(void *data, void *context EINA_UNUSED)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return (re->generic.ob->priv.destination_alpha) ||
     (re->outbuf_alpha_get(re->generic.ob));
}


/* module advertising code */
static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;

   _evas_engine_soft_x11_log_dom = 
     eina_log_domain_register("evas-software_x11", EVAS_DEFAULT_LOG_COLOR);

   if (_evas_engine_soft_x11_log_dom < 0)
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

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
  eina_log_domain_unregister(_evas_engine_soft_x11_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "software_x11", "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, software_x11);

#ifndef EVAS_STATIC_BUILD_SOFTWARE_X11
EVAS_EINA_MODULE_DEFINE(engine, software_x11);
#endif
