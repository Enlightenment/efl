#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"


START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *layout;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);

   layout = elm_layout_add(win);
   role = elm_interface_atspi_accessible_role_get(layout);

   ck_assert(role == ELM_ATSPI_ROLE_FILLER);

   elm_shutdown();
}
END_TEST

START_TEST(elm_layout_swallows)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly, *bt, *bt2;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);

   ly = eo_add(ELM_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_show(ly);

   bt = eo_add(ELM_BUTTON_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "element1"), bt));
   ck_assert_ptr_eq(eo_parent_get(bt), ly);

   bt = efl_content_unset(efl_part(ly, "element1"));
   ck_assert_ptr_eq(eo_parent_get(bt), evas_object_evas_get(bt));

   fail_if(!efl_content_set(efl_part(ly, "element1"), bt));
   ck_assert_ptr_eq(eo_parent_get(bt), ly);

   bt2 = eo_add(ELM_BUTTON_CLASS, ly);
   fail_if(!efl_content_set(efl_part(ly, "element1"), bt2));
   ck_assert_ptr_eq(eo_parent_get(bt2), ly);
   /* bt is deleted at this point. */
   ck_assert_ptr_eq(eo_parent_get(bt), evas_object_evas_get(bt));

   elm_shutdown();
}
END_TEST

START_TEST(elm_layout_edje_attach)
{
   char buf[PATH_MAX];
   Evas_Object *win, *layout;
   const Evas_Object *part;
   int r, g, b, a;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);

   layout = eo_add(ELM_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   edje_object_file_set(layout, buf, "layout_edje");
   evas_object_show(layout);

   edje_object_color_class_set(layout, "red", 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0);
   part = edje_object_part_object_get(layout, "red");
   evas_object_color_get(part, &r, &g, &b, &a);
   fail_if((r != 255) || (g != 0) || (b != 0) || (a != 255));

   edje_object_signal_emit(layout, "change", "test");
   edje_object_message_signal_process(layout);
   part = edje_object_part_object_get(layout, "blue");
   evas_object_color_get(part, &r, &g, &b, &a);
   fail_if((r != 0) || (g != 0) || (b != 255) || (a != 255));

   elm_shutdown();
}
END_TEST

START_TEST(elm_layout_model_connect)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly;
   Efl_Model_Item *model;
   Eina_Value v;
   const char *part_text;
   const char text_value[] = "A random string for elm_layout_model_connect test";

   elm_init(1, NULL);
   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);

   ly = eo_add(ELM_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_show(ly);

   model = eo_add(EFL_MODEL_ITEM_CLASS, win);
   ck_assert(!!eina_value_setup(&v, EINA_VALUE_TYPE_STRING));
   ck_assert(!!eina_value_set(&v, text_value));
   efl_model_property_set(model, "text_property", &v, NULL);

   efl_ui_model_connect(ly, "text", "text_property");
   efl_ui_view_model_set(ly, model);

   ecore_main_loop_iterate_may_block(EINA_TRUE);

   part_text = elm_layout_text_get(ly, "text");

   ck_assert_str_eq(part_text, text_value);

   elm_shutdown();
}
END_TEST

void elm_test_layout(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_layout_swallows);
   tcase_add_test(tc, elm_layout_edje_attach);
   tcase_add_test(tc, elm_layout_model_connect);
}
