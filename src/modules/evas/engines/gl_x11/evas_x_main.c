#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"
#include <dlfcn.h>

# define SET_RESTORE_CONTEXT() do { if (glsym_evas_gl_common_context_restore_set) glsym_evas_gl_common_context_restore_set(EINA_TRUE); } while(0)

static Eina_TLS _outbuf_key = 0;
static Eina_TLS _context_key = 0;

typedef void (*glsym_func_void) ();
glsym_func_void glsym_evas_gl_common_context_restore_set = NULL;

#ifdef GL_GLES
typedef EGLContext GLContext;
typedef EGLConfig GLConfig;
static int gles3_supported = -1;
#else
// FIXME: this will only work for 1 display connection (glx land can have > 1)
typedef GLXContext GLContext;
typedef GLXFBConfig GLConfig;
static Eina_TLS _rgba_context_key = 0;
#endif

typedef struct _Evas_GL_X11_Visual Evas_GL_X11_Visual;
struct _Evas_GL_X11_Visual
{
   XVisualInfo info;
   GLConfig config;
   Colormap cmap;
   Display *disp;
   Eina_Bool alpha;
};

/* index (_visuals_hash_index_get) -> Evas_GL_X11_Visual */
static Eina_Hash *_evas_gl_visuals = NULL;

static int win_count = 0;
static Eina_Bool initted = EINA_FALSE;

Eina_Bool
eng_init(void)
{
   if (initted)
     return EINA_TRUE;

#define LINK2GENERIC(sym) \
   glsym_##sym = dlsym(RTLD_DEFAULT, #sym); \
   if (!glsym_##sym) ERR("Could not find function '%s'", #sym);

   LINK2GENERIC(evas_gl_common_context_restore_set);

   // FIXME: These resources are never released
   if (!eina_tls_new(&_outbuf_key))
     goto error;
   if (!eina_tls_new(&_context_key))
     goto error;

   eina_tls_set(_outbuf_key, NULL);
   eina_tls_set(_context_key, NULL);

#ifndef GL_GLES
   if (!eina_tls_new(&_rgba_context_key))
     goto error;
   eina_tls_set(_rgba_context_key, NULL);
#endif

   initted = EINA_TRUE;
   return EINA_TRUE;

error:
   ERR("Could not create TLS key!");
   return EINA_FALSE;
}

static inline Outbuf *
_tls_outbuf_get(void)
{
   if (!initted) eng_init();
   return eina_tls_get(_outbuf_key);
}

static inline Eina_Bool
_tls_outbuf_set(Outbuf *xwin)
{
   if (!initted) eng_init();
   return eina_tls_set(_outbuf_key, xwin);
}

static inline GLContext
_tls_context_get(void)
{
   if (!initted) eng_init();
   return eina_tls_get(_context_key);
}

static inline Eina_Bool
_tls_context_set(GLContext ctx)
{
   if (!initted) eng_init();
   return eina_tls_set(_context_key, ctx);
}

#ifndef GL_GLES
static inline GLXContext
_tls_rgba_context_get(void)
{
   if (!initted) eng_init();
   return eina_tls_get(_rgba_context_key);
}

static inline Eina_Bool
_tls_rgba_context_set(GLXContext ctx)
{
   if (!initted) eng_init();
   return eina_tls_set(_rgba_context_key, ctx);
}

Eina_Bool
__glXMakeContextCurrent(Display *disp, GLXDrawable glxwin, GLXContext context)
{
   if (!glXMakeContextCurrent(disp, glxwin, glxwin, context)) return EINA_FALSE;
   return EINA_TRUE;
}
#endif

static void
_visuals_hash_del_cb(void *data)
{
   Evas_GL_X11_Visual *evis = data;
   if (evis && evis->cmap && evis->disp)
     XFreeColormap(evis->disp, evis->cmap);
   free(evis);
}

static inline int
_visuals_hash_index_get(int alpha, int zdepth, int stencil, int msaa)
{
   if (!_evas_gl_visuals)
     _evas_gl_visuals = eina_hash_int32_new(_visuals_hash_del_cb);
   return alpha | (zdepth << 8) | (stencil << 16) | (msaa << 24);
}

static inline int
_visuals_hash_index_get_from_info(Evas_Engine_Info_GL_X11 *info)
{
   if (!info) return -1;
   return _visuals_hash_index_get(info->info.destination_alpha,
                                  info->depth_bits, info->stencil_bits,
                                  info->msaa_bits);
}

#ifdef GL_GLES

static EGLDisplay *
_x11_eglGetDisplay(Display *x11_display)
{
   EGLDisplay (*eglsym_eglGetPlatformDisplay)
         (EGLenum platform, void *native_display, const EGLAttrib *attrib_list) = NULL;
   EGLDisplay *egldisp = EGL_NO_DISPLAY;

   eglsym_eglGetPlatformDisplay = dlsym(RTLD_DEFAULT, "eglGetPlatformDisplay");
   if (eglsym_eglGetPlatformDisplay)
     {
        egldisp = eglsym_eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR,
                                               (EGLNativeDisplayType) x11_display, NULL);
        if (egldisp) return egldisp;
     }

   return eglGetDisplay((EGLNativeDisplayType) x11_display);
}

#endif

