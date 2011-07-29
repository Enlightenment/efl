//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` menu_example_01.c -o menu_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

static void
_del_it(void *data, Evas_Object *obj, void *event_info)
{
   Eina_List *l;
   Elm_Menu_Item *it = elm_menu_first_item_get(data);
   it = elm_menu_item_next_get(it);
   l = elm_menu_item_subitems_get(it);
   elm_menu_item_del(eina_list_data_get(l));
}

static void
_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *menu, *button, *rect;
   Elm_Menu_Item *item;

   win = elm_win_add(NULL, "menu", ELM_WIN_BASIC);
   elm_win_title_set(win, "Menu");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_win_resize_object_add(win, rect);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   menu = elm_menu_add(win);
   elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);
   item = elm_menu_item_add(menu, NULL, "mail-reply-all", "second item", NULL, NULL);

   elm_menu_item_add(menu, item, "object-rotate-left", "menu 1", NULL, NULL);
   button = elm_button_add(win);
   elm_object_text_set(button, "button - delete items");
   elm_menu_item_add_object(menu, item, button, NULL, NULL);
   evas_object_smart_callback_add(button, "clicked", _del_it, menu);
   elm_menu_item_separator_add(menu, item);
   elm_menu_item_add(menu, item, NULL, "third item", NULL, NULL);
   elm_menu_item_add(menu, item, NULL, "fourth item", NULL, NULL);
   elm_menu_item_add(menu, item, "window-new", "sub menu", NULL, NULL);

   item = elm_menu_item_add(menu, NULL, NULL, "third item", NULL, NULL);
   elm_menu_item_disabled_set(item, EINA_TRUE);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _show, menu);
   evas_object_show(menu);

   evas_object_resize(win, 250, 350);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
