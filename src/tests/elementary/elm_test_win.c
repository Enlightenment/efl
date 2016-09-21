#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include <Elementary.h>
#ifdef HAVE_ELEMENTARY_X
# include <Ecore_X.h>
#endif
#include "elm_suite.h"

static const double _timeout1 = 0.1;
static const double _timeout2 = 0.2;
static const double _timeout3 = 0.3;
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
   elm_win_autohide_set(win, EINA_TRUE);
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
   ck_assert(efl_ref_get(win) >= 1);
   ck_assert(visible == EINA_FALSE);

   elm_shutdown();
}
END_TEST

START_TEST (elm_win_autohide_and_policy_quit_last_window_hidden)
{
   elm_init(0, NULL);

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   elm_win_autohide_set(win, EINA_TRUE);
   efl_gfx_visible_set(win, EINA_TRUE);

   Eina_Bool fail_flag = EINA_FALSE;
   ecore_timer_add(_timeout1, _timer_delete_request_cb, win);
   ecore_timer_add(_timeout_fail, _timer_fail_flag_cb, &fail_flag);

   elm_run();

   Eina_Bool visible;
   visible = efl_gfx_visible_get(win);

   ck_assert(fail_flag == EINA_FALSE);
   ck_assert(efl_ref_get(win) >= 1);
   ck_assert(visible == EINA_FALSE);

   elm_shutdown();
}
END_TEST

/* a very lax definition of == for doubles */
#define VALEQ(a, b) ((fabs((a) - (b))) <= 0.001)

typedef struct
{
   double x, y;
} point_t;

static point_t points[2][4] =
{
   {
      { 20, 20 },
      { 40, 10 },
      { 60, 120 },
      { 80, 80 }
   },
   {
      { 30, 30 },
      { 50, 50 },
      { 70, 60 },
      { 80, 80 }
   },
};

