#include "config.h"
#include "evas_engine.h"
#include <wayland-client.h>

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error eglfs should not get compiled if dlsym is not found on the system!
#endif

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif

/* external variables */
int _evas_engine_eglfs_log_dom = -1;
int _extn_have_buffer_age = 1;

/* local variables */
static Eina_Bool initted = EINA_FALSE;
static int gl_wins = 0;

/* local structures */
typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   Render_Output_GL_Generic generic;
};

typedef struct _Native Native;
struct _Native
{
   Evas_Native_Surface ns;
   struct wl_buffer *wl_buf;
   void *egl_surface;
};

/* local function prototype types */
typedef void (*_eng_fn)(void);
typedef _eng_fn (*glsym_func_eng_fn)();
typedef void (*glsym_func_void)();
typedef void *(*glsym_func_void_ptr)();
typedef int (*glsym_func_int)();
typedef unsigned int (*glsym_func_uint)();
typedef const char *(*glsym_func_const_char_ptr)();

/* external dynamic loaded Evas_GL function pointers */
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_ref = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_unref = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_free = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_native_disable = NULL;
Evas_GL_Common_Image_Call glsym_evas_gl_common_image_native_enable = NULL;
Evas_GL_Common_Image_New_From_Data glsym_evas_gl_common_image_new_from_data = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_image_all_unload = NULL;
Evas_GL_Preload glsym_evas_gl_preload_init = NULL;
Evas_GL_Preload glsym_evas_gl_preload_shutdown = NULL;
EVGL_Engine_Call glsym_evgl_engine_shutdown = NULL;
EVGL_Current_Native_Context_Get_Call glsym_evgl_current_native_context_get = NULL;
Evas_Gl_Symbols glsym_evas_gl_symbols = NULL;

Evas_GL_Common_Context_New glsym_evas_gl_common_context_new = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_flush = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_free = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_use = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_newframe = NULL;
Evas_GL_Common_Context_Call glsym_evas_gl_common_context_done = NULL;
Evas_GL_Common_Context_Resize_Call glsym_evas_gl_common_context_resize = NULL;
Evas_GL_Common_Buffer_Dump_Call glsym_evas_gl_common_buffer_dump = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_lock = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_unlock = NULL;
Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_relax = NULL;

glsym_func_void_ptr glsym_evas_gl_common_current_context_get = NULL;

