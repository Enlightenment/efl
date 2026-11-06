#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "efl_ui_suite.h"

/* A widget bound to a property of its model must write what the user did
 * back to that model, and must not do it for what the model told it. */

static Eo *
_model_new(const char *property, Eina_Value *value)
{
   Eo *model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   Eina_Future *f;

   ck_assert_ptr_ne(model, NULL);
   f = efl_model_property_set(model, property, value);
   eina_future_cancel(f);
   return model;
}

static void
_model_set(Eo *model, const char *property, Eina_Value *value)
{
   eina_future_cancel(efl_model_property_set(model, property, value));
}

static Eina_Value *
_model_get(Eo *model, const char *property)
{
   Eina_Value *value = efl_model_property_get(model, property);

   ck_assert_ptr_ne(value, NULL);
   return value;
}

static unsigned int model_changes;

static void
_model_changed(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   model_changes++;
}

static Eo *
_window(void)
{
   Eo *win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(400, 200));
   return win;
}

EFL_START_TEST(textbox_user_edit_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt;
   Eina_Value *v;
   char *s;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                 efl_ui_view_model_set(efl_added, model),
                 efl_ui_property_bind(efl_added, "text", "t"),
                 efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(300, 100)));
   ck_assert_str_eq(efl_text_get(txt), "Hello");

   efl_gfx_entity_visible_set(txt, EINA_TRUE);
   efl_ui_focus_util_focus(txt);
   efl_text_cursor_object_move(efl_text_interactive_main_cursor_get(txt),
                               EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   evas_event_feed_key_down(evas_object_evas_get(txt), "s", "s", "s", "s", time(NULL), NULL);
   ecore_main_loop_iterate();
   ck_assert_str_eq(efl_text_get(txt), "Hellos");

   v = _model_get(model, "t");
   ck_assert_ptr_eq(eina_value_type_get(v), EINA_VALUE_TYPE_STRING);
   s = eina_value_to_string(v);
   ck_assert_str_eq(s, "Hellos");
   free(s);
   eina_value_free(v);
}
EFL_END_TEST

EFL_START_TEST(textbox_model_change_does_not_loop)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("one"));
   Eo *txt;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                 efl_ui_view_model_set(efl_added, model),
                 efl_ui_property_bind(efl_added, "text", "t"));
   model_changes = 0;
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);

   _model_set(model, "t", eina_value_string_new("two"));
   ecore_main_loop_iterate();

   ck_assert_str_eq(efl_text_get(txt), "two");
   // Only the set made above: the widget must not have answered the model.
   ck_assert_uint_eq(model_changes, 1);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
}
EFL_END_TEST

static Eo *
_typing_textbox(Eo *win, Eo *model)
{
   Eo *txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                     efl_ui_view_model_set(efl_added, model),
                     efl_ui_property_bind(efl_added, "text", "t"),
                     efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(300, 100)));

   efl_gfx_entity_visible_set(txt, EINA_TRUE);
   efl_ui_focus_util_focus(txt);
   return txt;
}

static void
_type_s(Eo *txt)
{
   evas_event_feed_key_down(evas_object_evas_get(txt), "s", "s", "s", "s", time(NULL), NULL);
   ecore_main_loop_iterate();
}

static void
_assert_model_text(Eo *model, const char *expected)
{
   Eina_Value *v = _model_get(model, "t");
   char *s = eina_value_to_string(v);

   ck_assert_str_eq(s, expected);
   free(s);
   eina_value_free(v);
}

EFL_START_TEST(textbox_typing_in_the_middle_keeps_the_cursor)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt = _typing_textbox(win, model);
   Eo *cur = efl_text_interactive_main_cursor_get(txt);

   efl_text_cursor_object_position_set(cur, 2);
   _type_s(txt);
   ck_assert_str_eq(efl_text_get(txt), "Hesllo");
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur), 3);
   _assert_model_text(model, "Hesllo");
}
EFL_END_TEST

EFL_START_TEST(textbox_typing_at_the_start_keeps_the_cursor)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt = _typing_textbox(win, model);
   Eo *cur = efl_text_interactive_main_cursor_get(txt);

   efl_text_cursor_object_position_set(cur, 0);
   _type_s(txt);
   ck_assert_str_eq(efl_text_get(txt), "sHello");
   ck_assert_int_eq(efl_text_cursor_object_position_get(cur), 1);
   _assert_model_text(model, "sHello");
}
EFL_END_TEST

