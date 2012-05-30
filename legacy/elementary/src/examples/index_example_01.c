/**
 * Simple Elementary's <b>index widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g index_example.c -o index_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>
#include <assert.h>

struct example_data
{
   Evas_Object *list, *index;
};

struct example_data d = {NULL, NULL};

static const char *dict[] = \
{
#include "dict.txt"
};

static void
_index_item_del(void *data,
                Evas_Object *obj,
                void *event_info)
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
_item_del(void *data,
          Evas_Object *obj,
          void *event_info)
{
   Elm_Object_Item *iit = elm_index_selected_item_get(d.index, 0);

   if (!iit) return;

   fprintf(stdout, "Deleting last selected index item, which had letter"
                   " %s (pointing to %p)\n", elm_index_item_letter_get(iit),
           elm_object_item_data_get(iit));

   elm_object_item_del(iit);
}

static void
_item_del_all(void *data,
              Evas_Object *obj,
              void *event_info)
{
   elm_index_item_clear(d.index);
}

static void
_active_set(void *data,
            Evas_Object *obj,
            void *event_info)
{
   elm_index_autohide_disabled_set
     (d.index, !elm_index_autohide_disabled_get(d.index));

   fprintf(stdout, "Toggling index programmatically.\n");
}

/* "delay,changed" hook */
static void
_index_changed(void *data,
               Evas_Object *obj,
               void *event_info)
{
   elm_list_item_bring_in(elm_object_item_data_get(event_info));
}

static void
_index_selected(void *data,
                Evas_Object *obj,
                void *event_info)
{
   Elm_Object_Item *lit = event_info;

   fprintf(stdout, "New index item selected. Comparing item reported"
                   " via callback with the selection returned by the API: "
                   "%s.\n", lit == elm_index_selected_item_get(obj, 0) ? "OK" :
           "FAIL, something went wrong");
}

static void
_on_done(void *data,
         Evas_Object *obj,
         void *event_info)
{
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc,
         char **argv)
{
   Evas_Object *win, *bg, *hbox, *vbox, *bt, *sep;
   Elm_Object_Item *lit;
   unsigned int i;
   char curr = 0;

   win = elm_win_add(NULL, "index", ELM_WIN_BASIC);
   elm_win_title_set(win, "Index Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   vbox = elm_box_add(win);
   elm_win_resize_object_add(win, vbox);
   evas_object_size_hint_weight_set(vbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(vbox);

   d.list = elm_list_add(win);
   evas_object_size_hint_weight_set(d.list, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(d.list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(d.list);
   elm_box_pack_end(vbox, d.list);

   d.index = elm_index_add(win);
   elm_win_resize_object_add(win, d.index);
   evas_object_size_hint_weight_set(d.index, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(d.list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(d.index);

   for (i = 0; i < (sizeof(dict) / sizeof(dict[0])); i++)
     {
        lit = elm_list_item_append(d.list, dict[i], NULL, NULL, NULL, NULL);

        if (curr != dict[i][0])
          {
             Elm_Object_Item *index_it, *it;
             char buf[32];

             curr = dict[i][0];
             /* indexing by first letters */

             snprintf(buf, sizeof(buf), "%c", curr);
             index_it = elm_index_item_append(d.index, buf, NULL, lit);

             /* this is here just to demostrate the API call */
             it = elm_index_item_find(d.index, lit);
             assert(it == index_it);

             elm_object_item_del_cb_set(index_it, _index_item_del);
          }
     }

   evas_object_smart_callback_add(d.index, "delay,changed", _index_changed,
                                  NULL);
   evas_object_smart_callback_add(d.index, "selected", _index_selected, NULL);

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
   evas_object_smart_callback_add(bt, "clicked", _active_set, NULL);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "delete last selected item");
   evas_object_smart_callback_add(bt, "clicked", _item_del, NULL);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "delete all items");
   evas_object_smart_callback_add(bt, "clicked", _item_del_all, NULL);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 600);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}

ELM_MAIN()
