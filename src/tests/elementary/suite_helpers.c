#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efl_Ui.h>
#include <check.h>
#include "eo_internal.h"
#include "../efl_check.h"
#include "elm_widget.h"
#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "suite_helpers.h"


static int main_pid = -1;
static Eina_Bool did_shutdown;
static Evas_Object *global_win;
static Eina_Bool buffer = EINA_FALSE;
static Eina_Bool legacy_mode = EINA_FALSE;
static int log_abort;
static int log_abort_level;
static unsigned int ts = 1;

Eina_Bool abort_on_warnings = EINA_FALSE;

void elm_test_init(TCase *tc);

static char *args[] = { "exe" };

SUITE_INIT(elm)
{
   ck_assert_int_eq(elm_init(1, args), 1);
}

void
_elm2_suite_init(void)
{
   if (is_forked())
     {
        if (abort_on_warnings)
          fail_on_errors_setup();
        ecore_fork_reset();
        return;
     }
   if (!did_shutdown)
     {
        /* if running un-forked then remove cached init */
        ck_assert_int_eq(elm_shutdown(), 0);
        did_shutdown = 1;
     }
   ck_assert_int_eq(elm_init(1, args), 1);
   if (abort_on_warnings)
     fail_on_errors_setup();
   ts = 1;
}

void
_elm_suite_shutdown(void)
{
   ck_assert_int_eq(elm_shutdown(), 0);
   /* verify that ecore was de-initialized completely */
   ck_assert_int_eq(ecore_init(), 1);
   /* avoid slowdowns in fork mode */
   if (is_forked()) return;
   ck_assert_int_eq(ecore_shutdown(), 0);
}


static const Efl_Test_Case etc_init[] = {
  { "init", elm_test_init },
  { NULL, NULL }
};


EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   Efl_Loop_Arguments *arge = ev->info;


   fail_if(!arge->initialization);
   fprintf(stderr, "ARGC %d\n", eina_array_count(arge->argv));
   fail_if(eina_array_count(arge->argv) != 2);
   fail_if(!eina_streq(eina_array_data_get(arge->argv, 1), "test"));

   efl_loop_quit(ev->object, eina_value_string_init("success"));
}

EFL_START_TEST(efl_ui_test_init)
{
   /* EFL_MAIN */
   Eina_Value *ret__;
   int real__;

   int argc = 2;
   char *argv[] = { "efl_ui_suite", "test" };
   _efl_startup_time = ecore_time_unix_get();
   _EFL_APP_VERSION_SET();
   fail_if(!ecore_init());
   efl_event_callback_add(efl_app_main_get(), EFL_LOOP_EVENT_ARGUMENTS, efl_main, NULL);
   fail_if(!ecore_init_ex(argc, argv));
   __EFL_MAIN_CONSTRUCTOR;
   ret__ = efl_loop_begin(efl_app_main_get());
   real__ = efl_loop_exit_code_process(ret__);
   fail_if(real__ != 0);
   __EFL_MAIN_DESTRUCTOR;
   ecore_shutdown_ex();
   ecore_shutdown();
}
EFL_END_TEST

void efl_ui_init(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_init);
}


static const Efl_Test_Case ui_init[] = {
  { "init", efl_ui_init },
  { NULL, NULL }
};

#undef ecore_timer_add
#define BUFFER_RENDER_INTERVAL 0.005
#define LOOP_INCREMENT 0.1
#define TIMER_SCALE ((1.0 / BUFFER_RENDER_INTERVAL) * LOOP_INCREMENT)

void
loop_timer_interval_set(Eo *obj, double in)
{
   efl_loop_timer_interval_set(obj, in * TIMER_SCALE);
}

Eo *
timer_add(double in, Ecore_Task_Cb cb, void *data)
{
   return ecore_timer_add(in * TIMER_SCALE, cb, data);
}

void *
real_timer_add(double in, Ecore_Task_Cb cb, void *data)
{
   return ecore_timer_add(in, cb, data);
}

