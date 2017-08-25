#include "evas_common_private.h" /* Also includes international specific stuff */
#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"
#include "../software_generic/evas_native_common.h"

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_x11 should not get compiled if dlsym is not found on the system!
#endif

#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#define EVAS_GL_UPDATE_TILE_SIZE 16

typedef struct _Render_Engine               Render_Engine;

struct _Render_Engine
{
   Render_Engine_GL_Generic generic;
};

const char *debug_dir;
int swap_buffer_debug_mode = -1;
int swap_buffer_debug = 0;
int partial_render_debug = -1;
int extn_have_buffer_age = 1;

static int initted = 0;
static int gl_wins = 0;
#ifdef GL_GLES
static int extn_have_y_inverted = 1;
#endif

typedef void            (*_eng_fn) (void);
typedef _eng_fn         (*glsym_func_eng_fn) ();
typedef void            (*glsym_func_void) ();
typedef void           *(*glsym_func_void_ptr) ();
typedef int             (*glsym_func_int) ();
typedef unsigned int    (*glsym_func_uint) ();
typedef const char     *(*glsym_func_const_char_ptr) ();

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
EVGL_Native_Surface_Call glsym_evgl_native_surface_buffer_get = NULL;
EVGL_Native_Surface_Yinvert_Call glsym_evgl_native_surface_yinvert_get = NULL;
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

glsym_func_void     glsym_evas_gl_common_shaders_flush = NULL;
glsym_func_void     glsym_evas_gl_common_error_set = NULL;
glsym_func_int      glsym_evas_gl_common_error_get = NULL;
glsym_func_void_ptr glsym_evas_gl_common_current_context_get = NULL;

#ifdef GL_GLES

_eng_fn      (*glsym_eglGetProcAddress)            (const char *a) = NULL;
EGLImageKHR  (*glsym_evas_gl_common_eglCreateImage)(EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const EGLAttrib *e) = NULL;
int          (*glsym_evas_gl_common_eglDestroyImage) (EGLDisplay a, void *b) = NULL;
void         (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
unsigned int (*glsym_eglSwapBuffersWithDamage) (EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;
unsigned int (*glsym_eglSetDamageRegionKHR)  (EGLDisplay a, EGLSurface b, EGLint *c, EGLint d) = NULL;
unsigned int (*glsym_eglQueryWaylandBufferWL)(EGLDisplay a, /*struct wl_resource */void *b, EGLint c, EGLint *d) = NULL;

#else

typedef XID     (*glsym_func_xid) ();

_eng_fn  (*glsym_glXGetProcAddress)  (const char *a) = NULL;
void     (*glsym_glXBindTexImage)    (Display *a, GLXDrawable b, int c, int *d) = NULL;
void     (*glsym_glXReleaseTexImage) (Display *a, GLXDrawable b, int c) = NULL;
int      (*glsym_glXGetVideoSync)    (unsigned int *a) = NULL;
int      (*glsym_glXWaitVideoSync)   (int a, int b, unsigned int *c) = NULL;
XID      (*glsym_glXCreatePixmap)    (Display *a, void *b, Pixmap c, const int *d) = NULL;
void     (*glsym_glXDestroyPixmap)   (Display *a, XID b) = NULL;
int      (*glsym_glXQueryDrawable)   (Display *a, XID b, int c, unsigned int *d) = NULL;
int      (*glsym_glXSwapIntervalSGI) (int a) = NULL;
void     (*glsym_glXSwapIntervalEXT) (Display *s, GLXDrawable b, int c) = NULL;
void     (*glsym_glXReleaseBuffersMESA)   (Display *a, XID b) = NULL;

#endif

static inline Outbuf *
eng_get_ob(Render_Engine *re)
{
   return re->generic.software.ob;
}

//----------------------------------------------------------//
// NEW_EVAS_GL Engine Functions
static void *
evgl_eng_display_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->egl_disp;
#else
   if (eng_get_ob(re)->info)
      return (void*)eng_get_ob(re)->info->info.display;
#endif
   else
      return NULL;
}

static void *
evgl_eng_evas_surface_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        return NULL;
     }

#ifdef GL_GLES
   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->egl_surface;
#else
   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->win;
#endif
   else
      return NULL;
}

#ifdef GL_GLES
static EGLDisplay main_dpy  = EGL_NO_DISPLAY;
static EGLSurface main_draw = EGL_NO_SURFACE;
static EGLSurface main_read = EGL_NO_SURFACE;
static EGLContext main_ctx  = EGL_NO_CONTEXT;

EGLContext
evas_eglGetCurrentContext(void)
{
   if (eina_main_loop_is())
     return main_ctx;
   else
     return eglGetCurrentContext();
}

EGLSurface
evas_eglGetCurrentSurface(EGLint readdraw)
{
   if (eina_main_loop_is())
     return (readdraw == EGL_READ) ? main_read : main_draw;
   else
     return eglGetCurrentSurface(readdraw);
}

EGLDisplay
evas_eglGetCurrentDisplay(void)
{
   if (eina_main_loop_is())
     return main_dpy;
   else
     return eglGetCurrentDisplay();
}

EGLBoolean
evas_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
   if (eina_main_loop_is())
     {
        EGLBoolean ret;

        if ((dpy == main_dpy) && (draw == main_draw) &&
            (read == main_read) && (ctx == main_ctx))
          return 1;

        ret = eglMakeCurrent(dpy, draw, read, ctx);
        if (ret)
          {
             main_dpy  = dpy;
             main_draw = draw;
             main_read = read;
             main_ctx  = ctx;
          }
        return ret;
     }
   else
     return eglMakeCurrent(dpy, draw, read, ctx);
}
#endif

static int
evgl_eng_make_current(void *data, void *surface, void *context, int flush)
{
   Render_Engine *re = (Render_Engine *)data;
   int ret = 0;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

#ifdef GL_GLES
   EGLContext ctx = (EGLContext)context;
   EGLSurface sfc = (EGLSurface)surface;
   EGLDisplay dpy = eng_get_ob(re)->egl_disp; //eglGetCurrentDisplay();

   if ((!context) && (!surface))
     {
        ret = evas_eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (!ret)
          {
             int err = eglGetError();
             glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
             ERR("evas_eglMakeCurrent() failed! Error Code=%#x", err);
             return 0;
          }
        return 1;
     }

   // FIXME: Check (eglGetCurrentDisplay() != dpy) ?
   if ((evas_eglGetCurrentContext() != ctx) ||
       (evas_eglGetCurrentSurface(EGL_READ) != sfc) ||
       (evas_eglGetCurrentSurface(EGL_DRAW) != sfc) )
     {

        //!!!! Does it need to be flushed with it's set to NULL above??
        // Flush remainder of what's in Evas' pipeline
        if (flush) eng_window_use(NULL);

        // Do a make current
        ret = evas_eglMakeCurrent(dpy, sfc, sfc, ctx);

        if (!ret)
          {
             int err = eglGetError();
             glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
             ERR("evas_eglMakeCurrent() failed! Error Code=%#x", err);
             return 0;
          }
     }

   return 1;
#else
   GLXContext ctx = (GLXContext)context;
   Window     sfc = (Window)surface;

   if ((!context) && (!surface))
     {
        ret = __glXMakeContextCurrent(eng_get_ob(re)->info->info.display, 0, NULL);
        if (!ret)
          {
             ERR("glXMakeContextCurrent() failed!");
             glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_DISPLAY);
             return 0;
          }
        return 1;
     }


   if ((glXGetCurrentContext() != ctx))
     {
        //!!!! Does it need to be flushed with it's set to NULL above??
        // Flush remainder of what's in Evas' pipeline
        if (flush) eng_window_use(NULL);

        // Do a make current
        if ((sfc == eng_get_ob(re)->win) ||
            (sfc == eng_get_ob(re)->glxwin))
          ret = __glXMakeContextCurrent(eng_get_ob(re)->info->info.display,
                                        eng_get_ob(re)->glxwin, ctx);
        else
          ret = __glXMakeContextCurrent(eng_get_ob(re)->info->info.display,
                                        sfc, ctx);
        if (!ret)
          {
             ERR("glXMakeContextCurrent() failed. Ret: %d! Context: %p Surface: %p",
                 ret, (void *)ctx, (void *)sfc);
             glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_DISPLAY);
             return 0;
          }
     }
   return 1;
#endif
}



static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

   XSetWindowAttributes attr;
   Window win;

   attr.backing_store = NotUseful;
   attr.override_redirect = True;
   attr.border_pixel = 0;
   attr.background_pixmap = None;
   attr.bit_gravity = NorthWestGravity;
   attr.win_gravity = NorthWestGravity;
   attr.save_under = False;
   attr.do_not_propagate_mask = NoEventMask;
   attr.event_mask = 0;

   win = XCreateWindow(eng_get_ob(re)->info->info.display,
                       eng_get_ob(re)->win,
                       -20, -20, 2, 2, 0,
                       CopyFromParent, InputOutput, CopyFromParent,
                       CWBackingStore | CWOverrideRedirect |
                       CWBorderPixel | CWBackPixmap |
                       CWSaveUnder | CWDontPropagate |
                       CWEventMask | CWBitGravity |
                       CWWinGravity, &attr);
   if (!win)
     {
        ERR("Creating native X window failed.");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_DISPLAY);
        return NULL;
     }

   return (void*)win;
}

static int
evgl_eng_native_window_destroy(void *data, void *native_window)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if (!native_window)
     {
        ERR("Inavlid native surface.");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_NATIVE_WINDOW);
        return 0;
     }

   XDestroyWindow(eng_get_ob(re)->info->info.display, (Window)native_window);

   native_window = NULL;

   return 1;
}


// Theoretically, we wouldn't need this functoin if the surfaceless context
// is supported. But, until then...
static void *
evgl_eng_window_surface_create(void *data, void *native_window EINA_UNUSED)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

#ifdef GL_GLES
   EGLSurface surface = EGL_NO_SURFACE;

   // Create resource surface for EGL
   surface = eglCreateWindowSurface(eng_get_ob(re)->egl_disp,
                                    eng_get_ob(re)->egl_config,
                                    (EGLNativeWindowType)native_window,
                                    NULL);
   if (!surface)
     {
        ERR("Creating window surface failed. Error: %#x.", eglGetError());
        abort();
        return NULL;
     }

   return (void*)surface;
#else
   /*
   // We don't need to create new one for GLX
   Window surface;

   surface = eng_get_ob(re)->win;

   return (void *)surface;
   */
   return (void *)native_window;
#endif
}

static int
evgl_eng_window_surface_destroy(void *data, void *surface)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

#ifdef GL_GLES
   if (!surface)
     {
        ERR("Invalid surface.");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_SURFACE);
        return 0;
     }

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)surface);
#endif

   return 1;
   if (surface) return 0;
}

