/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"


/**
 * @defgroup Ecore_X_RandR_Group X RandR Extension Functions
 *
 * Functions related to the X RandR extension.
 */


#ifdef ECORE_XCB_RANDR
static int _randr_available = 0;
static xcb_randr_query_version_cookie_t _ecore_xcb_randr_init_cookie;
#endif /* ECORE_XCB_RANDR */


/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_randr_init and
   _ecore_xcb_randr_init_finalize. The first one gets the cookies and
   the second one gets the replies and set the atoms. */

void
_ecore_x_randr_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_RANDR
   if (reply && (reply->present))
      _ecore_xcb_randr_init_cookie = xcb_randr_query_version_unchecked(_ecore_xcb_conn, 1, 2);
#endif /* ECORE_XCB_RANDR */
}

void
_ecore_x_randr_init_finalize(void)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_query_version_reply_t *reply;

   reply = xcb_randr_query_version_reply(_ecore_xcb_conn,
                                         _ecore_xcb_randr_init_cookie, NULL);

   if (reply)
     {
        if ((reply->major_version >= 1) &&
            (reply->minor_version >= 1))
          _randr_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_RANDR */
}

/**
 * Return whether the X server supports the RandR Extension.
 * @return 1 if the X RandR Extension is available, 0 otherwise.
 *
 * Return 1 if the X server supports the RandR Extension version 1.1,
 * 0 otherwise.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI int
ecore_x_randr_query(void)
{
#ifdef ECORE_XCB_RANDR
   return _randr_available;
#else
   return 0;
#endif /* ECORE_XCB_RANDR */
}


static Ecore_X_Window
_xcb_randr_root_to_screen(Ecore_X_Window root)
{
   xcb_screen_iterator_t iter;

   iter = xcb_setup_roots_iterator(xcb_get_setup(_ecore_xcb_conn));
   for (; iter.rem; xcb_screen_next(&iter))
     {
        if (iter.data->root == root)
          return iter.data->root;
     }

   return XCB_NONE;
}

/**
 * Select if the ScreenChangeNotify events  will be sent.
 * @param window The window.
 * @param on     1 to enable, 0 to disable.
 * @return       1 on success, 0 otherwise.
 *
 * If @p on value is @c 1, ScreenChangeNotify events
 * will be sent when the screen configuration changes, either from
 * this protocol extension, or due to detected external screen
 * configuration changes. ScreenChangeNotify may also be sent when
 * this request executes if the screen configuration has changed since
 * the client connected, to avoid race conditions.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI int
ecore_x_randr_events_select(Ecore_X_Window window,
                            int            on)
{
#ifdef ECORE_XCB_RANDR
     xcb_randr_select_input(_ecore_xcb_conn, window,
                            on ? XCB_RANDR_SCREEN_CHANGE_NOTIFY : 0);
     return 1;
#else
   return 0;
#endif /* ECORE_XCB_RANDR */
}

/**
 * Sends the GetScreenInfo request.
 * @param window Window whose properties are requested.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI void
ecore_x_randr_get_screen_info_prefetch(Ecore_X_Window window)
{
   xcb_randr_get_screen_info_cookie_t cookie;

   cookie = xcb_randr_get_screen_info_unchecked(_ecore_xcb_conn,
                                                _xcb_randr_root_to_screen(window));
   _ecore_xcb_cookie_cache(cookie.sequence);
}


/**
 * Gets the reply of the GetScreenInfo request sent by ecore_x_randr_get_screen_info_prefetch().
 * @ingroup Ecore_X_RandR_Group
 */
EAPI void
ecore_x_randr_get_screen_info_fetch(void)
{
   xcb_randr_get_screen_info_cookie_t cookie;
   xcb_randr_get_screen_info_reply_t *reply;

   cookie.sequence = _ecore_xcb_cookie_get();
   reply =xcb_randr_get_screen_info_reply(_ecore_xcb_conn, cookie, NULL);
   _ecore_xcb_reply_cache(reply);
}

