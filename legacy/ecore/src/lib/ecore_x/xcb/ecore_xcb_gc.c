/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * Creates a new default graphics context associated with the given
 * drawable.
 * @param  drawable Drawable to create graphics context with.  If @c 0 is
 *                  given instead, the default root window is used.
 * @param value_mask Bitmask values.
 * @param value_list List of values. The order of values must be the
 *                   same than the corresponding bitmaks.
 * @return          The new default graphics context.
 *
 * Creates a new default graphics context associated with @p
 * drawable. The graphic context can be used with any destination
 * drawable having the same root and depth as @p drawable. Use with
 * other drawables results in a BadMatch error.
 */
EAPI Ecore_X_GC
ecore_x_gc_new(Ecore_X_Drawable drawable, Ecore_X_GC_Value_Mask value_mask, const unsigned int *value_list)
{
   xcb_gcontext_t gc;

   if (!drawable) drawable = ((xcb_screen_t *)_ecore_xcb_screen)->root;

   gc = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_gc(_ecore_xcb_conn, gc, drawable, value_mask, value_list);

   return gc;
}


/**
 * Deletes and frees the given graphics context.
 * @param gc The given graphics context.
 *
 * Destroyes the graphic context @p gc as well as the associated
 * storage.
 */
EAPI void
ecore_x_gc_del(Ecore_X_GC gc)
{
   xcb_free_gc(_ecore_xcb_conn, gc);
}
