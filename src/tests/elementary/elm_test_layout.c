#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_LAYOUT_CALC_PROTECTED
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

   ly = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_show(ly);

   bt = efl_add(EFL_UI_BUTTON_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "element1"), bt));
   ck_assert_ptr_eq(efl_parent_get(bt), ly);

   bt = efl_content_unset(efl_part(ly, "element1"));
   ck_assert_ptr_eq(efl_parent_get(bt), evas_object_evas_get(bt));

   fail_if(!efl_content_set(efl_part(ly, "element1"), bt));
   ck_assert_ptr_eq(efl_parent_get(bt), ly);

   bt2 = efl_add(EFL_UI_BUTTON_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "element1"), bt2));
   ck_assert_ptr_eq(efl_parent_get(bt2), ly);

}
EFL_END_TEST

static Eina_Value
_propagated_cb(void *data EINA_UNUSED,
               const Eina_Value v,
               const Eina_Future *dead_future EINA_UNUSED)
{
   ecore_main_loop_quit();

   fprintf(stderr, "delivered '%s'\n", eina_value_to_string(&v));

   return v;
}

EFL_START_TEST(elm_layout_test_model_connect)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly;
   Efl_Model_Item *model;
   Eina_Value v;
   Eina_Future *f;
   const char *part_text;
   const char text_value[] = "A random string for elm_layout_model_connect test";

   win = win_add(NULL, "layout", ELM_WIN_BASIC);

   ly = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_show(ly);

   model = efl_add(EFL_MODEL_ITEM_CLASS, win);
   ck_assert(!!eina_value_setup(&v, EINA_VALUE_TYPE_STRING));
   ck_assert(!!eina_value_set(&v, text_value));
   f = efl_model_property_set(model, "text_property", &v);
   eina_future_then(f, _propagated_cb, NULL, NULL);

   efl_ui_model_connect(ly, "text", "text_property");
   efl_ui_view_model_set(ly, model);

   ecore_main_loop_begin();

   part_text = elm_layout_text_get(ly, "text");

   ck_assert_str_eq(part_text, text_value);

}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_layout_api_size_min)
{
   Evas_Object *win;
   Eina_Size2D res;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   res = efl_layout_calc_size_min(layout, EINA_SIZE2D(2, 2));
   ck_assert_int_eq(res.w, 2);
   ck_assert_int_eq(res.h, 2);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_layout_api_update_hints)
{
   Evas_Object *win;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   efl_layout_calc_auto_update_hints_set(layout, EINA_TRUE);
   ck_assert_int_eq(efl_layout_calc_auto_update_hints_get(layout), EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_layout_force)
{
   Evas_Object *win;

   win = win_add(NULL, "layout", ELM_WIN_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   efl_layout_calc_force(layout);
}
EFL_END_TEST
void elm_test_layout(TCase *tc)
{
   tcase_add_test(tc, elm_layout_test_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_layout_test_swallows);
   tcase_add_test(tc, elm_layout_test_model_connect);
   tcase_add_test(tc, efl_ui_layout_layout_api_size_min);
   tcase_add_test(tc, efl_ui_layout_layout_api_update_hints);
   tcase_add_test(tc, efl_ui_layout_layout_force);
}
