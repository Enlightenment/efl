//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` layout_example_01.c -o layout_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
# define PACKAGE_DATA_DIR "."
#endif

#define TABLE "example/table"
#define BOX "example/box"
#define TITLE "example/title"
#define SWALLOW "example/custom"

static int _box_buttons = 0;

static void
_tbl_btn_cb(void *data, Evas_Object *btn, void *event_info __UNUSED__)
{
   Evas_Object *layout = data;

   elm_layout_table_unpack(layout, TABLE, btn);
   evas_object_del(btn);
}

static void
_box_btn_cb(void *data, Evas_Object *btn, void *event_info __UNUSED__)
{
   Evas_Object *layout = data;
   Evas_Object *item;
   char buf[30];

   snprintf(buf, sizeof(buf), "Button %02d", _box_buttons++);

   item = elm_button_add(elm_object_parent_widget_get(layout));
   elm_object_text_set(item, buf);
   evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(item, 100, 50);
   elm_layout_box_insert_before(layout, BOX, item, btn);
   evas_object_smart_callback_add(item, "clicked", _box_btn_cb, layout);
   evas_object_show(item);
}

static void
_swallow_btn_cb(void *data, Evas_Object *btn __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *layout = data;
   Evas_Object *item;

   elm_layout_table_clear(layout, TABLE, EINA_TRUE);
   elm_layout_box_remove_all(layout, BOX, EINA_TRUE);
   item = elm_layout_content_unset(layout, SWALLOW);
   evas_object_del(item);
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *icon, *icon2, *bt, *bt2, *layout;
   Evas_Object *clock;
   Evas_Object *item;

   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255,255 ,255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   // Adding layout and filling it with widgets
   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   elm_layout_file_set(
       layout, PACKAGE_DATA_DIR "/examples/layout_example.edj",
       "example/mylayout");
   evas_object_show(layout);

   // Setting title
   const char *title = elm_layout_data_get(layout, "title");
   if (title)
     {
	elm_win_title_set(win, title);
	elm_object_text_part_set(layout, TITLE, title);
     }

   // Add icon, clock and button to the table
   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, "home");
   evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_table_pack(layout, TABLE, icon, 0, 0, 1, 1);
   evas_object_show(icon);

   icon2 = elm_icon_add(win);
   elm_icon_standard_set(icon2, "close");
   evas_object_size_hint_weight_set(icon2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(icon2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_table_pack(layout, TABLE, icon2, 1, 0, 1, 1);
   evas_object_show(icon2);

   clock = elm_clock_add(win);
   evas_object_size_hint_weight_set(clock, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(clock, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_table_pack(layout, TABLE, clock, 2, 0, 1, 1);
   evas_object_show(clock);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Click me!");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_table_pack(layout, TABLE, bt, 0, 1, 3, 1);
   evas_object_smart_callback_add(bt, "clicked", _tbl_btn_cb, layout);
   evas_object_show(bt);

   item = elm_button_add(win);
   elm_object_text_set(item, "Position 0");
   evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(item, 100, 50);
   elm_layout_box_insert_at(layout, BOX, item, 0);
   evas_object_smart_callback_add(item, "clicked", _box_btn_cb, layout);
   evas_object_show(item);

   item = elm_button_add(win);
   elm_object_text_set(item, "Prepended");
   evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(item, 100, 50);
   elm_layout_box_prepend(layout, BOX, item);
   evas_object_smart_callback_add(item, "clicked", _box_btn_cb, layout);
   evas_object_show(item);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Delete All");
   elm_layout_content_set(layout, SWALLOW, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _swallow_btn_cb, layout);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
