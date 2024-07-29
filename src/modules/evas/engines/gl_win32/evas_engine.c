#include <windows.h>

#include <evil_private.h> /* dlsym */

#include "evas_common_private.h" /* Also includes international specific stuff */
#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"
#include "../software_generic/evas_native_common.h"

/*
 * FIXME
 * - native surface in Windows: EVAS_NATIVE_SURFACE_GDI or EVAS_NATIVE_SURFACE_WIN32 ?
 */

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Render_Output_GL_Generic generic;
   HINSTANCE instance;
   char *class_name;
};

typedef void          (*_eng_fn) (void);
typedef _eng_fn       (*glsym_func_eng_fn) ();
typedef void          (*glsym_func_void) ();
typedef void         *(*glsym_func_void_ptr) ();
typedef int           (*glsym_func_int) ();
typedef unsigned int  (*glsym_func_uint) ();
typedef const char   *(*glsym_func_const_char_ptr) ();

const char *debug_dir;
int swap_buffer_debug_mode = -1;
int swap_buffer_debug = 0;
int partial_render_debug = -1;
int extn_have_buffer_age = 1;

static int initted = 0;
static int gl_wins = 0;
static int extn_have_y_inverted = 1;

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

_eng_fn      (*glsym_eglGetProcAddress)            (const char *a) = NULL;
EGLImageKHR  (*glsym_evas_gl_common_eglCreateImage)(EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const EGLAttrib *e) = NULL;
int          (*glsym_evas_gl_common_eglDestroyImage) (EGLDisplay a, void *b) = NULL;
void         (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
unsigned int (*glsym_eglSwapBuffersWithDamage) (EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;
unsigned int (*glsym_eglSetDamageRegionKHR)  (EGLDisplay a, EGLSurface b, EGLint *c, EGLint d) = NULL;
unsigned int (*glsym_eglQueryWaylandBufferWL)(EGLDisplay a, /*struct wl_resource */void *b, EGLint c, EGLint *d) = NULL;

static inline Outbuf *
eng_get_ob(Render_Engine *re)
{
   return re->generic.software.ob;
}

static LRESULT CALLBACK
_procedure(HWND   window,
           UINT   message,
           WPARAM window_param,
           LPARAM data_param)
{
   switch(message)
     {
        case WM_CLOSE:
          DestroyWindow(window);
          return 0;
        /* case WM_PAINT: */
        /* { */
        /*     RECT r; */

        /*     if (GetUpdateRect(window, &r, FALSE)) */
        /*     { */
        /*         PAINTSTRUCT ps; */

        /*         if (BeginPaint(window, &ps)) */
        /*         { */
        /*             LONG_PTR ptr = GetWindowLongPtr(window, GWLP_USERDATA); */
        /*             win *w = (win *)ptr; */

        /*             w->data_paste(); */

        /*             EndPaint(window, &ps); */
        /*         } */
        /*     } */
        /*     return 0; */
        /* } */
        default:
          return DefWindowProc(window, message, window_param, data_param);
    }
}

/************* Evas GL Engine Funtions Overloading *************/

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

/************* EVGL interface *************/

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

   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->egl_disp;
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

   if (eng_get_ob(re))
      return (void*)eng_get_ob(re)->egl_surface;
   else
      return NULL;
}

static void *
evgl_eng_native_window_create(void *data)
{
   Render_Engine *re = (Render_Engine *)data;
   HWND win;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

   win = CreateWindowEx(WS_EX_TOOLWINDOW,
                        re->class_name, NULL,
                        WS_VISIBLE | WS_POPUP,
                        -20, -20, 1, 1,
                        eng_get_ob(re)->window, NULL,
                        re->instance, NULL);
   if (!win)
     {
        ERR("Creating native X window failed.");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_DISPLAY);
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
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if (!native_window)
     {
        ERR("Inavlid native surface.");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_NATIVE_WINDOW);
        return 0;
     }

   DestroyWindow((HWND)native_window);

   native_window = NULL;

   return 1;
}

// Theoretically, we wouldn't need this function if the surfaceless context
// is supported. But, until then...
static void *
evgl_eng_window_surface_create(void *data, void *native_window EINA_UNUSED)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

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
}

