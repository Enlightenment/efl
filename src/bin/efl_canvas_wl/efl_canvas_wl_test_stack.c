#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "Efl_Canvas_Wl.h"
#include "Elementary.h"

static Evas_Object *win;

static const char *cmds[] =
{
   "weston-terminal",
   "terminology",
};

static unsigned int num_cmds = EINA_C_ARRAY_LENGTH(cmds);
static unsigned int n;

static Eina_Bool
dostuff(void *data)
{
   efl_canvas_wl_run(data, cmds[n++]);
   evas_object_focus_set(data, 1);
   return n != num_cmds;
}

static void
prev_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_canvas_wl_surface_prev(data);
}

static void
next_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_canvas_wl_surface_next(data);
}

int
main(int argc, char *argv[])
{
   Evas_Object *o, *comp, *prev, *next;
   elm_init(argc, (char**)argv);

   win = elm_win_util_standard_add("comp", "comp");
   elm_win_autodel_set(win, 1);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   o = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(o, 0, 125, 0, 125);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   o = elm_table_add(win);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   prev = elm_button_add(win);
   elm_object_text_set(prev, "prev");
   evas_object_size_hint_align_set(prev, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(prev, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(prev);
   elm_table_pack(o, prev, 0, 0, 1, 1);

   next = elm_button_add(win);
   elm_object_text_set(next, "next");
   evas_object_size_hint_align_set(next, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(next, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(next);
   elm_table_pack(o, next, 1, 0, 1, 1);

   comp = efl_add(EFL_CANVAS_WL_CLASS, win);
   evas_object_size_hint_min_set(comp, 640, 480);
   elm_table_pack(o, comp, 0, 1, 2, 1);
   evas_object_size_hint_align_set(comp, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(comp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(comp);
   evas_object_show(win);
   ecore_timer_add(1, dostuff, comp);

   evas_object_smart_callback_add(prev, "clicked", prev_clicked, comp);
   evas_object_smart_callback_add(next, "clicked", next_clicked, comp);
   
   elm_run();
   elm_shutdown();
   return 0;
}