Outbuf *
eng_window_new(Evas_Engine_Info_GL_X11 *info,
               Display *disp,
               Window   win,
               int      screen,
               Visual  *vis,
               Colormap cmap,
               int      depth,
               unsigned int w,
               unsigned int h,
               int      indirect EINA_UNUSED,
               int      alpha,
               int      rot,
               Render_Output_Swap_Mode swap_mode,
               int depth_bits,
               int stencil_bits,
               int msaa_bits)
{
   Outbuf *gw;
   GLContext context;
#ifdef GL_GLES
   int context_attrs[3];
   int major_version, minor_version;
#else
   GLXContext rgbactx;
   Evas_GL_X11_Visual *evis2 = NULL;
   int tmp;
#endif
   const GLubyte *vendor, *renderer, *version, *glslversion;
   int blacklist = 0;
   int val = 0, idx;
   Evas_GL_X11_Visual *evis;

   idx = _visuals_hash_index_get_from_info(info);
   evis = eina_hash_find(_evas_gl_visuals, &idx);
   if (!evis)
     {
        eng_best_visual_get(info);
        evis = eina_hash_find(_evas_gl_visuals, &idx);
        if (!evis) return NULL;
     }

   vis = evis->info.visual;
   if (!vis) return NULL;

   gw = calloc(1, sizeof(Outbuf));
   if (!gw) return NULL;

   win_count++;
   gw->disp = disp;
   gw->win = win;
   gw->screen = screen;
   gw->visual = vis;
   gw->colormap = cmap;
   gw->depth = depth;
   gw->alpha = alpha;
   gw->w = w;
   gw->h = h;
   gw->rot = rot;
   gw->swap_mode = swap_mode;
   gw->info = info;
   gw->depth_bits = depth_bits;
   gw->stencil_bits = stencil_bits;
   gw->msaa_bits = msaa_bits;
   gw->visualinfo = &evis->info;

// EGL / GLES
#ifdef GL_GLES
   gw->gles3 = gles3_supported;
   gw->egl_disp = _x11_eglGetDisplay(gw->disp);
   if (!gw->egl_disp)
     {
        ERR("eglGetDisplay() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }
   if (!eglInitialize(gw->egl_disp, &major_version, &minor_version))
     {
        ERR("eglInitialize() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }
   if (!eglBindAPI(EGL_OPENGL_ES_API))
     {
        ERR("eglBindAPI() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }

   gw->egl_config = evis->config;

   gw->egl_surface = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                               (EGLNativeWindowType)gw->win,
                                               NULL);
   if (gw->egl_surface == EGL_NO_SURFACE)
     {
        int err = eglGetError();
        printf("surf creat fail! %x\n", err);
        ERR("eglCreateWindowSurface() fail for %#x. code=%#x",
            (unsigned int)gw->win, err);
        eng_window_free(gw);
        return NULL;
     }

try_gles2:
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = gw->gles3 ? 3 : 2;
   context_attrs[2] = EGL_NONE;

   context = _tls_context_get();
   gw->egl_context = eglCreateContext
     (gw->egl_disp, gw->egl_config, context, context_attrs);
   if (gw->egl_context == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        if (gw->gles3)
          {
             /* Note: this shouldn't happen */
             ERR("Trying again with an Open GL ES 2 context (fallback).");
             gw->gles3 = EINA_FALSE;
             goto try_gles2;
          }
        eng_window_free(gw);
        return NULL;
     }
   if (context == EGL_NO_CONTEXT)
     _tls_context_set(gw->egl_context);
   
   SET_RESTORE_CONTEXT();
   if (evas_eglMakeCurrent(gw->egl_disp,
                      gw->egl_surface,
                      gw->egl_surface,
                      gw->egl_context) == EGL_FALSE)
     {
        ERR("evas_eglMakeCurrent() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }

   vendor = glGetString(GL_VENDOR);
   renderer = glGetString(GL_RENDERER);
   version = glGetString(GL_VERSION);
   glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
   if (!vendor)   vendor   = (unsigned char *)"-UNKNOWN-";
   if (!renderer) renderer = (unsigned char *)"-UNKNOWN-";
   if (!version)  version  = (unsigned char *)"-UNKNOWN-";
   if (!glslversion) glslversion = (unsigned char *)"-UNKNOWN-";
   if (getenv("EVAS_GL_INFO"))
     {
        fprintf(stderr, "vendor  : %s\n", vendor);
        fprintf(stderr, "renderer: %s\n", renderer);
        fprintf(stderr, "version : %s\n", version);
        fprintf(stderr, "glsl ver: %s\n", glslversion);
     }

   if (strstr((const char *)vendor, "Mesa Project"))
     {
        if (strstr((const char *)renderer, "Software Rasterizer"))
          blacklist = 1;
     }
   if (strstr((const char *)renderer, "softpipe"))
     blacklist = 1;
   if (strstr((const char *)renderer, "llvmpipe"))
     blacklist = 1;
   if ((blacklist) && (!getenv("EVAS_GL_NO_BLACKLIST")))
     {
        WRN("OpenGL Driver blacklisted:");
        WRN("Vendor: %s", (const char *)vendor);
        WRN("Renderer: %s", (const char *)renderer);
        WRN("Version: %s", (const char *)version);
        eng_window_free(gw);
        return NULL;
     }

   eglGetConfigAttrib(gw->egl_disp, gw->egl_config, EGL_DEPTH_SIZE, &val);
   gw->detected.depth_buffer_size = val;
   DBG("Detected depth size %d", val);
   eglGetConfigAttrib(gw->egl_disp, gw->egl_config, EGL_STENCIL_SIZE, &val);
   gw->detected.stencil_buffer_size = val;
   DBG("Detected stencil size %d", val);
   eglGetConfigAttrib(gw->egl_disp, gw->egl_config, EGL_SAMPLES, &val);
   gw->detected.msaa = val;
   DBG("Detected msaa %d", val);

// GLX
#else
   context = _tls_context_get();
   if (!context)
     {
        if (!evis->alpha)
          evis2 = evis;
        else
          {
             tmp = info->info.destination_alpha;
             info->info.destination_alpha = 0;
             evis2 = eng_best_visual_get(info);
             info->info.destination_alpha = tmp;
             if (!evis2)
               {
                  ERR("Could not find visual! (rgb only)");
                  evis2 = evis;
               }
          }
        if (indirect)
          context = glXCreateNewContext(gw->disp, evis2->config,
                                        GLX_RGBA_TYPE, NULL,
                                        GL_FALSE);
        else
          context = glXCreateNewContext(gw->disp, evis2->config,
                                        GLX_RGBA_TYPE, NULL,
                                        GL_TRUE);
        _tls_context_set(context);
     }
   rgbactx = _tls_rgba_context_get();
   if ((gw->alpha) && (!rgbactx))
     {
        if (evis->alpha)
          evis2 = evis;
        else
          {
             tmp = info->info.destination_alpha;
             info->info.destination_alpha = 1;
             evis2 = eng_best_visual_get(info);
             info->info.destination_alpha = tmp;
             if (!evis2)
               {
                  ERR("Could not find visual! (rgba)");
                  evis2 = evis;
               }
          }
        if (indirect)
          rgbactx = glXCreateNewContext(gw->disp, evis2->config,
                                        GLX_RGBA_TYPE, context,
                                        GL_FALSE);
        else
          rgbactx = glXCreateNewContext(gw->disp, evis2->config,
                                        GLX_RGBA_TYPE, context,
                                        GL_TRUE);
        _tls_rgba_context_set(rgbactx);
     }

   if (gw->alpha != info->info.destination_alpha)
     {
        tmp = info->info.destination_alpha;
        info->info.destination_alpha = gw->alpha;
        evis2 = eng_best_visual_get(info);
        info->info.destination_alpha = tmp;
        if (!evis2)
          {
             ERR("Could not find visual! (alpha: %d)", gw->alpha);
             evis2 = evis;
          }
     }
   else
     evis2 = evis;
   gw->glxwin = glXCreateWindow(gw->disp, evis2->config, gw->win, NULL);

   if (!gw->glxwin)
     {
        ERR("glXCreateWindow failed.");
        eng_window_free(gw);
        return NULL;
     }

   if (gw->alpha) gw->context = rgbactx;
   else gw->context = context;

   if (!gw->context)
     {
        ERR("Failed to create a context.");
        eng_window_free(gw);
        return NULL;
     }
   if (!__glXMakeContextCurrent(gw->disp, gw->glxwin, gw->context))
     {
        ERR("glXMakeContextCurrent(%p, %p, %p, %p)\n", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->win, (void *)gw->context);
        eng_window_free(gw);
        return NULL;
     }
   // FIXME: move this up to context creation

   vendor = glGetString(GL_VENDOR);
   renderer = glGetString(GL_RENDERER);
   version = glGetString(GL_VERSION);
   glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
   if (!vendor)   vendor   = (unsigned char *)"-UNKNOWN-";
   if (!renderer) renderer = (unsigned char *)"-UNKNOWN-";
   if (!version)  version  = (unsigned char *)"-UNKNOWN-";
   if (!glslversion) glslversion = (unsigned char *)"-UNKNOWN-";
   if (getenv("EVAS_GL_INFO"))
     {
        fprintf(stderr, "vendor  : %s\n", vendor);
        fprintf(stderr, "renderer: %s\n", renderer);
        fprintf(stderr, "version : %s\n", version);
        fprintf(stderr, "glsl ver: %s\n", glslversion);
     }

   //   examples:
   // vendor: NVIDIA Corporation
   // renderer: NVIDIA Tegra
   // version: OpenGL ES 2.0
   //   or
   // vendor: Imagination Technologies
   // renderer: PowerVR SGX 540
   // version: OpenGL ES 2.0
   //   or
   // vendor: NVIDIA Corporation
   // renderer: GeForce GT 330M/PCI/SSE2
   // version: 3.3.0 NVIDIA 256.53
   //   or
   // vendor: NVIDIA Corporation
   // renderer: GeForce GT 220/PCI/SSE2
   // version: 3.2.0 NVIDIA 195.36.24
   //   or
   // vendor: NVIDIA Corporation
   // renderer: GeForce 8600 GTS/PCI/SSE2
   // version: 3.3.0 NVIDIA 260.19.36
   //   or
   // vendor: ATI Technologies Inc.
   // renderer: ATI Mobility Radeon HD 4650
   // version: 3.2.9756 Compatibility Profile Context
   //   or
   // vendor: Tungsten Graphics, Inc
   // renderer: Mesa DRI Mobile Intel® GM45 Express Chipset GEM 20100330 DEVELOPMENT x86/MMX/SSE2
   // version: 2.1 Mesa 7.9-devel
   //   or
   // vendor: Advanced Micro Devices, Inc.
   // renderer: Mesa DRI R600 (RS780 9610) 20090101  TCL DRI2
   // version: 2.1 Mesa 7.9-devel
   //   or
   // vendor: NVIDIA Corporation
   // renderer: GeForce 9600 GT/PCI/SSE2
   // version: 3.3.0 NVIDIA 260.19.29
   //   or
   // vendor: ATI Technologies Inc.
   // renderer: ATI Radeon HD 4800 Series
   // version: 3.3.10237 Compatibility Profile Context
   //   or
   // vendor: Advanced Micro Devices, Inc.
   // renderer: Mesa DRI R600 (RV770 9442) 20090101  TCL DRI2
   // version: 2.0 Mesa 7.8.2
   //   or
   // vendor: Tungsten Graphics, Inc
   // renderer: Mesa DRI Mobile Intel® GM45 Express Chipset GEM 20100330 DEVELOPMENT
   // version: 2.1 Mesa 7.9-devel
   //   or (bad - software renderer)
   // vendor: Mesa Project
   // renderer: Software Rasterizer
   // version: 2.1 Mesa 7.9-devel
   //   or (bad - software renderer)
   // vendor: VMware, Inc.
   // renderer: Gallium 0.4 on softpipe
   // version: 2.1 Mesa 7.9-devel
   //
   if (strstr((const char *)vendor, "Mesa Project"))
     {
        if (strstr((const char *)renderer, "Software Rasterizer"))
          blacklist = 1;
     }
   if (strstr((const char *)renderer, "softpipe"))
     blacklist = 1;
   if (strstr((const char *)renderer, "llvmpipe"))
     blacklist = 1;
   if ((blacklist) && (!getenv("EVAS_GL_NO_BLACKLIST")))
     {
        WRN("OpenGL Driver blacklisted:");
        WRN("Vendor: %s", (const char *)vendor);
        WRN("Renderer: %s", (const char *)renderer);
        WRN("Version: %s", (const char *)version);
        eng_window_free(gw);
        return NULL;
     }
   if (strstr((const char *)vendor, "NVIDIA"))
     {
        if (!strstr((const char *)renderer, "NVIDIA Tegra"))
          {
             int v1 = 0, v2 = 0, v3 = 0;

             if (sscanf((const char *)version,
                        "%*s %*s %i.%i.%i",
                        &v1, &v2, &v3) != 3)
               {
                  v1 = v2 = v3 = 0;
                  if (sscanf((const char *)version,
                             "%*s %*s %i.%i",
                             &v1, &v2) != 2)
                    v1 = 0;
               }
             // ALSO as of some nvidia driver version loose binding is
             // probably not needed
             if (v1 < 195) gw->detected.loose_binding = 1;
          }
     }
   else
     {
        // noothing yet. add more cases and options over time
     }

   glXGetConfig(gw->disp, gw->visualinfo, GLX_DEPTH_SIZE, &val);
   gw->detected.depth_buffer_size = val;
   glXGetConfig(gw->disp, gw->visualinfo, GLX_STENCIL_SIZE, &val);
   gw->detected.stencil_buffer_size = val;
   glXGetConfig(gw->disp, gw->visualinfo, GLX_SAMPLES, &val);
   gw->detected.msaa = val;
#endif

   eng_gl_symbols(gw);
   gw->gl_context = glsym_evas_gl_common_context_new();
   if (!gw->gl_context)
     {
        ERR("Unable to get a new context.");
        eng_window_free(gw);
        return NULL;
     }
#ifdef GL_GLES
   gw->gl_context->egldisp = gw->egl_disp;
   gw->gl_context->eglctxt = gw->egl_context;
#else
   glXGetFBConfigAttrib(gw->disp, evis->config, GLX_FBCONFIG_ID, &gw->gl_context->glxcfg_rgb);
   glXGetFBConfigAttrib(gw->disp, evis2->config, GLX_FBCONFIG_ID, &gw->gl_context->glxcfg_rgba);
#endif
   eng_window_use(gw);
   glsym_evas_gl_common_context_resize(gw->gl_context, w, h, rot);
   gw->surf = 1;
   return gw;
}

void
eng_window_free(Outbuf *gw)
{
   Outbuf *xwin;
   GLContext context;
   int ref = 0;
   win_count--;
   eng_window_use(gw);

   if (win_count == 0) evas_common_font_ext_clear();

   context = _tls_context_get();
   xwin = _tls_outbuf_get();

   if (gw == xwin) _tls_outbuf_set(NULL);
   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        glsym_evas_gl_common_context_free(gw->gl_context);
     }
#ifdef GL_GLES
   SET_RESTORE_CONTEXT();
   evas_eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   if (gw->egl_surface != EGL_NO_SURFACE)
      eglDestroySurface(gw->egl_disp, gw->egl_surface);
   if (gw->egl_context != context)
     eglDestroyContext(gw->egl_disp, gw->egl_context);
   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eina_hash_free(_evas_gl_visuals);
        _evas_gl_visuals = NULL;
        eglTerminate(gw->egl_disp);
        eglReleaseThread();
        _tls_context_set(EGL_NO_CONTEXT);
     }
#else
   glXDestroyWindow(gw->disp, gw->glxwin);
   if (ref == 0)
     {
        GLXContext rgbactx = _tls_rgba_context_get();
        if (context) glXDestroyContext(gw->disp, context);
        if (rgbactx) glXDestroyContext(gw->disp, rgbactx);
        eina_hash_free(_evas_gl_visuals);
        _evas_gl_visuals = NULL;
        _tls_context_set(0);
        _tls_rgba_context_set(0);
     }
#endif
   free(gw);
}

static Eina_Bool
eng_window_make_current(void *data, void *doit)
{
   Outbuf *gw = data;

#ifdef GL_GLES
   SET_RESTORE_CONTEXT();
   if (doit)
     {
        if (!evas_eglMakeCurrent(gw->egl_disp, gw->egl_surface, gw->egl_surface, gw->egl_context))
          return EINA_FALSE;
     }
   else
     {
        if (!evas_eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }
#else
   if (doit)
     {
        if (!__glXMakeContextCurrent(gw->disp, gw->glxwin, gw->context))
          {
             ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->win, (void *)gw->context);
             return EINA_FALSE;
          }
     }
   else
     {
        if (!__glXMakeContextCurrent(gw->disp, 0, NULL))
          return EINA_FALSE;
     }
#endif
   return EINA_TRUE;
}

void
eng_window_use(Outbuf *gw)
{
   Eina_Bool force_use = EINA_FALSE;
   Outbuf *xwin;

   xwin = _tls_outbuf_get();

   glsym_evas_gl_preload_render_lock(eng_window_make_current, gw);
   if ((gw) && (!gw->gl_context)) return;

#ifdef GL_GLES
   if (xwin)
     {
        if ((evas_eglGetCurrentDisplay() != xwin->egl_disp) ||
            (evas_eglGetCurrentContext() != xwin->egl_context))
          force_use = EINA_TRUE;
     }
#else
   if (xwin)
     {
        if (glXGetCurrentContext() != xwin->context)
           force_use = EINA_TRUE;
     }
#endif
   if ((xwin != gw) || (force_use))
     {
        if (xwin)
          {
             glsym_evas_gl_common_context_use(xwin->gl_context);
             glsym_evas_gl_common_context_flush(xwin->gl_context);
          }
        _tls_outbuf_set(gw);
        if (gw)
          {
// EGL / GLES
#ifdef GL_GLES
             if (gw->egl_surface != EGL_NO_SURFACE)
               {
                  SET_RESTORE_CONTEXT();
                  if (evas_eglMakeCurrent(gw->egl_disp,
                                     gw->egl_surface,
                                     gw->egl_surface,
                                     gw->egl_context) == EGL_FALSE)
                    {
                       ERR("evas_eglMakeCurrent() failed!");
                    }
               }
// GLX
#else
             if (!__glXMakeContextCurrent(gw->disp, gw->glxwin, gw->context))
               {
                  ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->win, (void *)gw->context);
               }
#endif
          }
     }
   if (gw) glsym_evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_unsurf(Outbuf *gw)
{
   if (!gw->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", gw);

#ifdef GL_GLES
   Outbuf *xwin;

   xwin = _tls_outbuf_get();
   if (xwin)
      glsym_evas_gl_common_context_flush(xwin->gl_context);
   if (xwin == gw)
     {
        SET_RESTORE_CONTEXT();
        evas_eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (gw->egl_surface != EGL_NO_SURFACE)
           eglDestroySurface(gw->egl_disp, gw->egl_surface);
        gw->egl_surface = EGL_NO_SURFACE;
        _tls_outbuf_set(NULL);
     }
#else
   if (gw->glxwin)
     {
        glXDestroyWindow(gw->disp, gw->glxwin);
        gw->glxwin = 0;
     }
#endif
   gw->surf = 0;
}

void
eng_window_resurf(Outbuf *gw)
{
   if (gw->surf) return;
   if (getenv("EVAS_GL_INFO")) printf("resurf %p\n", gw);
#ifdef GL_GLES
   gw->egl_surface = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                               (EGLNativeWindowType)gw->win,
                                               NULL);
   if (gw->egl_surface == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %#x. code=%#x",
            (unsigned int)gw->win, eglGetError());
        return;
     }
   SET_RESTORE_CONTEXT();
   if (evas_eglMakeCurrent(gw->egl_disp,
                      gw->egl_surface,
                      gw->egl_surface,
                      gw->egl_context) == EGL_FALSE)
     {
        ERR("evas_eglMakeCurrent() failed!");
     }
#else
   Evas_GL_X11_Visual *evis;
   int idx = _visuals_hash_index_get(gw->alpha, gw->depth_bits, gw->stencil_bits, gw->msaa_bits);
   evis = eina_hash_find(_evas_gl_visuals, &idx);
   if (!evis)
     {
        eng_best_visual_get(gw->info);
        evis = eina_hash_find(_evas_gl_visuals, &idx);
        if (!evis)
          {
             ERR("Could not find matching visual! Resurf failed.");
             return;
          }
     }
   gw->glxwin = glXCreateWindow(gw->disp, evis->config, gw->win, NULL);
   if (!__glXMakeContextCurrent(gw->disp, gw->glxwin, gw->context))
     {
        ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->win, (void *)gw->context);
     }
#endif
   gw->surf = 1;
}

void *
eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo)
{
   Evas_GL_X11_Visual *evis;
   int alpha;
   int depth_bits;
   int stencil_bits;
   int msaa_samples;
   int config_attrs[40], i, num, idx;
   Eina_Bool found;

   if (!einfo) return NULL;
   if (!einfo->info.display) return NULL;

   alpha = einfo->info.destination_alpha;
   depth_bits = einfo->depth_bits;
   stencil_bits = einfo->stencil_bits;
   msaa_samples = einfo->msaa_bits;

   idx = _visuals_hash_index_get_from_info(einfo);
   evis = eina_hash_find(_evas_gl_visuals, &idx);
   if (evis)
     return evis->info.visual;

   evis = calloc(1, sizeof(Evas_GL_X11_Visual));
   if (!evis) return NULL;

   evis->alpha = alpha;

   // EGL / GLES
#ifdef GL_GLES
   EGLDisplay *egl_disp;
   EGLConfig configs[200];
   int major_version, minor_version;
   const char *eglexts, *s;
   int depth = DefaultDepth(einfo->info.display, einfo->info.screen);

   egl_disp = _x11_eglGetDisplay(einfo->info.display);
   if (!egl_disp)
     {
        free(evis);
        return NULL;
     }
   if (!eglInitialize(egl_disp, &major_version, &minor_version))
     {
        free(evis);
        return NULL;
     }

   /* detect GLES 3.x support */
   if (gles3_supported == -1)
     {
        gles3_supported = EINA_FALSE;
        eglexts = eglQueryString(egl_disp, EGL_EXTENSIONS);
        if (eglexts && strstr(eglexts, "EGL_KHR_create_context"))
          {
             int k, numconfigs = 0, value;
             EGLConfig *eglconfigs;

             if (eglGetConfigs(egl_disp, NULL, 0, &numconfigs) &&
                 (numconfigs > 0))
               {
                  eglconfigs = alloca(numconfigs * sizeof(EGLConfig));
                  eglGetConfigs(egl_disp, eglconfigs, numconfigs, &numconfigs);
                  for (k = 0; k < numconfigs; k++)
                    {
                       value = 0;
                       if (eglGetConfigAttrib(egl_disp, eglconfigs[k], EGL_RENDERABLE_TYPE, &value) &&
                           ((value & EGL_OPENGL_ES3_BIT_KHR) != 0) &&
                           eglGetConfigAttrib(egl_disp, eglconfigs[k], EGL_SURFACE_TYPE, &value) &&
                           ((value & EGL_WINDOW_BIT) != 0))
                         {
                            INF("OpenGL ES 3.x is supported!");
                            gles3_supported = EINA_TRUE;
                            break;
                         }
                    }
               }
          }

        if (gles3_supported &&
            ((s = getenv("EVAS_GL_DISABLE_GLES3")) && (atoi(s) == 1)))
          {
             INF("Disabling OpenGL ES 3.x support.");
             gles3_supported = EINA_FALSE;
          }
     }

   /* Find matching config & visual */
try_again:
   i = 0;
   config_attrs[i++] = EGL_SURFACE_TYPE;
   config_attrs[i++] = EGL_WINDOW_BIT;
   config_attrs[i++] = EGL_RENDERABLE_TYPE;
   if (gles3_supported)
     config_attrs[i++] = EGL_OPENGL_ES3_BIT_KHR;
   else
     config_attrs[i++] = EGL_OPENGL_ES2_BIT;
# if 0
   // FIXME: n900 - omap3 sgx libs break here
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 1;
   // FIXME: end n900 breakage
# endif
   if (alpha)
     {
        config_attrs[i++] = EGL_ALPHA_SIZE;
        config_attrs[i++] = 1;
     }
   else
     {
        config_attrs[i++] = EGL_ALPHA_SIZE;
        config_attrs[i++] = 0;
     }

   if (depth_bits)
     {
        config_attrs[i++] = EGL_DEPTH_SIZE;
        config_attrs[i++] = depth_bits;
     }

   if (stencil_bits)
     {
        config_attrs[i++] = EGL_STENCIL_SIZE;
        config_attrs[i++] = stencil_bits;
     }

   if (msaa_samples)
     {
        config_attrs[i++] = EGL_SAMPLE_BUFFERS;
        config_attrs[i++] = 1;
        config_attrs[i++] = EGL_SAMPLES;
        config_attrs[i++] = msaa_samples;
     }
   config_attrs[i++] = EGL_NONE;
   num = 0;
   if ((!eglChooseConfig(egl_disp, config_attrs, configs, 200, &num))
       || (num < 1))
     {
        ERR("eglChooseConfig() can't find any configs (gles%d, alpha: %d, depth: %d, stencil: %d, msaa: %d)",
            gles3_supported ? 3 : 2, alpha, depth_bits, stencil_bits, msaa_samples);
        if ((depth_bits > 24) || (stencil_bits > 8))
          {
             WRN("Please note that your driver might not support 32-bit depth or "
                 "16-bit stencil buffers, so depth24, stencil8 are the maximum "
                 "recommended values.");
             if (depth_bits > 24) depth_bits = 24;
             if (stencil_bits > 8) stencil_bits = 8;
             DBG("Trying again with depth:%d, stencil:%d", depth_bits, stencil_bits);
             goto try_again;
          }
        else if (msaa_samples)
          {
             msaa_samples /= 2;
             DBG("Trying again with msaa_samples: %d", msaa_samples);
             goto try_again;
          }
        else if (depth_bits || stencil_bits)
          {
             depth_bits = 0;
             stencil_bits = 0;
             DBG("Trying again without any depth or stencil buffer");
             goto try_again;
          }
        free(evis);
        return NULL;
     }
   found = EINA_FALSE;
   for (i = 0; (i < num) && (!found); i++)
     {
        EGLint val = 0;
        VisualID visid = 0;
        XVisualInfo *xvi, vi_in;
        int nvi, j;

        if (!eglGetConfigAttrib(egl_disp, configs[i],
                                EGL_NATIVE_VISUAL_ID, &val))
          continue;
        visid = val;
        vi_in.screen = einfo->info.screen;
        vi_in.visualid = visid;
        xvi = XGetVisualInfo(einfo->info.display,
                             VisualScreenMask |
                             VisualIDMask,
                             &vi_in, &nvi);
        for (j = 0; j < nvi; j++)
          {
             if (!alpha)
               {
                  if (xvi[j].depth == depth)
                    {
                       memcpy(&evis->info, &(xvi[j]), sizeof(XVisualInfo));
                       evis->config = configs[i];
                       found = EINA_TRUE;
                       break;
                    }
               }
             else
               {
                  XRenderPictFormat *fmt;

                  fmt = XRenderFindVisualFormat
                        (einfo->info.display, xvi[j].visual);
                  if ((fmt->direct.alphaMask > 0) &&
                      (fmt->type == PictTypeDirect))
                    {
                       memcpy(&evis->info, &(xvi[j]), sizeof(XVisualInfo));
                       evis->config = configs[i];
                       found = EINA_TRUE;
                       break;
                    }
               }
          }
        if (xvi) XFree(xvi);
     }
   if (!found)
     {
        // this is a less correct fallback if the above fails to
        // find the right visuals/configs
        if (!alpha)
          {
             evis->config = configs[0];
             XMatchVisualInfo(einfo->info.display,
                              einfo->info.screen, depth, TrueColor,
                              &evis->info);
          }
        else
          {
             XVisualInfo *xvi, vi_in;
             int nvi, j;
             XRenderPictFormat *fmt;

             evis->config = configs[0];
             vi_in.screen = einfo->info.screen;
             vi_in.depth = 32;
             vi_in.class = TrueColor;
             xvi = XGetVisualInfo(einfo->info.display,
                                  VisualScreenMask | VisualDepthMask |
                                  VisualClassMask,
                                  &vi_in, &nvi);
             if (xvi)
               {
                  for (j = 0; j < nvi; j++)
                    {
                       fmt = XRenderFindVisualFormat(einfo->info.display,
                                                     xvi[j].visual);
                       if ((fmt->type == PictTypeDirect) &&
                           (fmt->direct.alphaMask))
                         {
                            memcpy(&evis->info, &(xvi[j]), sizeof(XVisualInfo));
                            break;
                         }
                    }
                  XFree(xvi);
               }
          }
     }

   // GLX
#else
   GLXFBConfig *configs = NULL, config = 0;

try_again:
   i = 0;
   config_attrs[i++] = GLX_DRAWABLE_TYPE;
   config_attrs[i++] = GLX_WINDOW_BIT;
   config_attrs[i++] = GLX_DOUBLEBUFFER;
   config_attrs[i++] = 1;
   config_attrs[i++] = GLX_RED_SIZE;
   config_attrs[i++] = 1;
   config_attrs[i++] = GLX_GREEN_SIZE;
   config_attrs[i++] = 1;
   config_attrs[i++] = GLX_BLUE_SIZE;
   config_attrs[i++] = 1;
   if (alpha)
     {
        config_attrs[i++] = GLX_RENDER_TYPE;
        config_attrs[i++] = GLX_RGBA_BIT;
        config_attrs[i++] = GLX_ALPHA_SIZE;
        config_attrs[i++] = 1;
     }
   else
     {
        config_attrs[i++] = GLX_ALPHA_SIZE;
        config_attrs[i++] = 0;
     }
   if (depth_bits)
     {
        config_attrs[i++] = GLX_DEPTH_SIZE;
        config_attrs[i++] = depth_bits;
     }
   if (stencil_bits)
     {
        config_attrs[i++] = GLX_STENCIL_SIZE;
        config_attrs[i++] = stencil_bits;
     }
   if (msaa_samples)
     {
        config_attrs[i++] = GLX_SAMPLE_BUFFERS;
        config_attrs[i++] = 1;
        config_attrs[i++] = GLX_SAMPLES;
        config_attrs[i++] = msaa_samples;
     }
   config_attrs[i++] = GLX_AUX_BUFFERS;
   config_attrs[i++] = 0;
   config_attrs[i++] = GLX_STEREO;
   config_attrs[i++] = 0;
   config_attrs[i++] = GLX_TRANSPARENT_TYPE;
   config_attrs[i++] = GLX_NONE;//GLX_TRANSPARENT_INDEX,GLX_TRANSPARENT_RGB
   config_attrs[i++] = 0;

   configs = glXChooseFBConfig(einfo->info.display,
                               einfo->info.screen,
                               config_attrs, &num);
   if ((!configs) || (num < 1))
     {
        ERR("glXChooseFBConfig() can't find any configs (alpha: %d, depth: %d, stencil: %d, msaa: %d)",
            alpha, depth_bits, stencil_bits, msaa_samples);
        if (configs) XFree(configs);
        if ((depth_bits > 24) || (stencil_bits > 8))
          {
             WRN("Please note that your driver might not support 32-bit depth or "
                 "16-bit stencil buffers, so depth24, stencil8 are the maximum "
                 "recommended values.");
             if (depth_bits > 24) depth_bits = 24;
             if (stencil_bits > 8) stencil_bits = 8;
             DBG("Trying again with depth:%d, stencil:%d", depth_bits, stencil_bits);
             goto try_again;
          }
        else if (msaa_samples)
          {
             msaa_samples /= 2;
             DBG("Trying again with msaa_samples: %d", msaa_samples);
             goto try_again;
          }
        else if (depth_bits || stencil_bits)
          {
             depth_bits = 0;
             stencil_bits = 0;
             DBG("Trying again without any depth or stencil buffer");
             goto try_again;
          }
        free(evis);
        return NULL;
     }

   found = EINA_FALSE;
   for (i = 0; (i < num) && !found; i++)
     {
        XVisualInfo *visinfo;
        XRenderPictFormat *format = NULL;

        visinfo = glXGetVisualFromFBConfig(einfo->info.display,
                                           configs[i]);
        if (!visinfo) continue;
        if (visinfo->visual->class != TrueColor)
          {
             XFree(visinfo);
             continue;
          }
        if (!alpha)
          {
             config = configs[i];
             // ensure depth matches default depth!
             if (DefaultDepth(einfo->info.display, 0) ==
                 visinfo->depth)
               {
                  memcpy(&evis->info, visinfo, sizeof(XVisualInfo));
                  evis->config = config;
                  found = EINA_TRUE;
                  XFree(visinfo);
                  break;
               }
          }
        else
          {
             format = XRenderFindVisualFormat
                   (einfo->info.display, visinfo->visual);
             if (!format)
               {
                  XFree(visinfo);
                  continue;
               }
             if ((format->direct.alphaMask > 0) &&
                 (format->type == PictTypeDirect))
               {
                  memcpy(&evis->info, visinfo, sizeof(XVisualInfo));
                  evis->config = configs[i];
                  evis->cmap = format->colormap;
                  found = EINA_TRUE;
                  XFree(visinfo);
                  break;
               }
          }
        XFree(visinfo);
     }

   XFree(configs);
   if (!found)
     {
        ERR("Could not find a matching config. Now what?");
        free(evis);
        return NULL;
     }
#endif

   if (!evis->cmap)
     {
        /* save colormap now */
        evis->disp = einfo->info.display;
        evis->cmap = XCreateColormap(einfo->info.display,
                                    RootWindow(einfo->info.display,
                                               einfo->info.screen),
                                    evis->info.visual, 0);
     }

   eina_hash_add(_evas_gl_visuals, &idx, evis);
   return evis->info.visual;
}

Colormap
eng_best_colormap_get(Evas_Engine_Info_GL_X11 *einfo)
{
   Evas_GL_X11_Visual *evis;
   int idx;

   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   idx = _visuals_hash_index_get_from_info(einfo);
   evis = eina_hash_find(_evas_gl_visuals, &idx);
   if (!evis)
     {
        eng_best_visual_get(einfo);
        evis = eina_hash_find(_evas_gl_visuals, &idx);
        if (!evis) return 0;
     }
   return evis->cmap;
}

int
eng_best_depth_get(Evas_Engine_Info_GL_X11 *einfo)
{
   Evas_GL_X11_Visual *evis;
   int idx;

   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   idx = _visuals_hash_index_get_from_info(einfo);
   evis = eina_hash_find(_evas_gl_visuals, &idx);
   if (!evis)
     {
        eng_best_visual_get(einfo);
        evis = eina_hash_find(_evas_gl_visuals, &idx);
        if (!evis) return 0;
     }
   return evis->info.depth;
}

Context_3D *
eng_gl_context_new(Outbuf *win)
{
   Context_3D *ctx;
#if GL_GLES
   int context_attrs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
#endif

   if (!win) return NULL;

   ctx = calloc(1, sizeof(Context_3D));
   if (!ctx) return NULL;

#if GL_GLES
   if (win->gles3)
     context_attrs[1] = 3;
   ctx->context = eglCreateContext(win->egl_disp, win->egl_config,
                                   win->egl_context, context_attrs);

   if (!ctx->context)
     {
        ERR("EGL context creation failed.");
        goto error;
     }

   ctx->display = win->egl_disp;
   ctx->surface = win->egl_surface;
#else
   ctx->context = glXCreateContext(win->disp, win->visualinfo, win->context, 1);

   if (!ctx->context)
     {
        ERR("GLX context creation failed.");
        goto error;
     }

   ctx->display = win->disp;
   ctx->glxwin = win->glxwin;
   ctx->win = win->win;
#endif

   return ctx;

error:
   free(ctx);
   return NULL;
}

void
eng_gl_context_free(Context_3D *ctx)
{
#if GL_GLES
   eglDestroyContext(ctx->display, ctx->context);
#else
   glXDestroyContext(ctx->display, ctx->context);
#endif

   free(ctx);
}

void
eng_gl_context_use(Context_3D *ctx)
{
#if GL_GLES
    SET_RESTORE_CONTEXT();
   if (evas_eglMakeCurrent(ctx->display, ctx->surface,
                      ctx->surface, ctx->context) == EGL_FALSE)
     {
        ERR("evas_eglMakeCurrent() failed.");
     }
#else
   if (!__glXMakeContextCurrent(ctx->display, ctx->glxwin, ctx->context))
     {
        ERR("glXMakeContextCurrent(%p, %p, %p, %p) faild.",
            (void *)ctx->display, (void *)ctx->glxwin,
            (void *)ctx->win, (void *)ctx->context);
     }
#endif
}

void
eng_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth EINA_UNUSED)
{
   ob->w = w;
   ob->h = h;
   ob->rot = rot;
   eng_window_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);
}