static Eina_Bool
_timer_quit()
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

void
wait_timer(double in)
{
   ecore_timer_add(in, _timer_quit, NULL);
   ecore_main_loop_begin();
}

static void
_win_manual_render(void *data, const Efl_Event *event EINA_UNUSED)
 {
   double t = ecore_loop_time_get();

   ecore_loop_time_set(t + LOOP_INCREMENT);
   ecore_animator_custom_tick();
   ecore_evas_render_prepare(ecore_evas_ecore_evas_get(evas_object_evas_get(data)));
   evas_norender_with_updates(evas_object_evas_get(data));
}

static void
_loop_iterate(void *data, const Efl_Event *event EINA_UNUSED)
{
   efl_loop_iterate(data);
}

static void
_win_show(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eo *timer = evas_object_data_get(obj, "timer");
   efl_event_thaw(timer);
   efl_event_callback_add(efl_loop_get(obj), EFL_LOOP_EVENT_IDLE, _loop_iterate, efl_parent_get(timer));
}

static void
_win_hide(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eo *timer = evas_object_data_get(obj, "timer");
   efl_event_freeze(timer);
   efl_loop_timer_reset(timer);
   efl_event_callback_del(efl_loop_get(obj), EFL_LOOP_EVENT_IDLE, _loop_iterate, efl_parent_get(timer));
}

static void
_ui_win_visibility_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Bool *visible = ev->info;
   if (*visible)
     _win_show(NULL, NULL, ev->object, NULL);
   else
     {
        _win_hide(NULL, NULL, ev->object, NULL);
        efl_key_data_set(ev->object, "timer", NULL);
     }
}

EFL_CLASS_SIMPLE_CLASS(efl_loop, "Efl.Loop", EFL_LOOP_CLASS)

static Evas_Object *
_elm_suite_win_create(void)
{
   Evas_Object *win;
   Eo *loop, *timer;

   if (legacy_mode)
     win = elm_win_add(NULL, "elm_suite", ELM_WIN_BASIC);
   else
     win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get());
   if (!buffer) return win;
   loop = efl_add(efl_loop_realized_class_get(), win);
   timer = efl_add(EFL_LOOP_TIMER_CLASS, loop,
     efl_loop_timer_interval_set(efl_added, BUFFER_RENDER_INTERVAL),
     efl_event_freeze(efl_added),
     efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _win_manual_render, win)
     );
   evas_object_data_set(win, "timer", timer);
   ecore_evas_manual_render_set(ecore_evas_ecore_evas_get(evas_object_evas_get(win)), EINA_TRUE);
   edje_frametime_set(BUFFER_RENDER_INTERVAL);
   ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
   if (legacy_mode)
     {
        evas_object_event_callback_add(win, EVAS_CALLBACK_SHOW, _win_show, NULL);
        evas_object_event_callback_add(win, EVAS_CALLBACK_HIDE, _win_hide, NULL);
     }
   else
     {
        efl_event_callback_add(win, EFL_GFX_ENTITY_EVENT_VISIBILITY_CHANGED, _ui_win_visibility_change, NULL);
     }
   return win;
}

#define TEST_FONT_DIR TESTS_SRC_DIR "/fonts/"

Evas_Object *
win_add(void)
{
   static Eina_Bool font_path = EINA_FALSE;

   if (!font_path)
     {
        evas_font_path_global_append(TEST_FONT_DIR);
        font_path = EINA_TRUE;
     }
   if (is_forked())
     {
        if (global_win) return global_win;
     }
   return _elm_suite_win_create();
}

static void
force_focus_win(Evas_Object *win)
{
   Ecore_Evas *ee;

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));
   ecore_evas_focus_set(ee, EINA_TRUE);
   ecore_evas_callback_focus_in_set(ee, NULL);
   ecore_evas_callback_focus_out_set(ee, NULL);
   Elm_Widget_Smart_Data *pd = efl_data_scope_safe_get(win, EFL_UI_WIDGET_CLASS);
   pd->top_win_focused = EINA_TRUE;
}