static void *
evgl_eng_context_create(void *data, void *share_ctx, Evas_GL_Context_Version version)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

   if ((version < EVAS_GL_GLES_1_X) || (version > EVAS_GL_GLES_3_X))
     {
        ERR("Invalid context version number %d", version);
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

#ifdef GL_GLES
   if ((version == EVAS_GL_GLES_3_X) &&
       ((!eng_get_ob(re)->gl_context) || (eng_get_ob(re)->gl_context->gles_version != EVAS_GL_GLES_3_X)))
     {
        ERR("GLES 3 version not supported!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_ATTRIBUTE);
        return NULL;
     }
   EGLContext context = EGL_NO_CONTEXT;
   int context_attrs[3];

   /* Upgrade GLES 2 to GLES 3.
    *
    * FIXME: Maybe we don't want to do this, unless we have no choice.
    * An alternative would be to use eglCreateImage() to share the indirect
    * rendering FBO between two contexts of incompatible version. For now,
    * we always upgrade the real context version to GLES 3 when it's available.
    * But this leads to some issues, namely that the list of extensions is
    * different, and MSAA surfaces also work differently.
    */
   if (eng_get_ob(re)->gles3 && (version >= EVAS_GL_GLES_2_X))
     version = 3;

   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = version;
   context_attrs[2] = EGL_NONE;

   // Share context already assumes that it's sharing with evas' context
   if (share_ctx)
     {
        context = eglCreateContext(eng_get_ob(re)->egl_disp,
                                   eng_get_ob(re)->egl_config,
                                   (EGLContext)share_ctx,
                                   context_attrs);
     }
   else if ((version == EVAS_GL_GLES_1_X) || (version == EVAS_GL_GLES_3_X))
     {
        context = eglCreateContext(eng_get_ob(re)->egl_disp,
                                   eng_get_ob(re)->egl_config,
                                   NULL,
                                   context_attrs);
     }
   else
     {
        context = eglCreateContext(eng_get_ob(re)->egl_disp,
                                   eng_get_ob(re)->egl_config,
                                   eng_get_ob(re)->egl_context, // Evas' GL Context
                                   context_attrs);
     }

   if (!context)
     {
        int err = eglGetError();
        ERR("Engine Context Creations Failed. Error: %#x.", err);
        glsym_evas_gl_common_error_set(data, err - EGL_SUCCESS);
        return NULL;
     }

   return (void*)context;
#else
   GLXContext context = NULL;

   // Share context already assumes that it's sharing with evas' context
   if (share_ctx)
     {
        context = glXCreateContext(eng_get_ob(re)->info->info.display,
                                   eng_get_ob(re)->visualinfo,
                                   (GLXContext)share_ctx,
                                   1);
     }
   else
     {
        context = glXCreateContext(eng_get_ob(re)->info->info.display,
                                   eng_get_ob(re)->visualinfo,
                                   eng_get_ob(re)->context,      // Evas' GL Context
                                   1);
     }

   if (!context)
     {
        ERR("Internal Resource Context Creations Failed.");
        if(!(eng_get_ob(re)->info->info.display)) glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_DISPLAY);
        if(!(eng_get_ob(re)->win)) glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_NATIVE_WINDOW);
        return NULL;
     }

   return (void*)context;
#endif

}

static int
evgl_eng_context_destroy(void *data, void *context)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if ((!re) || (!context))
     {
        ERR("Invalid Render Input Data. Engine: %p, Context: %p", data, context);
        if (!re) glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        if (!context) glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_CONTEXT);
        return 0;
     }

#ifdef GL_GLES
   eglDestroyContext(eng_get_ob(re)->egl_disp, (EGLContext)context);
#else
   glXDestroyContext(eng_get_ob(re)->info->info.display, (GLXContext)context);
#endif

   return 1;
}

static const char *
evgl_eng_string_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

#ifdef GL_GLES
   return eglQueryString(eng_get_ob(re)->egl_disp, EGL_EXTENSIONS);
#else
   return glXQueryExtensionsString(eng_get_ob(re)->info->info.display,
                                   eng_get_ob(re)->info->info.screen);
#endif
}

static void *
evgl_eng_proc_address_get(const char *name)
{
#ifdef GL_GLES
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#else
   if (glsym_glXGetProcAddress) return glsym_glXGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#endif
}

static int
evgl_eng_rotation_angle_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if ((eng_get_ob(re)) && (eng_get_ob(re)->gl_context))
     return eng_get_ob(re)->gl_context->rot;
   else
     {
        ERR("Unable to retrieve rotation angle.");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_CONTEXT);
        return 0;
     }
}

static void *
evgl_eng_pbuffer_surface_create(void *data, EVGL_Surface *sfc,
                                const int *attrib_list)
{
   Render_Engine_GL_Generic *re = data;

   // TODO: Add support for surfaceless pbuffers (EGL_NO_TEXTURE)
   // TODO: Add support for EGL_MIPMAP_TEXTURE??? (GLX doesn't support them)

   if (attrib_list)
     WRN("This PBuffer implementation does not support extra attributes yet");

#ifdef GL_GLES
   Evas_Engine_GL_Context *evasglctx;
   int config_attrs[20];
   int surface_attrs[20];
   EGLSurface egl_sfc;
   EGLConfig egl_cfg;
   int num_config, i = 0;
   EGLDisplay disp;
   EGLContext ctx;

   disp = re->window_egl_display_get(re->software.ob);
   evasglctx = re->window_gl_context_get(re->software.ob);
   ctx = evasglctx->eglctxt;

#if 0
   // Choose framebuffer configuration
   // DISABLED FOR NOW
   if (sfc->pbuffer.color_fmt != EVAS_GL_NO_FBO)
     {
        config_attrs[i++] = EGL_RED_SIZE;
        config_attrs[i++] = 1;
        config_attrs[i++] = EGL_GREEN_SIZE;
        config_attrs[i++] = 1;
        config_attrs[i++] = EGL_BLUE_SIZE;
        config_attrs[i++] = 1;

        if (sfc->pbuffer.color_fmt == EVAS_GL_RGBA_8888)
          {
             config_attrs[i++] = EGL_ALPHA_SIZE;
             config_attrs[i++] = 1;
             //config_attrs[i++] = EGL_BIND_TO_TEXTURE_RGBA;
             //config_attrs[i++] = EGL_TRUE;
          }
        else
          {
             //config_attrs[i++] = EGL_BIND_TO_TEXTURE_RGB;
             //config_attrs[i++] = EGL_TRUE;
          }
     }

   if (sfc->depth_fmt || sfc->depth_stencil_fmt)
     {
        config_attrs[i++] = EGL_DEPTH_SIZE;
        config_attrs[i++] = 1;
     }

   if (sfc->stencil_fmt || sfc->depth_stencil_fmt)
     {
        config_attrs[i++] = EGL_STENCIL_SIZE;
        config_attrs[i++] = 1;
     }

   config_attrs[i++] = EGL_RENDERABLE_TYPE;
   if (eng_get_ob(re)->gles3)
     config_attrs[i++] = EGL_OPENGL_ES3_BIT_KHR;
   else
     config_attrs[i++] = EGL_OPENGL_ES2_BIT;
   config_attrs[i++] = EGL_SURFACE_TYPE;
   config_attrs[i++] = EGL_PBUFFER_BIT;
   config_attrs[i++] = EGL_NONE;
#else
   // It looks like evas_eglMakeCurrent might fail if we use a different config from
   // the actual display surface. This is weird.
   i = 0;
   config_attrs[i++] = EGL_CONFIG_ID;
   config_attrs[i++] = 0;
   config_attrs[i++] = EGL_NONE;
   eglQueryContext(disp, ctx, EGL_CONFIG_ID, &config_attrs[1]);
#endif

   if (!eglChooseConfig(disp, config_attrs, &egl_cfg, 1, &num_config)
       || (num_config < 1))
     {
        int err = eglGetError();
        glsym_evas_gl_common_error_set(data, err - EGL_SUCCESS);
        ERR("eglChooseConfig failed with error %x", err);
        return NULL;
     }

   // Now, choose the config for the PBuffer
   i = 0;
   surface_attrs[i++] = EGL_WIDTH;
   surface_attrs[i++] = sfc->w;
   surface_attrs[i++] = EGL_HEIGHT;
   surface_attrs[i++] = sfc->h;
#if 0
   // Adding these parameters will trigger EGL_BAD_ATTRIBUTE because
   // the config also requires EGL_BIND_TO_TEXTURE_RGB[A]. But some drivers
   // don't support those configs (eg. nvidia)
   surface_attrs[i++] = EGL_TEXTURE_FORMAT;
   if (sfc->pbuffer.color_fmt == EVAS_GL_RGB_888)
     surface_attrs[i++] = EGL_TEXTURE_RGB;
   else
     surface_attrs[i++] = EGL_TEXTURE_RGBA;
   surface_attrs[i++] = EGL_TEXTURE_TARGET;
   surface_attrs[i++] = EGL_TEXTURE_2D;
   surface_attrs[i++] = EGL_MIPMAP_TEXTURE;
   surface_attrs[i++] = EINA_TRUE;
#endif
   surface_attrs[i++] = EGL_NONE;

   egl_sfc = eglCreatePbufferSurface(disp, egl_cfg, surface_attrs);
   if (!egl_sfc)
     {
        int err = eglGetError();
        glsym_evas_gl_common_error_set(data, err - EGL_SUCCESS);
        ERR("eglCreatePbufferSurface failed with error %x", err);
        return NULL;
     }

   return egl_sfc;
#else
   Evas_Engine_GL_Context *evasglctx;
   GLXPbuffer pbuf;
   GLXFBConfig *cfgs;
   int config_attrs[20];
   int surface_attrs[20];
   int ncfg = 0, i;

   evasglctx = re->window_gl_context_get(re->software.ob);

#ifndef GLX_VISUAL_ID
# define GLX_VISUAL_ID 0x800b
#endif

   i = 0;
#if 0
   // DISABLED BECAUSE BadMatch HAPPENS
   if (sfc->pbuffer.color_fmt != EVAS_GL_NO_FBO)
     {
        config_attrs[i++] = GLX_BUFFER_SIZE;
        if (sfc->pbuffer.color_fmt == EVAS_GL_RGBA_8888)
          {
             config_attrs[i++] = 32;
             //config_attrs[i++] = GLX_BIND_TO_TEXTURE_RGBA_EXT;
             //config_attrs[i++] = 1;
          }
        else
          {
             config_attrs[i++] = 24;
             //config_attrs[i++] = GLX_BIND_TO_TEXTURE_RGB_EXT;
             //config_attrs[i++] = 1;
          }
     }
   if (sfc->depth_fmt)
     {
        config_attrs[i++] = GLX_DEPTH_SIZE;
        config_attrs[i++] = 24; // FIXME: This should depend on the requested bits
     }
   if (sfc->stencil_fmt)
     {
        config_attrs[i++] = GLX_STENCIL_SIZE;
        config_attrs[i++] = 8; // FIXME: This should depend on the requested bits
     }
   //config_attrs[i++] = GLX_VISUAL_ID;
   //config_attrs[i++] = XVisualIDFromVisual(vis);
#else
   config_attrs[i++] = GLX_FBCONFIG_ID;
   if (sfc->pbuffer.color_fmt == EVAS_GL_RGB_888)
     config_attrs[i++] = evasglctx->glxcfg_rgb;
   else
     config_attrs[i++] = evasglctx->glxcfg_rgba;
#endif
   config_attrs[i++] = 0;

   cfgs = glXChooseFBConfig(re->software.ob->disp, re->software.ob->screen,
                            config_attrs, &ncfg);
   if (!cfgs || !ncfg)
     {
        ERR("GLX failed to find a valid config for the pbuffer");
        if (cfgs) XFree(cfgs);
        return NULL;
     }

   i = 0;
   surface_attrs[i++] = GLX_LARGEST_PBUFFER;
   surface_attrs[i++] = 0;
   surface_attrs[i++] = GLX_PBUFFER_WIDTH;
   surface_attrs[i++] = sfc->w;
   surface_attrs[i++] = GLX_PBUFFER_HEIGHT;
   surface_attrs[i++] = sfc->h;
   surface_attrs[i++] = 0;
   pbuf = glXCreatePbuffer(re->software.ob->disp, cfgs[0], surface_attrs);
   XFree(cfgs);

   if (!pbuf)
     {
        ERR("GLX failed to create a pbuffer");
        return NULL;
     }

   return (void*)(intptr_t)pbuf;
#endif
}

