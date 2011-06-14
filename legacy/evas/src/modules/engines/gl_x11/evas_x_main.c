#include "evas_engine.h"

static Evas_GL_X11_Window *_evas_gl_x11_window = NULL;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
static EGLContext context = EGL_NO_CONTEXT;
#else
// FIXME: this will only work for 1 display connection (glx land can have > 1)
static GLXContext context = 0;
static GLXContext rgba_context = 0;
static GLXFBConfig fbconf = 0;
static GLXFBConfig rgba_fbconf = 0;
#endif

// fixme: something is up/wrong here - dont know what tho...
// #define NEWGL 1

static XVisualInfo *_evas_gl_x11_vi = NULL;
static XVisualInfo *_evas_gl_x11_rgba_vi = NULL;
static Colormap     _evas_gl_x11_cmap = 0;
static Colormap     _evas_gl_x11_rgba_cmap = 0;

static int win_count = 0;

Evas_GL_X11_Window *
eng_window_new(Display *disp,
	       Window   win,
	       int      screen,
	       Visual  *vis,
	       Colormap cmap,
	       int      depth,
	       int      w,
	       int      h,
               int      indirect,
               int      alpha,
               int      rot)
{
   Evas_GL_X11_Window *gw;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   int context_attrs[3];
   int config_attrs[40];
   int major_version, minor_version;
   int num_config, n = 0;
#endif   
   XVisualInfo *vi_use;
   const GLubyte *vendor, *renderer, *version;
   
   if (!_evas_gl_x11_vi) return NULL;
   
   gw = calloc(1, sizeof(Evas_GL_X11_Window));
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

   vi_use = _evas_gl_x11_vi;
   if (gw->alpha)
     {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        if (_evas_gl_x11_rgba_vi)
          {
             vi_use = _evas_gl_x11_rgba_vi;
          }
#else
#ifdef NEWGL
        if (_evas_gl_x11_rgba_vi)
          {
             vi_use = _evas_gl_x11_rgba_vi;
          }
#endif        
#endif        
     }
   gw->visualinfo = vi_use;
   
// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

# if defined(GLES_VARIETY_S3C6410)
   if (gw->visualinfo->depth == 16) // 16bpp
     {
        config_attrs[n++] = EGL_SURFACE_TYPE;
        config_attrs[n++] = EGL_WINDOW_BIT;
        config_attrs[n++] = EGL_RENDERABLE_TYPE;
        config_attrs[n++] = EGL_OPENGL_ES2_BIT;
        config_attrs[n++] = EGL_RED_SIZE;
        config_attrs[n++] = 5;
        config_attrs[n++] = EGL_GREEN_SIZE;
        config_attrs[n++] = 6;
        config_attrs[n++] = EGL_BLUE_SIZE;
        config_attrs[n++] = 5;
        config_attrs[n++] = EGL_DEPTH_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_STENCIL_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_NONE;
     }
   else // 24/32bit. no one does 8bpp anymore. and 15bpp... dead
     {
        config_attrs[n++] = EGL_SURFACE_TYPE;
        config_attrs[n++] = EGL_WINDOW_BIT;
        config_attrs[n++] = EGL_RENDERABLE_TYPE;
        config_attrs[n++] = EGL_OPENGL_ES2_BIT;
        config_attrs[n++] = EGL_RED_SIZE;
        config_attrs[n++] = 8;
        config_attrs[n++] = EGL_GREEN_SIZE;
        config_attrs[n++] = 8;
        config_attrs[n++] = EGL_BLUE_SIZE;
        config_attrs[n++] = 8;
        config_attrs[n++] = EGL_DEPTH_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_STENCIL_SIZE;
        config_attrs[n++] = 0;
        config_attrs[n++] = EGL_NONE;
     }
# elif defined(GLES_VARIETY_SGX)
   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_WINDOW_BIT;
   config_attrs[n++] = EGL_RENDERABLE_TYPE;
   config_attrs[n++] = EGL_OPENGL_ES2_BIT;
#if 0
// FIXME: n900 - omap3 sgx libs break here
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 1;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 1;
// FIXME: end n900 breakage   
#endif
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
# endif
   
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
   if (context == EGL_NO_CONTEXT)
     context = eglCreateContext(gw->egl_disp, gw->egl_config, NULL, 
                                context_attrs);
   gw->egl_context[0] = context;
   if (gw->egl_context[0] == EGL_NO_CONTEXT)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
	eng_window_free(gw);
        return NULL;
     }
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
// GLX   
#else
   if (!context)
     {
#ifdef NEWGL        
        if (indirect)
          context = glXCreateNewContext(gw->disp, fbconf, 
                                        GLX_RGBA_TYPE, NULL, 
                                        GL_TRUE);
        else
          context = glXCreateNewContext(gw->disp, fbconf, 
                                        GLX_RGBA_TYPE, NULL, 
                                        GL_FALSE);
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
                                             GL_TRUE);
        else
          rgba_context = glXCreateNewContext(gw->disp, rgba_fbconf, 
                                             GLX_RGBA_TYPE, context, 
                                             GL_FALSE);
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
        int i, j,  num;
        GLXFBConfig *fbc;
        int blacklist = 0;

        if (gw->glxwin)
          {
             if (!glXMakeContextCurrent(gw->disp, gw->glxwin, gw->glxwin, 
                                        gw->context))
               {
                  printf("Error: glXMakeContextCurrent(%p, %p, %p, %p)\n", (void *)gw->disp, (void *)gw->win, (void *)gw->win, (void *)gw->context);
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
        if (blacklist)
          {
             ERR("OpenGL Driver blacklisted:");
             ERR("Vendor: %s", (const char *)vendor);
             ERR("Renderer: %s", (const char *)renderer);
             ERR("Version: %s", (const char *)version);
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
        
        fbc = glXGetFBConfigs(gw->disp, screen, &num);
        if (!fbc)
          {
             ERR("glXGetFBConfigs() returned no fb configs");
             eng_window_free(gw);
             return NULL;
          }
        for (i = 0; i <= 32; i++)
          {
             for (j = 0; j < num; j++)
               {
                  XVisualInfo *vi;
                  int vd;
                  int alph, val, dbuf, stencil, tdepth;
                  int rgba;
                  
                  vi = glXGetVisualFromFBConfig(gw->disp, fbc[j]);
                  if (!vi) continue;
                  vd = vi->depth;
                  XFree(vi);
                  
                  if (vd != i) continue;
                  
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_ALPHA_SIZE, &alph);
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_BUFFER_SIZE, &val);
                  
                  if ((val != i) && ((val - alph) != i)) continue;
                  
                  val = 0;
                  rgba = 0;
                  
                  if (i == 32)
                    {
                       glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_BIND_TO_TEXTURE_RGBA_EXT, &val);
                       if (val)
                         {
                            rgba = 1;
                            gw->depth_cfg[i].tex_format = GLX_TEXTURE_FORMAT_RGBA_EXT;
                         }
                    }
                  if (!val)
                    {
                       if (rgba) continue;
                       glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_BIND_TO_TEXTURE_RGB_EXT, &val);
                       if (!val) continue;
                       gw->depth_cfg[i].tex_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                    }
                  
                  dbuf = 0x7fff;
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_DOUBLEBUFFER, &val);
                  if (val > dbuf) continue;
                  dbuf = val;
                  
                  stencil = 0x7fff;
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_STENCIL_SIZE, &val);
                  if (val > stencil) continue;
                  stencil = val;
                  
                  tdepth = 0x7fff;
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_DEPTH_SIZE, &val);
                  if (val > tdepth) continue;
                  tdepth = val;
                  
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_BIND_TO_MIPMAP_TEXTURE_EXT, &val);
                  if (val < 0) continue;
                  gw->depth_cfg[i].mipmap = val;
                  
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_Y_INVERTED_EXT, &val);
                  gw->depth_cfg[i].yinvert = val;
                  
                  glXGetFBConfigAttrib(gw->disp, fbc[j], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &val);
                  gw->depth_cfg[i].tex_target = val;
                  
                  gw->depth_cfg[i].fbc = fbc[j];
               }
          }
        XFree(fbc);
        if (!gw->depth_cfg[DefaultDepth(gw->disp, screen)].fbc)
          {
             WRN("texture from pixmap not going to work");
          }
     }
