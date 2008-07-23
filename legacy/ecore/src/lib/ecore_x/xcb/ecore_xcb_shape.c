/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_Shape_Group X Shape extension
 *
 * Functions that use the shape extension of the X server to change shape of given windows.
 */


#ifdef ECORE_XCB_SHAPE
static int _shape_available = 0;
static xcb_shape_query_version_cookie_t _ecore_xcb_shape_init_cookie;
#endif /* ECORE_XCB_SHAPE */


/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_shape_init and
   _ecore_xcb_shape_init_finalize. The first one gets the cookies and
   the second one gets the replies. */

void
_ecore_x_shape_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_SHAPE
   if (reply && (reply->present))
      _ecore_xcb_shape_init_cookie = xcb_shape_query_version_unchecked(_ecore_xcb_conn);
#endif /* ECORE_XCB_SHAPE */
}

void
_ecore_x_shape_init_finalize(void)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_query_version_reply_t *reply;

   reply = xcb_shape_query_version_reply(_ecore_xcb_conn,
                                         _ecore_xcb_shape_init_cookie,
                                         NULL);
   if (reply)
     {
        _shape_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_SHAPE */
}


/**
 * Sets the shape of the given window to the given pixmap.
 * @param dest_win    The given window.
 * @param source_mask A 2-bit depth pixmap that provides the new shape of the window.
 *
 * Sets the shape of the window @p dest_win to the pixmap @p source_mask.
 * @ingroup Ecore_X_Shape_Group
 */
EAPI void
ecore_x_window_shape_mask_set(Ecore_X_Window dest_win,
                              Ecore_X_Pixmap source_mask)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_mask(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, dest_win, 0, 0, source_mask);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_window_set(Ecore_X_Window dest_win,
                                Ecore_X_Window shape_win)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_BOUNDING, dest_win, 0, 0, shape_win);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_window_set_xy(Ecore_X_Window dest_win,
                                   Ecore_X_Window shape_win,
                                   int            x,
                                   int            y)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_BOUNDING, dest_win, x, y, shape_win);
#endif /* ECORE_XCB_SHAPE */
}


/**
 * Sets the shape of the given window to a rectangle.
 * @param dest_win The given window.
 * @param x        The X coordinate of the top left corner of the rectangle.
 * @param y        The Y coordinate of the top left corner of the rectangle.
 * @param width    The width of the rectangle.
 * @param height   The height of the rectangle.
 *
 * Sets the shape of the window @p dest_win to a rectangle defined  by
 * @p x, @p y, @p width and @p height.
 * @ingroup Ecore_X_Shape_Group
 */
EAPI void
ecore_x_window_shape_rectangle_set(Ecore_X_Window dest_win,
                                   int            x,
                                   int            y,
                                   int            width,
                                   int            height)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;

   rect.x = x;
   rect.y = y;
   rect.width = width;
   rect.height = height;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, 0, dest_win, 0, 0, 1, &rect);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_rectangles_set(Ecore_X_Window     dest_win,
                                    Ecore_X_Rectangle *rects,
                                    int                num)
{
#ifdef ECORE_XCB_SHAPE
   if (num > 0)
     xcb_shape_rectangles(_ecore_xcb_conn,
                          XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING,
                          0, dest_win, 0, 0, num, (xcb_rectangle_t *)rects);
   else
     xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING, 0, dest_win, 0, 0, 0, NULL);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_window_add(Ecore_X_Window dest_win,
                                Ecore_X_Window shape_win)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_UNION, XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_BOUNDING, dest_win, 0, 0, shape_win);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_window_add_xy(Ecore_X_Window dest_win,
                                   Ecore_X_Window shape_win,
                                   int            x,
                                   int            y)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_UNION, XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_BOUNDING, dest_win, x, y, shape_win);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_rectangle_add(Ecore_X_Window dest_win,
                                   int            x,
                                   int            y,
                                   int            width,
                                   int            height)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;

   rect.x = x;
   rect.y = y;
   rect.width = width;
   rect.height = height;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION, XCB_SHAPE_SK_BOUNDING, 0, dest_win, 0, 0, 1, &rect);
