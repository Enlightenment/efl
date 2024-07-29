
# include <evil_private.h> /* dlsym */

#include "evas_engine.h"
#include "../gl_common/evas_gl_define.h"

#include <EGL/eglext_angle.h>

# define SET_RESTORE_CONTEXT() \
do { \
   if (glsym_evas_gl_common_context_restore_set)            \
     glsym_evas_gl_common_context_restore_set(EINA_TRUE);   \
} while(0)

static Eina_TLS _outbuf_key = 0;
static Eina_TLS _context_key = 0;

typedef void (*glsym_func_void) ();
glsym_func_void glsym_evas_gl_common_context_restore_set = NULL;

Eina_Bool gles3_supported = EINA_FALSE;

typedef EGLContext GLContext;
typedef EGLConfig GLConfig;
static Eina_Bool gles3_probed = EINA_FALSE;

static int win_count = 0;
static Eina_Bool initted = EINA_FALSE;

static double _freq = 0.0;

#if 0
static double
_time_get(void)
{
   LARGE_INTEGER count;

   QueryPerformanceCounter(&count);

   return count.QuadPart / _freq;
}

static void
measure(int inout, const char *what)
{
   static double t0 = 0.0;
   double t;

   if (inout == 0) t0 = _time_get();
   else if (inout ==1)
     {
        t = _time_get() - t0;
        printf("%s: %e\n", what, t);
        fflush(stdout);
     }
}
#else
# define measure(x, y)
#endif

Eina_Bool
eng_init(void)
{
   LARGE_INTEGER freq;

   if (initted)
     return EINA_TRUE;

   QueryPerformanceFrequency(&freq);
   _freq = (double)freq.QuadPart;

   QueryPerformanceFrequency(&freq);

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

#if 0

void *
eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo)
{
   Evas_GL_X11_Visual *evis;
   int alpha;
   int depth_bits;
   int stencil_bits;
   int msaa_samples;
   int config_attrs[40];
   int i;
   int num;
   Eina_Bool found;

   if (!einfo) return NULL;
   if (!einfo->info.display) return NULL;

   alpha = einfo->info.destination_alpha;
   depth_bits = alpha ? 32 : 24;
   stencil_bits = einfo->stencil_bits;
   msaa_samples = einfo->msaa_bits;

   evis->alpha = alpha;

   EGLDisplay *egl_disp;
   EGLConfig configs[200];
   int major_version, minor_version;
   const char *eglexts, *s;
   int depth = DefaultDepth(einfo->info.display, einfo->info.screen);

   /* detect GLES 3.x support */
   if (gles3_probed == EINA_FALSE)
     {
        gles3_probed = EINA_TRUE;
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
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 1;
   config_attrs[i++] = EGL_ALPHA_SIZE;
   config_attrs[i++] = (alpha ? 1 : 0);

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
          }
        else
          {
             XVisualInfo *xvi, vi_in;
             int nvi, j;
             XRenderPictFormat *fmt;

             evis->config = configs[0];
          }
     }
   return evis->info.visual;
}
#endif

static EGLDisplay *
_win32_eglGetDisplay(HDC dc)
{
   EGLDisplay (*eglsym_eglGetPlatformDisplay)
         (EGLenum platform, void *native_display, const EGLAttrib *attrib_list) = NULL;
   EGLDisplay *egldisp = EGL_NO_DISPLAY;

   eglsym_eglGetPlatformDisplay = dlsym(RTLD_DEFAULT, "eglGetPlatformDisplay");
   if (eglsym_eglGetPlatformDisplay)
     {
        egldisp = eglsym_eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                               (EGLNativeDisplayType)dc, NULL);
        if (egldisp) return egldisp;
     }

   return eglGetDisplay((EGLNativeDisplayType)dc);
}

