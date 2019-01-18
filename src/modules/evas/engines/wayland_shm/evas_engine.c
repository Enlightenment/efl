#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_engine.h"
#include "../software_generic/evas_native_common.h"

#ifdef HAVE_DLSYM
# include <dlfcn.h>
#endif

/* logging domain variable */
int _evas_engine_way_shm_log_dom = -1;

/* evas function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static Evas_Native_Tbm_Surface_Image_Set_Call  glsym__evas_native_tbm_surface_image_set = NULL;
static Evas_Native_Tbm_Surface_Stride_Get_Call  glsym__evas_native_tbm_surface_stride_get = NULL;

/* engine structure data */
typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   Render_Output_Software_Generic generic;
};

static void *
eng_output_setup(void *engine, void *info, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Wayland *einfo = info;
   Render_Engine *re;
   Outbuf *ob;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to allocate space for new render engine */
   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   ob = _evas_outbuf_setup(w, h, einfo);
   if (!ob) goto err;

   if (!evas_render_engine_software_generic_init(engine, &re->generic, ob,
                                                 _evas_outbuf_swap_mode_get,
                                                 _evas_outbuf_rotation_get,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 _evas_outbuf_update_region_new,
                                                 _evas_outbuf_update_region_push,
                                                 NULL,
                                                 _evas_outbuf_idle_flush,
                                                 _evas_outbuf_flush,
                                                 _evas_outbuf_redraws_clear,
                                                 _evas_outbuf_free,
                                                 w, h))
     goto err;

   evas_render_engine_software_generic_merge_mode_set(&re->generic);

   re->generic.ob->info = einfo;

   /* return allocated render engine */
   return re;

err:
   if (ob) _evas_outbuf_free(ob);
   free(re);
   return NULL;
}

static void
_symbols(void)
{
   static int done = 0;

   if (done) return;

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym);

   // Get function pointer to native_common that is now provided through the link of SW_Generic.
   LINK2GENERIC(_evas_native_tbm_surface_image_set);
   LINK2GENERIC(_evas_native_tbm_surface_stride_get);

   done = 1;
}

/* ENGINE API FUNCTIONS WE PROVIDE */
static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_Wayland *einfo = info;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void
eng_output_resize(void *engine EINA_UNUSED, void *data, int w, int h)
{
   Render_Engine *re;
   Evas_Engine_Info_Wayland *einfo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(re = (Render_Engine *)data)) return;
   if (!(einfo = re->generic.ob->info)) return;

   _evas_outbuf_reconfigure(re->generic.ob, w, h,
                            einfo->info.rotation, einfo->info.depth,
                            einfo->info.destination_alpha, 0);

   evas_common_tilebuf_free(re->generic.tb);
   if ((re->generic.tb = evas_common_tilebuf_new(w, h)))
     evas_common_tilebuf_set_tile_size(re->generic.tb, TILESIZE, TILESIZE);

   re->generic.w = w;
   re->generic.h = h;
}

static int
eng_output_update(void *engine, void *data, void *info, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Wayland *einfo = info;
   Render_Engine *re = data;
   Ecore_Wl2_Display *ewd;
   Outbuf *ob;

   ob = re->generic.ob;
   ewd = ecore_wl2_window_display_get(einfo->info.wl2_win);
   if (ob->ewd != ewd)
     {
        /* We don't use a purging flush because we don't want to
         * delete a buffer currently being displayed */
        if (ewd)
          ecore_wl2_surface_flush(ob->surface, EINA_FALSE);
        re->generic.ob->ewd = ewd;
     }

   eng_output_resize(engine, data, w, h);

   evas_render_engine_software_generic_update(&re->generic, re->generic.ob,
                                              w, h);

   return 1;
}

static void
eng_output_free(void *engine, void *data)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     {
        evas_render_engine_software_generic_clean(engine, &re->generic);
        free(re);
     }
}

static int
eng_image_native_init(void *engine EINA_UNUSED, Evas_Native_Surface_Type type)
{
   switch (type)
     {
      case EVAS_NATIVE_SURFACE_TBM:
        return _evas_native_tbm_init();
      case EVAS_NATIVE_SURFACE_EVASGL:
        return 1;
      default:
        ERR("Native surface type %d not supported!", type);
        return 0;
     }
}

static void
eng_image_native_shutdown(void *engine EINA_UNUSED, Evas_Native_Surface_Type type)
{
   switch (type)
     {
      case EVAS_NATIVE_SURFACE_TBM:
        _evas_native_tbm_shutdown();
        return;
      default:
        ERR("Native surface type %d not supported!", type);
        return;
     }
}

