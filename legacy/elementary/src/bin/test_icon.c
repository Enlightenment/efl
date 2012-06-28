#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
aspect_fixed_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_aspect_fixed_set(ic, elm_check_state_get(obj));
}

static void
fill_outside_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_fill_outside_set(ic, elm_check_state_get(obj));
}

static void
smooth_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *ic = (Evas_Object *)data;
   elm_image_smooth_set(ic, elm_check_state_get(obj));
}

static void
bt_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("preload-prescale", "Preload & Prescale");
   elm_win_autodel_set(win, EINA_TRUE);

   ic = elm_icon_add(win);
   elm_win_resize_object_add(win, ic);
   snprintf(buf, sizeof(buf), "%s/images/insanely_huge_test_image.jpg",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);

   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
   elm_image_aspect_fixed_set(ic, EINA_FALSE);
   elm_image_preload_disabled_set(ic, EINA_TRUE);
   elm_image_prescale_set(ic, EINA_TRUE);
   evas_object_show(ic);

   evas_object_resize(win, 350, 350);
   evas_object_show(win);
}

void
test_icon(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *box, *content_box, *hbox, *tg, *bt;
   win = elm_win_util_standard_add("icon test", "Icon Test");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_win_resize_object_add(win, box);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(box);

   content_box = elm_box_add(win);
   elm_win_resize_object_add(win, content_box);
   evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(content_box, EVAS_HINT_FILL,
                                   EVAS_HINT_FILL);
   elm_box_pack_end(box, content_box);
   evas_object_show(content_box);

   Evas_Object *ic = elm_icon_add(win);
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ic, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(content_box, ic);
   evas_object_show(ic);

   hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(content_box, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   elm_box_pack_end(box, hbox);
   evas_object_show(hbox);

   /* Test Aspect Fixed */
   tg = elm_check_add(win);
   elm_object_text_set(tg, "Aspect Fixed");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", aspect_fixed_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Fill Outside */
   tg = elm_check_add(win);
   elm_object_text_set(tg, "Fill Outside");
   evas_object_smart_callback_add(tg, "changed", fill_outside_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Smooth */
   tg = elm_check_add(win);
   elm_object_text_set(tg, "Smooth");
   elm_check_state_set(tg, EINA_TRUE);
   evas_object_smart_callback_add(tg, "changed", smooth_cb, ic);
   elm_box_pack_end(hbox, tg);
   evas_object_show(tg);

   /* Test Preload, Prescale */
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Preload & Prescale");
   evas_object_smart_callback_add(bt, "clicked", bt_clicked, NULL);
   elm_box_pack_end(hbox, bt);
   evas_object_show(bt);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

static void
icon_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("clicked!\n");
}

void
test_icon_transparent(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "icon-transparent", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Transparent");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_alpha_set(win, EINA_TRUE);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, 0, 0);
   elm_image_no_scale_set(ic, 1);
   evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(ic, 0.5, 0.5);
   elm_win_resize_object_add(win, ic);
   evas_object_show(ic);

   evas_object_smart_callback_add(ic, "clicked", icon_clicked, NULL);

   evas_object_show(win);
}
#endif