EFL_START_TEST(textbox_typing_keeps_the_selection)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt = _typing_textbox(win, model);
   Eo *start, *end;

   efl_text_interactive_all_select(txt);
   // Typing over a selection replaces it; select again afterwards to see the echo leave it alone.
   _type_s(txt);
   ck_assert_str_eq(efl_text_get(txt), "s");
   _assert_model_text(model, "s");

   efl_text_interactive_all_select(txt);
   efl_text_interactive_selection_cursors_get(txt, &start, &end);
   _model_set(model, "t", eina_value_string_new("s"));
   ck_assert(efl_text_interactive_have_selection_get(txt));
   ck_assert_int_eq(efl_text_cursor_object_position_get(end)
                    - efl_text_cursor_object_position_get(start), 1);
}
EFL_END_TEST

EFL_START_TEST(textbox_cut_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt = _typing_textbox(win, model);

   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_cut(txt);
   ecore_main_loop_iterate();
   ck_assert_str_eq(efl_text_get(txt), "");
   _assert_model_text(model, "");
}
EFL_END_TEST

EFL_START_TEST(textbox_paste_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt = _typing_textbox(win, model);
   int i;

   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_copy(txt);
   efl_text_interactive_all_unselect(txt);
   efl_text_cursor_object_move(efl_text_interactive_main_cursor_get(txt),
                               EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   efl_ui_textbox_selection_paste(txt);
   // The clipboard answers from the loop.
   for (i = 0; (i < 50) && strcmp(efl_text_get(txt), "HelloHello"); i++)
     ecore_main_loop_iterate();
   ck_assert_str_eq(efl_text_get(txt), "HelloHello");
   _assert_model_text(model, "HelloHello");
}
EFL_END_TEST

EFL_START_TEST(part_binding_does_not_swallow_the_widget_write_back)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("Hello"));
   Eo *txt;

   eina_future_cancel(efl_model_property_set(model, "hint", eina_value_string_new("Type here")));
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                 efl_ui_view_model_set(efl_added, model),
                 efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(300, 100)));
   // The part is bound first: it must not take the widget's key.
   ck_assert_int_eq(efl_ui_property_bind(efl_part(txt, "efl.text_guide"), "text", "hint"), 0);
   ck_assert_int_eq(efl_ui_property_bind(txt, "text", "t"), 0);
   efl_gfx_entity_visible_set(txt, EINA_TRUE);
   efl_ui_focus_util_focus(txt);
   efl_text_cursor_object_move(efl_text_interactive_main_cursor_get(txt),
                               EFL_TEXT_CURSOR_MOVE_TYPE_LAST);
   _type_s(txt);
   _assert_model_text(model, "Hellos");
}
EFL_END_TEST

EFL_START_TEST(check_click_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("on", eina_value_bool_new(EINA_FALSE));
   Eo *check;
   Eina_Value *v;
   Eina_Bool b = EINA_FALSE;

   check = efl_add(EFL_UI_CHECK_CLASS, win,
                   efl_ui_view_model_set(efl_added, model),
                   efl_ui_property_bind(efl_added, "selected", "on"),
                   efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(100, 50)));
   efl_gfx_entity_visible_set(check, EINA_TRUE);
   ck_assert(!efl_ui_selectable_selected_get(check));

   click_object(check);
   ecore_main_loop_iterate();
   ck_assert(efl_ui_selectable_selected_get(check));
   v = _model_get(model, "on");
   ck_assert_ptr_eq(eina_value_type_get(v), EINA_VALUE_TYPE_UCHAR);
   eina_value_bool_get(v, &b);
   ck_assert(b);
   eina_value_free(v);

   click_object(check);
   ecore_main_loop_iterate();
   v = _model_get(model, "on");
   eina_value_bool_get(v, &b);
   ck_assert(!b);
   eina_value_free(v);
}
EFL_END_TEST

EFL_START_TEST(check_model_change_does_not_loop)
{
   Eo *win = _window();
   Eo *model = _model_new("on", eina_value_bool_new(EINA_FALSE));
   Eo *check;

   check = efl_add(EFL_UI_CHECK_CLASS, win,
                   efl_ui_view_model_set(efl_added, model),
                   efl_ui_property_bind(efl_added, "selected", "on"));
   model_changes = 0;
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);

   _model_set(model, "on", eina_value_bool_new(EINA_TRUE));
   ecore_main_loop_iterate();
   ck_assert(efl_ui_selectable_selected_get(check));
   ck_assert_uint_eq(model_changes, 1);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
}
EFL_END_TEST

