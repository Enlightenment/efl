#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_hover_legacy_type_check)
{
   Evas_Object *win, *hover;
   const char *type;

   win = win_add(NULL, "hover", ELM_WIN_BASIC);

   hover = elm_hover_add(win);

   type = elm_object_widget_type_get(hover);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Hover"));

   type = evas_object_type_get(hover);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_hover"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *hover;
   Efl_Access_Role role;

   win = win_add(NULL, "hover", ELM_WIN_BASIC);

   hover = elm_hover_add(win);
   role = efl_access_object_role_get(hover);

   ck_assert(role == EFL_ACCESS_ROLE_POPUP_MENU);

}
EFL_END_TEST

EFL_START_TEST(elm_test_hover_behavior)
{
   Evas_Object *win, *hover, *target, *rect;
   int count = 0;
   int count_dismiss = 0;
   int count_clicked = 0;

   win = win_add(NULL, "hover", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);

   hover = elm_hover_add(win);
   evas_object_smart_callback_add(hover, "smart,changed", event_callback_that_increments_an_int_when_called, &count);
   evas_object_smart_callback_add(hover, "dismissed", event_callback_single_call_int_data, &count_dismiss);
   evas_object_smart_callback_add(hover, "clicked", event_callback_single_call_int_data, &count_clicked);

   target = elm_box_add(win);

   elm_hover_parent_set(hover, win);
   elm_hover_target_set(hover, target);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_min_set(rect, 20, 20);
   evas_object_show(rect);
   elm_object_part_content_set(hover, "smart", rect);

   evas_object_move(target, 40, 40);
   evas_object_show(win);
   evas_object_show(hover);
   get_me_to_those_events(hover);

   /* 1 from setting content, 1 from moving target object */
   ck_assert_int_eq(count, 2);

   assert_object_size_eq(rect, 20, 20);
   assert_object_size_eq(hover, 0, 0);
   assert_object_pos_eq(hover, 40, 40);
   /* using non-centered slot */
   assert_object_pos_eq(rect, 40, 30);

   elm_hover_dismiss(hover);
   get_me_to_those_events(hover);
   /* these are both triggered on dismiss */
   ck_assert_int_eq(count_dismiss, 1);
   ck_assert_int_eq(count_clicked, 1);
}
EFL_END_TEST

void elm_test_hover(TCase *tc)
{
   tcase_add_test(tc, elm_hover_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_test_hover_behavior);
}
