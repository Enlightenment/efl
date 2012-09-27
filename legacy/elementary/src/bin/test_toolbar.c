#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static void
_tb_sel1_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel2_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel3_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/wood_01.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel3a_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _tb_sel3_cb(data, obj, event_info);
   elm_toolbar_item_state_set(event_info, elm_toolbar_item_state_next(event_info));
}

static void
_tb_sel3b_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _tb_sel3_cb(data, obj, event_info);
   elm_toolbar_item_state_unset(event_info);
}

static void
_tb_sel4_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", elm_app_data_dir_get());
   elm_photo_file_set(data, buf);
}

static void
_tb_sel4a_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_toolbar_item_state_set(event_info, elm_toolbar_item_state_prev(event_info));
}

static void
_tb_sel5_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_photo_file_set(data, NULL);
}

static void
toolbar_clicked_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Object_Item *it = elm_toolbar_more_item_get(obj);

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
test_toolbar(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar", "Toolbar");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
   //elm_table_homogeneous_set(tb, 1);
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
test_toolbar2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar2", "Toolbar 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
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
   //elm_table_homogeneous_set(tb, 1);
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
test_toolbar3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar3", "Toolbar 3");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
test_toolbar4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar4", "Toolbar 4");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
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
test_toolbar5(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar5", "Toolbar 5");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
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

   snprintf(buf, sizeof(buf), "%s/images/icon_04.pngj", elm_app_data_dir_get());
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
test_toolbar6(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar6", "Toolbar 6");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
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
test_toolbar7(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
   Elm_Object_Item *menu_it;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("toolbar7", "Toolbar 7");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_object_style_set(tb, "item_horizontal");
   elm_toolbar_homogeneous_set(tb, 0);
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
   //elm_table_homogeneous_set(tb, 1);
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
test_toolbar8(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *tb, *ph, *sl;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Object_Item *tb_it;
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
#endif
