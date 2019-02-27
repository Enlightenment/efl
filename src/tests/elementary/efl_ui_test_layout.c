#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_LAYOUT_CALC_PROTECTED
#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eina_Value
_propagated_cb(void *data EINA_UNUSED,
               const Eina_Value v,
               const Eina_Future *dead_future EINA_UNUSED)
{
   ecore_main_loop_quit();

   fprintf(stderr, "delivered '%s'\n", eina_value_to_string(&v));

   return v;
}

EFL_START_TEST(efl_ui_layout_test_property_bind)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly;
   Efl_Generic_Model *model;
   Eina_Value v;
   Eina_Future *f;
   const char *part_text;
   const char text_value[] = "A random string for elm_layout_property_bind test";

   win = win_add(NULL, "layout", EFL_UI_WIN_BASIC);

   ly = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   efl_file_simple_load(ly, buf, "layout");
   efl_gfx_entity_visible_set(ly, EINA_TRUE);

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   ck_assert(!!eina_value_setup(&v, EINA_VALUE_TYPE_STRING));
   ck_assert(!!eina_value_set(&v, text_value));
   f = efl_model_property_set(model, "text_property", &v);
   eina_future_then(f, _propagated_cb, NULL, NULL);

   efl_ui_property_bind(ly, "text", "text_property");
   efl_ui_view_model_set(ly, model);

   ecore_main_loop_begin();

   part_text = efl_text_get(efl_part(ly, "text"));

   ck_assert_str_eq(part_text, text_value);

}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_layout_api_size_min)
{
   Evas_Object *win;
   Eina_Size2D res;

   win = win_add(NULL, "layout", EFL_UI_WIN_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   res = efl_layout_calc_size_min(layout, EINA_SIZE2D(2, 2));
   ck_assert_int_eq(res.w, 2);
   ck_assert_int_eq(res.h, 2);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_layout_api_update_hints)
{
   Evas_Object *win;

   win = win_add(NULL, "layout", EFL_UI_WIN_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   efl_layout_calc_auto_update_hints_set(layout, EINA_TRUE);
   ck_assert_int_eq(efl_layout_calc_auto_update_hints_get(layout), EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_layout_force)
{
   Evas_Object *win;

   win = win_add(NULL, "layout", EFL_UI_WIN_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   efl_layout_calc_force(layout);
}
EFL_END_TEST

void efl_ui_test_layout(TCase *tc)
{
   tcase_add_test(tc, efl_ui_layout_test_property_bind);
   tcase_add_test(tc, efl_ui_layout_test_layout_api_size_min);
   tcase_add_test(tc, efl_ui_layout_test_layout_api_update_hints);
   tcase_add_test(tc, efl_ui_layout_test_layout_force);
}
