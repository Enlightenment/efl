#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED
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

static Eina_Bool
weights_cb(void *data, const Eo_Event *event)
{
   Weight_Mode mode = elm_radio_state_value_get(event->object);

   switch (mode)
     {
      case NONE:
        efl_gfx_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < CNT; i++)
          efl_gfx_size_hint_weight_set(objects[i], 0, 0);
        break;
      case NONE_BUT_FILL:
        efl_gfx_size_hint_align_set(data, -1, -1);
        for (int i = 0; i < CNT; i++)
          efl_gfx_size_hint_weight_set(objects[i], 0, 0);
        break;
      case EQUAL:
        efl_gfx_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < CNT; i++)
          efl_gfx_size_hint_weight_set(objects[i], 1, 1);
        break;
      case ONE:
        efl_gfx_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 6; i++)
          efl_gfx_size_hint_weight_set(objects[i], 0, 0);
        efl_gfx_size_hint_weight_set(objects[6], 1, 1);
        for (int i = 7; i < CNT; i++)
          efl_gfx_size_hint_weight_set(objects[i], 0, 0);
        break;
      case TWO:
        efl_gfx_size_hint_align_set(data, 0.5, 0.5);
        for (int i = 0; i < 5; i++)
          efl_gfx_size_hint_weight_set(objects[i], 0, 0);
        efl_gfx_size_hint_weight_set(objects[5], 1, 1);
        efl_gfx_size_hint_weight_set(objects[6], 1, 1);
        for (int i = 7; i < CNT; i++)
          efl_gfx_size_hint_weight_set(objects[i], 0, 0);
        break;
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
user_min_slider_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->object);
   efl_gfx_size_hint_min_set(objects[3], val, val);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
padding_slider_cb(void *data, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->object);
   efl_pack_padding_set(data, val, val, EINA_TRUE);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
margin_slider_cb(void *data, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->object);
   efl_gfx_size_hint_margin_set(data, val, val, val, val);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
alignh_slider_cb(void *data, const Eo_Event *event)
{
   double av, val;
   val = elm_slider_value_get(event->object);
   efl_pack_align_get(data, NULL, &av);
   efl_pack_align_set(data, val, av);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
alignv_slider_cb(void *data, const Eo_Event *event)
{
   double ah, val;
   val = elm_slider_value_get(event->object);
   efl_pack_align_get(data, &ah, NULL);
   efl_pack_align_set(data, ah, val);
   return EO_CALLBACK_CONTINUE;
}

static Efl_Ui_Box_Flow_Params s_flow_params = { 0, 0 };
static Eina_Bool flow = EINA_FALSE;

static Eina_Bool
flow_check_cb(void *data, const Eo_Event *event)
{
   flow = elm_check_selected_get(event->object);
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
horiz_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   efl_orientation_set(data, chk ? EFL_ORIENT_HORIZONTAL : EFL_ORIENT_VERTICAL);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
homo_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   s_flow_params.homogenous = chk;
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
max_size_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   s_flow_params.max_size = chk;
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
left_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   if (chk)
     {
        efl_pack_align_set(data, 0, 0.5);
     }
   else
     {
        efl_pack_align_set(data, 0.5, 0.5);
     }
   return EO_CALLBACK_CONTINUE;
}

static void
_custom_engine_layout_do(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED,
                         Efl_Pack *pack, const void *data EINA_UNUSED)
{
   Eina_Iterator *it = efl_content_iterate(pack);
   int count = efl_content_count(pack), i = 0;
   int px, py, pw, ph;
   Eo *sobj;

   // Note: This is a TERRIBLE layout. Just an example of the API, not showing
   // how to write a proper layout function.

   efl_gfx_geometry_get(pack, &px, &py, &pw, &ph);
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        int x, y, h, w, mw, mh;
        efl_gfx_size_hint_combined_min_get(sobj, &mw, &mh);
        x = (pw / count) * i;
        y = (ph / count) * i;
        w = mw;
        h = mh;
        efl_gfx_geometry_set(sobj, x + px, y + py, w, h);
        i++;
     }
   eina_iterator_free(it);
}

/* Common Eo Class boilerplate. */
static const Eo_Op_Description custom_engine_op_desc[] = {
   EO_OP_CLASS_FUNC_OVERRIDE(efl_pack_layout_do, _custom_engine_layout_do),
};

static const Eo_Class_Description custom_engine_class_desc = {
   EO_VERSION, "Custom Layout Engine", EO_CLASS_TYPE_INTERFACE,
   EO_CLASS_DESCRIPTION_OPS(custom_engine_op_desc), NULL, 0, NULL, NULL
};

EO_DEFINE_CLASS(_test_ui_box_custom_engine_class_get, &custom_engine_class_desc, EFL_PACK_LAYOUT_INTERFACE, NULL)

