#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

#include "config.h"
#include "evas_common_private.h"
#include "evas_private.h"
#include "Evas.h"
#include "Evas_Engine_GL_Drm.h"
#include "evas_macros.h"

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "../gl_generic/Evas_Engine_GL_Generic.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

extern int extn_have_buffer_age;
extern int _evas_engine_gl_drm_log_dom;

extern Evas_GL_Common_Context_New glsym_evas_gl_common_context_new;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_flush;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_free;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_use;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_newframe;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_done;
extern Evas_GL_Common_Context_Resize_Call glsym_evas_gl_common_context_resize;
extern Evas_GL_Common_Buffer_Dump_Call glsym_evas_gl_common_buffer_dump;
extern Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_lock;
extern Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_unlock;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_gl_drm_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_gl_drm_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_gl_drm_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_gl_drm_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_gl_drm_log_dom, __VA_ARGS__)

# define NUM_BUFFERS 2

typedef struct _Buffer Buffer;
typedef struct _Plane Plane;
typedef struct _Outbuf Outbuf;

struct _Buffer
{
   int stride;
   int size;
   int handle;
   unsigned int fb;
   struct gbm_bo *bo; //used for hardware framebuffers
   Eina_Bool valid : 1;
};

struct _Plane
{
   unsigned int id;
   unsigned int crtcs;

   struct
     {
        unsigned int x, y;
        unsigned int w, h;
     } src, dst;

   unsigned int num_formats;
   unsigned int formats[];
};

struct _Outbuf
{
   EGLContext egl_context[1];
   EGLSurface egl_surface[1];
   EGLConfig egl_config;
   EGLDisplay egl_disp;
   struct gbm_device *gbm;
   struct gbm_surface *surface;
   Evas *evas;
   uint32_t format;
   uint32_t flags;
   Evas_Engine_GL_Context *gl_context;
   Evas_Engine_Info_GL_Drm *info;
   Render_Engine_Swap_Mode swap_mode;
   int w, h;
   int depth, rot, screen, alpha;
   int prev_age;
   int frame_cnt;
   int vsync;
   Eina_Bool lost_back : 1;
   Eina_Bool surf : 1;

   struct
     {
        Eina_Bool drew : 1;
     } draw;

   struct
     {
      int fd;
      unsigned int conn, crtc, fb;
      Buffer buffer[NUM_BUFFERS];
      int curr, num;
      drmModeModeInfo mode;
      drmEventContext ctx;
      Eina_List *pending_writes;
      Eina_List *planes;
      Eina_Bool pending_flip : 1;
     } priv;
};

struct _Context_3D
{
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
};

Outbuf *eng_window_new(Evas_Engine_Info_GL_Drm *info, Evas *e, struct gbm_device *gbm, struct gbm_surface *surface, int screen, int depth, int w, int h, int indirect, int alpha, int rot, Render_Engine_Swap_Mode swap_mode);
void eng_window_free(Outbuf *gw);
void eng_window_use(Outbuf *gw);
void eng_window_unsurf(Outbuf *gw);
void eng_window_resurf(Outbuf *gw);

void eng_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
int eng_outbuf_get_rot(Outbuf *ob);
Render_Engine_Swap_Mode eng_outbuf_swap_mode(Outbuf *ob);
Eina_Bool eng_outbuf_region_first_rect(Outbuf *ob);
void *eng_outbuf_new_region_for_update(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void eng_outbuf_push_free_region_for_update(Outbuf *ob, RGBA_Image *update);
void eng_outbuf_push_updated_region(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void eng_outbuf_flush(Outbuf *ob, Tilebuf_Rect *rects, Evas_Render_Mode render_mode);
Evas_Engine_GL_Context* eng_outbuf_gl_context_get(Outbuf *ob);
void *eng_outbuf_egl_display_get(Outbuf *ob);

void eng_gl_context_free(Context_3D *context);
void eng_gl_context_use(Context_3D *context);

Eina_Bool eng_preload_make_current(void *data, void *doit);

Context_3D *eng_gl_context_new(Outbuf *win);

static inline Eina_Bool
_re_wincheck(Outbuf *ob)
{
   if (ob->surf) return EINA_TRUE;
   eng_window_resurf(ob);
   ob->lost_back = 1;
   if (!ob->surf)
     {
        ERR("GL engine can't re-create window surface!");
     }
   return EINA_FALSE;
}

//TODO: Need to split below evas_drm_... apis
Eina_Bool evas_drm_init(Evas_Engine_Info_GL_Drm *info);
Eina_Bool evas_drm_shutdown(Evas_Engine_Info_GL_Drm *info);
Eina_Bool evas_drm_gbm_init(Evas_Engine_Info_GL_Drm *info, int w, int h);
Eina_Bool evas_drm_gbm_shutdown(Evas_Engine_Info_GL_Drm *info);
Eina_Bool evas_drm_outbuf_setup(Outbuf *ob);
void evas_drm_outbuf_framebuffer_set(Outbuf *ob, Buffer *buffer);
Eina_Bool evas_drm_framebuffer_send(Outbuf *ob, Buffer *buffer);
#endif
