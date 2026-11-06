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

static unsigned int _theme_changed_count;
static int _theme_changed_version;

static void
_theme_changed_cb(void *data, const Efl_Event *ev)
{
   Eo *ly = data;

   EINA_SAFETY_ON_FALSE_RETURN(ev->object == ly);
   _theme_changed_count++;
   _theme_changed_version = efl_ui_layout_theme_version_get(ly);
}

// theme,changed is emitted once the theme is applied, so a listener that
// reads the theme version sees the one of the theme just loaded.
EFL_START_TEST(efl_ui_layout_test_theme_changed_sees_new_version)
{
   Eo *win, *ly, *def;
   char first[PATH_MAX], second[PATH_MAX];

   snprintf(first, sizeof(first), "%s/objects/test.edj", ELM_TEST_DATA_DIR);
   snprintf(second, sizeof(second), "%s/objects/test_theme_version.edj", ELM_TEST_DATA_DIR);
   def = efl_ui_theme_default_get();
   efl_ui_theme_extension_add(def, first);

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   ly = efl_add(EFL_UI_LAYOUT_CLASS, win,
                efl_ui_layout_theme_set(efl_added, "version_test", NULL, NULL));
   ck_assert_int_eq(efl_ui_layout_theme_version_get(ly), 123);

   _theme_changed_count = 0;
   _theme_changed_version = 0;
   efl_event_callback_add(ly, EFL_UI_LAYOUT_EVENT_THEME_CHANGED, _theme_changed_cb, ly);

   // The overlay answers before the extension: the same group, a version
   // later. Widgets apply their theme again when the theme changes.
   efl_ui_theme_overlay_add(def, second);

   ck_assert_uint_eq(_theme_changed_count, 1);
   ck_assert_int_eq(_theme_changed_version, 124);
   ck_assert_int_eq(efl_ui_layout_theme_version_get(ly), 124);

   efl_event_callback_del(ly, EFL_UI_LAYOUT_EVENT_THEME_CHANGED, _theme_changed_cb, ly);
   efl_ui_theme_overlay_del(def, second);
   efl_ui_theme_extension_del(def, first);
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

typedef struct
{
   Eo *from;
   int heard;
   int freed;
} Signal_Probe;

static void
_probe_signal_cb(void *data, Efl_Layout_Signal *obj, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Signal_Probe *probe = data;

   ck_assert_int_eq(probe->freed, 0);
   probe->from = obj;
   probe->heard++;
}

static void
_probe_free(void *data)
{
   Signal_Probe *probe = data;

   probe->freed++;
}

/* A signal callback added to a widget is called with the widget -- not with
 * the edje object inside it, which the caller has never seen -- can be
 * deleted with what it was added with, and has its data freed once: when it
 * is deleted, or when the widget goes away. */
EFL_START_TEST(efl_ui_layout_test_signal_callback_object)
{
   Signal_Probe kept = { 0 }, dropped = { 0 };
   Eo *win, *layout;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   layout = efl_add(EFL_UI_BUTTON_CLASS, win);

   ck_assert(efl_layout_signal_callback_add(layout, "ego,test", "ego", &kept, _probe_signal_cb, _probe_free));
   ck_assert(efl_layout_signal_callback_add(layout, "ego,test", "ego", &dropped, _probe_signal_cb, _probe_free));
   efl_layout_signal_emit(layout, "ego,test", "ego");
   efl_layout_signal_process(layout, EINA_TRUE);
   ck_assert_int_eq(kept.heard, 1);
   ck_assert_ptr_eq(kept.from, layout);

   ck_assert(efl_layout_signal_callback_del(layout, "ego,test", "ego", &dropped, _probe_signal_cb, _probe_free));
   ck_assert_int_eq(dropped.freed, 1);
   ck_assert(!efl_layout_signal_callback_del(layout, "ego,test", "ego", &dropped, _probe_signal_cb, _probe_free));
   efl_layout_signal_emit(layout, "ego,test", "ego");
   efl_layout_signal_process(layout, EINA_TRUE);
   ck_assert_int_eq(kept.heard, 2);
   ck_assert_int_eq(dropped.heard, 1);

   efl_del(layout);
   ck_assert_int_eq(kept.freed, 1);
   ck_assert_int_eq(dropped.freed, 1);
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

/* One binding is as good as two.
 *
 * A widget subscribes to its model when something is bound to it, and the
 * subscription used to be attempted before the first binding was recorded --
 * so the first binding on a widget listened to nothing, and a widget with
 * exactly one bound property never heard the value arrive. Two bindings
 * worked, because the second registered what the first had set up. Any model
 * that answers later than it is asked, which is every asynchronous one, was
 * therefore never read at all through a single binding.
 */
EFL_START_TEST(efl_ui_layout_test_property_bind_alone)
{
   Evas_Object *win, *one, *two;
   Efl_Generic_Model *model;
   Eina_Value v;
   int spins;

   win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);

   one = efl_add(EFL_UI_BUTTON_CLASS, win, efl_ui_view_model_set(efl_added, model));
   efl_ui_property_bind(one, "text", "greeting");

   two = efl_add(EFL_UI_BUTTON_CLASS, win, efl_ui_view_model_set(efl_added, model));
   efl_ui_property_bind(two, "text", "greeting");
   efl_ui_property_bind(two, "tooltip", "greeting");

   /* Set after binding, which is the order an asynchronous model imposes. */
   v = eina_value_string_init("hello");
   efl_model_property_set(model, "greeting", &v);

   for (spins = 0; spins < 200; spins++) ecore_main_loop_iterate();

   ck_assert_str_eq(efl_text_get(two) ?: "", "hello");
   ck_assert_str_eq(efl_text_get(one) ?: "", "hello");
}
EFL_END_TEST

/* A part asked about before the theme has arrived.
 *
 * Which kind of part something is comes from the group, and a widget still
 * being built has none -- so part_get used to answer "not text, not swallow"
 * and hand back the plain part class, which implements neither Efl.Text nor
 * Efl.Content. Everything written through it was dropped without a word.
 *
 * The classes it bit were the ones whose text goes to a named theme part:
 * Efl.Ui.Check and everything built on Efl.Ui.Default_Item. Button and
 * Textbox route theirs elsewhere and always worked, which is what made it
 * look like the callers were holding it wrong.
 */
EFL_START_TEST(efl_ui_layout_text_set_while_building)
{
   Eo *win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   const Efl_Class *klasses[] = {
      EFL_UI_BUTTON_CLASS, EFL_UI_CHECK_CLASS, EFL_UI_LIST_DEFAULT_ITEM_CLASS,
      EFL_UI_GRID_DEFAULT_ITEM_CLASS,
   };
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(klasses); i++)
     {
        Eo *inside, *after;

        /* The idiomatic way round: everything in one efl_add. */
        inside = efl_add(klasses[i], win, efl_text_set(efl_added, "IN"));
        ck_assert_ptr_ne(inside, NULL);
        ck_assert_ptr_ne(efl_text_get(inside), NULL);
        ck_assert_str_eq(efl_text_get(inside), "IN");

        /* And the long way round, which always worked. */
        after = efl_add(klasses[i], win);
        efl_text_set(after, "AFTER");
        ck_assert_str_eq(efl_text_get(after), "AFTER");

        efl_del(inside);
        efl_del(after);
     }

   efl_del(win);
}
EFL_END_TEST