static Eina_Bool
custom_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->object);
   efl_pack_layout_engine_set(data, chk ? _test_ui_box_custom_engine_class_get() : NULL, NULL);
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
   efl_orientation_set(vbox, EFL_ORIENT_DOWN);
   efl_gfx_size_hint_weight_set(vbox, 1, 1);
   efl_gfx_size_hint_margin_set(vbox, 5, 5, 5, 5);
   elm_win_resize_object_add(win, vbox);
   efl_gfx_visible_set(vbox, 1);


   // create here to pass in cb
   bottombox = eo_add(EFL_UI_BOX_CLASS, win);


   /* controls */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Controls");
   efl_gfx_size_hint_align_set(f, -1, -1);
   efl_gfx_size_hint_weight_set(f, 1, 0);
   efl_pack(vbox, f);
   efl_gfx_visible_set(f, 1);

   hbox = eo_add(EFL_UI_BOX_CLASS, win);
   elm_object_content_set(f, hbox);
   efl_pack_padding_set(hbox, 10, 0, EINA_TRUE);
   efl_gfx_visible_set(hbox, 1);


   /* weights radio group */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_orientation_set(eo_self, EFL_ORIENT_DOWN));
   efl_gfx_size_hint_align_set(bx, 0, -1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   chk = o = elm_radio_add(win);
   elm_object_text_set(o, "No weight");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "No weight + box fill");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, NONE_BUT_FILL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Equal weights");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, EQUAL);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "One weight only");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, ONE);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_radio_add(win);
   elm_object_text_set(o, "Two weights");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, TWO);
   elm_radio_group_add(o, chk);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   elm_radio_value_set(chk, NONE);


   /* misc */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_orientation_set(eo_self, EFL_ORIENT_DOWN));
   efl_gfx_size_hint_align_set(bx, 0, -1);
   efl_gfx_size_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Misc");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Flow");
   eo_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, flow_check_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 1);
   elm_object_text_set(o, "Horizontal");
   eo_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, horiz_check_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Homogenous");
   eo_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, homo_check_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Homogenous + Max");
   eo_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, max_size_check_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Align left");
   eo_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, left_check_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Custom layout");
   eo_event_callback_add(o, EFL_UI_CHECK_EVENT_CHANGED, custom_check_cb, bottombox);
   efl_gfx_size_hint_align_set(o, 0, 0);
   efl_gfx_size_hint_weight_set(o, 0, 1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* user min size setter */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_orientation_set(eo_self, EFL_ORIENT_DOWN));
   efl_gfx_size_hint_align_set(bx, 0, -1);
   efl_gfx_size_hint_weight_set(bx, 0, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "User min size");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.0fpx");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_size_hint_align_set(o, 0.5, -1);
   efl_gfx_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, user_min_slider_cb, NULL);
   elm_slider_min_max_set(o, 0, 250);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* inner box padding */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_orientation_set(eo_self, EFL_ORIENT_DOWN));
   efl_gfx_size_hint_align_set(bx, 0, -1);
   efl_gfx_size_hint_weight_set(bx, 0, 1);
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
   efl_gfx_size_hint_align_set(o, 0.5, -1);
   efl_gfx_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, padding_slider_cb, bottombox);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* outer margin */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_orientation_set(eo_self, EFL_ORIENT_DOWN));
   efl_gfx_size_hint_align_set(bx, 0, -1);
   efl_gfx_size_hint_weight_set(bx, 0, 1);
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
   efl_gfx_size_hint_align_set(o, 0.5, -1);
   efl_gfx_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, margin_slider_cb, bottombox);
   elm_slider_min_max_set(o, 0, 40);
   elm_slider_inverted_set(o, 1);
   elm_slider_value_set(o, 10);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* Box align */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_orientation_set(eo_self, EFL_ORIENT_DOWN));
   efl_gfx_size_hint_align_set(bx, 0, -1);
   efl_gfx_size_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Box align");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.1f");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 0);
   efl_gfx_size_hint_align_set(o, 0.5, -1);
   efl_gfx_size_hint_weight_set(o, 1, 1);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, alignv_slider_cb, bottombox);
   elm_slider_min_max_set(o, -0.1, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_slider_add(win);
   elm_slider_indicator_format_set(o, "%.1f");
   elm_slider_indicator_show_set(o, 1);
   elm_slider_horizontal_set(o, 1);
   efl_gfx_size_hint_align_set(o, 0.5, -1);
   efl_gfx_size_hint_weight_set(o, 1, 0);
   efl_gfx_size_hint_min_set(o, 100, 0);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, alignh_slider_cb, bottombox);
   elm_slider_min_max_set(o, -0.1, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* contents */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Contents");
   efl_gfx_size_hint_align_set(f, -1, -1);
   efl_gfx_size_hint_weight_set(f, 1, 1);
   efl_pack(vbox, f);
   efl_gfx_visible_set(f, 1);

   bx = bottombox;
   efl_pack_padding_set(bx, 10, 10, EINA_TRUE);
   efl_gfx_size_hint_align_set(bx, 0.5, 0.5);
   efl_gfx_size_hint_weight_set(bx, 1, 1);
   elm_object_content_set(f, bx);
   efl_gfx_visible_set(bx, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Btn1");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button 2");
   efl_gfx_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_label_add(win);
   elm_label_line_wrap_set(o, ELM_WRAP_WORD);
   elm_object_text_set(o, "This label is not marked as fill");
   efl_gfx_size_hint_align_set(o, 0.5, 0.5);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Min size");
   efl_gfx_size_hint_align_set(o, 0.5, 1.0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Quit!");
   efl_gfx_size_hint_align_set(o, 0.5, 0.0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_label_add(win);
   elm_label_line_wrap_set(o, ELM_WRAP_WORD);
   elm_object_text_set(o, "This label on the other hand<br/>is marked as align=fill.");
   efl_gfx_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "Button with a quite long text.");
   efl_gfx_size_hint_align_set(o, -1, -1);
   efl_gfx_size_hint_max_set(o, 200, 100);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "BtnA");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "BtnB");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = elm_button_add(win);
   elm_object_text_set(o, "BtnC");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   assert(i == CNT);

   efl_gfx_visible_set(win, 1);
}
