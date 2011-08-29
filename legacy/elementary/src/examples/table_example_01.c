//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` table_example_01.c -o table_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *label, *table;

   win = elm_win_add(NULL, "table", ELM_WIN_BASIC);
   elm_win_title_set(win, "Table");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   table = elm_table_add(win);
   elm_win_resize_object_add(win, table);
   evas_object_show(table);
   elm_table_padding_set(table, 5, 5);
   elm_table_homogeneous_set(table, EINA_TRUE);

   label = elm_label_add(win);
   elm_object_text_set(label, "label 0");
   evas_object_show(label);
   elm_table_pack(table, label, 0, 0, 1, 1);

   label = elm_label_add(win);
   elm_object_text_set(label, "label 1");
   evas_object_show(label);
   elm_table_pack(table, label, 1, 0, 1, 1);

   label = elm_label_add(win);
   elm_object_text_set(label, "label 2");
   evas_object_show(label);
   elm_table_pack(table, label, 0, 1, 1, 1);

   label = elm_label_add(win);
   elm_object_text_set(label, "label 3");
   evas_object_show(label);
   elm_table_pack(table, label, 1, 1, 1, 1);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