static int
evgl_eng_pbuffer_surface_destroy(void *data, void *surface)
{
   /* EVGLINIT(re, 0); */
   if (!data)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(NULL, EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if (!surface)
     {
        ERR("Invalid surface.");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_SURFACE);
        return 0;
     }

#ifdef GL_GLES
   Render_Engine *re = data;

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)surface);
#else
   Render_Engine_GL_Generic *re = data;
   GLXPbuffer pbuf = (GLXPbuffer)(intptr_t) surface;

   glXDestroyPbuffer(re->software.ob->disp, pbuf);
#endif

   return 1;
}

// This function should create a surface that can be used for offscreen rendering
// and still be bindable to a texture in Evas main GL context.
// For now, this will create an X pixmap... Ideally it should be able to create
// a bindable pbuffer surface or just an FBO if that is supported and it can
// be shared with Evas.
// FIXME: Avoid passing evgl_engine around like that.
static void *
evgl_eng_indirect_surface_create(EVGL_Engine *evgl EINA_UNUSED, void *data,
                              EVGL_Surface *evgl_sfc,
                              Evas_GL_Config *cfg, int w, int h)
{
   Render_Engine *re = data;
#ifdef GL_GLES
   Eina_Bool alpha = EINA_FALSE;
#endif
   int colordepth;
   Pixmap px;

   if (!re || !evgl_sfc || !cfg)
     {
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   if ((w < 1) || (h < 1))
     {
        ERR("Inconsistent parameters, not creating any surface!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   /* Choose appropriate pixmap depth */
   if (cfg->color_format == EVAS_GL_RGBA_8888)
     {
#ifdef GL_GLES
        alpha = EINA_TRUE;
#endif
        colordepth = 32;
     }
   else if (cfg->color_format == EVAS_GL_RGB_888)
     colordepth = 24;
   else // this could also be XDefaultDepth but this case shouldn't happen
     colordepth = 24;

   px = XCreatePixmap(eng_get_ob(re)->disp, eng_get_ob(re)->win, w, h, colordepth);
   if (!px)
     {
        ERR("Failed to create XPixmap!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_ALLOC);
        return NULL;
     }

#ifdef GL_GLES
   EGLSurface egl_sfc;
   EGLConfig egl_cfg;
   int i, num = 0, best = -1;
   EGLConfig configs[200];
   int config_attrs[40];
   Eina_Bool found = EINA_FALSE;
   int msaa = 0, depth = 0, stencil = 0;
   Visual *visual = NULL;
   Eina_Bool retried = EINA_FALSE;
   EGLint val = 0;

   /* Now we need to iterate over all EGL configurations to check the compatible
    * ones and finally check their visual ID. */

   if ((cfg->depth_bits > EVAS_GL_DEPTH_NONE) &&
       (cfg->depth_bits <= EVAS_GL_DEPTH_BIT_32))
     depth = 8 * ((int) cfg->depth_bits);

   if ((cfg->stencil_bits > EVAS_GL_STENCIL_NONE) &&
       (cfg->stencil_bits <= EVAS_GL_STENCIL_BIT_16))
     stencil = 1 << ((int) cfg->stencil_bits - 1);

   if ((cfg->multisample_bits > EVAS_GL_MULTISAMPLE_NONE) &&
       (cfg->multisample_bits <= EVAS_GL_MULTISAMPLE_HIGH))
     msaa = evgl->caps.msaa_samples[(int) cfg->multisample_bits - 1];

try_again:
   i = 0;
   config_attrs[i++] = EGL_SURFACE_TYPE;
   config_attrs[i++] = EGL_PIXMAP_BIT;
   config_attrs[i++] = EGL_RENDERABLE_TYPE;
   if (cfg->gles_version == EVAS_GL_GLES_3_X)
     config_attrs[i++] = EGL_OPENGL_ES3_BIT;
   else if (cfg->gles_version == EVAS_GL_GLES_2_X)
     config_attrs[i++] = EGL_OPENGL_ES2_BIT;
   else
     config_attrs[i++] = EGL_OPENGL_ES_BIT;
   if (alpha)
     {
        config_attrs[i++] = EGL_ALPHA_SIZE;
        config_attrs[i++] = 1; // should it be 8?
        DBG("Requesting RGBA pixmap");
     }
   else
     {
        config_attrs[i++] = EGL_ALPHA_SIZE;
        config_attrs[i++] = 0;
     }
   if (depth)
     {
        depth = 8 * ((int) cfg->depth_bits);
        config_attrs[i++] = EGL_DEPTH_SIZE;
        config_attrs[i++] = depth;
        DBG("Requesting depth buffer size %d", depth);
     }
   if (stencil)
     {
        stencil = 1 << ((int) cfg->stencil_bits - 1);
        config_attrs[i++] = EGL_STENCIL_SIZE;
        config_attrs[i++] = stencil;
        DBG("Requesting stencil buffer size %d", stencil);
     }
   if (msaa)
     {
        msaa = evgl->caps.msaa_samples[(int) cfg->multisample_bits - 1];
        config_attrs[i++] = EGL_SAMPLE_BUFFERS;
        config_attrs[i++] = 1;
        config_attrs[i++] = EGL_SAMPLES;
        config_attrs[i++] = msaa;
        DBG("Requesting MSAA buffer with %d samples", msaa);
     }
   config_attrs[i++] = EGL_NONE;
   config_attrs[i++] = 0;

   if (!eglChooseConfig(eng_get_ob(re)->egl_disp, config_attrs, configs, 200, &num) || !num)
     {
        int err = eglGetError();
        ERR("eglChooseConfig() can't find any configs, error: %x", err);
        glsym_evas_gl_common_error_set(data, err - EGL_SUCCESS);
        XFreePixmap(eng_get_ob(re)->disp, px);
        return NULL;
     }

   DBG("Found %d potential configurations", num);
   for (i = 0; (i < num) && !found; i++)
     {
        VisualID visid = 0;
        XVisualInfo *xvi, vi_in;
        XRenderPictFormat *fmt;
        int nvi = 0, j;

        if (!eglGetConfigAttrib(eng_get_ob(re)->egl_disp, configs[i],
                                EGL_NATIVE_VISUAL_ID, &val))
          continue;

        // Find matching visuals. Only alpha & depth are really valid here.
        visid = val;
        vi_in.screen = eng_get_ob(re)->screen;
        vi_in.visualid = visid;
        xvi = XGetVisualInfo(eng_get_ob(re)->disp,
                             VisualScreenMask | VisualIDMask,
                             &vi_in, &nvi);
        if (xvi)
          {
             for (j = 0; (j < nvi) && !found; j++)
               {
                  if (xvi[j].depth >= colordepth)
                    {
                       if (best < 0) best = i;
                       if (alpha)
                         {
                            fmt = XRenderFindVisualFormat(eng_get_ob(re)->disp, xvi[j].visual);
                            if (fmt && (fmt->direct.alphaMask))
                              found = EINA_TRUE;
                         }
                       else found = EINA_TRUE;
                    }
               }
             if (found)
               {
                  egl_cfg = configs[i];
                  visual = xvi[j].visual;
                  XFree(xvi);
                  break;
               }
             XFree(xvi);
          }
     }

   if (!found)
     {
        if (num && (best >= 0))
          {
             ERR("No matching config found. Trying with EGL config #%d", best);
             egl_cfg = configs[best];
          }
        else if (msaa && !retried)
          {
             ERR("Trying again without MSAA.");
             msaa = 0;
             retried = EINA_TRUE;
             goto try_again;
          }
        else
          {
             // This config will probably not work, but we try anyways.
             // NOTE: Maybe it would be safer to just return NULL here, leaving
             // the app responsible for changing its config.
             ERR("XGetVisualInfo failed. Trying with the window's EGL config.");
             egl_cfg = eng_get_ob(re)->egl_config;
          }
     }

   egl_sfc = eglCreatePixmapSurface(eng_get_ob(re)->egl_disp, egl_cfg, px, NULL);
   if (!egl_sfc)
     {
        int err = eglGetError();
        ERR("eglCreatePixmapSurface failed with error: %x", err);
        glsym_evas_gl_common_error_set(data, err - EGL_SUCCESS);
        XFreePixmap(eng_get_ob(re)->disp, px);
        return NULL;
     }

   if (extn_have_y_inverted &&
       eglGetConfigAttrib(eng_get_ob(re)->egl_disp, egl_cfg,
                          EGL_Y_INVERTED_NOK, &val))
     evgl_sfc->yinvert = val;
   else
     evgl_sfc->yinvert = 1;

   evgl_sfc->indirect = EINA_TRUE;
   evgl_sfc->indirect_sfc = egl_sfc;
   evgl_sfc->indirect_sfc_native = (void *)(intptr_t) px;
   evgl_sfc->indirect_sfc_visual = visual;
   evgl_sfc->indirect_sfc_config = egl_cfg;
   DBG("Successfully created indirect surface: Pixmap %lu EGLSurface %p", px, egl_sfc);
   return evgl_sfc;

#else
   // TODO/FIXME: do the same as with EGL above...
   ERR("GLX support is not fully implemented for indirect surface");

   evgl_sfc->indirect = EINA_TRUE;
   evgl_sfc->indirect_sfc_native = (void *)(intptr_t) px;
   evgl_sfc->indirect_sfc = (void *)(intptr_t) px;
   evgl_sfc->indirect_sfc_visual = eng_get_ob(re)->info->info.visual; // FIXME: Check this!
   return evgl_sfc;
#endif
}

// This function should destroy the indirect surface as well as the X pixmap
static int
evgl_eng_indirect_surface_destroy(void *data, EVGL_Surface *evgl_sfc)
{
   Render_Engine *re = (Render_Engine *)data;

   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

#ifdef GL_GLES
   if ((!evgl_sfc) || (!evgl_sfc->indirect_sfc))
     {
        ERR("Invalid surface");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_SURFACE);
        return 0;
     }

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)evgl_sfc->indirect_sfc);
#endif

   if (!evgl_sfc->indirect_sfc_native)
     {
        ERR("Inconsistent parameters, not freeing XPixmap for indirect surface!");
        glsym_evas_gl_common_error_set(data, EVAS_GL_BAD_PARAMETER);
        return 0;
     }

   XFreePixmap(eng_get_ob(re)->disp, (Pixmap)evgl_sfc->indirect_sfc_native);

   return 1;
}

static void *
evgl_eng_gles_context_create(void *data,
                              EVGL_Context *share_ctx, EVGL_Surface *sfc)
{
   Render_Engine *re = data;
   if (!re) return NULL;

#ifdef GL_GLES
   EGLContext context = EGL_NO_CONTEXT;
   int context_attrs[3];
   EGLConfig config;

   if (!share_ctx)
     {
        ERR("Share context not set, Unable to retrieve GLES version");
        return NULL;
     }

   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = share_ctx->version;
   context_attrs[2] = EGL_NONE;

   if (!sfc || !sfc->indirect_sfc_config)
     {
        ERR("Surface is not set! Creating context anyways but evas_eglMakeCurrent "
            "might very well fail with EGL_BAD_MATCH (0x3009)");
        config = eng_get_ob(re)->egl_config;
     }
   else config = sfc->indirect_sfc_config;

   context = eglCreateContext(eng_get_ob(re)->egl_disp, config,
                              share_ctx->context,
                              context_attrs);
   if (!context)
     {
        int err = eglGetError();
        ERR("eglCreateContext failed with error 0x%x", err);
        glsym_evas_gl_common_error_set(data, err - EGL_SUCCESS);
        return NULL;
     }

   DBG("Successfully created context for indirect rendering.");
   return context;
#else
   CRI("Support for indirect rendering contexts is not implemented for GLX");
   (void) share_ctx; (void) sfc;
   return NULL;
#endif
}

static void
evgl_eng_native_win_surface_config_get(void *data, int *win_depth,
                                         int *win_stencil, int *win_msaa)
{
   Render_Engine *re = data;
   if (!re) return;

   if (win_depth)
     *win_depth = eng_get_ob(re)->detected.depth_buffer_size;
   if (win_stencil)
     *win_stencil = eng_get_ob(re)->detected.stencil_buffer_size;
   if (win_msaa)
     *win_msaa = eng_get_ob(re)->detected.msaa;

   DBG("Window config(depth %d, stencil %d, msaa %d)",
       eng_get_ob(re)->detected.depth_buffer_size,
       eng_get_ob(re)->detected.stencil_buffer_size,
       eng_get_ob(re)->detected.msaa);
}

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
   evgl_eng_pbuffer_surface_create,
   evgl_eng_pbuffer_surface_destroy,
   evgl_eng_indirect_surface_create,
   evgl_eng_indirect_surface_destroy,
   evgl_eng_gles_context_create,
   evgl_eng_native_win_surface_config_get,
};

