#ifndef _EVAS_ENGINE_H
# define _EVAS_ENGINE_H

# include "config.h"
# include "evas_common.h"
# include "evas_private.h"
# include "evas_gl_common.h"
# include "Evas.h"
# include "Evas_Engine_Wayland_Egl.h"

# define GL_GLEXT_PROTOTYPES

# include <wayland-egl.h>
// # include <GLES2/gl2.h>
# include <EGL/egl.h>

# if defined (GLES_VARIETY_SGX)
#  include <GLES2/gl2ext.h>
# endif

//# define LOGFNS 1

# ifdef LOGFNS
#  include <stdio.h>
#  define LOGFN(fl, ln, fn) printf("-EVAS-WL: %25s: %5i - %s\n", fl, ln, fn);
# else
#  define LOGFN(fl, ln, fn)
# endif

extern int _evas_engine_way_egl_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_way_egl_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_way_egl_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_way_egl_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_way_egl_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_way_egl_log_dom, __VA_ARGS__)

typedef struct _Outbuf Outbuf;
struct _Outbuf 
{
   int w, h, rot;
   int depth, alpha;

   struct 
     {
        struct wl_egl_window *win;
        struct wl_surface *surface;
        struct wl_shell *shell;
        struct 
          {
             Evas_Engine_GL_Context *context;
          } gl;
        struct 
          {
             EGLDisplay disp;
             EGLConfig config;
             EGLSurface surface;
             EGLContext context;
          } egl;
     } priv;
};

void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_resize(Outbuf *ob, int w, int h);
Outbuf *evas_outbuf_setup(struct wl_display *disp, struct wl_compositor *comp, struct wl_shell *shell, int w, int h, int rot);

#endif
