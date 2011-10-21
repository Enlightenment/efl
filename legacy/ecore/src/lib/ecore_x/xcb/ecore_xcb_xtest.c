#include "ecore_xcb_private.h"
#ifdef ECORE_XCB_XTEST
# include <xcb/xtest.h>
# include <X11/keysym.h>
#endif

/* local variables */
static Eina_Bool _test_avail = EINA_FALSE;

void
_ecore_xcb_xtest_init(void)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XTEST
   xcb_prefetch_extension_data(_ecore_xcb_conn, &xcb_test_id);
#endif
}

void
_ecore_xcb_xtest_finalize(void)
{
#ifdef ECORE_XCB_XTEST
   const xcb_query_extension_reply_t *ext_reply;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

#ifdef ECORE_XCB_XTEST
   ext_reply = xcb_get_extension_data(_ecore_xcb_conn, &xcb_test_id);
   if ((ext_reply) && (ext_reply->present))
     _test_avail = EINA_TRUE;
#endif
}

EAPI Eina_Bool
#ifdef ECORE_XCB_XTEST
ecore_x_test_fake_key_down(const char *key)
#else
ecore_x_test_fake_key_down(const char *key __UNUSED__)
#endif
{
#ifdef ECORE_XCB_XTEST
   xcb_keycode_t keycode = 0;
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_test_avail) return EINA_FALSE;

#ifdef ECORE_XCB_XTEST
   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   if (keycode == XCB_NO_SYMBOL) return EINA_FALSE;

   cookie =
     xcb_test_fake_input(_ecore_xcb_conn, XCB_KEY_PRESS,
                         keycode, XCB_CURRENT_TIME,
                         ((xcb_screen_t *)_ecore_xcb_screen)->root, 0, 0, 0);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err)
     {
        free(err);
        return EINA_FALSE;
     }
   return EINA_TRUE;
#endif

   return EINA_FALSE;
}

EAPI Eina_Bool
#ifdef ECORE_XCB_XTEST
ecore_x_test_fake_key_up(const char *key)
#else
ecore_x_test_fake_key_up(const char *key __UNUSED__)
#endif
{
#ifdef ECORE_XCB_XTEST
   xcb_keycode_t keycode = 0;
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_test_avail) return EINA_FALSE;

#ifdef ECORE_XCB_XTEST
   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   if (keycode == XCB_NO_SYMBOL) return EINA_FALSE;

   cookie =
     xcb_test_fake_input(_ecore_xcb_conn, XCB_KEY_RELEASE,
                         keycode, XCB_CURRENT_TIME,
                         ((xcb_screen_t *)_ecore_xcb_screen)->root, 0, 0, 0);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err)
     {
        free(err);
        return EINA_FALSE;
     }
   return EINA_TRUE;
#endif

   return EINA_FALSE;
}

EAPI Eina_Bool
#ifdef ECORE_XCB_XTEST
ecore_x_test_fake_key_press(const char *key)
#else
ecore_x_test_fake_key_press(const char *key __UNUSED__)
#endif
{
#ifdef ECORE_XCB_XTEST
   xcb_keycode_t keycode = 0;
   xcb_keysym_t keysym = 0;
   xcb_keycode_t shift_code = 0;
   xcb_void_cookie_t cookie;
   xcb_generic_error_t *err;
   Eina_Bool shift = EINA_FALSE;
#endif

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   CHECK_XCB_CONN;

   if (!_test_avail) return EINA_FALSE;

#ifdef ECORE_XCB_XTEST
   keycode = _ecore_xcb_keymap_string_to_keycode(key);
   keysym = _ecore_xcb_keymap_keycode_to_keysym(keycode, 0);
   if (keysym == XCB_NO_SYMBOL)
     {
        keysym = _ecore_xcb_keymap_keycode_to_keysym(keycode, 1);
        if (keysym != XCB_NO_SYMBOL)
          shift = EINA_TRUE;
     }

   if (shift)
     {
        xcb_keycode_t *keycodes;
        int i = 0;

        keycodes = _ecore_xcb_keymap_keysym_to_keycode(XK_Shift_L);
        while (keycodes[i] != XCB_NO_SYMBOL)
          {
             if (keycodes[i] != 0)
               {
                  shift_code = keycodes[i];
                  break;
               }
             i++;
          }
     }

   if (shift)
     {
        cookie =
          xcb_test_fake_input(_ecore_xcb_conn, XCB_KEY_PRESS,
                              shift_code, XCB_CURRENT_TIME,
                              ((xcb_screen_t *)_ecore_xcb_screen)->root,
                              0, 0, 0);
        err = xcb_request_check(_ecore_xcb_conn, cookie);
        if (err)
          {
             free(err);
             return EINA_FALSE;
          }
     }

   cookie =
     xcb_test_fake_input(_ecore_xcb_conn, XCB_KEY_PRESS,
                         keycode, XCB_CURRENT_TIME,
                         ((xcb_screen_t *)_ecore_xcb_screen)->root, 0, 0, 0);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err)
     {
        free(err);
        return EINA_FALSE;
     }
   cookie =
     xcb_test_fake_input(_ecore_xcb_conn, XCB_KEY_RELEASE,
                         keycode, XCB_CURRENT_TIME,
                         ((xcb_screen_t *)_ecore_xcb_screen)->root, 0, 0, 0);
   err = xcb_request_check(_ecore_xcb_conn, cookie);
   if (err)
     {
        free(err);
        return EINA_FALSE;
     }

   if (shift)
     {
        cookie =
          xcb_test_fake_input(_ecore_xcb_conn, XCB_KEY_RELEASE,
                              shift_code, XCB_CURRENT_TIME,
                              ((xcb_screen_t *)_ecore_xcb_screen)->root,
                              0, 0, 0);
        err = xcb_request_check(_ecore_xcb_conn, cookie);
        if (err)
          {
             free(err);
             return EINA_FALSE;
          }
     }

   return EINA_TRUE;
#endif

   return EINA_FALSE;
}
