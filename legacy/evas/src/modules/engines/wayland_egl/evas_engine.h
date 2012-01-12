#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "config.h"
#include "evas_common.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_Wayland_Egl.h"

#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <wayland-egl.h>

extern int _evas_engine_wl_egl_log_dom;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

typedef struct _Evas_GL_Wl_Window Evas_GL_Wl_Window;

struct _Evas_GL_Wl_Window
{
   struct wl_display *disp;
   struct wl_egl_window *win;
   struct wl_surface *surface;
   int              w, h;
   int              screen;
//   XVisualInfo     *visualinfo;
//   Visual          *visual;
//   Colormap         colormap;
   int              depth;
   int              alpha;
   int              rot;
   Evas_Engine_GL_Context *gl_context;
   struct {
      int              redraw : 1;
      int              drew : 1;
      int              x1, y1, x2, y2;
   } draw;
   EGLContext       egl_context[1];
   EGLSurface       egl_surface[1];
   EGLConfig        egl_config;
   EGLDisplay       egl_disp;
   int             surf : 1;
};

Evas_GL_Wl_Window *eng_window_new(struct wl_display *disp, struct wl_surface *surface, int screen,
                                   int depth, int w, int h, int indirect,
                                   int alpha, int rot);
void      eng_window_free(Evas_GL_Wl_Window *gw);
void      eng_window_use(Evas_GL_Wl_Window *gw);
void      eng_window_unsurf(Evas_GL_Wl_Window *gw);
void      eng_window_resurf(Evas_GL_Wl_Window *gw);

int       eng_best_depth_get(Evas_Engine_Info_Wayland_Egl *einfo);

#endif
