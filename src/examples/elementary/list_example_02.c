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

   win = elm_win_util_standard_add("list", "List Example");
   elm_win_autodel_set(win, EINA_TRUE);

   /* default */
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, li);

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

   return 0;
}
ELM_MAIN()
