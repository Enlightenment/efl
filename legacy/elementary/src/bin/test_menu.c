#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *win, *bg, *menu, *rect;

static void
_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

static void
_populate_4(Elm_Menu_Item *item)
{
   Elm_Menu_Item *item2;

   elm_menu_item_add(menu, item, "go-bottom", "menu 2", NULL, NULL);
   elm_menu_item_add(menu, item, "go-first", "menu 3", NULL, NULL);
   elm_menu_item_separator_add(menu, item);
   item2 = elm_menu_item_add(menu, item, "go-last", "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item2, 1);
   item2 = elm_menu_item_add(menu, item, "go-next", "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item2, 1);
   item2 = elm_menu_item_add(menu, item, "go-up", "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item2, 1);
}

static void
_populate_3(Elm_Menu_Item *item)
{
   Elm_Menu_Item *item2;

   elm_menu_item_add(menu, item, "media-eject", "menu 2", NULL, NULL);
   elm_menu_item_add(menu, item, "media-playback-start", "menu 3", NULL, NULL);
   elm_menu_item_separator_add(menu,item);
   item2 = elm_menu_item_add(menu, item, "media-playback-stop", "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item2, 1);
}

static void
_populate_2(Elm_Menu_Item *item)
{
   Elm_Menu_Item *item2, *item3;

   elm_menu_item_add(menu, item, "system-reboot", "menu 2", NULL, NULL);
   item2 = elm_menu_item_add(menu, item, "system-shutdown", "menu 3", NULL, NULL);

   _populate_3(item2);

   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);
   elm_menu_item_separator_add(menu,item);

   item2 = elm_menu_item_add(menu, item, "system-lock-screen", "menu 2", NULL, NULL);
   elm_menu_item_separator_add(menu,item);

   item3 = elm_menu_item_add(menu, item, "system-run", "Disabled item", NULL, NULL);
   elm_menu_item_disabled_set(item3, 1);

   _populate_4(item2);
}

static void
_populate_1(Elm_Menu_Item *item)
{
   Elm_Menu_Item *item2;

   item2 = elm_menu_item_add(menu, item, "object-rotate-left", "menu 1", NULL, NULL);

   _populate_2(item2);
}

void
test_menu(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Menu_Item *item;

   win = elm_win_add(NULL, "menu", ELM_WIN_BASIC);
   elm_win_title_set(win, "Menu");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_win_resize_object_add(win, rect);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   menu = elm_menu_add(win);
   elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);

   item = elm_menu_item_add(menu, NULL, "mail-reply-all", "second item", NULL, NULL);
   _populate_1(item);

   elm_menu_item_add(menu, item, "window-new", "sub menu", NULL, NULL);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _show, menu);

   evas_object_resize(win, 350, 200);
   evas_object_show(win);
}

#endif