int
eng_outbuf_get_rot(Outbuf *ob)
{
   return ob->rot;
}

Render_Output_Swap_Mode
eng_outbuf_swap_mode(Outbuf *ob)
{
   if (ob->swap_mode == MODE_AUTO && extn_have_buffer_age)
     {
        Render_Output_Swap_Mode swap_mode;
        eina_evlog("+gl_query_surf_swap_mode", ob, 0.0, NULL);
#ifdef GL_GLES
        EGLint age = 0;

        if (!eglQuerySurface(ob->egl_disp, ob->egl_surface,
                             EGL_BUFFER_AGE_EXT, &age))
          age = 0;
#else
        unsigned int age = 0;

        if (glsym_glXQueryDrawable)
          {
             if (glsym_glXQueryDrawable(ob->disp, ob->glxwin,
                                        GLX_BACK_BUFFER_AGE_EXT, &age) < 1)
               age = 0;
          }
#endif
        if (age == 1) swap_mode = MODE_COPY;
        else if (age == 2) swap_mode = MODE_DOUBLE;
        else if (age == 3) swap_mode = MODE_TRIPLE;
        else if (age == 4) swap_mode = MODE_QUADRUPLE;
        else swap_mode = MODE_FULL;
        if ((int)age != ob->prev_age)
          {
             char buf[16];
             snprintf(buf, sizeof(buf), "! %i", (int)age);
             eina_evlog("!gl_buffer_age", ob, 0.0, buf);
          }
        else
          {
             char buf[16];
             snprintf(buf, sizeof(buf), "%i", (int)age);
             eina_evlog("!gl_buffer_age", ob, 0.0, buf);
          }
        ob->prev_age = age;

        eina_evlog("-gl_query_surf_swap_mode", ob, 0.0, NULL);
        return swap_mode;
     }

   return ob->swap_mode;
}