static int
evgl_eng_window_surface_destroy(void *data, void *surface)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if (!surface)
     {
        ERR("Invalid surface.");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_SURFACE);
        return 0;
     }

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)surface);

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
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

   if ((version < EVAS_GL_GLES_1_X) || (version > EVAS_GL_GLES_3_X))
     {
        ERR("Invalid context version number %d", version);
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   if ((version == EVAS_GL_GLES_3_X) &&
       ((!eng_get_ob(re)->gl_context) || (eng_get_ob(re)->gl_context->gles_version != EVAS_GL_GLES_3_X)))
     {
        ERR("GLES 3 version not supported!");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_ATTRIBUTE);
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
   if (gles3_supported && (version >= EVAS_GL_GLES_2_X))
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
        glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
        return NULL;
     }

   return (void*)context;
}

static int
evgl_eng_context_destroy(void *data, void *context)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if ((!re) || (!context))
     {
        ERR("Invalid Render Input Data. Engine: %p, Context: %p", data, context);
        if (!re) glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        if (!context) glsym_evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
        return 0;
     }

   eglDestroyContext(eng_get_ob(re)->egl_disp, (EGLContext)context);

   return 1;
}

static int
evgl_eng_make_current(void *data, void *surface, void *context, int flush)
{
   Render_Engine *re = (Render_Engine *)data;
   int ret = 0;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

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
}

static void *
evgl_eng_proc_address_get(const char *name)
{
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
}

static const char *
evgl_eng_string_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, NULL); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

   return eglQueryString(eng_get_ob(re)->egl_disp, EGL_EXTENSIONS);
}

static int
evgl_eng_rotation_angle_get(void *data)
{
   Render_Engine *re = (Render_Engine *)data;

   /* EVGLINIT(re, 0); */
   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if ((eng_get_ob(re)) && (eng_get_ob(re)->gl_context))
     return eng_get_ob(re)->gl_context->rot;
   else
     {
        ERR("Unable to retrieve rotation angle.");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
        return 0;
     }
}

static void *
evgl_eng_pbuffer_surface_create(void *data, EVGL_Surface *sfc,
                                const int *attrib_list)
{
   Render_Output_GL_Generic *re = data;

   // TODO: Add support for surfaceless pbuffers (EGL_NO_TEXTURE)
   // TODO: Add support for EGL_MIPMAP_TEXTURE??? (GLX doesn't support them)

   if (attrib_list)
     WRN("This PBuffer implementation does not support extra attributes yet");

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
   if (gles3_supported)
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
        glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
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
        glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
        ERR("eglCreatePbufferSurface failed with error %x", err);
        return NULL;
     }

   return egl_sfc;
}

static int
evgl_eng_pbuffer_surface_destroy(void *data, void *surface)
{
   Render_Engine *re = data;

   /* EVGLINIT(re, 0); */
   if (!data)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if (!surface)
     {
        ERR("Invalid surface.");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_SURFACE);
        return 0;
     }

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)surface);

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
  // FIXME: to do
#if 0
   Render_Engine *re = data;
   Eina_Bool alpha = EINA_FALSE;
   int colordepth;
   Pixmap px;

   if (!re || !evgl_sfc || !cfg)
     {
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   if ((w < 1) || (h < 1))
     {
        ERR("Inconsistent parameters, not creating any surface!");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   /* Choose appropriate pixmap depth */
   if (cfg->color_format == EVAS_GL_RGBA_8888)
     {
        alpha = EINA_TRUE;
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
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
        return NULL;
     }

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
        glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
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
        glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
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

#endif
   return 0;
   (void)data;
   (void)evgl_sfc;
   (void)cfg;
   (void)w;
   (void)h;
}

// This function should destroy the indirect surface as well as the X pixmap
static int
evgl_eng_indirect_surface_destroy(void *data, EVGL_Surface *evgl_sfc)
{
  // FIXME: to do
#if 0
   Render_Engine *re = (Render_Engine *)data;

   if (!re)
     {
        ERR("Invalid Render Engine Data!");
        glsym_evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return 0;
     }

   if ((!evgl_sfc) || (!evgl_sfc->indirect_sfc))
     {
        ERR("Invalid surface");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_SURFACE);
        return 0;
     }

   eglDestroySurface(eng_get_ob(re)->egl_disp, (EGLSurface)evgl_sfc->indirect_sfc);

   if (!evgl_sfc->indirect_sfc_native)
     {
        ERR("Inconsistent parameters, not freeing XPixmap for indirect surface!");
        glsym_evas_gl_common_error_set(EVAS_GL_BAD_PARAMETER);
        return 0;
     }

   XFreePixmap(eng_get_ob(re)->disp, (Pixmap)evgl_sfc->indirect_sfc_native);

   return 1;
#endif
   return 0;
   (void)data;
   (void)evgl_sfc;
}