EFL_START_TEST(slider_key_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("level", eina_value_double_new(0.5));
   Eo *slider;
   Eina_Value *v;
   double d = 0;

   slider = efl_add(EFL_UI_SLIDER_CLASS, win,
                    efl_ui_view_model_set(efl_added, model),
                    efl_ui_property_bind(efl_added, "range_value", "level"),
                    efl_ui_range_limits_set(efl_added, 0.0, 1.0),
                    efl_ui_range_step_set(efl_added, 0.1),
                    efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(300, 50)));
   efl_gfx_entity_visible_set(slider, EINA_TRUE);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(slider), 0.5));
   efl_ui_focus_util_focus(slider);

   evas_event_feed_key_down(evas_object_evas_get(slider), "Right", "Right", "", "",
                            time(NULL), NULL);
   ecore_main_loop_iterate();
   ck_assert(efl_ui_range_value_get(slider) > 0.55);

   v = _model_get(model, "level");
   ck_assert_ptr_eq(eina_value_type_get(v), EINA_VALUE_TYPE_DOUBLE);
   eina_value_double_get(v, &d);
   ck_assert(EINA_DBL_EQ(d, efl_ui_range_value_get(slider)));
   eina_value_free(v);
}
EFL_END_TEST

EFL_START_TEST(slider_model_change_does_not_loop)
{
   Eo *win = _window();
   Eo *model = _model_new("level", eina_value_double_new(0.5));
   Eo *slider;

   slider = efl_add(EFL_UI_SLIDER_CLASS, win,
                    efl_ui_view_model_set(efl_added, model),
                    efl_ui_property_bind(efl_added, "range_value", "level"));
   model_changes = 0;
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);

   _model_set(model, "level", eina_value_double_new(0.25));
   ecore_main_loop_iterate();
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(slider), 0.25));
   ck_assert_uint_eq(model_changes, 1);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
}
EFL_END_TEST

EFL_START_TEST(spin_button_wheel_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("count", eina_value_double_new(3.0));
   Eo *spin;
   Eina_Value *v;
   double d = 0;
   Evas *e;

   spin = efl_add(EFL_UI_SPIN_BUTTON_CLASS, win,
                  efl_ui_view_model_set(efl_added, model),
                  efl_ui_property_bind(efl_added, "range_value", "count"),
                  efl_ui_range_limits_set(efl_added, 0, 10),
                  efl_ui_range_step_set(efl_added, 1),
                  efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(200, 50)));
   efl_gfx_entity_visible_set(spin, EINA_TRUE);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(spin), 3.0));

   e = evas_object_evas_get(spin);
   get_me_to_those_events(spin);
   evas_event_feed_mouse_in(e, 0, NULL);
   evas_event_feed_mouse_move(e, 100, 25, 0, NULL);
   evas_event_feed_mouse_wheel(e, 0, -1, 0, NULL);
   ecore_main_loop_iterate();
   ck_assert(!EINA_DBL_EQ(efl_ui_range_value_get(spin), 3.0));

   v = _model_get(model, "count");
   ck_assert_ptr_eq(eina_value_type_get(v), EINA_VALUE_TYPE_DOUBLE);
   eina_value_double_get(v, &d);
   ck_assert(EINA_DBL_EQ(d, efl_ui_range_value_get(spin)));
   eina_value_free(v);
}
EFL_END_TEST

EFL_START_TEST(check_toggle_style_emits_once)
{
   Eo *win = _window();
   Eo *model = _model_new("on", eina_value_bool_new(EINA_FALSE));
   Eo *check;
   int i;

   check = efl_add(EFL_UI_CHECK_CLASS, win,
                   efl_ui_widget_style_set(efl_added, "toggle"),
                   efl_ui_view_model_set(efl_added, model),
                   efl_ui_property_bind(efl_added, "selected", "on"),
                   efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(200, 100)));
   efl_gfx_entity_visible_set(check, EINA_TRUE);
   for (i = 0; i < 3; i++)
     {
        efl_layout_signal_process(check, EINA_TRUE);
        ecore_main_loop_iterate();
     }

   model_changes = 0;
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
   click_object(check);
   for (i = 0; i < 5; i++)
     {
        efl_layout_signal_process(check, EINA_TRUE);
        ecore_main_loop_iterate();
     }
   ck_assert(efl_ui_selectable_selected_get(check));
   ck_assert_uint_eq(model_changes, 1);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
}
EFL_END_TEST

