#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
glow_effect_on_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *chk = evas_object_data_get(obj, "glow_is_enable");

   if (elm_check_state_get(chk)) elm_win_focus_highlight_style_set(data, "glow_effect");
}

static void
glow_effect_off_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *chk = evas_object_data_get(obj, "glow_is_enable");

   if (elm_check_state_get(chk)) elm_win_focus_highlight_style_set(data, "glow");
}

void
test_focus_style(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *bx, *bx2, *bt, *spinner, *sp, *chk;
   char win_focus_theme[PATH_MAX] = { 0 };

   sprintf(win_focus_theme, "%s/objects/test_focus_style.edj", elm_app_data_dir_get());
   elm_theme_extension_add(NULL, win_focus_theme);

   win = elm_win_util_standard_add("focus-style", "Focus Style");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);
   elm_win_focus_highlight_style_set(win, "glow");

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, fr);
   elm_object_style_set(fr, "pad_large");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   chk = elm_check_add(bx);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(chk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(chk, "Enable glow effect on \"Glow\" Button");
   elm_check_state_set(chk, EINA_TRUE);
   elm_box_pack_end(bx, chk);
   evas_object_show(chk);

   spinner = elm_spinner_add(bx);
   evas_object_size_hint_weight_set(spinner, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(spinner, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, spinner);
   evas_object_show(spinner);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Glow Button");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_smart_callback_add(bt, "focused", glow_effect_on_cb, win);
   evas_object_smart_callback_add(bt, "unfocused", glow_effect_off_cb, win);
   evas_object_data_set(bt, "glow_is_enable", chk);
   evas_object_show(bt);

   sp = elm_separator_add(bx);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   evas_object_size_hint_weight_set(sp, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sp, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   bx2 = elm_box_add(bx);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Button 4");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

void
test_focus_part(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *bx, *chk, *layout, *ic, *btn;
   char win_focus_theme[PATH_MAX] = {0}, buf[PATH_MAX] = {0};

   sprintf(win_focus_theme, "%s/objects/test_focus_style.edj", elm_app_data_dir_get());
   elm_theme_extension_add(NULL, win_focus_theme);

   win = elm_win_util_standard_add("focus_part", "Focus On Part");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, fr);
   elm_object_style_set(fr, "pad_large");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   chk = elm_check_add(bx);
   elm_object_style_set(chk, "focus_text");
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(chk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(chk, "Focus highlight on text part only");
   elm_check_state_set(chk, EINA_TRUE);
   elm_box_pack_end(bx, chk);
   evas_object_show(chk);

   layout = elm_layout_add(bx);
   evas_object_size_hint_weight_set(chk, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(chk, EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (!elm_layout_theme_set(layout, "layout", "focus", "example"))
     printf("error setting focus example layout\n");
   elm_box_pack_end(bx, layout);
   elm_object_focus_allow_set(layout, EINA_TRUE);
   evas_object_show(layout);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   btn = elm_button_add(win);
   elm_object_style_set(btn, "focus_icon");
   elm_object_text_set(btn, "Focus highlight on icon part only");
   elm_object_part_content_set(btn, "icon", ic);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   btn = elm_button_add(win);
   elm_object_text_set(btn, "Button");
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

void
test_focus_object_style(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *bx, *bt;
   char win_focus_theme[PATH_MAX] = { 0 };

   snprintf(win_focus_theme, sizeof(win_focus_theme), "%s/objects/test_focus_custom.edj", elm_app_data_dir_get());
   elm_theme_extension_add(NULL, win_focus_theme);

   win = elm_win_util_standard_add("object-focus-style", "Object Focus Style");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_focus_highlight_animate_set(win, EINA_TRUE);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, fr);
   elm_object_style_set(fr, "pad_large");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button Glow In Focus Style");
   elm_object_focus_highlight_style_set(bt, "glow");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 4");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
