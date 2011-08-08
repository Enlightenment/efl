#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_COMPOSITE
# include <xcb/composite.h>
#endif

/* local variables */
static Eina_Bool _composite_avail = EINA_FALSE;

void 
_ecore_xcb_composite_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_composite_id);
#endif
}

void 
_ecore_xcb_composite_finalize(void) 
{
#ifdef ECORE_XCB_COMPOSITE
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_composite_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_composite_query_version_cookie_t cookie;
        xcb_composite_query_version_reply_t *reply;

        cookie = 
          xcb_composite_query_version_unchecked(_ecore_xcb_conn, 
                                                XCB_COMPOSITE_MAJOR_VERSION, 
                                                XCB_COMPOSITE_MINOR_VERSION);
        reply = 
          xcb_composite_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
//             if ((reply->major_version >= XCB_COMPOSITE_MAJOR_VERSION) && 
             if (reply->minor_version >= XCB_COMPOSITE_MINOR_VERSION) 
               {
# ifdef ECORE_XCB_RENDER
                  if (_ecore_xcb_render_avail_get()) 
                    {
#  ifdef ECORE_XCB_XFIXES
                       if (_ecore_xcb_xfixes_avail_get())
                         _composite_avail = EINA_TRUE;
#  endif
                    }
# endif
               }

             free(reply);
          }
     }
#endif
}

/**
 * @defgroup Ecore_X_Composite_Group X Composite Extension Functions
 * 
 * Functions related to the X Composite Extension
 */

/**
 * Return whether the Composite Extension is available
 * 
 * @return EINA_TRUE is the Composite Extension is available, EINA_FALSE if not
 * 
 * @ingroup Ecore_X_Composite_Group
 */
EAPI Eina_Bool 
ecore_x_composite_query(void) 
{
   return _composite_avail;
}

EAPI void 
ecore_x_composite_redirect_window(Ecore_X_Window win, Ecore_X_Composite_Update_Type type) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   uint8_t update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;

   switch (type) 
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
        update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;
        break;
      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
        update = XCB_COMPOSITE_REDIRECT_MANUAL;
        break;
     }
   xcb_composite_redirect_window(_ecore_xcb_conn, win, update);
#endif
}

EAPI void 
ecore_x_composite_redirect_subwindows(Ecore_X_Window win, Ecore_X_Composite_Update_Type type) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   uint8_t update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;

   switch (type) 
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
        update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;
        break;
      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
        update = XCB_COMPOSITE_REDIRECT_MANUAL;
        break;
     }
   xcb_composite_redirect_subwindows(_ecore_xcb_conn, win, update);
#endif
}

EAPI void 
ecore_x_composite_unredirect_window(Ecore_X_Window win, Ecore_X_Composite_Update_Type type) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   uint8_t update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;

   switch (type) 
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
        update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;
        break;
      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
        update = XCB_COMPOSITE_REDIRECT_MANUAL;
        break;
     }
   xcb_composite_unredirect_window(_ecore_xcb_conn, win, update);
#endif
}

EAPI void 
ecore_x_composite_unredirect_subwindows(Ecore_X_Window win, Ecore_X_Composite_Update_Type type) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   uint8_t update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;

   switch (type) 
     {
      case ECORE_X_COMPOSITE_UPDATE_AUTOMATIC:
        update = XCB_COMPOSITE_REDIRECT_AUTOMATIC;
        break;
      case ECORE_X_COMPOSITE_UPDATE_MANUAL:
        update = XCB_COMPOSITE_REDIRECT_MANUAL;
        break;
     }
   xcb_composite_unredirect_subwindows(_ecore_xcb_conn, win, update);
#endif
}

EAPI Ecore_X_Pixmap 
ecore_x_composite_name_window_pixmap_get(Ecore_X_Window win) 
{
   Ecore_X_Pixmap pmap = XCB_NONE;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   pmap = xcb_generate_id(_ecore_xcb_conn);
   xcb_composite_name_window_pixmap(_ecore_xcb_conn, win, pmap);
#endif

   return pmap;
}

EAPI void 
ecore_x_composite_window_events_disable(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SHAPE
   ecore_x_window_shape_input_rectangle_set(win, -1, -1, 1, 1);
#else
   return;
   win = 0;
#endif
}

EAPI void 
ecore_x_composite_window_events_enable(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SHAPE
   ecore_x_window_shape_input_rectangle_set(win, 0, 0, 65535, 65535);
#else
   return;
   win = 0;
#endif
}

EAPI Ecore_X_Window 
ecore_x_composite_render_window_enable(Ecore_X_Window root) 
{
   Ecore_X_Window win = 0;
#ifdef ECORE_XCB_COMPOSITE
   xcb_composite_get_overlay_window_cookie_t cookie;
   xcb_composite_get_overlay_window_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   cookie = xcb_composite_get_overlay_window_unchecked(_ecore_xcb_conn, root);
   reply = 
     xcb_composite_get_overlay_window_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return win;

   win = reply->overlay_win;
   free(reply);

   ecore_x_composite_window_events_disable(win);
#endif

   return win;
}

EAPI void 
ecore_x_composite_render_window_disable(Ecore_X_Window win) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_COMPOSITE
   xcb_composite_release_overlay_window(_ecore_xcb_conn, win);
#endif
}
