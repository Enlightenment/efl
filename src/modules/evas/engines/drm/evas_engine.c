#include "evas_engine.h"

typedef struct _Render_Engine
{
   Render_Engine_Software_Generic generic;
} Render_Engine;

static Evas_Func func, pfunc;

int _evas_engine_drm_log_dom;

static Render_Engine *
_render_engine_setup(Evas_Engine_Info_Drm *info, int w, int h)
{
   Render_Engine *re;
   Outbuf *ob;

   re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;

   ob = _outbuf_setup(info, w, h);
   if (!ob) goto err;

   if (!evas_render_engine_software_generic_init(&re->generic, ob,
                                                 _outbuf_state_get,
                                                 _outbuf_rotation_get,
                                                 _outbuf_reconfigure,
                                                 NULL,
                                                 _outbuf_update_region_new,
                                                 _outbuf_update_region_push,
                                                 _outbuf_update_region_free,
                                                 NULL,
                                                 _outbuf_flush,
                                                 _outbuf_free,
                                                 ob->w, ob->h))
     goto init_err;

   return re;

init_err:
   evas_render_engine_software_generic_clean(&re->generic);
err:
   free(re);
   return NULL;
}

static void *
eng_info(Evas *evas EINA_UNUSED)
{
   Evas_Engine_Info_Drm *info;

   /* try to allocate space for our engine info structure */
   info = calloc(1, sizeof(Evas_Engine_Info_Drm));
   if (!info) return NULL;

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
   info = (Evas_Engine_Info_Drm *)einfo;
   free(info);
}

static int
eng_setup(Evas *evas, void *einfo)
{
   Render_Engine *re;
   Evas_Public_Data *epd;
   Evas_Engine_Info_Drm *info;

   info = (Evas_Engine_Info_Drm *)einfo;
   if (!info) return 0;

   epd = eo_data_scope_get(evas, EVAS_CANVAS_CLASS);
   if (!epd) return 0;

   re = epd->engine.data.output;
   if (!re)
     {
        evas_common_init();

        re = _render_engine_setup(info, epd->output.w, epd->output.h);
        if (!re) return 0;
     }
   else
     {
        Outbuf *ob;

        ob = _outbuf_setup(info, epd->output.w, epd->output.h);
        if (!ob) return 0;

        evas_render_engine_software_generic_update(&re->generic, ob,
                                                   ob->w, ob->h);
     }

   epd->engine.data.output = re;
   if (!epd->engine.data.output) return 0;

   if (!epd->engine.data.context)
     {
        epd->engine.data.context =
          epd->engine.func->context_new(epd->engine.data.output);
     }

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = data;
   if (re)
     {
        evas_render_engine_software_generic_clean(&re->generic);
        free(re);
     }

   evas_common_shutdown();
}

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

   ecore_init();

   /* store parent functions */
   func = pfunc;

   /* override the methods we provide */
   EVAS_API_OVERRIDE(info, &func, eng_);
   EVAS_API_OVERRIDE(info_free, &func, eng_);
   EVAS_API_OVERRIDE(setup, &func, eng_);
   EVAS_API_OVERRIDE(output_free, &func, eng_);

   /* advertise our engine functions */
   em->functions = (void *)(&func);

   return 1;
}

static void 
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister the eina log domain for this engine */
   eina_log_domain_unregister(_evas_engine_drm_log_dom);

   ecore_shutdown();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "drm", "none", { module_open, module_close }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, drm);

#ifndef EVAS_STATIC_BUILD_DRM
EVAS_EINA_MODULE_DEFINE(engine, drm);
#endif
