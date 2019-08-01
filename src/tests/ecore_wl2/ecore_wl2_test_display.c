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

   disp = _display_setup();
   ck_assert(disp != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_destroy)
{
   Ecore_Wl2_Display *disp;

   disp = _display_setup();
   ck_assert(disp != NULL);

   ecore_wl2_display_destroy(disp);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_get)
{
   Ecore_Wl2_Display *disp;
   struct wl_display *wdisp;

   disp = _display_setup();
   ck_assert(disp != NULL);

   wdisp = ecore_wl2_display_get(disp);
   ck_assert(wdisp != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_name_get)
{
   Ecore_Wl2_Display *disp;

   disp = _display_setup();
   ck_assert(disp != NULL);

   ck_assert(ecore_wl2_display_name_get(disp) != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_connect)
{
   Ecore_Wl2_Display *disp;

   disp = _display_connect();
   ck_assert(disp != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_disconnect)
{
   Ecore_Wl2_Display *disp;

   disp = _display_connect();
   ck_assert(disp != NULL);

   ecore_wl2_display_disconnect(disp);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_registry_get)
{
   Ecore_Wl2_Display *disp;

   disp = _display_connect();
   ck_assert(disp != NULL);

   ck_assert(ecore_wl2_display_registry_get(disp) != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_shm_get)
{
   Ecore_Wl2_Display *disp;
   struct wl_shm *shm;

   disp = _display_connect();
   ck_assert(disp != NULL);

   shm = ecore_wl2_display_shm_get(disp);
   ck_assert(shm != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_dmabuf_get)
{
   Ecore_Wl2_Display *disp;
   void *dma;

   disp = _display_connect();
   ck_assert(disp != NULL);

   dma = ecore_wl2_display_dmabuf_get(disp);
   ck_assert(dma != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_globals_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Global *global;
   Eina_Iterator *itr;
   void *data;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_globals_get(disp);
   ck_assert(itr != NULL);

   EINA_ITERATOR_FOREACH(itr, data)
     {
        global = (Ecore_Wl2_Global *)data;
        printf("Interface: <%s>\n", global->interface);
     }

   eina_iterator_free(itr);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_screen_size_get)
{
   Ecore_Wl2_Display *disp;
   int w, h;

   disp = _display_connect();
   ck_assert(disp != NULL);

   ecore_wl2_display_screen_size_get(disp, &w, &h);
   ck_assert_int_ne(w, 0);
   ck_assert_int_ne(h, 0);
}
EFL_END_TEST

EFL_START_TEST(wl2_display_inputs_get)
{
   Ecore_Wl2_Display *disp;
   Eina_Iterator *itr;

   disp = _display_connect();
   ck_assert(disp != NULL);

   itr = ecore_wl2_display_inputs_get(disp);
   ck_assert(itr != NULL);
}
EFL_END_TEST

void
ecore_wl2_test_display(TCase *tc)
{
   if ((!getenv("WAYLAND_DISPLAY")) && (getenv("XDG_RUNTIME_DIR")))
     {
        /* tests here are for server-side functions */
        printf("No Wayland Compositor detected. Testing server-side functions\n");
        tcase_add_test(tc, wl2_display_create);
        tcase_add_test(tc, wl2_display_destroy);
        tcase_add_test(tc, wl2_display_get);
        tcase_add_test(tc, wl2_display_name_get);
     }
   else if (getenv("WAYLAND_DISPLAY"))
     {
        /* tests here are for client-side functions */
        printf("Wayland Compositor detected. Testing client-side functions\n");
        tcase_add_test(tc, wl2_display_connect);
        tcase_add_test(tc, wl2_display_disconnect);
        tcase_add_test(tc, wl2_display_registry_get);
        tcase_add_test(tc, wl2_display_shm_get);
        tcase_add_test(tc, wl2_display_dmabuf_get);
        tcase_add_test(tc, wl2_display_globals_get);
        tcase_add_test(tc, wl2_display_screen_size_get);
        tcase_add_test(tc, wl2_display_inputs_get);
     }
}
