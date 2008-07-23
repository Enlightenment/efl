/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_Pixmap_Group X Pixmap Functions
 *
 * Functions that operate on pixmaps.
 */


/**
 * Creates a new pixmap.
 * @param   win Window used to determine which screen of the display the
 *              pixmap should be created on.  If 0, the default root window
 *              is used.
 * @param   w   Width of the new pixmap.
 * @param   h   Height of the new pixmap.
 * @param   dep Depth of the pixmap.  If 0, the default depth of the default
 *              screen is used.
 * @return  New pixmap.
 * @ingroup Ecore_X_Pixmap_Group
 */
EAPI Ecore_X_Pixmap
ecore_x_pixmap_new(Ecore_X_Window win,
                   int            w,
                   int            h,
                   int            dep)
{
   Ecore_X_Pixmap pmap;

   if (win == 0) win = ((xcb_screen_t *)_ecore_xcb_screen)->root;
   if (dep == 0) dep = ((xcb_screen_t *)_ecore_xcb_screen)->root_depth;

   pmap = xcb_generate_id(_ecore_xcb_conn);
   xcb_create_pixmap(_ecore_xcb_conn, dep, pmap, win, w, h);

   return pmap;
}


/**
 * Deletes the reference to the given pixmap.
 *
 * If no other clients have a reference to the given pixmap, the server
 * will destroy it.
 *
 * @param   pmap The given pixmap.
 * @ingroup Ecore_X_Pixmap_Group
 */
EAPI void
ecore_x_pixmap_del(Ecore_X_Pixmap pmap)
{
   xcb_free_pixmap(_ecore_xcb_conn, pmap);
}


/**
 * Pastes a rectangular area of the given pixmap onto the given drawable.
 * @param   pmap The given pixmap.
 * @param   dest The given drawable.
 * @param   gc   The graphics context which governs which operation will
 *               be used to paste the area onto the drawable.
 * @param   sx   The X position of the area on the pixmap.
 * @param   sy   The Y position of the area on the pixmap.
 * @param   w    The width of the area.
 * @param   h    The height of the area.
 * @param   dx   The X position at which to paste the area on @p dest.
 * @param   dy   The Y position at which to paste the area on @p dest.
 * @ingroup Ecore_X_Pixmap_Group
 */
EAPI void
ecore_x_pixmap_paste(Ecore_X_Pixmap   pmap,
                     Ecore_X_Drawable dest,
		     Ecore_X_GC       gc,
                     int              sx,
                     int              sy,
		     int              w,
                     int              h,
                     int              dx,
                     int              dy)
{
   xcb_copy_area(_ecore_xcb_conn, pmap, dest, gc, sx, sy, dx, dy, w, h);
}


/**
 * Retrieves the size of the given pixmap.
 * @param   pmap The given pixmap.
 * @param   x    Pointer to an integer in which to store the X position.
 * @param   y    Pointer to an integer in which to store the Y position.
 * @param   w    Pointer to an integer in which to store the width.
 * @param   h    Pointer to an integer in which to store the height.
 * @ingroup Ecore_X_Pixmap_Group
 */
EAPI void
ecore_x_pixmap_geometry_get(Ecore_X_Pixmap pmap, int *x, int *y, int *w, int *h)
{
   if (pmap)
      ecore_x_drawable_geometry_get(pmap, x, y, w, h);
}


/**
 * Retrieves the depth of the given pixmap.
 * @param   pmap The given pixmap.
 * @return  The depth of the pixmap.
 * @ingroup Ecore_X_Pixmap_Group
 */
EAPI int
ecore_x_pixmap_depth_get(Ecore_X_Pixmap pmap)
{
   return ecore_x_drawable_depth_get(pmap);
}
