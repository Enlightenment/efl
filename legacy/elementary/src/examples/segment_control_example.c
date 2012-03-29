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

static void
_on_done(void *data,
        Evas_Object *obj,
        void *event_info)
{
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *sc, *ic;
   Elm_Object_Item *seg_it;
   int count, idx;

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
   elm_shutdown();

   return 0;
}
ELM_MAIN()
