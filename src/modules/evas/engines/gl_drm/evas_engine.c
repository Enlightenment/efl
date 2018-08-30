#include "config.h"
#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"
#include "../software_generic/evas_native_common.h"

#include <wayland-client.h>

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_drm should not get compiled if dlsym is not found on the system!
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

struct scanout_handle
{
   Evas_Native_Scanout_Handler handler;
   void *data;
};

/* external variables */
int _evas_engine_gl_drm_log_dom = -1;
int _extn_have_buffer_age = 1;
int _extn_have_context_priority = 0;

/* local variables */
static Eina_Bool initted = EINA_FALSE;
static Eina_Bool dmabuf_present = EINA_FALSE;
static int gl_wins = 0;
static struct gbm_device *gbm_dev = NULL;
static int gbm_dev_refs = 0;

/* local function prototype types */
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
Evas_Gl_Extension_String_Check _ckext = NULL;

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
void *(*glsym_eglGetProcAddress)(const char *a) = NULL;
EGLImageKHR  (*glsym_evas_gl_common_eglCreateImage) (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const EGLAttrib *e) = NULL;
int          (*glsym_evas_gl_common_eglDestroyImage) (EGLDisplay a, void *b) = NULL;
void (*glsym_glEGLImageTargetTexture2DOES)(int a, void *b) = NULL;
unsigned int (*glsym_eglSwapBuffersWithDamage)(EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;
unsigned int (*glsym_eglQueryWaylandBufferWL)(EGLDisplay a, void *b, EGLint c, EGLint *d) = NULL;
unsigned int (*glsym_eglSetDamageRegionKHR)(EGLDisplay a, EGLSurface b, EGLint *c, EGLint d) = NULL;

/* local function prototypes */
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

Eina_Bool
eng_gbm_init(Evas_Engine_Info_GL_Drm *info)
{
   int fd;
   if (!info) return EINA_FALSE;

   if (gbm_dev)
     {
        info->info.gbm = gbm_dev;
        gbm_dev_refs++;
        return EINA_TRUE;
     }

   fd = ecore_drm2_device_fd_get(info->info.dev);
   if (!(info->info.gbm = gbm_create_device(fd)))
     {
        ERR("Coult not create gbm device");
        return EINA_FALSE;
     }

   gbm_dev = info->info.gbm;
   gbm_dev_refs = 1;
   return EINA_TRUE;
}

Eina_Bool
eng_gbm_shutdown(Evas_Engine_Info_GL_Drm *info)
{
   if (!info) return EINA_TRUE;

   if (info->info.gbm)
     {
        gbm_dev_refs--;
        info->info.gbm = NULL;
        if (!gbm_dev_refs)
          {
             gbm_device_destroy(gbm_dev);
             gbm_dev = NULL;
          }
     }

   return EINA_TRUE;
}

static void
symbols(void)
{
   Evas_Gl_Extension_String_Check glsym_evas_gl_extension_string_check = NULL;
   static Eina_Bool done = EINA_FALSE;

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

   LINK2GENERIC(eglGetProcAddress);
   LINK2GENERIC(evas_gl_common_eglCreateImage);
   LINK2GENERIC(evas_gl_common_eglDestroyImage);
   LINK2GENERIC(evas_gl_extension_string_check);

   _ckext = glsym_evas_gl_extension_string_check;

   done = EINA_TRUE;
}

void
eng_egl_symbols(EGLDisplay edsp)
{
   static Eina_Bool done = EINA_FALSE;
   const char *exts;

   if (done) return;

#define FINDSYM(dst, sym, typ) \
   if (!dst) dst = (typ)glsym_eglGetProcAddress(sym);

   exts = eglQueryString(edsp, EGL_EXTENSIONS);

   FINDSYM(glsym_glEGLImageTargetTexture2DOES,
           "glEGLImageTargetTexture2DOES", glsym_func_void);

   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageEXT",
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageINTEL",
           glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamage",
           glsym_func_uint);

   FINDSYM(glsym_eglSetDamageRegionKHR, "eglSetDamageRegionKHR",
           glsym_func_uint);

   FINDSYM(glsym_eglQueryWaylandBufferWL, "eglQueryWaylandBufferWL",
           glsym_func_uint);

   if (evas_gl_extension_string_check(exts, "EGL_IMG_context_priority"))
     _extn_have_context_priority = 1;

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
             glsym_eglSetDamageRegionKHR = NULL;
          }
        if (!_ckext(str, "EGL_EXT_buffer_age"))
          _extn_have_buffer_age = 0;

        if (!_ckext(str, "EGL_KHR_partial_update"))
          glsym_eglSetDamageRegionKHR = NULL;

        if (!_ckext(str, "EGL_EXT_swap_buffers_with_damage"))
          glsym_eglSwapBuffersWithDamage = NULL;
        if (_ckext(str, "EGL_EXT_image_dma_buf_import"))
          dmabuf_present = EINA_TRUE;
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
     return (void *)eng_get_ob(re)->egl.surface;
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

