#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "Efl_Canvas_Wl.h"
#include "Elementary.h"

static Evas_Object *win;
static Eina_Strbuf *buf;
static Eo *exe;

static void
del_handler(void *d EINA_UNUSED, const Efl_Event *ev)
{
   if (ev->object == exe) ecore_main_loop_quit();
}

static void
focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_focus_set(data, 1);
}

static Eina_Bool
dostuff(void *data)
{
   exe = efl_canvas_wl_run(data, eina_strbuf_string_get(buf));
   efl_event_callback_add(exe, EFL_TASK_EVENT_EXIT, del_handler, NULL);
   evas_object_focus_set(data, 1);
   return EINA_FALSE;
}

static void
hints_changed(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;
   Evas_Aspect_Control aspect;

   evas_object_size_hint_aspect_get(obj, &aspect, &w, &h);
   evas_object_size_hint_aspect_set(data, aspect, w, h);
   evas_object_size_hint_min_get(obj, &w, &h);
   evas_object_size_hint_min_set(data, w, h);
   evas_object_size_hint_max_get(obj, &w, &h);
   evas_object_size_hint_max_set(data, w, h);
}

int
main(int argc, char *argv[])
{
   Evas_Object *o;
   int i;

   if (argc < 2) return 0;
   elm_init(argc, (char**)argv);
   buf = eina_strbuf_new();
   for (i = 1; i < argc; i++)
     {
        eina_strbuf_append_escaped(buf, argv[i]);
        if (i + 1 < argc) eina_strbuf_append_char(buf, ' ');
     }

   win = elm_win_util_standard_add("comp", "comp");
   elm_win_autodel_set(win, 1);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   o = efl_add(EFL_CANVAS_WL_CLASS, win);
   efl_canvas_wl_aspect_propagate_set(o, 1);
   efl_canvas_wl_minmax_propagate_set(o, 1);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_event_callback_add(o, EVAS_CALLBACK_CHANGED_SIZE_HINTS, hints_changed, win);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);
   evas_object_show(win);
   evas_object_resize(win, 640, 480);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FOCUS_IN, focus_in, o);
   ecore_timer_add(1, dostuff, o);
   
   elm_run();
   elm_shutdown();
   return 0;
}
