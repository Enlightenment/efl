#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_XINERAMA
# include <xcb/xinerama.h>
#endif

/* local variables */
static Eina_Bool _xinerama_avail = EINA_FALSE;
static Eina_Bool _xinerama_active = EINA_FALSE;

void 
_ecore_xcb_xinerama_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XINERAMA
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_xinerama_id);
#endif
}

void 
_ecore_xcb_xinerama_finalize(void) 
{
#ifdef ECORE_XCB_XINERAMA
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XINERAMA
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_xinerama_id);
   if ((ext_reply) && (ext_reply->present)) 
     {
        xcb_xinerama_query_version_cookie_t cookie;
        xcb_xinerama_query_version_reply_t *reply;

        cookie = 
          xcb_xinerama_query_version_unchecked(_ecore_xcb_conn, 
                                               XCB_XINERAMA_MAJOR_VERSION, 
                                               XCB_XINERAMA_MINOR_VERSION);
        reply = 
          xcb_xinerama_query_version_reply(_ecore_xcb_conn, cookie, NULL);
        if (reply) 
          {
             _xinerama_avail = EINA_TRUE;
             // NB: Do we need to compare version numbers here ?
             free(reply);
          }

        if (_xinerama_avail) 
          {
             xcb_xinerama_is_active_cookie_t acookie;
             xcb_xinerama_is_active_reply_t *areply;

             acookie = xcb_xinerama_is_active_unchecked(_ecore_xcb_conn);
             areply = 
               xcb_xinerama_is_active_reply(_ecore_xcb_conn, acookie, NULL);
             if (areply) 
               {
                  _xinerama_active = areply->state;
                  free(areply);
               }
          }
     }
#endif
}

EAPI int 
ecore_x_xinerama_screen_count_get(void) 
{
   int count = 0;
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_query_screens_cookie_t cookie;
   xcb_xinerama_query_screens_reply_t *reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!_xinerama_avail) return 0;

#ifdef ECORE_XCB_XINERAMA
   cookie = xcb_xinerama_query_screens_unchecked(_ecore_xcb_conn);
   reply = 
     xcb_xinerama_query_screens_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return 0;
   count = reply->number;
#endif

   return count;
}

EAPI Eina_Bool 
ecore_x_xinerama_screen_geometry_get(int screen, int *x, int *y, int *w, int *h) 
{
#ifdef ECORE_XCB_XINERAMA
   xcb_xinerama_query_screens_cookie_t cookie;
   xcb_xinerama_query_screens_reply_t *reply;
   xcb_xinerama_screen_info_t *info;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = ((xcb_screen_t *)_ecore_xcb_screen)->width_in_pixels;
   if (h) *h = ((xcb_screen_t *)_ecore_xcb_screen)->height_in_pixels;

   if (!_xinerama_avail) return EINA_FALSE;

#ifdef ECORE_XCB_XINERAMA
   cookie = xcb_xinerama_query_screens_unchecked(_ecore_xcb_conn);
   reply = 
     xcb_xinerama_query_screens_reply(_ecore_xcb_conn, cookie, NULL);
   if (!reply) return EINA_FALSE;

   info = xcb_xinerama_query_screens_screen_info(reply);
   if (!info) 
     {
        free(reply);
        return EINA_FALSE;
     }

   if (x) *x = info[screen].x_org;
   if (y) *y = info[screen].y_org;
   if (w) *w = info[screen].width;
   if (h) *h = info[screen].height;

   free(reply);
   return EINA_TRUE;
#endif

   return EINA_FALSE;
}
