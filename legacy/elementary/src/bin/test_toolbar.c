#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_tb_sel1_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel2_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel3_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/wood_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel3a_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _tb_sel3_cb(data, obj, event_info);
   elm_toolbar_item_state_set(event_info, elm_toolbar_item_state_next(event_info));
}

static void
_tb_sel3b_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _tb_sel3_cb(data, obj, event_info);
   elm_toolbar_item_state_unset(event_info);
}

static void
_tb_sel4_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel4a_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_toolbar_item_state_set(event_info, elm_toolbar_item_state_prev(event_info));
}

static void
_tb_sel5_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_photo_file_set(data, NULL);
}

static void
toolbar_clicked_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = elm_toolbar_more_item_get(obj);

   if (event_info)
     {
        printf("toolbar item %p clicked", event_info);
        if (elm_toolbar_item_selected_get(event_info))
          printf(" (selected)\n");
        else
          printf(" (unselected)\n");
     }
   if (!strcmp(elm_object_item_text_get(it), "Open") && (it == elm_toolbar_selected_item_get(obj)))
     {
        elm_toolbar_item_icon_set(it, "arrow_up");
        elm_object_item_text_set(it, "Close");
     }
   else if (!strcmp(elm_object_item_text_get(it), "Close"))
     {
        elm_toolbar_item_icon_set(it, "arrow_down");
        elm_object_item_text_set(it, "Open");
     }
}

void
test_toolbar(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar", "Toolbar");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);
//   elm_object_scale_set(tb, 0.9);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 100);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 150);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

void
test_toolbar2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar2", "Toolbar 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_SCROLL);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, 100);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -150);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Scroll", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

void
test_toolbar3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar3", "Toolbar 3");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_NONE);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, EINA_FALSE);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -10);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 50);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "None", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/* The same test of toolbar, but using hide shrink mode instead of menu */
void
test_toolbar4(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar4", "Toolbar 4");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_HIDE);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 100);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 150);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

/* Toolbar with multiple state buttons */
void
test_toolbar5(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar5", "Toolbar 5");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);
   elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_NONE);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 100);

   snprintf(buf, sizeof(buf), "%s/images/icon_04.png", elm_app_data_dir_get());
   tb_it = elm_toolbar_item_append(tb, buf, "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3a_cb, ph4);
   elm_toolbar_item_state_add(tb_it, "object-rotate-left", "H2", _tb_sel3b_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 150);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4a_cb, ph4);
   elm_toolbar_item_state_add(tb_it, "emptytrash", "Comes2", _tb_sel4a_cb, ph4);
   elm_toolbar_item_state_add(tb_it, "trashcan_full", "Comes3", _tb_sel4a_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

void
test_toolbar_vertical(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar-vertical", "Toolbar Vertical");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_horizontal_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 100);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 150);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

void
test_toolbar6(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar6", "Toolbar 6");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_object_style_set(tb, "item_horizontal");
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, 100);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, -100);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 150);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 0);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   tb_it = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -9999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(tb_it);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", _tb_sel3_cb, ph4);
   menu_it = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, menu_it, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", _tb_sel5_cb, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, EINA_TRUE);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

void
test_toolbar7(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph, *sl;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar7", "Toolbar 7");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_toolbar_transverse_expanded_set(tb, EINA_TRUE);
   elm_toolbar_standard_priority_set(tb, 0);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   tb_it = elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_priority_set(tb_it, -100);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_priority_set(tb_it, 100);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -150);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   sl = elm_slider_add(win);
   evas_object_size_hint_min_set(sl, 100, 50);
   tb_it = elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL);
   elm_object_item_part_content_set(tb_it, "object", sl);

   elm_toolbar_item_priority_set(tb_it, 500);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "edit-cut", "Shrink", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -200);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "edit-delete", "Menu", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 200);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, 200);

   elm_toolbar_item_separator_set(elm_toolbar_item_append(tb, NULL, NULL, NULL, NULL), EINA_TRUE);

   tb_it = elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);
   elm_toolbar_item_priority_set(tb_it, -300);

   elm_object_item_text_set(elm_toolbar_more_item_get(tb), "Open");

   evas_object_smart_callback_add(tb, "clicked", toolbar_clicked_cb, NULL);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 80);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 160);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 120);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 420, 250);
   evas_object_show(win);
}

