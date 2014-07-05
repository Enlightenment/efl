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

#include "../software_generic/Evas_Engine_Software_Generic.h"

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

typedef struct _Outbuf Outbuf;
typedef struct _Evas_GL_X11_Context Evas_GL_X11_Context;

struct _Outbuf
{
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

   Evas            *evas;
   Display         *disp;
   XVisualInfo     *visualinfo;
   Visual          *visual;
   Evas_Engine_GL_Context *gl_context;
   Evas_Engine_Info_GL_X11 *info;

   Render_Engine_Swap_Mode swap_mode;
   Colormap         colormap;
   Window           win;
   int              w, h;
   int              screen;
   int              depth;
   int              alpha;
   int              rot;
   int              prev_age;
   int              frame_cnt;
   int              vsync;

   unsigned char    lost_back : 1;
   unsigned char    surf : 1;

   struct {
      unsigned char drew : 1;
   } draw;
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

extern int extn_have_buffer_age;
extern int partial_render_debug;
extern int swap_buffer_debug_mode;
extern int swap_buffer_debug;
extern const char *debug_dir;

#ifdef GL_GLES

#ifndef EGL_BUFFER_AGE_EXT
# define EGL_BUFFER_AGE_EXT 0x313d
#endif

unsigned int   (*glsym_eglSwapBuffersWithDamage) (EGLDisplay a, void *b, const EGLint *d, EGLint c) = NULL;

#else

#ifndef GLX_BACK_BUFFER_AGE_EXT
# define GLX_BACK_BUFFER_AGE_EXT 0x20f4
#endif

extern void     (*glsym_glXQueryDrawable)   (Display *a, XID b, int c, unsigned int *d);
extern void     (*glsym_glXSwapIntervalEXT) (Display *s, GLXDrawable b, int c);
extern int      (*glsym_glXSwapIntervalSGI) (int a);
extern int      (*glsym_glXGetVideoSync)    (unsigned int *a);
extern int      (*glsym_glXWaitVideoSync)   (int a, int b, unsigned int *c);

#endif

Outbuf *eng_window_new(Evas_Engine_Info_GL_X11 *info, Evas *e,
                       Display *disp, Window win, int screen,
                       Visual *vis, Colormap cmap,
                       int depth, int w, int h, int indirect,
                       int alpha, int rot,
                       Render_Engine_Swap_Mode swap_mode);
void      eng_window_free(Outbuf *gw);
void      eng_window_use(Outbuf *gw);
void      eng_window_unsurf(Outbuf *gw);
void      eng_window_resurf(Outbuf *gw);

void     *eng_best_visual_get(Evas_Engine_Info_GL_X11 *einfo);
Colormap  eng_best_colormap_get(Evas_Engine_Info_GL_X11 *einfo);
int       eng_best_depth_get(Evas_Engine_Info_GL_X11 *einfo);

Evas_GL_X11_Context *eng_gl_context_new(Outbuf *win);
void      eng_gl_context_free(Evas_GL_X11_Context *context);
void      eng_gl_context_use(Evas_GL_X11_Context *context);

void eng_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
int eng_outbuf_get_rot(Outbuf *ob);
Render_Engine_Swap_Mode eng_outbuf_swap_mode(Outbuf *ob);
Eina_Bool eng_outbuf_region_first_rect(Outbuf *ob);
void *eng_outbuf_new_region_for_update(Outbuf *ob,
                                       int x, int y, int w, int h,
                                       int *cx, int *cy, int *cw, int *ch);
void eng_outbuf_push_free_region_for_update(Outbuf *ob, RGBA_Image *update);
void eng_outbuf_push_updated_region(Outbuf *ob, RGBA_Image *update,
                                    int x, int y, int w, int h);
void eng_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects, Evas_Render_Mode render_mode);

Eina_Bool eng_preload_make_current(void *data, void *doit);

static inline int
_re_wincheck(Outbuf *ob)
{
   if (ob->surf) return 1;
   eng_window_resurf(ob);
   ob->lost_back = 1;
   if (!ob->surf)
     {
        ERR("GL engine can't re-create window surface!");
     }
   return 0;
}

#endif
