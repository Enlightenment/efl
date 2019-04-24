#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>

static Eo *layout, *btn1, *btn2, *btn3;

typedef enum {
   LEFT,
   RIGHT,
   TOP,
   BOTTOM
} Options;

static void
_btn_color_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *layout = data;

   static Eina_Bool changed = EINA_TRUE;

   if (changed)
     efl_gfx_color_set(layout, 0, 88, 204, 255);
   else
     efl_gfx_color_set(layout, 255, 255, 255, 255);

   changed = !changed;
}

static void
_btn_clicked_to_cb(void *data, const Efl_Event *event)
{
   Eo *to, *btn, *obj = event->object;
   Options opt = (Options)data;
   double relative;

   btn = efl_key_wref_get(obj, "btn");
   to = efl_key_wref_get(obj, "to");

   do
     {
        if (to == layout)
          to = btn1;
        else if (to == btn1)
          to = btn2;
        else if (to == btn2)
          to = btn3;
        else if (to == btn3)
          to = layout;
     }
   while (btn == to);
   efl_key_wref_set(obj, "to", to);

   switch (opt)
     {
      case LEFT:
        efl_ui_relative_layout_relation_left_get(layout, btn, NULL, &relative);
        efl_ui_relative_layout_relation_left_set(layout, btn, to, relative);
        break;
      case RIGHT:
        efl_ui_relative_layout_relation_right_get(layout, btn, NULL, &relative);
        efl_ui_relative_layout_relation_right_set(layout, btn, to, relative);
        break;
      case TOP:
        efl_ui_relative_layout_relation_top_get(layout, btn, NULL, &relative);
        efl_ui_relative_layout_relation_top_set(layout, btn, to, relative);
        break;
      case BOTTOM:
        efl_ui_relative_layout_relation_bottom_get(layout, btn, NULL, &relative);
        efl_ui_relative_layout_relation_bottom_set(layout, btn, to, relative);
        break;
     }
   efl_text_set(obj, ((to == layout) ? "parent" : (char *)efl_text_get(to)));
   efl_pack_layout_request(layout);
}

static void
_slider_changed_relative_cb(void *data, const Efl_Event *event)
{
   Options opt = (Options)data;
   Eo *btn, *slider = event->object;
   double val;

   btn = efl_key_wref_get(slider, "btn");
   val = efl_ui_range_value_get(slider);

   switch (opt)
     {
      case LEFT:
        efl_ui_relative_layout_relation_left_set(layout, btn, NULL, val);
        break;
      case RIGHT:
        efl_ui_relative_layout_relation_right_set(layout, btn, NULL, val);
        break;
      case TOP:
        efl_ui_relative_layout_relation_top_set(layout, btn, NULL, val);
        break;
      case BOTTOM:
        efl_ui_relative_layout_relation_bottom_set(layout, btn, NULL, val);
        break;
     }
   efl_pack_layout_request(layout);
}

static void
_slider_changed_align_cb(void *data, const Efl_Event *event)
{
   char opt = (char)(uintptr_t)data;
   Eo *btn, *slider = event->object;
   double val, x, y;

   btn = efl_key_wref_get(slider, "btn");
   val = efl_ui_range_value_get(slider);

   efl_gfx_hint_align_get(btn, &x, &y);
   if (opt == 'x')
     efl_gfx_hint_align_set(btn, val, y);
   else if (opt == 'y')
     efl_gfx_hint_align_set(btn, x, val);

   efl_pack_layout_request(layout);
}

static void
_setter_add(Eo *vbox, Eo *btn, Options option)
{
   Eo *to, *hbox;
   char *text = NULL, *btn_text;
   double relative;

   switch (option)
     {
      case LEFT:
        text = "left";
        efl_ui_relative_layout_relation_left_get(layout, btn, &to, &relative);
        break;
      case RIGHT:
        text = "right";
        efl_ui_relative_layout_relation_right_get(layout, btn, &to, &relative);
        break;
      case TOP:
        text = "top";
        efl_ui_relative_layout_relation_top_get(layout, btn, &to, &relative);
        break;
      case BOTTOM:
        text = "bottom";
        efl_ui_relative_layout_relation_bottom_get(layout, btn, &to, &relative);
        break;
     }
   btn_text = ((to == layout) ? "parent" : (char *)efl_text_get(to));

   hbox = efl_add(EFL_UI_BOX_CLASS, vbox,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_pack_padding_set(efl_added, 2, 2, EINA_TRUE),
                  efl_pack(vbox, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, hbox,
           efl_text_set(efl_added, text),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_text_valign_set(efl_added, 0.5),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(40, 0)),
           efl_gfx_hint_weight_set(efl_added, 0, EFL_GFX_HINT_EXPAND),
           efl_pack(hbox, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, hbox,
           efl_text_set(efl_added, btn_text),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(60, 0)),
           efl_key_wref_set(efl_added, "to", to),
           efl_key_wref_set(efl_added, "btn", btn),
           efl_gfx_hint_weight_set(efl_added, 0, EFL_GFX_HINT_EXPAND),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _btn_clicked_to_cb, (void *)option),
           efl_pack(hbox, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, hbox,
           efl_ui_range_min_max_set(efl_added, 0.0, 1.0),
           efl_ui_range_step_set(efl_added, 0.1),
           efl_ui_range_value_set(efl_added, relative),
           efl_key_wref_set(efl_added, "btn", btn),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, _slider_changed_relative_cb, (void *)option),
           efl_pack(hbox, efl_added));
}