void
test_toolbar8(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *tb, *ph;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar8", "Toolbar 8");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   elm_toolbar_shrink_mode_set(tb, ELM_TOOLBAR_SHRINK_EXPAND);
   elm_toolbar_transverse_expanded_set(tb, EINA_TRUE);
   elm_toolbar_reorder_mode_set(tb, EINA_TRUE);
   elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_object_style_set(tb, "item_centered");
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   elm_toolbar_item_append(tb, "document-print", "Hello", _tb_sel1_cb, ph1);
   elm_toolbar_item_append(tb, "folder-new", "World", _tb_sel2_cb, ph1);
   elm_toolbar_item_append(tb, "object-rotate-right", "H", _tb_sel3_cb, ph4);
   elm_toolbar_item_append(tb, "mail-send", "Comes", _tb_sel4_cb, ph4);
   elm_toolbar_item_append(tb, "edit-cut", "Shrink", _tb_sel4_cb, ph4);
   elm_toolbar_item_append(tb, "edit-copy", "Mode", _tb_sel4_cb, ph4);
   elm_toolbar_item_append(tb, "edit-paste", "is set to", _tb_sel4_cb, ph4);
   elm_toolbar_item_append(tb, "edit-delete", "Menu", _tb_sel4_cb, ph4);
   elm_toolbar_item_append(tb, "clock", "Elementary", _tb_sel5_cb, ph4);

   evas_object_smart_callback_add(tb, "clicked", toolbar_clicked_cb, NULL);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 80);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 160);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 120);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 420, 250);
   evas_object_show(win);
}

static void
_item_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   printf("%s: %p\n", (char *)data, event_info);
}

static void
_test_toolbar_focus_focus_highlight_check_changed(void *data, Evas_Object *obj,
                                                  void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_enabled_set(data,
                                       elm_check_state_get(obj));
}

static void
_test_toolbar_focus_focus_animate_check_changed(void *data, Evas_Object *obj,
                                                void *event_info EINA_UNUSED)
{
   elm_win_focus_highlight_animate_set(data,
                                       elm_check_state_get(obj));
}

static void
_toolbar_focus_key_down_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   printf("\n=== Key Down : %s ===\n", ev->keyname);
}

static Ecore_Timer *timer = NULL;
static void
_test_toolbar_focus_win_del_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                               Evas_Object *obj EINA_UNUSED,
                               void *event_info EINA_UNUSED)
{
   ecore_timer_del(timer);
   timer = NULL;
}

static Eina_Bool
_focus_timer_cb(void *data)
{
   Elm_Object_Item *it = data;
   elm_object_item_focus_set(it, EINA_TRUE);
   timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_test_toolbar_focus_item_set_btn_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                    void *event_info EINA_UNUSED)
{
   ecore_timer_del(timer);
   timer = ecore_timer_add(1.5, _focus_timer_cb, data);
}

static void
_test_toolbar_focus_disable_item_btn_cb(void *data, Evas_Object *obj EINA_UNUSED,
                                        void *event_info EINA_UNUSED)
{
   elm_object_item_disabled_set(data, EINA_TRUE);
}

static void
test_toolbar_focus_focus_move_policy_changed(void *data EINA_UNUSED,
                                             Evas_Object *obj,
                                             void *event_info EINA_UNUSED)
{
   int val = elm_radio_value_get(obj);

   if (val == 0)
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_CLICK);
   else if (val == 1)
     elm_config_focus_move_policy_set(ELM_FOCUS_MOVE_POLICY_IN);
}