Eina_Bool
eng_outbuf_region_first_rect(Outbuf *ob)
{
   ob->gl_context->preserve_bit = GL_COLOR_BUFFER_BIT0_QCOM;

   glsym_evas_gl_preload_render_lock(eng_preload_make_current, ob);
#ifdef GL_GLES
   // dont need to for egl - eng_window_use() can check for other ctxt's
#else
   eng_window_use(NULL);
#endif
   eng_window_use(ob);
   if (!_re_wincheck(ob)) return EINA_TRUE;

   glsym_evas_gl_common_context_resize(ob->gl_context,
                                 ob->w, ob->h,
                                 ob->rot);

   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);
   if (partial_render_debug == 1)
     {
        glClearColor(0.2, 0.5, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
     }

   return EINA_FALSE;
}

#ifdef GL_GLES
static void
_convert_to_glcoords(int *result, Outbuf *ob, int x, int y, int w, int h)
{

   switch (ob->rot)
     {
      case 0:
        result[0] = x;
        result[1] = ob->gl_context->h - (y + h);
        result[2] = w;
        result[3] = h;
        break;
      case 90:
        result[0] = y;
        result[1] = x;
        result[2] = h;
        result[3] = w;
        break;
      case 180:
        result[0] = ob->gl_context->w - (x + w);
        result[1] = y;
        result[2] = w;
        result[3] = h;
        break;
      case 270:
        result[0] = ob->gl_context->h - (y + h);
        result[1] = ob->gl_context->w - (x + w);
        result[2] = h;
        result[3] = w;
        break;
      default:
        result[0] = x;
        result[1] = ob->gl_context->h - (y + h);
        result[2] = w;
        result[3] = h;
        break;
     }
}

