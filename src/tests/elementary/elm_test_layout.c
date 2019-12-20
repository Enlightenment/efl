#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_layout_test_legacy_type_check)
{
   Evas_Object *win, *layout;
   const char *type;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   layout = elm_layout_add(win);

   type = elm_object_widget_type_get(layout);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Layout"));

   type = evas_object_type_get(layout);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_layout"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *layout;
   Efl_Access_Role role;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   layout = elm_layout_add(win);
   role = efl_access_object_role_get(layout);

   ck_assert(role == EFL_ACCESS_ROLE_FILLER);

}
EFL_END_TEST

EFL_START_TEST(elm_layout_test_swallows)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly, *bt, *bt2;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_show(ly);

   bt = elm_button_add(ly);
   fail_if(!elm_layout_content_set(ly, "element1", bt));
   ck_assert_ptr_eq(efl_parent_get(bt), ly);

   bt = elm_object_part_content_unset(ly, "element1");
   ck_assert_ptr_eq(efl_parent_get(bt), evas_object_evas_get(bt));

   fail_if(!elm_layout_content_set(ly, "element1", bt));
   ck_assert_ptr_eq(efl_parent_get(bt), ly);

   bt2 = elm_button_add(ly);
   fail_if(!elm_layout_content_set(ly, "element1", bt2));
   ck_assert_ptr_eq(efl_parent_get(bt2), ly);

}
EFL_END_TEST

EFL_START_TEST(elm_layout_test_sizing)
{
   Evas_Object *win, *ly, *rect;
   int w, h;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);
   evas_object_show(win);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));

   ly = elm_button_add(win);
   elm_object_text_set(ly, "test");
   elm_object_content_set(ly, rect);
   evas_object_show(ly);

   /* verify that the button calc is the size of the text */
   evas_object_smart_need_recalculate_set(ly, 1);
   evas_object_smart_calculate(ly);
   evas_object_size_hint_min_get(ly, &w, &h);
   ck_assert_int_lt(w, 100);
   ck_assert_int_gt(w, 0);
   ck_assert_int_lt(h, 100);
   ck_assert_int_gt(h, 0);

   /* verify that the button calc is the size of the text + minsize of rect */
   evas_object_size_hint_min_set(rect, 100, 100);
   evas_object_smart_need_recalculate_set(ly, 1);
   evas_object_smart_calculate(ly);
   evas_object_size_hint_min_get(ly, &w, &h);
   ck_assert_int_ge(w, 100);
   ck_assert_int_ge(h, 100);

   /* verify that the button calc is once again the size of the text */
   evas_object_size_hint_min_set(rect, 0, 0);
   evas_object_smart_need_recalculate_set(ly, 1);
   evas_object_smart_calculate(ly);
   evas_object_size_hint_min_get(ly, &w, &h);
   ck_assert_int_lt(w, 100);
   ck_assert_int_gt(w, 0);
   ck_assert_int_lt(h, 100);
   ck_assert_int_gt(h, 0);
}
EFL_END_TEST

EFL_START_TEST(elm_layout_test_theme_changed_callback)
{
   Evas_Object *win, *ly;
   int called = 0;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);
   evas_object_show(win);

   ly = elm_button_add(win);
   evas_object_smart_callback_add(ly, "theme,changed", event_callback_single_call_int_data, &called);
   evas_object_show(ly);
   elm_object_style_set(ly, "anchor");
   ck_assert_int_eq(called, 1);
}
EFL_END_TEST

void elm_test_layout(TCase *tc)
{
   tcase_add_test(tc, elm_layout_test_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_layout_test_swallows);
   tcase_add_test(tc, elm_layout_test_sizing);
   tcase_add_test(tc, elm_layout_test_theme_changed_callback);
}
