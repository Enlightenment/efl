/**
 * Elementary's <b>list widget</b> example, illustrating its API,
 * covering most of item functions.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g list_example_03.c -o list_example_03 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static int counter = 3;

static void
_on_done(void        *data,
         Evas_Object *obj,
         void        *event_info)
{
   elm_exit();
}

static void
_prepend_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it;
   Evas_Object *li = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   list_it = elm_list_item_prepend(li, label, NULL, NULL, NULL, NULL);
   if (!list_it)
     printf("Error adding item\n");
}

static void
_add_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it;
   Evas_Object *li = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   list_it = elm_list_item_append(li, label, NULL, NULL, NULL, NULL);
   if (!list_it)
     printf("Error adding item\n");
}

static void
_add_ic_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it;
   Evas_Object *ic, *li = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   ic = elm_icon_add(li);
   elm_icon_standard_set(ic, "home");
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);

   list_it = elm_list_item_append(li, label, ic,  NULL, NULL, NULL);
   if (!list_it)
     printf("Error adding item with icon\n");
}

static void
_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it = event_info;
   printf("Selected label: %s\n", elm_object_item_text_get(list_it));
}

static void
_add_func_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it;
   Evas_Object *li = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   list_it = elm_list_item_append(li, label, NULL, NULL, _sel_cb, NULL);
   if (!list_it)
     printf("Error adding item\n");
}

static void
_sel_data_cb(void *data, Evas_Object *obj, void *event_info)
{
   char *content = data;
   Elm_Object_Item *list_it = event_info;
   printf("Selected label: %s with data: %s\n",
          elm_object_item_text_get(list_it), content);
}

static void
_free_data(void *data, Evas_Object *obj, void *event_info)
{
   free(data);
}

static void
_add_data_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *list_it;
   Evas_Object *li = data;
   char label[32];
   char *content = malloc(sizeof(char) * 32);

   snprintf(content, 32, "Item content %i", counter);
   snprintf(label, sizeof(label), "Item %i", counter++);
   list_it = elm_list_item_append(li, label, NULL, NULL, _sel_data_cb, content);
   if (!list_it) {
     printf("Error adding item\n");
     return;
   }
   elm_object_item_del_cb_set(list_it, _free_data);
}

static void
_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item;
   Evas_Object *li = data;

   selected_item = elm_list_selected_item_get(li);
   elm_object_item_del(selected_item);
}

static void
_unselect_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item;
   Evas_Object *li = data;

   selected_item = elm_list_selected_item_get(li);
   elm_list_item_selected_set(selected_item, EINA_FALSE);
}

static void
_print_cb(void *data, Evas_Object *obj, void *event_info)
{
   const Eina_List *l, *items;
   Elm_Object_Item *list_it;
   Evas_Object *li = data;

   items = elm_list_items_get(li);
   EINA_LIST_FOREACH(items, l, list_it)
     printf("%s\n", elm_object_item_text_get(list_it));
}

static void
_clear_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *li = data;
   elm_list_clear(li);
}

static void
_select_next_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item, *next_item;
   Evas_Object *li = data;

   selected_item = elm_list_selected_item_get(li);
   if (!selected_item) return;

   next_item = elm_list_item_next(selected_item);
   if (next_item)
     elm_list_item_selected_set(next_item, EINA_TRUE);
}

static void
_insert_after_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item, *list_it;
   Evas_Object *li = data;
   char label[32];

   selected_item = elm_list_selected_item_get(li);
   if (!selected_item) return;

   snprintf(label, sizeof(label), "Item %i", counter++);
   list_it = elm_list_item_insert_after(li, selected_item, label, NULL, NULL,
                                        NULL, NULL);
   if (!list_it)
     printf("Error adding item\n");
}

static void
_select_prev_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item, *prev_item;
   Evas_Object *li = data;

   selected_item = elm_list_selected_item_get(li);
   if (!selected_item) return;

   prev_item = elm_list_item_prev(selected_item);
   if (prev_item)
     elm_list_item_selected_set(prev_item, EINA_TRUE);
}

static void
_insert_before_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item, *list_it;
   Evas_Object *li = data;
   char label[32];

   selected_item = elm_list_selected_item_get(li);
   if (!selected_item) return;

   snprintf(label, sizeof(label), "Item %i", counter++);
   list_it = elm_list_item_insert_before(li, selected_item, label, NULL, NULL,
                                    NULL, NULL);
   if (!list_it)
     printf("Error adding item\n");
}

static void
_set_separator_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item;
   Evas_Object *li = data;

   selected_item = elm_list_selected_item_get(li);
   if (!selected_item) return;
   elm_list_item_separator_set(selected_item, EINA_TRUE);
}

static void
_disable_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_item;
   Evas_Object *li = data;

   selected_item = elm_list_selected_item_get(li);
   if (!selected_item) return;
   elm_object_item_disabled_set(selected_item, EINA_TRUE);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *hbx, *li, *bt;

   win = elm_win_add(NULL, "list", ELM_WIN_BASIC);
   elm_win_title_set(win, "List Items Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, li);

   elm_list_item_append(li, "Item 0", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Item 1", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Item 2", NULL, NULL, NULL, NULL);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Prepend item");
   evas_object_smart_callback_add(bt, "clicked", _prepend_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append item");
   evas_object_smart_callback_add(bt, "clicked", _add_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append with icon");
   evas_object_smart_callback_add(bt, "clicked", _add_ic_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append with selected function");
   evas_object_smart_callback_add(bt, "clicked", _add_func_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Append with data");
   evas_object_smart_callback_add(bt, "clicked", _add_data_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, hbx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete item");
   evas_object_smart_callback_add(bt, "clicked", _del_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unselect item");
   evas_object_smart_callback_add(bt, "clicked", _unselect_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Print items");
   evas_object_smart_callback_add(bt, "clicked", _print_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", _clear_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, hbx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select next item");
   evas_object_smart_callback_add(bt, "clicked", _select_next_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert after item");
   evas_object_smart_callback_add(bt, "clicked", _insert_after_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select previous item");
   evas_object_smart_callback_add(bt, "clicked", _select_prev_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Insert before item");
   evas_object_smart_callback_add(bt, "clicked", _insert_before_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, 0);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Set as separator");
   evas_object_smart_callback_add(bt, "clicked", _set_separator_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Disable item");
   evas_object_smart_callback_add(bt, "clicked", _disable_cb, li);
   elm_box_pack_end(hbx, bt);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0);
   evas_object_show(bt);

   elm_list_go(li);
   evas_object_show(li);

   evas_object_resize(win, 320, 600);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
