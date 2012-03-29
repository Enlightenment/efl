/**
 * Simple Elementary's <b>list widget</b> example, illustrating its
 * creation.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g list_example_01.c -o list_example_01 `pkg-config --cflags --libs elementary`
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
   Evas_Object *win, *bg, *bx, *li;
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

   win = elm_win_add(NULL, "list", ELM_WIN_BASIC);
   elm_win_title_set(win, "List Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, li);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_list_item_append(li, lbl[i], NULL, NULL, NULL, NULL);

   evas_object_show(li);
   elm_list_go(li);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
