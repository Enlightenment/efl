#include <X11/Xlib.h>
#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Cairo_X11.h"
#include "evas_cairo_common.h"

static Evas_Cairo_X11_Window *_evas_cairo_x11_window = NULL;

Evas_Cairo_X11_Window *
evas_engine_cairo_x11_window_new(Display *disp,
				 Window   win,
				 int      screen,
				 Visual  *vis,
				 Colormap cmap,
				 int      depth,
				 int      w,
				 int      h)
{
   Evas_Cairo_X11_Window *cw;

   cw = calloc(1, sizeof(Evas_Cairo_X11_Window));
   if (!cw) return NULL;
   cw->disp = disp;
   cw->win = win;
   cw->screen = screen;
   cw->visual = vis;
   cw->colormap = cmap;
   cw->depth = depth;
   cw->surface = cairo_xlib_surface_create(disp, win, vis, w, h);
   return cw;
}

void
evas_engine_cairo_x11_window_free(Evas_Cairo_X11_Window *cw)
{
   if (cw == _evas_cairo_x11_window) _evas_cairo_x11_window = NULL;
   cairo_surface_destroy(cw->surface);
   free(cw);
}

void
evas_engine_cairo_x11_window_use(Evas_Cairo_X11_Window *cw)
{
   if (_evas_cairo_x11_window != cw)
     {
	_evas_cairo_x11_window = cw;
     }
}

void
evas_engine_cairo_x11_window_size_set(Evas_Cairo_X11_Window *cw, int w, int h)
{
   cairo_xlib_surface_set_size(cw->surface, w, h);
}
