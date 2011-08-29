//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` radio_example_01.c -o radio_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static int val = 1;

static void _cb(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *radio, *group, *ic;

   win = elm_win_add(NULL, "radio", ELM_WIN_BASIC);
   elm_win_title_set(win, "Radio");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   group = radio = elm_radio_add(win);
   elm_object_text_set(radio, "Radio 1");
   elm_radio_state_value_set(radio, 1);
   elm_radio_value_pointer_set(radio, &val);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "home");
   elm_radio_icon_set(radio, ic);
   elm_box_pack_end(bx, radio);
   evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(radio);
   evas_object_smart_callback_add(radio, "changed", _cb, NULL);

   radio = elm_radio_add(win);
   elm_object_text_set(radio, "Radio 2");
   elm_radio_state_value_set(radio, 2);
   elm_radio_value_pointer_set(radio, &val);
   elm_radio_group_add(radio, group);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "file");
   elm_radio_icon_set(radio, ic);
   elm_box_pack_end(bx, radio);
   evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(radio);
   evas_object_smart_callback_add(radio, "changed", _cb, NULL);

   radio = elm_radio_add(win);
   elm_object_text_set(radio, "Radio 3");
   elm_radio_state_value_set(radio, 3);
   elm_radio_value_pointer_set(radio, &val);
   elm_radio_group_add(radio, group);
   elm_box_pack_end(bx, radio);
   evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(radio);
   evas_object_smart_callback_add(radio, "changed", _cb, NULL);

   radio = elm_radio_add(win);
   elm_object_text_set(radio, "Radio 4");
   elm_radio_state_value_set(radio, 4);
   elm_box_pack_end(bx, radio);
   evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(radio);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_cb(void *data, Evas_Object *obj, void *event_info)
{
   printf("val is now: %d\n", val);
}
