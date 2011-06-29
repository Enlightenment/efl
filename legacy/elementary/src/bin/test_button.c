#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
_bt_repeated(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   static int count;
   char buf[16];

   snprintf(buf, sizeof(buf), "count=%d", count++);
   if (count >= 10000)
     count = 0;
   elm_object_text_set(obj, buf);
}

static void
_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int param = (int)(data);

   printf("clicked event on Button:%d\n", param);
}

static void
_bt_unpressed(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   int param = (int)(data);

   printf("unpressed event on Button:%d\n", param);
}

void
test_button(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *scr, *bx, *ic, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "buttons", ELM_WIN_BASIC);
   elm_win_title_set(win, "Buttons");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   scr = elm_scroller_add(win);
   elm_scroller_bounce_set(scr, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(scr, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
   elm_win_resize_object_add(win, scr);
   evas_object_show(scr);
   evas_object_size_hint_weight_set(scr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_scroller_content_set(scr, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Icon sized to button");
   elm_button_icon_set(bt, ic);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)1);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)1);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(bx);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Icon no scale");
   elm_button_icon_set(bt, ic);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)2);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)2);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Disabled Button");
   elm_button_icon_set(bt, ic);
   elm_object_disabled_set(bt, 1);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)3);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)3);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_button_icon_set(bt, ic);
   elm_object_disabled_set(bt, 1);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)4);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)4);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Label Only");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)5);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)5);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "repeated", _bt_repeated, NULL);
   elm_button_autorepeat_set(bt, 1);
   elm_button_autorepeat_initial_timeout_set(bt, 2.0);
   elm_button_autorepeat_gap_timeout_set(bt, 0.5);
   evas_object_show(bt);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   bt = elm_button_add(win);
   elm_button_icon_set(bt, ic);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)6);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)6);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_object_style_set(bt, "anchor");
   elm_object_text_set(bt, "Anchor style");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)7);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)7);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_style_set(bt, "anchor");
   elm_button_icon_set(bt, ic);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)8);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)8);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_object_style_set(bt, "anchor");
   elm_button_icon_set(bt, ic);
   elm_object_disabled_set(bt, 1);
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, (void *)9);
   evas_object_smart_callback_add(bt, "unpressed", _bt_unpressed, (void *)9);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);
   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