EFL_START_TEST(slider_refused_value_does_not_reach_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("level", eina_value_double_new(1.0));
   Eo *slider;

   slider = efl_add(EFL_UI_SLIDER_CLASS, win,
                    efl_ui_view_model_set(efl_added, model),
                    efl_ui_property_bind(efl_added, "range_value", "level"),
                    efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(300, 50)));
   efl_gfx_entity_visible_set(slider, EINA_TRUE);
   efl_ui_focus_util_focus(slider);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(slider), 1.0));

   model_changes = 0;
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
   // Already at the maximum: the key moves nothing.
   evas_event_feed_key_down(evas_object_evas_get(slider), "Right", "Right", "", "",
                            time(NULL), NULL);
   ecore_main_loop_iterate();
   ck_assert_uint_eq(model_changes, 0);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
}
EFL_END_TEST

static Eo *
_spin_button(Eo *win, Eo *model)
{
   Eo *spin = efl_add(EFL_UI_SPIN_BUTTON_CLASS, win,
                      efl_ui_view_model_set(efl_added, model),
                      efl_ui_property_bind(efl_added, "range_value", "count"),
                      efl_ui_range_limits_set(efl_added, 0, 10),
                      efl_ui_range_step_set(efl_added, 1),
                      efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(200, 50)));

   efl_gfx_entity_visible_set(spin, EINA_TRUE);
   return spin;
}

static double
_model_double(Eo *model, const char *property)
{
   Eina_Value *v = _model_get(model, property);
   double d = -1;

   eina_value_double_get(v, &d);
   eina_value_free(v);
   return d;
}

EFL_START_TEST(spin_button_model_change_does_not_loop)
{
   Eo *win = _window();
   Eo *model = _model_new("count", eina_value_double_new(3.0));
   Eo *spin = _spin_button(win, model);

   model_changes = 0;
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
   _model_set(model, "count", eina_value_double_new(7.0));
   ecore_main_loop_iterate();
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(spin), 7.0));
   ck_assert_uint_eq(model_changes, 1);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _model_changed, NULL);
}
EFL_END_TEST

static Eo *
_open_entry(Eo *spin)
{
   Eo *entry;

   efl_ui_spin_button_direct_text_input_set(spin, EINA_TRUE);
   efl_ui_focus_util_focus(efl_content_get(efl_part(spin, "efl.text_button")));
   get_me_to_those_events(spin);
   entry = efl_content_get(efl_part(spin, "efl.entry"));
   ck_assert_ptr_ne(entry, NULL);
   efl_ui_focus_util_focus(entry);
   get_me_to_those_events(spin);
   return entry;
}

static void
_leave_entry(Eo *spin)
{
   efl_ui_focus_util_focus(efl_content_get(efl_part(spin, "efl.inc_button")));
   get_me_to_those_events(spin);
}

EFL_START_TEST(spin_button_entry_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("count", eina_value_double_new(3.0));
   Eo *spin = _spin_button(win, model);
   Eo *entry;

   get_me_to_those_events(spin);
   entry = _open_entry(spin);
   elm_object_text_set(entry, "");
   write_key_sequence(spin, "8");
   get_me_to_those_events(spin);
   _leave_entry(spin);
   ck_assert_msg(EINA_DBL_EQ(efl_ui_range_value_get(spin), 8.0), "value %f entry %s",
                 efl_ui_range_value_get(spin), elm_object_text_get(entry));
   ck_assert(EINA_DBL_EQ(_model_double(model, "count"), 8.0));
}
EFL_END_TEST

EFL_START_TEST(spin_button_stale_entry_is_not_written_back)
{
   Eo *win = _window();
   Eo *model = _model_new("count", eina_value_double_new(3.0));
   Eo *spin = _spin_button(win, model);

   get_me_to_those_events(spin);
   _open_entry(spin);
   // The model moves on while the entry shows the value it opened with.
   _model_set(model, "count", eina_value_double_new(7.0));
   ecore_main_loop_iterate();
   _leave_entry(spin);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(spin), 7.0));
   ck_assert(EINA_DBL_EQ(_model_double(model, "count"), 7.0));
}
EFL_END_TEST

EFL_START_TEST(radio_click_reaches_the_model)
{
   Eo *win = _window();
   Eo *model = _model_new("on", eina_value_bool_new(EINA_FALSE));
   Eo *radio;
   Eina_Value *v;
   Eina_Bool b = EINA_FALSE;

   radio = efl_add(EFL_UI_RADIO_CLASS, win,
                   efl_ui_view_model_set(efl_added, model),
                   efl_ui_property_bind(efl_added, "selected", "on"),
                   efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(100, 50)));
   efl_gfx_entity_visible_set(radio, EINA_TRUE);

   click_object(radio);
   ecore_main_loop_iterate();
   ck_assert(efl_ui_selectable_selected_get(radio));
   v = _model_get(model, "on");
   eina_value_bool_get(v, &b);
   eina_value_free(v);
   ck_assert(b);
}
EFL_END_TEST