//----------------------------------------------------------//

static inline int
_has_ext(const char *exts, const char *ext)
{
   if (!exts || !ext) return EINA_FALSE;
   return strstr(exts, ext) != NULL;
}

static void
gl_symbols(void)
{
   static int done = 0;

   if (done) return;

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym); \
   if (!glsym_##sym) ERR("Could not find function '%s'", #sym);

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
   LINK2GENERIC(evgl_native_surface_buffer_get);
   LINK2GENERIC(evgl_native_surface_yinvert_get);
   LINK2GENERIC(evgl_current_native_context_get);
   LINK2GENERIC(evas_gl_symbols);
   LINK2GENERIC(evas_gl_common_error_get);
   LINK2GENERIC(evas_gl_common_error_set);
   LINK2GENERIC(evas_gl_common_current_context_get);
   LINK2GENERIC(evas_gl_common_shaders_flush);

#define FINDSYM(dst, sym, typ) if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym);
#ifdef GL_GLES

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);

#else

   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddress", glsym_func_eng_fn);

#endif
#undef FINDSYM

   done = 1;
}

void
eng_gl_symbols(Outbuf *ob)
{
   static int done = 0;
   const char *exts;

   if (done) return;

   /* GetProcAddress() may not return NULL, even if the extension is not
    * supported. Nvidia drivers since version 360 never return NULL, thus
    * we need to always match the function name with their full extension
    * name. Other drivers tend to return NULL for glX/egl prefixed names, but
    * this could change in the future.
    *
    * -- jpeg, 2016/08/04
    */

#ifdef GL_GLES
#define FINDSYM(dst, sym, ext, typ) do { \
   if (!dst) { \
      if (_has_ext(exts, ext) && glsym_eglGetProcAddress) \
        dst = (typ) glsym_eglGetProcAddress(sym); \
      if (!dst) \
        dst = (typ) dlsym(RTLD_DEFAULT, sym); \
   }} while (0)

   // Find EGL extensions
   exts = eglQueryString(ob->egl_disp, EGL_EXTENSIONS);

   // Find GL extensions
   glsym_evas_gl_symbols((void*)glsym_eglGetProcAddress, exts);

   LINK2GENERIC(evas_gl_common_eglCreateImage);
   LINK2GENERIC(evas_gl_common_eglDestroyImage);

   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamage", NULL, glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageEXT", "EGL_EXT_swap_buffers_with_damage", glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageKHR", "EGL_KHR_swap_buffers_with_damage", glsym_func_uint);
   FINDSYM(glsym_eglSwapBuffersWithDamage, "eglSwapBuffersWithDamageINTEL", "EGL_INTEL_swap_buffers_with_damage", glsym_func_uint);

   FINDSYM(glsym_eglSetDamageRegionKHR, "eglSetDamageRegionKHR", "EGL_KHR_partial_update", glsym_func_uint);

   FINDSYM(glsym_eglQueryWaylandBufferWL, "eglQueryWaylandBufferWL", "EGL_WL_bind_wayland_display", glsym_func_uint);

   // This is a GL extension
   exts = (const char *) glGetString(GL_EXTENSIONS);
   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", "GL_OES_EGL_image", glsym_func_void);
   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", "GL_OES_EGL_image_external", glsym_func_void);

#else

#define FINDSYM(dst, sym, ext, typ) do { \
   if (!dst) { \
      if (_has_ext(exts, ext) && glsym_glXGetProcAddress) \
        dst = (typ) glsym_glXGetProcAddress(sym); \
      if (!dst) \
        dst = (typ) dlsym(RTLD_DEFAULT, sym); \
   }} while (0)

   // Find GLX extensions
   exts = glXQueryExtensionsString((Display *) ob->disp, ob->screen);

   // Find GL extensions
   glsym_evas_gl_symbols((void*)glsym_glXGetProcAddress, exts);

   FINDSYM(glsym_glXBindTexImage, "glXBindTexImage", NULL, glsym_func_void);
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageEXT", "GLX_EXT_texture_from_pixmap", glsym_func_void);
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageARB", "GLX_ARB_render_texture", glsym_func_void);

   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImage", NULL, glsym_func_void);
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageEXT", "GLX_EXT_texture_from_pixmap", glsym_func_void);
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageARB", "GLX_ARB_render_texture", glsym_func_void);

   FINDSYM(glsym_glXGetVideoSync, "glXGetVideoSyncSGI", "GLX_SGI_video_sync", glsym_func_int);
   FINDSYM(glsym_glXWaitVideoSync, "glXWaitVideoSyncSGI", "GLX_SGI_video_sync", glsym_func_int);

   // GLX 1.3
   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmap", NULL, glsym_func_xid);
   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmap", NULL, glsym_func_void);
   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawable", NULL, glsym_func_int);

   // swap interval: MESA and SGI take (interval)
   FINDSYM(glsym_glXSwapIntervalSGI, "glXSwapIntervalMESA", "GLX_MESA_swap_control", glsym_func_int);
   FINDSYM(glsym_glXSwapIntervalSGI, "glXSwapIntervalSGI", "GLX_SGI_swap_control", glsym_func_int);

   // swap interval: EXT takes (dpy, drawable, interval)
   FINDSYM(glsym_glXSwapIntervalEXT, "glXSwapIntervalEXT", "GLX_EXT_swap_control", glsym_func_void);

   FINDSYM(glsym_glXReleaseBuffersMESA, "glXReleaseBuffersMESA", "GLX_MESA_release_buffers", glsym_func_void);

#endif
#undef FINDSYM

   done = 1;
}

static void
gl_extn_veto(Render_Engine *re)
{
   const char *str = NULL;
#ifdef GL_GLES
   str = eglQueryString(eng_get_ob(re)->egl_disp, EGL_EXTENSIONS);
   if (str)
     {
        const char *s;
        if (getenv("EVAS_GL_INFO"))
          printf("EGL EXTN:\n%s\n", str);
        // Disable Partial Rendering
        if ((s = getenv("EVAS_GL_PARTIAL_DISABLE")) && atoi(s))
          {
             extn_have_buffer_age = 0;
             glsym_eglSwapBuffersWithDamage = NULL;
             glsym_eglSetDamageRegionKHR = NULL;
          }
        if (!strstr(str, "EGL_EXT_buffer_age"))
          {
             if (!strstr(str, "EGL_KHR_partial_update"))
               extn_have_buffer_age = 0;
          }
        if (!strstr(str, "EGL_KHR_partial_update"))
          {
             glsym_eglSetDamageRegionKHR = NULL;
          }
        if (!strstr(str, "EGL_NOK_texture_from_pixmap"))
          {
             extn_have_y_inverted = 0;
          }
        else
          {
             const GLubyte *vendor, *renderer;

             vendor = glGetString(GL_VENDOR);
             renderer = glGetString(GL_RENDERER);
             // XXX: workaround mesa bug!
             // looking for mesa and intel build which is known to
             // advertise the EGL_NOK_texture_from_pixmap extension
             // but not set it correctly. guessing vendor/renderer
             // strings will be like the following:
             // OpenGL vendor string: Intel Open Source Technology Center
             // OpenGL renderer string: Mesa DRI Intel(R) Sandybridge Desktop
             if (((vendor) && (strstr((const char *)vendor, "Intel"))) &&
                 ((renderer) && (strstr((const char *)renderer, "Mesa"))) &&
                 ((renderer) && (strstr((const char *)renderer, "Intel")))
                )
               extn_have_y_inverted = 0;
          }
        if (!strstr(str, "EGL_EXT_swap_buffers_with_damage"))
          {
             glsym_eglSwapBuffersWithDamage = NULL;
          }
        if (strstr(str, "EGL_TIZEN_image_native_surface"))
          {
             eng_get_ob(re)->gl_context->shared->info.egl_tbm_ext = 1;
          }
     }
   else
     {
        if (getenv("EVAS_GL_INFO"))
          printf("NO EGL EXTN!\n");
        extn_have_buffer_age = 0;
     }
#else
   str = glXQueryExtensionsString(eng_get_ob(re)->info->info.display,
                                  eng_get_ob(re)->info->info.screen);
   if (str)
     {
        if (getenv("EVAS_GL_INFO"))
          printf("GLX EXTN:\n%s\n", str);
        if (!strstr(str, "_texture_from_pixmap"))
          {
             glsym_glXBindTexImage = NULL;
             glsym_glXReleaseTexImage = NULL;
          }
        if (!strstr(str, "GLX_SGI_video_sync"))
          {
             glsym_glXGetVideoSync = NULL;
             glsym_glXWaitVideoSync = NULL;
          }
        if (!strstr(str, "GLX_EXT_buffer_age"))
          {
             extn_have_buffer_age = 0;
          }
        if (!strstr(str, "GLX_EXT_swap_control"))
          {
             glsym_glXSwapIntervalEXT = NULL;
          }
        if (!strstr(str, "GLX_SGI_swap_control"))
          {
             glsym_glXSwapIntervalSGI = NULL;
          }
        if (!strstr(str, "GLX_MESA_release_buffers"))
          {
             glsym_glXReleaseBuffersMESA = NULL;
          }
     }
   else
     {
        if (getenv("EVAS_GL_INFO"))
          printf("NO GLX EXTN!\n");
        glsym_glXBindTexImage = NULL;
        glsym_glXReleaseTexImage = NULL;
        glsym_glXGetVideoSync = NULL;
        glsym_glXWaitVideoSync = NULL;
        extn_have_buffer_age = 0;
        glsym_glXSwapIntervalEXT = NULL;
        glsym_glXSwapIntervalSGI = NULL;
        glsym_glXReleaseBuffersMESA = NULL;
     }
#endif
}

