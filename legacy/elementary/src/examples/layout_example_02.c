//Compile with:
//gcc -g layout_example_02.c -o layout_example_02 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

#define TABLE "example/table"
#define BOX "example/box"
#define TITLE "example/title"
#define SWALLOW "example/custom"

static const char *images[] = { "home", "close", "arrow_up", "arrow_down", NULL };

struct _App {
     int current;
};

static void
_signal_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   struct _App *app = data;
   Evas_Object *icon = elm_object_part_content_get(o, "elm.swallow.content");

   printf("signal received\n");

   if (!strcmp("elm,action,back", emission))
     app->current--;
   else if (!strcmp("elm,action,next", emission))
     app->current++;

   if (app->current < 0)
     app->current = (sizeof(images) / sizeof(images[0])) - 2;
   else if (images[app->current] == NULL)
     app->current = 0;

   elm_icon_standard_set(icon, images[app->current]);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *layout, *icon;
   struct _App app;

   app.current = 0;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("layout", "Layout");
   elm_win_autodel_set(win, EINA_TRUE);

   // Adding layout and filling it with widgets
   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   if (!elm_layout_theme_set(
         layout, "layout", "application", "content-back-next"))
     fprintf(stderr, "Failed to set layout");
   evas_object_show(layout);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, images[app.current]);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(layout, "elm.swallow.content", icon);

   elm_object_signal_callback_add(layout, "elm,action,back", "elm", _signal_cb, &app);
   elm_object_signal_callback_add(layout, "elm,action,next", "elm", _signal_cb, &app);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();
   elm_shutdown();

   return 0;
}
ELM_MAIN()
