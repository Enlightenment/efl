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


static int main_pid = -1;
static Eina_Bool did_shutdown;
static Evas_Object *global_win;
static Eina_Bool buffer = EINA_FALSE;
static Eina_Bool legacy_mode = EINA_FALSE;

void elm_test_init(TCase *tc);

SUITE_INIT(elm)
{
   char *args[] = { "exe" };
   ck_assert_int_eq(elm_init(1, args), 1);
}

void
_elm2_suite_init(void)
{
   char *args[] = { "exe" };

   if (getpid() != main_pid)
     {
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
}

void
_elm_suite_shutdown(void)
{
   ck_assert_int_eq(elm_shutdown(), 0);
   /* verify that ecore was de-initialized completely */
   ck_assert_int_eq(ecore_init(), 1);
   /* avoid slowdowns in fork mode */
   if (getpid() != main_pid) return;
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

static void
_win_manual_render(void *data, const Efl_Event *event EINA_UNUSED)
 {
   double t = ecore_loop_time_get();

   ecore_loop_time_set(t + LOOP_INCREMENT);
   ecore_animator_custom_tick();
   ecore_evas_render_prepare(ecore_evas_ecore_evas_get(evas_object_evas_get(data)));
   evas_norender(evas_object_evas_get(data));
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
_elm_suite_win_create()
{
   Evas_Object *win;
   Eo *loop, *timer;

   if (legacy_mode)
     win = elm_win_add(NULL, "elm_suite", ELM_WIN_BASIC);
   else
     win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(), efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC));
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

Evas_Object *
win_add()
{
   if (getpid() != main_pid)
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

   if (getpid() != main_pid)
     {
        if (global_win) return global_win;
     }

   win = _elm_suite_win_create();
   force_focus_win(win);
   return win;
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

   failed_count += !elm_init(1, (char*[]){"exe"});
   if (buffer)
     {
        global_win = _elm_suite_win_create();
        force_focus_win(global_win);
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
