#include <Elementary.h>
#include <Elementary_Cursor.h>
#ifndef ELM_LIB_QUICKLAUNCH

void
test_cursor(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bt, *list, *entry, *ck;

   win = elm_win_add(NULL, "cursor", ELM_WIN_BASIC);
   elm_win_title_set(win, "Cursor");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);
   elm_object_cursor_set(bg, ELM_CURSOR_CIRCLE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ck = elm_clock_add(win);
   elm_object_cursor_set(ck, ELM_CURSOR_CLOCK);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   bt = elm_button_add(win);
   elm_object_cursor_set(bt, ELM_CURSOR_COFFEE_MUG);
   elm_button_label_set(bt, "Coffee Mug");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_cursor_set(bt, ELM_CURSOR_CLOCK);
   elm_button_label_set(bt, "Cursor unset");
   elm_object_cursor_unset(bt);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   list = elm_list_add(win);
   elm_box_pack_end(bx, list);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_cursor_set(list, ELM_CURSOR_WATCH);
   elm_list_item_append(list, "watch over list", NULL, NULL,  NULL, NULL);
   elm_list_item_append(list, "watch over list", NULL, NULL,  NULL, NULL);
   elm_list_go(list);
   evas_object_show(list);

   entry = elm_scrolled_entry_add(win);
   elm_scrolled_entry_entry_set(entry, "Xterm cursor");
   elm_scrolled_entry_single_line_set(entry, EINA_TRUE);
   evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, entry);
   evas_object_show(entry);
   elm_object_cursor_set(entry, ELM_CURSOR_XTERM);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_cursor2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *o;
   Elm_Toolbar_Item *tit;
   Elm_List_Item *lit;

   win = elm_win_add(NULL, "cursor", ELM_WIN_BASIC);
   elm_win_title_set(win, "Cursor2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   o = elm_toolbar_add(win);
   elm_toolbar_homogenous_set(o, 0);
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, 0.0);
   tit = elm_toolbar_item_add(o, NULL, "Bogosity", NULL, NULL);
   elm_toolbar_item_cursor_set(tit, ELM_CURSOR_BOGOSITY);
   tit = elm_toolbar_item_add(o, NULL, "Unset", NULL, NULL);
   elm_toolbar_item_cursor_set(tit, ELM_CURSOR_BOGOSITY);
   elm_toolbar_item_cursor_unset(tit);
   tit = elm_toolbar_item_add(o, NULL, "Xterm", NULL, NULL);
   elm_toolbar_item_cursor_set(tit, ELM_CURSOR_XTERM);
   elm_box_pack_end(bx, o);
   evas_object_show(o);

   o = elm_list_add(win);
   elm_box_pack_end(bx, o);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   lit = elm_list_item_append(o, "cursor bogosity", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_BOGOSITY);
   lit = elm_list_item_append(o, "cursor unset", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_BOGOSITY);
   elm_list_item_cursor_unset(lit);
   lit = elm_list_item_append(o, "cursor xterm", NULL, NULL,  NULL, NULL);
   elm_list_item_cursor_set(lit, ELM_CURSOR_XTERM);
   elm_list_go(o);
   evas_object_show(o);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

#endif
