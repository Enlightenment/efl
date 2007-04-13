/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_Xinerama_Group X Xinerama Extension Functions
 *
 * Functions related to the X Xinerama extension.
 */


#ifdef ECORE_XCB_XINERAMA
static int _xinerama_available = 0;
static xcb_xinerama_query_version_cookie_t _ecore_xcb_xinerama_init_cookie;
#endif /* ECORE_XCB_XINERAMA */


/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_xinerama_init and
   _ecore_xcb_xinerama_init_finalize. The first one gets the cookies and
   the second one gets the replies. */

void
_ecore_x_xinerama_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_XINERAMA
   if (reply && (reply->present))
      _ecore_xcb_xinerama_init_cookie = xcb_xinerama_query_version_unchecked(_ecore_xcb_conn, 1, 2);
#endif /* ECORE_XCB_XINERAMA */
}

void
_ecore_x_xinerama_init_finalize(void)
{
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_query_version_reply_t *reply;

   reply = xcb_xinerama_query_version_reply(_ecore_xcb_conn,
                                          _ecore_xcb_xinerama_init_cookie, NULL);

   if (reply)
     {
        if ((reply->major >= 1) &&
            (reply->minor >= 1))
          _xinerama_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_XINERAMA */
}


/**
 * Return whether the X server supports the Xinerama Extension.
 * @return 1 if the X Xinerama Extension is available, 0 otherwise.
 *
 * Return 1 if the X server supports the Fixes Xinerama version 1.1,
 * 0 otherwise.
 * @ingroup Ecore_X_Xinerama_Group
 */
EAPI int
ecore_x_xinerama_query(void)
{
#ifdef ECORE_XCB_XINERAMA
   return _xinerama_available;
#else
   return 0;
#endif /* ECORE_XCB_XINERAMA */
}


/**
 * Sends the XineramaQueryScreens request.
 * @ingroup Ecore_X_Xinerama_Group
 */
EAPI void
ecore_x_xinerama_query_screens_prefetch(void)
{
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_query_screens_cookie_t cookie;

   cookie = xcb_xinerama_query_screens_unchecked(_ecore_xcb_conn);
   _ecore_xcb_cookie_cache(cookie.sequence);
#endif /* ECORE_XCB_XINERAMA */
}


/**
 * Gets the reply of the XineramaQueryScreens request sent by ecore_x_xinerama_query_screens_prefetch().
 * @ingroup Ecore_X_Xinerama_Group
 */
EAPI void
ecore_x_xinerama_query_screens_fetch(void)
{
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_query_screens_cookie_t cookie;
   xcb_xinerama_query_screens_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply = xcb_xinerama_query_screens_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
#endif /* ECORE_XCB_XINERAMA */
}


/**
 * Return the number of screens.
 * @return The screen count.
 *
 * Return the number of screens.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_xinerama_query_screens_prefetch(), which sends the XineramaQueryScreens request,
 * then ecore_x_xinerama_query_screens_fetch(), which gets the reply.
 * @ingroup Ecore_X_Xinerama_Group
 */
EAPI int
ecore_x_xinerama_screen_count_get(void)
{
   int                                 screen_count = 0;
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_screen_info_iterator_t iter;
   xcb_xinerama_query_screens_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply) return 0;

   iter = xcb_xinerama_query_screens_screen_info_iterator(reply);
   screen_count = iter.rem;
#endif /* ECORE_XCB_XINERAMA */

   return screen_count;
}


/**
 * Get the geometry of the screen.
 * @param screen The screen (Unused).
 * @param x      The X coordinate of the screen.
 * @param y      The Y coordinate of the screen
 * @param width  The width of the screen
 * @param height The height of the screen
 * @return       1 on success, 0 otherwise.
 *
 * Get the geometry of the screen whose number is @p screen. The
 * returned values are stored in @p x, @p y, @p width and @p height.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_xinerama_query_screens_prefetch(), which sends the XineramaQueryScreens request,
 * then ecore_x_xinerama_query_screens_fetch(), which gets the reply.
 * @ingroup Ecore_X_Xinerama_Group
 */
EAPI int
ecore_x_xinerama_screen_geometry_get(int  screen,
                                     int *x,
                                     int *y,
                                     int *width,
                                     int *height)
{
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_screen_info_iterator_t iter;
   xcb_xinerama_query_screens_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = ((xcb_screen_t *)_ecore_xcb_screen)->width_in_pixels;
        if (height) *height = ((xcb_screen_t *)_ecore_xcb_screen)->height_in_pixels;

        return 0;
     }

   iter = xcb_xinerama_query_screens_screen_info_iterator(reply);
   for (; iter.rem; screen--, xcb_xinerama_screen_info_next(&iter))
     {
        if (screen == 0)
          {
             if (x) *x = iter.data->x_org;
             if (y) *y = iter.data->y_org;
             if (width) *width = iter.data->width;
             if (height) *height = iter.data->height;
             return 1;
          }
     }
#endif /* ECORE_XCB_XINERAMA */

   if (x) *x = 0;
   if (y) *y = 0;
   if (width) *width = ((xcb_screen_t *)_ecore_xcb_screen)->width_in_pixels;
   if (height) *height = ((xcb_screen_t *)_ecore_xcb_screen)->height_in_pixels;

   return 0;
}
