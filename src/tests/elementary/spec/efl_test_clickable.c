#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Clickable",
       "test-widgets": ["Efl.Ui.Button", "Efl.Ui.Image", "Efl.Ui.Panes", "Efl.Ui.Frame"]
       }
   spec-meta-end
 */

/*
 * Here follows a fixture that ensures that the window is correctly rendered, and the mouse is moved into the middle of the window.
 */

static void
emit_mouse_events(void *data)
{
   Eina_Position2D *pos = data;
   Evas *e = evas_object_evas_get(win);
   evas_event_feed_mouse_in(e, 0, NULL);
   evas_event_feed_mouse_move(e, pos->x, pos->y, 0, NULL);
   efl_task_end(efl_app_main_get());
   free(pos);
}

static void
prepare_window_norendered(void *data, Evas *e, void *event_info EINA_UNUSED)
{
   ecore_job_add(emit_mouse_events, data);
   evas_event_callback_del(e, EVAS_CALLBACK_RENDER_POST, prepare_window_norendered);
}

static void
prepare_window(void)
{
   Eina_Position2D *pos = calloc(1, sizeof(Eina_Position2D));
   ck_assert_ptr_ne(win, NULL);
   ck_assert_ptr_ne(widget, NULL);
   efl_gfx_entity_geometry_set(win, EINA_RECT(0, 0, 60, 60));
   pos->x = 30;
   pos->y = 30;

   if (efl_isa(widget, EFL_UI_IMAGE_CLASS))
     {
        efl_gfx_hint_size_min_set(widget, EINA_SIZE2D(200, 200));
        efl_file_simple_load(widget, ELM_IMAGE_DATA_DIR"/images/bubble.png", NULL);
        pos->x = 100;
        pos->y = 100;
     }
   else if (efl_isa(widget, EFL_UI_FRAME_CLASS))
     {
        efl_text_set(widget, "Test, here has to be text in order to make the frame y > 0");
        pos->x = 30;
        pos->y = 10;
     }

   evas_smart_objects_calculate(evas_object_evas_get(win));
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_RENDER_POST, prepare_window_norendered, pos);
   efl_loop_begin(efl_app_main_get());
}

/*
 * General helpers for emitting and checking mouse events.
 */

typedef struct {
   Efl_Ui_Clickable_Clicked clicked_params;
   unsigned int clicked;
   Efl_Ui_Clickable_Clicked clicked_all_params;
   unsigned int clicked_all;
   unsigned int pressed;
   unsigned int unpressed;
   unsigned int long_pressed;
   unsigned int repeated;
} Clickable_Event_Register;

Clickable_Event_Register event_caller = { 0 };

static void
_event_register(void *data EINA_UNUSED, const Efl_Event *ev)
{
#define EVENT_CHECK(e,f)   if (ev->desc == EFL_UI_EVENT_ ##e ) event_caller.f ++
  EVENT_CHECK(CLICKED, clicked);
  EVENT_CHECK(CLICKED_ANY, clicked_all);
  EVENT_CHECK(PRESSED, pressed);
  EVENT_CHECK(UNPRESSED, unpressed);
  EVENT_CHECK(LONGPRESSED, long_pressed);

  if (ev->desc == EFL_UI_EVENT_CLICKED)
    {
       Efl_Ui_Clickable_Clicked *clicked = ev->info;

       event_caller.clicked_params.repeated = clicked->repeated;
       event_caller.clicked_params.button = clicked->button;
    }
  if (ev->desc == EFL_UI_EVENT_CLICKED_ANY)
    {
       Efl_Ui_Clickable_Clicked *clicked = ev->info;

       event_caller.clicked_all_params.repeated = clicked->repeated;
       event_caller.clicked_all_params.button = clicked->button;
    }
}

EFL_CALLBACKS_ARRAY_DEFINE(clickable,
  {EFL_UI_EVENT_CLICKED, _event_register},
  {EFL_UI_EVENT_CLICKED_ANY, _event_register},
  {EFL_UI_EVENT_PRESSED, _event_register},
  {EFL_UI_EVENT_UNPRESSED, _event_register},
  {EFL_UI_EVENT_LONGPRESSED, _event_register},
)

static void
assert_event_empty(void)
{
   ck_assert_int_eq(event_caller.clicked, 0);
   ck_assert_int_eq(event_caller.clicked_all, 0);
   ck_assert_int_eq(event_caller.pressed, 0);
   ck_assert_int_eq(event_caller.unpressed, 0);
   ck_assert_int_eq(event_caller.long_pressed, 0);
   ck_assert_int_eq(event_caller.repeated, 0);
}

