/**
 * Simple Elementary's <b>segment_control widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g segment_control_example.c -o segment_control_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *sc, *ic;
   Elm_Object_Item *seg_it;
   int count, idx;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("segment_control", "Segment Control Example");
   elm_win_autodel_set(win, EINA_TRUE);

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

   seg_it = elm_segment_control_item_insert_at(sc, NULL, "To be deleted", 2);
   elm_object_item_del(seg_it);

   elm_segment_control_item_insert_at(sc, NULL, "To be deleted", 2);
   elm_segment_control_item_del_at(sc, 2);

   seg_it = elm_segment_control_item_get(sc, 0);
   elm_object_item_text_set(seg_it, "Only Text");

   count = elm_segment_control_item_count_get(sc);
   seg_it = elm_segment_control_item_get(sc, count / 2);
   elm_segment_control_item_selected_set(seg_it, EINA_TRUE);
   seg_it = elm_segment_control_item_selected_get(sc);
   idx = elm_segment_control_item_index_get(seg_it);
   printf("Item selected: %i\n", idx);

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
