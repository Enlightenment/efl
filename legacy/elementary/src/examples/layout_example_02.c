//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` layout_example_02.c -o layout_example_02

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#define TABLE "example/table"
#define BOX "example/box"
#define TITLE "example/title"
#define SWALLOW "example/custom"

static const char *images[] = { "home", "close", "arrow_up", "arrow_down", NULL };

struct _App {
     int current;
};

static void
_signal_cb(void *data, Evas_Object *o, const char *emission, const char *source __UNUSED__)
{
   struct _App *app = data;
   Evas_Object *icon = elm_object_content_part_get(o, "elm.swallow.content");

   printf("signal received\n");

   if (!strcmp("elm,action,back", emission))
     app->current--;
   else if (!strcmp("elm,action,next", emission))
     app->current++;

   if (app->current < 0)
     app->current = sizeof(images) - 1;
   else if (images[app->current] == NULL)
     app->current = 0;

   elm_icon_standard_set(icon, images[app->current]);
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *layout, *icon;
   struct _App app;

   app.current = 0;

   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);
   elm_win_title_set(win, "Layout");
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_bg_color_set(bg, 255, 255, 255);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   // Adding layout and filling it with widgets
   layout = elm_layout_add(win);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, layout);
   elm_layout_theme_set(
       layout, "layout", "application", "content-back-next");
   evas_object_show(layout);

   icon = elm_icon_add(win);
   elm_icon_standard_set(icon, images[app.current]);
   evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_part_set(layout, "elm.swallow.content", icon);

   elm_object_signal_callback_add(layout, "elm,action,back", "", _signal_cb, &app);
   elm_object_signal_callback_add(layout, "elm,action,next", "", _signal_cb, &app);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
