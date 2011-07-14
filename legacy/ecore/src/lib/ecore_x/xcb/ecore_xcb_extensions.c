#include "ecore_xcb_private.h"

void 
_ecore_xcb_extensions_init(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_big_requests_id);
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_shm_id);

#ifdef ECORE_XCB_SHAPE
   _ecore_xcb_shape_init();
#endif

#ifdef ECORE_XCB_SCREENSAVER
   _ecore_xcb_screensaver_init();
#endif

#ifdef ECORE_XCB_SYNC
   _ecore_xcb_sync_init();
#endif

#ifdef ECORE_XCB_RANDR
   _ecore_xcb_randr_init();
#endif

#ifdef ECORE_XCB_XFIXES
   _ecore_xcb_xfixes_init();
#endif

#ifdef ECORE_XCB_DAMAGE
   _ecore_xcb_damage_init();
#endif

#ifdef ECORE_XCB_RENDER
   _ecore_xcb_render_init();
#endif

#ifdef ECORE_XCB_COMPOSITE
   _ecore_xcb_composite_init();
#endif

#ifdef ECORE_XCB_DPMS
   _ecore_xcb_dpms_init();
#endif

#ifdef ECORE_XCB_DPMS
   _ecore_xcb_dpms_init();
#endif

#ifdef ECORE_XCB_CURSOR
   _ecore_xcb_cursor_init();
#endif

#ifdef ECORE_XCB_XINERAMA
   _ecore_xcb_xinerama_init();
#endif

#ifdef ECORE_XCB_XINPUT
   _ecore_xcb_input_init();
#endif

#ifdef ECORE_XCB_DRI
   _ecore_xcb_dri_init();
#endif

#ifdef ECORE_XCB_XTEST
   _ecore_xcb_xtest_init();
#endif

   xcb_prefetch_maximum_request_length(_ecore_xcb_conn);
}

void 
_ecore_xcb_extensions_finalize(void) 
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   xcb_get_extension_data(_ecore_xcb_conn, &xcb_big_requests_id);
   xcb_get_extension_data(_ecore_xcb_conn, &xcb_shm_id);

#ifdef ECORE_XCB_SHAPE
   _ecore_xcb_shape_finalize();
#endif

#ifdef ECORE_XCB_SCREENSAVER
   _ecore_xcb_screensaver_finalize();
#endif

#ifdef ECORE_XCB_SYNC
   _ecore_xcb_sync_finalize();
#endif

#ifdef ECORE_XCB_RANDR
   _ecore_xcb_randr_finalize();
#endif

#ifdef ECORE_XCB_XFIXES
   _ecore_xcb_xfixes_finalize();
#endif

#ifdef ECORE_XCB_DAMAGE
   _ecore_xcb_damage_finalize();
#endif

#ifdef ECORE_XCB_RENDER
   _ecore_xcb_render_finalize();
#endif

#ifdef ECORE_XCB_COMPOSITE
   _ecore_xcb_composite_finalize();
#endif

#ifdef ECORE_XCB_DPMS
   _ecore_xcb_dpms_finalize();
#endif

#ifdef ECORE_XCB_CURSOR
   _ecore_xcb_cursor_finalize();
#endif

#ifdef ECORE_XCB_XINERAMA
   _ecore_xcb_xinerama_finalize();
#endif

#ifdef ECORE_XCB_XINPUT
   _ecore_xcb_input_finalize();
#endif

#ifdef ECORE_XCB_DRI
   _ecore_xcb_dri_finalize();
#endif

#ifdef ECORE_XCB_XTEST
   _ecore_xcb_xtest_finalize();
#endif

   xcb_get_maximum_request_length(_ecore_xcb_conn);
}
