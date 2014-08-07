#include "evas_engine.h"

static Outbuf *_evas_gl_x11_window = NULL;

#ifdef GL_GLES
static EGLContext context = EGL_NO_CONTEXT;
#else
// FIXME: this will only work for 1 display connection (glx land can have > 1)
static GLXContext context = 0;
static GLXContext rgba_context = 0;
static GLXFBConfig fbconf = 0;
static GLXFBConfig rgba_fbconf = 0;
#endif

// fixme: something is up/wrong here - dont know what tho...
//#define NEWGL 1

static XVisualInfo *_evas_gl_x11_vi = NULL;
static XVisualInfo *_evas_gl_x11_rgba_vi = NULL;
static Colormap     _evas_gl_x11_cmap = 0;
static Colormap     _evas_gl_x11_rgba_cmap = 0;

static int win_count = 0;

Outbuf *
eng_window_new(Evas_Engine_Info_GL_X11 *info,
               Evas *e,
               Display *disp,
               Window   win,
               int      screen,
               Visual  *vis,
               Colormap cmap,
               int      depth,
               int      w,
               int      h,
               int      indirect,
               int      alpha,
               int      rot,
               Render_Engine_Swap_Mode swap_mode)
{
   Outbuf *gw;
#ifdef GL_GLES
   int context_attrs[3];
   int config_attrs[40];
   int major_version, minor_version;
   int num_config, n = 0;
#endif
   XVisualInfo *vi_use;
   const GLubyte *vendor, *renderer, *version;
   int blacklist = 0;

   if (!_evas_gl_x11_vi) return NULL;

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
   gw->evas = e;

   vi_use = _evas_gl_x11_vi;
   if (gw->alpha)
     {
#ifdef GL_GLES
        if (_evas_gl_x11_rgba_vi)
          {
             vi_use = _evas_gl_x11_rgba_vi;
          }
#else
//#ifdef NEWGL
        if (_evas_gl_x11_rgba_vi)
          {
             vi_use = _evas_gl_x11_rgba_vi;
          }
//#endif
#endif
     }
   gw->visualinfo = vi_use;

// EGL / GLES
#ifdef GL_GLES
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_WINDOW_BIT;
   config_attrs[n++] = EGL_RENDERABLE_TYPE;
   config_attrs[n++] = EGL_OPENGL_ES2_BIT;
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
   if (gw->alpha)
     {
        config_attrs[n++] = EGL_ALPHA_SIZE;
        config_attrs[n++] = 1;
     }
   else
     {
        config_attrs[n++] = EGL_ALPHA_SIZE;
        config_attrs[n++] = 0;
     }
   config_attrs[n++] = EGL_DEPTH_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_STENCIL_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_NONE;
   
   gw->egl_disp = eglGetDisplay((EGLNativeDisplayType)(gw->disp));
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
   eglBindAPI(EGL_OPENGL_ES_API);
   if (eglGetError() != EGL_SUCCESS)
     {
        ERR("eglBindAPI() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }

   num_config = 0;
   if (!eglChooseConfig(gw->egl_disp, config_attrs, &gw->egl_config,
                        1, &num_config) || (num_config != 1))
     {
        ERR("eglChooseConfig() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }
   gw->egl_surface[0] = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                               (EGLNativeWindowType)gw->win,
                                               NULL);
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %#x. code=%#x",
            (unsigned int)gw->win, eglGetError());
        eng_window_free(gw);
        return NULL;
     }
   
   gw->egl_context[0] = eglCreateContext
     (gw->egl_disp, gw->egl_config, context, context_attrs);
   if (gw->egl_context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }
   if (context == EGL_NO_CONTEXT) context = gw->egl_context[0];
   
   if (eglMakeCurrent(gw->egl_disp,
                      gw->egl_surface[0],
                      gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() fail. code=%#x", eglGetError());
        eng_window_free(gw);
        return NULL;
     }

   vendor = glGetString(GL_VENDOR);
   renderer = glGetString(GL_RENDERER);
   version = glGetString(GL_VERSION);
   if (!vendor)   vendor   = (unsigned char *)"-UNKNOWN-";
   if (!renderer) renderer = (unsigned char *)"-UNKNOWN-";
   if (!version)  version  = (unsigned char *)"-UNKNOWN-";
   if (getenv("EVAS_GL_INFO"))
     {
        fprintf(stderr, "vendor: %s\n", vendor);
        fprintf(stderr, "renderer: %s\n", renderer);
        fprintf(stderr, "version: %s\n", version);
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
// GLX
#else
   if (!context)
     {
#ifdef NEWGL
        if (indirect)
          context = glXCreateNewContext(gw->disp, fbconf,
                                        GLX_RGBA_TYPE, NULL,
                                        GL_FALSE);
        else
          context = glXCreateNewContext(gw->disp, fbconf,
                                        GLX_RGBA_TYPE, NULL,
                                        GL_TRUE);
#else
        if (indirect)
          context = glXCreateContext(gw->disp, gw->visualinfo, NULL, GL_FALSE);
        else
          context = glXCreateContext(gw->disp, gw->visualinfo, NULL, GL_TRUE);
#endif
     }
#ifdef NEWGL
   if ((gw->alpha) && (!rgba_context))
     {
        if (indirect)
          rgba_context = glXCreateNewContext(gw->disp, rgba_fbconf,
                                             GLX_RGBA_TYPE, context,
                                             GL_FALSE);
        else
          rgba_context = glXCreateNewContext(gw->disp, rgba_fbconf,
                                             GLX_RGBA_TYPE, context,
                                             GL_TRUE);
     }
   if (gw->alpha)
     gw->glxwin = glXCreateWindow(gw->disp, rgba_fbconf, gw->win, NULL);
   else
     gw->glxwin = glXCreateWindow(gw->disp, fbconf, gw->win, NULL);
   if (!gw->glxwin)
     {
        eng_window_free(gw);
        return NULL;
     }

   if (gw->alpha) gw->context = rgba_context;
   else gw->context = context;
#else
   gw->context = context;
#endif

   if (!gw->context)
     {
        eng_window_free(gw);
        return NULL;
     }
   if (gw->context)
     {
        if (gw->glxwin)
          {
             if (!glXMakeContextCurrent(gw->disp, gw->glxwin, gw->glxwin,
                                        gw->context))
               {
                  printf("Error: glXMakeContextCurrent(%p, %p, %p, %p)\n", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->glxwin, (void *)gw->context);
                  eng_window_free(gw);
                  return NULL;
               }
          }
        else
          {
             if (!glXMakeCurrent(gw->disp, gw->win, gw->context))
               {
                  printf("Error: glXMakeCurrent(%p, 0x%x, %p) failed\n", (void *)gw->disp, (unsigned int)gw->win, (void *)gw->context);
                  eng_window_free(gw);
                  return NULL;
               }
          }

        // FIXME: move this up to context creation

        vendor = glGetString(GL_VENDOR);
        renderer = glGetString(GL_RENDERER);
        version = glGetString(GL_VERSION);
        if (getenv("EVAS_GL_INFO"))
          {
             fprintf(stderr, "vendor: %s\n", vendor);
             fprintf(stderr, "renderer: %s\n", renderer);
             fprintf(stderr, "version: %s\n", version);
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
     }
#endif

   gw->gl_context = glsym_evas_gl_common_context_new();
   if (!gw->gl_context)
     {
        eng_window_free(gw);
        return NULL;
     }
#ifdef GL_GLES
   gw->gl_context->egldisp = gw->egl_disp;
   gw->gl_context->eglctxt = gw->egl_context[0];
#endif
   eng_window_use(gw);
   glsym_evas_gl_common_context_resize(gw->gl_context, w, h, rot);
   gw->surf = 1;
   return gw;
   (void) (indirect = 0);
}

void
eng_window_free(Outbuf *gw)
{
   int ref = 0;
   win_count--;
   eng_window_use(gw);
   if (gw == _evas_gl_x11_window) _evas_gl_x11_window = NULL;
   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        glsym_evas_gl_common_context_free(gw->gl_context);
     }
#ifdef GL_GLES
   eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   if (gw->egl_surface[0] != EGL_NO_SURFACE)
      eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
   if (gw->egl_context[0] != context)
     eglDestroyContext(gw->egl_disp, gw->egl_context[0]);
   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eglTerminate(gw->egl_disp);
        eglReleaseThread();
        context = EGL_NO_CONTEXT;
     }
#else
   if (gw->glxwin)
     {
        glXMakeContextCurrent(gw->disp, 0, 0, gw->context);
        glXDestroyWindow(gw->disp, gw->glxwin);
     }
   if (ref == 0)
     {
        if (!gw->glxwin)
          {
             if (glXGetCurrentContext() == gw->context)
               glXMakeCurrent(gw->disp, 0, NULL);
          }
        if (context) glXDestroyContext(gw->disp, context);
        if (rgba_context) glXDestroyContext(gw->disp, rgba_context);
        context = 0;
        rgba_context = 0;
        fbconf = 0;
        rgba_fbconf = 0;
     }
   else if (!gw->glxwin)
     {
        if (glXGetCurrentDrawable() == gw->win)
          glXMakeCurrent(gw->disp, 0, gw->context);
     }
#endif
   free(gw);
}

static Eina_Bool
eng_window_make_current(void *data, void *doit)
{
   Outbuf *gw = data;

#ifdef GL_GLES
   if (doit)
     {
        if (!eglMakeCurrent(gw->egl_disp, gw->egl_surface[0], gw->egl_surface[0], gw->egl_context[0]))
          return EINA_FALSE;
     }
   else
     {
        if (!eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
          return EINA_FALSE;
     }
#else
   if (doit)
     {
        if (gw->glxwin)
          {
             if (!glXMakeContextCurrent(gw->disp, gw->glxwin, gw->glxwin, gw->context))
               {
                  ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->glxwin, (void *)gw->context);
                  return EINA_FALSE;
               }
          }
        else
          {
             if (!glXMakeCurrent(gw->disp, gw->win, gw->context))
               {
                  ERR("glXMakeCurrent(%p, 0x%x, %p) failed", gw->disp, (unsigned int)gw->win, (void *)gw->context);
                  return EINA_FALSE;
               }
          }
     }
   else
     {
        if (!glXMakeCurrent(gw->disp, None, NULL))
          return EINA_FALSE;
     }
#endif
   return EINA_TRUE;
}

void
eng_window_use(Outbuf *gw)
{
   Eina_Bool force_use = EINA_FALSE;

   glsym_evas_gl_preload_render_lock(eng_window_make_current, gw);
#ifdef GL_GLES
   if (_evas_gl_x11_window)
     {
        if (eglGetCurrentContext() != _evas_gl_x11_window->egl_context[0])
           force_use = EINA_TRUE;
     }
#else
   if (_evas_gl_x11_window)
     {
        if (glXGetCurrentContext() != _evas_gl_x11_window->context)
           force_use = EINA_TRUE;
     }
#endif
   if ((_evas_gl_x11_window != gw) || (force_use))
     {
        if (_evas_gl_x11_window)
          {
             glsym_evas_gl_common_context_use(_evas_gl_x11_window->gl_context);
             glsym_evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
          }
        _evas_gl_x11_window = gw;
        if (gw)
          {
// EGL / GLES
#ifdef GL_GLES
             if (gw->egl_surface[0] != EGL_NO_SURFACE)
               {
                  if (eglMakeCurrent(gw->egl_disp,
                                     gw->egl_surface[0],
                                     gw->egl_surface[0],
                                     gw->egl_context[0]) == EGL_FALSE)
                    {
                       ERR("eglMakeCurrent() failed!");
                    }
               }
// GLX
#else
             if (gw->glxwin)
               {
                  if (!glXMakeContextCurrent(gw->disp, gw->glxwin, gw->glxwin,
                                             gw->context))
                    {
                       ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->glxwin, (void *)gw->context);
                    }
               }
             else
               {
                  if (!glXMakeCurrent(gw->disp, gw->win, gw->context))
                    {
                       ERR("glXMakeCurrent(%p, 0x%x, %p) failed", gw->disp, (unsigned int)gw->win, (void *)gw->context);
                    }
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
   if (getenv("EVAS_GL_INFO"))
      printf("unsurf %p\n", gw);
#ifdef GL_GLES
   if (_evas_gl_x11_window)
      glsym_evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
   if (_evas_gl_x11_window == gw)
     {
        eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (gw->egl_surface[0] != EGL_NO_SURFACE)
           eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
        gw->egl_surface[0] = EGL_NO_SURFACE;
        _evas_gl_x11_window = NULL;
     }
#else
   if (gw->glxwin)
      {
         glXDestroyWindow(gw->disp, gw->glxwin);
         gw->glxwin = 0;
      }
   else
     {
     }
#endif
   gw->surf = 0;
}

void
eng_window_resurf(Outbuf *gw)
{
   if (gw->surf) return;
   if (getenv("EVAS_GL_INFO"))
      printf("resurf %p\n", gw);
#ifdef GL_GLES
   gw->egl_surface[0] = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                               (EGLNativeWindowType)gw->win,
                                               NULL);
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        ERR("eglCreateWindowSurface() fail for %#x. code=%#x",
            (unsigned int)gw->win, eglGetError());
        return;
     }
   if (eglMakeCurrent(gw->egl_disp,
                      gw->egl_surface[0],
                      gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() failed!");
     }
#else
#ifdef NEWGL
   if (gw->alpha)
     gw->glxwin = glXCreateWindow(gw->disp, rgba_fbconf, gw->win, NULL);
   else
     gw->glxwin = glXCreateWindow(gw->disp, fbconf, gw->win, NULL);
   if (!glXMakeContextCurrent(gw->disp, gw->glxwin, gw->glxwin,
                              gw->context))
     {
        ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->glxwin, (void *)gw->glxwin, (void *)gw->context);
     }
#else
   if (!glXMakeCurrent(gw->disp, gw->win, gw->context))
     {
        ERR("glXMakeCurrent(%p, 0x%x, %p) failed", (void *)gw->disp, (unsigned int)gw->win, (void *)gw->context);
     }
#endif
#endif
   gw->surf = 1;
}

void *
eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return NULL;
   if (!einfo->info.display) return NULL;
   if (!_evas_gl_x11_vi)
     {
        int alpha;

// EGL / GLES
#ifdef GL_GLES
        for (alpha = 0; alpha < 2; alpha++)
          {
             int depth = DefaultDepth(einfo->info.display,
                                      einfo->info.screen);
             if (alpha)
               {
                  XVisualInfo *xvi, vi_in;
                  int nvi, i;
                  XRenderPictFormat *fmt;

                  vi_in.screen = einfo->info.screen;
                  vi_in.depth = 32;
                  vi_in.class = TrueColor;
                  xvi = XGetVisualInfo(einfo->info.display,
                                       VisualScreenMask | VisualDepthMask |
                                       VisualClassMask,
                                       &vi_in, &nvi);
                  if (xvi)
                    {
                       for (i = 0; i < nvi; i++)
                         {
                            fmt = XRenderFindVisualFormat(einfo->info.display,
                                                          xvi[i].visual);
                            if ((fmt->type == PictTypeDirect) &&
                                (fmt->direct.alphaMask))
                              {
                                 _evas_gl_x11_rgba_vi =
                                   calloc(1, sizeof(XVisualInfo));
                                 if (_evas_gl_x11_rgba_vi)
                                   memcpy(_evas_gl_x11_rgba_vi,
                                          &(xvi[i]), sizeof(XVisualInfo));
                                 break;
                              }
                         }
                       XFree (xvi);
                    }
               }
             else
               {
                  _evas_gl_x11_vi = calloc(1, sizeof(XVisualInfo));
                  XMatchVisualInfo(einfo->info.display,
                                   einfo->info.screen, depth, TrueColor,
                                   _evas_gl_x11_vi);
               }
          }
// GLX
#else
        for (alpha = 0; alpha < 2; alpha++)
          {
             int config_attrs[40];
             GLXFBConfig *configs = NULL, config = 0;
             int i, num;

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
             config_attrs[i++] = GLX_DEPTH_SIZE;
             config_attrs[i++] = 0;
             config_attrs[i++] = GLX_STENCIL_SIZE;
             config_attrs[i++] = 0;
             config_attrs[i++] = GLX_AUX_BUFFERS;
             config_attrs[i++] = 0;
             config_attrs[i++] = GLX_STEREO;
             config_attrs[i++] = 0;
             config_attrs[i++] = GLX_TRANSPARENT_TYPE;
             config_attrs[i++] = GLX_NONE;//GLX_NONE;//GLX_TRANSPARENT_INDEX//GLX_TRANSPARENT_RGB;
             config_attrs[i++] = 0;

             configs = glXChooseFBConfig(einfo->info.display,
                                         einfo->info.screen,
                                         config_attrs, &num);
             if ((!configs) || (num < 1))
               {
                  ERR("glXChooseFBConfig returned no configs");
                  return NULL;
               }
             for (i = 0; i < num; i++)
               {
                  XVisualInfo *visinfo;
                  XRenderPictFormat *format = NULL;

                  visinfo = glXGetVisualFromFBConfig(einfo->info.display,
                                                     configs[i]);
                  if (!visinfo) continue;
                  if (!alpha)
                    {
                       config = configs[i];
                       // ensure depth matches default depth!
                       if (DefaultDepth(einfo->info.display, 0) ==
                           visinfo->depth)
                         {
                            _evas_gl_x11_vi = malloc(sizeof(XVisualInfo));
                            memcpy(_evas_gl_x11_vi, visinfo,
                                   sizeof(XVisualInfo));
                            fbconf = config;
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
                       if (format->direct.alphaMask > 0)
                         {
                            config = configs[i];
                            _evas_gl_x11_rgba_vi = malloc(sizeof(XVisualInfo));
                            memcpy(_evas_gl_x11_rgba_vi, visinfo, sizeof(XVisualInfo));
                            rgba_fbconf = config;
                            XFree(visinfo);
                            break;
                         }
                    }
                  XFree(visinfo);
               }
          }
#endif
     }
   if (!_evas_gl_x11_vi) return NULL;
   if (einfo->info.destination_alpha)
     {
// EGL / GLES
#ifdef GL_GLES
        if (_evas_gl_x11_rgba_vi) return _evas_gl_x11_rgba_vi->visual;
#else
//# ifdef NEWGL
        if (_evas_gl_x11_rgba_vi) return _evas_gl_x11_rgba_vi->visual;
//# endif
#endif
     }
   return _evas_gl_x11_vi->visual;
}

Colormap
eng_best_colormap_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   if (!_evas_gl_x11_vi) eng_best_visual_get(einfo);
   if (!_evas_gl_x11_vi) return 0;
   if (einfo->info.destination_alpha)
     {
        if (!_evas_gl_x11_rgba_cmap)
          _evas_gl_x11_rgba_cmap =
          XCreateColormap(einfo->info.display,
                          RootWindow(einfo->info.display,
                                     einfo->info.screen),
                          _evas_gl_x11_rgba_vi->visual,
                          0);
        return _evas_gl_x11_rgba_cmap;
     }
   if (!_evas_gl_x11_cmap)
     _evas_gl_x11_cmap =
     XCreateColormap(einfo->info.display,
                     RootWindow(einfo->info.display,
                                einfo->info.screen),
                     _evas_gl_x11_vi->visual,
                     0);
   return _evas_gl_x11_cmap;
}

int
eng_best_depth_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   if (!_evas_gl_x11_vi) eng_best_visual_get(einfo);
   if (!_evas_gl_x11_vi) return 0;
   if (einfo->info.destination_alpha)
     {
        if (_evas_gl_x11_rgba_vi) return _evas_gl_x11_rgba_vi->depth;
     }
   return _evas_gl_x11_vi->depth;
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
   ctx->context = eglCreateContext(win->egl_disp, win->egl_config,
                                   win->egl_context[0], context_attrs);

   if (!ctx->context)
     {
        ERR("EGL context creation failed.");
        goto error;
     }

   ctx->display = win->egl_disp;
   ctx->surface = win->egl_surface[0];
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
   if (eglMakeCurrent(ctx->display, ctx->surface,
                      ctx->surface, ctx->context) == EGL_FALSE)
     {
        ERR("eglMakeCurrent() failed.");
     }
#else
   if (ctx->glxwin)
     {
        if (!glXMakeContextCurrent(ctx->display, ctx->glxwin,
                                   ctx->glxwin, ctx->context))
          {
             ERR("glXMakeContextCurrent(%p, %p, %p, %p) faild.",
                 (void *)ctx->display, (void *)ctx->glxwin,
                 (void *)ctx->glxwin, (void *)ctx->context);
          }
     }
   else
     {
        if (!glXMakeCurrent(ctx->display, ctx->win, ctx->context))
          {
             ERR("glXMakeCurrent(%p, %p, %p) failed.",
                 (void *)ctx->display, (void *)ctx->win,
                 (void *)ctx->context);
          }
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

Render_Engine_Swap_Mode
eng_outbuf_swap_mode(Outbuf *ob)
{
   if (ob->swap_mode == MODE_AUTO && extn_have_buffer_age)
     {
        Render_Engine_Swap_Mode swap_mode;
#ifdef GL_GLES
        EGLint age = 0;

        if (!eglQuerySurface(ob->egl_disp,
                             ob->egl_surface[0],
                             EGL_BUFFER_AGE_EXT, &age))
          age = 0;
#else
        unsigned int age = 0;

        if (glsym_glXQueryDrawable)
          {
             if (ob->glxwin)
               glsym_glXQueryDrawable(ob->disp,
                                      ob->glxwin,
                                      GLX_BACK_BUFFER_AGE_EXT,
                                      &age);
             else
               glsym_glXQueryDrawable(ob->disp,
                                      ob->win,
                                      GLX_BACK_BUFFER_AGE_EXT,
                                      &age);
          }
#endif
        if (age == 1) swap_mode = MODE_COPY;
        else if (age == 2) swap_mode = MODE_DOUBLE;
        else if (age == 3) swap_mode = MODE_TRIPLE;
        else if (age == 4) swap_mode = MODE_QUADRUPLE;
        else swap_mode = MODE_FULL;
        if ((int)age != ob->prev_age) swap_mode = MODE_FULL;
        ob->prev_age = age;

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

void*
eng_outbuf_new_region_for_update(Outbuf *ob,
                                 int x, int y, int w, int h,
                                 int *cx EINA_UNUSED, int *cy EINA_UNUSED,
                                 int *cw EINA_UNUSED, int *ch EINA_UNUSED)
{
   if (w == ob->w && h == ob->h)
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
#ifdef GL_GLES
   // this is needed to make sure all previous rendering is flushed to
   // buffers/surfaces
   // previous rendering should be done and swapped
//xx   if (!safe_native) eglWaitNative(EGL_CORE_NATIVE_ENGINE);
//   if (eglGetError() != EGL_SUCCESS)
//     {
//        printf("Error:  eglWaitNative(EGL_CORE_NATIVE_ENGINE) fail.\n");
//     }
#else
   // previous rendering should be done and swapped
//xx   if (!safe_native) glXWaitX();
#endif
}

void
eng_outbuf_push_free_region_for_update(Outbuf *ob EINA_UNUSED,
                                       RGBA_Image *update EINA_UNUSED)
{
   /* Nothing to do here as we don't really create an image per area */
}

void
eng_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects, Evas_Render_Mode render_mode)
{
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
   if (ob->info->callback.pre_swap)
     {
        ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);
     }
   if ((glsym_eglSwapBuffersWithDamage) && (ob->swap_mode != MODE_FULL))
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
                  int gw, gh;

                  gw = ob->gl_context->w;
                  gh = ob->gl_context->h;
                  switch (ob->rot)
                    {
                     case 0:
                       result[i + 0] = r->x;
                       result[i + 1] = gh - (r->y + r->h);
                       result[i + 2] = r->w;
                       result[i + 3] = r->h;
                       break;
                     case 90:
                       result[i + 0] = r->y;
                       result[i + 1] = r->x;
                       result[i + 2] = r->h;
                       result[i + 3] = r->w;
                       break;
                     case 180:
                       result[i + 0] = gw - (r->x + r->w);
                       result[i + 1] = r->y;
                       result[i + 2] = r->w;
                       result[i + 3] = r->h;
                       break;
                     case 270:
                       result[i + 0] = gh - (r->y + r->h);
                       result[i + 1] = gw - (r->x + r->w);
                       result[i + 2] = r->h;
                       result[i + 3] = r->w;
                       break;
                     default:
                       result[i + 0] = r->x;
                       result[i + 1] = gh - (r->y + r->h);
                       result[i + 2] = r->w;
                       result[i + 3] = r->h;
                       break;
                    }
                  i += 4;
               }
             glsym_eglSwapBuffersWithDamage(ob->egl_disp,
                                            ob->egl_surface[0],
                                            result, num);
          }
     }
   else
      eglSwapBuffers(ob->egl_disp, ob->egl_surface[0]);

//xx   if (!safe_native) eglWaitGL();
   if (ob->info->callback.post_swap)
     {
        ob->info->callback.post_swap(ob->info->callback.data, ob->evas);
     }
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
   if (ob->info->callback.pre_swap)
     {
        ob->info->callback.pre_swap(ob->info->callback.data, ob->evas);
     }
   // XXX: if partial swaps can be done use re->rects
//   measure(0, "swap");
   glXSwapBuffers(ob->disp, ob->win);
//   measure(1, "swap");
   if (ob->info->callback.post_swap)
     {
        ob->info->callback.post_swap(ob->info->callback.data, ob->evas);
     }
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
