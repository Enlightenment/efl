/**
 * Elementary's <b>list widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g list_example_02.c -o list_example_02 `pkg-config --cflags --libs elementary`
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

   /* default */
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, li);
   for (i = 0; i < sizeof(lbl) / sizeof(lbl[0]); i++)
     elm_list_item_append(li, lbl[i], NULL, NULL, NULL, NULL);

   /* display horizontally and set compress mode */
   elm_list_horizontal_set(li, EINA_TRUE);
   elm_list_mode_set(li, ELM_LIST_COMPRESS);

   /* enable multiple selection and always select */
   elm_list_multi_select_set(li, EINA_TRUE);
   elm_list_select_mode_set(li, ELM_OBJECT_SELECT_MODE_ALWAYS);

   /* set bounce and scroller policy */
   elm_scroller_bounce_set(li, EINA_TRUE, EINA_TRUE);
   elm_scroller_policy_set(li, ELM_SCROLLER_POLICY_AUTO,
                                ELM_SCROLLER_POLICY_ON);

   elm_list_go(li);
   evas_object_show(li);

   evas_object_resize(win, 320, 120);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
