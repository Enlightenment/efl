#include "ecore_xcb_private.h"
#include <xcb/xcb.h>

/**
 * @defgroup Ecore_X_Drawable_Group X Drawable Functions
 *
 * Functions that operate on drawables.
 */

/**
 * Sends the GetGeometry request.
 * @param drawable Drawable whose characteristics are sought.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI void
ecore_x_drawable_geometry_get_prefetch(Ecore_X_Drawable drawable)
{
   xcb_get_geometry_cookie_t cookie;

   cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, drawable);
   _ecore_xcb_cookie_cache(cookie.sequence);
} /* ecore_x_drawable_geometry_get_prefetch */

/**
 * Gets the reply of the GetGeometry request sent by ecore_x_atom_get_prefetch().
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI void
ecore_x_drawable_geometry_get_fetch(void)
{
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
} /* ecore_x_drawable_geometry_get_fetch */

/**
 * Retrieves the geometry of the given drawable.
 * @param drawable Unused.
 * @param x        Pointer to an integer into which the X position is to be stored.
 * @param y        Pointer to an integer into which the Y position is to be stored.
 * @param width    Pointer to an integer into which the width is to be stored.
 * @param height   Pointer to an integer into which the height is to be stored.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_drawable_geometry_get_prefetch(), which sends the GetGeometry request,
 * then ecore_x_drawable_geometry_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI void
ecore_x_drawable_geometry_get(Ecore_X_Drawable drawable __UNUSED__,
                              int                      *x,
                              int                      *y,
                              int                      *width,
                              int                      *height)
{
   xcb_get_geometry_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     {
        if (x)
           *x = 0;

        if (y)
           *y = 0;

        if (width)
           *width = 0;

        if (height)
           *height = 0;

        return;
     }

   if (x)
      *x = reply->x;

   if (y)
      *y = reply->y;

   if (width)
      *width = reply->width;

   if (height)
      *height = reply->height;
} /* ecore_x_drawable_geometry_get */

/**
 * Retrieves the width of the border of the given drawable.
 * @param  drawable Unused.
 * @return          The border width of the given drawable.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_drawable_geometry_get_prefetch(), which sends the GetGeometry request,
 * then ecore_x_drawable_geometry_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI int
ecore_x_drawable_border_width_get(Ecore_X_Drawable drawable __UNUSED__)
{
   xcb_get_geometry_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return 0;

   return reply->border_width;
} /* ecore_x_drawable_border_width_get */

/**
 * Retrieves the depth of the given drawable.
 * @param  drawable Unused.
 * @return          The depth of the given drawable.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_drawable_geometry_get_prefetch(), which sends the GetGeometry request,
 * then ecore_x_drawable_geometry_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI int
ecore_x_drawable_depth_get(Ecore_X_Drawable drawable __UNUSED__)
{
   xcb_get_geometry_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
      return 0;

   return reply->depth;
} /* ecore_x_drawable_depth_get */

/**
 * Fill the specified rectangle on a drawable.
 * @param d The given drawable.
 * @param gc The graphic context that controls the fill rules.
 * @param x The X coordinate of the top-left corner of the rectangle.
 * @param y The Y coordinate of the top-left corner of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 */
EAPI void
ecore_x_drawable_rectangle_fill(Ecore_X_Drawable d, Ecore_X_GC gc, int x, int y, int width, int height)
{
   xcb_rectangle_t rectangle;

   rectangle.x = x;
   rectangle.y = y;
   rectangle.width = width;
   rectangle.height = height;
   xcb_poly_fill_rectangle(_ecore_xcb_conn, d, gc, 1, &rectangle);
} /* ecore_x_drawable_rectangle_fill */