static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re;
   struct gbm_surface *surface;
   Evas_Engine_Info_GL_Drm *info;
   unsigned int format = GBM_FORMAT_XRGB8888;
   unsigned int flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;

   re = (Render_Engine *)data;
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

   info = eng_get_ob(re)->info;
   if (!info)
     {
        ERR("Invalid Evas Engine GL_DRM Info!");
        return NULL;
     }

   surface =
     gbm_surface_create(info->info.gbm,
                        eng_get_ob(re)->w, eng_get_ob(re)->h, format, flags);
   if (!surface)
     {
        ERR("Could not create gl drm window");
        return NULL;
     }

   return (void *)surface;
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
        ERR("Inavlid native surface.");
        return 0;
     }

   gbm_surface_destroy((struct gbm_surface *)native_window);

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
                                   eng_get_ob(re)->egl.context, // Evas' GL Context
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
        if (!eglMakeCurrent(ob->egl.disp, ob->egl.surface,
                            ob->egl.surface, ob->egl.context))
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

static Ecore_Drm2_Fb *
drm_import_simple_dmabuf(Ecore_Drm2_Device *dev, struct dmabuf_attributes *attributes)
{
   unsigned int stride[4] = { 0 };
   int dmabuf_fd[4] = { 0 };
   int i;

   for (i = 0; i < attributes->n_planes; i++)
     {
        stride[i] = attributes->stride[i];
        dmabuf_fd[i] = attributes->fd[i];
     }

   return ecore_drm2_fb_dmabuf_import(dev, attributes->width,
                                      attributes->height, 32, 32,
                                      attributes->format, stride,
                                      dmabuf_fd, attributes->n_planes);
}