Outbuf *
eng_window_new(Evas_Engine_Info_GL_Win32 *info,
               unsigned int w,
               unsigned int h,
               Render_Output_Swap_Mode swap_mode)
{
   Outbuf *gw;
   HDC dc;
   GLContext context;
   int context_attrs[3];
   int major_version, minor_version;
   const GLubyte *vendor, *renderer, *version, *glslversion;
   int blacklist = 0;
   int val = 0;

   dc = GetDC(info->info.window);
   if (!dc)
     return NULL;

   gw = calloc(1, sizeof(Outbuf));
   if (!gw) return NULL;

   win_count++;
   gw->window = info->info.window;
   gw->dc = dc;
   gw->alpha = info->info.destination_alpha;
   gw->w = w;
   gw->h = h;
   gw->rotation = info->info.rotation;
   gw->swap_mode = swap_mode;
   gw->info = info;
   gw->depth_bits = gw->alpha ? 32 : 24;
   gw->stencil_bits = info->stencil_bits;
   gw->msaa_bits = info->msaa_bits;

   gw->egl_disp = _win32_eglGetDisplay(dc);
   if (gw->egl_disp == EGL_NO_DISPLAY)
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

   {
      EGLConfig configs[200];
      int config_attrs[40], i, num, idx;
      int alpha;
      int depth_bits;
      int stencil_bits;
      int msaa_samples;
      Eina_Bool found;

      /* detect GLES 3.x support */
      if (gles3_probed == EINA_FALSE)
        {
          const char *eglexts;

          gles3_probed = EINA_TRUE;
          eglexts = eglQueryString(gw->egl_disp, EGL_EXTENSIONS);
        }

      alpha = info->info.destination_alpha;
      depth_bits = alpha ? 32 : 24;
      stencil_bits = info->stencil_bits;
      msaa_samples = info->msaa_bits;

   try_again:
      i = 0;
      config_attrs[i++] = EGL_SURFACE_TYPE;
      config_attrs[i++] = EGL_WINDOW_BIT;
      config_attrs[i++] = EGL_RENDERABLE_TYPE;
      if (gles3_supported)
        config_attrs[i++] = EGL_OPENGL_ES3_BIT_KHR;
      else
        config_attrs[i++] = EGL_OPENGL_ES2_BIT;
      config_attrs[i++] = EGL_RED_SIZE;
      config_attrs[i++] = 1;
      config_attrs[i++] = EGL_GREEN_SIZE;
      config_attrs[i++] = 1;
      config_attrs[i++] = EGL_BLUE_SIZE;
      config_attrs[i++] = 1;
      config_attrs[i++] = EGL_ALPHA_SIZE;
      config_attrs[i++] = (alpha ? 1 : 0);

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
      if ((!eglChooseConfig(gw->egl_disp, config_attrs, configs, 200, &num))
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

          eng_window_free(gw);
          return NULL;
        }

      found = EINA_FALSE;

      // FIXME to do

      if (!found)
        {
          gw->egl_config = configs[0];
        }
   }

   gw->egl_surface = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                            (EGLNativeWindowType)gw->window,
                                            NULL);
   if (gw->egl_surface == EGL_NO_SURFACE)
     {
        unsigned int err = eglGetError();
        printf("surf creat fail! %x\n", err);
        ERR("eglCreateWindowSurface() fail for %#x. code=%#x",
            (unsigned int)(uintptr_t)gw->window, err);
        eng_window_free(gw);
        return NULL;
     }

try_gles2:
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = gles3_supported ? 3 : 2;
   context_attrs[2] = EGL_NONE;

   context = _tls_context_get();
   gw->egl_context = eglCreateContext
     (gw->egl_disp, gw->egl_config, context, context_attrs);
   if (gw->egl_context == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        if (gles3_supported)
          {
             /* Note: this shouldn't happen */
             ERR("Trying again with an Open GL ES 2 context (fallback).");
             gles3_supported = EINA_FALSE;
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

   eng_gl_symbols(gw);
   gw->gl_context = glsym_evas_gl_common_context_new();
   if (!gw->gl_context)
     {
        ERR("Unable to get a new context.");
        eng_window_free(gw);
        return NULL;
     }
   gw->gl_context->egldisp = gw->egl_disp;
   gw->gl_context->eglctxt = gw->egl_context;
   gw->gl_context->msaa = (Eina_Bool) !!gw->msaa_bits;
   eng_window_use(gw);
   glsym_evas_gl_common_context_resize(gw->gl_context,
                                       w, h, info->info.rotation);
   gw->surf = 1;
   return gw;
}

void
eng_window_free(Outbuf *gw)
{
   Outbuf *buf;
   GLContext context;
   int ref = 0;

   win_count--;
   eng_window_use(gw);

   if (win_count == 0) evas_common_font_ext_clear();

   context = _tls_context_get();
   buf = _tls_outbuf_get();

   if (gw == buf) _tls_outbuf_set(NULL);
   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        glsym_evas_gl_common_context_free(gw->gl_context);
     }

   SET_RESTORE_CONTEXT();
   if (!evas_eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
     ERR("evas_eglMakeCurrent() failed!");

   if (gw->egl_surface != EGL_NO_SURFACE)
     eglDestroySurface(gw->egl_disp, gw->egl_surface);

   if (gw->egl_context != context)
     eglDestroyContext(gw->egl_disp, gw->egl_context);

   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eglTerminate(gw->egl_disp);
        eglReleaseThread();
        _tls_context_set(EGL_NO_CONTEXT);
     }

   free(gw);
}

static Eina_Bool
eng_window_make_current(void *data, void *doit)
{
   Outbuf *gw = data;

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
   return EINA_TRUE;
}

