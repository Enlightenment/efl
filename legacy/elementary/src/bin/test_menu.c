#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *win, *bg, *menu, *rect, *ic;;
static char buf[PATH_MAX];

static void
_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info; 
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

static void 
_populate_4(Elm_Menu_Item *item)
{
   Evas_Object *ic;
   Elm_Menu_Item *item2, *item3;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_menu_item_add(menu, item, ic, "menu 2", NULL, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);

   item2 = elm_menu_item_add(menu, item, ic, "menu 3", NULL, NULL);
   
   elm_menu_item_separator_add(menu, item);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item3 = elm_menu_item_add(menu, item, ic, "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item3, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item3 = elm_menu_item_add(menu, item, ic, "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item3, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item3 = elm_menu_item_add(menu, item, ic, "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item3, 1);
}

static void 
_populate_3(Elm_Menu_Item *item)
{
   Evas_Object *ic;
   Elm_Menu_Item *item2, *item3;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_menu_item_add(menu, item, ic, "menu 2", NULL, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);

   item2 = elm_menu_item_add(menu, item, ic, "menu 3", NULL, NULL);
   
   elm_menu_item_separator_add(menu,item);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item3 = elm_menu_item_add(menu, item, ic, "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item3, 1);
}

static void 
_populate_2(Elm_Menu_Item *item)
{
   Evas_Object *ic;
   Elm_Menu_Item *item2, *item3;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_menu_item_add(menu, item, ic, "menu 2", NULL, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);

   item2 = elm_menu_item_add(menu, item, ic, "menu 3", NULL, NULL);
   
   _populate_3(item2);

   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);

   item2 = elm_menu_item_add(menu, item, ic, "menu 2", NULL, NULL);

   elm_menu_item_separator_add(menu,item);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item3 = elm_menu_item_add(menu, item, ic, "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item3, 1);

   _populate_4(item2);
}

static void 
_populate_1(Elm_Menu_Item *item)
{
   Elm_Menu_Item *item2;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   item2 = elm_menu_item_add(menu, item, ic, "menu 1", NULL, NULL);

   _populate_2(item2);
}

void
test_menu(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Menu_Item *item;

   win = elm_win_add(NULL, "Menu", ELM_WIN_BASIC);
   elm_win_title_set(win, "Menu");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_win_resize_object_add(win, rect);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   menu = elm_menu_add(win);
   item = elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);

   item = elm_menu_item_add(menu, NULL, ic, "second item", NULL, NULL);
   _populate_1(item);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_menu_item_add(menu, item, ic, "sub menu", NULL, NULL);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _show, menu);

   evas_object_resize(win, 350, 200);
   evas_object_show(win);
}

#endif
