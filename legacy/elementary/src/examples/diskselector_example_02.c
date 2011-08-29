/**
 * Elementary's <b>diskselector widget</b> example, illustrating its
 * covering most of item functions.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` diskselector_example_02.c -o diskselector_example_02
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static int counter = 3;

static void
_on_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info __UNUSED__)
{
   elm_exit();
}

static void
_add_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *it;
   Evas_Object *ds = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   it = elm_diskselector_item_append(ds, label, NULL, NULL, NULL);
   if (!it)
     printf("Error adding item\n");
}

static void
_add_ic_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *it;
   Evas_Object *ic, *ds = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   ic = elm_icon_add(ds);
   elm_icon_standard_set(ic, "home");

   it = elm_diskselector_item_append(ds, label, ic, NULL, NULL);
   if (!it)
     printf("Error adding item with icon\n");
}

static void
_sel_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Diskselector_Item *it = event_info;
   printf("Selected label: %s\n", elm_diskselector_item_label_get(it));
}

static void
_add_func_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *it;
   Evas_Object *ds = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   it = elm_diskselector_item_append(ds, label, NULL, _sel_cb, NULL);
   if (!it)
     printf("Error adding item\n");
}

static void
_sel_data_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info)
{
   char *content = data;
   Elm_Diskselector_Item *it = event_info;
   printf("Selected label: %s with data: %s\n",
          elm_diskselector_item_label_get(it), content);
}

static void
_free_data(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   free(data);
}

static void
_add_data_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *it;
   Evas_Object *ds = data;
   char label[32];
   char *content = malloc(sizeof(char) * 32);

   snprintf(content, 32, "Item content %i", counter);
   snprintf(label, sizeof(label), "Item %i", counter++);
   it = elm_diskselector_item_append(ds, label, NULL, _sel_data_cb, content);
   if (!it) {
     printf("Error adding item\n");
     return;
   }
   elm_diskselector_item_del_cb_set(it, _free_data);
}

static void
_del_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *selected_item;
   Evas_Object *ds = data;

   selected_item = elm_diskselector_selected_item_get(ds);
   elm_diskselector_item_del(selected_item);
}

static void
_unselect_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *selected_item;
   Evas_Object *ds = data;

   selected_item = elm_diskselector_selected_item_get(ds);
   elm_diskselector_item_selected_set(selected_item, EINA_FALSE);
}

static void
_print_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   const Eina_List *l, *items;
   Elm_Diskselector_Item *it;
   Evas_Object *ds = data;

   items = elm_diskselector_items_get(ds);
   EINA_LIST_FOREACH(items, l, it)
      printf("%s\n", elm_diskselector_item_label_get(it));
}

static void
_clear_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ds = data;
   elm_diskselector_clear(ds);
}

static void
_select_first_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *first_item;
   Evas_Object *ds = data;

   first_item = elm_diskselector_first_item_get(ds);
   if (first_item)
     elm_diskselector_item_selected_set(first_item, EINA_TRUE);
}

static void
_select_last_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *last_item;
   Evas_Object *ds = data;

   last_item = elm_diskselector_last_item_get(ds);
   if (last_item)
     elm_diskselector_item_selected_set(last_item, EINA_TRUE);
}

static void
_select_next_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *selected_item, *next_item;
   Evas_Object *ds = data;

   selected_item = elm_diskselector_selected_item_get(ds);
   if (!selected_item) return;

   next_item = elm_diskselector_item_next_get(selected_item);
   if (next_item)
     elm_diskselector_item_selected_set(next_item, EINA_TRUE);
}

static void
_select_prev_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Diskselector_Item *selected_item, *prev_item;
   Evas_Object *ds = data;

   selected_item = elm_diskselector_selected_item_get(ds);
   if (!selected_item) return;

   prev_item = elm_diskselector_item_prev_get(selected_item);
   if (prev_item)
     elm_diskselector_item_selected_set(prev_item, EINA_TRUE);
}

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *hbx, *ds, *bt;

   win = elm_win_add(NULL, "diskselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Diskselector Items Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ds);
   evas_object_show(ds);

   elm_diskselector_item_append(ds, "Item 0", NULL, NULL, NULL);
   elm_diskselector_item_append(ds, "Item 1", NULL, NULL, NULL);
   elm_diskselector_item_append(ds, "Item 2", NULL, NULL, NULL);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append item");
   evas_object_smart_callback_add(bt, "clicked", _add_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append with icon");
   evas_object_smart_callback_add(bt, "clicked", _add_ic_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append with selected function");
   evas_object_smart_callback_add(bt, "clicked", _add_func_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append with data");
   evas_object_smart_callback_add(bt, "clicked", _add_data_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete item");
   evas_object_smart_callback_add(bt, "clicked", _del_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unselect item");
   evas_object_smart_callback_add(bt, "clicked", _unselect_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Print items");
   evas_object_smart_callback_add(bt, "clicked", _print_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", _clear_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select first item");
   evas_object_smart_callback_add(bt, "clicked", _select_first_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select last item");
   evas_object_smart_callback_add(bt, "clicked", _select_last_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select next item");
   evas_object_smart_callback_add(bt, "clicked", _select_next_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select previous item");
   evas_object_smart_callback_add(bt, "clicked", _select_prev_cb, ds);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);

   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
