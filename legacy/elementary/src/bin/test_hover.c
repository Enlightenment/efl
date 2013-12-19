#include "elementary_config.h"
#include <Elementary.h>

static void
_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   printf("hover dismissed callback is called!\n");
}

static void
my_hover_bt(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *hv = data;

   evas_object_show(hv);
}

static void
_dismiss_hover(void *data, Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Evas_Object *hv = data;

   elm_hover_dismiss(hv);
}

void
test_hover(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *hv, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("hover", "Hover");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hv = elm_hover_add(win);
   evas_object_smart_callback_add(hv, "dismissed", _dismissed_cb, NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Popup");
   elm_object_part_content_set(hv, "middle", bt);
   evas_object_show(bt);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 1");
   evas_object_smart_callback_add(bt, "clicked", _dismiss_hover, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 2");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 3");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(bx);
   elm_object_part_content_set(hv, "top", bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bottom");
   elm_object_part_content_set(hv, "bottom", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   elm_object_part_content_set(hv, "left", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right");
   elm_object_part_content_set(hv, "right", bt);
   evas_object_show(bt);

   evas_object_resize(win, 440, 440);
   evas_object_show(win);
}

void
test_hover2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *hv, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("hover2", "Hover 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hv = elm_hover_add(win);
   elm_object_style_set(hv, "popout");

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   evas_object_smart_callback_add(bt, "clicked", my_hover_bt, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Popup");
   elm_object_part_content_set(hv, "middle", bt);
   evas_object_show(bt);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 1");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 2");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Top 3");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(bx);
   elm_object_part_content_set(hv, "top", bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Bot");
   elm_object_part_content_set(hv, "bottom", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   elm_object_part_content_set(hv, "left", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right");
   elm_object_part_content_set(hv, "right", bt);
   evas_object_show(bt);

   evas_object_resize(win, 440, 440);
   evas_object_show(win);
}

static void
_hover_show_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
               void *event_info)
{
   Evas_Object *fake_obj = evas_object_data_get(obj, "fake_obj");
   if (!fake_obj) return;

   Evas_Event_Mouse_Down *ev = event_info;
   printf("position x: %d, y: %d \n", ev->canvas.x, ev->canvas.y);

   evas_object_move(fake_obj, ev->canvas.x, ev->canvas.y);
   evas_object_show(data);
}

/*
 * hover acts like elm_menu but it has all the hover features such as:
 * 1. positioning: left, top-left, top, top-right, right, bottom-right, bottom,
 *                 bottom-left, middle
 * 2. content: one can set any object object as hover content
 */
void
test_hover3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
            void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fake_obj, *bx, *bt, *hv, *ic, *rect;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("hover3", "Hover 3");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, rect);
   evas_object_color_set(rect, 0, 0, 0, 0);
   evas_object_show(rect);

   // fake object to move hover object as we wish
   fake_obj = elm_box_add(win);
   evas_object_data_set(rect, "fake_obj", fake_obj);

   hv = elm_hover_add(win);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, fake_obj);
   elm_object_style_set(hv, "transparent");

   evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN,
                                  _hover_show_cb, hv);

   bx = elm_box_add(win);
   elm_object_part_content_set(hv, "bottom-right", bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _dismiss_hover, hv);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Close");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _dismiss_hover, hv);

   evas_object_resize(win, 440, 440);
   evas_object_show(win);
}
