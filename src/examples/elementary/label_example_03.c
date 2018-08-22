//Compile with:
//gcc -g label_example_03.c -o label_example_03 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

typedef struct _appdata_s
{
   Evas_Object *style;
   Evas_Object *label;
} appdata_s;

static void
_theme_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   appdata_s *ad = data;

   if (!strcmp("slide_long", elm_object_text_get(ad->style)))
      elm_object_text_set(ad->style, "slide_short");
   else if (!strcmp("slide_short", elm_object_text_get(ad->style)))
      elm_object_text_set(ad->style, "slide_bounce");
   else
      elm_object_text_set(ad->style, "slide_long");

   elm_object_style_set(ad->label, elm_object_text_get(ad->style));
}

static void
_win_del_request_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   appdata_s *ad = data;
   free(ad);
}

static void
_slide_start_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *label = data;

   elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_label_slide_go(label);
}

static void
_slide_stop_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *label = data;

   elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);
   elm_label_slide_go(label);
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char** argv EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx1, *bx2, *style, *label, *btn;
   appdata_s *ad = (appdata_s*)calloc(1, sizeof(appdata_s));

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   // Win
   win = elm_win_util_standard_add("label", "Label Theme Change");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 85);
   evas_object_show(win);
   evas_object_smart_callback_add(win, "delete,request", _win_del_request_cb, ad);

   // Box
   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);
   elm_win_resize_object_add(win, bx);

   // Box1 for label
   bx1 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx1, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, bx1);
   evas_object_show(bx1);

   // Label for showing style
   style = elm_label_add(bx1);
   elm_object_text_set(style, "slide_long");
   elm_box_pack_end(bx1, style);
   evas_object_show(style);

   ad->style = style;

   // Label
   label = elm_label_add(bx1);
   elm_object_style_set(label, "slide_long");
   elm_object_text_set(label, "This is a label widget, you can change its theme or whether slide start or stop.");
   elm_label_wrap_width_set(label, 200);
   elm_label_slide_duration_set(label, 4.0);
   elm_box_pack_end(bx1, label);
   evas_object_show(label);

   ad->label = label;

   // Box2 for button
   bx2 = elm_box_add(bx);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   // Button
   btn = elm_button_add(bx2);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(btn, "Change");
   elm_box_pack_end(bx2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _theme_change_cb, ad);

   btn = elm_button_add(bx2);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(btn, "Change");
   elm_object_text_set(btn, "Go");
   elm_box_pack_end(bx2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _slide_start_cb, label);

   btn = elm_button_add(bx2);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_text_set(btn, "Change");
   elm_object_text_set(btn, "Stop");
   elm_box_pack_end(bx2, btn);
   evas_object_show(btn);
   evas_object_smart_callback_add(btn, "clicked", _slide_stop_cb, label);

   elm_run();

   return 0;
}
ELM_MAIN()
