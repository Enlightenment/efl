#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_FB.h"

int _evas_engine_fb_log_dom = -1;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Render_Engine_Software_Generic generic;
};

/* prototypes we will use here */
static void *_output_setup(int w, int h, int rot, int vt, int dev, int refresh);

static void *eng_info(Evas *eo_e);
static void eng_info_free(Evas *eo_e, void *info);
static int eng_setup(Evas *eo_e, void *info);
static void eng_output_free(void *data);

/* internal engine routines */
static void *
_output_setup(int w, int h, int rot, int vt, int dev, int refresh)
{
   Render_Engine *re;
   Outbuf *ob;

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

   evas_fb_outbuf_fb_init();

   /* get any stored performance metrics from device */
   ob = evas_fb_outbuf_fb_setup_fb(w, h, rot, OUTBUF_DEPTH_INHERIT, vt, dev, refresh);
   if (!ob) goto on_error;

   if (!evas_render_engine_software_generic_init(&re->generic, ob, NULL,
                                                 evas_fb_outbuf_fb_get_rot,
                                                 evas_fb_outbuf_fb_reconfigure,
                                                 NULL,
                                                 evas_fb_outbuf_fb_new_region_for_update,
                                                 evas_fb_outbuf_fb_push_updated_region,
                                                 evas_fb_outbuf_fb_free_region_for_update,
                                                 NULL,
                                                 NULL,
                                                 evas_fb_outbuf_fb_free,
                                                 evas_fb_outbuf_fb_get_width(ob),
                                                 evas_fb_outbuf_fb_get_height(ob)))
     goto on_error;

   /* no backbuf! */
   evas_fb_outbuf_fb_set_have_backbuf(ob, 0);
   return re;

 on_error:
   if (ob) evas_fb_outbuf_fb_free(ob);
   free(re);
   evas_common_font_shutdown();
   evas_common_image_shutdown();
   return NULL;
}

/* engine api this module provides */
static void *
eng_info(Evas *eo_e EINA_UNUSED)
{
   Evas_Engine_Info_FB *info;
   info = calloc(1, sizeof(Evas_Engine_Info_FB));
   if (!info) return NULL;
   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;
   return info;
}

static void
eng_info_free(Evas *eo_e EINA_UNUSED, void *info)
{
   Evas_Engine_Info_FB *in;
   in = (Evas_Engine_Info_FB *)info;
   free(in);
}

static int
eng_setup(Evas *eo_e, void *in)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CANVAS_CLASS);
   Render_Engine *re;
   Evas_Engine_Info_FB *info;

   info = (Evas_Engine_Info_FB *)in;
   re = _output_setup(e->output.w,
		      e->output.h,
		      info->info.rotation,
		      info->info.virtual_terminal,
		      info->info.device_number,
		      info->info.refresh);
   e->engine.data.output = re;
   if (!e->engine.data.output) return 0;
   e->engine.data.context = e->engine.func->context_new(e->engine.data.output);

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   evas_render_engine_software_generic_clean(&re->generic);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static Eina_Bool
eng_canvas_alpha_get(void *data, void *context EINA_UNUSED)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return (re->generic.ob->priv.fb.fb->fb_var.transp.length > 0);
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
  eina_log_domain_unregister(_evas_engine_fb_log_dom);
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
