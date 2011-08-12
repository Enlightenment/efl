#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
icon_clicked(void *data , Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ic;
   ic = data;
   Eina_Bool rec;
   rec = elm_icon_animated_play_get(ic);
   rec = !rec;
   printf("clicked!rec =%d\n",rec);
   elm_icon_animated_play_set(ic, rec);

}

void
test_icon_animated(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "icon-animated-gif", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Animated Gif");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_alpha_set(win, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/animated_logo.gif", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   if (elm_icon_animated_available_get(ic))
     {
        printf("============Support animator==============\n");
     }
   elm_icon_animated_set(ic, TRUE);
   elm_icon_animated_play_set(ic, TRUE);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_no_scale_set(ic, 1);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ic, 0.5, 0.5);
   elm_win_resize_object_add(win, ic);
   evas_object_show(ic);

   evas_object_smart_callback_add(ic, "clicked", icon_clicked, ic);

   evas_object_show(win);
}
#endif
