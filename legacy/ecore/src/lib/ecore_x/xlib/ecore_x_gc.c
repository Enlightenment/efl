#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * Creates a new default graphics context associated with the given
 * drawable.
 * @param  draw Drawable to create graphics context with.  If @c 0 is
 *              given instead, the default root window is used.
 * @return The new default graphics context.
 */
EAPI Ecore_X_GC
ecore_x_gc_new(Ecore_X_Drawable draw)
{
   XGCValues           gcv;

   if (!draw) draw = DefaultRootWindow(_ecore_x_disp);   
   return XCreateGC(_ecore_x_disp, draw, 0, &gcv);   
}

/**
 * Deletes and frees the given graphics context.
 * @param gc The given graphics context.
 */
EAPI void
ecore_x_gc_del(Ecore_X_GC gc)
{
   XFreeGC(_ecore_x_disp, gc);   
}
