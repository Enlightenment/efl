/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

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
}


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
}


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
                              int             *x,
                              int             *y,
                              int             *width,
                              int             *height)
{
   xcb_get_geometry_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;
        return;
     }

   if (x) *x = reply->x;
   if (y) *y = reply->y;
   if (width) *width = reply->width;
   if (height) *height = reply->height;
}


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
}


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
}
