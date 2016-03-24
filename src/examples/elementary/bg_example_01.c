//Compile with:
//gcc -o bg_example_01 bg_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   win = elm_win_util_standard_add("bg-plain", "Bg Plain");
   elm_win_autodel_set(win, EINA_TRUE);

   /* and now just resize the window to a size you want. normally widgets
    * will determine the initial size though */
   evas_object_resize(win, 320, 320);
   /* and show the window */
   evas_object_show(win);

   elm_run(); /* and run the program now, starting to handle all
               * events, etc. */

   /* exit code */
   return 0;
}
ELM_MAIN()
