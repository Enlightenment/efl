#include "evas_common_private.h"
#include "evas_private.h"
#include "Evas_Engine_Wayland_Shm.h"
#include "evas_engine.h"
#include "evas_swapbuf.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Render_Engine_Software_Generic generic;

   void (*outbuf_reconfigure)(Outbuf *ob, int x, int y, int w, int h, int rot, Outbuf_Depth depth, Eina_Bool alpha);
};

/* engine function prototypes */
static void *eng_info(Evas *eo_evas EINA_UNUSED);
static void eng_info_free(Evas *eo_evas EINA_UNUSED, void *einfo);
static int eng_setup(Evas *eo_evas, void *einfo);
static void eng_output_free(void *data);

/* local variables */
static Evas_Func func, pfunc;

/* external variables */
int _evas_engine_way_shm_log_dom = -1;

/* local functions */
static void *
_output_engine_setup(Evas_Engine_Info_Wayland_Shm *info,
                     int w, int h,
                     unsigned int rotation, unsigned int depth,
                     Eina_Bool destination_alpha,
                     struct wl_shm *wl_shm,
                     struct wl_surface *wl_surface)
{
   Render_Engine *re = NULL;
   Outbuf *ob;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to allocate a new render engine */
   if (!(re = calloc(1, sizeof(Render_Engine))))
     return NULL;


   ob = evas_swapbuf_setup(info, w, h, rotation, depth,
                           destination_alpha, wl_shm,
                           wl_surface);
   if (!ob) goto on_error;

   if (!evas_render_engine_software_generic_init(&re->generic, ob,
                                                 evas_swapbuf_state_get,
                                                 evas_swapbuf_rotation_get,
                                                 NULL,
                                                 NULL,
                                                 evas_swapbuf_update_region_new,
                                                 evas_swapbuf_update_region_push,
                                                 evas_swapbuf_update_region_free,
                                                 evas_swapbuf_idle_flush,
                                                 evas_swapbuf_flush,
                                                 evas_swapbuf_free,
                                                 w, h))
     goto on_error;

   re->outbuf_reconfigure = evas_swapbuf_reconfigure;

   /* return allocated render engine */
   return re;

 on_error:
   if (ob) evas_swapbuf_free(ob);
   free(re);
   return NULL;
}

/* engine functions */
static void *
eng_info(Evas *eo_evas EINA_UNUSED)
{
   Evas_Engine_Info_Wayland_Shm *info;

   /* try to allocate space for engine info */
   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Wayland_Shm))))
     return NULL;

   /* fill in default engine info fields */
   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   /* return allocated engine info */
   return info;
}

static void 
eng_info_free(Evas *eo_evas EINA_UNUSED, void *einfo)
{
   Evas_Engine_Info_Wayland_Shm *info;

   /* try to free previously allocated engine info */
   if ((info = (Evas_Engine_Info_Wayland_Shm *)einfo))
     free(info);
}

static int 
eng_setup(Evas *eo_evas, void *einfo)
{
   Evas_Engine_Info_Wayland_Shm *info;
   Evas_Public_Data *epd;
   Render_Engine *re = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to cast the engine info to our engine info */
   if (!(info = (Evas_Engine_Info_Wayland_Shm *)einfo))
     return 0;

   /* try to get evas public data from the canvas */
   if (!(epd = eo_data_scope_get(eo_evas, EVAS_CANVAS_CLASS)))
     return 0;

   /* test for valid engine output */
   if (!(re = epd->engine.data.output))
     {
        static int try_swap = -1;

        /* NB: If we have no valid output then assume we have not been 
         * initialized yet and call any needed common init routines */
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

        if (try_swap == -1)
          {
             /* check for env var to see if we should try swapping */
             if (getenv("EVAS_NO_WAYLAND_SWAPBUF")) try_swap = 0;
             else try_swap = 1;
          }

        if (!(re =
              _output_engine_setup(info, epd->output.w, epd->output.h,
                                   info->info.rotation, info->info.depth,
                                   info->info.destination_alpha,
                                   info->info.wl_shm, info->info.wl_surface)))
          return 0;
      }
   else
     {
        Outbuf *ob;
        int ponebuf = 0;

        if ((re) && (re->generic.ob)) ponebuf = re->generic.ob->onebuf;

        ob = evas_swapbuf_setup(info, epd->output.w, epd->output.h,
                                info->info.rotation,
                                info->info.depth,
                                info->info.destination_alpha,
                                info->info.wl_shm,
                                info->info.wl_surface);
        if (!ob) return 0;

        evas_render_engine_software_generic_update(&re->generic, ob, epd->output.w, epd->output.h);

        if ((re) && (re->generic.ob)) re->generic.ob->onebuf = ponebuf;
     }

   /* reassign render engine to output */
   epd->engine.data.output = re;
   if (!epd->engine.data.output) return 0;

   if (!epd->engine.data.context)
     {
        epd->engine.data.context = 
          epd->engine.func->context_new(epd->engine.data.output);
     }

   /* return success */
   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re = data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   evas_render_engine_software_generic_clean(&re->generic);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;
   Evas_Engine_Info_Wayland_Shm *info;
   int dx = 0, dy = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!(re = (Render_Engine *)data)) return;

   if (!(info = re->generic.ob->info)) return;

   if (info->info.edges & 4)
     {
        if ((info->info.rotation == 90) || (info->info.rotation == 270))
          dx = re->generic.ob->h - h;
        else
          dx = re->generic.ob->w - w;
     }

   if (info->info.edges & 1)
     {
        if ((info->info.rotation == 90) || (info->info.rotation == 270))
          dy = re->generic.ob->w - w;
        else
          dy = re->generic.ob->h - h;
     }

   re->outbuf_reconfigure(re->generic.ob, dx, dy, w, h,
                          info->info.rotation, info->info.depth,
                          info->info.destination_alpha);

   evas_common_tilebuf_free(re->generic.tb);
   if ((re->generic.tb = evas_common_tilebuf_new(w, h)))
     evas_common_tilebuf_set_tile_size(re->generic.tb, TILESIZE, TILESIZE);
}

/* module functions */
static int 
module_open(Evas_Module *em)
{
   /* check for valid evas module */
   if (!em) return 0;

   /* try to create our logging domain */
   _evas_engine_way_shm_log_dom = 
     eina_log_domain_register("evas-wayland_shm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_way_shm_log_dom < 0)
     {
        /* creating the logging domain failed. notify user */
        EINA_LOG_ERR("Could not create a module log domain.");

        /* return failure */
        return 0;
     }

   /* try to inherit base functions from the software generic engine */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic"))
     return 0;

   /* copy base functions from the software_generic engine */
   func = pfunc;

   /* override any engine specific functions that we provide */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(output_free);
   ORD(output_resize);

   /* advertise out our own api */
   em->functions = (void *)(&func);

   /* return success */
   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   /* if we have the log domain, unregister it */
   if (_evas_engine_way_shm_log_dom > -1)
     eina_log_domain_unregister(_evas_engine_way_shm_log_dom);
}

static Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, "wayland_shm", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_shm);

#ifndef EVAS_STATIC_BUILD_WAYLAND_SHM
EVAS_EINA_MODULE_DEFINE(engine, wayland_shm);
#endif