Evas_Object *
win_add_focused()
{
   Evas_Object *win;

   if (is_forked())
     {
        if (global_win) return global_win;
     }

   win = _elm_suite_win_create();
   force_focus_win(win);
   return win;
}

Eina_Bool
is_forked(void)
{
   return getpid() != main_pid;
}

Eina_Bool
is_buffer(void)
{
   return buffer;
}

static void (*suite_setup_cb)(Eo*);

void
suite_setup_cb_set(void (*cb)(Eo*))
{
   suite_setup_cb = cb;
}

int
suite_setup(Eina_Bool legacy)
{
   int failed_count;

   legacy_mode = legacy;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif
   if (!getenv("ELM_ENGINE"))
     putenv("ELM_ENGINE=buffer");
   if (eina_streq(getenv("ELM_ENGINE"), "buffer"))
     {
        putenv("TESTS_GL_DISABLED=1");
        buffer = EINA_TRUE;
     }

   main_pid = getpid();
   ecore_app_no_system_modules();
   efreet_cache_disable();

   if (legacy)
     failed_count = _efl_suite_build_and_run(0, NULL,
                                             "Elementary_Init", etc_init, SUITE_INIT_FN(elm), SUITE_SHUTDOWN_FN(elm));
   else
     failed_count = _efl_suite_build_and_run(0, NULL,
                                             "Efl_Ui_Init", ui_init, NULL, NULL);

   failed_count += !elm_init(1, args);
   if (buffer)
     {
        global_win = _elm_suite_win_create();
        force_focus_win(global_win);
        if (suite_setup_cb) suite_setup_cb(global_win);
     }
   EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
   /* preload default theme */
   if (legacy)
     {
        failed_count += !elm_theme_group_path_find(NULL, "elm/button/base/default");
        EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
        failed_count += !elm_theme_group_path_find(NULL, "elm/border/base/default");
        EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
        failed_count += !elm_theme_group_path_find(NULL, "elm/win/base/default");
        EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
     }
   else
     {
        failed_count += !elm_theme_group_path_find(NULL, "efl/button");
        EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
        failed_count += !elm_theme_group_path_find(NULL, "efl/border");
        EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
        failed_count += !elm_theme_group_path_find(NULL, "efl/win");
        EINA_SAFETY_ON_TRUE_RETURN_VAL(failed_count, 255);
     }

   /* none of these will be found in the default theme,
    * but all are "fetched" hundreds of times
    * T6865
    */
   if (legacy)
     {
        elm_theme_group_path_find(NULL, "elm/cursor/top_side/default");
        elm_theme_group_path_find(NULL, "elm/cursor/bottom_side/default");
        elm_theme_group_path_find(NULL, "elm/cursor/top_left_corner/default");
        elm_theme_group_path_find(NULL, "elm/cursor/bottom_left_corner/default");
        elm_theme_group_path_find(NULL, "elm/cursor/top_right_corner/default");
        elm_theme_group_path_find(NULL, "elm/cursor/bottom_right_corner/default");


        elm_theme_group_path_find(NULL, "elm/colorselector/item/color/default");
     }
   else
     {
        elm_theme_group_path_find(NULL, "efl/cursor/top_side");
        elm_theme_group_path_find(NULL, "efl/cursor/bottom_side");
        elm_theme_group_path_find(NULL, "efl/cursor/top_left_corner");
        elm_theme_group_path_find(NULL, "efl/cursor/bottom_left_corner");
        elm_theme_group_path_find(NULL, "efl/cursor/top_right_corner");
        elm_theme_group_path_find(NULL, "efl/cursor/bottom_right_corner");


        elm_theme_group_path_find(NULL, "efl/colorselector/item/color");
     }
   return 0;
}

void
fail_on_errors_teardown(void)
{
   eina_log_abort_on_critical_set(log_abort);
   eina_log_abort_on_critical_level_set(log_abort_level);
}