/* dynamic loaded local egl function pointers */
_eng_fn (*glsym_eglGetProcAddress)(const char *a) = NULL;
void *(*glsym_eglCreateImage)(EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void (*glsym_eglDestroyImage)(EGLDisplay a, void *b) = NULL;
void (*glsym_glEGLImageTargetTexture2DOES)(int a, void *b)  = NULL;
unsigned int (*glsym_eglSwapBuffersWithDamage)(EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;
unsigned int (*glsym_eglQueryWaylandBufferWL)(EGLDisplay a, struct wl_resource *b, EGLint c, EGLint *d) = NULL;

/* local function prototypes */
static void gl_symbols(void);
static void gl_extn_veto(Render_Engine *re);

static void *evgl_eng_display_get(void *data);
static void *evgl_eng_evas_surface_get(void *data);
static int evgl_eng_make_current(void *data, void *surface, void *context, int flush);
static void *evgl_eng_native_window_create(void *data);
static int evgl_eng_native_window_destroy(void *data, void *native_window);
static void *evgl_eng_window_surface_create(void *data, void *native_window);
static int evgl_eng_window_surface_destroy(void *data, void *surface);
static void *evgl_eng_context_create(void *data, void *share_ctx, Evas_GL_Context_Version version);
static int evgl_eng_context_destroy(void *data, void *context);
static const char *evgl_eng_string_get(void *data);
static void *evgl_eng_proc_address_get(const char *name);
static int evgl_eng_rotation_angle_get(void *data);

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;
static const EVGL_Interface evgl_funcs =
{
   evgl_eng_display_get,
   evgl_eng_evas_surface_get,
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


/* local inline functions */
static inline Outbuf *
eng_get_ob(Render_Engine *re)
{
   return re->generic.software.ob;
}

/* local functions */
static void
gl_symbols(void)
{
   static Eina_Bool done = EINA_FALSE;
   const char *exts = NULL;

   if (done) return;

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym);

   // Get function pointer to evas_gl_common that is now provided through the link of GL_Generic.
   LINK2GENERIC(evas_gl_common_image_all_unload);
   LINK2GENERIC(evas_gl_common_image_ref);
   LINK2GENERIC(evas_gl_common_image_unref);
   LINK2GENERIC(evas_gl_common_image_new_from_data);
   LINK2GENERIC(evas_gl_common_image_native_disable);
   LINK2GENERIC(evas_gl_common_image_free);
   LINK2GENERIC(evas_gl_common_image_native_enable);
   LINK2GENERIC(evas_gl_common_context_new);
   LINK2GENERIC(evas_gl_common_context_flush);
   LINK2GENERIC(evas_gl_common_context_free);
   LINK2GENERIC(evas_gl_common_context_use);
   LINK2GENERIC(evas_gl_common_context_newframe);
   LINK2GENERIC(evas_gl_common_context_done);
   LINK2GENERIC(evas_gl_common_context_resize);
   LINK2GENERIC(evas_gl_common_buffer_dump);
   LINK2GENERIC(evas_gl_preload_render_lock);
   LINK2GENERIC(evas_gl_preload_render_unlock);
   LINK2GENERIC(evas_gl_preload_render_relax);
   LINK2GENERIC(evas_gl_preload_init);
   LINK2GENERIC(evas_gl_preload_shutdown);
   LINK2GENERIC(evgl_engine_shutdown);
   LINK2GENERIC(evas_gl_symbols);

#define FINDSYM(dst, sym, typ) \
   if (glsym_eglGetProcAddress) { \
      if (!dst) dst = (typ)glsym_eglGetProcAddress(sym); \
   } else { \
      if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym); \
   }

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);

   // Find EGL extensions
   // FIXME: whgen above eglGetDisplay() is fixed... fix the below...
//   exts = eglQueryString(ob->egl_disp, EGL_EXTENSIONS);
   glsym_evas_gl_symbols((void*)glsym_eglGetProcAddress, exts);

   FINDSYM(glsym_eglCreateImage, "eglCreateImageKHR", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageEXT", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageARB", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImage", glsym_func_void_ptr);

   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageKHR", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageEXT", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageARB", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImage", glsym_func_void);

   FINDSYM(glsym_glEGLImageTargetTexture2DOES,
           "glEGLImageTargetTexture2DOES", glsym_func_void);

   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageEXT",
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageINTEL",
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamage",
           glsym_func_uint);

   FINDSYM(glsym_eglQueryWaylandBufferWL, "eglQueryWaylandBufferWL",
           glsym_func_uint);

   done = EINA_TRUE;
}

static void
gl_extn_veto(Render_Engine *re)
{
   const char *str = NULL;

   str = eglQueryString(eng_get_ob(re)->egl.disp, EGL_EXTENSIONS);
   if (str)
     {
        const char *s = NULL;

        if (getenv("EVAS_GL_INFO")) printf("EGL EXTN:\n%s\n", str);

        // Disable Partial Rendering
        s = getenv("EVAS_GL_PARTIAL_DISABLE");
        if ((s) && (atoi(s)))
          {
             _extn_have_buffer_age = 0;
             glsym_eglSwapBuffersWithDamage = NULL;
          }
        if (!strstr(str, "EGL_EXT_buffer_age")) _extn_have_buffer_age = 0;
        if (!strstr(str, "EGL_EXT_swap_buffers_with_damage"))
          glsym_eglSwapBuffersWithDamage = NULL;
     }
   else
     {
        if (getenv("EVAS_GL_INFO")) printf("NO EGL EXTN!\n");
        _extn_have_buffer_age = 0;
     }
}

static void *
evgl_eng_display_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   if (eng_get_ob(re))
     return (void *)eng_get_ob(re)->egl.disp;
   else
     return NULL;
}

static void *
evgl_eng_evas_surface_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   if (eng_get_ob(re))
     return (void *)eng_get_ob(re)->egl.surface[0];
   else
     return NULL;
}