static void *
evgl_eng_gles_context_create(void *data,
                              EVGL_Context *share_ctx, EVGL_Surface *sfc)
{
   Render_Engine *re = data;
   if (!re) return NULL;

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
        glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
        return NULL;
     }

   DBG("Successfully created context for indirect rendering.");
   return context;
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

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);

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

#define FINDSYM(dst, sym, ext, typ) do { \
   if (!dst) { \
      if (eina_str_has_extension(exts, ext) && glsym_eglGetProcAddress) \
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
   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", "GL_OES_EGL_image_external", glsym_func_void);
   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", "GL_OES_EGL_image", glsym_func_void);

#undef FINDSYM

   done = 1;
}

static void
gl_extn_veto(Render_Engine *re)
{
   const char *str = NULL;

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
        if ((!strstr(str, "EGL_EXT_swap_buffers_with_damage")) &&
            (!strstr(str, "EGL_KHR_swap_buffers_with_damage")))
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
}

Eina_Bool
eng_preload_make_current(void *data, void *doit)
{
   Outbuf *ob = data;

   if (doit)
     {
        if (!evas_eglMakeCurrent(ob->egl_disp, ob->egl_surface, ob->egl_surface, ob->egl_context))
          return EINA_FALSE;
     }
   else
     {
        if (!evas_eglMakeCurrent(ob->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }
   return EINA_TRUE;
}

/************* Evas Module Engine Funtions Overloading *************/

int _evas_engine_gl_win32_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

static void
_re_winfree(Render_Engine *re)
{
   if (!eng_get_ob(re)->surf) return;
   glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));
   eng_window_unsurf(eng_get_ob(re));
}

static void
eng_outbuf_idle_flush(Outbuf *ob)
{
   if (glsym_evas_gl_common_shaders_flush)
     glsym_evas_gl_common_shaders_flush(ob->gl_context->shared);
}

