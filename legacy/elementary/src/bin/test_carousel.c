#include <Elementary.h>

void
test_carousel(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "carousel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Carousel");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}