void
eng_outbuf_damage_region_set(Outbuf *ob, Tilebuf_Rect *damage)
{
   if (glsym_eglSetDamageRegionKHR)
     {
        Tilebuf_Rect *tr;
        int *rect, *rects, count;

        count = eina_inlist_count(EINA_INLIST_GET(damage));
        rects = alloca(sizeof(int) * 4 * count);
        rect = rects;
        EINA_INLIST_FOREACH(damage, tr)
          {
             _convert_to_glcoords(rect, ob, tr->x, tr->y, tr->w, tr->h);
             rect += 4;
          }
        glsym_eglSetDamageRegionKHR(ob->egl_disp, ob->egl_surface, rects, count);
     }
}
#endif

void*
eng_outbuf_new_region_for_update(Outbuf *ob,
                                 int x, int y, int w, int h,
                                 int *cx EINA_UNUSED, int *cy EINA_UNUSED,
                                 int *cw EINA_UNUSED, int *ch EINA_UNUSED)
{
   if (w == (int) ob->w && h == (int) ob->h)
     {
        ob->gl_context->master_clip.enabled = EINA_FALSE;
     }
   else
     {
        ob->gl_context->master_clip.enabled = EINA_TRUE;
        ob->gl_context->master_clip.x = x;
        ob->gl_context->master_clip.y = y;
        ob->gl_context->master_clip.w = w;
        ob->gl_context->master_clip.h = h;
     }
   return ob->gl_context->def_surface;
}