static void
_native_evasgl_free(void *image)
{
   RGBA_Image *im = image;
   Native *n = im->native.data;

   im->native.data        = NULL;
   im->native.func.bind   = NULL;
   im->native.func.unbind = NULL;
   im->native.func.free   = NULL;
   //im->image.data         = NULL;
   free(n);
}

static void *
eng_image_native_set(void *engine EINA_UNUSED, void *image, void *native)
{
   Evas_Native_Surface *ns = native;
   Image_Entry *ie = image;
   RGBA_Image *im = image, *im2;
   int stride;

   if (!im || !ns) return im;

   if (ns->type == EVAS_NATIVE_SURFACE_TBM)
     {
        if (im->native.data)
          {
             //image have native surface already
             Evas_Native_Surface *ens = im->native.data;

             if ((ens->type == ns->type) &&
                 (ens->data.tbm.buffer == ns->data.tbm.buffer))
                return im;
          }
      }

   if (ns->type == EVAS_NATIVE_SURFACE_EVASGL)
     {
        im2 = (RGBA_Image *) evas_cache_image_data(evas_common_image_cache_get(),
                                                   ie->w, ie->h, ns->data.evasgl.surface, 1,
                                                   EVAS_COLORSPACE_ARGB8888);
     }
   else if (ns->type == EVAS_NATIVE_SURFACE_TBM)
     {
        stride = glsym__evas_native_tbm_surface_stride_get(NULL, ns);
        im2 = (RGBA_Image *)evas_cache_image_copied_data(evas_common_image_cache_get(),
                                                         stride, ie->h, NULL, ie->flags.alpha,
                                                         EVAS_COLORSPACE_ARGB8888);
     }
   else
     im2 = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                               ie->w, ie->h,
                                               NULL, 1,
                                               EVAS_COLORSPACE_ARGB8888);

   if (im->native.data)
      {
         if (im->native.func.free)
            im->native.func.free(im);
      }

   evas_cache_image_drop(ie);
   im = im2;

   if (ns->type == EVAS_NATIVE_SURFACE_TBM)
     {
        return glsym__evas_native_tbm_surface_image_set(NULL, im, ns);
     }
   else if (ns->type == EVAS_NATIVE_SURFACE_EVASGL)
     {
        /* Native contains Evas_Native_Surface. What a mess. */
        Native *n = calloc(1, sizeof(Native));
        if (n)
          {
             n->ns_data.evasgl.surface = ns->data.evasgl.surface;
             n->ns.type = EVAS_NATIVE_SURFACE_EVASGL;
             n->ns.version = EVAS_NATIVE_SURFACE_VERSION;
             n->ns.data.evasgl.surface = ns->data.evasgl.surface;
             im->native.data = n;
             im->native.func.free = _native_evasgl_free;
             im->native.func.bind = NULL;
             im->native.func.unbind = NULL;
          }
     }

   return im;
}

static void *
eng_image_native_get(void *engine EINA_UNUSED, void *image)
{
   RGBA_Image *im = image;
   Native *n;
   if (!im) return NULL;
   n = im->native.data;
   if (!n) return NULL;
   return &(n->ns);
}

/* EVAS MODULE FUNCTIONS */
static int
module_open(Evas_Module *em)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid evas module */
   if (!em) return 0;

   /* try to get functions from whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic",
                                    sizeof(Evas_Engine_Info_Wayland)))
     return 0;

   /* try to create our logging domain */
   _evas_engine_way_shm_log_dom =
     eina_log_domain_register("evas-wayland_shm", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_way_shm_log_dom < 0)
     {
        EINA_LOG_ERR("Cannot create a module logging domain");
        return 0;
     }

   /* copy parent functions */
   func = pfunc;

   /* override engine specific functions */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info_setup);
   ORD(output_setup);
   ORD(output_update);
   ORD(output_free);
   ORD(output_resize);
   ORD(image_native_set);
   ORD(image_native_get);
   ORD(image_native_init);
   ORD(image_native_shutdown);

   _symbols();
   /* advertise our own engine functions */
   em->functions = (void *)(&func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* unregister logging domain */
   if (_evas_engine_way_shm_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_way_shm_log_dom);
        _evas_engine_way_shm_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, "wayland_shm", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_shm);

#ifndef EVAS_STATIC_BUILD_WAYLAND_SHM
EVAS_EINA_MODULE_DEFINE(engine, wayland_shm);
#endif
