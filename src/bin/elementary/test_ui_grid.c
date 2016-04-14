#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static Evas_Object *objects[7] = {};

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
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        break;
      case NONE_BUT_FILL:
        evas_object_size_hint_align_set(data, -1, -1);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        break;
      case EQUAL:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(objects[i], 1, 1);
        break;
      case ONE:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 6; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        evas_object_size_hint_weight_set(objects[6], 1, 1);
        break;
      case TWO:
        evas_object_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 5; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        evas_object_size_hint_weight_set(objects[5], 1, 1);
        evas_object_size_hint_weight_set(objects[6], 1, 1);
        break;
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
request_slider_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   for (int i = 0; i < 6; i++)
     evas_object_size_hint_request_set(objects[i], val, 0);
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

static Eina_Bool
btnmargins_slider_cb(void *data, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   for (int i = 1; i < 7; i++)
     evas_object_size_hint_padding_set(data, val, val, val, val);
   return EO_CALLBACK_CONTINUE;
}

void
test_ui_grid(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o, *vbox, *f, *hbox, *chk, *grid;
   int i = 0;

   win = elm_win_util_standard_add("ui-grid", "Efl.Ui.Grid");
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
   grid = eo_add(EFL_UI_GRID_CLASS, win);


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
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, grid);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "No weight + grid fill");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, grid);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE_BUT_FILL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Equal weights");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, grid);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, EQUAL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "One weight only");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, grid);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, ONE);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Two weights");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, grid);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, TWO);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   elm_radio_value_set(chk, EQUAL);


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
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, padding_slider_cb, grid);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* outer margin */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 0, 1);
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
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, margin_slider_cb, grid);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* button margins */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Buttons margins");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, btnmargins_slider_cb, grid);
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

   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(f, grid);
   evas_object_show(grid);

   objects[i++] = o = evas_object_rectangle_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(o, 10, 10);
   evas_object_color_set(o, 64, 96, 128, 255);
   efl_pack_grid(grid, o, 0, 0, 3, 1);
   evas_object_show(o);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 1");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 0, 0, 1, 1);
   evas_object_show(o);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 2");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 1, 0, 1, 1);
   evas_object_show(o);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 3");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 2, 0, 1, 1);
   evas_object_show(o);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 4");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 0, 1, 2, 1);
   evas_object_show(o);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 5");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 2, 1, 1, 2);
   evas_object_show(o);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 6");
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 0, 2, 2, 1);
   evas_object_show(o);

   evas_object_show(win);
}