int _evas_engine_GL_X11_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static void *
eng_info(void)
{
   Evas_Engine_Info_GL_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   info->magic.magic = rand();
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;
   return info;
}

static void
eng_info_free(void *info)
{
   Evas_Engine_Info_GL_X11 *in;
// dont free! why bother? its not worth it
//   eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static void
eng_outbuf_idle_flush(Outbuf *ob)
{
   if (glsym_evas_gl_common_shaders_flush)
     glsym_evas_gl_common_shaders_flush(ob->gl_context->shared);
}

static void
_re_winfree(Render_Engine *re)
{
   if (!eng_get_ob(re)->surf) return;
   glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));
   eng_window_unsurf(eng_get_ob(re));
}

static void *
eng_setup(void *engine EINA_UNUSED, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_GL_X11 *info = in;
   Render_Engine *re = NULL;
   Outbuf *ob = NULL;
   Render_Engine_Merge_Mode merge_mode = MERGE_SMART;
   Render_Engine_Swap_Mode swap_mode;
   const char *s;

   swap_mode = evas_render_engine_gl_swap_mode_get(info->swap_mode);

   // Set this env var to dump files every frame
   // Or set the global var in gdb to 1|0 to turn it on and off
   if (getenv("EVAS_GL_SWAP_BUFFER_DEBUG_ALWAYS"))
     swap_buffer_debug = 1;

   if (swap_buffer_debug_mode == -1)
     {
        if (
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
            (getuid() == geteuid()) &&
#endif
            ((debug_dir = getenv("EVAS_GL_SWAP_BUFFER_DEBUG_DIR"))))
          {
             int stat;
             // Create a directory with 0775 permission
             stat = mkdir(debug_dir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
             if ((!stat) || errno == EEXIST) swap_buffer_debug_mode = 1;
          }
        else
           swap_buffer_debug_mode = 0;
     }


   if (!initted)
     {
        glsym_evas_gl_preload_init();
     }

#ifdef GL_GLES
#else
   int eb, evb;

   if (!glXQueryExtension(info->info.display, &eb, &evb)) return 0;
#endif
   re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;

   ob = eng_window_new(info,
                       info->info.display,
                       info->info.drawable,
                       info->info.screen,
                       info->info.visual,
                       info->info.colormap,
                       info->info.depth,
                       w, h,
                       info->indirect,
                       info->info.destination_alpha,
                       info->info.rotation,
                       swap_mode,
                       info->depth_bits,
                       info->stencil_bits,
                       info->msaa_bits);
   if (!ob) goto on_error;

   if (!evas_render_engine_gl_generic_init(&re->generic, ob,
                                           eng_outbuf_swap_mode,
                                           eng_outbuf_get_rot,
                                           eng_outbuf_reconfigure,
                                           eng_outbuf_region_first_rect,
#ifdef GL_GLES
                                           eng_outbuf_damage_region_set,
#else
                                           NULL,
#endif
                                           eng_outbuf_new_region_for_update,
                                           eng_outbuf_push_updated_region,
                                           eng_outbuf_push_free_region_for_update,
                                           eng_outbuf_idle_flush,
                                           eng_outbuf_flush,
                                           NULL,
                                           eng_window_free,
                                           eng_window_use,
                                           eng_outbuf_gl_context_get,
                                           eng_outbuf_egl_display_get,
                                           eng_gl_context_new,
                                           eng_gl_context_use,
                                           &evgl_funcs,
                                           w, h))
     goto on_error;

   gl_wins++;

   if ((s = getenv("EVAS_GL_PARTIAL_MERGE")))
     {
        if ((!strcmp(s, "bounding")) ||
            (!strcmp(s, "b")))
          merge_mode = MERGE_BOUNDING;
        else if ((!strcmp(s, "full")) ||
                 (!strcmp(s, "f")))
          merge_mode = MERGE_FULL;
        else if ((!strcmp(s, "smart")) ||
                 (!strcmp(s, "s")))
          merge_mode = MERGE_SMART;
     }

   evas_render_engine_software_generic_merge_mode_set(&re->generic.software, merge_mode);

   if (!initted)
     {
        gl_extn_veto(re);
        //             evgl_engine_init(re, &evgl_funcs);
        initted = 1;
     }

   if (re->generic.software.tb)
     evas_render_engine_software_generic_tile_strict_set
       (&re->generic.software, EINA_TRUE);

   eng_window_use(eng_get_ob(re));

   return re;

 on_error:
   if (ob) eng_window_free(ob);
   free(re);
   return NULL;
}

static int
eng_update(void *engine EINA_UNUSED, void *data, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_GL_X11 *info = in;
   Render_Engine *re = data;
   Render_Engine_Swap_Mode swap_mode;

   swap_mode = evas_render_engine_gl_swap_mode_get(info->swap_mode);

   if (eng_get_ob(re) && _re_wincheck(eng_get_ob(re)))
     {
        if ((info->info.display != eng_get_ob(re)->disp) ||
            (info->info.drawable != eng_get_ob(re)->win) ||
            (info->info.screen != eng_get_ob(re)->screen) ||
            (info->info.visual != eng_get_ob(re)->visual) ||
            (info->info.colormap != eng_get_ob(re)->colormap) ||
            (info->info.depth != eng_get_ob(re)->depth) ||
            (info->depth_bits != eng_get_ob(re)->depth_bits) ||
            (info->stencil_bits != eng_get_ob(re)->stencil_bits) ||
            (info->msaa_bits != eng_get_ob(re)->msaa_bits) ||
            (info->info.destination_alpha != eng_get_ob(re)->alpha))
          {
             Outbuf *ob, *ob_old;

             ob_old = re->generic.software.ob;
             re->generic.software.ob = NULL;
             gl_wins--;

             ob = eng_window_new(info,
                                 info->info.display,
                                 info->info.drawable,
                                 info->info.screen,
                                 info->info.visual,
                                 info->info.colormap,
                                 info->info.depth,
                                 w, h,
                                 info->indirect,
                                 info->info.destination_alpha,
                                 info->info.rotation,
                                 swap_mode,
                                 info->depth_bits,
                                 info->stencil_bits,
                                 info->msaa_bits);
             if (!ob)
               {
                  if (ob_old) eng_window_free(ob_old);
                  return 0;
               }

             eng_window_use(ob);
             if (ob_old) eng_window_free(ob_old);
             evas_render_engine_software_generic_update(&re->generic.software, ob,
                                                        w, h);
             gl_wins++;
          }
        else if ((eng_get_ob(re)->w != w) ||
                 (eng_get_ob(re)->h != h) ||
                 (eng_get_ob(re)->info->info.rotation != eng_get_ob(re)->rot))
          {
             eng_outbuf_reconfigure(eng_get_ob(re), w, h, eng_get_ob(re)->info->info.rotation, 0);
             if (re->generic.software.tb)
               evas_common_tilebuf_free(re->generic.software.tb);
             re->generic.software.tb = evas_common_tilebuf_new(w, h);
             if (re->generic.software.tb)
               evas_common_tilebuf_set_tile_size(re->generic.software.tb,
                                                 TILESIZE, TILESIZE);
          }
     }

   eng_window_use(eng_get_ob(re));

   return 1;
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re)
     {
#ifndef GL_GLES
        Display *disp = eng_get_ob(re)->disp;
        Window win = eng_get_ob(re)->win;
#endif

        glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));

#if 0
#ifdef GL_GLES
        // Destroy the resource surface
        // Only required for EGL case
        if (re->surface)
           eglDestroySurface(eng_get_ob(re)->egl_disp, re->surface);
#endif

        // Destroy the resource context
        _destroy_internal_context(re, context);
#endif

        if (gl_wins == 1) glsym_evgl_engine_shutdown(re);

        evas_render_engine_software_generic_clean(&re->generic.software);

#ifndef GL_GLES
        if (glsym_glXReleaseBuffersMESA)
          glsym_glXReleaseBuffersMESA(disp, win);
#endif
        gl_wins--;

        free(re);
     }
   if ((initted == 1) && (gl_wins == 0))
     {
        glsym_evas_gl_preload_shutdown();
        initted = 0;
     }
}

/* vsync games - not for now though */
#define VSYNC_TO_SCREEN 1

