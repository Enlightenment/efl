#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "evas_gl_common.h"

typedef struct _Evas_GL_X11_Window Evas_GL_X11_Window;

struct _Evas_GL_X11_Window
{
   Display         *disp;
   Window           win;
   int              w, h;
   int              screen;
   XVisualInfo     *visualinfo;
   Visual          *visual;
   Colormap         colormap;
   int              depth;
   GLXContext       context;
   Evas_GL_Context *gl_context;
   struct {
      int              redraw : 1;
      int              drew : 1;
      int              x1, y1, x2, y2;
   } draw;
};

extern int          _evas_gl_x11_configuration[9];
extern XVisualInfo *_evas_gl_x11_vi;
extern Colormap     _evas_gl_x11_cmap;

Evas_GL_X11_Window *
  eng_window_new(Display *disp,
		 Window   win,
		 int      screen,
		 Visual  *vis,
		 Colormap cmap,
		 int      depth,
		 int      w,
		 int      h);
void
  eng_window_free(Evas_GL_X11_Window *gw);
void
  eng_window_use(Evas_GL_X11_Window *gw);

#endif
