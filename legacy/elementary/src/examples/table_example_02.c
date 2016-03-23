//Compile with:
//gcc -g table_example_02.c -o table_example_02 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *rect, *table;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("table", "Table");
   elm_win_autodel_set(win, EINA_TRUE);

   table = elm_table_add(win);
   elm_win_resize_object_add(win, table);
   evas_object_show(table);
   elm_table_homogeneous_set(table, EINA_FALSE);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_size_hint_min_set(rect, 100, 50);
   evas_object_show(rect);
   elm_table_pack(table, rect, 0, 0, 2, 1);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 255, 0, 255);
   evas_object_size_hint_min_set(rect, 50, 100);
   evas_object_show(rect);
   elm_table_pack(table, rect, 0, 1, 1, 2);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 0, 0, 255, 255);
   evas_object_size_hint_min_set(rect, 50, 50);
   evas_object_show(rect);
   elm_table_pack(table, rect, 1, 1, 1, 1);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rect, 255, 255, 0, 255);
   evas_object_size_hint_min_set(rect, 50, 50);
   evas_object_show(rect);
   elm_table_pack(table, rect, 1, 2, 1, 1);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