Eina_Bool
eng_preload_make_current(void *data, void *doit)
{
   Outbuf *ob = data;

   if (doit)
     {
#ifdef GL_GLES
        if (!evas_eglMakeCurrent(ob->egl_disp, ob->egl_surface, ob->egl_surface, ob->egl_context))
          return EINA_FALSE;
#else
        if (!__glXMakeContextCurrent(ob->info->info.display, ob->glxwin, ob->context))
          {
             ERR("glXMakeContextCurrent(%p, %p, %p) failed",
                 ob->info->info.display, (void *)ob->win, (void *)ob->context);
             GLERRV("__glXMakeContextCurrent");
             return EINA_FALSE;
          }
#endif
     }
   else
     {
#ifdef GL_GLES
        if (!evas_eglMakeCurrent(ob->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
#else
        if (!__glXMakeContextCurrent(ob->info->info.display, 0, NULL))
          {
             ERR("glXMakeContextCurrent(%p, None, NULL) failed",
                 ob->info->info.display);
             GLERRV("__glXMakeContextCurrent");
             return EINA_FALSE;
          }
#endif
     }
   return EINA_TRUE;
}

static Eina_Bool
eng_canvas_alpha_get(void *engine)
{
   Render_Engine *re = (Render_Engine *)engine;
   return re->generic.software.ob->alpha;
}

static void
eng_output_dump(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re = data;

   eng_window_use(eng_get_ob(re));
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   glsym_evas_gl_common_image_all_unload(eng_get_ob(re)->gl_context);
   _re_winfree(re);
}

static void *
eng_gl_current_context_get(void *engine EINA_UNUSED)
{
   EVGL_Context *ctx;
   EVGLNative_Context context;

   ctx = glsym_evas_gl_common_current_context_get();
   if (!ctx)
     return NULL;

   context = glsym_evgl_current_native_context_get(ctx);

#ifdef GL_GLES
   if (evas_eglGetCurrentContext() == context)
     return ctx;
#else
   if (glXGetCurrentContext() == context)
     return ctx;
#endif

   return NULL;
}

static int
eng_gl_error_get(void *engine)
{
   int err;

   if ((err = glsym_evas_gl_common_error_get(engine)) != EVAS_GL_SUCCESS)
     goto end;

#ifdef GL_GLES
   err = eglGetError() - EGL_SUCCESS;
#else
   Render_Engine *re = engine;

   if (!eng_get_ob(re)->win)
     err = EVAS_GL_BAD_DISPLAY;
   else if (!eng_get_ob(re)->info)
     err = EVAS_GL_BAD_SURFACE;
#endif

end:
   glsym_evas_gl_common_error_set(engine, EVAS_GL_SUCCESS);
   return err;
}

/////////////////////////////////////////////////////////////////////////
//
//
// FIXME: this is enabled so updates happen - but its SLOOOOOOOOOOOOOOOW
// (i am sure this is the reason)  not to mention seemingly superfluous. but
// i need to enable it for it to work on fglrx at least. havent tried nvidia.
//
// why is this the case? does anyone know? has anyone tried it on other gfx
// drivers?
//
//#define GLX_TEX_PIXMAP_RECREATE 1

static void
_native_bind_cb(void *image)
{
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
#ifdef GL_GLES
       if (n->ns_data.x11.surface)
         {
            if (n->ns_data.x11.multiple_buffer)
              {
                 EGLint err;
                 if (!glsym_evas_gl_common_eglDestroyImage)
                   {
                      ERR("Try eglDestroyImage()/eglCreateImage() on EGL with no support");
                      return;
                   }

                 glsym_evas_gl_common_eglDestroyImage(im->native.disp,
                                                      n->ns_data.x11.surface);
                 if ((err = eglGetError()) != EGL_SUCCESS)
                   {
                      ERR("eglDestroyImage() failed.");
                      glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
                   }

                 n->ns_data.x11.surface = glsym_evas_gl_common_eglCreateImage(im->native.disp,
                                                          EGL_NO_CONTEXT,
                                                          EGL_NATIVE_PIXMAP_KHR,
                                                          (void *)n->ns_data.x11.pixmap,
                                                          NULL);
                 if (!n->ns_data.x11.surface)
                   ERR("eglCreateImage() for Pixmap 0x%#lx failed: %#x", n->ns_data.x11.pixmap, eglGetError());

              }
            if (glsym_glEGLImageTargetTexture2DOES)
              {
                 glsym_glEGLImageTargetTexture2DOES(im->native.target, n->ns_data.x11.surface);
                 GLERRV("glsym_glEGLImageTargetTexture2DOES");
              }
            else
              ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
         }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT

       if (glsym_glXBindTexImage)
         {
            glsym_glXBindTexImage(im->native.disp, (XID)n->ns_data.x11.surface,
                                  GLX_FRONT_LEFT_EXT, NULL);
            GLERRV("glsym_glXBindTexImage");
         }
       else
         ERR("Try glXBindTexImage on GLX with no support");
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       glBindTexture(im->native.target, n->ns.data.opengl.texture_id);
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_TBM)
    {
#ifdef GL_GLES
       if (n->ns_data.tbm.surface)
         {
            if (glsym_glEGLImageTargetTexture2DOES)
              {
                 glsym_glEGLImageTargetTexture2DOES(im->native.target, n->ns_data.tbm.surface);
                 GLERRV("glsym_glEGLImageTargetTexture2DOES");
              }
             else
               ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
         }
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_EVASGL)
    {
       if (n->ns_data.evasgl.surface)
         {
             Eina_Bool is_egl_image = EINA_FALSE;
             void *surface = NULL;

             if (glsym_evgl_native_surface_buffer_get)
               surface = glsym_evgl_native_surface_buffer_get(n->ns_data.evasgl.surface, &is_egl_image);
            if (is_egl_image)
              {
#ifdef GL_GLES
                 if (glsym_glEGLImageTargetTexture2DOES)
                   {
                      glsym_glEGLImageTargetTexture2DOES(im->native.target, surface);
                      GLERRV("glsym_glEGLImageTargetTexture2DOES");
                   }
                 else
#endif
                   ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
              }
            else
              {
                 glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)surface);
              }
         }
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
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
# endif
#endif
     }
}

static void
_native_unbind_cb(void *image)
{
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

   if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
     {
#ifdef GL_GLES
        // nothing
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT

        if (glsym_glXReleaseTexImage)
          {
             glsym_glXReleaseTexImage(im->native.disp, (XID)(n->ns_data.x11.surface),
                                      GLX_FRONT_LEFT_EXT);
          }
        else
          ERR("Try glXReleaseTexImage on GLX with no support");
# endif
#endif
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        glBindTexture(im->native.target, 0);
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_TBM)
     {
        // nothing
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_EVASGL)
     {
#ifdef GL_GLES
        // nothing
#else
        glBindTexture(GL_TEXTURE_2D, 0);
#endif
     }
}

static void
_native_free_cb(void *image)
{
  Evas_GL_Image *im = image;
  Native *n = im->native.data;
  uint32_t pmid, texid;
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
  void *wlid;
# endif
#endif

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
       pmid = n->ns_data.x11.pixmap;
       eina_hash_del(im->native.shared->native_pm_hash, &pmid, im);
#ifdef GL_GLES
       if (n->ns_data.x11.surface)
         {
            int err;
            if (glsym_evas_gl_common_eglDestroyImage)
              {
                 glsym_evas_gl_common_eglDestroyImage(im->native.disp,
                                                      n->ns_data.x11.surface);
                 if ((err = eglGetError()) != EGL_SUCCESS)
                   {
                      ERR("eglDestroyImage() failed.");
                      glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
                   }
              }
            else
              ERR("Try eglDestroyImage on EGL with no support");
         }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
       if (n->ns_data.x11.surface)
         {
            if (im->native.loose)
              {
                 if (glsym_glXReleaseTexImage)
                   {
                     glsym_glXReleaseTexImage(im->native.disp, (XID)n->ns_data.x11.surface,
                                              GLX_FRONT_LEFT_EXT);
                   }
                 else
                   ERR("Try glXReleaseTexImage on GLX with no support");
              }
            if (glsym_glXDestroyPixmap)
              {
                 glsym_glXDestroyPixmap(im->native.disp, (XID)n->ns_data.x11.surface);
                 GLERRV("glsym_glXDestroyPixmap");
              }
            else
              ERR("Try glXDestroyPixmap on GLX with no support");
            n->ns_data.x11.surface = 0;
         }
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = n->ns.data.opengl.texture_id;
       eina_hash_del(im->native.shared->native_tex_hash, &texid, im);
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_TBM)
    {
       eina_hash_del(im->native.shared->native_tbm_hash, &n->ns_data.tbm.buffer, im);
#ifdef GL_GLES
       if (n->ns_data.tbm.surface)
         {
            int err;
            if (glsym_evas_gl_common_eglDestroyImage)
              {
                 glsym_evas_gl_common_eglDestroyImage(im->native.disp,
                                                      n->ns_data.tbm.surface);
                 if ((err = eglGetError()) != EGL_SUCCESS)
                   {
                      ERR("eglDestroyImage() failed.");
                      glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
                   }
              }
            else
              ERR("Try eglDestroyImage on EGL with no support");
         }
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_EVASGL)
    {
       eina_hash_del(im->native.shared->native_evasgl_hash, &n->ns.data.evasgl.surface, im);
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_WL)
     {
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
        wlid = (void*)n->ns_data.wl_surface.wl_buf;
        eina_hash_del(im->native.shared->native_wl_hash, &wlid, image);
        if (n->ns_data.wl_surface.surface)
          {
             if (glsym_evas_gl_common_eglDestroyImage)
               {
                  glsym_evas_gl_common_eglDestroyImage(im->native.disp,
                                                       n->ns_data.wl_surface.surface);
                  if (eglGetError() != EGL_SUCCESS)
                    ERR("eglDestroyImage() failed.");
               }
             else
               ERR("Try eglDestroyImage on EGL with  no support");
          }
# endif
#endif
     }
  im->native.data        = NULL;
  im->native.func.bind   = NULL;
  im->native.func.unbind = NULL;
  im->native.func.free   = NULL;
  free(n);
}

static int
_native_yinvert_cb(void *image)
{
   Evas_GL_Image *im = image;
   Native *n = im->native.data;
   int yinvert = 0, val;

   // Yinvert callback should only be used for EVAS_NATIVE_SURFACE_EVASGL type now,
   // as yinvert value is not changed for other types.
   if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
     {
#if GL_GLES
        if (extn_have_y_inverted &&
            eglGetConfigAttrib(im->native.disp, n->ns_data.x11.config,
                               EGL_Y_INVERTED_NOK, &val))
          yinvert = val;
#else
        glXGetFBConfigAttrib(im->native.disp, n->ns_data.x11.config,
                             GLX_Y_INVERTED_EXT, &val);
        if (val) yinvert = 1;
#endif
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
     {
        yinvert = 0;
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_TBM)
     {
        yinvert = 1;
     }
   else if (n->ns.type == EVAS_NATIVE_SURFACE_EVASGL)
     {
        yinvert = glsym_evgl_native_surface_yinvert_get(n->ns_data.evasgl.surface);
     }

   return yinvert;
}

static int
eng_image_native_init(void *engine EINA_UNUSED, Evas_Native_Surface_Type type)
{
   switch (type)
     {
#ifdef GL_GLES
      case EVAS_NATIVE_SURFACE_TBM:
        return _evas_native_tbm_init();
#endif
      case EVAS_NATIVE_SURFACE_X11:
      case EVAS_NATIVE_SURFACE_OPENGL:
      case EVAS_NATIVE_SURFACE_EVASGL:
        return 1;
#if defined(GL_GLES) && defined(HAVE_WAYLAND)
      case EVAS_NATIVE_SURFACE_WL:
        return (glsym_eglQueryWaylandBufferWL != NULL) ? 1 : 0;
#endif
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
#ifdef GL_GLES
      case EVAS_NATIVE_SURFACE_TBM:
        _evas_native_tbm_shutdown();
        return;
#endif
      case EVAS_NATIVE_SURFACE_X11:
      case EVAS_NATIVE_SURFACE_OPENGL:
      case EVAS_NATIVE_SURFACE_EVASGL:
#if defined(GL_GLES) && defined(HAVE_WAYLAND)
      case EVAS_NATIVE_SURFACE_WL:
#endif
        return;
      default:
        ERR("Native surface type %d not supported!", type);
        return;
     }
}

static void *
eng_image_native_set(void *engine, void *image, void *native)
{
  Render_Engine *re = (Render_Engine *)engine;
  const Evas_Native_Surface *ns = native;
  Evas_GL_Image *im = image, *im2 = NULL;
  Visual *vis = NULL;
  Pixmap pm = 0;
  Native *n = NULL;
  uint32_t pmid, texid;
  unsigned int tex = 0;
  unsigned int fbo = 0;
  void *buffer = NULL;
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
  void *wlid, *wl_buf = NULL;
# endif
#endif

  if (!im)
    {
       if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
         {
            im = glsym_evas_gl_common_image_new_from_data(eng_get_ob(re)->gl_context,
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
       if (ns->type == EVAS_NATIVE_SURFACE_X11)
         {
            vis = ns->data.x11.visual;
            pm = ns->data.x11.pixmap;
            if (im->native.data)
              {
                 Evas_Native_Surface *ens = im->native.data;
                 if ((ens->data.x11.visual == vis) &&
                     (ens->data.x11.pixmap == pm))
                   return im;
              }
         }
       else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
         {
            tex = ns->data.opengl.texture_id;
            fbo = ns->data.opengl.framebuffer_id;
            if (im->native.data)
              {
                 Evas_Native_Surface *ens = im->native.data;
                 if ((ens->data.opengl.texture_id == tex) &&
                     (ens->data.opengl.framebuffer_id == fbo))
                   return im;
              }
         }
       else if (ns->type == EVAS_NATIVE_SURFACE_TBM)
         {
            buffer = ns->data.tbm.buffer;
            if (im->native.data)
              {
                 Evas_Native_Surface *ens = im->native.data;
                 if (ens->data.tbm.buffer == buffer)
                   return im;
              }
         }
       else if (ns->type == EVAS_NATIVE_SURFACE_EVASGL)
         {
            buffer = ns->data.evasgl.surface;
            if (im->native.data)
              {
                 Evas_Native_Surface *ens = im->native.data;
                 if (ens->data.evasgl.surface == buffer)
                   return im;
              }
         }
       else if (ns->type == EVAS_NATIVE_SURFACE_WL)
          {
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
             wl_buf = ns->data.wl.legacy_buffer;
             if (im->native.data)
               {
                  Evas_Native_Surface *ens;

                  ens = im->native.data;
                  if (ens->data.wl.legacy_buffer == wl_buf)
                    return im;
               }
# endif
#endif
          }
    }
  if ((!ns) && (!im->native.data)) return im;

  eng_window_use(eng_get_ob(re));

  if (im->native.data)
    {
       if (im->native.func.free)
         im->native.func.free(im);
       glsym_evas_gl_common_image_native_disable(im);
    }

  if (!ns) return im;

  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
       pmid = pm;
       im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
              {
                 glsym_evas_gl_common_image_ref(im2);
                 glsym_evas_gl_common_image_free(im);
                 return im2;
              }
         }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = tex;
       im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_tex_hash, &texid);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
              {
                 glsym_evas_gl_common_image_ref(im2);
                 glsym_evas_gl_common_image_free(im);
                 return im2;
              }
         }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_TBM)
    {
       im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_tbm_hash, &buffer);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
             {
                glsym_evas_gl_common_image_ref(im2);
                glsym_evas_gl_common_image_free(im);
                return im2;
             }
         }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_EVASGL)
    {
       im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_evasgl_hash, &buffer);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
              {
                 glsym_evas_gl_common_image_ref(im2);
                 glsym_evas_gl_common_image_free(im);
                 return im2;
              }
         }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_WL)
     {
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
        wlid = wl_buf;
        im2 = eina_hash_find(eng_get_ob(re)->gl_context->shared->native_wl_hash, &wlid);
        if (im2 == im) return im;
        if (im2)
          {
             if((n = im2->native.data))
               {
                  glsym_evas_gl_common_image_ref(im2);
                  glsym_evas_gl_common_image_free(im);
                  return im2;
               }
          }
# endif
#endif
     }
  im2 = glsym_evas_gl_common_image_new_from_data(eng_get_ob(re)->gl_context,
                                                 im->w, im->h, NULL, im->alpha,
                                                 EVAS_COLORSPACE_ARGB8888);
  glsym_evas_gl_common_image_free(im);
  im = im2;
  if (!im) return NULL;
  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