void
eng_outbuf_push_updated_region(Outbuf *ob, RGBA_Image *update EINA_UNUSED,
                               int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   /* Is it really necessary to flush per region ? Shouldn't we be able to
      still do that for the full canvas when doing partial update */
   if (!_re_wincheck(ob)) return;
   ob->draw.drew = 1;
   glsym_evas_gl_common_context_flush(ob->gl_context);
}

void
eng_outbuf_push_free_region_for_update(Outbuf *ob EINA_UNUSED,
                                       RGBA_Image *update EINA_UNUSED)
{
   /* Nothing to do here as we don't really create an image per area */
}

void
eng_outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage EINA_UNUSED, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode)
{
   Tilebuf_Rect *rects = buffer_damage;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) goto end;

   if (!_re_wincheck(ob)) goto end;
   if (!ob->draw.drew) goto end;

   ob->draw.drew = 0;
   eng_window_use(ob);
   glsym_evas_gl_common_context_done(ob->gl_context);

   // Save contents of the framebuffer to a file
   if (swap_buffer_debug_mode == 1)
     {
        if (swap_buffer_debug)
          {
             char fname[100];
             int ret = 0;
             snprintf(fname, sizeof(fname), "%p", (void*)ob);

             ret = glsym_evas_gl_common_buffer_dump(ob->gl_context,
                                                    (const char*)debug_dir,
                                                    (const char*)fname,
                                                    ob->frame_cnt,
                                                    NULL);
             if (!ret) swap_buffer_debug_mode = 0;
          }
     }

