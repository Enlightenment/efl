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
typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Render_Engine_Software_Generic generic;
};

typedef struct _Region_Push_Hook_Ctx {
   void *pixels;
   Outbuf *buf;
   int x;
   int y;
   int w;
   int h;
} Region_Push_Hook_Ctx;

/* prototypes we will use here */
static void *_output_setup(Evas *eo_e, int w, int h, int rot, int vt, int dev, int refresh,
                           void (*region_push_hook)(Evas *e, int x, int y, int w, int h,
                                                    const void *pixels));

static void *eng_info(Evas *eo_e);
static void eng_info_free(Evas *eo_e, void *info);
static void eng_output_free(void *data);

static void
_evas_fb_region_push_hook_call(void *data)
{
   Region_Push_Hook_Ctx *ctx = data;


   if (eina_list_data_find(_outbufs, ctx->buf))
     {
        ctx->buf->region_push_hook.cb(ctx->buf->region_push_hook.evas,
                                      ctx->x, ctx->y, ctx->w, ctx->h,
                                      ctx->pixels);
     }

   free(ctx->pixels);
   free(ctx);
}

void
evas_fb_region_push_hook_call(Outbuf *buf, int x, int y, int w, int h,
                              const void *pixels)
{
   Region_Push_Hook_Ctx *ctx;
   size_t s;

   if (!buf->region_push_hook.cb)
     return;

   s = w * h * buf->priv.fb.fb->bpp;
   ctx = malloc(sizeof(Region_Push_Hook_Ctx));
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   ctx->pixels = malloc(s);
   EINA_SAFETY_ON_NULL_GOTO(ctx->pixels, err_pixels);
   ctx->x = x;
   ctx->y = y;
   ctx->w = w;
   ctx->h = h;
   ctx->buf = buf;
   memcpy(ctx->pixels, pixels, s);

   ecore_main_loop_thread_safe_call_async(_evas_fb_region_push_hook_call, ctx);
   return;

 err_pixels:
   free(ctx);
}

/* internal engine routines */
static void *
_output_setup(Evas *eo_e, int w, int h, int rot, int vt, int dev, int refresh,
              void (*region_push_hook)(Evas *e, int x, int y, int w, int h,
                                       const void *pixels))
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

   ob->region_push_hook.cb = region_push_hook;
   ob->region_push_hook.evas = eo_e;
   if (!evas_render_engine_software_generic_init(&re->generic, ob, NULL,
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

static void *
eng_setup(void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_FB *info = in;

   return _output_setup(w,
                        h,
                        info->info.rotation,
                        info->info.virtual_terminal,
                        info->info.device_number,
                        info->info.refresh,
                        info->func.region_push_hook);
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re)
     {
        _outbufs = eina_list_remove(_outbufs, re->generic.ob);
        evas_render_engine_software_generic_clean(&re->generic);
        free(re);
     }
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
