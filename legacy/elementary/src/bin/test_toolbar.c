#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static void
tb_1(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_2(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_3(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/wood_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_4(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_5(void *data, Evas_Object *obj, void *event_info)
{
   elm_photo_file_set(data, NULL);
}

void
test_toolbar(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *tb, *ic, *ph, *menu;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   Elm_Toolbar_Item *item;
   Elm_Menu_Item *menu_item;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogenous_set(tb, 0);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);
   
   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item = elm_toolbar_item_add(tb, ic, "Hello", tb_1, ph1);
   elm_toolbar_item_disabled_set(item, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "World", tb_2, ph1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "H", tb_3, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Comes", tb_4, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Elementary", tb_5, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item = elm_toolbar_item_add(tb, ic, "Menu", NULL, NULL);
   elm_toolbar_item_menu_set(item, 1);
   elm_toolbar_menu_parent_set(tb, win);
   menu = elm_toolbar_item_menu_get(item);
 
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_menu_item_add(menu, NULL, ic, "Here", tb_3, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   menu_item = elm_menu_item_add(menu, NULL, ic, "Comes", tb_4, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_menu_item_add(menu, menu_item, ic, "hey ho", tb_4, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   menu_item = elm_menu_item_add(menu, NULL, ic, "Elementary", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
   //elm_table_homogenous_set(tb, 1);
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
#endif
