#include "evas_engine.h"
#include "../software_generic/evas_native_common.h"

typedef struct _Render_Engine
{
   Render_Engine_Software_Generic generic;

   int fd;
} Render_Engine;

struct scanout_handle
{
   Evas_Native_Scanout_Handler handler;
   void *data;
};

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

   re->fd = info->info.fd;

   if (!evas_render_engine_software_generic_init(&re->generic, ob,
                                                 _outbuf_state_get,
                                                 _outbuf_rotation_get,
                                                 _outbuf_reconfigure,
                                                 NULL,
                                                 NULL,
                                                 _outbuf_update_region_new,
                                                 _outbuf_update_region_push,
                                                 _outbuf_update_region_free,
                                                 NULL,
                                                 _outbuf_flush,
                                                 _outbuf_redraws_clear,
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

static void *
eng_setup(void *engine EINA_UNUSED, void *einfo, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Drm *info = einfo;

   return _render_engine_setup(info, w, h);
}

static int
eng_update(void *engine EINA_UNUSED, void *data, void *einfo, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Drm *info = einfo;
   Render_Engine *re = data;

   _outbuf_reconfigure(re->generic.ob, w, h,
                       info->info.rotation, info->info.depth);

   evas_render_engine_software_generic_update(&re->generic,
                                              re->generic.ob, w, h);

   return 1;
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   re = data;
   if (re)
     {
        evas_render_engine_software_generic_clean(&re->generic);
        free(re);
     }
}

static Ecore_Drm2_Fb *
drm_import_simple_dmabuf(int fd, struct dmabuf_attributes *attributes)
{
   unsigned int stride[4] = { 0 };
   int dmabuf_fd[4] = { 0 };
   int i;

   for (i = 0; i < attributes->n_planes; i++)
     {
        stride[i] = attributes->stride[i];
        dmabuf_fd[i] = attributes->fd[i];
     }

   return ecore_drm2_fb_dmabuf_import(fd, attributes->width,
                                      attributes->height, 32, 32,
                                      attributes->format, stride,
                                      dmabuf_fd, attributes->n_planes);
}

static void
_eng_fb_release(Ecore_Drm2_Fb *fb EINA_UNUSED, Ecore_Drm2_Fb_Status status, void *data)
{
   struct scanout_handle *sh;

   sh = data;
   if (status == ECORE_DRM2_FB_STATUS_DELETED)
     {
        free(sh);
        return;
     }

   if (!sh->handler) return;

   switch (status)
     {
      case ECORE_DRM2_FB_STATUS_SCANOUT_ON:
        sh->handler(sh->data, EVAS_NATIVE_SURFACE_STATUS_SCANOUT_ON);
        break;
      case ECORE_DRM2_FB_STATUS_SCANOUT_OFF:
        sh->handler(sh->data, EVAS_NATIVE_SURFACE_STATUS_SCANOUT_OFF);
        break;
      case ECORE_DRM2_FB_STATUS_PLANE_ASSIGN:
        sh->handler(sh->data, EVAS_NATIVE_SURFACE_STATUS_PLANE_ASSIGN);
        break;
      case ECORE_DRM2_FB_STATUS_PLANE_RELEASE:
        sh->handler(sh->data, EVAS_NATIVE_SURFACE_STATUS_PLANE_RELEASE);
        break;
      default:
        ERR("Unhandled framebuffer status");
     }
}

static void *
eng_image_plane_assign(void *data, void *image, int x, int y)
{
   Render_Engine *re;
   Outbuf *ob;
   RGBA_Image *img;
   Native *n;
   Ecore_Drm2_Fb *fb = NULL;
   Ecore_Drm2_Plane *plane;
   struct scanout_handle *g;

   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);

   re = (Render_Engine *)data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(re, EINA_FALSE);

   ob = re->generic.ob;
   EINA_SAFETY_ON_NULL_RETURN_VAL(ob, EINA_FALSE);

   img = image;
   n = img->native.data;

   /* Perhaps implementable on other surface types, but we're
    * sticking to this one for now */
   if (n->ns.type != EVAS_NATIVE_SURFACE_WL_DMABUF) return NULL;

   fb = drm_import_simple_dmabuf(re->fd, &n->ns_data.wl_surface_dmabuf.attr);

   if (!fb) return NULL;

   g = calloc(1, sizeof(struct scanout_handle));
   if (!g) goto out;

   g->handler = n->ns.data.wl_dmabuf.scanout.handler;
   g->data = n->ns.data.wl_dmabuf.scanout.data;
   ecore_drm2_fb_status_handler_set(fb, _eng_fb_release, g);

   /* Fail or not, we're going to drop that fb and let refcounting get rid of
    * it later
    */
   plane = ecore_drm2_plane_assign(ob->priv.output, fb, x, y);

out:
   ecore_drm2_fb_discard(fb);
   return plane;
}

static void
eng_image_plane_release(void *data EINA_UNUSED, void *image EINA_UNUSED, void *plin)
{
   Ecore_Drm2_Plane *plane = plin;

   ecore_drm2_plane_release(plane);
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
   EVAS_API_OVERRIDE(update, &func, eng_);
   EVAS_API_OVERRIDE(output_free, &func, eng_);
   EVAS_API_OVERRIDE(image_plane_assign, &func, eng_);
   EVAS_API_OVERRIDE(image_plane_release, &func, eng_);

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