static int
evgl_eng_make_current(void *data, void *surface, void *context, int flush)
{
   Render_Engine *re;
   EGLContext ctx;
   EGLSurface sfc;
   EGLDisplay dpy;
   int ret = 0;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

   dpy = eng_get_ob(re)->egl.disp;
   ctx = (EGLContext)context;
   sfc = (EGLSurface)surface;

   if ((!context) && (!surface))
     {
        ret = eglMakeCurrent(dpy, EGL_NO_SURFACE,
                             EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (!ret)
          {
             ERR("eglMakeCurrent() failed! Error Code=%#x", eglGetError());
             return 0;
          }

        return 1;
     }

   if ((eglGetCurrentContext() != ctx) ||
       (eglGetCurrentSurface(EGL_READ) != sfc) ||
       (eglGetCurrentSurface(EGL_DRAW) != sfc) )
     {
        if (flush) evas_outbuf_use(NULL);

        ret = eglMakeCurrent(dpy, sfc, sfc, ctx);
        if (!ret)
          {
             ERR("eglMakeCurrent() failed! Error Code=%#x", eglGetError());
             return 0;
          }
     }

   return 1;
}

static void _hwc_present_cb(void *user_data, struct ANativeWindow *window,
                            struct ANativeWindowBuffer *buffer)
{

}

static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re;
   Evas_Engine_Info_Eglfs *info;
   struct ANativeWindow *native_window;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   info = eng_get_ob(re)->info;
   if (!info)
     {
        ERR("Invalid Evas Engine Eglfs Info!");
        return NULL;
     }
   EGLNativeWindowType win;
   win = create_hwcomposernativewindow();
   return (void *)win;
}

static int
evgl_eng_native_window_destroy(void *data, void *native_window)
{
   Render_Engine *re = (Render_Engine *)data;

   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

   if (!native_window)
     {
        ERR("Invalid native surface.");
        return 0;
     }

   HWCNativeWindowDestroy(native_window);

   return 1;
}

static void *
evgl_eng_window_surface_create(void *data, void *native_window)
{
   Render_Engine *re;
   EGLSurface surface = EGL_NO_SURFACE;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   // Create resource surface for EGL
   surface = eglCreateWindowSurface(eng_get_ob(re)->egl.disp,
                                    eng_get_ob(re)->egl.config,
                                    (EGLNativeWindowType)native_window,
                                    NULL);
   if (!surface)
     {
        ERR("Creating window surface failed. Error: %#x.", eglGetError());
        return NULL;
     }

   return (void *)surface;
}

static int
evgl_eng_window_surface_destroy(void *data, void *surface)
{
   Render_Engine *re;
   EGLBoolean ret = EGL_FALSE;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

   if (!surface)
     {
        ERR("Invalid surface.");
        return 0;
     }

   ret = eglDestroySurface(eng_get_ob(re)->egl.disp, (EGLSurface)surface);
   if (ret == EGL_TRUE) return 1;

   return 0;
}

static void *
evgl_eng_context_create(void *data, void *share_ctx, Evas_GL_Context_Version version)
{
   Render_Engine *re;
   EGLContext context = EGL_NO_CONTEXT;
   int context_attrs[3];

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   if (version != EVAS_GL_GLES_2_X)
     {
        ERR("This engine only supports OpenGL-ES 2.0 contexts for now!");
        return NULL;
     }

   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   // Share context already assumes that it's sharing with evas' context
   if (share_ctx)
     {
        context = eglCreateContext(eng_get_ob(re)->egl.disp,
                                   eng_get_ob(re)->egl.config,
                                   (EGLContext)share_ctx,
                                   context_attrs);
     }
   else
     {
        context = eglCreateContext(eng_get_ob(re)->egl.disp,
                                   eng_get_ob(re)->egl.config,
                                   eng_get_ob(re)->egl.context[0], // Evas' GL Context
                                   context_attrs);
     }

   if (!context)
     {
        ERR("eglMakeCurrent() failed! Error Code=%#x", eglGetError());
        return NULL;
     }

   return (void *)context;
}

