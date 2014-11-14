//Compile with:
//gcc -g menu_example_01.c -o menu_example_01 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void
_del_it(void *data, Evas_Object *obj, void *event_info)
{
   const Eina_List *l;
   Elm_Object_Item *menu_it = elm_menu_first_item_get(data);
   menu_it = elm_menu_item_next_get(menu_it);
   l = elm_menu_item_subitems_get(menu_it);
   elm_object_item_del(eina_list_data_get(l));
}

static void
_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *menu, *button, *rect;
   Elm_Object_Item *menu_it, *menu_it1;

   win = elm_win_util_standard_add("menu", "Menu");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_win_resize_object_add(win, rect);
   evas_object_size_hint_min_set(rect, 0, 0);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   menu = elm_menu_add(win);
   elm_menu_item_add(menu, NULL, NULL, "first item", NULL, NULL);
   menu_it = elm_menu_item_add(menu, NULL, "mail-reply-all", "second item", NULL, NULL);

   elm_menu_item_add(menu, menu_it, "object-rotate-left", "menu 1", NULL, NULL);
   button = elm_button_add(win);
   elm_object_text_set(button, "button - delete items");
   menu_it1 = elm_menu_item_add(menu, menu_it, NULL, NULL, NULL, NULL);
   elm_object_item_content_set(menu_it1, button);
   evas_object_smart_callback_add(button, "clicked", _del_it, menu);
   elm_menu_item_separator_add(menu, menu_it);
   elm_menu_item_add(menu, menu_it, NULL, "third item", NULL, NULL);
   elm_menu_item_add(menu, menu_it, NULL, "fourth item", NULL, NULL);
   elm_menu_item_add(menu, menu_it, "window-new", "sub menu", NULL, NULL);

   menu_it = elm_menu_item_add(menu, NULL, NULL, "third item", NULL, NULL);
   elm_object_item_disabled_set(menu_it, EINA_TRUE);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _show, menu);
   evas_object_show(menu);

   evas_object_resize(win, 250, 350);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
