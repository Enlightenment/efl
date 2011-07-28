/**
 * Simple Elementary's <b>segment_control widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` segment_control_example.c -o segment_control_example
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

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *sc, *ic;
   Elm_Segment_Item *it;
   int count, index;

   win = elm_win_add(NULL, "segment_control", ELM_WIN_BASIC);
   elm_win_title_set(win, "Segment Control Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   sc = elm_segment_control_add(win);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, sc);
   evas_object_show(sc);

   elm_segment_control_item_add(sc, NULL, "only text");

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "close");
   elm_segment_control_item_add(sc, ic, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "home");
   elm_segment_control_item_add(sc, ic, "Home");

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "close");
   elm_segment_control_item_add(sc, ic, "");

   count = elm_segment_control_item_count_get(sc);
   elm_segment_control_item_insert_at(sc, NULL, "Inserted at", count - 1);

   it = elm_segment_control_item_insert_at(sc, NULL, "To be deleted", 2);
   elm_segment_control_item_del(it);

   elm_segment_control_item_insert_at(sc, NULL, "To be deleted", 2);
   elm_segment_control_item_del_at(sc, 2);

   it = elm_segment_control_item_get(sc, 0);
   elm_segment_control_item_label_set(it, "Only Text");

   count = elm_segment_control_item_count_get(sc);
   it = elm_segment_control_item_get(sc, count / 2);
   elm_segment_control_item_selected_set(it, EINA_TRUE);
   it = elm_segment_control_item_selected_get(sc);
   index = elm_segment_control_item_index_get(it);
   printf("Item selected: %i\n", index);

   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