/* Code from weston's gl-renderer... */
static EGLImageKHR
gl_import_simple_dmabuf(EGLDisplay display, struct dmabuf_attributes *attributes)
{
   EGLAttrib attribs[30];
   int atti = 0;

   if (!dmabuf_present) return NULL;
   if (!glsym_evas_gl_common_eglDestroyImage) return NULL;

   /* This requires the Mesa commit in
    * Mesa 10.3 (08264e5dad4df448e7718e782ad9077902089a07) or
    * Mesa 10.2.7 (55d28925e6109a4afd61f109e845a8a51bd17652).
    * Otherwise Mesa closes the fd behind our back and re-importing
    * will fail.
    * https://bugs.freedesktop.org/show_bug.cgi?id=76188
    */

   attribs[atti++] = EGL_WIDTH;
   attribs[atti++] = attributes->width;
   attribs[atti++] = EGL_HEIGHT;
   attribs[atti++] = attributes->height;
   attribs[atti++] = EGL_LINUX_DRM_FOURCC_EXT;
   attribs[atti++] = attributes->format;
   /* XXX: Add modifier here when supported */

   if (attributes->n_planes > 0)
     {
        attribs[atti++] = EGL_DMA_BUF_PLANE0_FD_EXT;
        attribs[atti++] = attributes->fd[0];
        attribs[atti++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
        attribs[atti++] = attributes->offset[0];
        attribs[atti++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
        attribs[atti++] = attributes->stride[0];
     }

   if (attributes->n_planes > 1)
     {
        attribs[atti++] = EGL_DMA_BUF_PLANE1_FD_EXT;
        attribs[atti++] = attributes->fd[1];
        attribs[atti++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
        attribs[atti++] = attributes->offset[1];
        attribs[atti++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
        attribs[atti++] = attributes->stride[1];
     }

   if (attributes->n_planes > 2)
     {
        attribs[atti++] = EGL_DMA_BUF_PLANE2_FD_EXT;
        attribs[atti++] = attributes->fd[2];
        attribs[atti++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
        attribs[atti++] = attributes->offset[2];
        attribs[atti++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
        attribs[atti++] = attributes->stride[2];
     }

   attribs[atti++] = EGL_NONE;

   return glsym_evas_gl_common_eglCreateImage(display, EGL_NO_CONTEXT,
                                              EGL_LINUX_DMA_BUF_EXT,
                                              NULL, attribs);
}

static void
_native_cb_bind(void *image)
{
   Evas_GL_Image *img;
   Native *n;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_WL_DMABUF)
     {
        void *v;

        /* Must re-import every time for coherency. */
        if (n->ns_data.wl_surface_dmabuf.image)
          glsym_evas_gl_common_eglDestroyImage(img->native.disp, n->ns_data.wl_surface_dmabuf.image);
        v = gl_import_simple_dmabuf(img->native.disp, &n->ns_data.wl_surface_dmabuf.attr);
        if (!v) return;
        glsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, v);
        n->ns_data.wl_surface_dmabuf.image = v;
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
        if (n->ns_data.wl_surface.surface)
          {
             if (glsym_glEGLImageTargetTexture2DOES)
               {
                  glsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, n->ns_data.wl_surface.surface);
                  GLERRV("glsym_glEGLImageTargetTexture2DOES");
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

   if (n->ns.type == EVAS_NATIVE_SURFACE_WL_DMABUF)
     {
        if (n->ns_data.wl_surface_dmabuf.image)
          glsym_evas_gl_common_eglDestroyImage(img->native.disp, n->ns_data.wl_surface_dmabuf.image);
        n->ns_data.wl_surface_dmabuf.image = NULL;
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
        //glBindTexture(GL_TEXTURE_2D, 0); //really need?
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     glBindTexture(GL_TEXTURE_2D, 0);

   /* TODO: NATIVE_SURFACE_TBM and NATIVE_SURFACE_EVASGL */
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
   Evas_GL_Image *img;
   Native *n;
   Ecore_Drm2_Fb *fb = NULL;
   Ecore_Drm2_Plane *plane = NULL;
   struct scanout_handle *g;

   EINA_SAFETY_ON_NULL_RETURN_VAL(image, NULL);

   re = (Render_Engine *)data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(re, NULL);

   ob = eng_get_ob(re);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ob, NULL);

   img = image;
   n = img->native.data;

   /* Perhaps implementable on other surface types, but we're
    * sticking to this one for now */
   if (n->ns.type != EVAS_NATIVE_SURFACE_WL_DMABUF) return NULL;

   fb = drm_import_simple_dmabuf(re->dev, &n->ns_data.wl_surface_dmabuf.attr);

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

 static void
_native_cb_free(void *image)
{
   Evas_GL_Image *img;
   Native *n;
   uint32_t texid;
   void *wlid;

   if (!(img = image)) return;
   if (!(n = img->native.data)) return;
   if (!(img->native.shared)) return;

   if (n->ns.type == EVAS_NATIVE_SURFACE_WL_DMABUF)
     {
        wlid = n->ns_data.wl_surface_dmabuf.resource;
        eina_hash_del(img->native.shared->native_wl_hash, &wlid, img);
        if (n->ns_data.wl_surface.surface)
          {
             if (glsym_evas_gl_common_eglDestroyImage && n->ns_data.wl_surface_dmabuf.image)
               {
                  glsym_evas_gl_common_eglDestroyImage(img->native.disp, n->ns_data.wl_surface_dmabuf.image);
                  GLERRV("eglDestroyImage() failed.");
               }
          }
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
        wlid = (void*)n->ns_data.wl_surface.wl_buf;
        eina_hash_del(img->native.shared->native_wl_hash, &wlid, img);
        if (n->ns_data.wl_surface.surface)
          {
             if (glsym_evas_gl_common_eglDestroyImage)
               {
                  glsym_evas_gl_common_eglDestroyImage(img->native.disp, n->ns_data.wl_surface.surface);
                  GLERRV("eglDestroyImage() failed.");
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
static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_GL_Drm *einfo = info;

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_GL_Drm *info = in;
   Render_Engine *re = NULL;
   Outbuf *ob;
   Render_Output_Swap_Mode swap_mode;

   swap_mode = evas_render_engine_gl_swap_mode_get(info->info.swap_mode);

   if (!initted)
     {
        glsym_evas_gl_preload_init();
     }

   if (!(re = calloc(1, sizeof(Render_Engine)))) return 0;

   if (!eng_gbm_init(info))
     {
        free(re);
        return NULL;
     }

   re->dev = info->info.dev;

   /* try to create new outbuf */
   ob = evas_outbuf_new(info, w, h, swap_mode);
   if (!ob)
     {
        eng_gbm_shutdown(info);
        free(re);
        return NULL;
     }

   if (!evas_render_engine_gl_generic_init(engine, &re->generic, ob,
                                           evas_outbuf_buffer_state_get,
                                           evas_outbuf_rot_get,
                                           evas_outbuf_reconfigure,
                                           evas_outbuf_update_region_first_rect,
                                           evas_outbuf_damage_region_set,
                                           evas_outbuf_update_region_new,
                                           evas_outbuf_update_region_push,
                                           NULL,
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
     {
        /* free outbuf */
        evas_outbuf_free(ob);
        eng_gbm_shutdown(info);
        free(re);
        return NULL;
     }

   gl_wins++;

   evas_render_engine_software_generic_merge_mode_set(&re->generic.software);

   if (!initted)
     {
        gl_extn_veto(re);
        initted = EINA_TRUE;
     }

   evas_outbuf_use(eng_get_ob(re));

   return re;
}

static int
eng_output_update(void *engine EINA_UNUSED, void *data, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_GL_Drm *info = (Evas_Engine_Info_GL_Drm *)in;
   Render_Engine *re = data;

   if (eng_get_ob(re) && _re_wincheck(eng_get_ob(re)))
     {
        if ((info->info.depth != eng_get_ob(re)->depth) ||
            (info->info.destination_alpha != eng_get_ob(re)->destination_alpha))
          {
             Outbuf *ob, *ob_old;
             Render_Output_Swap_Mode swap_mode = MODE_AUTO;

             ob_old = re->generic.software.ob;
             re->generic.software.ob = NULL;
             gl_wins--;

             if (ob_old) evas_outbuf_free(ob_old);

             swap_mode = evas_render_engine_gl_swap_mode_get(info->info.swap_mode);
             ob = evas_outbuf_new(info, w, h, swap_mode);
             if (!ob)
               {
                  free(re);
                  return 0;
               }

             evas_outbuf_use(ob);

             evas_render_engine_software_generic_update(&re->generic.software,
                                                        ob, w, h);

             gl_wins++;
          }
        else if ((eng_get_ob(re)->w != (int)w) ||
                 (eng_get_ob(re)->h != (int)h) ||
                 (info->info.rotation != eng_get_ob(re)->rotation))
          {
             evas_outbuf_reconfigure(eng_get_ob(re), w, h,
                                     info->info.rotation, info->info.depth);
             evas_render_engine_software_generic_update(&re->generic.software,
                                                        re->generic.software.ob,
                                                        w, h);
          }
     }

   evas_outbuf_use(eng_get_ob(re));

   return 1;
}

static void
eng_output_free(void *engine, void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (re)
     {
        Evas_Engine_Info_GL_Drm *info;

        glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));

        if (gl_wins == 1) glsym_evgl_engine_shutdown(re);

        info = eng_get_ob(re)->info;
        /* NB: evas_render_engine_software_generic_clean() frees ob */
        evas_render_engine_software_generic_clean(engine, &re->generic.software);

        eng_gbm_shutdown(info);
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
eng_output_dump(void *engine, void *data)
{
   Render_Engine *re;
   Render_Engine_GL_Generic *e = engine;

   re = (Render_Engine *)data;
   if (!re) return;
   generic_cache_dump(e->software.surface_cache);
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   glsym_evas_gl_common_image_all_unload(eng_get_ob(re)->gl_context);
   _re_winfree(re);
}

static int
eng_image_native_init(void *engine EINA_UNUSED, Evas_Native_Surface_Type type)
{
   switch (type)
     {
      case EVAS_NATIVE_SURFACE_OPENGL:
      case EVAS_NATIVE_SURFACE_WL:
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
      case EVAS_NATIVE_SURFACE_OPENGL:
      case EVAS_NATIVE_SURFACE_WL:
        return;
      default:
        ERR("Native surface type %d not supported!", type);
        return;
     }
}

static void *
eng_image_native_set(void *engine, void *image, void *native)
{
   Outbuf *ob;
   Native *n;
   Evas_Native_Surface *ns;
   Evas_GL_Image *img, *img2;
   unsigned int tex = 0, fbo = 0;
   uint32_t texid;
   void *wlid, *wl_buf = NULL;

   ob = gl_generic_any_output_get(engine);
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
        else if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_WL_DMABUF))
          {
             if (!ns->data.wl_dmabuf.resource)
               {
                  struct dmabuf_attributes *attr;
                  void *v = NULL;

                  attr = ns->data.wl_dmabuf.attr;
                  if (attr->version == EVAS_DMABUF_ATTRIBUTE_VERSION)
                    v = gl_import_simple_dmabuf(ob->egl.disp, attr);
                  if (!v) return NULL;

                  glsym_evas_gl_common_eglDestroyImage(ob->egl.disp, v);
                  img =
                    glsym_evas_gl_common_image_new_from_data(ob->gl_context,
                                                             attr->width,
                                                             attr->height,
                                                             NULL, 1,
                                                             EVAS_COLORSPACE_ARGB8888);
                  return img;
               }
          }
        else
          return NULL;
     }

   if (ns)
     {
        if (ns->type == EVAS_NATIVE_SURFACE_WL_DMABUF)
          {
             wl_buf = ns->data.wl_dmabuf.resource;
          }
        else if (ns->type == EVAS_NATIVE_SURFACE_WL)
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

   evas_outbuf_use(ob);

   if (!ns)
     {
        glsym_evas_gl_common_image_free(img);
        return NULL;
     }

   if (ns->type == EVAS_NATIVE_SURFACE_WL_DMABUF)
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
   else if (ns->type == EVAS_NATIVE_SURFACE_WL)
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

   if (ns->type == EVAS_NATIVE_SURFACE_WL_DMABUF)
     {
        if (native)
          {
             struct dmabuf_attributes *a;

             a = ns->data.wl_dmabuf.attr;
             if (a->version != EVAS_DMABUF_ATTRIBUTE_VERSION)
               {
                  glsym_evas_gl_common_image_free(img);
                  return NULL;
               }
             if ((n = calloc(1, sizeof(Native))))
               {
                  struct dmabuf_attributes *a2;

                  a2 = ns->data.wl_dmabuf.attr;
                  memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                  memcpy(&n->ns_data.wl_surface_dmabuf.attr, a2, sizeof(*a2));
                  eina_hash_add(ob->gl_context->shared->native_wl_hash,
                                &wlid, img);

                  n->ns_data.wl_surface_dmabuf.resource = wl_buf;
                  img->native.yinvert = 1;
                  img->native.loose = 0;
                  img->native.disp = ob->egl.disp;
                  img->native.shared = ob->gl_context->shared;
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
   else if (ns->type == EVAS_NATIVE_SURFACE_WL)
     {
        if (native)
          {
             if ((n = calloc(1, sizeof(Native))))
               {
                  EGLAttrib attribs[3];
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
                  if (glsym_eglQueryWaylandBufferWL(ob->egl.disp, wl_buf,
                                                    EGL_WAYLAND_Y_INVERTED_WL,
                                                    &yinvert) == EGL_FALSE)
                    yinvert = 1;
                  eina_hash_add(ob->gl_context->shared->native_wl_hash,
                                &wlid, img);

                  n->ns_data.wl_surface.wl_buf = wl_buf;
                  if (glsym_evas_gl_common_eglDestroyImage)
                    n->ns_data.wl_surface.surface = glsym_evas_gl_common_eglCreateImage(ob->egl.disp,
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

                  if (!n->ns_data.wl_surface.surface)
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
                  img->native.disp = ob->egl.disp;
                  img->native.shared = ob->gl_context->shared;
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

                  n->ns_data.opengl.surface = 0;

                  img->native.yinvert = 0;
                  img->native.loose = 0;
                  img->native.disp = ob->egl.disp;
                  img->native.shared = ob->gl_context->shared;
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
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic", sizeof (Evas_Engine_Info_GL_Drm))) return 0;

   /* try to create eina logging domain */
   if (_evas_engine_gl_drm_log_dom < 0)
     {
        _evas_engine_gl_drm_log_dom =
          eina_log_domain_register("evas-gl-drm", EVAS_DEFAULT_LOG_COLOR);
     }

   /* if we could not create a logging domain, error out */
   if (_evas_engine_gl_drm_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   ecore_init();

   /* store it for later use */
   func = pfunc;

   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info_setup);
   ORD(output_setup);
   ORD(output_update);
   ORD(canvas_alpha_get);
   ORD(output_free);
   ORD(output_dump);
   ORD(image_native_set);
   ORD(image_native_init);
   ORD(image_native_shutdown);
   ORD(image_plane_assign);
   ORD(image_plane_release);

   /* Mesa's EGL driver loads wayland egl by default. (called by eglGetProcaddr() )
    * implicit env set (EGL_PLATFORM=drm) prevent that. */
   setenv("EGL_PLATFORM", "drm", 1);

   symbols();

   /* now advertise out own api */
   em->functions = (void *)(&func);

   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   /* unregister the eina log domain for this engine */
   if (_evas_engine_gl_drm_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_gl_drm_log_dom);
        _evas_engine_gl_drm_log_dom = -1;
     }

   ecore_shutdown();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "gl_drm", "none", { module_open, module_close }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_drm);

#ifndef EVAS_STATIC_BUILD_DRM
EVAS_EINA_MODULE_DEFINE(engine, gl_drm);
#endif
