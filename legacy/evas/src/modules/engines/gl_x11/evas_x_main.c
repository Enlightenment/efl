#include "evas_engine.h"

static Evas_GL_X11_Window *_evas_gl_x11_window = NULL;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
static EGLContext context = EGL_NO_CONTEXT;
#else
// FIXME: this will only work for 1 display connection (glx land can have > 1)
static GLXContext context = 0;
#endif

XVisualInfo *_evas_gl_x11_vi = NULL;
Colormap     _evas_gl_x11_cmap = 0;

Evas_GL_X11_Window *
eng_window_new(Display *disp,
	       Window   win,
	       int      screen,
	       Visual  *vis,
	       Colormap cmap,
	       int      depth,
	       int      w,
	       int      h)
{
   Evas_GL_X11_Window *gw;
   int context_attrs[3];
   int config_attrs[20];
   int major_version, minor_version;
   int num_config;
   
   if (!_evas_gl_x11_vi) return NULL;
   
   gw = calloc(1, sizeof(Evas_GL_X11_Window));
   if (!gw) return NULL;
   gw->disp = disp;
   gw->win = win;
   gw->screen = screen;
   gw->visual = vis;
   gw->colormap = cmap;
   gw->depth = depth;

   gw->visualinfo = _evas_gl_x11_vi;

// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;
# if defined(GLES_VARIETY_S3C6410)
   if (_evas_gl_x11_vi->depth == 16) // 16bpp
     {
        config_attrs[0]  = EGL_SURFACE_TYPE;
        config_attrs[1]  = EGL_WINDOW_BIT;
        config_attrs[2]  = EGL_RENDERABLE_TYPE;
        config_attrs[3]  = EGL_OPENGL_ES2_BIT;
        config_attrs[4]  = EGL_RED_SIZE;
        config_attrs[5]  = 5;
        config_attrs[6]  = EGL_GREEN_SIZE;
        config_attrs[7]  = 6;
        config_attrs[8]  = EGL_BLUE_SIZE;
        config_attrs[9]  = 5;
        config_attrs[10] = EGL_DEPTH_SIZE;
        config_attrs[11] = 0;
        config_attrs[12] = EGL_STENCIL_SIZE;
        config_attrs[13] = 0;
        config_attrs[14] = EGL_NONE;
     }
   else // 24/32bit. no one does 8bpp anymore. and 15bpp... dead
     {
        config_attrs[0]  = EGL_SURFACE_TYPE;
        config_attrs[1]  = EGL_WINDOW_BIT;
        config_attrs[2]  = EGL_RENDERABLE_TYPE;
        config_attrs[3]  = EGL_OPENGL_ES2_BIT;
        config_attrs[4]  = EGL_RED_SIZE;
        config_attrs[5]  = 8;
        config_attrs[6]  = EGL_GREEN_SIZE;
        config_attrs[7]  = 8;
        config_attrs[8]  = EGL_BLUE_SIZE;
        config_attrs[9]  = 8;
        config_attrs[10] = EGL_DEPTH_SIZE;
        config_attrs[11] = 0;
        config_attrs[12] = EGL_STENCIL_SIZE;
        config_attrs[13] = 0;
        config_attrs[14] = EGL_NONE;
     }
# elif defined(GLES_VARIETY_SGX)
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;
   
   config_attrs[0] = EGL_SURFACE_TYPE;
   config_attrs[1] = EGL_WINDOW_BIT;
   config_attrs[2] = EGL_RENDERABLE_TYPE;
   config_attrs[3] = EGL_OPENGL_ES2_BIT;
   config_attrs[4] = EGL_NONE;
# endif
   gw->egl_disp= eglGetDisplay((EGLNativeDisplayType)(gw->disp));
   if (!gw->egl_disp)
     {
        printf("Error: eglGetDisplay() fail.\n");
     }
   if (!eglInitialize(gw->egl_disp, &major_version, &minor_version))
     {
        printf("Error: eglInitialize() fail.\n");
     }
   eglBindAPI(EGL_OPENGL_ES_API);
   if (eglGetError() != EGL_SUCCESS)
     {
        printf("Error: eglBindAPI() fail.\n");
     }
   if (!eglChooseConfig(gw->egl_disp, config_attrs, &gw->egl_config,
                        1, &num_config) || (num_config != 1))
     {
        printf("Error: eglChooseConfig() fail.\n");
     }
   gw->egl_surface[0] = eglCreateWindowSurface(gw->egl_disp, gw->egl_config,
                                               (EGLNativeWindowType)gw->win,
                                               NULL);
   if (gw->egl_surface[0] == EGL_NO_SURFACE)
     {
        printf("Error: eglCreateWindowSurface() fail.\n", gw->win);
     }
   if (context == EGL_NO_CONTEXT)
     context = eglCreateContext(gw->egl_disp, gw->egl_config, NULL, 
                                context_attrs);
   gw->egl_context[0] = context;
   if (gw->egl_context[0] == EGL_NO_CONTEXT)
     {
        printf("Error: eglCreateContext() fail.\n");
     }
   if (eglMakeCurrent(gw->egl_disp, 
                      gw->egl_surface[0], 
                      gw->egl_surface[0],
                      gw->egl_context[0]) == EGL_FALSE)
     {
        printf("Error: eglMakeCurrent() fail.\n");
     }
// GLX   
#else
   if (!context)
     context = glXCreateContext(disp, gw->visualinfo, NULL, GL_TRUE);
   gw->context = context;
   
   glXMakeCurrent(gw->disp, gw->win, gw->context);
#endif
   _evas_gl_x11_window = gw;
   
   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     {
	free(gw);
	return NULL;
     }
   evas_gl_common_context_use(gw->gl_context);
   evas_gl_common_context_resize(gw->gl_context, w, h);
   return gw;
}