/**
 * Get the set of rotations and reflections.
 * @param root The window (Unused).
 * @return     The set of rotations and reflections.
 *
 * Get the set of rotations and reflections supported by the screen
 * associated to @p window (passed to
 * ecore_x_randr_get_screen_info_prefetch()).
 *
 * To use this function, you must call before, and in order,
 * ecore_x_randr_get_screen_info_prefetch(), which sends the GetScreenInfo request,
 * then ecore_x_randr_get_screen_info_fetch(), which gets the reply.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI Ecore_X_Randr_Rotation
ecore_x_randr_screen_rotations_get(Ecore_X_Window root __UNUSED__)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_get_screen_info_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return 0;

   return reply->rotations;
#else
   return 0;
#endif /* ECORE_XCB_RANDR */
}

/**
 * Get the rotation.
 * @param root The window (Unused).
 * @return     The rotation.
 *
 * Get the rotation supported by the screen
 * associated to @p window (passed to
 * ecore_x_randr_get_screen_info_prefetch()).
 *
 * To use this function, you must call before, and in order,
 * ecore_x_randr_get_screen_info_prefetch(), which sends the GetScreenInfo request,
 * then ecore_x_randr_get_screen_info_fetch(), which gets the reply.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI Ecore_X_Randr_Rotation
ecore_x_randr_screen_rotation_get(Ecore_X_Window root __UNUSED__)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_get_screen_info_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return 0;

   return reply->rotation;
#else
   return 0;
#endif /* ECORE_XCB_RANDR */
}

/**
 * Get the frame buffer sizes.
 * @param root The window (Unused).
 * @param num  The number of sizes.
 * @return     The sizes.
 *
 * Get the list of possible frame buffer sizes (at the normal
 * orientation supported by the screen associated to @p window (passed
 * to ecore_x_randr_get_screen_info_prefetch()). Each size indicates
 * both the linear physical size of the screen and the pixel size.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_randr_get_screen_info_prefetch(), which sends the GetScreenInfo request,
 * then ecore_x_randr_get_screen_info_fetch(), which gets the reply.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI Ecore_X_Screen_Size *
ecore_x_randr_screen_sizes_get(Ecore_X_Window root __UNUSED__,
                               int           *num)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_get_screen_info_reply_t *reply;
   xcb_randr_screen_size_t           *sizes;
   Ecore_X_Screen_Size               *ret;
   int                                n;
   int                                i;

   if (num) *num = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return NULL;

   n = xcb_randr_get_screen_info_sizes_length(reply);
   ret = calloc(n, sizeof(Ecore_X_Screen_Size));
   if (!ret) return NULL;

   if (num) *num = n;
   sizes = xcb_randr_get_screen_info_sizes(reply);
   for (i = 0; i < n; i++)
     {
        ret[i].width = sizes[i].width;
        ret[i].height = sizes[i].height;
     }

   return ret;
#else
   if (num) *num = 0;
   return NULL;
#endif /* ECORE_XCB_RANDR */
}

/**
 * Get the current frame buffer size.
 * @param root The window (Unused).
 * @return     The active size.
 *
 * Get the active frame buffer size supported by the screen associated
 * to @p window (passed to
 * ecore_x_randr_get_screen_info_prefetch()).
 *
 * To use this function, you must call before, and in order,
 * ecore_x_randr_get_screen_info_prefetch(), which sends the GetScreenInfo request,
 * then ecore_x_randr_get_screen_info_fetch(), which gets the reply.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI Ecore_X_Screen_Size
ecore_x_randr_current_screen_size_get(Ecore_X_Window root __UNUSED__)
{
   Ecore_X_Screen_Size ret = { -1, -1 };
#ifdef ECORE_XCB_RANDR
   xcb_randr_get_screen_info_reply_t *reply;
   xcb_randr_screen_size_t           *sizes;
   uint16_t                           size_index;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return ret;

   size_index = reply->sizeID;
   sizes = xcb_randr_get_screen_info_sizes(reply);
   if (size_index < reply->nSizes)
     {
        ret.width = sizes[size_index].mwidth;
        ret.height = sizes[size_index].mheight;
     }
#endif /* ECORE_XCB_RANDR */

   return ret;
}

