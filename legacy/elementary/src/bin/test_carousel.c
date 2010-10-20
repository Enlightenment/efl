#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_carousel(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *car;

   win = elm_win_add(NULL, "carousel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Carousel");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   car = elm_carousel_add(win);
   elm_win_resize_object_add(win, car);
   evas_object_size_hint_weight_set(car, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_carousel_item_add(car, NULL, "Item 1", NULL, NULL);
   elm_carousel_item_add(car, NULL, "Item 2", NULL, NULL);
   elm_carousel_item_add(car, NULL, "Tinga", NULL, NULL);
   elm_carousel_item_add(car, NULL, "Item 4", NULL, NULL);
   evas_object_show(car);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}
#endif
