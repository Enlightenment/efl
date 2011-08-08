#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_SHAPE
# include <xcb/shape.h>
#endif

/* external variables */
int _ecore_xcb_event_shape = -1;

void 
_ecore_xcb_shape_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SHAPE
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_shape_id);
#endif
}

void 
_ecore_xcb_shape_finalize(void) 
{
#ifdef ECORE_XCB_SHAPE
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_SHAPE
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_shape_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_shape_query_version_cookie_t cookie;
        xcb_shape_query_version_reply_t *reply;
        Eina_Bool _shape_avail = EINA_FALSE;

        cookie = xcb_shape_query_version_unchecked(_ecore_xcb_conn);
        reply = xcb_shape_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
             _shape_avail = EINA_TRUE;
             free(reply);
          }

        if (_shape_avail)
          _ecore_xcb_event_shape = ext_reply->first_event;
     }
#endif
}
