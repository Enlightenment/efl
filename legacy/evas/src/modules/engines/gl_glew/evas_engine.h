#ifndef __EVAS_ENGINE_H__
#define __EVAS_ENGINE_H__

#include "evas_gl_common.h"


typedef struct _Evas_GL_Glew_Window Evas_GL_Glew_Window;

struct _Evas_GL_Glew_Window
{
   HDC              dc;
   HWND             window;
   int              width;
   int              height;
   int              depth;
   HGLRC            context;
   Evas_GL_Context *gl_context;
   struct {
      int              redraw : 1;
      int              drew : 1;
      int              x1, y1, x2, y2;
   } draw;
};

Evas_GL_Glew_Window *eng_window_new(HDC  dc,
                                    HWND window,
                                    int  depth,
                                    int  width,
                                    int  height);

void eng_window_free(Evas_GL_Glew_Window *gw);
void eng_window_use(Evas_GL_Glew_Window *gw);


#endif /* __EVAS_ENGINE_H__ */
