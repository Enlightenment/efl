#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "efl_ui_suite.h"

static void
_boolean_flag_set(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Bool *flag = data;
   *flag = EINA_TRUE;

   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
}

EFL_START_TEST(efl_ui_win_test_scene_focus)
{
   Efl_Ui_Win *win1;
   Ecore_Evas *ee;

   Eina_Bool win1_focus_in = EINA_FALSE;
   Eina_Bool win1_focus_out = EINA_FALSE;

   win1 = efl_new(EFL_UI_WIN_CLASS);

   //we want to test here the correct propagation of events from ecore_evas to the win object
   //in order to ensure that we are resetting ee focus first here.
   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win1));
   ecore_evas_focus_set(ee, EINA_TRUE);
   ecore_evas_focus_set(ee, EINA_FALSE);
   efl_event_callback_add(win1, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_IN, _boolean_flag_set, &win1_focus_in);
   efl_event_callback_add(win1, EFL_CANVAS_SCENE_EVENT_SCENE_FOCUS_OUT, _boolean_flag_set, &win1_focus_out);

   //focus in check
   ecore_evas_focus_set(ee, EINA_TRUE);
   if (!win1_focus_in)
     efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(win1_focus_in, EINA_TRUE);
   ck_assert_int_eq(win1_focus_out, EINA_FALSE);
   win1_focus_in = EINA_FALSE;

   //focus out check
   ecore_evas_focus_set(ee, EINA_FALSE);
   if (!win1_focus_out)
     efl_loop_begin(efl_main_loop_get());

   ck_assert_int_eq(win1_focus_out, EINA_TRUE);
   ck_assert_int_eq(win1_focus_in, EINA_FALSE);

   efl_unref(win1);
}
EFL_END_TEST
void
efl_ui_test_win(TCase *tc)
{
   tcase_add_test(tc, efl_ui_win_test_scene_focus);
}
