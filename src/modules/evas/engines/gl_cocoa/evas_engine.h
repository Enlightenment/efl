#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <OpenGL/gl.h>

#include "evas_gl_common.h"
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
   Render_Engine_GL_Generic generic;

   Outbuf *win;
   int                  end;
};

struct _Outbuf
{
   Evas_Engine_Info_GL_Cocoa *info;
   Evas_Engine_GL_Context *gl_context;
   Evas *evas;

   void *ns_gl_view; // NSOpenGLView*
   void *ns_window;  // NSWindow*

   int width;
   int height;

   // FIXME
 struct {
      int           x1;
      int           y1;
      int           x2;
      int           y2;
      int           redraw : 1;
      int           drew : 1;
   } draw;

};


extern Evas_Gl_Symbols glsym_evas_gl_symbols;
extern Evas_GL_Common_Context_New glsym_evas_gl_common_context_new;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_free;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_flush;
extern Evas_GL_Common_Context_Call glsym_evas_gl_common_context_use;
extern Evas_GL_Common_Context_Resize_Call glsym_evas_gl_common_context_resize;

Outbuf *evas_outbuf_new(Evas_Engine_Info_GL_Cocoa *info, int w, int h);
void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_use(Outbuf *ob);
void evas_outbuf_lock_focus(Outbuf *ob);
void evas_outbuf_unlock_focus(Outbuf *ob);
void evas_outbuf_resize(Outbuf *ob, int w, int h);
void evas_outbuf_swap_buffers(Outbuf *ob);
void evas_outbuf_vsync_set(int on); // FIXME

#endif /* __EVAS_ENGINE_H__ */