static int
evgl_eng_context_destroy(void *data, void *context)
{
   Render_Engine *re;
   EGLBoolean ret = EGL_FALSE;

   re = (Render_Engine *)data;
   if ((!re) || (!context))
     {
        ERR("Invalid Render Input Data. Engine: %p, Context: %p",
            data, context);
        return 0;
     }

   ret = eglDestroyContext(eng_get_ob(re)->egl.disp, (EGLContext)context);
   if (ret == EGL_TRUE) return 1;

   return 0;
}

static const char *
evgl_eng_string_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   return eglQueryString(eng_get_ob(re)->egl.disp, EGL_EXTENSIONS);
}

static void *
evgl_eng_proc_address_get(const char *name)
{
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
}

static int
evgl_eng_rotation_angle_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return 0;
     }

   if ((eng_get_ob(re)) && (eng_get_ob(re)->gl_context))
     return eng_get_ob(re)->gl_context->rot;
   else
     {
        ERR("Unable to retrieve rotation angle.");
        return 0;
     }
}

static Eina_Bool
eng_preload_make_current(void *data, void *doit)
{
   Outbuf *ob;

   ob = (Outbuf *)data;
   if (!ob) return EINA_FALSE;

   if (doit)
     {
        if (!eglMakeCurrent(ob->egl.disp, ob->egl.surface[0],
                            ob->egl.surface[0], ob->egl.context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(ob->egl.disp, EGL_NO_SURFACE,
                            EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_re_winfree(Render_Engine *re)
{
   if (!re) return;
   if (!eng_get_ob(re)->surf) return;
   glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));
   evas_outbuf_unsurf(eng_get_ob(re));
}

static void
_native_cb_bind(void *image)
{
   Evas_GL_Image *img;
   Native *n;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
        if (n->egl_surface)
          {
             if (glsym_glEGLImageTargetTexture2DOES)
               {
                  glsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, n->egl_surface);
                  if (eglGetError() != EGL_SUCCESS)
                    ERR("glEGLImageTargetTexture2DOES() failed.");
               }
             else
               ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
          }
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     glBindTexture(GL_TEXTURE_2D, n->ns.data.opengl.texture_id);

   /* TODO: NATIVE_SURFACE_TBM and NATIVE_SURFACE_EVASGL */
}

static void
_native_cb_unbind(void *image)
{
   Evas_GL_Image *img;
   Native *n;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;

   else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     glBindTexture(GL_TEXTURE_2D, 0);

   /* TODO: NATIVE_SURFACE_TBM and NATIVE_SURFACE_EVASGL */
}

static void
_native_cb_free(void *image)
{
   Evas_GL_Image *img;
   Native *n;
   uint32_t texid;
   void *wlid;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;
   if (!img->native.shared) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
        wlid = n->wl_buf;
        eina_hash_del(img->native.shared->native_wl_hash, &wlid, img);
        if (n->egl_surface)
          {
             if (glsym_eglDestroyImage)
               {
                  glsym_eglDestroyImage(img->native.disp, n->egl_surface);
                  if (eglGetError() != EGL_SUCCESS)
                    ERR("eglDestroyImage() failed.");
               }
             else
               ERR("Try eglDestroyImage on EGL with  no support");
          }
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        texid = n->ns.data.opengl.texture_id;
        eina_hash_del(img->native.shared->native_tex_hash, &texid, img);
     }

   img->native.data = NULL;
   img->native.func.bind = NULL;
   img->native.func.unbind = NULL;
   img->native.func.free = NULL;

   free(n);
}

/* engine specific override functions */
static void *
eng_output_info(void)
{
   Evas_Engine_Info_Eglfs *info;

   /* try to allocate space for our engine info */
   if (!(info = calloc(1, sizeof(Evas_Engine_Info_Eglfs))))
     return NULL;

   info->magic.magic = rand();
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void
eng_output_info_free(void *in)
{
   Evas_Engine_Info_Eglfs *info;

   if ((info = (Evas_Engine_Info_Eglfs *)in))
     free(info);
}

static void *
eng_output_setup(void *engine EINA_UNUSED, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Eglfs *info = in;
   Render_Engine *re = NULL;
   Outbuf *ob = NULL;
   Render_Engine_Merge_Mode merge_mode = MERGE_BOUNDING;
   Render_Engine_Swap_Mode swap_mode;

   swap_mode = evas_render_engine_gl_swap_mode_get(info->info.swap_mode);

   if (!initted)
     {
        glsym_evas_gl_preload_init();
     }

   if (!(re = calloc(1, sizeof(Render_Engine)))) return NULL;

   /* try to create new outbuf */
   ob = evas_outbuf_new(info, w, h, swap_mode);
   if (!ob) goto on_error;

   ob->evas = evas;

   if (!evas_render_engine_gl_generic_init(&re->generic, ob,
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
                                           &evgl_funcs, ob->w, ob->h))
     goto on_error;

   gl_wins++;

   s = getenv("EVAS_GL_PARTIAL_MERGE");
   if (s)
     {
        if ((!strcmp(s, "bounding")) || (!strcmp(s, "b")))
          merge_mode = MERGE_BOUNDING;
        else if ((!strcmp(s, "full")) || (!strcmp(s, "f")))
          merge_mode = MERGE_FULL;
     }

   evas_render_engine_software_generic_merge_mode_set(&re->generic.software, merge_mode);

   evas_outbuf_use(eng_get_ob(re));

   if (!initted)
     {
        gl_extn_veto(re);
        initted = EINA_TRUE;
     }

   return re;

 on_error:
   /* free outbuf */
   evas_outbuf_free(ob);
   free(re);
   return NULL;
}

static int
eng_output_update(void *engine EINA_UNUSED, void *data, void *info, unsigned int w, unsigned int h)
{
   Render_Engine *re = data;

   if (eng_get_ob(re) && _re_wincheck(eng_get_ob(re)))
     {
        if ((info->info.depth != eng_get_ob(re)->depth) ||
            (info->info.destination_alpha != eng_get_ob(re)->destination_alpha))
          {
             Outbuf *ob, *ob_old;

             ob_old = re->generic.software.ob;
             re->generic.software.ob = NULL;
             gl_wins--;

             ob = evas_outbuf_new(info, w, h, swap_mode);
             if (!ob)
               {
                  if (ob_old) evas_outbuf_free(ob_old);
                  return 0;
               }

             evas_outbuf_use(ob);
             if (ob_old) evas_outbuf_free(ob_old);

             ob->evas = evas;

             evas_render_engine_software_generic_update(&re->generic.software, ob,
                                                        w, h);

             gl_wins++;
          }
        else if ((eng_get_ob(re)->w != w) ||
                 (eng_get_ob(re)->h != h) ||
                 (info->info.rotation != eng_get_ob(re)->rotation))
          {
             evas_outbuf_reconfigure(eng_get_ob(re),
                                     w, h,
                                     info->info.rotation,
                                     info->info.depth);
             evas_render_engine_software_generic_update(&re->generic.software,
                                                        re->generic.software.ob,
                                                        w, h);
          }
     }

   evas_outbuf_use(eng_get_ob(re));

   return 1;
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re)
     {
        glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));

        if (gl_wins == 1) glsym_evgl_engine_shutdown(re);

        /* NB: evas_render_engine_software_generic_clean() frees ob */
        evas_render_engine_software_generic_clean(&re->generic.software);

        gl_wins--;

        free(re);
     }

   if ((initted == EINA_TRUE) && (gl_wins == 0))
     {
        glsym_evas_gl_preload_shutdown();
        initted = EINA_FALSE;
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re) return EINA_FALSE;

   return eng_get_ob(re)->destination_alpha;
}

