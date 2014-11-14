/*
 * gcc -o button_example_00 button_example_00.c `pkg-config --cflags --libs elementary`
 */
#include <Elementary.h>

static void
on_click(void *data, Evas_Object *obj, void *event_info)
{
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win = NULL;
   Evas_Object *btn = NULL;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   /* Create an win, associate it with a canvas and */
   /* turn it visible on WM (Window Manager).       */
   win = elm_win_util_standard_add("Greetings", "Hello, World!");
   elm_win_autodel_set(win, EINA_TRUE);

   /* Create a btn, associate to a function, associate */
   /* to win,  give a dimension and position.          */
   btn = elm_button_add(win);
   elm_object_text_set(btn, "Good-Bye, World!");
   evas_object_smart_callback_add(btn, "clicked", on_click, NULL);
   evas_object_resize(btn, 120, 30);
   evas_object_move(btn, 60, 15);
   evas_object_show(btn);

   evas_object_resize(win, 240, 60);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
