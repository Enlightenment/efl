#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <OpenGL/gl.h>

#include "evas_gl_common.h"
#include "Evas_Engine_GL_Cocoa.h"

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

typedef struct _Evas_GL_Cocoa_Window Evas_GL_Cocoa_Window;

struct _Evas_GL_Cocoa_Window
{
   void*            window;
   void*            view;
   int              width;
   int              height;
   int              depth;
   Evas_Engine_GL_Context *gl_context;
   struct {
      int           x1;
      int           y1;
      int           x2;
      int           y2;
      int           redraw : 1;
      int           drew : 1;
   } draw;
};

Evas_GL_Cocoa_Window *eng_window_new(void *window,
	                             int  width,
				     int  height);
void eng_window_free(Evas_GL_Cocoa_Window *gw);
void eng_window_use(Evas_GL_Cocoa_Window *gw);
void eng_window_swap_buffers(Evas_GL_Cocoa_Window *gw);
void eng_window_vsync_set(int on);
void eng_window_resize(Evas_GL_Cocoa_Window *gw, int width, int height);

#endif /* __EVAS_ENGINE_H__ */
