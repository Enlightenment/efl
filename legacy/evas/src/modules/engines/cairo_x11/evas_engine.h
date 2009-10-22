#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H
#include "evas_common.h"
#include "evas_cairo_common.h"

extern int _evas_engine_cairo_X11_log_dom ;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_cairo_X11_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_cairo_X11_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_cairo_X11_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_cairo_X11_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_cairo_X11_log_dom, __VA_ARGS__)


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
   cairo_surface_t *surface;

   struct {
      int redraw : 1;
      int x1, y1, x2, y2;
   } draw;
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
void
  evas_engine_cairo_x11_window_size_set(Evas_Cairo_X11_Window *cw, int w, int h);

#endif