#ifdef GL_GLES
   if (!ob->vsync)
     {
        if (ob->info->vsync) eglSwapInterval(ob->egl_disp, 1);
        else eglSwapInterval(ob->egl_disp, 0);
        ob->vsync = 1;
     }
   if ((glsym_eglSwapBuffersWithDamage) && (rects) &&
       (ob->swap_mode != MODE_FULL))
     {
        EGLint num = 0, *result = NULL, i = 0;
        Tilebuf_Rect *r;

        // if partial swaps can be done use re->rects
        num = eina_inlist_count(EINA_INLIST_GET(rects));
        if (num > 0)
          {
             result = alloca(sizeof(EGLint) * 4 * num);
             EINA_INLIST_FOREACH(EINA_INLIST_GET(rects), r)
               {
                  _convert_to_glcoords(&result[i], ob, r->x, r->y, r->w, r->h);
                  i += 4;
               }
             glsym_eglSwapBuffersWithDamage(ob->egl_disp,
                                            ob->egl_surface,
                                            result, num);
          }
     }
   else
     eglSwapBuffers(ob->egl_disp, ob->egl_surface);

//xx   if (!safe_native) eglWaitGL();
//   if (eglGetError() != EGL_SUCCESS)
//     {
//        printf("Error:  eglSwapBuffers() fail.\n");
//     }
#else
   (void)rects;
