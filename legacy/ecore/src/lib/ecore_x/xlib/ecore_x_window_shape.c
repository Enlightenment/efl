/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

/**
 * @defgroup Ecore_X_Window_Shape X Window Shape Functions
 *
 * These functions use the shape extension of the X server to change
 * shape of given windows.
 */

/**
 * Sets the shape of the given window to that given by the pixmap @p mask.
 * @param   win  The given window.
 * @param   mask A 2-bit depth pixmap that provides the new shape of the
 *               window.
 * @ingroup Ecore_X_Window_Shape
 */
EAPI void
ecore_x_window_shape_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XShapeCombineMask(_ecore_x_disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
} /* ecore_x_window_shape_mask_set */

EAPI void
ecore_x_window_shape_window_set(Ecore_X_Window win, Ecore_X_Window shape_win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XShapeCombineShape(_ecore_x_disp,
                      win,
                      ShapeBounding,
                      0,
                      0,
                      shape_win,
                      ShapeBounding,
                      ShapeSet);
} /* ecore_x_window_shape_window_set */

EAPI void
ecore_x_window_shape_window_set_xy(Ecore_X_Window win,
                                   Ecore_X_Window shape_win,
                                   int            x,
                                   int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XShapeCombineShape(_ecore_x_disp,
                      win,
                      ShapeBounding,
                      x,
                      y,
                      shape_win,
                      ShapeBounding,
                      ShapeSet);
} /* ecore_x_window_shape_window_set_xy */

EAPI void
ecore_x_window_shape_rectangle_set(Ecore_X_Window win,
                                   int            x,
                                   int            y,
                                   int            w,
                                   int            h)
{
   XRectangle rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(_ecore_x_disp,
                           win,
                           ShapeBounding,
                           0,
                           0,
                           &rect,
                           1,
                           ShapeSet,
                           Unsorted);
} /* ecore_x_window_shape_rectangle_set */

EAPI void
ecore_x_window_shape_rectangles_set(Ecore_X_Window     win,
                                    Ecore_X_Rectangle *rects,
                                    int                num)
{
   XRectangle *rect = NULL;
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (num > 0)
     {
        rect = malloc(sizeof(XRectangle) * num);
        if (rect)
           for (i = 0; i < num; i++)
             {
                rect[i].x = rects[i].x;
                rect[i].y = rects[i].y;
                rect[i].width = rects[i].width;
                rect[i].height = rects[i].height;
             }
        else
           num = 0;
     }

   XShapeCombineRectangles(_ecore_x_disp,
                           win,
                           ShapeBounding,
                           0,
                           0,
                           rect,
                           num,
                           ShapeSet,
                           Unsorted);
   if (rect)
      free(rect);
} /* ecore_x_window_shape_rectangles_set */

EAPI void
ecore_x_window_shape_window_add(Ecore_X_Window win, Ecore_X_Window shape_win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XShapeCombineShape(_ecore_x_disp,
                      win,
                      ShapeBounding,
                      0,
                      0,
                      shape_win,
                      ShapeBounding,
                      ShapeUnion);
} /* ecore_x_window_shape_window_add */

EAPI void
ecore_x_window_shape_window_add_xy(Ecore_X_Window win,
                                   Ecore_X_Window shape_win,
                                   int            x,
                                   int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XShapeCombineShape(_ecore_x_disp,
                      win,
                      ShapeBounding,
                      x,
                      y,
                      shape_win,
                      ShapeBounding,
                      ShapeUnion);
} /* ecore_x_window_shape_window_add_xy */

EAPI void
ecore_x_window_shape_rectangle_add(Ecore_X_Window win,
                                   int            x,
                                   int            y,
                                   int            w,
                                   int            h)
{
   XRectangle rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(_ecore_x_disp,
                           win,
                           ShapeBounding,
                           0,
                           0,
                           &rect,
                           1,
                           ShapeUnion,
                           Unsorted);
} /* ecore_x_window_shape_rectangle_add */

EAPI void
ecore_x_window_shape_rectangle_clip(Ecore_X_Window win,
                                    int            x,
                                    int            y,
                                    int            w,
                                    int            h)
{
   XRectangle rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(_ecore_x_disp,
                           win,
                           ShapeBounding,
                           0,
                           0,
                           &rect,
                           1,
                           ShapeIntersect,
                           Unsorted);
} /* ecore_x_window_shape_rectangle_clip */

EAPI void
ecore_x_window_shape_rectangles_add(Ecore_X_Window     win,
                                    Ecore_X_Rectangle *rects,
                                    int                num)
{
   XRectangle *rect = NULL;
   int i;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (num > 0)
     {
        rect = malloc(sizeof(XRectangle) * num);
        if (rect)
           for (i = 0; i < num; i++)
             {
                rect[i].x = rects[i].x;
                rect[i].y = rects[i].y;
                rect[i].width = rects[i].width;
                rect[i].height = rects[i].height;
             }
        else
           num = 0;
     }

   XShapeCombineRectangles(_ecore_x_disp,
                           win,
                           ShapeBounding,
                           0,
                           0,
                           rect,
                           num,
                           ShapeUnion,
                           Unsorted);
   if (rect)
      free(rect);
} /* ecore_x_window_shape_rectangles_add */

EAPI Ecore_X_Rectangle *
ecore_x_window_shape_rectangles_get(Ecore_X_Window win, int *num_ret)
{
   XRectangle *rect;
   Ecore_X_Rectangle *rects = NULL;
   int i, num = 0, ord;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   rect = XShapeGetRectangles(_ecore_x_disp, win, ShapeBounding, &num, &ord);
   if (rect)
     {
        rects = malloc(sizeof(Ecore_X_Rectangle) * num);
        if (rects)
           for (i = 0; i < num; i++)
             {
                rects[i].x = rect[i].x;
                rects[i].y = rect[i].y;
                rects[i].width = rect[i].width;
                rects[i].height = rect[i].height;
             }

        XFree(rect);
     }

   if (num_ret)
      *num_ret = num;

   return rects;
} /* ecore_x_window_shape_rectangles_get */

EAPI void
ecore_x_window_shape_events_select(Ecore_X_Window win, int on)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   if (on)
      XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
   else
      XShapeSelectInput(_ecore_x_disp, win, 0);
} /* ecore_x_window_shape_events_select */

/**
 * Sets the input shape of the given window to that given by the pixmap @p mask.
 * @param   win  The given window.
 * @param   mask A 2-bit depth pixmap that provides the new input shape of the
 *               window.
 * @ingroup Ecore_X_Window_Shape
 */
EAPI void
ecore_x_window_shape_input_mask_set(Ecore_X_Window win, Ecore_X_Pixmap mask)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
#ifdef ShapeInput
   XShapeCombineMask(_ecore_x_disp, win, ShapeInput,    0, 0, mask, ShapeSet);
#else /* ifdef ShapeInput */
   XShapeCombineMask(_ecore_x_disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
#endif /* ifdef ShapeInput */
} /* ecore_x_window_shape_input_mask_set */