void
eng_window_use(Outbuf *gw)
{
   Eina_Bool force_use = EINA_FALSE;
   Outbuf *buf;

   buf = _tls_outbuf_get();

   glsym_evas_gl_preload_render_lock(eng_window_make_current, gw);
   if ((gw) && (!gw->gl_context)) return;

   if (buf)
     {
        if ((evas_eglGetCurrentDisplay() != buf->egl_disp) ||
            (evas_eglGetCurrentContext() != buf->egl_context))
          force_use = EINA_TRUE;
     }
   if ((buf != gw) || (force_use))
     {
        if (buf)
          {
             glsym_evas_gl_common_context_use(buf->gl_context);
             glsym_evas_gl_common_context_flush(buf->gl_context);
          }
        _tls_outbuf_set(gw);
        if (gw)
          {
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
          }
     }
   if (gw) glsym_evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_unsurf(Outbuf *gw)
{
   Outbuf *buf;

   if (!gw->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO")) printf("unsurf %p\n", gw);

   buf = _tls_outbuf_get();
   if (buf)
      glsym_evas_gl_common_context_flush(buf->gl_context);
   if (buf == gw)
     {
        SET_RESTORE_CONTEXT();
        if (!evas_eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
          ERR("evas_eglMakeCurrent() failed!");
        if (gw->egl_surface != EGL_NO_SURFACE)
          eglDestroySurface(gw->egl_disp, gw->egl_surface);
        gw->egl_surface = EGL_NO_SURFACE;
        _tls_outbuf_set(NULL);
     }

   gw->surf = 0;
}

void
eng_window_resurf(Outbuf *gw)
{
   if (gw->surf) return;
   if (getenv("EVAS_GL_INFO")) printf("resurf %p\n", gw);

   if (gw->egl_surface)
     eglDestroySurface(gw->egl_disp, gw->egl_surface);
   gw->egl_surface = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                            (EGLNativeWindowType)gw->window,
                                            NULL);
   if (gw->egl_surface == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %#x. code=%#x",
            (unsigned int)(uintptr_t)gw->window, eglGetError());
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

   gw->surf = 1;
}

Context_3D *
eng_gl_context_new(Outbuf *win)
{
   Context_3D *ctx;
   int context_attrs[3] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

   if (!win) return NULL;

   ctx = calloc(1, sizeof(Context_3D));
   if (!ctx) return NULL;

   if (gles3_supported)
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

   return ctx;

error:
   free(ctx);
   return NULL;
}

void
eng_gl_context_free(Context_3D *ctx)
{
   eglDestroyContext(ctx->display, ctx->context);

   free(ctx);
}

void
eng_gl_context_use(Context_3D *ctx)
{
   SET_RESTORE_CONTEXT();
   if (evas_eglMakeCurrent(ctx->display, ctx->surface,
                           ctx->surface, ctx->context) == EGL_FALSE)
     {
        ERR("evas_eglMakeCurrent() failed.");
     }
}

void
eng_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth EINA_UNUSED)
{
   ob->w = w;
   ob->h = h;
   ob->rotation = rot;
   eng_window_use(ob);
   glsym_evas_gl_common_context_resize(ob->gl_context, w, h, rot);
}

int
eng_outbuf_get_rot(Outbuf *ob)
{
   return ob->rotation;
}

Render_Output_Swap_Mode
eng_outbuf_swap_mode(Outbuf *ob)
{
   if (ob->swap_mode == MODE_AUTO && extn_have_buffer_age)
     {
        Render_Output_Swap_Mode swap_mode;
        eina_evlog("+gl_query_surf_swap_mode", ob, 0.0, NULL);
        measure(0, "query age");
        EGLint age = 0;

        if (!eglQuerySurface(ob->egl_disp, ob->egl_surface,
                             EGL_BUFFER_AGE_EXT, &age))
          age = 0;
        measure(1, "query age");
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
             swap_mode = MODE_FULL;
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
   eng_window_use(ob);
   if (!_re_wincheck(ob)) return EINA_TRUE;

   glsym_evas_gl_common_context_resize(ob->gl_context,
                                 ob->w, ob->h,
                                 ob->rotation);

   glsym_evas_gl_common_context_flush(ob->gl_context);
   glsym_evas_gl_common_context_newframe(ob->gl_context);
   if (partial_render_debug == 1)
     {
        glClearColor(0.2, 0.5, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
     }

   return EINA_FALSE;
}
static void
_convert_to_glcoords(int *result, Outbuf *ob, int x, int y, int w, int h)
{

   switch (ob->rotation)
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

   if (!ob->vsync)
     {
        if (ob->info->vsync) eglSwapInterval(ob->egl_disp, 1);
        else eglSwapInterval(ob->egl_disp, 0);
        ob->vsync = 1;
     }
   if ((glsym_eglSwapBuffersWithDamage) && (rects) &&
       (ob->swap_mode != MODE_FULL) && (!partial_render_debug))
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
             measure(0, "swap with damage");
             glsym_eglSwapBuffersWithDamage(ob->egl_disp,
                                            ob->egl_surface,
                                            result, num);
             measure(1, "swap with damage");
          }
     }
   else
     {
        measure(0, "swap");
        eglSwapBuffers(ob->egl_disp, ob->egl_surface);
        measure(1, "swap");
     }

//xx   if (!safe_native) eglWaitGL();
//   if (eglGetError() != EGL_SUCCESS)
//     {
//        printf("Error:  eglSwapBuffers() fail.\n");
//     }

   // clear out rects after swap as we may use them during swap

   ob->frame_cnt++;
   ob->gl_context->frame_cnt++;

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
   return ob->egl_disp;
}
