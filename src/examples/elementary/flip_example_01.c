//Compile with:
//gcc -o flip_example_01 flip_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void _change_interaction(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *rect, *rect2, *flip, *radio, *radio2, *radio3;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("flip", "Flip");
   elm_win_autodel_set(win, EINA_TRUE);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_resize(rect, 150, 150);
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_show(rect);

   rect2 = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect2, 0, 255, 0, 255);
   evas_object_show(rect2);

   flip = elm_flip_add(win);
   elm_object_part_content_set(flip, "front", rect);
   elm_object_part_content_set(flip, "back", rect2);
   evas_object_resize(flip, 150, 150);
   evas_object_move(flip, 10, 10);
   evas_object_show(flip);

   elm_flip_interaction_set(flip, ELM_FLIP_INTERACTION_PAGE);
   elm_flip_interaction_direction_enabled_set(flip, ELM_FLIP_DIRECTION_UP, EINA_TRUE);
   elm_flip_interaction_direction_enabled_set(flip, ELM_FLIP_DIRECTION_DOWN, EINA_TRUE);
   elm_flip_interaction_direction_enabled_set(flip, ELM_FLIP_DIRECTION_LEFT, EINA_TRUE);
   elm_flip_interaction_direction_enabled_set(flip, ELM_FLIP_DIRECTION_RIGHT, EINA_TRUE);
   elm_flip_interaction_direction_hitsize_set(flip, ELM_FLIP_DIRECTION_UP, 1);
   elm_flip_interaction_direction_hitsize_set(flip, ELM_FLIP_DIRECTION_DOWN, 1);
   elm_flip_interaction_direction_hitsize_set(flip, ELM_FLIP_DIRECTION_LEFT, 1);
   elm_flip_interaction_direction_hitsize_set(flip, ELM_FLIP_DIRECTION_RIGHT, 1);

   radio = elm_radio_add(win);
   elm_object_text_set(radio, "page");
   elm_radio_value_set(radio, ELM_FLIP_INTERACTION_PAGE);
   elm_radio_state_value_set(radio, ELM_FLIP_INTERACTION_PAGE);
   evas_object_resize(radio, 55, 30);
   evas_object_move(radio, 10, 160);
   evas_object_show(radio);
   evas_object_smart_callback_add(radio, "changed", _change_interaction, flip);

   radio2 = elm_radio_add(win);
   elm_object_text_set(radio2, "cube");
   elm_radio_state_value_set(radio2, ELM_FLIP_INTERACTION_CUBE);
   elm_radio_group_add(radio2, radio);
   evas_object_resize(radio2, 55, 30);
   evas_object_move(radio2, 75, 160);
   evas_object_show(radio2);
   evas_object_smart_callback_add(radio2, "changed", _change_interaction, flip);

   radio3 = elm_radio_add(win);
   elm_object_text_set(radio3, "rotate");
   elm_radio_state_value_set(radio3, ELM_FLIP_INTERACTION_ROTATE);
   elm_radio_group_add(radio3, radio);
   evas_object_resize(radio3, 55, 30);
   evas_object_move(radio3, 140, 160);
   evas_object_show(radio3);
   evas_object_smart_callback_add(radio3, "changed", _change_interaction, flip);

   evas_object_resize(win, 200, 200);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()

static void
_change_interaction(void *data, Evas_Object *obj, void *event_info)
{
   elm_flip_interaction_set(data, elm_radio_state_value_get(obj));
   elm_flip_go(data, ELM_FLIP_ROTATE_XZ_CENTER_AXIS);
}
