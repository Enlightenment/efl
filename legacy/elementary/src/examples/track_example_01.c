/**
 * Simple Elementary's <b>object item track</b> example, illustrating its
 * creation.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g track_example_01.c -o track_example_01 `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

void
item_select_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Coord x, y, w, h;
   Eina_Bool visible;
   Elm_Object_Item *it = event_info;
   Evas_Object *track = elm_object_item_track(it);
   evas_object_geometry_get(track, &x, &y, &w, &h);

   visible = evas_object_visible_get(track);
   printf("track geometry = (%d %d %d %d) visible(%d)\n",
          x, y, w, h, visible);

   elm_object_item_untrack(it);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *li;
   unsigned int i;
   static const char *lbl[] =
     {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
     };

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("track-example", "Track Example");
   elm_win_autodel_set(win, EINA_TRUE);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, li);

   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_list_item_append(li, lbl[i], NULL, NULL, item_select_cb, NULL);

   evas_object_show(li);
   elm_list_go(li);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
