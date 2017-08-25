#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_FB.h"

#include <Ecore.h>
#include <Eina.h>

int _evas_engine_fb_log_dom = -1;

static Eina_List *_outbufs = NULL;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef Render_Output_Software_Generic Render_Engine;

/* internal engine routines */
static void *
_output_setup(int w, int h, int rot, int vt, int dev, int refresh)
{
   Render_Engine *re;
   Outbuf *ob;

   re = calloc(1, sizeof(Render_Engine));
   if (!re)
     return NULL;

   evas_fb_outbuf_fb_init();

   /* get any stored performance metrics from device */
   ob = evas_fb_outbuf_fb_setup_fb(w, h, rot, OUTBUF_DEPTH_INHERIT, vt, dev, refresh);
   if (!ob) goto on_error;

   if (!evas_render_engine_software_generic_init(re, ob, NULL,
                                                 evas_fb_outbuf_fb_get_rot,
                                                 evas_fb_outbuf_fb_reconfigure,
                                                 NULL,
                                                 NULL,
                                                 evas_fb_outbuf_fb_new_region_for_update,
                                                 evas_fb_outbuf_fb_push_updated_region,
                                                 evas_fb_outbuf_fb_free_region_for_update,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 evas_fb_outbuf_fb_free,
                                                 evas_fb_outbuf_fb_get_width(ob),
                                                 evas_fb_outbuf_fb_get_height(ob)))
     goto on_error;

   /* no backbuf! */
   evas_fb_outbuf_fb_set_have_backbuf(ob, 0);
   _outbufs = eina_list_append(_outbufs, ob);
   return re;

 on_error:
   if (ob) evas_fb_outbuf_fb_free(ob);
   free(re);
   return NULL;
}

/* engine api this module provides */
static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_FB *einfo = info;

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine EINA_UNUSED, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_FB *info = in;

   return _output_setup(w,
                        h,
                        info->info.rotation,
                        info->info.virtual_terminal,
                        info->info.device_number,
                        info->info.refresh);
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re)
     {
        _outbufs = eina_list_remove(_outbufs, re->ob);
        evas_render_engine_software_generic_clean(re);
        free(re);
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return (re->ob->priv.fb.fb->fb_var.transp.length > 0);
}

/* module advertising code */
static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   _evas_engine_fb_log_dom = eina_log_domain_register
     ("evas-fb", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_fb_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info_setup);
   ORD(output_setup);
   ORD(canvas_alpha_get);
   ORD(output_free);

   func.info_size = sizeof (Evas_Engine_Info_FB);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_engine_fb_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_fb_log_dom);
        _evas_engine_fb_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
  EVAS_MODULE_API_VERSION,
  "fb",
  "none",
  {
    module_open,
    module_close
  }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, fb);

#ifndef EVAS_STATIC_BUILD_FB
EVAS_EINA_MODULE_DEFINE(engine, fb);
#endif