#endif /* ECORE_XCB_SHAPE */
}

EAPI void
ecore_x_window_shape_rectangle_clip(Ecore_X_Window dest_win,
                                    int            x,
                                    int            y,
                                    int            width,
                                    int            height)
{
   xcb_rectangle_t rect;
   
   rect.x = x;
   rect.y = y;
   rect.width = width;
   rect.height = height;
   xcb_shape_rectangles(_ecore_xcb_conn,
                        XCB_SHAPE_SO_INTERSECT, XCB_SHAPE_SK_BOUNDING,
                        0, dest_win, 0, 0, 1, &rect);
}

EAPI void
ecore_x_window_shape_rectangles_add(Ecore_X_Window     dest_win,
                                    Ecore_X_Rectangle *rects,
                                    int                num)
{
#ifdef ECORE_XCB_SHAPE
   if (num > 0)
     xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION, XCB_SHAPE_SK_BOUNDING, 0, dest_win, 0, 0, num, (const xcb_rectangle_t *)rects);
   else
     xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION, XCB_SHAPE_SK_BOUNDING, 0, dest_win, 0, 0, 0, NULL);
#endif /* ECORE_XCB_SHAPE */
}


/**
 * Sends the ShapeGetRectangles request.
 * @param window Requested window.
 * @ingroup Ecore_X_Shape_Group
 */
EAPI void
ecore_x_window_shape_rectangles_get_prefetch(Ecore_X_Window window)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_get_rectangles_cookie_t cookie;

   cookie = xcb_shape_get_rectangles_unchecked(_ecore_xcb_conn, window, XCB_SHAPE_SK_BOUNDING);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_SHAPE */
}


/**
 * Gets the reply of the ShapeGetRectangles request sent by ecore_x_window_shape_rectangles_get_prefetch().
 * @ingroup Ecore_X_Shape_Group
 */
EAPI void
ecore_x_window_shape_rectangles_get_fetch(void)
{
#ifdef ECORE_XCB_SHAPE
   xcb_shape_get_rectangles_cookie_t cookie;
   xcb_shape_get_rectangles_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_shape_get_rectangles_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_SHAPE */
}


/**
 * To document.
 * @param  window  Unused.
 * @param  num_ret To document.
 * @return         To document.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_window_shape_rectangles_get_prefetch(), which sends the ShapeGetRectangles request,
 * then ecore_x_window_shape_rectangles_get_fetch(), which gets the reply.
 * @ingroup Ecore_X_Shape_Group
 */
EAPI Ecore_X_Rectangle *
ecore_x_window_shape_rectangles_get(Ecore_X_Window window __UNUSED__,
                                    int           *num_ret)
{
   Ecore_X_Rectangle *rects = NULL;
   uint32_t           num = 0;
#ifdef ECORE_XCB_SHAPE
   xcb_shape_get_rectangles_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     {
       if (num_ret) *num_ret = 0;
       return NULL;
     }

   num = reply->rectangles_len;
   rects = malloc(sizeof(Ecore_X_Rectangle) * num);
   if (rects)
     memcpy (rects,
             xcb_shape_get_rectangles_rectangles(reply),
             num * sizeof (Ecore_X_Rectangle));
   else
     num = 0;
#endif /* ECORE_XCB_SHAPE */

   if (num_ret) *num_ret = num;

   return rects;
}

EAPI void
ecore_x_window_shape_events_select(Ecore_X_Window dest_win,
                                   int            on)
{
#ifdef ECORE_XCB_SHAPE
     xcb_shape_select_input(_ecore_xcb_conn, dest_win, on ? 1 : 0);
#endif /* ECORE_XCB_SHAPE */
}