static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_GL_Win32 *einfo = info;

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine, void *in, unsigned int w, unsigned int h)
{
   WNDCLASS wc;
   char buf[16];
   Evas_Engine_Info_GL_Win32 *info = in;
   Render_Engine *re = NULL;
   Outbuf *ob = NULL;
   Render_Output_Swap_Mode swap_mode;

   swap_mode = evas_render_engine_gl_swap_mode_get(info->swap_mode);

   // Set this env var to dump files every frame
   // Or set the global var in gdb to 1|0 to turn it on and off
   if (getenv("EVAS_GL_SWAP_BUFFER_DEBUG_ALWAYS"))
     swap_buffer_debug = 1;

   if (swap_buffer_debug_mode == -1)
     {
        if ((debug_dir = getenv("EVAS_GL_SWAP_BUFFER_DEBUG_DIR")))
          {
             int stat;

             stat = CreateDirectoryA(debug_dir, NULL);
             if ((!stat) || errno == EEXIST) swap_buffer_debug_mode = 1;
          }
        else
           swap_buffer_debug_mode = 0;
     }

   if (!initted)
     {
        glsym_evas_gl_preload_init();
     }

   re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;

   re->instance = GetModuleHandle(NULL);
   if (!re->instance) goto on_error;

   snprintf(buf, sizeof(buf), "%p", re);
   re->class_name = strdup(buf);
   if (!re->class_name) goto on_error;

   ZeroMemory(&wc, sizeof(wc));
   wc.lpfnWndProc = _procedure;
   wc.hInstance = re->instance;
   wc.lpszClassName = re->class_name;
   if (!RegisterClass(&wc)) goto on_error;

   ob = eng_window_new(info,
                       w, h,
                       swap_mode);
   if (!ob) goto on_error;

   if (!evas_render_engine_gl_generic_init(engine, &re->generic, ob,
                                           eng_outbuf_swap_mode,
                                           eng_outbuf_get_rot,
                                           eng_outbuf_reconfigure,
                                           eng_outbuf_region_first_rect,
                                           eng_outbuf_damage_region_set,
                                           eng_outbuf_new_region_for_update,
                                           eng_outbuf_push_updated_region,
                                           NULL,
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

   evas_render_engine_software_generic_merge_mode_set(&re->generic.software);

   if (!initted)
     {
        gl_extn_veto(re);
        //             evgl_engine_init(re, &evgl_funcs);
        initted = 1;
     }

   eng_window_use(eng_get_ob(re));

   return re;

 on_error:
   if (ob) eng_window_free(ob);
   if (re->class_name) free(re->class_name);
   if (re->instance) FreeLibrary(re->instance);
   free(re);
   return NULL;
}

static int
eng_output_update(void *engine EINA_UNUSED, void *data, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_GL_Win32 *info = in;
   Render_Engine *re = data;
   Render_Output_Swap_Mode swap_mode;

   swap_mode = evas_render_engine_gl_swap_mode_get(info->swap_mode);

   if (eng_get_ob(re) && _re_wincheck(eng_get_ob(re)))
     {
        if ((info->info.window != eng_get_ob(re)->window) ||
            (info->stencil_bits != eng_get_ob(re)->stencil_bits) ||
            (info->msaa_bits != eng_get_ob(re)->msaa_bits) ||
            (info->info.destination_alpha != eng_get_ob(re)->alpha))
          {
             Outbuf *ob;

             gl_wins--;

             ob = eng_window_new(info, w, h, swap_mode);
             if (!ob) return 0;

             eng_window_use(ob);
             evas_render_engine_software_generic_update(&re->generic.software,
                                                        ob, w, h);
             gl_wins++;
          }
        else if ((eng_get_ob(re)->w != w) ||
                 (eng_get_ob(re)->h != h) ||
                 (eng_get_ob(re)->info->info.rotation != eng_get_ob(re)->rotation))
          {
             eng_outbuf_reconfigure(eng_get_ob(re), w, h, eng_get_ob(re)->info->info.rotation, 0);
             evas_render_engine_software_generic_update(&re->generic.software,
                                                        re->generic.software.ob,
                                                        w, h);
          }
     }

   eng_window_use(eng_get_ob(re));

   return 1;
}

static Eina_Bool
eng_canvas_alpha_get(void *engine)
{
   Render_Engine *re = (Render_Engine *)engine;
   return re->generic.software.ob->alpha;
}

static void
eng_output_free(void *engine, void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re)
     {
        glsym_evas_gl_preload_render_relax(eng_preload_make_current, eng_get_ob(re));

#if 0
        // Destroy the resource surface
        // Only required for EGL case
        if (re->surface)
           eglDestroySurface(eng_get_ob(re)->egl_disp, re->surface);

        // Destroy the resource context
        _destroy_internal_context(re, context);
#endif

        if (gl_wins == 1) glsym_evgl_engine_shutdown(re);

        evas_render_engine_software_generic_clean(engine, &re->generic.software);

        gl_wins--;

        UnregisterClass(re->class_name, re->instance);
        free(re->class_name);
        FreeLibrary(re->instance);
        free(re);
     }
   if ((initted == 1) && (gl_wins == 0))
     {
        glsym_evas_gl_preload_shutdown();
        initted = 0;
     }
}

static void
eng_output_dump(void *engine, void *data)
{
   Render_Engine *re = data;
   Render_Engine_GL_Generic *e = engine;

   eng_window_use(eng_get_ob(re));
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
      case EVAS_NATIVE_SURFACE_TBM:
        return _evas_native_tbm_init();
      case EVAS_NATIVE_SURFACE_OPENGL:
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
      case EVAS_NATIVE_SURFACE_OPENGL:
      case EVAS_NATIVE_SURFACE_EVASGL:
        return;
      default:
        ERR("Native surface type %d not supported!", type);
        return;
     }
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
#if 0
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
       if (n->ns_data.x11.surface)
         {
            if ((n->frame_cnt != im->gc->frame_cnt) &&
                (n->ns_data.x11.multiple_buffer))
              {
                 EGLint err;

                 if (!glsym_evas_gl_common_eglDestroyImage)
                   {
                      ERR("Try eglDestroyImage()/eglCreateImage() on EGL with no support");
                      return;
                   }
                 n->frame_cnt = im->gc->frame_cnt;
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
                   WRN("eglCreateImage() for Pixmap 0x%#lx failed: %#x", n->ns_data.x11.pixmap, eglGetError());
              }
            if (glsym_glEGLImageTargetTexture2DOES)
              {
                 glsym_glEGLImageTargetTexture2DOES(im->native.target, n->ns_data.x11.surface);
                 GLERRV("glsym_glEGLImageTargetTexture2DOES");
              }
            else
              ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
         }
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       glBindTexture(im->native.target, n->ns.data.opengl.texture_id);
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_TBM)
    {
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
                 if (glsym_glEGLImageTargetTexture2DOES)
                   {
                      glsym_glEGLImageTargetTexture2DOES(im->native.target, surface);
                      GLERRV("glsym_glEGLImageTargetTexture2DOES");
                   }
                 else
                   ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
              }
            else
              {
                 glBindTexture(GL_TEXTURE_2D, (GLuint)(uintptr_t)surface);
              }
         }
    }

