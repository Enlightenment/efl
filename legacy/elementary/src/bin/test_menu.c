/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info; 
   elm_menu_move(data, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}



void
test_menu(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *notify, *menu, *rect;
   char buf[PATH_MAX];
   const Eina_List *l;
   const char *transition;

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
   elm_menu_item_add(menu, NULL, "first item", NULL, NULL);
   elm_menu_item_add(menu, NULL, "second item", NULL, NULL);

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, _show, menu);

   evas_object_resize(win, 350, 200);
   evas_object_show(win);
}

#endif