void
fail_on_errors_setup(void)
{
   log_abort = eina_log_abort_on_critical_get();
   log_abort_level = eina_log_abort_on_critical_level_get();
   eina_log_abort_on_critical_level_set(2);
   eina_log_abort_on_critical_set(1);
}

static void
next_event_job()
{
   ecore_main_loop_quit();
}

static void
events_norendered(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   evas_event_callback_del(e, EVAS_CALLBACK_RENDER_POST, events_norendered);
   ecore_job_add(next_event_job, NULL);
}

void
get_me_to_those_events(Eo *obj)
{
   Evas *e = obj;

   if ((!efl_isa(obj, EFL_CANVAS_SCENE_INTERFACE)) || efl_isa(obj, EFL_UI_WIN_CLASS))
     e = evas_object_evas_get(obj);
   evas_smart_objects_calculate(e);
   evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, events_norendered, NULL);
   ecore_main_loop_begin();
}

enum
{
   NONE = 0,
   LEFT = 1 << 0,
   RIGHT = 1 << 1,
   TOP = 1 << 2,
   BOTTOM = 1 << 3,
};

static Eina_Position2D
attempt_to_find_the_right_point_for_mouse_positioning(Eo *obj, int dir)
{
   int x, y;
   Eina_Rect r = efl_gfx_entity_geometry_get(obj);
   if (dir & LEFT)
     x = r.x + (.1 * r.w);
   else if (dir & RIGHT)
     x = r.x + (.9 * r.w);
   else
     x = r.x + r.w / 2;
   if (dir & TOP)
     y = r.y + (.1 * r.h);
   else if (dir & BOTTOM)
     y = r.y + (.9 * r.h);
   else
     y = r.y + r.h / 2;
   return EINA_POSITION2D(x, y);
}

static void
click_object_internal(Eo *obj, int dir, int flags, Eina_Bool up)
{
   Evas *e = evas_object_evas_get(obj);
   Eina_Position2D pos = attempt_to_find_the_right_point_for_mouse_positioning(obj, dir);
   evas_event_feed_mouse_move(e, pos.x, pos.y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, flags, ts++, NULL);
   if (up)
     evas_event_feed_mouse_up(e, 1, 0, ts++, NULL);
}

void
click_object(Eo *obj)
{
   click_object_internal(obj, NONE, 0, EINA_TRUE);
}

void
press_object(Eo *obj)
{
   click_object_internal(obj, NONE, 0, EINA_FALSE);
}

void
click_object_flags(Eo *obj, int flags)
{
   click_object_internal(obj, NONE, flags, EINA_TRUE);
}

void
press_object_flags(Eo *obj, int flags)
{
   click_object_internal(obj, NONE, flags, EINA_FALSE);
}

static void
click_part_flags_internal(Eo *obj, const char *part, int flags, Eina_Bool up)
{
   Efl_Part *part_obj = efl_ref(efl_part(obj, part));
   Eo *content;
   int dir = 0;

   if (efl_canvas_layout_part_type_get(part_obj) == EFL_CANVAS_LAYOUT_PART_TYPE_SWALLOW)
     content = efl_content_get(part_obj);
   else
     {
        content = part_obj;
        if (strstr(part, "left"))
          dir |= LEFT;
        else if (strstr(part, "right"))
          dir |= RIGHT;
        if (strstr(part, "top"))
          dir |= TOP;
        else if (strstr(part, "bottom"))
          dir |= BOTTOM;
     }
   click_object_internal(content, dir, flags, up);
   if (efl_isa(content, EFL_LAYOUT_SIGNAL_INTERFACE))
     edje_object_message_signal_process(content);
   /* can be false if obj is an elm_widget_item */
   if (efl_isa(obj, EFL_LAYOUT_SIGNAL_INTERFACE))
     edje_object_message_signal_process(obj);
   efl_unref(part_obj);
}

void
click_part_flags(Eo *obj, const char *part, int flags)
{
   click_part_flags_internal(obj, part, flags, EINA_TRUE);
}