/**
 * Get the current refresh rate.
 * @param root The window (Unused).
 * @return     The current refresh rate.
 *
 * Get the current refresh rate supported by the screen associated
 * to @p window (passed to
 * ecore_x_randr_get_screen_info_prefetch()).
 *
 * To use this function, you must call before, and in order,
 * ecore_x_randr_get_screen_info_prefetch(), which sends the GetScreenInfo request,
 * then ecore_x_randr_get_screen_info_fetch(), which gets the reply.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI Ecore_X_Screen_Refresh_Rate
ecore_x_randr_current_screen_refresh_rate_get(Ecore_X_Window root __UNUSED__)
{
   Ecore_X_Screen_Refresh_Rate        ret = { -1 };
#ifdef ECORE_XCB_RANDR
   xcb_randr_get_screen_info_reply_t *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return ret;

   ret.rate = reply->rate;
#endif /* ECORE_XCB_RANDR */

   return ret;
}

/**
 * Get the refresh rates.
 * @param root The window (Unused).
 * @param num  The number of refresh rates.
 * @return     The refresh rates.
 *
 * Get the list of refresh rates for each size supported by the screen
 * associated to @p window (passed to
 * ecore_x_randr_get_screen_info_prefetch()). Each element
 * of 'sizes' has a corresponding element in 'refresh'. An empty list
 * indicates no known rates, or a device for which refresh is not
 * relevant.
 *
 * To use this function, you must call before, and in order,
 * ecore_x_randr_get_screen_info_prefetch(), which sends the GetScreenInfo request,
 * then ecore_x_randr_get_screen_info_fetch(), which gets the reply.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI Ecore_X_Screen_Refresh_Rate *
ecore_x_randr_screen_refresh_rates_get(Ecore_X_Window root __UNUSED__,
                                       int            size_id __UNUSED__,
                                       int           *num)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_get_screen_info_reply_t *reply;
   Ecore_X_Screen_Refresh_Rate       *ret;
   Ecore_X_Screen_Refresh_Rate       *tmp;
   xcb_randr_refresh_rates_iterator_t iter;
   uint16_t                           n;

   if (num) *num = 0;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return NULL;

   n = reply->nSizes;
   ret = calloc(n, sizeof(Ecore_X_Screen_Refresh_Rate));
   if (!ret)
     return NULL;

   if (num) *num = n;

   /* FIXME: maybe there's a missing function in xcb randr implementation */
   iter = xcb_randr_get_screen_info_rates_iterator(reply);
   tmp = ret;
   for (; iter.rem; xcb_randr_refresh_rates_next(&iter), tmp++)
     {
       tmp->rate = iter.data->nRates;;
     }

   return ret;
#else
   if (num) *num = 0;
   return NULL;
#endif /* ECORE_XCB_RANDR */
}

/* FIXME: round trip. Should we remove it ? */

/**
 * Set the screen rotation.
 * @param root The root window.
 * @param rot  The rotation.
 *
 * Set the rotation of the screen associated to @p root.
 *
 * Note that that function is blocking.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI void
ecore_x_randr_screen_rotation_set(Ecore_X_Window         root,
                                  Ecore_X_Randr_Rotation rot)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_set_screen_config_cookie_t cookie;
   xcb_randr_set_screen_config_reply_t *reply_config;
   xcb_randr_get_screen_info_reply_t   *reply;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return;

   cookie = xcb_randr_set_screen_config_unchecked(_ecore_xcb_conn, root,
                                                  XCB_CURRENT_TIME,
                                                  reply->config_timestamp,
                                                  reply->sizeID,
                                                  rot,
                                                  0);
   reply_config = xcb_randr_set_screen_config_reply(_ecore_xcb_conn, cookie, NULL);
   if (reply_config)
     free(reply_config);
#endif /* ECORE_XCB_RANDR */
}

