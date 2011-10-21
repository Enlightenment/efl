#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "ecore_x_private.h"

/**
 * @defgroup Ecore_X_Drawable_Group X Drawable Functions
 *
 * Functions that operate on drawables.
 */

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
ecore_x_drawable_geometry_get(Ecore_X_Drawable d,
                              int             *x,
                              int             *y,
                              int             *w,
                              int             *h)
{
   Window dummy_win;
   int ret_x, ret_y;
   unsigned int ret_w, ret_h, dummy_border, dummy_depth;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!XGetGeometry(_ecore_x_disp, d, &dummy_win, &ret_x, &ret_y,
                     &ret_w, &ret_h, &dummy_border, &dummy_depth))
     {
        ret_x = 0;
        ret_y = 0;
        ret_w = 0;
        ret_h = 0;
     }

   if (x)
     *x = ret_x;

   if (y)
     *y = ret_y;

   if (w)
     *w = (int)ret_w;

   if (h)
     *h = (int)ret_h;
} /* ecore_x_drawable_geometry_get */

/**
 * Retrieves the width of the border of the given drawable.
 * @param  d The given drawable.
 * @return The border width of the given drawable.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI int
ecore_x_drawable_border_width_get(Ecore_X_Drawable d)
{
   Window dummy_win;
   int dummy_x, dummy_y;
   unsigned int dummy_w, dummy_h, border_ret, dummy_depth;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!XGetGeometry(_ecore_x_disp, d, &dummy_win, &dummy_x, &dummy_y,
                     &dummy_w, &dummy_h, &border_ret, &dummy_depth))
     border_ret = 0;

   return (int)border_ret;
} /* ecore_x_drawable_border_width_get */

/**
 * Retrieves the depth of the given drawable.
 * @param  d The given drawable.
 * @return The depth of the given drawable.
 * @ingroup Ecore_X_Drawable_Group
 */
EAPI int
ecore_x_drawable_depth_get(Ecore_X_Drawable d)
{
   Window dummy_win;
   int dummy_x, dummy_y;
   unsigned int dummy_w, dummy_h, dummy_border, depth_ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (!XGetGeometry(_ecore_x_disp, d, &dummy_win, &dummy_x, &dummy_y,
                     &dummy_w, &dummy_h, &dummy_border, &depth_ret))
     depth_ret = 0;

   return (int)depth_ret;
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
ecore_x_drawable_rectangle_fill(Ecore_X_Drawable d,
                                Ecore_X_GC       gc,
                                int              x,
                                int              y,
                                int              width,
                                int              height)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XFillRectangle(_ecore_x_disp, d, gc, x, y, width, height);
} /* ecore_x_drawable_rectangle_fill */

