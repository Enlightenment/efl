#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <OpenGL/gl.h>

#include "../gl_common/evas_gl_common.h"
#include "Evas_Engine_GL_Cocoa.h"
#include "../gl_generic/Evas_Engine_GL_Generic.h"


extern int _evas_engine_gl_cocoa_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_gl_cocoa_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_gl_cocoa_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_gl_cocoa_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_gl_cocoa_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_gl_cocoa_log_dom, __VA_ARGS__)

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Render_Output_GL_Generic generic;

   Outbuf *win;
};

struct _Context_3D
{
   Outbuf *ob;
   void   *ns_gl_context;
};

struct _Outbuf
{
   Evas_Engine_Info_GL_Cocoa *info;
   Evas_Engine_GL_Context *gl_context;

   void *ns_gl_view; // NSOpenGLView*
   void *ns_window;  // NSWindow*

   int w;
   int h;
   int rot;
   Render_Output_Swap_Mode swap_mode;

   Eina_Bool drew;
};


static inline Outbuf *
eng_get_ob(const Render_Engine *re)
{
   return re->generic.software.ob;
}

extern Evas_GL_Common_Context_New glsym_evas_gl_common_context_new;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_free;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_flush;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_use;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_done;
extern Evas_GL_Common_Context_Resize_Call glsym_evas_gl_common_context_resize;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_newframe;
extern Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_lock;
extern Evas_GL_Preload_Render_Call glsym_evas_gl_preload_render_unlock;
extern Evas_Gl_Symbols glsym_evas_gl_symbols;

Outbuf *evas_outbuf_new(Evas_Engine_Info_GL_Cocoa *info, int w, int h);
void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_use(Outbuf *ob);
int evas_outbuf_rot_get(Outbuf *ob);
Render_Output_Swap_Mode evas_outbuf_buffer_state_get(Outbuf *ob);
void evas_outbuf_gl_context_use(Context_3D *ctx);
Eina_Bool evas_outbuf_update_region_first_rect(Outbuf *ob);
void *evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void evas_outbuf_update_region_free(Outbuf *ob, RGBA_Image *update);
void evas_outbuf_flush(Outbuf *ob, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);
Evas_Engine_GL_Context *evas_outbuf_gl_context_get(Outbuf *ob);
Context_3D *evas_outbuf_gl_context_new(Outbuf *ob);
void evas_outbuf_gl_context_use(Context_3D *ctx);
void evas_outbuf_reconfigure(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
void *evas_outbuf_egl_display_get(Outbuf *ob);

#define GL_COCOA_UNIMPLEMENTED_CALL_SO_RETURN(...) \
   do { \
      CRI("[%s] IS AN UNIMPLEMENTED CALL. PLEASE REPORT!!", __func__); \
      return __VA_ARGS__; \
   } while (0)

#endif /* __EVAS_ENGINE_H__ */
