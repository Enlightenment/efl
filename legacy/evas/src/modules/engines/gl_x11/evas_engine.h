#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "config.h"
#ifdef HAVE_GL_GLEW_H
# include <GL/glxew.h>
#else
# if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#  if defined(GLES_VARIETY_S3C6410)
#   include <EGL/egl.h>
#   include <GLES/gl.h>
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#   include <X11/Xutil.h>
#   include <X11/extensions/Xrender.h>
#  elif defined(GLES_VARIETY_SGX)
#   define SUPPORT_X11 1
#   include <EGL/egl.h>
#   include <GLES2/gl2.h>
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#   include <X11/Xutil.h>
#   include <X11/extensions/Xrender.h>
#  endif
# else
#  include <GL/glx.h>
#  include <X11/Xlib.h>
#  include <X11/Xatom.h>
#  include <X11/Xutil.h>
#  include <X11/extensions/Xrender.h>
#  include <GL/gl.h>
#  include <GL/glx.h>
# endif
#endif
#include "evas_common.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_GL_X11.h"

typedef struct _Evas_GL_X11_Window Evas_GL_X11_Window;

struct _Evas_GL_X11_Window
{
   Display         *disp;
   Window           win;
   int              w, h;
   int              screen;
   XVisualInfo     *visualinfo;
   Visual          *visual;
   Colormap         colormap;
   int              depth;
   Evas_GL_Context *gl_context;
   struct {
      int              redraw : 1;
      int              drew : 1;
      int              x1, y1, x2, y2;
   } draw;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   EGLContext       egl_context[1];
   EGLSurface       egl_surface[1];
   EGLConfig        egl_config;
   EGLDisplay       egl_disp;
#else   
   GLXContext       context;
#endif

};

Evas_GL_X11_Window *eng_window_new(Display *disp, Window win, int screen,
                                   Visual *vis, Colormap cmap,
                                   int depth, int w, int h);
void      eng_window_free(Evas_GL_X11_Window *gw);
void      eng_window_use(Evas_GL_X11_Window *gw);
Visual   *eng_best_visual_get(Display *disp, int screen);
Colormap  eng_best_colormap_get(Display *disp, int screen);
int       eng_best_depth_get(Display *disp, int screen);

#endif
