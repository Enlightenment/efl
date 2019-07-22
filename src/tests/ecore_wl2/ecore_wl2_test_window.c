#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

#include "ecore_wl2_suite.h"

static Ecore_Wl2_Display *
_display_connect(void)
{
   Ecore_Wl2_Display *disp;

   disp = ecore_wl2_display_connect(NULL);
   return disp;
}

static Ecore_Wl2_Window *
_window_create(Ecore_Wl2_Display *disp)
{
   Ecore_Wl2_Window *win;

   win = ecore_wl2_window_new(disp, NULL, 100, 100, 500, 500);
   return win;
}

EFL_START_TEST(wl2_window_new)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_surface_test)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   struct wl_surface *surf;
   int id = -1;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   surf = ecore_wl2_window_surface_get(win);
   ck_assert(surf != NULL);

   id = ecore_wl2_window_surface_id_get(win);
   ck_assert_int_lt(id, 0);

   ck_assert_int_eq(wl_proxy_get_id((struct wl_proxy *)surf), id);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_rotation_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   int rot = -1;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   rot = ecore_wl2_window_rotation_get(win);
   ck_assert_int_lt(rot, 0);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_output_find)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Output *out;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   out = ecore_wl2_window_output_find(win);
   ck_assert(out != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_aux_hints_supported_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_List *l;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   l = ecore_wl2_window_aux_hints_supported_get(win);
   ck_assert(l != NULL);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_display_get)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ck_assert(ecore_wl2_window_display_get(win) != NULL);
}
EFL_END_TEST

void
ecore_wl2_test_window(TCase *tc)
{
   if (getenv("WAYLAND_DISPLAY"))
     {
        /* window tests can only run if there is an existing compositor */
        tcase_add_test(tc, wl2_window_new);
        tcase_add_test(tc, wl2_window_surface_test);
        tcase_add_test(tc, wl2_window_rotation_get);
        tcase_add_test(tc, wl2_window_output_find);
        tcase_add_test(tc, wl2_window_aux_hints_supported_get);
        tcase_add_test(tc, wl2_window_display_get);
     }
}