void
eng_window_free(Evas_GL_X11_Window *gw)
{
   if (gw == _evas_gl_x11_window) _evas_gl_x11_window = NULL;
   evas_gl_common_context_free(gw->gl_context);
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (gw->egl_surface[0] != EGL_NO_SURFACE)
     eglDestroySurface(gw->egl_disp, gw->egl_surface[0]);
#else
   // FIXME: refcount context   
   //   glXDestroyContext(gw->disp, gw->context);
#endif
   free(gw);
}

void
eng_window_use(Evas_GL_X11_Window *gw)
{
   if (_evas_gl_x11_window != gw)
     {
        if (_evas_gl_x11_window)
          evas_gl_common_context_flush(_evas_gl_x11_window->gl_context);
	_evas_gl_x11_window = gw;
// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        if (eglMakeCurrent(gw->egl_disp, 
                           gw->egl_surface[0], 
                           gw->egl_surface[0],
                           gw->egl_context[0]) == EGL_FALSE)
          {
             printf("Error: eglMakeCurrent() failed!\n");
          }
// GLX        
#else
        glXMakeCurrent(gw->disp, gw->win, gw->context);
#endif
     }
   evas_gl_common_context_use(gw->gl_context);
}

Visual *
eng_best_visual_get(Display *disp, int screen)
{
   if (!disp) return NULL;
   if (!_evas_gl_x11_vi)
     {
// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        int depth = DefaultDepth(disp, screen);
        _evas_gl_x11_vi = calloc(1, sizeof(XVisualInfo));
        XMatchVisualInfo(disp, screen, depth, TrueColor, _evas_gl_x11_vi);
// GLX
#else

#if 0 // use this if we want alpha
        int config_attrs[20];
        GLXFBConfig *configs = NULL, config = 0;
        int i, num;
        
        config_attrs[0] = GLX_DRAWABLE_TYPE;
        config_attrs[1] = GLX_WINDOW_BIT;
        config_attrs[2] = GLX_DOUBLEBUFFER;
        config_attrs[3] = 1;
        config_attrs[4] = GLX_RED_SIZE;
        config_attrs[5] = 1;
        config_attrs[6] = GLX_GREEN_SIZE;
        config_attrs[7] = 1;
        config_attrs[8] = GLX_BLUE_SIZE;
        config_attrs[9] = 1;
        config_attrs[10] = None;
        
        // if rgba
        config_attrs[10] = GLX_ALPHA_SIZE;
        config_attrs[11] = 1;
        config_attrs[12] = GLX_RENDER_TYPE;
        config_attrs[13] = GLX_RGBA_BIT;
        config_attrs[14] = None;
        
        configs = glXChooseFBConfig(disp, 0, config_attrs, &num);
        for (i = 0; i < num; i++)
          {
             XVisualInfo *visinfo;
             XRenderPictFormat *format;
             
             visinfo = glXGetVisualFromFBConfig(disp, configs[i]);
             if (!visinfo) continue;
             format = XRenderFindVisualFormat(disp, visinfo->visual);
             if (!format) continue;
             
             if (format->direct.alphaMask > 0)
               {
                  config = configs[i];
                  _evas_gl_x11_vi = visinfo;
                  break;
               }
             XFree(visinfo);
          }
#else   
        int _evas_gl_x11_configuration[] =
          {
             GLX_RGBA, GLX_DOUBLEBUFFER,
               GLX_LEVEL, 0,
               GLX_DEPTH_SIZE, 0,
               GLX_STENCIL_SIZE, 0,
               GLX_RED_SIZE, 1,
               GLX_GREEN_SIZE,1,
               GLX_BLUE_SIZE, 1,
               None
          };
        _evas_gl_x11_vi = glXChooseVisual(disp, screen,
                                          _evas_gl_x11_configuration);
#endif
        
#endif
     }
   if (!_evas_gl_x11_vi) return NULL;
   return _evas_gl_x11_vi->visual;
}

Colormap
eng_best_colormap_get(Display *disp, int screen)
{
   if (!disp) return 0;
   if (!_evas_gl_x11_vi) eng_best_visual_get(disp, screen);
   if (!_evas_gl_x11_vi) return 0;
   if (!_evas_gl_x11_cmap)
     {
        _evas_gl_x11_cmap = XCreateColormap(disp, 
                                            RootWindow(disp, screen),
                                            _evas_gl_x11_vi->visual, 
                                            0);
     }
   return _evas_gl_x11_cmap;
}
                                 
int
eng_best_depth_get(Display *disp, int screen)
{
   if (!disp) return 0;
   if (!_evas_gl_x11_vi) eng_best_visual_get(disp, screen);
   if (!_evas_gl_x11_vi) return 0;
   return _evas_gl_x11_vi->depth;
}
