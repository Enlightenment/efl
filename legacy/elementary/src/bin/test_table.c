#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_table(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table", "Table");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 4");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 1, 2, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 5");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 2, 1, 1, 2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button a");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 2, 2, 1);
   evas_object_show(bt);

   evas_object_show(win);
}

void
test_table2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table2", "Table Homogeneous");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   elm_table_homogeneous_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "A");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 2, 2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Blah blah blah");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 3, 0, 2, 3);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Hallow");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 3, 10, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "B");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 2, 5, 2, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "C");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 8, 8, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wide");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 7, 7, 2);
   evas_object_show(bt);

   evas_object_show(win);
}

static void
my_tb_ch(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *tb = evas_object_data_get(win, "tb");
   Evas_Object *b2 = evas_object_data_get(win, "b2");

   elm_table_unpack(tb, b2);
   elm_table_pack(tb, b2, 1, 0, 1, 2);
}

void
test_table3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table3", "Table 3");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_data_set(win, "tb", tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 0, 1, 1);
   evas_object_data_set(win, "b1", bt);
   evas_object_smart_callback_add(bt, "clicked", my_tb_ch, win);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 0, 1, 1);
   evas_object_data_set(win, "b2", bt);
   evas_object_smart_callback_add(bt, "clicked", my_tb_ch, win);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 1, 1, 1);
   evas_object_data_set(win, "b3", bt);
   evas_object_smart_callback_add(bt, "clicked", my_tb_ch, win);
   evas_object_show(bt);

   evas_object_show(win);
}

void
test_table4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table4", "Table 4");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_data_set(win, "tb", tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, 0.25, 0.25);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 0, 1, 1);
   evas_object_data_set(win, "b1", bt);
   evas_object_smart_callback_add(bt, "clicked", my_tb_ch, win);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, 0.75, 0.25);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 0, 1, 1);
   evas_object_data_set(win, "b2", bt);
   evas_object_smart_callback_add(bt, "clicked", my_tb_ch, win);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, 0.25, 0.75);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 1, 1, 1);
   evas_object_data_set(win, "b3", bt);
   evas_object_smart_callback_add(bt, "clicked", my_tb_ch, win);
   evas_object_show(bt);

   evas_object_show(win);
}

void
test_table5(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table5", "Table 5");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   elm_table_homogeneous_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "A");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 33, 0, 34, 33);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "B");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 67, 33, 33, 34);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "C");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 33, 67, 34, 33);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "D");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 33, 33, 34);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "X");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 33, 33, 34, 34);
   evas_object_show(bt);

   evas_object_show(win);
}

void
test_table6(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table6", "Table 6");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   elm_table_homogeneous_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "C");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 2, 2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "A");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 2, 2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Blah blah blah");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 3, 0, 2, 3);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Hallow");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 3, 10, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "B");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wide");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 7, 7, 2);
   evas_object_show(bt);

   evas_object_show(win);
}

void
test_table7(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bt;

   win = elm_win_util_standard_add("table7", "Table 7");
   elm_win_autodel_set(win, EINA_TRUE);

   tb = elm_table_add(win);
   elm_table_padding_set(tb, 10, 20);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, tb);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "C");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 2, 2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "A");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 2, 2);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Blah blah blah");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 3, 0, 2, 3);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Hallow");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 3, 10, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "B");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 1, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wide");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 7, 7, 2);
   evas_object_show(bt);

   evas_object_show(win);
}