static void
_button_frame_add(Eo *box, Eo *btn)
{
   Eo *f, *vbox, *hbox;
   double align_x, align_y;

   f = efl_add(EFL_UI_FRAME_CLASS, box,
               efl_text_set(efl_added, efl_text_get(btn)),
               efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
               efl_pack(box, efl_added));

   vbox = efl_add(EFL_UI_BOX_CLASS, f,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
                  efl_pack_padding_set(efl_added, 2, 2, EINA_TRUE),
                  efl_gfx_hint_margin_set(efl_added, 2, 2, 2, 2),
                  efl_content_set(f, efl_added));

   hbox = efl_add(EFL_UI_BOX_CLASS, vbox,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_pack_padding_set(efl_added, 2, 2, EINA_TRUE),
                  efl_pack(vbox, efl_added));

   efl_add(EFL_CANVAS_RECTANGLE_CLASS, hbox,
           efl_gfx_color_set(efl_added, 0, 0, 0, 0),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(40, 0)),
           efl_gfx_hint_weight_set(efl_added, 0, EFL_GFX_HINT_EXPAND),
           efl_pack(hbox, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, hbox,
           efl_text_set(efl_added, "to"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_text_halign_set(efl_added, 0.5),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(60, 0)),
           efl_gfx_hint_weight_set(efl_added, 0, EFL_GFX_HINT_EXPAND),
           efl_pack(hbox, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, hbox,
           efl_text_set(efl_added, "relative"),
           efl_text_halign_set(efl_added, 0.5),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(hbox, efl_added));

   _setter_add(vbox, btn, LEFT);
   _setter_add(vbox, btn, RIGHT);
   _setter_add(vbox, btn, TOP);
   _setter_add(vbox, btn, BOTTOM);

   /* align setter */
   efl_gfx_hint_align_get(btn, &align_x, &align_y);

   hbox = efl_add(EFL_UI_BOX_CLASS, vbox,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_pack_padding_set(efl_added, 2, 2, EINA_TRUE),
                  efl_pack(vbox, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, hbox,
           efl_text_set(efl_added, "align_x"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(40, 0)),
           efl_gfx_hint_weight_set(efl_added, 0, EFL_GFX_HINT_EXPAND),
           efl_pack(hbox, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, hbox,
           efl_ui_range_min_max_set(efl_added, 0.0, 1.0),
           efl_ui_range_step_set(efl_added, 0.1),
           efl_ui_range_value_set(efl_added, align_x),
           efl_key_wref_set(efl_added, "btn", btn),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, _slider_changed_align_cb, (void *)'x'),
           efl_pack(hbox, efl_added));

   hbox = efl_add(EFL_UI_BOX_CLASS, vbox,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_pack_padding_set(efl_added, 2, 2, EINA_TRUE),
                  efl_pack(vbox, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, hbox,
           efl_text_set(efl_added, "align_y"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(40, 0)),
           efl_gfx_hint_weight_set(efl_added, 0, EFL_GFX_HINT_EXPAND),
           efl_pack(hbox, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, hbox,
           efl_ui_range_min_max_set(efl_added, 0.0, 1.0),
           efl_ui_range_step_set(efl_added, 0.1),
           efl_ui_range_value_set(efl_added, align_y),
           efl_key_wref_set(efl_added, "btn", btn),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, _slider_changed_align_cb, (void *)'y'),
           efl_pack(hbox, efl_added));
}

void
test_ui_relative_layout(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *vbox, *f, *hbox;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Relative_Layout"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   vbox = efl_add(EFL_UI_BOX_CLASS, win,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
                  efl_pack_padding_set(efl_added, 10, 10, EINA_TRUE),
                  efl_gfx_hint_margin_set(efl_added, 5, 5, 5, 5),
                  efl_content_set(win, efl_added));

   /* controls */
   f = efl_add(EFL_UI_FRAME_CLASS, vbox,
               efl_text_set(efl_added, "Controls"),
               efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
               efl_pack(vbox, efl_added));

   hbox = efl_add(EFL_UI_BOX_CLASS, f,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                  efl_pack_padding_set(efl_added, 10, 0, EINA_TRUE),
                  efl_content_set(f, efl_added));

   /* contents */
   f = efl_add(EFL_UI_FRAME_CLASS, vbox,
               efl_text_set(efl_added, "Contents"),
               efl_pack(vbox, efl_added));

   layout = efl_add(EFL_UI_RELATIVE_LAYOUT_CLASS, f,
                    efl_content_set(f, efl_added));

   btn1 = efl_add(EFL_UI_BUTTON_CLASS, layout,
                  efl_text_set(efl_added, "button1"),
                  efl_gfx_hint_align_set(efl_added, 0.0, 0.0),
                  efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _btn_color_clicked_cb, layout),
                  efl_ui_relative_layout_relation_right_set(layout, efl_added, layout, 0.0),
                  efl_ui_relative_layout_relation_bottom_set(layout, efl_added, layout, 0.0));

   btn2 = efl_add(EFL_UI_BUTTON_CLASS, layout,
                  efl_text_set(efl_added, "button2"),
                  efl_gfx_hint_align_set(efl_added, 0.5, 0.0),
                  efl_ui_relative_layout_relation_left_set(layout, efl_added, btn1, 1.0),
                  efl_ui_relative_layout_relation_bottom_set(layout, efl_added, layout, 0.0));

   btn3 = efl_add(EFL_UI_BUTTON_CLASS, layout,
                  efl_text_set(efl_added, "button3"),
                  efl_ui_relative_layout_relation_left_set(layout, efl_added, btn2, 0.0),
                  efl_ui_relative_layout_relation_top_set(layout, efl_added, btn2, 1.0));

   _button_frame_add(hbox, btn1);
   _button_frame_add(hbox, btn2);
   _button_frame_add(hbox, btn3);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(600, 400));
   efl_gfx_entity_visible_set(win, EINA_TRUE);
}
