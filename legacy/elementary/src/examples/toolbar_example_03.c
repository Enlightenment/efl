/**
 * Simple Elementary's <b>toolbar widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` toolbar_example_03.c -o toolbar_example_03
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_on_done(void *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void *event_info __UNUSED__)
{
   elm_exit();
}

static void
_item_pressed(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Toolbar_Item *item = event_info;
   elm_toolbar_item_state_set(item, elm_toolbar_item_state_next(item));
   elm_toolbar_item_selected_set(item, EINA_FALSE);
}

EAPI int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *tb, *bt, *menu;
   Elm_Toolbar_Item *item;

   win = elm_win_add(NULL, "toolbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);

   bt = elm_button_add(win);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt, "Button");
   evas_object_show(bt);

   tb = elm_toolbar_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);
   evas_object_show(tb);

   elm_toolbar_item_append(tb, "document-print", "Print", NULL, NULL);
   elm_toolbar_item_append(tb, "folder-new", "Folder", NULL, NULL);
   elm_toolbar_item_append(tb, "clock", "Clock", NULL, NULL);
   elm_toolbar_item_append(tb, "refresh", "Update", NULL, NULL);

   elm_toolbar_menu_parent_set(tb, win);

   item = elm_toolbar_item_append(tb, "mail-send", "Send Mail", NULL, NULL);
   elm_toolbar_item_menu_set(item, EINA_TRUE);

   menu = elm_toolbar_item_menu_get(item);
   elm_menu_item_add(menu, NULL, "emptytrash", "Empty Trash", NULL, NULL);
   elm_menu_item_add(menu, NULL, "trashcan_full", "Full Trash", NULL, NULL);

   elm_toolbar_item_priority_set(item, 10);

   elm_box_pack_end(bx, tb);
   elm_box_pack_end(bx, bt);

   evas_object_resize(win, 230, 200);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
