#include "evas_common_private.h" /* Also includes international specific stuff */
#include "evas_engine.h"

#include "evas_private.h"

#include <dlfcn.h>      /* dlopen,dlclose,etc */
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"


Evas_Gl_Symbols glsym_evas_gl_symbols = NULL;
Evas_GL_Common_Context_New glsym_evas_gl_common_context_new = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_free = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_flush = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_use = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_done = NULL;
Evas_GL_Common_Context_Resize_Call glsym_evas_gl_common_context_resize = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_newframe = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_lock = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_unlock = NULL;
static Evas_GL_Preload glsym_evas_gl_preload_init = NULL;
static Evas_GL_Preload glsym_evas_gl_preload_shutdown = NULL;

int _evas_engine_gl_cocoa_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static Eina_Bool _initted = EINA_FALSE;
static int _gl_wins = 0;

static void *
evgl_eng_display_get(void *data EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

static void *
evgl_eng_native_window_create(void *data EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

static int
evgl_eng_native_window_destroy(void *data          EINA_UNUSED,
                               void *native_window EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(1);
}

static void *
evgl_eng_window_surface_create(void *data          EINA_UNUSED,
                               void *native_window EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

static int
evgl_eng_window_surface_destroy(void *data EINA_UNUSED,
                                void *surface EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(1);
}

static void *
evgl_eng_context_create(void                    *data      EINA_UNUSED,
                        void                    *share_ctx EINA_UNUSED,
                        Evas_GL_Context_Version  version   EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

static int
evgl_eng_context_destroy(void *data    EINA_UNUSED,
                         void *context EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(1);
}

static int
evgl_eng_make_current(void *data    EINA_UNUSED,
                      void *surface EINA_UNUSED,
                      void *context EINA_UNUSED,
                      int   flush   EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(EINA_FALSE);
}

static void *
evgl_eng_proc_address_get(const char *name EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

static const char *
evgl_eng_string_get(void *data EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(NULL);
}

static int
evgl_eng_rotation_angle_get(void *data EINA_UNUSED)
{
   GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(0);
}

static const EVGL_Interface evgl_funcs =
{
   evgl_eng_display_get,
   NULL,
   evgl_eng_native_window_create,
   evgl_eng_native_window_destroy,
   evgl_eng_window_surface_create,
   evgl_eng_window_surface_destroy,
   evgl_eng_context_create,
   evgl_eng_context_destroy,
   evgl_eng_make_current,
   evgl_eng_proc_address_get,
   evgl_eng_string_get,
   evgl_eng_rotation_angle_get,
   NULL, // PBuffer
   NULL, // PBuffer
   NULL, // OpenGL-ES 1
   NULL, // OpenGL-ES 1
   NULL, // OpenGL-ES 1
   NULL, // native_win_surface_config_get
};


static void *
eng_output_info(void)
{
   Evas_Engine_Info_GL_Cocoa *info;

   info = calloc(1, sizeof(*info));
   if (EINA_UNLIKELY(!info))
     {
        CRI("Failed to allocate memory");
        return NULL;
     }
   info->magic.magic = rand();
   return info;
}

static void
eng_output_info_free(void *info)
{
   Evas_Engine_Info_GL_Cocoa *const in = info;
   free(in);
}

static void *
eng_output_setup(void *engine EINA_UNUSED, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_GL_Cocoa *const info = in;
   Render_Engine *re;
   Outbuf *ob;
   Eina_Bool chk;

   // TODO SWAP MODE

   if (!_initted)
     {
        glsym_evas_gl_preload_init();
        _initted = EINA_TRUE;
     }

   re = calloc(1, sizeof(*re));
   if (EINA_UNLIKELY(!re))
     {
        CRI("Failed to allocate memory");
        goto err;
     }

   ob = evas_outbuf_new(info, w, h);
   if (EINA_UNLIKELY(!ob))
     {
        CRI("Failed to create outbuf");
        goto err;
     }

   info->view = ob->ns_gl_view;

   chk = evas_render_engine_gl_generic_init(&re->generic, ob,
                                            evas_outbuf_buffer_state_get,
                                            evas_outbuf_rot_get,
                                            evas_outbuf_reconfigure,
                                            evas_outbuf_update_region_first_rect,
                                            NULL,
                                            evas_outbuf_update_region_new,
                                            evas_outbuf_update_region_push,
                                            evas_outbuf_update_region_free,
                                            NULL,
                                            evas_outbuf_flush,
                                            NULL,
                                            evas_outbuf_free,
                                            evas_outbuf_use,
                                            evas_outbuf_gl_context_get,
                                            evas_outbuf_egl_display_get,
                                            evas_outbuf_gl_context_new,
                                            evas_outbuf_gl_context_use,
                                            &evgl_funcs, ob->w, ob->h);
   if (EINA_UNLIKELY(!ob))
     {
        CRI("Failed to initialize gl_generic");
        evas_outbuf_free(re->win);
        goto err;
     }
   re->win = ob;
   _gl_wins++;

   evas_outbuf_use(re->win);
   return re;

err:
   free(re);
   return NULL;
}

static int
eng_output_update(void *engine EINA_UNUSED,
                  void         *data EINA_UNUSED,
                  void         *info EINA_UNUSED,
                  unsigned int  w    EINA_UNUSED,
                  unsigned int  h    EINA_UNUSED)
{
   //Evas_Engine_Info_GL_Cocoa *const info = info;
   //Render_Engine *re = data;

   CRI("ALREADY A DATA OUTPUT. THIS PART IS NOT IMPLEMENTED YET. PLEASE REPORT.");
   return 0;
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *const re = data;

   evas_outbuf_free(re->win);
   free(re);

   _gl_wins--;
   if (_initted && (_gl_wins == 0))
     {
        glsym_evas_gl_preload_shutdown();
        _initted = EINA_FALSE;
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data EINA_UNUSED)
{
   return EINA_TRUE;
}

static void
_gl_symbols(void)
{
   static Eina_Bool done = EINA_FALSE;

   if (done) return;

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym);

   LINK2GENERIC(evas_gl_common_context_new);
   LINK2GENERIC(evas_gl_common_context_flush);
   LINK2GENERIC(evas_gl_common_context_free);
   LINK2GENERIC(evas_gl_common_context_use);
   LINK2GENERIC(evas_gl_common_context_done);
   LINK2GENERIC(evas_gl_common_context_resize);
   LINK2GENERIC(evas_gl_common_context_newframe);
   LINK2GENERIC(evas_gl_preload_render_lock);
   LINK2GENERIC(evas_gl_preload_render_unlock);
   LINK2GENERIC(evas_gl_preload_init);
   LINK2GENERIC(evas_gl_preload_shutdown);

   LINK2GENERIC(evas_gl_symbols);

#undef LINK2GENERIC

   done = EINA_TRUE;
}


static int
module_open(Evas_Module *em)
{
   if (!em) return 0;

   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic")) return 0;

   if (_evas_engine_gl_cocoa_log_dom < 0)
     {
        _evas_engine_gl_cocoa_log_dom =
           eina_log_domain_register("evas-gl_cocoa", EVAS_DEFAULT_LOG_COLOR);
        if (EINA_UNLIKELY(_evas_engine_gl_cocoa_log_dom < 0))
          {
             EINA_LOG_ERR("Cannot create a module log domain");
             return 0;
          }
     }

   /* store it for later use */
   func = pfunc;

   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info);
   ORD(output_info_free);
   ORD(output_setup);
   ORD(output_update);
   ORD(canvas_alpha_get);
   ORD(output_free);

   _gl_symbols();

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_engine_gl_cocoa_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_gl_cocoa_log_dom);
        _evas_engine_gl_cocoa_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_cocoa",
   "none",
   {
      module_open,
      module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_cocoa);

#ifndef EVAS_STATIC_BUILD_GL_COCOA
EVAS_EINA_MODULE_DEFINE(engine, gl_cocoa);
#endif
