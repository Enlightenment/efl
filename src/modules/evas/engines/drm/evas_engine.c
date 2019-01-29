#include "evas_engine.h"

typedef struct _Render_Engine
{
   Render_Output_Software_Generic generic;
} Render_Engine;

static Evas_Func func, pfunc;

int _evas_engine_drm_log_dom;

/* engine functions we override */
static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_Drm *einfo;

   DBG("Engine Info Setup");

   einfo = info;
   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine, void *info, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Drm *einfo;
   Render_Engine *re;
   Outbuf *ob;

   einfo = info;

   re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;

   ob = _outbuf_setup(einfo, w, h);
   if (!ob) goto err;

   if (!evas_render_engine_software_generic_init(engine, &re->generic, ob,
                                                 NULL, //swap_mode_get
                                                 _outbuf_get_rotation,
                                                 _outbuf_reconfigure,
                                                 NULL, //region_first_rect
                                                 NULL, //damage_region_set
                                                 _outbuf_new_region_for_update,
                                                 NULL, //push_updated_region
                                                 NULL, //free_region_for_update
                                                 NULL, //idle_flush
                                                 NULL, //flush
                                                 NULL, //redraws_clear
                                                 _outbuf_free, //free
                                                 w, h))
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

static void
eng_output_free(void *engine, void *data)
{
   Render_Engine *re;

   re = data;
   evas_render_engine_software_generic_clean(engine, &re->generic);
   free(re);
}

static int
eng_output_update(void *engine EINA_UNUSED, void *data, void *info, unsigned int w, unsigned int h)
{
   Outbuf *ob;
   Render_Engine *re;
   Evas_Engine_Info_Drm *einfo;

   re = data;
   einfo = info;
   ob = re->generic.ob;

   if ((ob->w != (int)w) || (ob->h != (int)h) ||
       (ob->rotation != (int)einfo->info.rotation))
     {
        _outbuf_reconfigure(ob, w, h, einfo->info.rotation, einfo->info.depth);
        evas_render_engine_software_generic_update(&re->generic, ob, w, h);
     }

   return 1;
}

/* module functions */
static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   /* try to inherit functions from software_generic engine */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic",
                                    sizeof(Evas_Engine_Info_Drm)))
     return 0;

   /* register eina logging domain */
   _evas_engine_drm_log_dom =
     eina_log_domain_register("evas-drm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_drm_log_dom < 0)
     {
        EINA_LOG_ERR("Could not create module log domain");
        return 0;
     }

   /* store parent engine functions so we can override the ones we want */
   func = pfunc;

   /* override engine functions we provide */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info_setup);
   ORD(output_setup);
   ORD(output_free);
   ORD(output_update);

   /* advertise our engine functions */
   em->functions = (void *)(&func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister eina logging domain */
   if (_evas_engine_drm_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_drm_log_dom);
        _evas_engine_drm_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "drm", "none", { module_open, module_close }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, drm);

#ifndef EVAS_STATIC_BUILD_DRM
EVAS_EINA_MODULE_DEFINE(engine, drm);
#endif
