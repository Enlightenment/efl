#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

static Evas_Object *boxes[7] = {};

typedef enum {
   NONE,
   NONE_BUT_FILL,
   EQUAL,
   ONE,
   TWO
} Weight_Mode;

#define P(i) ((void*)(intptr_t)i)
#define I(p) ((int)(intptr_t)p)

static Eina_Bool
weights_cb(void *data, const Eo_Event *event)
{
   Weight_Mode mode = elm_radio_state_value_get(event->obj);

   switch (mode)
     {
      case NONE:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(boxes[i], 0, 0);
        break;
      case NONE_BUT_FILL:
        evas_object_size_hint_align_set(data, -1, -1);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(boxes[i], 0, 0);
        break;
      case EQUAL:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(boxes[i], 1, 1);
        break;
      case ONE:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 6; i++)
          evas_object_size_hint_weight_set(boxes[i], 0, 0);
        evas_object_size_hint_weight_set(boxes[6], 1, 1);
        break;
      case TWO:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 5; i++)
          evas_object_size_hint_weight_set(boxes[i], 0, 0);
        evas_object_size_hint_weight_set(boxes[5], 1, 1);
        evas_object_size_hint_weight_set(boxes[6], 1, 1);
        break;
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
min_slider_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   for (int i = 0; i < 7; i++)
     evas_object_size_hint_min_set(boxes[i], val, 50);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
request_slider_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   evas_object_size_hint_request_set(boxes[3], val, val);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
padding_slider_cb(void *data, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   efl_pack_padding_set(data, val, val, EINA_TRUE);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
margin_slider_cb(void *data, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   evas_object_size_hint_padding_set(data, val, val, val, val);
   return EO_CALLBACK_CONTINUE;
}

void
test_ui_box(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o, *vbox, *f, *hbox, *chk, *bottombox;
   int i = 0;

   win = elm_win_util_standard_add("ui-box", "Efl.Ui.Box");
   elm_win_autodel_set(win, EINA_TRUE);
   efl_gfx_size_set(win, 600, 400);

   vbox = eo_add(EFL_UI_BOX_CLASS, win);
   efl_pack_padding_set(vbox, 10, 10, EINA_TRUE);
   efl_pack_direction_set(vbox, EFL_ORIENT_DOWN);
   evas_object_size_hint_weight_set(vbox, 1, 1);
   evas_object_size_hint_padding_set(vbox, 5, 5, 5, 5);
   elm_win_resize_object_add(win, vbox);
   efl_gfx_visible_set(vbox, 1);


   // create here to pass in cb
   bottombox = eo_add(EFL_UI_BOX_CLASS, win);


   /* controls */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Controls");
   evas_object_size_hint_align_set(f, -1, -1);
   evas_object_size_hint_weight_set(f, 1, 0);
   efl_pack(vbox, f);
   efl_gfx_visible_set(f, 1);

   hbox = eo_add(EFL_UI_BOX_CLASS, win);
   elm_object_content_set(f, hbox);
   efl_pack_padding_set(hbox, 10, 0, EINA_TRUE);
   efl_gfx_visible_set(hbox, 1);


   /* weights radio group */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   chk = o = elm_radio_add(win);
   elm_object_text_set(o, "No weight");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "No weight + box fill");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE_BUT_FILL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Equal weights");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, EQUAL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "One weight only");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, ONE);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Two weights");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, TWO);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   elm_radio_value_set(chk, NONE);


   /* min size setter */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0.5, -1);
   evas_object_size_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Min size");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, min_slider_cb, NULL);
   elm_slider_min_max_set(o, 1, 50);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 50);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* requested size setter */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0.5, -1);
   evas_object_size_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Requested size");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, request_slider_cb, NULL);
   elm_slider_min_max_set(o, 0, 250);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* inner box padding */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Padding");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, padding_slider_cb, bottombox);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* outer margin */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Margin");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, margin_slider_cb, bottombox);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);



   /* contents */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Contents");
   evas_object_size_hint_align_set(f, -1, -1);
   evas_object_size_hint_weight_set(f, 1, 1);
   efl_pack(vbox, f);
   efl_gfx_visible_set(f, 1);

   bx = bottombox;
   efl_pack_padding_set(bx, 10, 10, EINA_TRUE);
   evas_object_size_hint_align_set(bx, 0.5, 0.5);
   evas_object_size_hint_weight_set(bx, 1, 1);
   elm_object_content_set(f, bx);
   efl_gfx_visible_set(bx, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 255, 0, 0, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 0, 128, 0, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 0, 0, 255, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 1.0, 0.5);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 128, 128, 255, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 0.5, 1.0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 128, 128, 128, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 0.5, 0.0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 0, 128, 128, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 0, -1);
   evas_object_size_hint_max_set(o, 150, 100);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   boxes[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 128, 0, 128, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   efl_gfx_visible_set(win, 1);
}
