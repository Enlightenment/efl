#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_print_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("bubble clicked\n");
}

void
test_bubble(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ic, *bb, *ct;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-vert", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bubble");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 1");
   elm_object_text_part_set(bb, "info", "Corner: bottom_right");
   elm_bubble_icon_set(bb, ic);
   elm_bubble_corner_set(bb, "bottom_right");
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ct = elm_label_add(win);
   elm_object_text_set(ct,
                       "\"The future of the art: R or G or B?\",  by Rusty");
   elm_bubble_content_set(bb, ct);

   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 2");
   elm_object_text_part_set(bb, "info", "10:32 4/11/2008");
   evas_object_smart_callback_add(bb, "clicked", _print_clicked, NULL);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ct = elm_label_add(win);
   elm_object_text_set(ct, "Corner: base (top-left) - no icon");
   elm_bubble_content_set(bb, ct);

   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   evas_object_show(win);
}
#endif
