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
	       int      h,
               int      indirect)
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
        printf("Error: eglCreateWindowSurface() fail for 0x%x.\n", (unsigned int)gw->win);
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
     {
        if (indirect)
          context = glXCreateContext(disp, gw->visualinfo, NULL, GL_FALSE);
        else
          context = glXCreateContext(disp, gw->visualinfo, NULL, GL_TRUE);
        if (!context)
          context = glXCreateContext(disp, gw->visualinfo, NULL, GL_TRUE);
        if (!context)
          context = glXCreateContext(disp, gw->visualinfo, NULL, GL_FALSE);
     }
   gw->context = context;

   if (gw->context)
     {
        int i, j,  num;
        GLXFBConfig *fbc;
        const GLubyte *vendor, *renderer, *version;
        
        glXMakeCurrent(gw->disp, gw->win, gw->context);
        
        // FIXME: move this up to context creation

        vendor = glGetString(GL_VENDOR);
        renderer = glGetString(GL_RENDERER);
        version = glGetString(GL_VERSION);
        
        printf("vendor: %s\n", vendor);
        printf("renderer: %s\n", renderer);
        printf("version: %s\n", version);
        
        if (strstr(vendor, "NVIDIA"))
          {
             gw->detected.loose_binding = 1;
          }
        else
          {
             // noothing yet. add more cases and options over time
          }
        
        fbc = glXGetFBConfigs(disp, 0/* FIXME: assume screen 0 */, &num);
        if (!fbc)
          {
             printf("ERROR: glXGetFBConfigs() returned no fb configs\n");
          }
        for (i = 0; i <= 32; i++)
          {
             for (j = 0; j < num; j++)
               {
                  XVisualInfo *vi;
                  int vd;
                  int alpha, val, dbuf, stencil, depth;
                  int rgba;
                  
                  vi = glXGetVisualFromFBConfig(disp, fbc[j]);
                  if (!vi) continue;
                  vd = vi->depth;
                  XFree(vi);
                  
                  if (vd != i) continue;
                  
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_ALPHA_SIZE, &alpha);
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_BUFFER_SIZE, &val);
                  
                  if ((val != i) && ((val - alpha) != i)) continue;
                  
                  val = 0;
                  rgba = 0;
                  
                  if (i == 32)
                    {
                       glXGetFBConfigAttrib(disp, fbc[j], GLX_BIND_TO_TEXTURE_RGBA_EXT, &val);
                       if (val)
                         {
                            rgba = 1;
                            gw->depth_cfg[i].tex_format = GLX_TEXTURE_FORMAT_RGBA_EXT;
                         }
                    }
                  if (!val)
                    {
                       if (rgba) continue;
                       glXGetFBConfigAttrib(disp, fbc[j], GLX_BIND_TO_TEXTURE_RGB_EXT, &val);
                       if (!val) continue;
                       gw->depth_cfg[i].tex_format = GLX_TEXTURE_FORMAT_RGB_EXT;
                    }
                  
                  dbuf = 0x7fff;
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_DOUBLEBUFFER, &val);
                  if (val > dbuf) continue;
                  dbuf = val;
                  
                  stencil = 0x7fff;
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_STENCIL_SIZE, &val);
                  if (val > stencil) continue;
                  stencil = val;
                  
                  depth = 0x7fff;
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_DEPTH_SIZE, &val);
                  if (val > depth) continue;
                  depth = val;
                  
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_BIND_TO_MIPMAP_TEXTURE_EXT, &val);
                  if (val < 0) continue;
                  gw->depth_cfg[i].mipmap = val;
                  
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_Y_INVERTED_EXT, &val);
                  gw->depth_cfg[i].yinvert = val;
                  
                  glXGetFBConfigAttrib(disp, fbc[j], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &val);
                  gw->depth_cfg[i].tex_target = val;
                  
                  gw->depth_cfg[i].fbc = fbc[j];
               }
          }
        XFree(fbc);
        if (!gw->depth_cfg[DefaultDepth(disp, 0/* FIXMEL assume screen 0*/)].fbc)
          {
             printf("text from pixmap not going to work\n");
          }
     }
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
eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return NULL;
   if (!einfo->info.display) return NULL;
   if (!_evas_gl_x11_vi)
     {
// EGL / GLES
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        int depth = DefaultDepth(einfo->info.display,
                                 einfo->info.screen);
        _evas_gl_x11_vi = calloc(1, sizeof(XVisualInfo));
        XMatchVisualInfo(einfo->info.display,
                         einfo->info.screen, depth, TrueColor,
                         _evas_gl_x11_vi);
// GLX
#else

#if 1
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
        config_attrs[i++] = GLX_ALPHA_SIZE;
        config_attrs[i++] = 0;
//// only needed if we want argb        
//        config_attrs[i++] = GLX_RENDER_TYPE;
//        config_attrs[i++] = 0;//GLX_RGBA_BIT;
        config_attrs[i++] = GLX_DEPTH_SIZE;
        config_attrs[i++] = 0;
        config_attrs[i++] = GLX_STENCIL_SIZE;
        config_attrs[i++] = 0;
        config_attrs[i++] = GLX_AUX_BUFFERS;
        config_attrs[i++] = 0;
        config_attrs[i++] = GLX_STEREO;
        config_attrs[i++] = 0;
        config_attrs[i++] = GLX_TRANSPARENT_TYPE;
        config_attrs[i++] = GLX_NONE;//GLX_TRANSPARENT_RGB;
//        config_attrs[i++] = GLX_TRANSPARENT_TYPE;
//        config_attrs[i++] = GLX_NONE;//GLX_TRANSPARENT_INDEX;
        config_attrs[i++] = 0;
        
        configs = glXChooseFBConfig(einfo->info.display, 
                                    einfo->info.screen,
                                    config_attrs, &num);
        if ((!configs) || (num < 1))
          {
             printf("ERROR: glXChooseFBConfig returned no configs\n");
          }
        for (i = 0; i < num; i++)
          {
             XVisualInfo *visinfo;
             XRenderPictFormat *format;
             
             visinfo = glXGetVisualFromFBConfig(einfo->info.display, 
                                                configs[i]);
             if (!visinfo) continue;
             if (1) // non argb
               {
                  config = configs[i];
                  _evas_gl_x11_vi = visinfo;
                  break;
               }
             else
               {
                  format = XRenderFindVisualFormat(einfo->info.display, visinfo->visual);
                  if (!format)
                    {
                       XFree(visinfo);
                       continue;
                    }
                  if (format->direct.alphaMask > 0)
                    {
                       config = configs[i];
                       _evas_gl_x11_vi = visinfo;
                       break;
                    }
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
        _evas_gl_x11_vi = glXChooseVisual(einfo->info.display,
                                          einfo->info.screen,
                                          _evas_gl_x11_configuration);
#endif
        
#endif
     }
   if (!_evas_gl_x11_vi) return NULL;
   return _evas_gl_x11_vi->visual;
}

Colormap
eng_best_colormap_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   if (!_evas_gl_x11_vi) eng_best_visual_get(einfo);
   if (!_evas_gl_x11_vi) return 0;
   if (!_evas_gl_x11_cmap)
     {
        _evas_gl_x11_cmap = XCreateColormap(einfo->info.display,
                                            RootWindow(einfo->info.display,
                                                       einfo->info.screen),
                                            _evas_gl_x11_vi->visual, 
                                            0);
     }
   return _evas_gl_x11_cmap;
}
                                 
int
eng_best_depth_get(Evas_Engine_Info_GL_X11 *einfo)
{
   if (!einfo) return 0;
   if (!einfo->info.display) return 0;
   if (!_evas_gl_x11_vi) eng_best_visual_get(einfo);
   if (!_evas_gl_x11_vi) return 0;
   return _evas_gl_x11_vi->depth;
}
