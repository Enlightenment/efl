#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_SHAPE
# include <xcb/shape.h>
#endif

/**
 * @defgroup Ecore_X_Window_Shape X Window Shape Functions
 *
 * These functions use the shape extension of the X server to change
 * shape of given windows.
 */

/**
 * Sets the input shape of the given window to that given by the pixmap @p mask.
 * @param   win  The given window.
 * @param   mask A 1-bit depth pixmap that provides the new input shape of the
 *               window.
 * @ingroup Ecore_X_Window_Shape
 */
EAPI void
ecore_x_window_shape_input_mask_set(Ecore_X_Window win,
                                    Ecore_X_Pixmap mask)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_mask(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                  win, 0, 0, mask);
//   ecore_x_flush();
#else
   return;
   win = 0;
   mask = 0;
#endif
}

/**
 * Sets the shape of the given window to that given by the pixmap @p mask.
 * @param   win  The given window.
 * @param   mask A 2-bit depth pixmap that provides the new shape of the
 *               window.
 * @ingroup Ecore_X_Window_Shape
 */
EAPI void
ecore_x_window_shape_mask_set(Ecore_X_Window win,
                              Ecore_X_Pixmap mask)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_mask(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING,
                  win, 0, 0, mask);
//   ecore_x_flush();
#else
   return;
   win = 0;
   mask = 0;
#endif
}

EAPI void
ecore_x_window_shape_window_set(Ecore_X_Window win,
                                Ecore_X_Window shape_win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING,
                     XCB_SHAPE_SK_BOUNDING, win, 0, 0, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
#endif
}

EAPI void
ecore_x_window_shape_window_set_xy(Ecore_X_Window win,
                                   Ecore_X_Window shape_win,
                                   int            x,
                                   int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_BOUNDING,
                     XCB_SHAPE_SK_BOUNDING, win, x, y, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
   x = 0;
   y = 0;
#endif
}

EAPI void
ecore_x_window_shape_rectangle_set(Ecore_X_Window win,
                                   int            x,
                                   int            y,
                                   int            w,
                                   int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SET,
                        XCB_SHAPE_SK_BOUNDING, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_rectangles_set(Ecore_X_Window     win,
                                    Ecore_X_Rectangle *rects,
                                    int                num)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t *rect = NULL;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!rects) return;

#ifdef ECORE_XCB_SHAPE
   if (num > 0)
     {
        int i = 0;

        if (!(rect = malloc(sizeof(xcb_rectangle_t) * num)))
          return;

        for (i = 0; i < num; i++)
          {
             rect[i].x = rects[i].x;
             rect[i].y = rects[i].y;
             rect[i].width = rects[i].width;
             rect[i].height = rects[i].height;
          }
     }
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SET,
                        XCB_SHAPE_SK_BOUNDING, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, num, (xcb_rectangle_t *)rect);

   if (rect) free(rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   num = 0;
   rects = NULL;
#endif
}

EAPI void
ecore_x_window_shape_window_add(Ecore_X_Window win,
                                Ecore_X_Window shape_win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_UNION,
                     XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_BOUNDING,
                     win, 0, 0, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
#endif
}

EAPI void
ecore_x_window_shape_window_add_xy(Ecore_X_Window win,
                                   Ecore_X_Window shape_win,
                                   int            x,
                                   int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_UNION,
                     XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_BOUNDING,
                     win, x, y, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
   x = 0;
   y = 0;
#endif
}