/* Content set the same way, which never had the problem -- a swallow is
   remembered as the object it is -- and must go on not having it. */
EFL_START_TEST(efl_ui_layout_content_set_while_building)
{
   Eo *win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   Eo *item, *icon;

   item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, win);
   icon = efl_add(EFL_UI_IMAGE_CLASS, item);
   efl_content_set(efl_part(item, "icon"), icon);
   ck_assert_ptr_eq(efl_content_get(efl_part(item, "icon")), icon);

   efl_del(item);
   efl_del(win);
}
EFL_END_TEST

/* A part handle is only something alongside the widget it belongs to, which
 * efl_part() attaches before finalize. Built directly -- which efl_add()
 * allows, and which anything walking every class will do -- there is no
 * widget, and the part used to work off NULL and take the process down. It
 * is refused instead. */
EFL_START_TEST(efl_ui_layout_part_without_widget)
{
   Eo *win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   const Efl_Class *klasses[] = {
      EFL_UI_WIDGET_PART_CLASS, EFL_UI_WIDGET_PART_BG_CLASS,
      EFL_UI_LAYOUT_PART_CLASS, EFL_UI_LAYOUT_PART_TEXT_CLASS,
      EFL_UI_LAYOUT_PART_CONTENT_CLASS, EFL_UI_LAYOUT_PART_BG_CLASS,
   };
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(klasses); i++)
     ck_assert_ptr_eq(efl_add(klasses[i], win), NULL);

   /* and a part handed out by its widget still works */
   efl_text_set(efl_part(efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, win), "text"), "works");
}
EFL_END_TEST

/* Content replaced in a part is the content the part then has, even while
   something -- a binding, an application -- still holds a reference to what
   it replaced: the replaced object is taken out of the layout, not just
   deleted and left to be forgotten once it is freed. */
