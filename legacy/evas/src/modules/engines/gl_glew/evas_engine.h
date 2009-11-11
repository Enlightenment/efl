#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <windows.h>

#include "evas_gl_common.h"
#include "Evas_Engine_GL_Glew.h"

extern int _evas_engine_GL_glew_log_dom ;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_GL_glew_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_GL_glew_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_GL_glew_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_GL_glew_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_glew_log_dom, __VA_ARGS__)

typedef struct _Evas_GL_Glew_Window Evas_GL_Glew_Window;

struct _Evas_GL_Glew_Window
{
#ifdef _WIN32
   HWND             window;
   HDC              dc;
   HGLRC            context;
#endif
   int              width;
   int              height;
   int              depth;
   Evas_GL_Context *gl_context;
   struct {
      int           x1;
      int           y1;
      int           x2;
      int           y2;
      int           redraw : 1;
      int           drew : 1;
   } draw;
};

#ifdef _WIN32
Evas_GL_Glew_Window *eng_window_new(HWND window,
                                    int  depth,
                                    int  width,
                                    int  height);
#endif

void eng_window_free(Evas_GL_Glew_Window *gw);
void eng_window_use(Evas_GL_Glew_Window *gw);
void eng_window_swap_buffers(Evas_GL_Glew_Window *gw);
void eng_window_vsync_set(int on);


#endif /* __EVAS_ENGINE_H__ */
