#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_X_GC
ecore_x_gc_new(Ecore_X_Drawable draw)
{
   XGCValues           gcv;

   if (!draw) draw = DefaultRootWindow(_ecore_x_disp);   
   return XCreateGC(_ecore_x_disp, draw, 0, &gcv);   
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_x_gc_del(Ecore_X_GC gc)
{
   XFreeGC(_ecore_x_disp, gc);   
}
