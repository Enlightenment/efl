#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
changed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ck2 = data;
   printf("ck %p to %i\n", obj, elm_check_state_get(obj));
   elm_check_state_set(ck2, elm_check_state_get(obj));
   printf("ck2 %p is now %i\n", ck2, elm_check_state_get(ck2));
}

void
test_check(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ic, *ck, *ck0;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "checks", ELM_WIN_BASIC);
   elm_win_title_set(win, "Checks");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(ck, "Icon sized to check");
   elm_object_part_content_set(ck, "icon", ic);
   elm_check_state_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);
   evas_object_show(ic);
   
   ck0 = ck;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   ck = elm_check_add(win);
   elm_object_text_set(ck, "Icon no scale");
   elm_object_part_content_set(ck, "icon", ic);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);
   evas_object_show(ic);
   
   evas_object_smart_callback_add(ck, "changed", changed_cb, ck0);
   
   ck = elm_check_add(win);
   elm_object_text_set(ck, "Label Only");
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, EVAS_HINT_FILL, 0.5);
   elm_object_text_set(ck, "Disabled check");
   elm_object_part_content_set(ck, "icon", ic);
   elm_check_state_set(ck, 1);
   elm_box_pack_end(bx, ck);
   elm_object_disabled_set(ck, 1);
   evas_object_show(ck);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   ck = elm_check_add(win);
   elm_object_part_content_set(ck, "icon", ic);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);
   evas_object_show(ic);

   evas_object_show(win);
}
#endif
