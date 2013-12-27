#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "config.h"
#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_GL_X11.h"

#define GL_GLEXT_PROTOTYPES

#ifdef GL_GLES
# define SUPPORT_X11 1
# include <EGL/egl.h>
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
# include <X11/Xlib.h>
# include <X11/Xatom.h>
# include <X11/Xutil.h>
# include <X11/extensions/Xrender.h>
# include <X11/Xresource.h> // xres - dpi
#else
# include <X11/Xlib.h>
# include <X11/Xatom.h>
# include <X11/Xutil.h>
# include <X11/extensions/Xrender.h>
# include <X11/Xresource.h> // xres - dpi
# include <GL/gl.h>
# include <GL/glext.h>
# include <GL/glx.h>
#endif

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

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_X11_log_dom, __VA_ARGS__)

typedef struct _Evas_GL_X11_Window Evas_GL_X11_Window;
typedef struct _Evas_GL_X11_Context Evas_GL_X11_Context;

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
   int              rot;
   Evas_Engine_GL_Context *gl_context;
   struct {
      int           drew : 1;
   } draw;
#ifdef GL_GLES
   EGLContext       egl_context[1];
   EGLSurface       egl_surface[1];
   EGLConfig        egl_config;
   EGLDisplay       egl_disp;
#else
   GLXContext       context;
   GLXWindow        glxwin;

   struct {
      unsigned int loose_binding : 1;
   } detected;
#endif
   int             surf : 1;
};

struct _Evas_GL_X11_Context
{
#ifdef GL_GLES
   EGLDisplay      display;
   EGLContext      context;
   EGLSurface      surface;
#else
   Display        *display;
   GLXContext      context;
   GLXWindow       glxwin;
   Window          win;
#endif
};

Evas_GL_X11_Window *eng_window_new(Display *disp, Window win, int screen,
                                   Visual *vis, Colormap cmap,
                                   int depth, int w, int h, int indirect,
                                   int alpha, int rot);
void      eng_window_free(Evas_GL_X11_Window *gw);
void      eng_window_use(Evas_GL_X11_Window *gw);
void      eng_window_unsurf(Evas_GL_X11_Window *gw);
void      eng_window_resurf(Evas_GL_X11_Window *gw);

void     *eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo);
Colormap  eng_best_colormap_get(Evas_Engine_Info_GL_X11 *einfo);
int       eng_best_depth_get(Evas_Engine_Info_GL_X11 *einfo);

Evas_GL_X11_Context *eng_gl_context_new(Evas_GL_X11_Window *win);
void      eng_gl_context_free(Evas_GL_X11_Context *context);
void      eng_gl_context_use(Evas_GL_X11_Context *context);

#endif
