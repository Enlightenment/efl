#include "evas_common.h"
#include "evas_private.h"
#include "Evas_Engine_Drm.h"
#include "evas_engine.h"

/* local structures */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Evas_Engine_Info_Drm *info;

   Tilebuf *tb;
   Tilebuf_Rect *rects;
   Tilebuf_Rect *prev_rects[3];

   Outbuf *ob;

   Eina_Inlist *cur_rect;

   Eina_Bool end : 1;
};

/* local function prototypes */
static void *_output_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool alpha, int swap);

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* external variables */
int _evas_engine_drm_log_dom;

/* local functions */
static void *
_output_setup(int w, int h, unsigned int rotation, unsigned int depth, Eina_Bool alpha, int swap)
{
   Render_Engine *re;

   /* try to allocate space for our render engine structure */
   if (!(re = calloc(1, sizeof(Render_Engine))))
     return NULL;

   /* try to create a new tilebuffer */
   if (!(re->tb = evas_common_tilebuf_new(w, h)))
     {
        free(re);
        return NULL;
     }

   /* set tilesize */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   if (swap)
     {
        /* free any existing outbuf */
        if (re->ob) evas_outbuf_free(re->ob);

        /* try to create new outbuf */
        if (!(re->ob = evas_outbuf_setup(w, h, rotation, depth, alpha)))
          {
             if (re->tb) evas_common_tilebuf_free(re->tb);
             free(re);
             return NULL;
          }
     }

   /* return the allocated render_engine structure */
   return re;
}

/* engine api functions */
static void *
eng_info(Evas *evas EINA_UNUSED)
{
   Evas_Engine_Info_Drm *info;

   /* try to allocate space for our engine info structure */
   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Drm))))
     return NULL;

   /* set some engine default properties */
   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void 
eng_info_free(Evas *evas EINA_UNUSED, void *einfo)
{
   Evas_Engine_Info_Drm *info;

   /* free the engine info */
   if ((info = (Evas_Engine_Info_Drm *)einfo))
     free(info);
}

static int 
eng_setup(Evas *evas, void *einfo)
{
   Evas_Engine_Info_Drm *info;
   Evas_Public_Data *epd;
   Render_Engine *re;

   /* try to cast to our engine info structure */
   if (!(info = (Evas_Engine_Info_Drm *)einfo)) return 0;

   /* try to get the evas public data */
   if (!(epd = eo_data_get(evas, EVAS_CLASS))) return 0;

   /* check for valid engine output */
   if (!(re = epd->engine.data.output))
     {
        static int swap = -1;

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

        /* check if swapping is disabled */
        if (swap == -1)
          {
             if (getenv("EVAS_DRM_NO_SWAP")) swap = 0;
             else swap = 1;
          }

        /* try to create a new render_engine */
        if (!(re = _output_setup(epd->output.w, epd->output.h, 
                                 info->info.rotation, info->info.depth, 
                                 info->info.destination_alpha, swap)))
          return 0;
     }
   else
     {
        /* if we have an existing outbuf, free it */
        if (re->ob) evas_outbuf_free(re->ob);

        /* try to create a new outbuf */
        if (!(re->ob = 
              evas_outbuf_setup(epd->output.w, epd->output.h, 
                                info->info.rotation, info->info.depth, 
                                info->info.destination_alpha)))
          return 0;
     }

   /* update the info structure pointer */
   re->info = info;

   /* reassign engine output */
   epd->engine.data.output = re;
   if (!epd->engine.data.output) return 0;

   /* check for valid engine context */
   if (!epd->engine.data.context)
     {
        /* create a context if needed */
        epd->engine.data.context = 
          epd->engine.func->context_new(epd->engine.data.output);
     }

   return 1;
}

/* module api functions */
static int
module_open(Evas_Module *em)
{
   /* check for valid evas module */
   if (!em) return 0;

   /* try to inherit functions from software_generic engine */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;

   /* try to create eina logging domain */
   _evas_engine_drm_log_dom = 
     eina_log_domain_register("evas-drm", EVAS_DEFAULT_LOG_COLOR);

   /* if we could not create a logging domain, error out */
   if (_evas_engine_drm_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store parent functions */
   func = pfunc;

   /* override the methods we provide */
   EVAS_API_OVERRIDE(info, &func, eng_);
   EVAS_API_OVERRIDE(info_free, &func, eng_);
   EVAS_API_OVERRIDE(setup, &func, eng_);

   /* advertise our engine functions */
   em->functions = (void *)(&func);

   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister the eina log domain for this engine */
   eina_log_domain_unregister(_evas_engine_drm_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "drm", "none", { module_open, module_close }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, drm);

#ifndef EVAS_STATIC_BUILD_DRM
EVAS_EINA_MODULE_DEFINE(engine, drm);
#endif
