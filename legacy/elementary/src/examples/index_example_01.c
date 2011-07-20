/**
 * Simple Elementary's <b>index widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` index_example.c -o index_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

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
_index_item_del(void        *data,
                Evas_Object *obj __UNUSED__,
                void        *event_info)
{
   fprintf(stdout, "Deleting associated list node (%s). Comparing index "
                   "item data reported via callback with the one returned by "
                   "index's API on items: %s.\n",
           elm_list_item_label_get(data),
           data == elm_index_item_data_get(event_info) ? "OK" :
           "FAIL, something went wrong");

   elm_list_item_del(data);
}

/* delete an index item */
static void
_item_del(void        *data __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void        *event_info __UNUSED__)
{
   Elm_Index_Item *it;
   Elm_List_Item *lit = elm_index_item_selected_get(d.index, 0);

   it = elm_index_item_find(d.index, lit);

   if (!it) return;

   fprintf(stdout, "Deleting last selected index item, which had letter"
                    " %s (pointing to %s)\n", elm_index_item_letter_get(it),
           elm_list_item_label_get(lit));

   elm_index_item_del(d.index, lit);
}

static void
_item_del_all(void        *data __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void        *event_info __UNUSED__)
{
   elm_index_item_clear(d.index);
}

static void
_active_set(void        *data __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void        *event_info __UNUSED__)
{
   elm_index_active_set(d.index, !elm_index_active_get(d.index));

   fprintf(stdout, "Toggling index programmatically.\n");
}

/* "delay,changed" hook */
static void
_index_changed(void        *data __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void        *event_info)
{
   elm_list_item_bring_in(event_info);
}

static void
_index_selected(void        *data __UNUSED__,
                Evas_Object *obj,
                void        *event_info)
{
   Elm_List_Item *it = event_info;

   fprintf(stdout, "New index item selected. Comparing item reported"
                   " via callback with the selection returned by the API: "
                   "%s.\n", it == elm_index_item_selected_get(obj, 0) ? "OK" :
           "FAIL, something went wrong");
}

static void
_on_done(void        *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void        *event_info __UNUSED__)
{
   elm_exit();
}

int
elm_main(int    argc __UNUSED__,
         char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *hbox, *vbox, *bt, *sep;
   Elm_List_Item *it;
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
        it = elm_list_item_append(d.list, dict[i], NULL, NULL, NULL, NULL);

        if (curr != dict[i][0])
          {
             Elm_Index_Item *index_it;
             char buf[32];

             curr = dict[i][0];
             /* indexing by first letters */

             snprintf(buf, sizeof(buf), "%c", curr);
             elm_index_item_append(d.index, buf, it);
             index_it = elm_index_item_find(d.index, it);

             elm_index_item_del_cb_set(index_it, _index_item_del);
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
   return 0;
}

ELM_MAIN()
