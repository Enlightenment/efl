#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "evas_cairo_common.h"

typedef struct _Evas_Cairo_X11_Window Evas_Cairo_X11_Window;

struct _Evas_Cairo_X11_Window
{
   Display         *disp;
   Window           win;
   int              w, h;
   int              screen;
   Visual          *visual;
   Colormap         colormap;
   int              depth;
   cairo_t         *cairo;
};

Evas_Cairo_X11_Window *
  evas_engine_cairo_x11_window_new(Display *disp,
				   Window   win,
				   int      screen,
				   Visual  *vis,
				   Colormap cmap,
				   int      depth,
				   int      w,
				   int      h);
void
  evas_engine_cairo_x11_window_free(Evas_Cairo_X11_Window *cw);
void
  evas_engine_cairo_x11_window_use(Evas_Cairo_X11_Window *cw);
    
#endif