void
press_part_flags(Eo *obj, const char *part, int flags)
{
   click_part_flags_internal(obj, part, flags, EINA_FALSE);
}

void
click_part(Eo *obj, const char *part)
{
   click_part_flags(obj, part, 0);
}

void
press_part(Eo *obj, const char *part)
{
   press_part_flags(obj, part, 0);
}

static void
wheel_object_internal(Eo *obj, int dir, Eina_Bool horiz, Eina_Bool down)
{
   Eina_Position2D pos = attempt_to_find_the_right_point_for_mouse_positioning(obj, dir);
   wheel_object_at(obj, pos.x, pos.y, horiz, down);
}

void
wheel_object(Eo *obj, Eina_Bool horiz, Eina_Bool down)
{
   wheel_object_internal(obj, NONE, horiz, down);
}

void
wheel_part(Eo *obj, const char *part, Eina_Bool horiz, Eina_Bool down)
{
   Efl_Part *part_obj = efl_ref(efl_part(obj, part));
   Eo *content;
   int dir = 0;

   if (efl_canvas_layout_part_type_get(part_obj) == EFL_CANVAS_LAYOUT_PART_TYPE_SWALLOW)
     content = efl_content_get(part_obj);
   else
     {
        content = part_obj;
        if (strstr(part, "left"))
          dir |= LEFT;
        else if (strstr(part, "right"))
          dir |= RIGHT;
        if (strstr(part, "top"))
          dir |= TOP;
        else if (strstr(part, "bottom"))
          dir |= BOTTOM;
     }
   wheel_object_internal(content, dir, horiz, down);
   if (efl_isa(content, EFL_LAYOUT_SIGNAL_INTERFACE))
     edje_object_message_signal_process(content);
   edje_object_message_signal_process(obj);
   efl_unref(part_obj);
}

void
event_callback_single_call_int_data(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int *called = data;

   if (*called) fprintf(stderr, "TEST FAILURE IN %s\n", __func__);
   ck_assert_int_eq(*called, 0);
   *called = 1;
}

void
event_callback_that_increments_an_int_when_called(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int *called = data;

   *called += 1;
}

void
event_callback_that_quits_the_main_loop_when_called()
{
   ecore_main_loop_quit();
}

void
smart_callback_that_stores_event_info_to_data(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   void **real_data = data;

   *real_data = event_info;
}

void
click_object_at(Eo *obj, int x, int y)
{
   Evas *e = evas_object_evas_get(obj);
   evas_event_feed_mouse_move(e, x, y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, 0, ts++, NULL);
   evas_event_feed_mouse_up(e, 1, 0, ts++, NULL);
}

void
multi_click_object(Eo *obj, int ids)
{
   Evas *e = evas_object_evas_get(obj);
   Eina_Position2D pos = attempt_to_find_the_right_point_for_mouse_positioning(obj, NONE);
   for (int i = 0; i < ids; i++)
     evas_event_feed_multi_down(e, i, pos.x + i, pos.y + i, 1, 1, 1, 1, 0, pos.x + i, pos.y + i, 0, ts, NULL);
   ts++;
   for (int i = 0; i < ids; i++)
     evas_event_feed_multi_up(e, i, pos.x + i, pos.y + i, 1, 1, 1, 1, 0, pos.x + i, pos.y + i, 0, ts, NULL);
}

void
multi_press_object(Eo *obj, int ids)
{
   Evas *e = evas_object_evas_get(obj);
   Eina_Position2D pos = attempt_to_find_the_right_point_for_mouse_positioning(obj, NONE);
   for (int i = 0; i < ids; i++)
     evas_event_feed_multi_down(e, i, pos.x + i, pos.y + i, 1, 1, 1, 1, 0, pos.x + i, pos.y + i, 0, ts, NULL);
   ts++;
}

