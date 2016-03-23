/**
 * Simple Elementary's <b>toolbar widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g toolbar_example_03.c -o toolbar_example_03 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bx, *tb, *bt, *menu;
   Elm_Object_Item *tb_it;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("toolbar", "Toolbar Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
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

   tb_it = elm_toolbar_item_append(tb, "mail-send", "Send Mail", NULL, NULL);
   elm_toolbar_item_menu_set(tb_it, EINA_TRUE);

   menu = elm_toolbar_item_menu_get(tb_it);
   elm_menu_item_add(menu, NULL, "emptytrash", "Empty Trash", NULL, NULL);
   elm_menu_item_add(menu, NULL, "trashcan_full", "Full Trash", NULL, NULL);

   elm_toolbar_item_priority_set(tb_it, 10);

   elm_box_pack_end(bx, tb);
   elm_box_pack_end(bx, bt);

   evas_object_resize(win, 230, 200);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