EAPI void
ecore_x_window_shape_rectangle_add(Ecore_X_Window win,
                                   int            x,
                                   int            y,
                                   int            w,
                                   int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION,
                        XCB_SHAPE_SK_BOUNDING, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_rectangle_subtract(Ecore_X_Window win,
                                        int            x,
                                        int            y,
                                        int            w,
                                        int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SUBTRACT,
                        XCB_SHAPE_SK_BOUNDING, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_rectangle_clip(Ecore_X_Window win,
                                    int            x,
                                    int            y,
                                    int            w,
                                    int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_INTERSECT,
                        XCB_SHAPE_SK_BOUNDING, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_rectangles_add(Ecore_X_Window     win,
                                    Ecore_X_Rectangle *rects,
                                    int                num)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t *rect = NULL;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   if (num > 0)
     {
        int i = 0;

        if (!(rect = malloc(sizeof(xcb_rectangle_t) * num)))
          return;

        for (i = 0; i < num; i++)
          {
             rect[i].x = rects[i].x;
             rect[i].y = rects[i].y;
             rect[i].width = rects[i].width;
             rect[i].height = rects[i].height;
          }
     }

   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION,
                        XCB_SHAPE_SK_BOUNDING, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, num, (xcb_rectangle_t *)&rect);

   if (rect) free(rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   num = 0;
   rects = NULL;
#endif
}

EAPI Ecore_X_Rectangle *
ecore_x_window_shape_rectangles_get(Ecore_X_Window win,
                                    int           *num_ret)
{
   Ecore_X_Rectangle *rects = NULL;
#ifdef ECORE_XCB_SHAPE
   xcb_shape_get_rectangles_cookie_t cookie;
   xcb_shape_get_rectangles_reply_t *reply;
   xcb_rectangle_t *r;
   unsigned int i = 0;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (num_ret) *num_ret = 0;

#ifdef ECORE_XCB_SHAPE
   cookie =
     xcb_shape_get_rectangles(_ecore_xcb_conn, win, XCB_SHAPE_SK_BOUNDING);
   reply = xcb_shape_get_rectangles_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;
   if (num_ret) *num_ret = reply->rectangles_len;

   if (reply->rectangles_len < 1)
     {
        free(reply);
        if (num_ret) *num_ret = 0;
        return NULL;
     }

   rects = malloc(sizeof(Ecore_X_Rectangle) * reply->rectangles_len);
   if (!rects)
     {
        free(reply);
        if (num_ret) *num_ret = 0;
        return NULL;
     }
   r = xcb_shape_get_rectangles_rectangles(reply);
   for (i = 0; i < reply->rectangles_len; i++)
     {
        rects[i].x = r[i].x;
        rects[i].y = r[i].y;
        rects[i].width = r[i].width;
        rects[i].height = r[i].height;
     }

   free(reply);

   return rects;
#else
   return rects;
   win = 0;
#endif
}

EAPI void
ecore_x_window_shape_events_select(Ecore_X_Window win,
                                   Eina_Bool      on)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_select_input(_ecore_xcb_conn, win, on);
//   ecore_x_flush();
#else
   return;
   win = 0;
   on = 0;
#endif
}

EAPI Ecore_X_Rectangle *
ecore_x_window_shape_input_rectangles_get(Ecore_X_Window win,
                                          int           *num_ret)
{
   Ecore_X_Rectangle *rects = NULL;
#ifdef ECORE_XCB_SHAPE
   xcb_shape_get_rectangles_cookie_t cookie;
   xcb_shape_get_rectangles_reply_t *reply;
   xcb_rectangle_t *r;
   unsigned int i = 0;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (num_ret) *num_ret = 0;

#ifdef ECORE_XCB_SHAPE
   cookie =
     xcb_shape_get_rectangles(_ecore_xcb_conn, win, XCB_SHAPE_SK_INPUT);
   reply = xcb_shape_get_rectangles_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return NULL;
   if (num_ret) *num_ret = reply->rectangles_len;

   if (reply->rectangles_len < 1)
     {
        free(reply);
        if (num_ret) *num_ret = 0;
        return NULL;
     }

   rects = malloc(sizeof(Ecore_X_Rectangle) * reply->rectangles_len);
   if (!rects)
     {
        free(reply);
        if (num_ret) *num_ret = 0;
        return NULL;
     }
   r = xcb_shape_get_rectangles_rectangles(reply);
   for (i = 0; i < reply->rectangles_len; i++)
     {
        rects[i].x = r[i].x;
        rects[i].y = r[i].y;
        rects[i].width = r[i].width;
        rects[i].height = r[i].height;
     }

   free(reply);

   return rects;
#else
   xcb_get_geometry_cookie_t cookie;
   xcb_get_geometry_reply_t *reply;

   if (!(rects = malloc(sizeof(Ecore_X_Rectangle))))
     return NULL;

   /* get geometry */
   cookie = xcb_get_geometry_unchecked(_ecore_xcb_conn, win);
   reply = xcb_get_geometry_reply(_ecore_xcb_conn, cookie, NULL);
   if (reply)
     {
        rects[0].x = reply->x;
        rects[0].y = reply->y;
        rects[0].width = reply->width;
        rects[0].height = reply->height;
        free(reply);
     }
   if (num_ret) *num_ret = 1;
   return rects;
#endif
}

EAPI void
ecore_x_window_shape_input_rectangles_set(Ecore_X_Window     win,
                                          Ecore_X_Rectangle *rects,
                                          int                num)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t *rect = NULL;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!rects) return;