void
multi_click_object_at(Eo *obj, int x, int y, int ids)
{
   Evas *e = evas_object_evas_get(obj);
   for (int i = 0; i < ids; i++)
     evas_event_feed_multi_down(e, i, x + i, y + i, 1, 1, 1, 1, 0, x + i, y + i, 0, ts, NULL);
   ts++;
   for (int i = 0; i < ids; i++)
     evas_event_feed_multi_up(e, i, x + i, y + i, 1, 1, 1, 1, 0, x + i, y + i, 0, ts, NULL);
   ts++;
}

void
press_object_at(Eo *obj, int x, int y)
{
   Evas *e = evas_object_evas_get(obj);
   evas_event_feed_mouse_move(e, x, y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, 0, ts++, NULL);
}

void
multi_press_object_at(Eo *obj, int x, int y, int ids)
{
   Evas *e = evas_object_evas_get(obj);
   for (int i = 0; i < ids; i++)
     evas_event_feed_multi_down(e, i, x + i, y + i, 1, 1, 1, 1, 0, x + i, y + i, 0, ts, NULL);
   ts++;
}

void
click_object_at_flags(Eo *obj, int x, int y, int flags)
{
   Evas *e = evas_object_evas_get(obj);
   evas_event_feed_mouse_move(e, x, y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, flags, ts++, NULL);
   evas_event_feed_mouse_up(e, 1, 0, ts++, NULL);
}

void
press_object_at_flags(Eo *obj, int x, int y, int flags)
{
   Evas *e = evas_object_evas_get(obj);
   evas_event_feed_mouse_move(e, x, y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, flags, ts++, NULL);
}

void
wheel_object_at(Eo *obj, int x, int y, Eina_Bool horiz, Eina_Bool down)
{
   Evas *e = evas_object_evas_get(obj);
   evas_event_feed_mouse_move(e, x, y, 0, NULL);
   evas_event_feed_mouse_wheel(e, horiz, down, 0, NULL);
}

void
drag_object(Eo *obj, int x, int y, int dx, int dy, Eina_Bool iterate)
{
   Evas *e = evas_object_evas_get(obj);
   int i;
   evas_event_feed_mouse_move(e, x, y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, 0, ts++, NULL);
   if (iterate)
     {
        /* iterate twice to trigger timers */
        ecore_main_loop_iterate();
        ecore_main_loop_iterate();
      }
   /* create DRAG_OBJECT_NUM_MOVES move events distinct from up/down */
   for (i = 0; i < DRAG_OBJECT_NUM_MOVES; i++)
     {
        evas_event_feed_mouse_move(e, x + (i * dx / DRAG_OBJECT_NUM_MOVES), y + (i * dy / DRAG_OBJECT_NUM_MOVES), ts++, NULL);
        /* also trigger smart calc if we're iterating just in case that's important */
        evas_smart_objects_calculate(e);
     }
   evas_event_feed_mouse_move(e, x + dx, y + dy, ts++, NULL);
   evas_event_feed_mouse_up(e, 1, 0, ts++, NULL);
}

int
drag_object_around(Eo *obj, int cx, int cy, int radius, int degrees)
{
   Evas *e = evas_object_evas_get(obj);
   /* clamp num mouse moves to a vaguely sane value */
   int i, num = MIN(abs(degrees), DRAG_OBJECT_AROUND_NUM_MOVES);
   int last_x = round(cx + radius);
   int last_y = round(cy);
   /* start at 0 degrees */
   evas_event_feed_mouse_move(e, last_x, last_y, ts++, NULL);
   evas_event_feed_mouse_down(e, 1, 0, ts++, NULL);
   for (i = 1; i < num; i++)
     {
        /* x = cx + r * cos(a), y = cy + r * sin(a) */
        int ax, ay;
        /* each iteration is 1 degree */
        double angle = (i * (degrees / DRAG_OBJECT_AROUND_NUM_MOVES)) * M_PI / 180.0;
        ax = round(cx + radius * cos(angle));
        ay = round(cy + radius * sin(angle));
        if ((ax == last_x) && (ay == last_y)) continue;
        evas_event_feed_mouse_move(e, ax, ay, ts++, NULL);
        last_x = ax, last_y = ay;
     }
   evas_event_feed_mouse_up(e, 1, 0, ts++, NULL);
   /* only count arc motion: subtract initial move, mouse down, mouse up */
   return num;
}

