#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

EAPI Ecore_Wl2_Window *
ecore_wl2_window_new(Ecore_Wl2_Display *display, Ecore_Wl2_Window *parent, int x, int y, int w, int h)
{
   Ecore_Wl2_Window *win;
   static int _win_id = 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(display, NULL);

   /* try to allocate space for window structure */
   win = calloc(1, sizeof(Ecore_Wl2_Window));
   if (!win) return NULL;

   win->display = display;
   win->parent = parent;
   win->id = _win_id++;

   win->geometry.x = x;
   win->geometry.y = y;
   win->geometry.w = w;
   win->geometry.h = h;

   return win;
}