#ifdef VSYNC_TO_SCREEN
   if (ob->info->vsync)
     {
        if (glsym_glXSwapIntervalEXT)
          {
             if (!ob->vsync)
               {
                  if (ob->info->vsync) glsym_glXSwapIntervalEXT(ob->disp, ob->win, 1);
                  else glsym_glXSwapIntervalEXT(ob->disp, ob->win, 0);
                  ob->vsync = 1;
               }
          }
        else if (glsym_glXSwapIntervalSGI)
          {
             if (!ob->vsync)
               {
                  if (ob->info->vsync) glsym_glXSwapIntervalSGI(1);
                  else glsym_glXSwapIntervalSGI(0);
                  ob->vsync = 1;
               }
          }
        else
          {
             if ((glsym_glXGetVideoSync) && (glsym_glXWaitVideoSync))
               {
                  unsigned int rc;

                  glsym_glXGetVideoSync(&rc);
                  glsym_glXWaitVideoSync(1, 0, &rc);
               }
          }
     }
#endif
   // XXX: if partial swaps can be done use re->rects
//   measure(0, "swap");
   glXSwapBuffers(ob->disp, ob->glxwin);
//   measure(1, "swap");
#endif
   // clear out rects after swap as we may use them during swap

   ob->frame_cnt++;

 end:
   glsym_evas_gl_preload_render_unlock(eng_preload_make_current, ob);
}

Evas_Engine_GL_Context *
eng_outbuf_gl_context_get(Outbuf *ob)
{
   return ob->gl_context;
}

void *
eng_outbuf_egl_display_get(Outbuf *ob)
{
#ifdef GL_GLES
   return ob->egl_disp;
#else
   (void) ob;
   return NULL;
#endif
}
