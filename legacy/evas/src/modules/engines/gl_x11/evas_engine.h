#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "config.h"
#ifdef HAVE_GL_GLEW_H
# include <GL/glxew.h>
#else
# if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#  if defined(GLES_VARIETY_S3C6410)
#   include <EGL/egl.h>
#   include <GLES2/gl2.h>
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#   include <X11/Xutil.h>
#   include <X11/extensions/Xrender.h>
# include <X11/Xresource.h> // xres - dpi
#  elif defined(GLES_VARIETY_SGX)
#   define SUPPORT_X11 1
#   include <EGL/egl.h>
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#   include <X11/Xutil.h>
#   include <X11/extensions/Xrender.h>
#   include <X11/Xresource.h> // xres - dpi
#endif
# else
#  include <GL/glx.h>
#  include <X11/Xlib.h>
#  include <X11/Xatom.h>
#  include <X11/Xutil.h>
#  include <X11/extensions/Xrender.h>
#  include <X11/Xresource.h> // xres - dpi
#  include <GL/gl.h>
#  include <GL/glext.h>
#  include <GL/glx.h>
# endif
#endif
#include "evas_common.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_GL_X11.h"

extern int _evas_engine_GL_X11_log_dom ;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_GL_X11_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_GL_X11_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_GL_X11_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_GL_X11_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_X11_log_dom, __VA_ARGS__)

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
   int              alpha;
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
   GLXWindow        glxwin;
   struct {
      GLXFBConfig   fbc;
      int           tex_format;
      int           tex_target;
      int           mipmap;
      unsigned char yinvert : 1;
   } depth_cfg[33]; // config for all 32 possible depths!
   
   struct {
      unsigned int loose_binding : 1;
   } detected;
#endif

};

Evas_GL_X11_Window *eng_window_new(Display *disp, Window win, int screen,
                                   Visual *vis, Colormap cmap,
                                   int depth, int w, int h, int indirect,
                                   int alpha);
void      eng_window_free(Evas_GL_X11_Window *gw);
void      eng_window_use(Evas_GL_X11_Window *gw);
Visual   *eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo);
Colormap  eng_best_colormap_get(Evas_Engine_Info_GL_X11 *einfo);
int       eng_best_depth_get(Evas_Engine_Info_GL_X11 *einfo);

#endif
