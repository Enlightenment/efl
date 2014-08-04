#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include "config.h"
# include "evas_common_private.h"
# include "evas_private.h"
# include "Evas.h"
# include "Evas_Engine_Wayland_Egl.h"

/* NB: This already includes wayland-client.h */
# include <wayland-egl.h>

# define GL_GLEXT_PROTOTYPES

# ifdef GL_GLES
#  include <EGL/egl.h>
#  include <GLES2/gl2.h>
#  include <GLES2/gl2ext.h>
# endif

# include "../gl_generic/Evas_Engine_GL_Generic.h"

extern int _evas_engine_wl_egl_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_wl_egl_log_dom, __VA_ARGS__)

typedef struct _Outbuf Outbuf;

struct _Outbuf
{
   struct wl_display *disp;
   struct wl_egl_window *win;
   struct wl_surface *surface;
   int w, h;
   int depth, screen, rot, alpha;

   Evas_Engine_GL_Context *gl_context;

   struct 
     {
        Eina_Bool drew : 1;
     } draw;

#ifdef GL_GLES
   EGLContext egl_context[1];
   EGLSurface egl_surface[1];
   EGLConfig egl_config;
   EGLDisplay egl_disp;
#endif

   Eina_Bool surf : 1;
};

Outbuf *eng_window_new(struct wl_display *disp, struct wl_surface *surface, int screen, int depth, int w, int h, int indirect, int alpha, int rot);
void eng_window_free(Outbuf *gw);
void eng_window_use(Outbuf *gw);
void eng_window_unsurf(Outbuf *gw);
void eng_window_resurf(Outbuf *gw);
Eina_Bool eng_window_make_current(void *data, void *doit);

#endif
