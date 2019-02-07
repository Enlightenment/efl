#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_spinner_legacy_type_check)
{
   Evas_Object *win, *spinner;
   const char *type;

   win = win_add(NULL, "spinner", ELM_WIN_BASIC);

   spinner = elm_spinner_add(win);

   type = elm_object_widget_type_get(spinner);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Spinner"));

   type = evas_object_type_get(spinner);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_spinner"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *spinner;
   Efl_Access_Role role;

   win = win_add(NULL, "spinner", ELM_WIN_BASIC);

   spinner = elm_spinner_add(win);
   role = efl_access_object_role_get(spinner);

   ck_assert(role == EFL_ACCESS_ROLE_SPIN_BUTTON);

}
EFL_END_TEST


EFL_START_TEST(elm_spinner_focus_next_object)
{
   Evas_Object *win, *box, *spinner, *btn, *o;

   win = win_add(NULL, "spinner", ELM_WIN_BASIC);

   box = elm_box_add(win);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   spinner = elm_spinner_add(box);
   elm_box_pack_end(box, spinner);
   evas_object_show(spinner);

   btn = elm_button_add(box);
   elm_box_pack_end(box, btn);
   evas_object_show(btn);

   evas_object_show(win);

   elm_object_focus_set(spinner, EINA_TRUE);

   elm_object_focus_next_object_set(spinner, btn, EFL_UI_FOCUS_DIRECTION_UP);
   o = elm_object_focus_next_object_get(spinner, EFL_UI_FOCUS_DIRECTION_UP);
   ck_assert_ptr_eq(o, btn);

   elm_object_focus_next(win, EFL_UI_FOCUS_DIRECTION_UP);
   ck_assert_ptr_eq(elm_object_focused_object_get(win), btn);
}
EFL_END_TEST

void elm_test_spinner(TCase *tc)
{
   tcase_add_test(tc, elm_spinner_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_spinner_focus_next_object);
}

