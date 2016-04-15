#include "test.h"
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
min_slider_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   for (int i = 0; i < 7; i++)
     evas_object_size_hint_min_set(objects[i], val, 50);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
request_slider_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   int val = elm_slider_value_get(event->obj);
   evas_object_size_hint_request_set(objects[3], val, val);
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
alignh_slider_cb(void *data, const Eo_Event *event)
{
   double av, val;
   val = elm_slider_value_get(event->obj);
   efl_pack_align_get(data, NULL, &av);
   efl_pack_align_set(data, val, av);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
alignv_slider_cb(void *data, const Eo_Event *event)
{
   double ah, val;
   val = elm_slider_value_get(event->obj);
   efl_pack_align_get(data, &ah, NULL);
   efl_pack_align_set(data, ah, val);
   return EO_CALLBACK_CONTINUE;
}

static Efl_Ui_Box_Flow_Params s_flow_params = { 0.5, 0.5, 0, 0 };
static Eina_Bool flow = EINA_FALSE;

static Eina_Bool
flow_check_cb(void *data, const Eo_Event *event)
{
   flow = elm_check_selected_get(event->obj);
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
horiz_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->obj);
   efl_pack_direction_set(data, chk ? EFL_ORIENT_HORIZONTAL : EFL_ORIENT_VERTICAL);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
homo_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->obj);
   s_flow_params.homogenous = chk;
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
max_size_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->obj);
   s_flow_params.max_size = chk;
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
left_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->obj);
   if (chk)
     {
        s_flow_params.align_x = 0;
        s_flow_params.align_y = 0;
     }
   else
     {
        s_flow_params.align_x = 0.5;
        s_flow_params.align_y = 0.5;
     }
   efl_pack_layout_engine_set(data, flow ? EFL_UI_BOX_FLOW_CLASS : NULL, &s_flow_params);
   return EO_CALLBACK_CONTINUE;
}

static void
_custom_engine_layout_do(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED,
                         Efl_Pack *pack, const void *data EINA_UNUSED)
{
   Eina_Iterator *it = efl_pack_contents_iterate(pack);
   int count = efl_pack_contents_count(pack), i = 0;
   int px, py, pw, ph;
   Eo *sobj;

   // Note: This is a TERRIBLE layout. Just an example of the API, not showing
   // how to write a proper layout function.

   evas_object_geometry_get(pack, &px, &py, &pw, &ph);
   EINA_ITERATOR_FOREACH(it, sobj)
     {
        int x, y, h, w, mw, mh;
        evas_object_size_hint_min_get(sobj, &mw, &mh);
        x = (pw / count) * i;
        y = (ph / count) * i;
        w = mw;
        h = mh;
        evas_object_geometry_set(sobj, x + px, y + py, w, h);
        i++;
     }
   eina_iterator_free(it);
}

/* Common Eo Class boilerplate. */
static const Eo_Op_Description custom_engine_op_desc[] = {
     EO_OP_CLASS_FUNC_OVERRIDE(efl_pack_engine_layout_do, _custom_engine_layout_do),
};

static const Eo_Class_Description custom_engine_class_desc = {
     EO_VERSION, "Custom Layout Engine", EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(custom_engine_op_desc), NULL, 0, NULL, NULL
};

EO_DEFINE_CLASS(custom_engine_class_get, &custom_engine_class_desc, EFL_PACK_ENGINE_INTERFACE, NULL)

static Eina_Bool
custom_check_cb(void *data, const Eo_Event *event)
{
   Eina_Bool chk = elm_check_selected_get(event->obj);
   efl_pack_layout_engine_set(data, chk ? custom_engine_class_get() : NULL, NULL);
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


   /* misc */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "Misc");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Flow");
   eo_event_callback_add(o, ELM_CHECK_EVENT_CHANGED, flow_check_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 1);
   elm_object_text_set(o, "Horizontal");
   eo_event_callback_add(o, ELM_CHECK_EVENT_CHANGED, horiz_check_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Homogenous");
   eo_event_callback_add(o, ELM_CHECK_EVENT_CHANGED, homo_check_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Homogenous + Max");
   eo_event_callback_add(o, ELM_CHECK_EVENT_CHANGED, max_size_check_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Align left");
   eo_event_callback_add(o, ELM_CHECK_EVENT_CHANGED, left_check_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_check_add(win);
   elm_check_selected_set(o, 0);
   elm_object_text_set(o, "Custom layout");
   eo_event_callback_add(o, ELM_CHECK_EVENT_CHANGED, custom_check_cb, bottombox);
   evas_object_size_hint_align_set(o, 0, 0);
   evas_object_size_hint_weight_set(o, 0, 1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


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
   elm_slider_min_max_set(o, 1, 150);
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


   /* Box align */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 1, 1);
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
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 1);
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
   evas_object_size_hint_align_set(o, 0.5, -1);
   evas_object_size_hint_weight_set(o, 1, 0);
   eo_event_callback_add(o, ELM_SLIDER_EVENT_CHANGED, alignh_slider_cb, bottombox);
   elm_slider_min_max_set(o, -0.1, 1.0);
   elm_slider_step_set(o, 0.1);
   elm_slider_value_set(o, 0.5);
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

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 255, 0, 0, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 0, 128, 0, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 0, 0, 255, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 1.0, 0.5);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 128, 128, 255, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 0.5, 1.0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 128, 128, 128, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 0.5, 0.0);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 0, 128, 128, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, 0, -1);
   evas_object_size_hint_max_set(o, 150, 100);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   objects[i++] = o = evas_object_rectangle_add(win);
   efl_gfx_color_set(o, 128, 0, 128, 255);
   evas_object_size_hint_min_set(o, 50, 50);
   evas_object_size_hint_align_set(o, -1, -1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   efl_gfx_visible_set(win, 1);
}
