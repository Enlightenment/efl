/**
 * Elementary's <b>diskselector widget</b> example, illustrating its
 * covering most of item functions.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g diskselector_example_02.c -o diskselector_example_02 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static int counter = 3;

static void
_ds_selected_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   Elm_Object_Item *it = event_info;
   printf("Selected Item %s\n", elm_object_item_text_get(it));
}

static void
_add_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *ds_it;
   Evas_Object *ds = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   ds_it = elm_diskselector_item_append(ds, label, NULL, NULL, NULL);
   if (!ds_it) printf("Error adding item\n");
}

static void
_add_ic_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *ds_it;
   Evas_Object *ic, *ds = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   ic = elm_icon_add(ds);
   elm_icon_standard_set(ic, "home");

   ds_it = elm_diskselector_item_append(ds, label, ic, NULL, NULL);
   if (!ds_it) printf("Error adding item with icon\n");
}

static void
_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *ds_it = event_info;
   printf("Selected label: %s\n", elm_object_item_text_get(ds_it));
}

static void
_add_func_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *ds_it;
   Evas_Object *ds = data;
   char label[32];

   snprintf(label, sizeof(label), "Item %i", counter++);
   ds_it = elm_diskselector_item_append(ds, label, NULL, _sel_cb, NULL);
   if (!ds_it) printf("Error adding item\n");
}

static void
_sel_data_cb(void *data, Evas_Object *obj, void *event_info)
{
   char *content = data;
   Elm_Object_Item *ds_it = event_info;
   printf("Selected label: %s with data: %s\n",
          elm_object_item_text_get(ds_it), content);
}

static void
_free_data(void *data, Evas_Object *obj, void *event_info)
{
   free(data);
}

static void
_add_data_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *ds_it;
   Evas_Object *ds = data;
   char label[32];
   char *content = malloc(sizeof(char) * 32);

   snprintf(content, 32, "Item content %i", counter);
   snprintf(label, sizeof(label), "Item %i", counter++);
   ds_it = elm_diskselector_item_append(ds, label, NULL, _sel_data_cb, content);
   if (!ds_it)
     {
        printf("Error adding item\n");
        return;
     }
   elm_object_item_del_cb_set(ds_it, _free_data);
}

static void
_del_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_ds_it;
   Evas_Object *ds = data;

   selected_ds_it = elm_diskselector_selected_item_get(ds);
   elm_object_item_del(selected_ds_it);
}

static void
_unselect_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_ds_it;
   Evas_Object *ds = data;

   selected_ds_it = elm_diskselector_selected_item_get(ds);
   elm_diskselector_item_selected_set(selected_ds_it, EINA_FALSE);
}

static void
_print_cb(void *data, Evas_Object *obj, void *event_info)
{
   const Eina_List *l, *items;
   Elm_Object_Item *ds_it;
   Evas_Object *ds = data;

   items = elm_diskselector_items_get(ds);
   EINA_LIST_FOREACH(items, l, ds_it)
     printf("%s\n", elm_object_item_text_get(ds_it));
}

static void
_clear_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *ds = data;
   elm_diskselector_clear(ds);
}

static void
_select_first_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *first_ds_it;
   Evas_Object *ds = data;

   first_ds_it = elm_diskselector_first_item_get(ds);
   if (first_ds_it)
     elm_diskselector_item_selected_set(first_ds_it, EINA_TRUE);
}

static void
_select_last_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *last_ds_it;
   Evas_Object *ds = data;

   last_ds_it = elm_diskselector_last_item_get(ds);
   if (last_ds_it)
     elm_diskselector_item_selected_set(last_ds_it, EINA_TRUE);
}

static void
_select_next_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_ds_it, *next_ds_it;
   Evas_Object *ds = data;

   selected_ds_it = elm_diskselector_selected_item_get(ds);
   if (!selected_ds_it) return;

   next_ds_it = elm_diskselector_item_next_get(selected_ds_it);
   if (next_ds_it)
     elm_diskselector_item_selected_set(next_ds_it, EINA_TRUE);
}

static void
_select_prev_cb(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *selected_ds_it, *prev_ds_it;
   Evas_Object *ds = data;

   selected_ds_it = elm_diskselector_selected_item_get(ds);
   if (!selected_ds_it) return;

   prev_ds_it = elm_diskselector_item_prev_get(selected_ds_it);
   if (prev_ds_it)
     elm_diskselector_item_selected_set(prev_ds_it, EINA_TRUE);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bx, *hbx, *ds, *bt;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("diskselector", "Diskselector Items Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ds = elm_diskselector_add(win);
   evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(ds, "selected", _ds_selected_cb, NULL);
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
   elm_shutdown();

   return 0;
}
ELM_MAIN()