static void
eng_output_dump(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!re) return;

   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   glsym_evas_gl_common_image_all_unload(eng_get_ob(re)->gl_context);
   _re_winfree(re);
}

static void *
eng_image_native_set(void *engine EINA_UNUSED, void *data, void *image, void *native)
{
   Render_Engine *re;
   Outbuf *ob;
   Native *n;
   Evas_Native_Surface *ns;
   Evas_GL_Image *img, *img2;
   unsigned int tex = 0, fbo = 0;
   uint32_t texid;
   void *wlid, *wl_buf = NULL;

   re = (Render_Engine *)data;
   if (!re) return NULL;

   ob = eng_get_ob(re);
   if (!ob) return NULL;

   ns = native;

   if (!(img = image))
     {
        if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
          {
             img =
               glsym_evas_gl_common_image_new_from_data(ob->gl_context,
                                                        ns->data.opengl.w,
                                                        ns->data.opengl.h,
                                                        NULL, 1,
                                                        EVAS_COLORSPACE_ARGB8888);
          }
        else
          return NULL;
     }

   if (ns)
     {
        if (ns->type == EVAS_NATIVE_SURFACE_WL)
          {
             wl_buf = ns->data.wl.legacy_buffer;
             if (img->native.data)
               {
                  Evas_Native_Surface *ens;

                  ens = img->native.data;
                  if (ens->data.wl.legacy_buffer == wl_buf)
                    return img;
               }
          }
        else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
          {
             tex = ns->data.opengl.texture_id;
             fbo = ns->data.opengl.framebuffer_id;
             if (img->native.data)
               {
                  Evas_Native_Surface *ens;

                  ens = img->native.data;
                  if ((ens->data.opengl.texture_id == tex) &&
                      (ens->data.opengl.framebuffer_id == fbo))
                     return img;
               }
          }
     }

   if ((!ns) && (!img->native.data)) return img;

   evas_outbuf_use(ob);

   if (img->native.data)
     {
        if (img->native.func.free)
          img->native.func.free(img);
        glsym_evas_gl_common_image_native_disable(img);
     }

   if (!ns) return img;

   if (ns->type == EVAS_NATIVE_SURFACE_WL)
     {
        wlid = wl_buf;
        img2 = eina_hash_find(ob->gl_context->shared->native_wl_hash, &wlid);
        if (img2 == img) return img;
        if (img2)
          {
             if((n = img2->native.data))
               {
                  glsym_evas_gl_common_image_ref(img2);
                  glsym_evas_gl_common_image_free(img);
                  return img2;
               }
          }
     }
   else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        texid = tex;
        img2 = eina_hash_find(ob->gl_context->shared->native_tex_hash, &texid);
        if (img2 == img) return img;
        if (img2)
          {
             if ((n = img2->native.data))
               {
                  glsym_evas_gl_common_image_ref(img2);
                  glsym_evas_gl_common_image_free(img);
                  return img2;
               }
          }
     }

   img2 = glsym_evas_gl_common_image_new_from_data(ob->gl_context, img->w,
                                                   img->h, NULL, img->alpha,
                                                   EVAS_COLORSPACE_ARGB8888);
   glsym_evas_gl_common_image_free(img);

   if (!(img = img2)) return NULL;

   if (ns->type == EVAS_NATIVE_SURFACE_WL)
     {
        if (native)
          {
             if ((n = calloc(1, sizeof(Native))))
               {
                  EGLint attribs[3];
                  int format, yinvert = 1;

                  glsym_eglQueryWaylandBufferWL(ob->egl.disp, wl_buf,
                                                EGL_TEXTURE_FORMAT, &format);
                  if ((format != EGL_TEXTURE_RGB) &&
                      (format != EGL_TEXTURE_RGBA))
                    {
                       ERR("eglQueryWaylandBufferWL() %d format is not supported ", format);
                       glsym_evas_gl_common_image_free(img);
                       free(n);
                       return NULL;
                    }

#ifndef EGL_WAYLAND_PLANE_WL
# define EGL_WAYLAND_PLANE_WL 0x31D6
#endif
#ifndef EGL_WAYLAND_BUFFER_WL
# define EGL_WAYLAND_BUFFER_WL 0x31D5
#endif
                  attribs[0] = EGL_WAYLAND_PLANE_WL;
                  attribs[1] = 0; //if plane is 1 then 0, if plane is 2 then 1
                  attribs[2] = EGL_NONE;

                  memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                  glsym_eglQueryWaylandBufferWL(ob->egl.disp, wl_buf,
                                                EGL_WAYLAND_Y_INVERTED_WL,
                                                &yinvert);
                  eina_hash_add(ob->gl_context->shared->native_wl_hash,
                                &wlid, img);

                  n->wl_buf = wl_buf;
                  if (glsym_eglCreateImage)
                    n->egl_surface = glsym_eglCreateImage(ob->egl.disp,
                                                          NULL,
                                                          EGL_WAYLAND_BUFFER_WL,
                                                          wl_buf, attribs);
                  else
                    {
                       ERR("Try eglCreateImage on EGL with no support");
                       eina_hash_del(ob->gl_context->shared->native_wl_hash,
                                     &wlid, img);
                       glsym_evas_gl_common_image_free(img);
                       free(n);
                       return NULL;
                    }

                  if (!n->egl_surface)
                    {
                       ERR("eglCreatePixmapSurface() for %p failed", wl_buf);
                       eina_hash_del(ob->gl_context->shared->native_wl_hash,
                                     &wlid, img);
                       glsym_evas_gl_common_image_free(img);
                       free(n);
                       return NULL;
                    }

                  //XXX: workaround for mesa-10.2.8
                  // mesa's eglQueryWaylandBufferWL() with EGL_WAYLAND_Y_INVERTED_WL works incorrect.
                  //img->native.yinvert = yinvert;
                  img->native.yinvert = 1;
                  img->native.loose = 0;
                  img->native.data = n;
                  img->native.func.bind = _native_cb_bind;
                  img->native.func.unbind = _native_cb_unbind;
                  img->native.func.free = _native_cb_free;
                  img->native.target = GL_TEXTURE_2D;
                  img->native.mipmap = 0;

                  glsym_evas_gl_common_image_native_enable(img);
               }
          }
     }
   else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        if (native)
          {
             if ((n = calloc(1, sizeof(Native))))
               {
                  memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                  eina_hash_add(ob->gl_context->shared->native_tex_hash,
                                &texid, img);

                  n->egl_surface = 0;

                  img->native.yinvert = 0;
                  img->native.loose = 0;
                  img->native.data = n;
                  img->native.func.bind = _native_cb_bind;
                  img->native.func.unbind = _native_cb_unbind;
                  img->native.func.free = _native_cb_free;
                  img->native.target = GL_TEXTURE_2D;
                  img->native.mipmap = 0;

                  glsym_evas_gl_common_image_native_enable(img);
               }
          }
     }

   /* TODO: NATIVE_SURFACE_TBM and NATIVE_SURFACE_EVASGL */

   return img;
}