#endif
  (void)image;
}

static void
_native_unbind_cb(void *image)
{
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

   if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
     {
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
     }
}

static void
_native_free_cb(void *image)
{
  Evas_GL_Image *im = image;
  Native *n = im->native.data;
  uint32_t pmid, texid;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
       pmid = n->ns_data.x11.pixmap;
       eina_hash_del(im->native.shared->native_pm_hash, &pmid, im);
       if (n->ns_data.x11.surface)
         {
            int err;
            if (glsym_evas_gl_common_eglDestroyImage)
              {
                 glsym_evas_gl_common_eglDestroyImage(im->native.disp,
                                                      n->ns_data.x11.surface);
                 n->ns_data.x11.surface = 0;
                 if ((err = eglGetError()) != EGL_SUCCESS)
                   {
                      ERR("eglDestroyImage() failed.");
                      glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
                   }
              }
            else
              ERR("Try eglDestroyImage on EGL with no support");
         }
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = n->ns.data.opengl.texture_id;
       eina_hash_del(im->native.shared->native_tex_hash, &texid, im);
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_TBM)
    {
       eina_hash_del(im->native.shared->native_tbm_hash, &n->ns_data.tbm.buffer, im);
       if (n->ns_data.tbm.surface)
         {
            int err;
            if (glsym_evas_gl_common_eglDestroyImage)
              {
                 glsym_evas_gl_common_eglDestroyImage(im->native.disp,
                                                      n->ns_data.tbm.surface);
                 n->ns_data.tbm.surface = 0;
                 if ((err = eglGetError()) != EGL_SUCCESS)
                   {
                      ERR("eglDestroyImage() failed.");
                      glsym_evas_gl_common_error_set(err - EGL_SUCCESS);
                   }
              }
            else
              ERR("Try eglDestroyImage on EGL with no support");
         }
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_EVASGL)
    {
       eina_hash_del(im->native.shared->native_evasgl_hash, &n->ns.data.evasgl.surface, im);
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
        if (extn_have_y_inverted &&
            eglGetConfigAttrib(im->native.disp, n->ns_data.x11.config,
                               EGL_Y_INVERTED_NOK, &val))
          yinvert = val;
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

static void *
eng_image_native_set(void *engine, void *image, void *native)
{
  const Evas_Native_Surface *ns = native;
  Evas_Engine_GL_Context *gl_context;
  Evas_GL_Image *im = image, *im2 = NULL;
#if 0
  Visual *vis = NULL;
  Pixmap pm = 0;
  uint32_t pmid;
#endif
  Native *n = NULL;
  uint32_t texid;
  unsigned int tex = 0;
  unsigned int fbo = 0;
  void *buffer = NULL;
  Outbuf *ob;

  gl_context = gl_generic_context_find(engine, 1);
  ob = gl_generic_any_output_get(engine);
  if (!im)
    {
       if ((ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
         {
            im = glsym_evas_gl_common_image_new_from_data(gl_context,
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
       if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
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
    }
   if (!ns)
     {
        glsym_evas_gl_common_image_free(im);
        return NULL;
     }


  if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = tex;
       im2 = eina_hash_find(gl_context->shared->native_tex_hash, &texid);
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
       im2 = eina_hash_find(gl_context->shared->native_tbm_hash, &buffer);
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
       im2 = eina_hash_find(gl_context->shared->native_evasgl_hash, &buffer);
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

  im2 = glsym_evas_gl_common_image_new_from_data(gl_context,
                                                 im->w, im->h, NULL, im->alpha,
                                                 EVAS_COLORSPACE_ARGB8888);
  glsym_evas_gl_common_image_free(im);
  im = im2;
  if (!im) return NULL;
  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
      // FIXME to do
#if 0
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
                 if (gles3_supported)
                   config_attrs[i++] = EGL_OPENGL_ES3_BIT_KHR;
                 else
                   config_attrs[i++] = EGL_OPENGL_ES2_BIT;
                 config_attrs[i++] = EGL_SURFACE_TYPE;
                 config_attrs[i++] = EGL_PIXMAP_BIT;
                 config_attrs[i++] = EGL_NONE;

                 if (!eglChooseConfig(ob->egl_disp, config_attrs,
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
                          eglGetConfigAttrib(ob->egl_disp, egl_config,
                                             EGL_Y_INVERTED_NOK, &val))
                            yinvert = val;
                   }

                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                 n->ns_data.x11.pixmap = pm;
                 n->ns_data.x11.visual = vis;
                 n->ns_data.x11.surface = glsym_evas_gl_common_eglCreateImage(ob->egl_disp,
                                                          EGL_NO_CONTEXT,
                                                          EGL_NATIVE_PIXMAP_KHR,
                                                          (void *)pm, NULL);

                 if ((ns->version < 4) ||
                     ((ns->version == 4) && !(ns->data.x11.multiple_buffer == 1)))
                   n->ns_data.x11.multiple_buffer = 0;
                 else
                   n->ns_data.x11.multiple_buffer = 1;
                 if (ob->detected.no_multi_buffer_native)
                   n->ns_data.x11.multiple_buffer = 0;

                 if (!n->ns_data.x11.surface)
                   {
                      WRN("eglCreateImage() for Pixmap %#lx failed: %#x", pm, eglGetError());
                      free(n);
                      return NULL;
                   }
                 n->ns_data.x11.config = (void *)egl_config;
                 im->native.yinvert     = yinvert;
                 im->native.loose       = 0;
                 im->native.disp        = ob->egl_disp;
                 im->native.shared      = gl_context->shared;
                 im->native.data        = n;
                 im->native.func.bind   = _native_bind_cb;
                 im->native.func.unbind = _native_unbind_cb;
                 im->native.func.free   = _native_free_cb;
                 im->native.target      = GL_TEXTURE_2D;
                 im->native.mipmap      = 0;
                 eina_hash_add(ob->gl_context->shared->native_pm_hash, &pmid, im);
                 glsym_evas_gl_common_image_native_enable(im);
             }
         }
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

                 eina_hash_add(gl_context->shared->native_tex_hash, &texid, im);

                 n->ns_data.opengl.surface = 0;

                 im->native.yinvert     = 0;
                 im->native.loose       = 0;
                 im->native.disp        = ob->egl_disp;
                 im->native.shared      = gl_context->shared;
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
       if (native)
         {
            n = calloc(1, sizeof(Native));
            if (n)
              {
                 eina_hash_add(gl_context->shared->native_tbm_hash, &buffer, im);

                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
                 n->ns_data.tbm.buffer = buffer;
                 if (glsym_evas_gl_common_eglDestroyImage)
                   n->ns_data.tbm.surface =
                     glsym_evas_gl_common_eglCreateImage(ob->egl_disp,
                                                         EGL_NO_CONTEXT,
                                                         EGL_NATIVE_SURFACE_TIZEN,
                                                         (void *)buffer,
                                                         NULL);
                 else
                   ERR("Try eglCreateImage on EGL with no support");
                 if (!n->ns_data.tbm.surface)
                   WRN("eglCreateImage() for %p failed", buffer);
                 im->native.yinvert     = 1;
                 im->native.loose       = 0;
                 im->native.disp        = ob->egl_disp;
                 im->native.shared      = gl_context->shared;
                 im->native.data        = n;
                 im->native.func.bind   = _native_bind_cb;
                 im->native.func.unbind = _native_unbind_cb;
                 im->native.func.free   = _native_free_cb;
                 im->native.target      = GL_TEXTURE_EXTERNAL_OES;
                 im->native.mipmap      = 0;
                 glsym_evas_gl_common_image_native_enable(im);
              }
         }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_EVASGL)
    {
       if (native)
         {
            n = calloc(1, sizeof(Native));
            if (n)
              {
                 memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));

                 eina_hash_add(gl_context->shared->native_evasgl_hash, &buffer, im);

                 n->ns_data.evasgl.surface = ns->data.evasgl.surface;

                 im->native.yinvert      = 0;
                 im->native.loose        = 0;
#ifdef GL_GLES
                 im->native.disp        = ob->egl_disp;
#else
                 im->native.disp        = ob->disp;
#endif
                 im->native.shared      = gl_context->shared;
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

   return im;
}

static int
eng_gl_error_get(void *engine)
{
   int err;

   if ((err = glsym_evas_gl_common_error_get(engine)) != EVAS_GL_SUCCESS)
     goto end;

   err = eglGetError() - EGL_SUCCESS;

end:
   glsym_evas_gl_common_error_set(EVAS_GL_SUCCESS);
   return err;
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

   if (evas_eglGetCurrentContext() == context)
     return ctx;

   return NULL;
}

static int
module_open(Evas_Module *em)
{
   const char *egl_platform_env = NULL;
   const char *egl_log_level_env = NULL;

   if (!em) return 0;

   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "gl_generic", sizeof (Evas_Engine_Info_GL_Win32))) return 0;

   if (_evas_engine_gl_win32_log_dom < 0)
     _evas_engine_gl_win32_log_dom = eina_log_domain_register
       ("evas-gl_win32", EVAS_DEFAULT_LOG_COLOR);

   if (_evas_engine_gl_win32_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   if (partial_render_debug == -1)
     {
        if (getenv("EVAS_GL_PARTIAL_DEBUG")) partial_render_debug = 1;
        else partial_render_debug = 0;
     }

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

   ORD(image_native_init);
   ORD(image_native_shutdown);
   ORD(image_native_set);

   ORD(gl_error_get);
   // gl_current_surface_get is in gl generic
   ORD(gl_current_context_get);

   /* FIXME: should be set to what on Windows ? */
   if (!(egl_platform_env = getenv("EGL_PLATFORM")))
     SetEnvironmentVariable("EGL_PLATFORM", "angle");

   /*
    * FIXME: set to something else when release, but used in Angle
    * possible values: debug, info, warning, and fatal
    */
   if (!(egl_log_level_env = getenv("EGL_LOG_LEVEL")))
     SetEnvironmentVariable("EGL_LOG_LEVEL", "debug");

   gl_symbols();

   if (!egl_log_level_env)
     SetEnvironmentVariable("EGL_LOG_LEVEL", NULL);

   if (!egl_platform_env)
     SetEnvironmentVariable("EGL_PLATFORM", NULL);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_engine_gl_win32_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_gl_win32_log_dom);
        _evas_engine_gl_win32_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_win32",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_win32);

#ifndef EVAS_STATIC_BUILD_GL_WIN32
EVAS_EINA_MODULE_DEFINE(engine, gl_win32);
#endif
