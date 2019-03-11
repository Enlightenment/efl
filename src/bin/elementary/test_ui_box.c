#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED
#include <Efl_Ui.h>
#include <Elementary.h>
#include <assert.h>

#define CNT 10
static Evas_Object *objects[CNT] = {};

typedef enum {
   NONE,
   NONE_BUT_FILL,
   EQUAL,
   ONE,
   TWO
} Weight_Mode;

static void
weights_cb(void *data, const Efl_Event *event)
{
   Weight_Mode mode = elm_radio_state_value_get(event->object);

   switch (mode)
     {
      case NONE:
        efl_gfx_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < CNT; i++)
          efl_gfx_hint_weight_set(objects[i], 0, 0);
        break;
      case NONE_BUT_FILL:
        efl_gfx_hint_align_set(data, -1, -1);
        for (int i = 0; i < CNT; i++)
          efl_gfx_hint_weight_set(objects[i], 0, 0);
        break;
      case EQUAL:
        efl_gfx_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < CNT; i++)
          efl_gfx_hint_weight_set(objects[i], 1, 1);
        break;
      case ONE:
        efl_gfx_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 6; i++)
          efl_gfx_hint_weight_set(objects[i], 0, 0);
        efl_gfx_hint_weight_set(objects[6], 1, 1);
        for (int i = 7; i < CNT; i++)
          efl_gfx_hint_weight_set(objects[i], 0, 0);
        break;
      case TWO:
        efl_gfx_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 5; i++)
          efl_gfx_hint_weight_set(objects[i], 0, 0);
        efl_gfx_hint_weight_set(objects[5], 1, 1);
        efl_gfx_hint_weight_set(objects[6], 1, 1);
        for (int i = 7; i < CNT; i++)
          efl_gfx_hint_weight_set(objects[i], 0, 0);
        break;
     }
}

static void
user_min_slider_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   int val = elm_slider_value_get(event->object);

   efl_gfx_hint_size_min_set(objects[3], EINA_SIZE2D(val, val));
}

static void
padding_slider_cb(void *data, const Efl_Event *event)
{
   int val = elm_slider_value_get(event->object);
   Eo *win = data, *box;

   box = efl_key_wref_get(win, "box");
   efl_pack_padding_set(box, val, val, EINA_TRUE);
}

static void
margin_slider_cb(void *data, const Efl_Event *event)
{
   int val = elm_slider_value_get(event->object);
   Eo *win = data, *box;

   box = efl_key_wref_get(win, "box");
   efl_gfx_hint_margin_set(box, val, val, val, val);
}

static void
alignh_slider_cb(void *data, const Efl_Event *event)
{
   double av, val;
   Eo *win = data, *box;

   box = efl_key_wref_get(win, "box");
   val = elm_slider_value_get(event->object);
   efl_pack_align_get(box, NULL, &av);
   efl_pack_align_set(box, val, av);
}

static void
alignv_slider_cb(void *data, const Efl_Event *event)
{
   double ah, val;
   Eo *win = data, *box;

   box = efl_key_wref_get(win, "box");
   val = elm_slider_value_get(event->object);
   efl_pack_align_get(box, &ah, NULL);
   efl_pack_align_set(box, ah, val);
}

static void
alignh_btn_slider_cb(void *data, const Efl_Event *event)
{
   double av, val;
   Eo *win = data, *btn;

   btn = efl_key_wref_get(win, "button");
   val = elm_slider_value_get(event->object);
   efl_gfx_hint_align_get(btn, NULL, &av);
   efl_gfx_hint_align_set(btn, val, av);
}

static void
alignv_btn_slider_cb(void *data, const Efl_Event *event)
{
   double ah, val;
   Eo *win = data, *btn;

   btn = efl_key_wref_get(win, "button");
   val = elm_slider_value_get(event->object);
   efl_gfx_hint_align_get(btn, &ah, NULL);
   efl_gfx_hint_align_set(btn, ah, val);
}

static void
flow_check_cb(void *data, const Efl_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   Eina_List *list = NULL;
   Eina_Iterator *it;
   Eo *box, *win, *sobj, *parent;

   // Unpack all children from the box, delete it and repack into the new box

   win = data;
   box = efl_key_wref_get(win, "box");
   parent = efl_parent_get(box);
   it = efl_content_iterate(box);
   EINA_ITERATOR_FOREACH(it, sobj)
     list = eina_list_append(list, sobj);
   eina_iterator_free(it);
   efl_pack_unpack_all(box);
   efl_del(box);

   box = efl_add(chk ? EFL_UI_BOX_FLOW_CLASS : EFL_UI_BOX_CLASS, win,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));
   efl_content_set(parent, box);
   efl_key_wref_set(win, "box", box);

   EINA_LIST_FREE(list, sobj)
     efl_pack(box, sobj);
}

