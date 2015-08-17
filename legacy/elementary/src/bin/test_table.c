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

static void
_hor_sl_delay_changed_cb(void *data, Evas_Object *obj,
                         void *event_info EINA_UNUSED)
{
   Evas_Coord h = 0;

   elm_table_padding_get(data, NULL, &h);
   elm_table_padding_set(data, elm_slider_value_get(obj), h);
}

static void
_vert_sl_delay_changed_cb(void *data, Evas_Object *obj,
                          void *event_info EINA_UNUSED)
{
   Evas_Coord w = 0;

   elm_table_padding_get(data, &w, NULL);
   elm_table_padding_set(data, w, elm_slider_value_get(obj));
}

void
test_table8(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb_out, *tb, *bt, *bg, *sl;

   win = elm_win_util_standard_add("table8", "Table Padding");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   // outer table for the table alignment and background
   tb_out = elm_table_add(bx);
   evas_object_size_hint_weight_set(tb_out, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb_out, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, tb_out);
   evas_object_show(tb_out);

   // table background
   bg = elm_bg_add(tb_out);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb_out, bg, 0, 0, 1, 1);
   elm_bg_color_set(bg, 255, 255, 0);
   evas_object_show(bg);

   // actual table for a padding test
   tb = elm_table_add(tb_out);
   elm_table_padding_set(tb, 10, 10);
   evas_object_size_hint_align_set(tb_out, 0.5, 0.5);
   elm_table_pack(tb_out, tb, 0, 0, 1, 1);
   evas_object_show(tb);

   // first row
   bt = elm_button_add(tb);
   elm_object_text_set(bt, "colspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(tb);
   elm_object_text_set(bt, "colspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(tb);
   elm_object_text_set(bt, "colspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   // second row
   bt = elm_button_add(tb);
   elm_object_text_set(bt, "colspan 3");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 1, 3, 1);
   evas_object_show(bt);

   // third row
   bt = elm_button_add(tb);
   elm_object_text_set(bt, "rowspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(tb);
   elm_object_text_set(bt, "rowspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 2, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(tb);
   elm_object_text_set(bt, "rowspan 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 2, 2, 1, 2);
   evas_object_show(bt);

   // fourth row
   bt = elm_button_add(tb);
   elm_object_text_set(bt, "rowspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 0, 3, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(tb);
   elm_object_text_set(bt, "rowspan 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, bt, 1, 3, 1, 1);
   evas_object_show(bt);

   // horizontal padding sliders
   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Horizontal Padding");
   elm_slider_unit_format_set(sl, "%1.0f pixel");
   elm_slider_min_max_set(sl, 0, 100);
   elm_slider_value_set(sl, 10);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(sl, "delay,changed", _hor_sl_delay_changed_cb, tb);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   // vertical padding sliders
   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Vertical Padding");
   elm_slider_unit_format_set(sl, "%1.0f pixel");
   elm_slider_min_max_set(sl, 0, 100);
   elm_slider_value_set(sl, 10);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.0);
   evas_object_smart_callback_add(sl, "delay,changed", _vert_sl_delay_changed_cb, tb);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   evas_object_show(win);
}