#endif
   
   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     {
	eng_window_free(gw);
	return NULL;
     }
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   gw->gl_context->egldisp = gw->egl_disp;
#endif   
   eng_window_use(gw);
   evas_gl_common_context_resize(gw->gl_context, w, h, rot);
   gw->surf = 1;
   return gw;
   indirect = 0;
}

void
eng_window_free(Evas_GL_X11_Window *gw)
{
   int ref = 0;
   win_count--;
   eng_window_use(gw);
   if (gw == _evas_gl_x11_window) _evas_gl_x11_window = NULL;
   if (gw->gl_context)
     {
        ref = gw->gl_context->references - 1;
        evas_gl_common_context_free(gw->gl_context);
     }
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (gw->egl_surface[0] != EGL_NO_SURFACE)
      eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
   if (ref == 0)
     {
        if (context) eglDestroyContext(gw->egl_disp, context);
        eglTerminate(gw->egl_disp);
        context = EGL_NO_CONTEXT;
     }
   eglMakeCurrent(gw->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
   if (gw->glxwin) glXDestroyWindow(gw->disp, gw->glxwin);
   if (ref == 0)
     {
        if (context) glXDestroyContext(gw->disp, context);
        if (rgba_context) glXDestroyContext(gw->disp, rgba_context);
        context = 0;
        rgba_context = 0;
        fbconf = 0;
        rgba_fbconf = 0;
     }
#endif
   free(gw);
}

void
eng_window_use(Evas_GL_X11_Window *gw)
{
   Eina_Bool force_use = EINA_FALSE;
   
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (_evas_gl_x11_window)
     {
        if ((eglGetCurrentContext() != 
             _evas_gl_x11_window->egl_context[0]) ||
            (eglGetCurrentSurface(EGL_READ) != 
                _evas_gl_x11_window->egl_surface[0]) ||
            (eglGetCurrentSurface(EGL_DRAW) != 
                _evas_gl_x11_window->egl_surface[0]))
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
             evas_gl_common_context_use(_evas_gl_x11_window->gl_context);
             evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
          }
        _evas_gl_x11_window = gw;
        if (gw)
          {
// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
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
                   ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->win, (void *)gw->win, (void *)gw->context);
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
   if (gw) evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_unsurf(Evas_GL_X11_Window *gw)
{
   if (!gw->surf) return;
   if (!getenv("EVAS_GL_WIN_RESURF")) return;
   if (getenv("EVAS_GL_INFO"))
      printf("unsurf %p\n", gw);
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (_evas_gl_x11_window)
      evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
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
      }
   else
     {
     }
#endif
   gw->surf = 0;
}

void
eng_window_resurf(Evas_GL_X11_Window *gw)
{
   if (gw->surf) return;
   if (getenv("EVAS_GL_INFO"))
      printf("resurf %p\n", gw);
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
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
        ERR("glXMakeContextCurrent(%p, %p, %p, %p)", (void *)gw->disp, (void *)gw->win, (void *)gw->win, (void *)gw->context);
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

Visual *
eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return NULL;
   if (!einfo->info.display) return NULL;
   if (!_evas_gl_x11_vi)
     {
        int alpha;
        
// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
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
             config_attrs[i++] =1;
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
                       _evas_gl_x11_vi = malloc(sizeof(XVisualInfo));
                       memcpy(_evas_gl_x11_vi, visinfo, sizeof(XVisualInfo));
                       fbconf = config;
                       XFree(visinfo);
                       break;
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
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        if (_evas_gl_x11_rgba_vi) return _evas_gl_x11_rgba_vi->visual;
#else        
# ifdef NEWGL
        if (_evas_gl_x11_rgba_vi) return _evas_gl_x11_rgba_vi->visual;
# endif
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