/* FIXME: round trip. Should we remove it ? */

/**
 * Set the screen size.
 * @param root The root window.
 * @param size The size.
 *
 * Set the size of the screen associated to @p root.
 *
 * Note that that function is blocking.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI int
ecore_x_randr_screen_size_set(Ecore_X_Window      root,
                              Ecore_X_Screen_Size size)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_set_screen_config_cookie_t cookie;
   xcb_randr_set_screen_config_reply_t *reply_config;
   xcb_randr_get_screen_info_reply_t   *reply;
   xcb_randr_screen_size_iterator_t     iter;
   int                                  size_index = -1;
   int                                  i;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return 0;

   iter = xcb_randr_get_screen_info_sizes_iterator(reply);
   for (i = 0; iter.rem; xcb_randr_screen_size_next(&iter), i++)
     {
        if ((iter.data->width = size.width) &&
            (iter.data->height = size.height) &&
            (iter.data->mwidth = size.width) &&
            (iter.data->mheight = size.height))
          {
            size_index = i;
            break;
          }
     }
   if (size_index == -1) return 0;

   cookie = xcb_randr_set_screen_config_unchecked(_ecore_xcb_conn, root,
                                                  XCB_CURRENT_TIME,
                                                  reply->config_timestamp,
                                                  size_index,
                                                  XCB_RANDR_ROTATION_ROTATE_0,
                                                  0);
   reply_config = xcb_randr_set_screen_config_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply_config)
     return 0;

   free(reply_config);

   return 1;
#else
   return 0;
#endif /* ECORE_XCB_RANDR */
}

/* FIXME: round trip. Should we remove it ? */

/**
 * Set the screen refresh rate.
 * @param root The root window.
 * @param size The size.
 * @param rate The refresh rate.
 *
 * Set the size and the refresh rate of the screen associated to
 * @p root.
 *
 * Note that that function is blocking.
 * @ingroup Ecore_X_RandR_Group
 */
EAPI int
ecore_x_randr_screen_refresh_rate_set(Ecore_X_Window              root,
                                      Ecore_X_Screen_Size         size,
                                      Ecore_X_Screen_Refresh_Rate rate)
{
#ifdef ECORE_XCB_RANDR
   xcb_randr_set_screen_config_cookie_t cookie;
   xcb_randr_set_screen_config_reply_t *reply_config;
   xcb_randr_get_screen_info_reply_t   *reply;
   xcb_randr_screen_size_iterator_t     iter;
   int                                  size_index = -1;
   int                                  i;

   reply = _ecore_xcb_reply_get();
   if (!reply)
     return 0;

   iter = xcb_randr_get_screen_info_sizes_iterator(reply);
   for (i = 0; iter.rem; xcb_randr_screen_size_next(&iter), i++)
     {
        if ((iter.data->width = size.width) &&
            (iter.data->height = size.height) &&
            (iter.data->mwidth = size.width) &&
            (iter.data->mheight = size.height))
          {
            size_index = i;
            break;
          }
     }
   if (size_index == -1) return 0;

   cookie = xcb_randr_set_screen_config_unchecked(_ecore_xcb_conn, root,
                                                  XCB_CURRENT_TIME,
                                                  reply->config_timestamp,
                                                  size_index,
                                                  XCB_RANDR_ROTATION_ROTATE_0,
                                                  rate.rate);
   reply_config = xcb_randr_set_screen_config_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply_config)
     return 0;

   free(reply_config);

   return 1;
#else
   return 0;
#endif /* ECORE_XCB_RANDR */
}
