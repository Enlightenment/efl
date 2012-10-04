/*
 * gcc -o web_example_01 web_example_01.c `pkg-config --cflags --libs elementary ewebkit` -D_GNU_SOURCE
 */

#define _GNU_SOURCE
#include <Elementary.h>
#ifdef HAVE_ELEMENTARY_WEB
#include <EWebKit.h>
#endif

#define URL "http://www.enlightenment.org"

EAPI_MAIN int
elm_main(int argc, char *argv[])
{
   Evas_Object *win, *web;

   /* The program will proceed only if Ewebkit library is available. */
   if (elm_need_web() == EINA_FALSE)
     return -1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   /* Window */
   win = elm_win_add(NULL, "Elementary Webkit Widget", ELM_WIN_BASIC);
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 720, 600);
   evas_object_show(win);

   /* Web */
   web = elm_web_add(win);
   evas_object_size_hint_weight_set(web, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_web_window_create_hook_set(web, NULL, NULL);
   elm_win_resize_object_add(win, web);
   elm_web_history_enabled_set(web, EINA_FALSE);

   if (!elm_web_uri_set(web, URL))
     {
        printf("URL NOT LOADED");
        return -1;
     }
   evas_object_show(web);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