#ifdef ECORE_XCB_SHAPE
   if (num > 0)
     {
        int i = 0;

        if (!(rect = malloc(sizeof(xcb_rectangle_t) * num)))
          return;

        for (i = 0; i < num; i++)
          {
             rect[i].x = rects[i].x;
             rect[i].y = rects[i].y;
             rect[i].width = rects[i].width;
             rect[i].height = rects[i].height;
          }
     }
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SET,
                        XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, num, (xcb_rectangle_t *)rect);

   if (rect) free(rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   num = 0;
   rects = NULL;
#endif
}

EAPI void
ecore_x_window_shape_input_rectangle_subtract(Ecore_X_Window win,
                                              int            x,
                                              int            y,
                                              int            w,
                                              int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SUBTRACT,
                        XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_rectangle_add(Ecore_X_Window win,
                                         int            x,
                                         int            y,
                                         int            w,
                                         int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION,
                        XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_rectangle_set(Ecore_X_Window win,
                                         int            x,
                                         int            y,
                                         int            w,
                                         int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_SET,
                        XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_window_set_xy(Ecore_X_Window win,
                                         Ecore_X_Window shape_win,
                                         int            x,
                                         int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                     XCB_SHAPE_SK_INPUT, win, x, y, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
   x = 0;
   y = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_window_add_xy(Ecore_X_Window win,
                                         Ecore_X_Window shape_win,
                                         int            x,
                                         int            y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_UNION, XCB_SHAPE_SK_INPUT,
                     XCB_SHAPE_SK_INPUT, win, x, y, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
   x = 0;
   y = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_window_set(Ecore_X_Window win,
                                      Ecore_X_Window shape_win)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   xcb_shape_combine(_ecore_xcb_conn, XCB_SHAPE_SO_SET, XCB_SHAPE_SK_INPUT,
                     XCB_SHAPE_SK_INPUT, win, 0, 0, shape_win);
//   ecore_x_flush();
#else
   return;
   win = 0;
   shape_win = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_rectangle_clip(Ecore_X_Window win,
                                          int            x,
                                          int            y,
                                          int            w,
                                          int            h)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t rect;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   rect.x = x;
   rect.y = y;
   rect.width = w;
   rect.height = h;
   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_INTERSECT,
                        XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, 1, &rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
#endif
}

EAPI void
ecore_x_window_shape_input_rectangles_add(Ecore_X_Window     win,
                                          Ecore_X_Rectangle *rects,
                                          int                num)
{
#ifdef ECORE_XCB_SHAPE
   xcb_rectangle_t *rect = NULL;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

#ifdef ECORE_XCB_SHAPE
   if (num > 0)
     {
        int i = 0;

        if (!(rect = malloc(sizeof(xcb_rectangle_t) * num)))
          return;

        for (i = 0; i < num; i++)
          {
             rect[i].x = rects[i].x;
             rect[i].y = rects[i].y;
             rect[i].width = rects[i].width;
             rect[i].height = rects[i].height;
          }
     }

   xcb_shape_rectangles(_ecore_xcb_conn, XCB_SHAPE_SO_UNION,
                        XCB_SHAPE_SK_INPUT, XCB_CLIP_ORDERING_UNSORTED,
                        win, 0, 0, num, (xcb_rectangle_t *)&rect);

   if (rect) free(rect);
//   ecore_x_flush();
#else
   return;
   win = 0;
   num = 0;
   rects = NULL;
#endif
}