EFL_START_TEST(efl_ui_layout_content_replaced_while_held)
{
   Eo *win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   Eo *item, *a, *b;

   item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, win);
   a = efl_add(EFL_UI_IMAGE_CLASS, item);
   b = efl_add(EFL_UI_IMAGE_CLASS, item);
   efl_ref(a);

   efl_content_set(efl_part(item, "extra"), a);
   ck_assert_ptr_eq(efl_content_get(efl_part(item, "extra")), a);
   efl_content_set(efl_part(item, "extra"), b);
   ck_assert_ptr_eq(efl_content_get(efl_part(item, "extra")), b);

   /* Emptying the part leaves nothing behind in it. */
   efl_content_set(efl_part(item, "extra"), NULL);
   ck_assert_ptr_eq(efl_content_get(efl_part(item, "extra")), NULL);

   efl_unref(a);
   efl_del(item);
   efl_del(win);
}
EFL_END_TEST

static void
_count_errors(const Eina_Log_Domain *d, Eina_Log_Level level,
              const char *file, const char *fnc, int line,
              const char *fmt, void *data, va_list args)
{
   int *errors = data;

   if (level <= EINA_LOG_LEVEL_ERR)
     (*errors)++;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

/* A panel calculates its sub-objects when it is told to calculate, and one of
   them is a plain rectangle that has no calculation to run. */
EFL_START_TEST(efl_ui_layout_calc_subobjs_without_calc)
{
   Eo *win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   Eo *panel;
   int errors = 0;

   panel = efl_add(EFL_UI_PANEL_CLASS, win);

   eina_log_print_cb_set(_count_errors, &errors);
   efl_canvas_group_calculate(panel);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   ck_assert_int_eq(errors, 0);
}
EFL_END_TEST

static void
_building_count(void *data, const Efl_Event *ev EINA_UNUSED)
{
   unsigned int *count = data;

   (*count)++;
}

/* Binding a part again to the factory it is bound to leaves the content it
 * has: nothing is unset, released or asked of the factory a second time. */
EFL_START_TEST(efl_ui_layout_test_factory_bind_same_factory)
{
   Eo *win = win_add(NULL, "layout", EFL_UI_WIN_TYPE_BASIC);
   Eo *model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   Eo *factory, *item, *content;
   unsigned int building = 0;
   int i;

   factory = efl_add(EFL_UI_WIDGET_FACTORY_CLASS, win,
                     efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_BUTTON_CLASS),
                     efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                            _building_count, &building));
   item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, win,
                  efl_ui_view_model_set(efl_added, model));

   efl_ui_factory_bind(item, "icon", factory);
   for (i = 0; (i < 100) && !efl_content_get(efl_part(item, "icon")); i++)
     {
        ecore_main_loop_iterate();
     }
   content = efl_content_get(efl_part(item, "icon"));
   ck_assert_ptr_ne(content, NULL);
   ck_assert_uint_eq(building, 1);

   efl_ui_factory_bind(item, "icon", factory);
   for (i = 0; i < 10; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_eq(efl_content_get(efl_part(item, "icon")), content);
   ck_assert_uint_eq(building, 1);
}
EFL_END_TEST

void efl_ui_test_layout(TCase *tc)
{
   tcase_add_test(tc, efl_ui_layout_calc_subobjs_without_calc);
   tcase_add_test(tc, efl_ui_layout_content_replaced_while_held);
   tcase_add_test(tc, efl_ui_layout_part_without_widget);
   tcase_add_test(tc, efl_ui_layout_text_set_while_building);
   tcase_add_test(tc, efl_ui_layout_content_set_while_building);
   tcase_add_test(tc, efl_ui_layout_test_property_bind);
   tcase_add_test(tc, efl_ui_layout_test_theme_changed_sees_new_version);
   tcase_add_test(tc, efl_ui_layout_test_layout_api_size_min);
   tcase_add_test(tc, efl_ui_layout_test_layout_api_update_hints);
   tcase_add_test(tc, efl_ui_layout_test_layout_force);
   tcase_add_test(tc, efl_ui_layout_test_layout_theme);
   tcase_add_test(tc, efl_ui_layout_test_api_ordering);
   tcase_add_test(tc, efl_ui_layout_test_callback);
   tcase_add_test(tc, efl_ui_layout_test_property_bind_provider);
   tcase_add_test(tc, efl_ui_layout_test_signal_callback_object);
   tcase_add_test(tc, efl_ui_layout_test_property_bind_alone);
   tcase_add_test(tc, efl_ui_layout_test_versioning);
   tcase_add_test(tc, efl_ui_layout_test_freeze);
   tcase_add_test(tc, efl_ui_layout_test_factory_bind_same_factory);
}
