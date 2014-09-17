/**
 * Simple Elementary's <b>index widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g index_example_01.c -o index_example_01 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>
#include <assert.h>

static const char *dict[] = \
{
#include "dict.txt"
};

static void
_index_item_del(void *data, Evas_Object *obj, void *event_info)
{
   fprintf(stdout, "Deleting index node (%s). Comparing index "
           "item data reported via callback with the one returned by "
           "index's API on items: %s.\n",
           elm_index_item_letter_get(event_info),
           data == elm_object_item_data_get(event_info) ? "OK" :
           "FAIL, something went wrong");
}

/* delete an index item */
static void
_item_del(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *it = elm_index_selected_item_get(data, 0);

   if (!it) return;

   fprintf(stdout, "Deleting last selected index item, which had letter"
           " %s (pointing to %p)\n", elm_index_item_letter_get(it),
           elm_object_item_data_get(it));

   elm_object_item_del(it);
   elm_index_level_go(data, 0);
}

static void
_item_del_all(void *data, Evas_Object *obj, void *event_info)
{
   elm_index_item_clear(data);
   elm_index_level_go(data, 0);
}

static void
_active_set(void *data, Evas_Object *obj, void *event_info)
{
   Eina_Bool disabled = elm_index_autohide_disabled_get(data);
   elm_index_autohide_disabled_set(data, !disabled);

   fprintf(stdout, "Toggling index programmatically to %s.\n",
           !disabled ? "On" : "Off");
}

/* "delay,changed" hook */
static void
_index_changed(void *data, Evas_Object *obj, void *event_info)
{
   elm_list_item_bring_in(elm_object_item_data_get(event_info));
}

static void
_index_selected(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Object_Item *lit = event_info;

   fprintf(stdout, "New index item selected. Comparing item reported"
           " via callback with the selection returned by the API: "
           "%s.\n", lit == elm_index_selected_item_get(obj, 0) ? "OK" :
           "FAIL, something went wrong");
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *hbox, *vbox, *bt, *sep, *list, *id;
   Elm_Object_Item *lit;
   unsigned int i;
   char curr = 0;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("index", "Index Example");
   elm_win_autodel_set(win, EINA_TRUE);

   vbox = elm_box_add(win);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, vbox);
   evas_object_show(vbox);

   list = elm_list_add(win);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(list);
   elm_box_pack_end(vbox, list);

   id = elm_index_add(win);
   evas_object_size_hint_weight_set(id, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, id);
   evas_object_show(id);

   for (i = 0; i < (sizeof(dict) / sizeof(dict[0])); i++)
     {
        lit = elm_list_item_append(list, dict[i], NULL, NULL, NULL, NULL);

        if (curr != dict[i][0])
          {
             Elm_Object_Item *index_it;
             char buf[32];

             curr = dict[i][0];
             /* indexing by first letters */

             snprintf(buf, sizeof(buf), "%c", curr);
             index_it = elm_index_item_append(id, buf, NULL, lit);

             /* this is here just to demostrate the API call */
             (void)elm_index_item_find(id, lit);
             assert(elm_index_item_find(id, lit) == index_it);

             elm_object_item_del_cb_set(index_it, _index_item_del);
          }
     }

   evas_object_smart_callback_add(id, "delay,changed", _index_changed,
                                  NULL);
   evas_object_smart_callback_add(id, "selected", _index_selected, NULL);
   elm_index_level_go(id, 0);

   /* attribute setting knobs */
   sep = elm_separator_add(win);
   elm_separator_horizontal_set(sep, EINA_TRUE);
   elm_box_pack_end(vbox, sep);
   evas_object_show(sep);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0);
   evas_object_size_hint_fill_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(vbox, hbox);
   evas_object_show(hbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "bring in index");
   evas_object_smart_callback_add(bt, "clicked", _active_set, id);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "delete last selected item");
   evas_object_smart_callback_add(bt, "clicked", _item_del, id);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "delete all items");
   evas_object_smart_callback_add(bt, "clicked", _item_del_all, id);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 600);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