static void
horiz_check_cb(void *data, const Efl_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   Eo *box = efl_key_wref_get(data, "box");
   efl_ui_direction_set(box, chk ? EFL_UI_DIR_HORIZONTAL : EFL_UI_DIR_VERTICAL);
}

static void
homo_check_cb(void *data, const Efl_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   Eo *box = efl_key_wref_get(data, "box");
   efl_ui_box_flow_homogenous_set(box, chk);
}

static void
max_size_check_cb(void *data, const Efl_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   Eo *box = efl_key_wref_get(data, "box");
   efl_ui_box_flow_max_size_set(box, chk);
}

static void
_custom_layout_update(Eo *pack, const void *data EINA_UNUSED)
{
   Eina_Iterator *it = efl_content_iterate(pack);
   int count = efl_content_count(pack), i = 0;
   Eina_Rect rp;
   Eo *sobj;

   // Note: This is a TERRIBLE layout. Just an example of the API, not showing
   // how to write a proper layout function.

   if (!count) return;

   rp = efl_gfx_entity_geometry_get(pack);
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        Eina_Rect r;

        r.x = (rp.w / count) * i;
        r.y = (rp.h / count) * i;
        r.size = efl_gfx_hint_size_combined_min_get(sobj);
        efl_gfx_entity_geometry_set(sobj, r);
        i++;
     }
   eina_iterator_free(it);
}

static void
custom_check_cb(void *data, const Efl_Event *event)
{
   EFL_OPS_DEFINE(custom_layout_ops,
                  EFL_OBJECT_OP_FUNC(efl_pack_layout_update, _custom_layout_update));

   Eina_Bool chk = elm_check_selected_get(event->object);
   Eo *box, *win = data;

   box = efl_key_wref_get(win, "box");

   // Overriding just the one function we need
   efl_object_override(box, chk ? &custom_layout_ops : NULL);

   // Layout request is required as the pack object doesn't know the layout
   // function was just overridden.
   efl_pack_layout_request(box);
}