static Eina_Bool
_inputs_timer1_cb(void *data)
{
   Efl_Input_Pointer *ptr;
   Eo *win = data;

   /* Send down events first (note: stupid, all at once) */
   for (size_t i = 0; i < 4; i++)
     {
        ptr = efl_add(EFL_INPUT_POINTER_CLASS, win);
        efl_input_pointer_position_set(ptr, points[0][i].x, points[0][i].y);
        efl_input_pointer_tool_set(ptr, i);
        efl_input_pointer_button_set(ptr, 1);

        if (i == 0)
          {
             /* in first */
             efl_input_pointer_action_set(ptr, EFL_POINTER_ACTION_IN);
             efl_event_callback_call(win, EFL_EVENT_POINTER_IN, ptr);

             /* move second */
             efl_input_pointer_position_set(ptr, points[0][i].x, points[0][i].y);
             efl_input_pointer_action_set(ptr, EFL_POINTER_ACTION_MOVE);
             efl_event_callback_call(win, EFL_EVENT_POINTER_MOVE, ptr);
          }

        /* down finally */
        efl_input_pointer_action_set(ptr, EFL_POINTER_ACTION_DOWN);
        efl_event_callback_call(win, EFL_EVENT_POINTER_DOWN, ptr);
     }

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_inputs_timer2_cb(void *data)
{
   Efl_Input_Pointer *ptr;
   Eina_Iterator *it;
   Eo *win = data;
   size_t cnt = 0;
   int i = 0;

   it = efl_input_pointer_iterate(win, 0);
   EINA_ITERATOR_FOREACH(it, ptr)
     {
        double x, y;
        int tool, ok = 0;

        fail_if(!efl_input_pointer_value_has_get(ptr, EFL_INPUT_VALUE_X));
        fail_if(!efl_input_pointer_value_has_get(ptr, EFL_INPUT_VALUE_Y));
        fail_if(!efl_input_pointer_value_has_get(ptr, EFL_INPUT_VALUE_TOOL));
        fail_if(efl_input_pointer_action_get(ptr) != EFL_POINTER_ACTION_DOWN);

        x = efl_input_pointer_value_get(ptr, EFL_INPUT_VALUE_X);
        y = efl_input_pointer_value_get(ptr, EFL_INPUT_VALUE_Y);
        tool = efl_input_pointer_tool_get(ptr);

        for (i = 0; i < 4; i++)
          if (tool == i)
            {
               fail_if(!VALEQ(x, points[0][i].x));
               fail_if(!VALEQ(y, points[0][i].y));
               ok = 1;
               break;
            }
        fail_if(!ok);

        cnt++;
     }
   eina_iterator_free(it);
   fail_if(cnt != 4);

   /* Send some moves */
   for (i = 0; i < 4; i++)
     {
        ptr = efl_add(EFL_INPUT_POINTER_CLASS, win);
        efl_input_pointer_position_set(ptr, points[1][i].x, points[1][i].y);
        efl_input_pointer_tool_set(ptr, i);
        efl_input_pointer_button_set(ptr, 1);

        /* move first */
        efl_input_pointer_action_set(ptr, EFL_POINTER_ACTION_MOVE);
        efl_event_callback_call(win, EFL_EVENT_POINTER_MOVE, ptr);

        /* then up (one 2 fingers up: #1 and #3) */
        if ((i % 2) == 1)
          {
             efl_input_pointer_action_set(ptr, EFL_POINTER_ACTION_UP);
             efl_event_callback_call(win, EFL_EVENT_POINTER_UP, ptr);
          }
     }

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_inputs_timer3_cb(void *data)
{
   Efl_Input_Pointer *ptr;
   Eina_Iterator *it;
   Eo *win = data;
   size_t cnt = 0;
   int i = 0;

   it = efl_input_pointer_iterate(win, 0);
   EINA_ITERATOR_FOREACH(it, ptr)
     {
        int tool, ok = 0;
        double x, y;

        fail_if(!efl_input_pointer_value_has_get(ptr, EFL_INPUT_VALUE_X));
        fail_if(!efl_input_pointer_value_has_get(ptr, EFL_INPUT_VALUE_Y));
        fail_if(!efl_input_pointer_value_has_get(ptr, EFL_INPUT_VALUE_TOOL));
        fail_if(efl_input_pointer_action_get(ptr) != EFL_POINTER_ACTION_MOVE);

        x = efl_input_pointer_value_get(ptr, EFL_INPUT_VALUE_X);
        y = efl_input_pointer_value_get(ptr, EFL_INPUT_VALUE_Y);
        tool = efl_input_pointer_tool_get(ptr);

        for (i = 0; i < 4; i++)
          if (tool == i)
            {
               fail_if(!VALEQ(x, points[1][i].x));
               fail_if(!VALEQ(y, points[1][i].y));
               ok = 1;
               break;
            }
        fail_if(!ok);

        cnt++;
     }
   eina_iterator_free(it);
   fail_if(cnt != 2); // 2 moves (in the list), 2 ups (gone)

   elm_exit();

   return ECORE_CALLBACK_DONE;
}

START_TEST (efl_ui_win_multi_touch_inputs)
{
   Eina_Bool fail_flag = EINA_FALSE;
   Eo *win;

   /* this tests only multi touch inputs - a lot of code but quite basic
    * faking events is quite hard, as an exact sequence must be followed:
    *
    * pointer 0: in, move, down, move, move, ...
    * pointer x: down, move, move, ...
    */

   elm_init(0, NULL);

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   elm_win_autohide_set(win, EINA_TRUE);
   efl_gfx_visible_set(win, EINA_TRUE);
   efl_gfx_size_set(win, 100, 100);

   ecore_timer_add(_timeout1, _inputs_timer1_cb, win);
   ecore_timer_add(_timeout2, _inputs_timer2_cb, win);
   ecore_timer_add(_timeout3, _inputs_timer3_cb, win);
   ecore_timer_add(_timeout_fail, _timer_fail_flag_cb, &fail_flag);

   elm_run();

   fail_if(fail_flag != EINA_FALSE);

   elm_shutdown();
}
END_TEST

void elm_test_win(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_component_position);
   tcase_add_test(tc, elm_atspi_component_size);
   tcase_add_test(tc, elm_win_policy_quit_last_window_hidden);
   tcase_add_test(tc, efl_ui_win_multi_touch_inputs);
#ifdef HAVE_ELEMENTARY_X
   tcase_add_test(tc, elm_win_autohide);
   tcase_add_test(tc, elm_win_autohide_and_policy_quit_last_window_hidden);
#endif
}
