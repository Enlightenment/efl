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
   ck_assert_int_gt(id, 0);

   ck_assert_int_eq(wl_proxy_get_id((struct wl_proxy *)surf), id);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_rotation)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   int rot = -1;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   rot = ecore_wl2_window_rotation_get(win);
   ck_assert_int_ge(rot, 0);

   ecore_wl2_window_rotation_set(win, 90);

   rot = ecore_wl2_window_rotation_get(win);
   fail_if(rot != 90);
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

EFL_START_TEST(wl2_window_alpha)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool alpha = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_alpha_set(win, EINA_TRUE);

   alpha = ecore_wl2_window_alpha_get(win);
   fail_if(alpha != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_floating_mode)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool f = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_floating_mode_set(win, EINA_TRUE);

   f = ecore_wl2_window_floating_mode_get(win);
   fail_if(f != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_focus_skip)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool skip = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_focus_skip_set(win, EINA_TRUE);

   skip = ecore_wl2_window_focus_skip_get(win);
   fail_if(skip != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_fullscreen)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool full = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_fullscreen_set(win, EINA_TRUE);

   full = ecore_wl2_window_fullscreen_get(win);
   fail_if(full != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_maximize)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool m = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_maximized_set(win, EINA_TRUE);

   m = ecore_wl2_window_maximized_get(win);
   fail_if(m != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_preferred_rot)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   int rot = 0;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_preferred_rotation_set(win, 90);

   rot = ecore_wl2_window_preferred_rotation_get(win);
   fail_if(rot != 90);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_rotation_app)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool r = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_rotation_app_set(win, EINA_TRUE);

   r = ecore_wl2_window_rotation_app_get(win);
   fail_if(r != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_wm_window_rotation_app)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool r = EINA_FALSE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_wm_rotation_supported_set(win, EINA_TRUE);

   r = ecore_wl2_window_wm_rotation_supported_get(win);
   fail_if(r != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_geometry)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   int x, y, w, h;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_geometry_set(win, 10, 10, 100, 100);

   ecore_wl2_window_geometry_get(win, &x, &y, &w, &h);

   fail_if(x != 10);
   fail_if(y != 10);
   fail_if(w != 100);
   fail_if(h != 100);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_type)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Window_Type type = ECORE_WL2_WINDOW_TYPE_NONE;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_type_set(win, ECORE_WL2_WINDOW_TYPE_TOPLEVEL);

   type = ecore_wl2_window_type_get(win);
   fail_if(type != ECORE_WL2_WINDOW_TYPE_TOPLEVEL);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_activated)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool ret;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ret = ecore_wl2_window_activated_get(win);

   fail_if(ret != EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_aspect)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   int w, h;
   unsigned int aspect;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_aspect_set(win, 1, 1, 3);
   ecore_wl2_window_aspect_get(win, &w, &h, &aspect);

   fail_if(w != 1);
   fail_if(h != 1);
   fail_if(aspect != 3);
}
EFL_END_TEST

EFL_START_TEST(wl2_window_class)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   const char *class;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_class_set(win, "TEST");
   class = ecore_wl2_window_class_get(win);

   fail_if(strcmp(class, "TEST"));
}
EFL_END_TEST

EFL_START_TEST(wl2_window_available_rotation)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Eina_Bool ret;
   int rots[2] = { 90, 180 };
   int *ret_rots;
   unsigned int ret_count;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   ecore_wl2_window_available_rotations_set(win, rots, 2);

   ret = ecore_wl2_window_available_rotations_get(win, &ret_rots, &ret_count);

   fail_if(ret != EINA_TRUE);
   fail_if(ret_rots[0] != 90);
   fail_if(ret_rots[1] != 180);
   fail_if(ret_count != 2);
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
        tcase_add_test(tc, wl2_window_rotation);
        tcase_add_test(tc, wl2_window_output_find);
        if (getenv("E_START"))
          tcase_add_test(tc, wl2_window_aux_hints_supported_get);
        tcase_add_test(tc, wl2_window_display_get);
        tcase_add_test(tc, wl2_window_alpha);
        tcase_add_test(tc, wl2_window_floating_mode);
        tcase_add_test(tc, wl2_window_focus_skip);
        tcase_add_test(tc, wl2_window_fullscreen);
        tcase_add_test(tc, wl2_window_maximize);
        tcase_add_test(tc, wl2_window_preferred_rot);
        tcase_add_test(tc, wl2_window_rotation_app);
        tcase_add_test(tc, wl2_wm_window_rotation_app);
        tcase_add_test(tc, wl2_window_geometry);
        tcase_add_test(tc, wl2_window_type);
        tcase_add_test(tc, wl2_window_activated);
        tcase_add_test(tc, wl2_window_available_rotation);
        tcase_add_test(tc, wl2_window_aspect);
        tcase_add_test(tc, wl2_window_class);
     }
}
