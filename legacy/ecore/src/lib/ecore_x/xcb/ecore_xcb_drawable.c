#include "ecore_xcb_private.h"

/**
 * @defgroup Ecore_X_Drawable_Group X Drawable Functions
 *
 * Functions that operate on drawables.
 */

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
ecore_x_drawable_rectangle_fill(Ecore_X_Drawable draw, Ecore_X_GC gc, int x, int y, int w, int h) 
{
   xcb_rectangle_t rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_poly_fill_rectangle(_ecore_xcb_conn, draw, gc, 1, 
                           (const xcb_rectangle_t *)&rect);
//   ecore_x_flush();
}

/**
 * Retrieves the geometry of the given drawable.
 * @param d The given drawable.
 * @param x Pointer to an integer into which the X position is to be stored.
 * @param y Pointer to an integer into which the Y position is to be stored.
 * @param w Pointer to an integer into which the width is to be stored.
 * @param h Pointer to an integer into which the height is to be stored.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI void 
ecore_x_drawable_geometry_get(Ecore_X_Drawable draw, int *x, int *y, int *w, int *h) 
{
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, draw);
   reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return;
   if (x) *x = reply->x;
   if (y) *y = reply->y;
   if (w) *w = (int)reply->width;
   if (h) *h = (int)reply->height;
   free(reply);
}

/**
 * Retrieves the width of the border of the given drawable.
 * @param  d The given drawable.
 * @return The border width of the given drawable.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI int 
ecore_x_drawable_border_width_get(Ecore_X_Drawable d) 
{
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, d);
   reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   ret = (int)reply->border_width;
   free(reply);
   return ret;
}

/**
 * Retrieves the depth of the given drawable.
 * @param  d The given drawable.
 * @return The depth of the given drawable.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI int 
ecore_x_drawable_depth_get(Ecore_X_Drawable d) 
{
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;
   int ret = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, d);
   reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   ret = (int)reply->depth;
   free(reply);
   return ret;
}
