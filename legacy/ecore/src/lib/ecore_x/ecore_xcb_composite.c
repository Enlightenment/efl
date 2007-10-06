/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_xcb_private.h"

/**
 * @defgroup Ecore_X_Composite_Group X Composite Extension Functions
 *
 * Functions related to the X Composite extension.
 */

#ifdef ECORE_XCB_COMPOSITE
static uint8_t _composite_available = 0;
static xcb_composite_query_version_cookie_t _ecore_xcb_composite_init_cookie;
#endif /* ECORE_XCB_COMPOSITE */


/* To avoid round trips, the initialization is separated in 2
   functions: _ecore_xcb_composite_init and
   _ecore_xcb_composite_init_finalize. The first one gets the cookies and
   the second one gets the replies. */

void
_ecore_x_composite_init(const xcb_query_extension_reply_t *reply)
{
#ifdef ECORE_XCB_COMPOSITE
   if (reply && reply->present)
      _ecore_xcb_composite_init_cookie = xcb_composite_query_version_unchecked(_ecore_xcb_conn, XCB_COMPOSITE_MAJOR_VERSION, XCB_COMPOSITE_MINOR_VERSION);
#endif /* ECORE_XCB_COMPOSITE */
}

void
_ecore_x_composite_init_finalize(void)
{
#ifdef ECORE_XCB_COMPOSITE
   xcb_composite_query_version_reply_t *reply;

   reply = xcb_composite_query_version_reply(_ecore_xcb_conn,
                                          _ecore_xcb_composite_init_cookie,
                                          NULL);
   if (reply)
     {
        if (reply->major_version = XCB_COMPOSITE_MAJOR_VERSION &&
	    reply->minor_version >= XCB_COMPOSITE_MINOR_VERSION)
          _composite_available = 1;
        free(reply);
     }
#endif /* ECORE_XCB_COMPOSITE */
}

/**
 * Return whether the Composite Extension is available.
 * @return 1 if the Composite Extension is available, 0 if not.
 *
 * Return 1 if the X server supports the Composite Extension version 0.4
 * or greater, 0 otherwise.
 * @ingroup Ecore_X_Composite_Group
 */
EAPI int
ecore_x_composite_query(void)
{
#ifdef ECORE_XCB_COMPOSITE
   return _composite_available;
#else
   return 0;
#endif /* ECORE_XCB_COMPOSITE */
}
