#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#include <windows.h>

#include "evas_gl_common.h"
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
   HWND             window;
   HDC              dc;
   HGLRC            context;
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

int evas_glew_init(HWND window, HDC *dc, HGLRC *context);

void evas_glew_shutdown(HWND  window,
                        HDC   dc,
                        HGLRC context);

Evas_GL_Glew_Window *eng_window_new(HWND window,
                                    HDC   dc,
                                    HGLRC context,
                                    int  depth,
                                    int  width,
                                    int  height);

void eng_window_free(Evas_GL_Glew_Window *gw);
void eng_window_use(Evas_GL_Glew_Window *gw);


#endif /* __EVAS_ENGINE_H__ */
