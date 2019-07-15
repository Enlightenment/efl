#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

#include "ecore_wl2_suite.h"
#include "ecore_wl2_tests_helpers.h"

EFL_START_TEST(wl2_display_create)
{
   Ecore_Wl2_Display *disp;

   disp = ECORE_WL2_TEST_DISPLAY_SETUP();
   ck_assert(disp != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_destroy)
{
   Ecore_Wl2_Display *disp;

   disp = ECORE_WL2_TEST_DISPLAY_SETUP();
   ck_assert(disp != NULL);

   ecore_wl2_display_destroy(disp);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_get)
{
   Ecore_Wl2_Display *disp;
   struct wl_display *wdisp;

   disp = ECORE_WL2_TEST_DISPLAY_SETUP();
   ck_assert(disp != NULL);

   wdisp = ecore_wl2_display_get(disp);
   ck_assert(wdisp != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_name_get)
{
   Ecore_Wl2_Display *disp;

   disp = ECORE_WL2_TEST_DISPLAY_SETUP();
   ck_assert(disp != NULL);

   ck_assert(ecore_wl2_display_name_get(disp) != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_connect)
{
   Ecore_Wl2_Display *disp;

   disp = ECORE_WL2_TEST_DISPLAY_CONNECT();
   ck_assert(disp != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_disconnect)
{
   Ecore_Wl2_Display *disp;

   disp = ECORE_WL2_TEST_DISPLAY_CONNECT();
   ck_assert(disp != NULL);

   ecore_wl2_display_disconnect(disp);
}
EFL_END_TEST

void
ecore_wl2_test_display(TCase *tc)
{
   if (!getenv("WAYLAND_DISPLAY"))
     {
        /* tests here are for server-side functions */
        tcase_add_test(tc, wl2_display_create);
        tcase_add_test(tc, wl2_display_destroy);
        tcase_add_test(tc, wl2_display_get);
        tcase_add_test(tc, wl2_display_name_get);
     }

   if (getenv("WAYLAND_DISPLAY"))
     {
        /* tests here are for client-side functions */
        tcase_add_test(tc, wl2_display_connect);
        tcase_add_test(tc, wl2_display_disconnect);
     }
}