/* A widget that finds its model through a provider follows the provider to a
 * new model, and then hears that model change as it heard the first. */
EFL_START_TEST(efl_ui_model_provider_live_edit)
{
   Eo *win = _window();
   Eo *m1 = _model_new("t", eina_value_string_new("one"));
   Eo *m2 = _model_new("t", eina_value_string_new("two"));
   Eo *box, *provider, *label;

   box = efl_add(EFL_UI_BOX_CLASS, win);
   provider = efl_add(EFL_MODEL_PROVIDER_CLASS, box);
   efl_provider_register(box, EFL_MODEL_PROVIDER_CLASS, provider);
   label = efl_add(EFL_UI_TEXTBOX_CLASS, box,
                   efl_ui_property_bind(efl_added, "text", "t"));
   efl_pack_end(box, label);
   efl_content_set(win, box);

   efl_ui_view_model_set(provider, m1);
   ck_assert_str_eq(efl_text_get(label), "one");
   _model_set(m1, "t", eina_value_string_new("one again"));
   ck_assert_str_eq(efl_text_get(label), "one again");

   // The provider moves on to another model
   efl_ui_view_model_set(provider, m2);
   ck_assert_str_eq(efl_text_get(label), "two");

   _model_set(m2, "t", eina_value_string_new("live edit"));
   ck_assert_str_eq(efl_text_get(label), "live edit");

   // What the old model does is not the label's business any more
   _model_set(m1, "t", eina_value_string_new("stale"));
   ck_assert_str_eq(efl_text_get(label), "live edit");

   efl_unref(m1);
   efl_unref(m2);
}
EFL_END_TEST

/* A factory binds every item again each time it hands one out, reused or
 * not. Binding the same key again replaces the binding: nothing piles up. */
EFL_START_TEST(rebinding_a_key_replaces_the_binding)
{
   Eo *win = _window();
   Eo *model = _model_new("t", eina_value_string_new("first"));
   Eo *txt;
   Efl_Ui_Widget_Data *pd;
   int i;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                 efl_ui_view_model_set(efl_added, model));
   for (i = 0; i < 6; i++)
     ck_assert_int_eq(efl_ui_property_bind(txt, "text", "t"), 0);

   pd = efl_data_scope_get(txt, EFL_UI_WIDGET_CLASS);
   ck_assert_int_eq(eina_hash_population(pd->properties.model_lookup), 1);
   ck_assert_int_eq(eina_hash_population(pd->properties.view_lookup), 1);
   ck_assert_str_eq(efl_text_get(txt), "first");

   _model_set(model, "t", eina_value_string_new("second"));
   ck_assert_str_eq(efl_text_get(txt), "second");
   efl_unref(model);
}
EFL_END_TEST

void
efl_ui_test_property_bind(TCase *tc)
{
   tcase_add_test(tc, textbox_user_edit_reaches_the_model);
   tcase_add_test(tc, efl_ui_model_provider_live_edit);
   tcase_add_test(tc, rebinding_a_key_replaces_the_binding);
   tcase_add_test(tc, textbox_model_change_does_not_loop);
   tcase_add_test(tc, textbox_typing_in_the_middle_keeps_the_cursor);
   tcase_add_test(tc, textbox_typing_at_the_start_keeps_the_cursor);
   tcase_add_test(tc, textbox_typing_keeps_the_selection);
   tcase_add_test(tc, textbox_cut_reaches_the_model);
   tcase_add_test(tc, textbox_paste_reaches_the_model);
   tcase_add_test(tc, part_binding_does_not_swallow_the_widget_write_back);
   tcase_add_test(tc, check_toggle_style_emits_once);
   tcase_add_test(tc, slider_refused_value_does_not_reach_the_model);
   tcase_add_test(tc, spin_button_model_change_does_not_loop);
   tcase_add_test(tc, spin_button_entry_reaches_the_model);
   tcase_add_test(tc, spin_button_stale_entry_is_not_written_back);
   tcase_add_test(tc, radio_click_reaches_the_model);
   tcase_add_test(tc, check_click_reaches_the_model);
   tcase_add_test(tc, check_model_change_does_not_loop);
   tcase_add_test(tc, slider_key_reaches_the_model);
   tcase_add_test(tc, slider_model_change_does_not_loop);
   tcase_add_test(tc, spin_button_wheel_reaches_the_model);
}