static void
_timer_expired(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_del(ev->object);
   efl_task_end(efl_app_main_get());
}

static void
iterate_mainloop(double interval)
{
  efl_add(EFL_LOOP_TIMER_CLASS, efl_app_main_get(),
    efl_loop_timer_interval_set(efl_added, interval),
    efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _timer_expired, NULL));
  efl_loop_begin(efl_app_main_get());
}

static void
down(int btn)
{
   Evas *e = evas_object_evas_get(win);
   evas_event_feed_mouse_down(e, btn, 0, 0, NULL);
}

static void
up(int btn)
{
   Evas *e = evas_object_evas_get(win);
   evas_event_feed_mouse_up(e, btn, 0, 0, NULL);
}

EFL_START_TEST(simple_left_down_up)
{
   efl_event_callback_array_add(widget, clickable(), NULL);
   down(1);
   iterate_mainloop(0.1);
   ck_assert_int_eq(event_caller.pressed, 1);
   event_caller.pressed = 0;
   assert_event_empty();

   up(1);
   iterate_mainloop(0.1);
   ck_assert_int_eq(event_caller.clicked, 1);
   event_caller.clicked = 0;
   ck_assert_int_eq(event_caller.clicked_params.repeated, 0);
   ck_assert_int_eq(event_caller.clicked_params.button, 1);
   ck_assert_int_eq(event_caller.clicked_all, 1);
   event_caller.clicked_all = 0;
   ck_assert_int_eq(event_caller.clicked_all_params.repeated, 0);
   ck_assert_int_eq(event_caller.clicked_all_params.button, 1);
   ck_assert_int_eq(event_caller.unpressed, 1);
   event_caller.unpressed = 0;
   assert_event_empty();
}
EFL_END_TEST

EFL_START_TEST(long_press_event)
{
   efl_event_callback_array_add(widget, clickable(), NULL);
   down(1);
   iterate_mainloop(0.1);
   ck_assert_int_eq(event_caller.pressed, 1);
   event_caller.pressed = 0;
   assert_event_empty();
   iterate_mainloop(2.0);
   ck_assert_int_eq(event_caller.long_pressed, 1);
   event_caller.long_pressed = 0;
   assert_event_empty();
   up(1);
   iterate_mainloop(0.1);
   ck_assert_int_eq(event_caller.clicked, 1);
   event_caller.clicked = 0;
   ck_assert_int_eq(event_caller.clicked_params.repeated, 0);
   ck_assert_int_eq(event_caller.clicked_params.button, 1);
   ck_assert_int_eq(event_caller.clicked_all, 1);
   event_caller.clicked_all = 0;
   ck_assert_int_eq(event_caller.clicked_all_params.repeated, 0);
   ck_assert_int_eq(event_caller.clicked_all_params.button, 1);
   ck_assert_int_eq(event_caller.unpressed, 1);
   event_caller.unpressed = 0;
   assert_event_empty();
}
EFL_END_TEST

EFL_START_TEST(repeated_event)
{
   efl_event_callback_array_add(widget, clickable(), NULL);

   for (int i = 0; i < 20; ++i)
     {
        down(1);
        iterate_mainloop(0.01);
        ck_assert_int_eq(event_caller.pressed, 1);
        event_caller.pressed = 0;
        assert_event_empty();

        up(1);
        iterate_mainloop(0.01);
        ck_assert_int_eq(event_caller.clicked, 1);
        event_caller.clicked = 0;
        ck_assert_int_eq(event_caller.clicked_params.repeated, i);
        ck_assert_int_eq(event_caller.clicked_params.button, 1);
        ck_assert_int_eq(event_caller.clicked_all, 1);
        event_caller.clicked_all = 0;
        ck_assert_int_eq(event_caller.clicked_all_params.repeated, i);
        ck_assert_int_eq(event_caller.clicked_all_params.button, 1);
        ck_assert_int_eq(event_caller.unpressed, 1);
        event_caller.unpressed = 0;
     }
}
EFL_END_TEST

void
efl_ui_clickable_behavior_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, prepare_window, NULL);
   tcase_add_test(tc, simple_left_down_up);
   tcase_add_test(tc, long_press_event);
   tcase_add_test(tc, repeated_event);
}
