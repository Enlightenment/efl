#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include <Elementary.h>
#include <Ecore_X.h>
#include "elm_suite.h"

static const double _timeout1 = 0.1;
static const double _timeout2 = 0.2;
static const double _timeout_fail = 2.0;

static void
_do_delete_request(Eo *win)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin;
   xwin = elm_win_xwindow_get(win);
   ecore_x_window_delete_request_send(xwin);
#endif

   (void) win;
}


static Eina_Bool
_timer_delete_request_cb(void *data)
{
   Eo *win = (Eo*) data;
   _do_delete_request(win);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_timer_hide_window_cb(void *data)
{
   Eo *win = (Eo*) data;
   efl_gfx_visible_set(win, EINA_FALSE);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_timer_exit_cb(void *data EINA_UNUSED)
{
   elm_exit();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_timer_fail_flag_cb(void *data)
{
   Eina_Bool *fail_flag = (Eina_Bool*) data;
   *fail_flag = EINA_TRUE;
   elm_exit();
   return ECORE_CALLBACK_PASS_ON;
}


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "win", ELM_WIN_BASIC);

   role = elm_interface_atspi_accessible_role_get(win);

   ck_assert(role == ELM_ATSPI_ROLE_WINDOW);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_position)
{
   Eina_Bool ret;
   int x, y;

   elm_init(0, NULL);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);

   ret = elm_interface_atspi_component_position_set(win, EINA_TRUE, 45, 45);
   ck_assert(ret == EINA_TRUE);

   Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));
   ck_assert(ee != NULL);
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);

   ck_assert((x == 45) && (y == 45));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_size)
{
   Eina_Bool ret;
   int w, h;

   elm_init(0, NULL);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   evas_object_resize(win, 50, 50);

   ret = elm_interface_atspi_component_size_set(win, 100, 100);
   ck_assert(ret == EINA_TRUE);

   evas_object_geometry_get(win, NULL, NULL, &w, &h);
   ck_assert((w == 100) && (h == 100));

   elm_shutdown();
}
END_TEST

START_TEST (elm_win_autohide)
{
   elm_init(0, NULL);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   elm_obj_win_autohide_set(win, EINA_TRUE);
   efl_gfx_visible_set(win, EINA_TRUE);

   Eina_Bool fail_flag = EINA_FALSE;
   ecore_timer_add(_timeout1, _timer_delete_request_cb, win);
   ecore_timer_add(_timeout2, _timer_exit_cb, &fail_flag);

   elm_run();

   Eina_Bool visible;
   visible = efl_gfx_visible_get(win);
   ck_assert(visible == EINA_FALSE);

   elm_shutdown();
}
END_TEST

START_TEST (elm_win_policy_quit_last_window_hidden)
{
   elm_init(0, NULL);

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   efl_gfx_visible_set(win, EINA_TRUE);

   Eina_Bool fail_flag = EINA_FALSE;
   ecore_timer_add(_timeout1, _timer_hide_window_cb, win);
   ecore_timer_add(_timeout_fail, _timer_fail_flag_cb, &fail_flag);

   elm_run();

   Eina_Bool visible;
   visible = efl_gfx_visible_get(win);

   ck_assert(fail_flag == EINA_FALSE);
   ck_assert(eo_ref_get(win) >= 1);
   ck_assert(visible == EINA_FALSE);

   elm_shutdown();
}
END_TEST

START_TEST (elm_win_autohide_and_policy_quit_last_window_hidden)
{
   elm_init(0, NULL);

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   elm_obj_win_autohide_set(win, EINA_TRUE);
   efl_gfx_visible_set(win, EINA_TRUE);

   Eina_Bool fail_flag = EINA_FALSE;
   ecore_timer_add(_timeout1, _timer_delete_request_cb, win);
   ecore_timer_add(_timeout_fail, _timer_fail_flag_cb, &fail_flag);

   elm_run();

   Eina_Bool visible;
   visible = efl_gfx_visible_get(win);

   ck_assert(fail_flag == EINA_FALSE);
   ck_assert(eo_ref_get(win) >= 1);
   ck_assert(visible == EINA_FALSE);

   elm_shutdown();
}
END_TEST

void elm_test_win(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_component_position);
   tcase_add_test(tc, elm_atspi_component_size);
   tcase_add_test(tc, elm_win_policy_quit_last_window_hidden);
#ifdef HAVE_ELEMENTARY_X
   tcase_add_test(tc, elm_win_autohide);
   tcase_add_test(tc, elm_win_autohide_and_policy_quit_last_window_hidden);
#endif
}