void
test_ui_box(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o, *vbox, *f, *hbox, *chk;
   int i = 0;

   win = elm_win_util_standard_add("ui-box", "Efl.Ui.Box");
   elm_win_autodel_set(win, EINA_TRUE);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(600,  400));

   vbox = efl_add(EFL_UI_BOX_CLASS, win,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_pack_padding_set(vbox, 10, 10, EINA_TRUE);
   efl_gfx_hint_margin_set(vbox, 5, 5, 5, 5);
   elm_win_resize_object_add(win, vbox);


   /* controls */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Controls");
   efl_gfx_hint_align_set(f, -1, -1);
   efl_gfx_hint_weight_set(f, 1, 0);
   efl_pack(vbox, f);
   efl_gfx_entity_visible_set(f, 1);

   hbox = efl_add(EFL_UI_BOX_CLASS, win,
                  efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));
   efl_content_set(f, hbox);
   efl_pack_padding_set(hbox, 10, 0, EINA_TRUE);


   /* weights radio group */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_pack(hbox, bx);

   chk = o = elm_radio_add(win);
   elm_object_text_set(o, "No weight");
   efl_event_callback_add(o, EFL_UI_RADIO_EVENT_CHANGED, weights_cb, win);
   efl_gfx_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "No weight + box fill");
   efl_event_callback_add(o, EFL_UI_RADIO_EVENT_CHANGED, weights_cb, win);
   efl_gfx_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE_BUT_FILL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Equal weights");
   efl_event_callback_add(o, EFL_UI_RADIO_EVENT_CHANGED, weights_cb, win);
   efl_gfx_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, EQUAL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "One weight only");
   efl_event_callback_add(o, EFL_UI_RADIO_EVENT_CHANGED, weights_cb, win);
   efl_gfx_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, ONE);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Two weights");
   efl_event_callback_add(o, EFL_UI_RADIO_EVENT_CHANGED, weights_cb, win);
   efl_gfx_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, TWO);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   elm_radio_value_set(chk, NONE);


   /* misc */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_gfx_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);

   o = elm_label_add(win);
   elm_object_text_set(o, "Misc");
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Flow");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, flow_check_cb, win);
   efl_gfx_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 1);
   elm_object_text_set(o, "Horizontal");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, horiz_check_cb, win);
   efl_gfx_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Homogenous");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, homo_check_cb, win);
   efl_gfx_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Homogenous + Max");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, max_size_check_cb, win);
   efl_gfx_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Custom layout");
   efl_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, custom_check_cb, win);
   efl_gfx_hint_align_set(o, 0, 0);
   efl_gfx_hint_weight_set(o, 0, 1);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);


   /* user min size setter */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_gfx_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);

   o = elm_label_add(win);
   elm_object_text_set(o, "User min size");
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 1);
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, user_min_slider_cb, NULL);
   elm_slider_min_max_set(o, 0, 250);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);


   /* inner box padding */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_gfx_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);

   o = elm_label_add(win);
   elm_object_text_set(o, "Padding");
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 1);
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, padding_slider_cb, win);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);


   /* outer margin */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_gfx_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);

   o = elm_label_add(win);
   elm_object_text_set(o, "Margin");
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 1);
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, margin_slider_cb, win);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);


   /* Box align */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_gfx_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);

   o = elm_label_add(win);
   elm_object_text_set(o, "Box align");
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.1f");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 1);
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, alignv_slider_cb, win);
   elm_slider_min_max_set(o, -0.1, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.1f");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 1);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 0);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(100, 0));
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, alignh_slider_cb, win);
   elm_slider_min_max_set(o, -0.1, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   /* Button align */
   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));
   efl_gfx_hint_align_set(bx, 0, 0.5);
   efl_gfx_hint_fill_set(bx, EINA_FALSE, EINA_TRUE);
   efl_gfx_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);

   o = elm_label_add(win);
   elm_object_text_set(o, "Button align");
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.1f");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 1);
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, alignv_btn_slider_cb, win);
   elm_slider_min_max_set(o, 0.0, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.1f");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 1);
   efl_gfx_hint_align_set(o, 0.5, -1);
   efl_gfx_hint_weight_set(o, 1, 0);
   efl_gfx_hint_size_min_set(o, EINA_SIZE2D(100, 0));
   efl_event_callback_add(o, EFL_UI_SLIDER_EVENT_CHANGED, alignh_btn_slider_cb, win);
   elm_slider_min_max_set(o, -0.1, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);


   /* contents */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Contents");
   efl_gfx_hint_align_set(f, -1, -1);
   efl_gfx_hint_weight_set(f, 1, 1);
   efl_pack(vbox, f);
   efl_gfx_entity_visible_set(f, 1);

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL));
   efl_key_wref_set(win, "box", bx);
   efl_pack_padding_set(bx, 10, 10, EINA_TRUE);
   efl_gfx_hint_align_set(bx, 0.5, 0.5);
   efl_gfx_hint_weight_set(bx, 1, 1);
   efl_content_set(f, bx);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "Btn1");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_align_set(o, 0.5, 0.5);
   efl_gfx_hint_fill_set(o, EINA_FALSE, EINA_FALSE);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "Button 2");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = elm_label_add(win);
   elm_label_line_wrap_set(o, ELM_WRAP_WORD);
   elm_object_text_set(o, "This label is not marked as fill");
   efl_gfx_hint_align_set(o, 0.5, 0.5);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "Min size");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_fill_set(o, EINA_FALSE, EINA_FALSE);
   efl_gfx_hint_align_set(o, 0.5, 1.0);
   efl_gfx_hint_aspect_set(o, EFL_GFX_HINT_ASPECT_BOTH, EINA_SIZE2D(1, 1));
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "Quit!");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_fill_set(o, EINA_FALSE, EINA_FALSE);
   efl_gfx_hint_align_set(o, 0.5, 0.0);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = elm_label_add(win);
   elm_label_line_wrap_set(o, ELM_WRAP_WORD);
   elm_object_text_set(o, "This label on the other hand<br/>is marked as align=fill.");
   efl_gfx_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_key_wref_set(win, "button", o);
   efl_text_set(o, "Button with a quite long text.");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_size_max_set(o, EINA_SIZE2D(200, 100));
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "BtnA");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_fill_set(o, EINA_FALSE, EINA_FALSE);
   efl_gfx_hint_aspect_set(o, EFL_GFX_HINT_ASPECT_BOTH, EINA_SIZE2D(1, 2));
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "BtnB");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_fill_set(o, EINA_FALSE, EINA_FALSE);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   objects[i++] = o = efl_add(EFL_UI_BUTTON_CLASS, bx);
   efl_text_set(o, "BtnC");
   efl_gfx_hint_weight_set(o, 0, 0);
   efl_gfx_hint_fill_set(o, EINA_FALSE, EINA_FALSE);
   efl_pack(bx, o);
   efl_gfx_entity_visible_set(o, 1);

   assert(i == CNT);

   efl_gfx_entity_visible_set(win, 1);
}
