//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` toggle_example_01.c -o toggle_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void _cb(void *data, Evas_Object *obj, void *event_info);
static void _cb2(void *data, Evas_Object *obj, void *event_info);

static Eina_Bool val = EINA_FALSE;

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *toggle, *ic;

   win = elm_win_add(NULL, "toggle", ELM_WIN_BASIC);
   elm_win_title_set(win, "toggle");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_FALSE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   toggle = elm_toggle_add(win);
   elm_object_text_set(toggle, "Toggle 1");
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "home");
   elm_toggle_icon_set(toggle, ic);
   elm_toggle_state_pointer_set(toggle, &val);
   elm_box_pack_end(bx, toggle);
   evas_object_size_hint_weight_set(toggle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(toggle, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(toggle);
   evas_object_smart_callback_add(toggle, "changed", _cb, NULL);

   toggle = elm_toggle_add(win);
   elm_object_text_set(toggle, "Toggle 2");
   elm_toggle_states_labels_set(toggle, "Enabled", "Disabled");
   elm_toggle_state_set(toggle, EINA_TRUE);
   elm_box_pack_end(bx, toggle);
   evas_object_size_hint_weight_set(toggle, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(toggle, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(toggle);
   evas_object_smart_callback_add(toggle, "changed", _cb2, NULL);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("val is now: %s\n", val ? "true" : "false");
}

static void
_cb2(void *data, Evas_Object *obj, void *event_info)
{
   printf("toggle2's state is now: %s\n", elm_toggle_state_get(obj) ? "true" : "false");
}