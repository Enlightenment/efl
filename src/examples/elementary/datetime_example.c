//Compile with:
//gcc -g datetime_example.c -o datetime_example `pkg-config --cflags --libs elementary`

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   Evas_Object *win, *bx, *datetime;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("Datetime", "Datetime");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_FALSE);
   evas_object_show(bx);

   //datetime showing only DATE
   datetime = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, 0.5);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_HOUR, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_MINUTE, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_AMPM, EINA_FALSE);
   elm_box_pack_end(bx, datetime);
   evas_object_show(datetime);

   //datetime showing only TIME
   datetime = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, 0.5);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_YEAR, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_MONTH, EINA_FALSE);
   elm_datetime_field_visible_set(datetime, ELM_DATETIME_DATE, EINA_FALSE);
   elm_box_pack_end(bx, datetime);
   evas_object_show(datetime);

   //datetime showing both DATE and TIME
   datetime = elm_datetime_add(bx);
   evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, datetime);
   evas_object_show(datetime);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