int
multi_drag_object_around(Eo *obj, int touch_point, int cx, int cy, int radius, int degrees)
{
   Evas *e = evas_object_evas_get(obj);
   /* clamp num mouse moves to a vaguely sane value */
   int i, num = MIN(abs(degrees), DRAG_OBJECT_AROUND_NUM_MOVES);
   int last_x = round(cx + radius);
   int last_y = round(cy);
   /* start at 0 degrees */
   evas_event_feed_multi_down(e, touch_point, last_x, last_y, 1, 1, 1, 1, 0, last_x, last_y, 0, ts++, NULL);
   for (i = 1; i < num; i++)
     {
        /* x = cx + r * cos(a), y = cy + r * sin(a) */
        int ax, ay;
        /* each iteration is 1 degree */
        double angle = (i * (degrees / DRAG_OBJECT_AROUND_NUM_MOVES)) * M_PI / 180.0;
        ax = round(cx + radius * cos(angle));
        ay = round(cy + radius * sin(angle));
        if ((ax == last_x) && (ay == last_y)) continue;
        evas_event_feed_multi_move(e, touch_point, ax, ay, 1, 1, 1, 1, 0, ax, ay, ts++, NULL);
        last_x = ax, last_y = ay;
     }
   evas_event_feed_multi_up(e, touch_point, last_x, last_y, 1, 1, 1, 1, 0, last_x, last_y, 0, ts++, NULL);
   /* only count arc motion: subtract initial move, mouse down, mouse up */
   return num;
}

int
pinch_object(Eo *obj, int x, int y, int x2, int y2, int dx, int dy, int dx2, int dy2)
{
   Evas *e = evas_object_evas_get(obj);
   int i, idx, idy, idx2, idy2;
   idx = idy = idx2 = idy2 = 0;
   evas_event_feed_multi_down(e, 0, x, y, 1, 1, 1, 1, 0, x, y, 0, ts, NULL);
   evas_event_feed_multi_down(e, 1, x2, y2, 1, 1, 1, 1, 0, x2, y2, 0, ts++, NULL);
   for (i = 1; i < abs(dx); i++)
     {
        idx = (i * dx / abs(dx));
        idy = (i * dy / abs(dx));
        idx2 = (i * dx2 / abs(dx));
        idy2 = (i * dy2 / abs(dx));
        /* move first point along positive vector */
        evas_event_feed_multi_move(e, 0, x + idx, y + idy, 1, 1, 1, 1, 0, x + idx, y + idy, ts, NULL);
        /* move second point along negative vector */
        evas_event_feed_multi_move(e, 1, x2 + idx2, y2 + idy2, 1, 1, 1, 1, 0, x2 + idx2, y2 + idy2, ts++, NULL);
        /* also trigger smart calc if we're iterating just in case that's important */
        evas_smart_objects_calculate(e);
     }
   evas_event_feed_multi_up(e, 0, x + idx, y + idy, 1, 1, 1, 1, 0, x + idx, y + idy, 0, ts, NULL);
   evas_event_feed_multi_up(e, 1, x2 + idx2, y2 + idy2, 1, 1, 1, 1, 0, x2 + idx2, y2 + idy2, 0, ts++, NULL);
   return i - 2;
}

void
write_key_sequence(Eo *obj, const char *seqence)
{
  Evas *e = evas_object_evas_get(obj);
  for (unsigned int i = 0; i < strlen(seqence); ++i)
    {
       const char part_seq[] = {seqence[i], '\0'};

       evas_event_feed_key_down(e, part_seq, part_seq, part_seq, part_seq, 0, NULL);
       ecore_main_loop_iterate();
       evas_event_feed_key_up(e, part_seq, part_seq, part_seq, part_seq, 0, NULL);
       ecore_main_loop_iterate();
    }
}

void
force_render(Eo *win)
{
   _win_manual_render(win, NULL);
}
