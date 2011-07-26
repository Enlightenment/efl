/**
 * Simple Elementary's <b>spinner widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` spinner_example.c -o spinner_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_on_done(void *data __UNUSED__,
        Evas_Object *obj __UNUSED__,
        void *event_info __UNUSED__)
{
   elm_exit();
}

static void
_changed_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("Value changed to %0.f\n", elm_spinner_value_get(obj));
}

static void
_delay_changed_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("Value delay changed to %0.f\n", elm_spinner_value_get(obj));
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *sp;

   win = elm_win_add(NULL, "spinner", ELM_WIN_BASIC);
   elm_win_title_set(win, "Spinner Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

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
   evas_object_smart_callback_add(sp, "changed", _changed_cb, NULL);
   evas_object_smart_callback_add(sp, "delay,changed", _delay_changed_cb, NULL);

   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