void
test_toolbar_focus(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *toolbar, *fr, *btn, *bx_opt, *chk, *bx_mv, *rd, *rdg;
   Elm_Object_Item *tb_it, *it_0, *it_3;

   win = elm_win_util_standard_add("toolbar-focus", "Toolbar Focus");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL,
                                  _test_toolbar_focus_win_del_cb, NULL);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Up");
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   toolbar = elm_toolbar_add(win);
   elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_SCROLL);
   evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, 0.0);
   it_0 = elm_toolbar_item_append(toolbar, "document-print", "Print", NULL, NULL);
   elm_toolbar_item_append(toolbar, "folder-new", "Folder", NULL, NULL);
   it_3 = elm_toolbar_item_append(toolbar, "clock", "Clock", NULL, NULL);
   elm_toolbar_item_append(toolbar, "refresh", "Update", NULL, NULL);
   elm_toolbar_item_append(toolbar, "folder-new", "Folder", NULL, NULL);
   elm_toolbar_item_append(toolbar, "clock", "Clock", NULL, NULL);
   tb_it = elm_toolbar_item_append(toolbar, "document-print", "Print", NULL, NULL);
   elm_object_item_disabled_set(tb_it, EINA_TRUE);
   elm_toolbar_item_append(toolbar, "folder-new", "Folder", NULL, NULL);
   elm_toolbar_item_append(toolbar, "refresh", "Update", NULL, NULL);
   elm_toolbar_item_append(toolbar, "folder-new", "Folder", NULL, NULL);
   elm_toolbar_item_append(toolbar, "clock", "Clock", NULL, NULL);
   elm_toolbar_item_append(toolbar, "document-print", "Print", NULL, NULL);
   elm_toolbar_item_append(toolbar, "folder-new", "Folder", NULL, NULL);
   elm_toolbar_item_append(toolbar, "refresh", "Update", NULL, NULL);
   elm_toolbar_item_append(toolbar, "folder-new", "Folder", NULL, NULL);
   elm_box_pack_end(bx, toolbar);
   evas_object_show(toolbar);
   evas_object_smart_callback_add(toolbar, "clicked", _item_cb, "clicked");
   evas_object_smart_callback_add(toolbar, "item,focused", _item_cb, "item,focused");
   evas_object_smart_callback_add(toolbar, "item,unfocused", _item_cb, "item,unfcoused");
   evas_object_event_callback_add(toolbar, EVAS_CALLBACK_KEY_DOWN, _toolbar_focus_key_down_cb, NULL);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Down");
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   //Options
   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Options");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_opt = elm_box_add(fr);
   elm_box_horizontal_set(bx_opt, EINA_TRUE);
   elm_object_content_set(fr, bx_opt);
   evas_object_show(bx_opt);

   chk = elm_check_add(bx_opt);
   elm_object_text_set(chk, "Focus Highlight");
   elm_check_state_set(chk, EINA_TRUE);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx_opt, chk);
   evas_object_show(chk);
   evas_object_smart_callback_add(chk, "changed",
                                  _test_toolbar_focus_focus_highlight_check_changed,
                                  win);

   chk = elm_check_add(bx_opt);
   elm_object_text_set(chk, "Focus Animation");
   elm_check_state_set(chk, EINA_TRUE);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx_opt, chk);
   evas_object_show(chk);
   evas_object_smart_callback_add(chk, "changed",
                                  _test_toolbar_focus_focus_animate_check_changed,
                                  win);
   // Focus movement policy
   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Focus Movement Policy");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_mv = elm_box_add(fr);
   elm_box_horizontal_set(bx_mv, EINA_TRUE);
   elm_object_content_set(fr, bx_mv);
   evas_object_show(bx_mv);

   rdg = rd = elm_radio_add(bx_mv);
   elm_object_text_set(rd, "Focus Move by Click");
   elm_radio_state_value_set(rd, 0);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(rd, "changed",
                                  test_toolbar_focus_focus_move_policy_changed,
                                  NULL);
   elm_box_pack_end(bx_mv, rd);
   evas_object_show(rd);

   rd = elm_radio_add(bx_mv);
   elm_object_text_set(rd, "Focus Move by Mouse-In");
   elm_radio_group_add(rd, rdg);
   elm_radio_state_value_set(rd, 1);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   evas_object_smart_callback_add(rd, "changed",
                                  test_toolbar_focus_focus_move_policy_changed,
                                  NULL);
   elm_box_pack_end(bx_mv, rd);
   evas_object_show(rd);

   fr = elm_frame_add(bx);
   elm_object_text_set(fr, "Focus");
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   bx_opt = elm_box_add(fr);
   elm_object_content_set(fr, bx_opt);
   evas_object_show(bx_opt);

   btn = elm_button_add(bx_opt);
   elm_object_text_set(btn, "Set focus to 3rd toolbar item after 1.5 seconds.");
   evas_object_size_hint_weight_set(btn, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_opt, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked",
                                  _test_toolbar_focus_item_set_btn_cb,
                                  it_3);

   btn = elm_button_add(bx_opt);
   elm_object_text_set(btn, "Disable 1st item.");
   evas_object_size_hint_weight_set(btn, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_opt, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked",
                                  _test_toolbar_focus_disable_item_btn_cb,
                                  it_0);

   evas_object_resize(win, 420, 200);
   evas_object_show(win);
}
