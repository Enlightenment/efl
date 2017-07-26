//Compile with:
//gcc -g layout_example_01.c -o layout_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

#define TABLE "example/table"
#define BOX "example/box"
#define TITLE "example/title"
#define SWALLOW "example/custom"

static int _box_buttons = 0;

static void
_tbl_btn_cb(void *data, Evas_Object *btn, void *event_info EINA_UNUSED)
{
   Evas_Object *layout = data;

   elm_layout_table_unpack(layout, TABLE, btn);
   evas_object_del(btn);
}

static void
_box_btn_cb(void *data, Evas_Object *btn, void *event_info EINA_UNUSED)
{
   Evas_Object *layout = data;
   Evas_Object *item;
   char buf[30];

   snprintf(buf, sizeof(buf), "Button %02d", _box_buttons++);

   item = elm_button_add(elm_object_parent_widget_get(layout));
   elm_object_text_set(item, buf);
   evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_box_insert_before(layout, BOX, item, btn);
   evas_object_smart_callback_add(item, "clicked", _box_btn_cb, layout);
   evas_object_show(item);
}

static void
_swallow_btn_cb(void *data, Evas_Object *btn EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *layout = data;
   Evas_Object *item;

   elm_layout_table_clear(layout, TABLE, EINA_TRUE);
   elm_layout_box_remove_all(layout, BOX, EINA_TRUE);
   item = elm_object_part_content_unset(layout, SWALLOW);
   evas_object_del(item);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *icon, *icon2, *bt, *bt2, *layout;
   Evas_Object *clk;
   Evas_Object *item;
   char buf[PATH_MAX];

   elm_app_info_set(elm_main, "elementary", "examples/layout_example.edj");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("layout", "Layout");
   elm_win_autodel_set(win, EINA_TRUE);

   // Adding layout and filling it with widgets
   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //elm_win_resize_object_add(win, layout);
   //evas_object_resize(layout, 100, 100);
   evas_object_size_hint_min_set(layout, 100, 100);
   evas_object_move(layout, 0, 0);
   //snprintf(buf, sizeof(buf), "%s/examples/layout_example.edj", elm_app_data_dir_get());
   snprintf(buf, sizeof(buf), "%s/layout_example.edj", ".");
   elm_layout_file_set(layout, buf, "example/mylayout");
   evas_object_show(layout);

   // Setting title
   const char *title = elm_layout_data_get(layout, "title");
   if (title)
     {
        elm_win_title_set(win, title);
        elm_object_part_text_set(layout, TITLE, title);
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

   clk = elm_clock_add(win);
   evas_object_size_hint_weight_set(clk, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(clk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_table_pack(layout, TABLE, clk, 2, 0, 1, 1);
   evas_object_show(clk);

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
   elm_layout_box_insert_at(layout, BOX, item, 0);
   evas_object_smart_callback_add(item, "clicked", _box_btn_cb, layout);
   evas_object_show(item);

   item = elm_button_add(win);
   elm_object_text_set(item, "Prepended");
   evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_box_prepend(layout, BOX, item);
   evas_object_smart_callback_add(item, "clicked", _box_btn_cb, layout);
   evas_object_show(item);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Delete All");
   elm_object_part_content_set(layout, SWALLOW, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _swallow_btn_cb, layout);


   //proxy of layout: OK
   //proxy of mapped object: No
   Evas_Object *proxy = evas_object_image_filled_add(evas_object_evas_get(layout));
   //Evas_Object *proxy = evas_object_image_add(evas_object_evas_get(layout)); //must be filled image
   evas_object_image_source_set(proxy, layout);
   evas_object_resize(proxy, 100, 100);
   evas_object_move(proxy, 200, 200);
   evas_object_show(proxy);

   //map of layout: OK
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(layout, &x, &y, &w, &h);
   printf("layout geo: %d %d %d %d\n", x, y, w, h);
   Evas_Map *m = evas_map_new(4);
   evas_map_point_coord_set(m, 0, 100, 100, 0);
   evas_map_point_coord_set(m, 1, 200, 100, 0);
   evas_map_point_coord_set(m, 2, 100, 200, 0);
   evas_map_point_coord_set(m, 3, 0, 200, 0);
   evas_map_point_image_uv_set(m, 0, 0, 0);
   evas_map_point_image_uv_set(m, 1, 100, 0);
   evas_map_point_image_uv_set(m, 2, 100, 100);
   evas_map_point_image_uv_set(m, 3, 0, 100);
   evas_object_map_enable_set(layout, EINA_TRUE);
   evas_object_map_set(layout, m);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