#ifdef GL_GLES
       if (native)
         {
            if (!glsym_evas_gl_common_eglDestroyImage)
              {
                 ERR("Try eglCreateImage on EGL with no support");
                 return NULL;
              }
            n = calloc(1, sizeof(Native));
            if (n)
              {
                 EGLConfig egl_config;
                 int config_attrs[20];
                 int num_config, i = 0;
                 int yinvert = 1;

                 // assume 32bit pixmap! :)
                 config_attrs[i++] = EGL_RED_SIZE;
                 config_attrs[i++] = 8;
                 config_attrs[i++] = EGL_GREEN_SIZE;
                 config_attrs[i++] = 8;
                 config_attrs[i++] = EGL_BLUE_SIZE;
                 config_attrs[i++] = 8;
                 config_attrs[i++] = EGL_ALPHA_SIZE;
                 config_attrs[i++] = 8;
                 config_attrs[i++] = EGL_DEPTH_SIZE;
                 config_attrs[i++] = 0;
                 config_attrs[i++] = EGL_STENCIL_SIZE;
                 config_attrs[i++] = 0;
                 config_attrs[i++] = EGL_RENDERABLE_TYPE;
                 if (eng_get_ob(re)->gles3)
                   config_attrs[i++] = EGL_OPENGL_ES3_BIT_KHR;
                 else
                   config_attrs[i++] = EGL_OPENGL_ES2_BIT;
                 config_attrs[i++] = EGL_SURFACE_TYPE;
                 config_attrs[i++] = EGL_PIXMAP_BIT;
                 config_attrs[i++] = EGL_NONE;

                 if (!eglChooseConfig(eng_get_ob(re)->egl_disp, config_attrs,
                                      &egl_config, 1, &num_config))
                   {
                      int err = eglGetError();
                      ERR("eglChooseConfig() failed for pixmap %#lx, "
                          "num_config = %i with error %d", pm, num_config, err);
                      glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
                      free(n);
                      return NULL;
                   }
                 else
                   {
                      int val;
                      if (extn_have_y_inverted &&
                          eglGetConfigAttrib(eng_get_ob(re)->egl_disp, egl_config,
                                             EGL_Y_INVERTED_NOK, &val))
                            yinvert = val;
                   }

                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                 n->ns_data.x11.pixmap = pm;
                 n->ns_data.x11.visual = vis;
                 n->ns_data.x11.surface = glsym_evas_gl_common_eglCreateImage(eng_get_ob(re)->egl_disp,
                                                          EGL_NO_CONTEXT,
                                                          EGL_NATIVE_PIXMAP_KHR,
                                                          (void *)pm, NULL);

                 if ((ns->version < 4) ||
                     ((ns->version == 4) && !(ns->data.x11.multiple_buffer == 1)))
                   n->ns_data.x11.multiple_buffer = 0;
                 else
                   n->ns_data.x11.multiple_buffer = 1;

                 if (!n->ns_data.x11.surface)
                   {
                      ERR("eglCreateImage() for Pixmap %#lx failed: %#x", pm, eglGetError());
                      free(n);
                      return NULL;
                   }
                 n->ns_data.x11.config = (void *)egl_config;
                 im->native.yinvert     = yinvert;
                 im->native.loose       = 0;
                 im->native.disp        = eng_get_ob(re)->egl_disp;
                 im->native.shared      = eng_get_ob(re)->gl_context->shared;
                 im->native.data        = n;
                 im->native.func.bind   = _native_bind_cb;
                 im->native.func.unbind = _native_unbind_cb;
                 im->native.func.free   = _native_free_cb;
                 im->native.target      = GL_TEXTURE_2D;
                 im->native.mipmap      = 0;
                 eina_hash_add(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid, im);
                 glsym_evas_gl_common_image_native_enable(im);
             }
         }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
       if (native)
         {
            int dummy;
            unsigned int w, h, depth = 32, border;
            Window wdummy;

            // fixme: round trip :(
            XGetGeometry(eng_get_ob(re)->disp, pm, &wdummy, &dummy, &dummy,
                         &w, &h, &border, &depth);
            if (depth <= 32)
              {
                 n = calloc(1, sizeof(Native));
                 if (n)
                   {
                      int pixmap_att[20], i;
                      int config_attrs[40], num = 0;
                      int tex_format = 0, tex_target = 0, yinvert = 0, mipmap = 0;
                      unsigned int target = 0;
                      GLXFBConfig *configs;

                      i = 0;
                      config_attrs[i++] = GLX_BUFFER_SIZE;
                      config_attrs[i++] = depth;
                      if (depth == 32)
                        {
                           config_attrs[i++] = GLX_BIND_TO_TEXTURE_RGBA_EXT;
                           config_attrs[i++] = 1;
                        }
                      else
                        {
                           config_attrs[i++] = GLX_BIND_TO_TEXTURE_RGB_EXT;
                           config_attrs[i++] = 1;
                        }

#ifndef GLX_VISUAL_ID
# define GLX_VISUAL_ID 0x800b
#endif
                      config_attrs[i++] = GLX_VISUAL_ID;
                      config_attrs[i++] = XVisualIDFromVisual(vis);
#ifndef GLX_SAMPLE_BUFFERS
# define GLX_SAMPLE_BUFFERS 0x186a0
#endif
                      config_attrs[i++] = GLX_SAMPLE_BUFFERS;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_DEPTH_SIZE;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_STENCIL_SIZE;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_AUX_BUFFERS;
                      config_attrs[i++] = 0;
                      config_attrs[i++] = GLX_STEREO;
                      config_attrs[i++] = 0;

                      config_attrs[i++] = 0;

                      configs = glXChooseFBConfig(eng_get_ob(re)->disp,
                                                  eng_get_ob(re)->screen,
                                                  config_attrs,
                                                  &num);
                      if (configs)
                        {
                           int j = 0, val = 0, found = 0;

                           try_again:
                           for (j = 0; j < num; j++)
                             {
                                if (found == 0)
                                  {
                                     XVisualInfo *vi;

                                     vi = glXGetVisualFromFBConfig(eng_get_ob(re)->disp, configs[j]);
                                     if (!vi) continue;
                                     if (vi->depth != (int)depth) continue;
                                     XFree(vi);

                                     glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                          GLX_BUFFER_SIZE, &val);
                                     if (val != (int) depth) continue;
                                  }
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_DRAWABLE_TYPE, &val);
                                if (!(val & GLX_PIXMAP_BIT)) continue;
                                tex_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_ALPHA_SIZE, &val);
                                if ((depth == 32) && (!val)) continue;
                                if (val > 0)
                                  {
                                     glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                          GLX_BIND_TO_TEXTURE_RGBA_EXT, &val);
                                     if (val) tex_format = GLX_TEXTURE_FORMAT_RGBA_EXT;
                                  }
                                else
                                  {
                                     glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                          GLX_BIND_TO_TEXTURE_RGB_EXT, &val);
                                     if (val) tex_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                                  }
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_Y_INVERTED_EXT, &val);
                                if (val) yinvert = 1;
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                                                     &val);
                                tex_target = val;
                                glXGetFBConfigAttrib(eng_get_ob(re)->disp, configs[j],
                                                     GLX_BIND_TO_MIPMAP_TEXTURE_EXT, &val);
                                mipmap = val;
                                n->ns_data.x11.config = configs[j];
                                found = 1;
                                break;
                             }
                           if (found == 0)
                             {
                                found = -1;
                                goto try_again;
                             }
                           XFree(configs);
                        }

                      eina_hash_add(eng_get_ob(re)->gl_context->shared->native_pm_hash, &pmid, im);
                      if ((tex_target & GLX_TEXTURE_2D_BIT_EXT))
                        target = GLX_TEXTURE_2D_EXT;
                      else if ((tex_target & GLX_TEXTURE_RECTANGLE_BIT_EXT))
                        {
                           ERR("rect!!! (not handled)");
                           target = GLX_TEXTURE_RECTANGLE_EXT;
                        }
                      if (!target)
                        {
                           ERR("broken tex-from-pixmap");
                           if (!(tex_target & GLX_TEXTURE_2D_BIT_EXT))
                             target = GLX_TEXTURE_RECTANGLE_EXT;
                           else if (!(tex_target & GLX_TEXTURE_RECTANGLE_BIT_EXT))
                             target = GLX_TEXTURE_2D_EXT;
                        }

                      i = 0;
                      pixmap_att[i++] = GLX_TEXTURE_FORMAT_EXT;
                      pixmap_att[i++] = tex_format;
                      pixmap_att[i++] = GLX_MIPMAP_TEXTURE_EXT;
                      pixmap_att[i++] = mipmap;
                      if (target)
                        {
                           pixmap_att[i++] = GLX_TEXTURE_TARGET_EXT;
                           pixmap_att[i++] = target;
                        }
                      pixmap_att[i++] = 0;

                      memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                      n->ns_data.x11.pixmap = pm;
                      n->ns_data.x11.visual = vis;
                      if (glsym_glXCreatePixmap)
                        n->ns_data.x11.surface = (void *)glsym_glXCreatePixmap(eng_get_ob(re)->disp,
                                                                   n->ns_data.x11.config,
                                                                   n->ns_data.x11.pixmap,
                                                                   pixmap_att);
                      else
                        ERR("Try glXCreatePixmap on GLX with no support");
                      if (n->ns_data.x11.surface)
                        {
//                          printf("%p: new native texture for %x | %4i x %4i @ %2i = %p\n",
//                                  n, pm, w, h, depth, n->surface);
                           if (!target)
                             {
                                ERR("no target :(");
                                if (glsym_glXQueryDrawable)
                                  glsym_glXQueryDrawable(eng_get_ob(re)->disp,
                                                         n->ns_data.x11.pixmap,
                                                         GLX_TEXTURE_TARGET_EXT,
                                                         &target);
                             }
                           if (target == GLX_TEXTURE_2D_EXT)
                             {
                                im->native.target = GL_TEXTURE_2D;
                                im->native.mipmap = mipmap;
                             }
#  ifdef GL_TEXTURE_RECTANGLE_ARB
                           else if (target == GLX_TEXTURE_RECTANGLE_EXT)
                             {
                                im->native.target = GL_TEXTURE_RECTANGLE_ARB;
                                im->native.mipmap = 0;
                             }
#  endif
                           else
                             {
                                im->native.target = GL_TEXTURE_2D;
                                im->native.mipmap = 0;
                                ERR("still unknown target");
                             }
                        }
                      else
                        ERR("GLX Pixmap create fail");
                      im->native.yinvert     = yinvert;
                      im->native.loose       = eng_get_ob(re)->detected.loose_binding;
                      im->native.disp        = eng_get_ob(re)->disp;
                      im->native.shared      = eng_get_ob(re)->gl_context->shared;
                      im->native.data        = n;
                      im->native.func.bind   = _native_bind_cb;
                      im->native.func.unbind = _native_unbind_cb;
                      im->native.func.free   = _native_free_cb;

                      glsym_evas_gl_common_image_native_enable(im);
                   }
              }
         }
