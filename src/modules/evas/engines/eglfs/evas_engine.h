#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

# include "evas_common_private.h"
# include "evas_macros.h"
# include "evas_private.h"
# include "Evas.h"
# include "Evas_Engine_Eglfs.h"

# define EGL_EGLEXT_PROTOTYPES
# define GL_GLEXT_PROTOTYPES

# include <EGL/egl.h>
# include <EGL/eglext.h>
# include <EGL/eglmesaext.h>
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
# include <hwcomposer.h>
# include <hardware/hardware.h>
# include <hardware/hwcomposer.h>
# include "../gl_generic/Evas_Engine_GL_Generic.h"

extern int _evas_engine_eglfs_log_dom;
extern int _extn_have_buffer_age;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_eglfs_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_eglfs_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_eglfs_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_eglfs_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_eglfs_log_dom, __VA_ARGS__)

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

struct _Context_3D
{
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
};

struct _Outbuf
{
   Evas_Engine_Info_Eglfs *info;
   Evas_Engine_GL_Context *gl_context;

   Evas *evas; // used for pre_swap, post_swap

   int w, h;
   unsigned int rotation, depth;
   Render_Output_Swap_Mode swap_mode;

   struct
     {
        EGLContext context[1];
        EGLSurface surface[1];
        EGLConfig config;
        EGLDisplay disp;
     } egl;

   struct
     {
        int prev_age, frame_cnt;
        int curr, last, num;
        Eina_List *pending_writes;
     } priv;

   Eina_Bool destination_alpha : 1;
   Eina_Bool vsync : 1;
   Eina_Bool lost_back : 1;
   Eina_Bool surf : 1;
   Eina_Bool drew : 1;
};

Outbuf *evas_outbuf_new(Evas_Engine_Info_Eglfs *info, int w, int h, Render_Output_Swap_Mode swap_mode);
void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_use(Outbuf *ob);
void evas_outbuf_resurf(Outbuf *ob);
void evas_outbuf_unsurf(Outbuf *ob);
void evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
Render_Output_Swap_Mode evas_outbuf_buffer_state_get(Outbuf *ob);
int evas_outbuf_rot_get(Outbuf *ob);
Eina_Bool evas_outbuf_update_region_first_rect(Outbuf *ob);
void *evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void evas_outbuf_update_region_free(Outbuf *ob, RGBA_Image *update);
void evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);
Evas_Engine_GL_Context* evas_outbuf_gl_context_get(Outbuf *ob);
void *evas_outbuf_egl_display_get(Outbuf *ob);
Context_3D *evas_outbuf_gl_context_new(Outbuf *ob);
void evas_outbuf_gl_context_use(Context_3D *ctx);
EGLNativeWindowType create_hwcomposernativewindow(void);

static inline Eina_Bool
_re_wincheck(Outbuf *ob)
{
   if (ob->surf) return EINA_TRUE;
   evas_outbuf_resurf(ob);
   ob->lost_back = 1;
   if (!ob->surf) ERR("GL engine can't re-create window surface!");
   return EINA_FALSE;
}

extern unsigned int (*glsym_eglSwapBuffersWithDamage)(EGLDisplay a, void *b, const EGLint *d, EGLint c);

#endif
