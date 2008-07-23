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
   XShapeCombineMask(_ecore_x_disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
}

EAPI void
ecore_x_window_shape_window_set(Ecore_X_Window win, Ecore_X_Window shape_win)
{
   XShapeCombineShape(_ecore_x_disp, win, ShapeBounding, 0, 0, shape_win, ShapeBounding, ShapeSet);
}

EAPI void
ecore_x_window_shape_window_set_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y)
{
   XShapeCombineShape(_ecore_x_disp, win, ShapeBounding, x, y, shape_win, ShapeBounding, ShapeSet);
}

EAPI void
ecore_x_window_shape_rectangle_set(Ecore_X_Window win, int x, int y, int w, int h)
{
   XRectangle rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(_ecore_x_disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeSet, Unsorted);
}

EAPI void
ecore_x_window_shape_rectangles_set(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num)
{
   XRectangle *rect = NULL;
   int i;
   
   if (num > 0)
     {
	rect = malloc(sizeof(XRectangle) * num);
	if (rect)
	  {
	     for (i = 0; i < num; i++)
	       {
		  rect[i].x = rects[i].x;
		  rect[i].y = rects[i].y;
		  rect[i].width = rects[i].width;
		  rect[i].height = rects[i].height;
	       }
	  }
	else
	  num = 0;
     }
   XShapeCombineRectangles(_ecore_x_disp, win, ShapeBounding, 0, 0, rect, num, ShapeSet, Unsorted);
   if (rect) free(rect);
}

EAPI void
ecore_x_window_shape_window_add(Ecore_X_Window win, Ecore_X_Window shape_win)
{
   XShapeCombineShape(_ecore_x_disp, win, ShapeBounding, 0, 0, shape_win, ShapeBounding, ShapeUnion);
}

EAPI void
ecore_x_window_shape_window_add_xy(Ecore_X_Window win, Ecore_X_Window shape_win, int x, int y)
{
   XShapeCombineShape(_ecore_x_disp, win, ShapeBounding, x, y, shape_win, ShapeBounding, ShapeUnion);
}

EAPI void
ecore_x_window_shape_rectangle_add(Ecore_X_Window win, int x, int y, int w, int h)
{
   XRectangle rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(_ecore_x_disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeUnion, Unsorted);
}

EAPI void
ecore_x_window_shape_rectangle_clip(Ecore_X_Window win, int x, int y, int w, int h)
{
   XRectangle rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   XShapeCombineRectangles(_ecore_x_disp, win, ShapeBounding, 0, 0, &rect, 1, ShapeIntersect, Unsorted);
}

EAPI void
ecore_x_window_shape_rectangles_add(Ecore_X_Window win, Ecore_X_Rectangle *rects, int num)
{
   XRectangle *rect = NULL;
   int i;
   
   if (num > 0)
     {
	rect = malloc(sizeof(XRectangle) * num);
	if (rect)
	  {
	     for (i = 0; i < num; i++)
	       {
		  rect[i].x = rects[i].x;
		  rect[i].y = rects[i].y;
		  rect[i].width = rects[i].width;
		  rect[i].height = rects[i].height;
	       }
	  }
	else
	  num = 0;
     }
   XShapeCombineRectangles(_ecore_x_disp, win, ShapeBounding, 0, 0, rect, num, ShapeUnion, Unsorted);
   if (rect) free(rect);
}

EAPI Ecore_X_Rectangle *
ecore_x_window_shape_rectangles_get(Ecore_X_Window win, int *num_ret)
{
   XRectangle *rect;
   Ecore_X_Rectangle *rects = NULL;
   int i, num = 0, ord;
   
   rect = XShapeGetRectangles(_ecore_x_disp, win, ShapeBounding, &num, &ord);
   if (rect)
     {
	rects = malloc(sizeof(Ecore_X_Rectangle) * num);
	if (rects)
	  {
	     for (i = 0; i < num; i++)
	       {
		  rects[i].x = rect[i].x;
		  rects[i].y = rect[i].y;
		  rects[i].width = rect[i].width;
		  rects[i].height = rect[i].height;
	       }
	  }
	XFree(rect);
     }
   if (num_ret) *num_ret = num;
   return rects;
}

EAPI void
ecore_x_window_shape_events_select(Ecore_X_Window win, int on)
{
   if (on)
     XShapeSelectInput(_ecore_x_disp, win, ShapeNotifyMask);
   else
     XShapeSelectInput(_ecore_x_disp, win, 0);
}

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
#ifdef ShapeInput
   XShapeCombineMask(_ecore_x_disp, win, ShapeInput, 0, 0, mask, ShapeSet);
#else   
   XShapeCombineMask(_ecore_x_disp, win, ShapeBounding, 0, 0, mask, ShapeSet);
#endif
}