/* module api functions */
static int
module_open(Evas_Module *em)
{
   /* check for valid evas module */
   if (!em) return 0;

   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic")) return 0;

   /* try to create eina logging domain */
   if (_evas_engine_eglfs_log_dom < 0)
     {
        _evas_engine_eglfs_log_dom =
          eina_log_domain_register("evas-eglfs", EVAS_DEFAULT_LOG_COLOR);
     }

   /* if we could not create a logging domain, error out */
   if (_evas_engine_eglfs_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store it for later use */
   func = pfunc;

   /* now to override methods */
   EVAS_API_OVERRIDE(output_info, &func, eng_);
   EVAS_API_OVERRIDE(output_info_free, &func, eng_);
   EVAS_API_OVERRIDE(output_setup, &func, eng_);
   EVAS_API_OVERRIDE(output_update, &func, eng_);
   EVAS_API_OVERRIDE(canvas_alpha_get, &func, eng_);
   EVAS_API_OVERRIDE(output_free, &func, eng_);
   EVAS_API_OVERRIDE(output_dump, &func, eng_);
   EVAS_API_OVERRIDE(image_native_set, &func, eng_);

   setenv("EGL_PLATFORM", "fbdev", 1);

   gl_symbols();

   /* now advertise out own api */
   em->functions = (void *)(&func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister the eina log domain for this engine */
   if (_evas_engine_eglfs_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_eglfs_log_dom);
        _evas_engine_eglfs_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "eglfs", "none", { module_open, module_close }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, eglfs);

#ifndef EVAS_STATIC_BUILD_EGLFS
EVAS_EINA_MODULE_DEFINE(engine, eglfs);
#endif
