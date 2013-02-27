#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

#include "config.h"
#include "evas_common.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_Wayland_Egl.h"

#include <EGL/egl.h>
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
   struct wl_surface *surf;
   struct wl_egl_window *win;
   Evas_Coord w, h;
   Eina_Bool alpha : 1;
   int rotation;
   Evas_Engine_GL_Context *gl_context;
   Eina_Bool drawn : 1;
   Eina_Bool surf : 1;
   struct 
     {
        EGLContext context[1];
        EGLSurface surface[1];
        EGLConfig config;
        EGLDisplay disp;
     } egl;
};

Evas_GL_Wl_Window *eng_window_new(struct wl_display *disp, 
                                  struct wl_surface *surface, int screen,
                                   int depth, int w, int h, int indirect,
                                   int alpha, int rot);
void eng_window_free(Evas_GL_Wl_Window *gw);
void eng_window_use(Evas_GL_Wl_Window *gw);
void eng_window_unsurf(Evas_GL_Wl_Window *gw);
void eng_window_resurf(Evas_GL_Wl_Window *gw);

#endif
