#include "evas_engine.h"
#include "../software_generic/evas_native_common.h"

typedef struct _Render_Engine
{
   Render_Output_Software_Generic generic;

   Ecore_Drm2_Device *dev;
} Render_Engine;

static Evas_Func func, pfunc;

int _evas_engine_drm_log_dom;

static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_Drm *einfo = info;

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine, void *einfo, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Drm *info = einfo;
   Render_Engine *re;
   Outbuf *ob;

   re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;

   ob = _outbuf_setup(info, w, h);
   if (!ob) goto err;

   re->dev = info->dev;

   if (!evas_render_engine_software_generic_init(engine, &re->generic, ob,
                                                 _outbuf_swap_mode_get,
                                                 _outbuf_rotation_get,
                                                 _outbuf_reconfigure,
                                                 NULL, // region first rect get
                                                 _outbuf_damage_region_set,
                                                 _outbuf_update_region_new,
                                                 _outbuf_update_region_push,
                                                 NULL, // free_region_for_update
                                                 _outbuf_idle_flush,
                                                 _outbuf_flush,
                                                 NULL, // redraws_clear
                                                 _outbuf_free,
                                                 ob->w, ob->h))
     goto init_err;

   evas_render_engine_software_generic_merge_mode_set(&re->generic);

   re->generic.ob->info = einfo;

   return re;

init_err:
   evas_render_engine_software_generic_clean(engine, &re->generic);
err:
   free(re);
   return NULL;
}

static int
eng_output_update(void *engine EINA_UNUSED, void *data, void *einfo, unsigned int w, unsigned int h)
{
   Render_Engine *re = data;
   Evas_Engine_Info_Drm *info;

   info = (Evas_Engine_Info_Drm *)einfo;

   _outbuf_reconfigure(re->generic.ob, w, h, info->rotation, info->depth);

   evas_render_engine_software_generic_update(&re->generic,
                                              re->generic.ob, w, h);

   return 1;
}

static void
eng_output_free(void *engine, void *data)
{
   Render_Engine *re = data;

   evas_render_engine_software_generic_clean(engine, &re->generic);
   free(re);
}

static int
module_open(Evas_Module *em)
{
   /* check for valid evas module */
   if (!em) return 0;

   /* try to inherit functions from software_generic engine */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic",
                                    sizeof(Evas_Engine_Info_Drm)))
     return 0;

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
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info_setup);
   ORD(output_setup);
   ORD(output_update);
   ORD(output_free);

   /* advertise our engine functions */
   em->functions = (void *)(&func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister the eina log domain for this engine */
   if (_evas_engine_drm_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_drm_log_dom);
        _evas_engine_drm_log_dom = -1;
     }

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
