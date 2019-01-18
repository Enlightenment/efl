/**
 * Simple Elementary's <b>spinner widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g spinner_example.c -o spinner_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("Value changed to %0.f\n", elm_spinner_value_get(obj));
}

static void
_delay_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("Value delay changed to %0.f\n", elm_spinner_value_get(obj));
}

static void
_focused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("spinner focused\n");
}

static void
_unfocused_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("spinner unfocused\n");
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *bx, *sp;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("spinner", "Spinner Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /* default */
   sp = elm_spinner_add(win);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* format */
   sp = elm_spinner_add(win);
   elm_spinner_label_format_set(sp, "Percentage %%%1.2f something");
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* min max, step and wrap */
   sp = elm_spinner_add(win);
   elm_spinner_label_format_set(sp, "%1.1f units");
   elm_spinner_step_set(sp, 1.5);
   elm_spinner_wrap_set(sp, EINA_TRUE);
   elm_spinner_min_max_set(sp, -50.0, 250.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* vertical */
   sp = elm_spinner_add(win);
   elm_object_style_set(sp, "vertical");
   elm_spinner_interval_set(sp, 0.2);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* disabled edition */
   sp = elm_spinner_add(win);
   elm_spinner_editable_set(sp, EINA_FALSE);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   /* special values */
   sp = elm_spinner_add(win);
   elm_spinner_editable_set(sp, EINA_FALSE);
   elm_spinner_min_max_set(sp, 1, 12);
   elm_spinner_special_value_add(sp, 1, "January");
   elm_spinner_special_value_add(sp, 2, "February");
   elm_spinner_special_value_add(sp, 3, "March");
   elm_spinner_special_value_add(sp, 4, "April");
   elm_spinner_special_value_add(sp, 5, "May");
   elm_spinner_special_value_add(sp, 6, "June");
   elm_spinner_special_value_add(sp, 7, "July");
   elm_spinner_special_value_add(sp, 8, "August");
   elm_spinner_special_value_add(sp, 9, "September");
   elm_spinner_special_value_add(sp, 10, "October");
   elm_spinner_special_value_add(sp, 11, "November");
   elm_spinner_special_value_add(sp, 12, "December");
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   sp = elm_spinner_add(win);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);
   elm_spinner_editable_set(sp, EINA_TRUE);
   evas_object_smart_callback_add(sp, "focused", _focused_cb, NULL);
   evas_object_smart_callback_add(sp, "unfocused", _unfocused_cb, NULL);
   evas_object_smart_callback_add(sp, "changed", _changed_cb, NULL);
   evas_object_smart_callback_add(sp, "delay,changed", _delay_changed_cb, NULL);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
