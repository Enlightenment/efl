#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_icon_clicked_cb(void *data , Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *ic = data;
   Eina_Bool play = EINA_FALSE;

   play = !elm_image_animated_play_get(ic);
   printf("image clicked! play = %d\n", play);
   elm_image_animated_play_set(ic, play);
}

void
test_icon_animated(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *lbl, *ic;
   char buf[PATH_MAX];

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "icon-animated-gif", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Animated Gif");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   lbl = elm_label_add(bx);
   elm_object_text_set(lbl, "Clicking the image will play/pause animation.");
   elm_box_pack_end(bx, lbl);
   evas_object_show(lbl);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/animated_logo.gif", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   if (elm_image_animated_available_get(ic))
     {
        printf("animation is available for this image.\n");
        elm_image_animated_set(ic, EINA_TRUE);
        elm_image_animated_play_set(ic, EINA_TRUE);
     }
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_smart_callback_add(ic, "clicked", _icon_clicked_cb, ic);

   evas_object_show(win);
}
