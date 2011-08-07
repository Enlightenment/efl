#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
tb_1(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_2(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_3(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/wood_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_3a(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   tb_3(data, obj, event_info);
   elm_toolbar_item_state_set(event_info, elm_toolbar_item_state_next(event_info));
}

static void
tb_3b(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   tb_3(data, obj, event_info);
   elm_toolbar_item_state_unset(event_info);
}

static void
tb_4(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_4a(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_toolbar_item_state_set(event_info, elm_toolbar_item_state_prev(event_info));
}

static void
tb_5(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_photo_file_set(data, NULL);
}

void
test_toolbar(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
   elm_toolbar_mode_shrink_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   item = elm_toolbar_item_append(tb, "document-print", "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);
   elm_toolbar_item_priority_set(item, 100);

   item = elm_toolbar_item_append(tb, "folder-new", "World", tb_2, ph1);
   elm_toolbar_item_priority_set(item, -100);

   item = elm_toolbar_item_append(tb, "object-rotate-right", "H", tb_3, ph4);
   elm_toolbar_item_priority_set(item, 150);

   item = elm_toolbar_item_append(tb, "mail-send", "Comes", tb_4, ph4);
   elm_toolbar_item_priority_set(item, 0);

   item = elm_toolbar_item_append(tb, "clock", "Elementary", tb_5, ph4);
   elm_toolbar_item_priority_set(item, -200);

   item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_item_priority_set(item, -999999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", tb_3, ph4);
   menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", tb_4, ph4);
   elm_menu_item_add(menu, menu_item, "edit-paste", "is set to", tb_4, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, 1);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
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
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
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
   Evas_Object *win, *bg, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
   elm_toolbar_mode_shrink_set(tb, ELM_TOOLBAR_SHRINK_SCROLL);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   item = elm_toolbar_item_append(tb, "document-print", "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);
   elm_toolbar_item_priority_set(item, -100);

   item = elm_toolbar_item_append(tb, "folder-new", "World", tb_2, ph1);
   elm_toolbar_item_priority_set(item, 100);

   item = elm_toolbar_item_append(tb, "object-rotate-right", "H", tb_3, ph4);
   elm_toolbar_item_priority_set(item, -150);

   item = elm_toolbar_item_append(tb, "mail-send", "Comes", tb_4, ph4);
   elm_toolbar_item_priority_set(item, -200);

   item = elm_toolbar_item_append(tb, "clock", "Elementary", tb_5, ph4);
   elm_toolbar_item_priority_set(item, 0);

   item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_item_priority_set(item, -999999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", tb_3, ph4);
   menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", tb_4, ph4);
   elm_menu_item_add(menu, menu_item, "edit-paste", "is set to", tb_4, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Scroll", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, 1);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
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
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
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
   Evas_Object *win, *bg, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar 3");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
   elm_toolbar_mode_shrink_set(tb, ELM_TOOLBAR_SHRINK_NONE);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   item = elm_toolbar_item_append(tb, "document-print", "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);
   elm_toolbar_item_priority_set(item, 0);

   item = elm_toolbar_item_append(tb, "folder-new", "World", tb_2, ph1);
   elm_toolbar_item_priority_set(item, -200);

   item = elm_toolbar_item_append(tb, "object-rotate-right", "H", tb_3, ph4);
   elm_toolbar_item_priority_set(item, 1);

   item = elm_toolbar_item_append(tb, "mail-send", "Comes", tb_4, ph4);
   elm_toolbar_item_priority_set(item, -10);

   item = elm_toolbar_item_append(tb, "clock", "Elementary", tb_5, ph4);
   elm_toolbar_item_priority_set(item, 50);

   item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_item_priority_set(item, 999999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", tb_3, ph4);
   menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", tb_4, ph4);
   elm_menu_item_add(menu, menu_item, "edit-paste", "is set to", tb_4, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "None", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, 1);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
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
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
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
   Evas_Object *win, *bg, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar4", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar 4");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
   elm_toolbar_mode_shrink_set(tb, ELM_TOOLBAR_SHRINK_HIDE);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   item = elm_toolbar_item_append(tb, "document-print", "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);
   elm_toolbar_item_priority_set(item, 100);

   item = elm_toolbar_item_append(tb, "folder-new", "World", tb_2, ph1);
   elm_toolbar_item_priority_set(item, -100);

   item = elm_toolbar_item_append(tb, "object-rotate-right", "H", tb_3, ph4);
   elm_toolbar_item_priority_set(item, 150);

   item = elm_toolbar_item_append(tb, "mail-send", "Comes", tb_4, ph4);
   elm_toolbar_item_priority_set(item, 0);

   item = elm_toolbar_item_append(tb, "clock", "Elementary", tb_5, ph4);
   elm_toolbar_item_priority_set(item, -200);

   item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_item_priority_set(item, -999999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", tb_3, ph4);
   menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", tb_4, ph4);
   elm_menu_item_add(menu, menu_item, "edit-paste", "is set to", tb_4, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
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
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
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
   Evas_Object *win, *bg, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar5", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar 5");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, 0);
   elm_toolbar_mode_shrink_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);
   elm_toolbar_no_select_mode_set(tb, EINA_TRUE);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   item = elm_toolbar_item_append(tb, "document-print", "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);
   elm_toolbar_item_priority_set(item, 100);

   item = elm_toolbar_item_append(tb, PACKAGE_DATA_DIR"/images/icon_04.png", "World", tb_2, ph1);
   elm_toolbar_item_priority_set(item, -100);

   item = elm_toolbar_item_append(tb, "object-rotate-right", "H", tb_3a, ph4);
   elm_toolbar_item_state_add(item, "object-rotate-left", "H2", tb_3b, ph4);
   elm_toolbar_item_priority_set(item, 150);

   item = elm_toolbar_item_append(tb, "mail-send", "Comes", tb_4a, ph4);
   elm_toolbar_item_state_add(item, "emptytrash", "Comes2", tb_4a, ph4);
   elm_toolbar_item_state_add(item, "trashcan_full", "Comes3", tb_4a, ph4);
   elm_toolbar_item_priority_set(item, 0);

   item = elm_toolbar_item_append(tb, "clock", "Elementary", tb_5, ph4);
   elm_toolbar_item_priority_set(item, -200);

   item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_item_priority_set(item, -999999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", tb_3, ph4);
   menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", tb_4, ph4);
   elm_menu_item_add(menu, menu_item, "edit-paste", "is set to", tb_4, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, 1);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
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
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
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
   Evas_Object *win, *bg, *bx, *tb, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_orientation_set(tb, EINA_TRUE);
   elm_toolbar_mode_shrink_set(tb, ELM_TOOLBAR_SHRINK_MENU);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   item = elm_toolbar_item_append(tb, "document-print", "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);
   elm_toolbar_item_priority_set(item, 100);

   item = elm_toolbar_item_append(tb, "folder-new", "World", tb_2, ph1);
   elm_toolbar_item_priority_set(item, -100);

   item = elm_toolbar_item_append(tb, "object-rotate-right", "H", tb_3, ph4);
   elm_toolbar_item_priority_set(item, 150);

   item = elm_toolbar_item_append(tb, "mail-send", "Comes", tb_4, ph4);
   elm_toolbar_item_priority_set(item, 0);

   item = elm_toolbar_item_append(tb, "clock", "Elementary", tb_5, ph4);
   elm_toolbar_item_priority_set(item, -200);

   item = elm_toolbar_item_append(tb, "refresh", "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_item_priority_set(item, -999999);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);

   elm_menu_item_add(menu, NULL, "edit-cut", "Shrink", tb_3, ph4);
   menu_item = elm_menu_item_add(menu, NULL, "edit-copy", "Mode", tb_4, ph4);
   elm_menu_item_add(menu, menu_item, "edit-paste", "is set to", tb_4, ph4);
   elm_menu_item_add(menu, NULL, "edit-delete", "Menu", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogeneous_set(tb, 1);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
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
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
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
#endif