# endif
#endif
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       if (native)
         {
            n = calloc(1, sizeof(Native));
            if (n)
              {
                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));

                 eina_hash_add(eng_get_ob(re)->gl_context->shared->native_tex_hash, &texid, im);

                 n->ns_data.opengl.surface = 0;

                 im->native.yinvert     = 0;
                 im->native.loose       = 0;
#ifdef GL_GLES
                 im->native.disp        = eng_get_ob(re)->egl_disp;
#else
                 im->native.disp        = eng_get_ob(re)->disp;
#endif
                 im->native.shared      = eng_get_ob(re)->gl_context->shared;
                 im->native.data        = n;
                 im->native.func.bind   = _native_bind_cb;
                 im->native.func.unbind = _native_unbind_cb;
                 im->native.func.free   = _native_free_cb;
                 im->native.target      = GL_TEXTURE_2D;
                 im->native.mipmap      = 0;

                 // FIXME: need to implement mapping sub texture regions
                 // x, y, w, h for possible texture atlasing

                 glsym_evas_gl_common_image_native_enable(im);
              }
         }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_TBM)
    {
#ifdef GL_GLES
       if (native)
         {
            n = calloc(1, sizeof(Native));
            if (n)
              {
                 eina_hash_add(eng_get_ob(re)->gl_context->shared->native_tbm_hash, &buffer, im);

                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                 n->ns_data.tbm.buffer = buffer;
                 if (glsym_evas_gl_common_eglDestroyImage)
                   n->ns_data.tbm.surface =
                     glsym_evas_gl_common_eglCreateImage(eng_get_ob(re)->egl_disp,
                                                         EGL_NO_CONTEXT,
                                                         EGL_NATIVE_SURFACE_TIZEN,
                                                         (void *)buffer,
                                                         NULL);
                 else
                   ERR("Try eglCreateImage on EGL with no support");
                 if (!n->ns_data.tbm.surface)
                   ERR("eglCreateImage() for %p failed", buffer);
                 im->native.yinvert     = 1;
                 im->native.loose       = 0;
                 im->native.disp        = eng_get_ob(re)->egl_disp;
                 im->native.shared      = eng_get_ob(re)->gl_context->shared;
                 im->native.data        = n;
                 im->native.func.bind   = _native_bind_cb;
                 im->native.func.unbind = _native_unbind_cb;
                 im->native.func.free   = _native_free_cb;
                 im->native.target      = GL_TEXTURE_EXTERNAL_OES;
                 im->native.mipmap      = 0;
                 glsym_evas_gl_common_image_native_enable(im);
              }
         }
#endif
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_EVASGL)
    {
       if (native)
         {
            n = calloc(1, sizeof(Native));
            if (n)
              {
                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));

                 eina_hash_add(eng_get_ob(re)->gl_context->shared->native_evasgl_hash, &buffer, im);

                 n->ns_data.evasgl.surface = ns->data.evasgl.surface;

                 im->native.yinvert      = 0;
                 im->native.loose        = 0;
#ifdef GL_GLES
                 im->native.disp        = eng_get_ob(re)->egl_disp;
#else
                 im->native.disp        = eng_get_ob(re)->disp;
#endif
                 im->native.shared      = eng_get_ob(re)->gl_context->shared;
                 im->native.data         = n;
                 im->native.func.bind    = _native_bind_cb;
                 im->native.func.unbind  = _native_unbind_cb;
                 im->native.func.free    = _native_free_cb;
                 im->native.func.yinvert = _native_yinvert_cb;
                 im->native.target       = GL_TEXTURE_2D;
                 im->native.mipmap       = 0;

                 // FIXME: need to implement mapping sub texture regions
                 // x, y, w, h for possible texture atlasing

                 glsym_evas_gl_common_image_native_enable(im);
              }
         }
    }
   else if (ns->type == EVAS_NATIVE_SURFACE_WL)
     {
#ifdef GL_GLES
# ifdef HAVE_WAYLAND
        if (native)
          {
             if ((n = calloc(1, sizeof(Native))))
               {
                  EGLAttrib attribs[3];
                  int format, yinvert = 1;

                  glsym_eglQueryWaylandBufferWL(eng_get_ob(re)->egl_disp, wl_buf,
                                                EGL_TEXTURE_FORMAT, &format);
                  if ((format != EGL_TEXTURE_RGB) &&
                      (format != EGL_TEXTURE_RGBA))
                    {
                       ERR("eglQueryWaylandBufferWL() %d format is not supported ", format);
                       glsym_evas_gl_common_image_free(im);
                       free(n);
                       return NULL;
                    }

#  ifndef EGL_WAYLAND_PLANE_WL
#   define EGL_WAYLAND_PLANE_WL 0x31D6
#  endif
#  ifndef EGL_WAYLAND_BUFFER_WL
#   define EGL_WAYLAND_BUFFER_WL 0x31D5
#  endif
                  attribs[0] = EGL_WAYLAND_PLANE_WL;
                  attribs[1] = 0; //if plane is 1 then 0, if plane is 2 then 1
                  attribs[2] = EGL_NONE;

                  memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                  if (glsym_eglQueryWaylandBufferWL(eng_get_ob(re)->egl_disp, wl_buf,
                                                    EGL_WAYLAND_Y_INVERTED_WL,
                                                    &yinvert) == EGL_FALSE)
                    yinvert = 1;
                  eina_hash_add(eng_get_ob(re)->gl_context->shared->native_wl_hash,
                                &wlid, im);

                  n->ns_data.wl_surface.wl_buf = wl_buf;
                  if (glsym_evas_gl_common_eglDestroyImage)
                    n->ns_data.wl_surface.surface =
                      glsym_evas_gl_common_eglCreateImage(eng_get_ob(re)->egl_disp,
                                                          NULL,
                                                          EGL_WAYLAND_BUFFER_WL,
                                                          wl_buf, attribs);
                  else
                    {
                       ERR("Try eglCreateImage on EGL with no support");
                       eina_hash_del(eng_get_ob(re)->gl_context->shared->native_wl_hash,
                                     &wlid, im);
                       glsym_evas_gl_common_image_free(im);
                       free(n);
                       return NULL;
                    }

                  if (!n->ns_data.wl_surface.surface)
                    {
                       ERR("eglCreatePixmapSurface() for %p failed", wl_buf);
                       eina_hash_del(eng_get_ob(re)->gl_context->shared->native_wl_hash,
                                     &wlid, im);
                       glsym_evas_gl_common_image_free(im);
                       free(n);
                       return NULL;
                    }

                  //XXX: workaround for mesa-10.2.8
                  // mesa's eglQueryWaylandBufferWL() with EGL_WAYLAND_Y_INVERTED_WL works incorrect.
                  //im->native.yinvert = yinvert;
                  im->native.yinvert = 1;
                  im->native.loose = 0;
                  im->native.disp        = eng_get_ob(re)->egl_disp;
                  im->native.shared      = eng_get_ob(re)->gl_context->shared;
                  im->native.data = n;
                  im->native.func.bind = _native_bind_cb;
                  im->native.func.unbind = _native_unbind_cb;
                  im->native.func.free = _native_free_cb;
                  im->native.target = GL_TEXTURE_2D;
                  im->native.mipmap = 0;

                  glsym_evas_gl_common_image_native_enable(im);
               }
          }
# endif
#endif
     }
   return im;
}

static int
module_open(Evas_Module *em)
{
   static Eina_Bool xrm_inited = EINA_FALSE;
   const char *platform_env = NULL;
   if (!xrm_inited)
     {
        xrm_inited = EINA_TRUE;
        XrmInitialize();
     }
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic")) return 0;
   if (_evas_engine_GL_X11_log_dom < 0)
     _evas_engine_GL_X11_log_dom = eina_log_domain_register
       ("evas-gl_x11", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_X11_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   if (partial_render_debug == -1)
     {
        if (getenv("EVAS_GL_PARTIAL_DEBUG")) partial_render_debug = 1;
        else partial_render_debug = 0;
     }
//   partial_render_debug = 1;

   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(update);
   ORD(canvas_alpha_get);
   ORD(output_free);
   ORD(output_dump);

   ORD(image_native_init);
   ORD(image_native_shutdown);
   ORD(image_native_set);

   ORD(gl_error_get);
   // gl_current_surface_get is in gl generic
   ORD(gl_current_context_get);

   if (!(platform_env = getenv("EGL_PLATFORM")))
      setenv("EGL_PLATFORM", "x11", 0);

   gl_symbols();

   if (!platform_env)
      unsetenv("EGL_PLATFORM");

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_engine_GL_X11_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
        _evas_engine_GL_X11_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_x11",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_x11);

#ifndef EVAS_STATIC_BUILD_GL_XLIB
EVAS_EINA_MODULE_DEFINE(engine, gl_x11);
#endif

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
