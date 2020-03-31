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
   Eina_Value vt, vs;
   Eina_Future *f, *f1, *f2;
   int r = 0, g = 0, b = 0, a = 0;
   const char *part_text;
   const char text_value[] = "A random string for elm_layout_property_bind test";

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);

   ly = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   ck_assert(efl_file_simple_load(ly, buf, "layout"));

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);

   vt = eina_value_string_init(text_value);
   f1 = efl_model_property_set(model, "text_property", &vt);

   vs = eina_value_string_init("#0A0D0EFF");
   f2 = efl_model_property_set(model, "background_color", &vs);

   f = eina_future_all(f1, f2);
   eina_future_then(f, _propagated_cb, NULL, NULL);

   efl_ui_property_bind(ly, "text", "text_property");
   efl_ui_property_bind(efl_part(ly, "background"), "color_code", "background_color");
   efl_ui_view_model_set(ly, model);

   ecore_main_loop_begin();

   part_text = efl_text_get(efl_part(ly, "text"));
   ck_assert_str_eq(part_text, text_value);

   efl_gfx_color_get(efl_part(ly, "background"), &r, &g, &b, &a);
   ck_assert_int_eq(r, 0xA);
   ck_assert_int_eq(g, 0xD);
   ck_assert_int_eq(b, 0xE);
   ck_assert_int_eq(a, 0xFF);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_property_bind_provider)
{
   char buf[PATH_MAX];
   Evas_Object *win, *ly;
   Efl_Generic_Model *model;
   Efl_Model_Provider *provider;
   Eina_Value vt, vs;
   Eina_Future *f, *f1, *f2;
   int r = 0, g = 0, b = 0, a = 0;
   const char *part_text;
   const char text_value[] = "A random string for elm_layout_property_bind test";

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);

   provider = efl_add(EFL_MODEL_PROVIDER_CLASS, win);
   efl_provider_register(win, EFL_MODEL_PROVIDER_CLASS, provider);

   ly = efl_add(EFL_UI_LAYOUT_CLASS, win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   ck_assert(efl_file_simple_load(ly, buf, "layout"));

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);

   vt = eina_value_string_init(text_value);
   f1 = efl_model_property_set(model, "text_property", &vt);

   vs = eina_value_string_init("#0A0D0EFF");
   f2 = efl_model_property_set(model, "background_color", &vs);

   f = eina_future_all(f1, f2);
   eina_future_then(f, _propagated_cb, NULL, NULL);

   efl_ui_property_bind(ly, "text", "text_property");
   efl_ui_property_bind(efl_part(ly, "background"), "color_code", "background_color");
   efl_ui_view_model_set(provider, model);

   ecore_main_loop_begin();

   part_text = efl_text_get(efl_part(ly, "text"));
   ck_assert_str_eq(part_text, text_value);

   efl_gfx_color_get(efl_part(ly, "background"), &r, &g, &b, &a);
   ck_assert_int_eq(r, 0xA);
   ck_assert_int_eq(g, 0xD);
   ck_assert_int_eq(b, 0xE);
   ck_assert_int_eq(a, 0xFF);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_layout_api_size_min)
{
   Evas_Object *win;
   Eina_Size2D res;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
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

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   efl_layout_calc_auto_update_hints_set(layout, EINA_TRUE);
   ck_assert_int_eq(efl_layout_calc_auto_update_hints_get(layout), EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_layout_force)
{
   Evas_Object *win;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   /* this is just a test to not get segfaults in those calls */
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win);
   efl_layout_calc_force(layout);
}
EFL_END_TEST

/* private */
EAPI Eina_Bool elm_widget_theme_klass_set(Evas_Object *obj, const char *name);
EAPI Eina_Bool elm_widget_theme_style_set(Evas_Object *obj, const char *name);

EFL_START_TEST(efl_ui_layout_test_callback)
{
   Evas_Object *win;
   int called = 0;
   Eina_Bool klass, style;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   efl_add(EFL_UI_LAYOUT_CLASS, win,
     efl_event_callback_add(efl_added, EFL_UI_LAYOUT_EVENT_THEME_CHANGED, (void*)event_callback_single_call_int_data, &called),
     klass = elm_widget_theme_klass_set(efl_added, "button"),
     style = elm_widget_theme_style_set(efl_added, "anchor")
   );
   ck_assert_int_eq(klass, 1);
   ck_assert_int_eq(style, 1);
   ck_assert_int_eq(called, 1);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_layout_theme)
{
   Evas_Object *win;
   const char *klass, *group, *style;
   Eina_Error err;
   int called = 0;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   Eo *layout = efl_add(EFL_UI_LAYOUT_CLASS, win,
     efl_event_callback_add(efl_added, EFL_UI_LAYOUT_EVENT_THEME_CHANGED, (void*)event_callback_single_call_int_data, &called),
     err = efl_ui_layout_theme_set(efl_added, "button", NULL, "anchor")
   );
   ck_assert_int_eq(err, 0);
   efl_ui_layout_theme_get(layout, &klass, &group, &style);
   ck_assert_str_eq(klass, "button");
   ck_assert(!group);
   ck_assert_str_eq(style, "anchor");
   ck_assert_int_eq(called, 1);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_api_ordering)
{
   Evas_Object *win, *box;
   int count = 0;
   const char text_text[] = "test text";

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   box = efl_add(EFL_UI_BOX_CLASS, win);
   Eo *layout = efl_add(EFL_UI_BUTTON_CLASS, win,
     efl_layout_signal_callback_add(efl_added, "efl,content,set", "efl", &count, (void*)event_callback_single_call_int_data, NULL),
     efl_ui_widget_style_set(efl_added, "anchor"),
     efl_content_set(efl_added, box),
     efl_text_set(efl_added, text_text)
   );
   ecore_main_loop_iterate();
   ck_assert_int_eq(count, 1);
   ck_assert_ptr_eq(efl_content_get(layout), box);
   ck_assert_str_eq(efl_text_get(layout), text_text);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_versioning)
{
   Evas_Object *win;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   Eo *layout = efl_add(EFL_UI_BUTTON_CLASS, win);
   ck_assert_int_eq(efl_ui_layout_theme_version_get(layout), 123);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_layout_test_freeze)
{
   Evas_Object *win;
   int count = 0;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));
   Eo *layout = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_gfx_entity_size_set(layout, EINA_SIZE2D(100, 100));
   efl_text_set(layout, "button");
   get_me_to_those_events(layout);

   efl_event_callback_add(win, EFL_CANVAS_SCENE_EVENT_RENDER_PRE, (void*)event_callback_single_call_int_data, &count);
   efl_layout_calc_freeze(layout);
   efl_layout_calc_thaw(layout);
   force_render(win);
   ck_assert_int_eq(count, 0);
}
EFL_END_TEST

void efl_ui_test_layout(TCase *tc)
{
   tcase_add_test(tc, efl_ui_layout_test_property_bind);
   tcase_add_test(tc, efl_ui_layout_test_layout_api_size_min);
   tcase_add_test(tc, efl_ui_layout_test_layout_api_update_hints);
   tcase_add_test(tc, efl_ui_layout_test_layout_force);
   tcase_add_test(tc, efl_ui_layout_test_layout_theme);
   tcase_add_test(tc, efl_ui_layout_test_api_ordering);
   tcase_add_test(tc, efl_ui_layout_test_callback);
   tcase_add_test(tc, efl_ui_layout_test_property_bind_provider);
   tcase_add_test(tc, efl_ui_layout_test_versioning);
   tcase_add_test(tc, efl_ui_layout_test_freeze);
}
