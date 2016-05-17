#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED
#include <Elementary.h>

static Evas_Object *objects[7] = {};

typedef enum {
   NONE,
   NONE_BUT_FILL,
   EQUAL,
   ONE,
   TWO,
   CUSTOM
} Weight_Mode;

static void _custom_engine_layout_do(Eo *obj, void *pd, Efl_Pack *pack, const void *data);

/* Common Eo Class boilerplate. */
static const Eo_Op_Description custom_engine_op_desc[] = {
   EO_OP_CLASS_FUNC_OVERRIDE(efl_pack_layout_do, _custom_engine_layout_do),
};

static const Eo_Class_Description custom_engine_class_desc = {
   EO_VERSION, "Custom Layout Engine", EO_CLASS_TYPE_INTERFACE,
   EO_CLASS_DESCRIPTION_OPS(custom_engine_op_desc), NULL, 0, NULL, NULL
};

EO_DEFINE_CLASS(_test_ui_grid_custom_engine_class_get, &custom_engine_class_desc, EFL_PACK_LAYOUT_INTERFACE, NULL)

#define CUSTOM_ENGINE_CLASS _test_ui_grid_custom_engine_class_get()

static Eina_Bool
weights_cb(void *data, const Eo_Event *event)
{
   Weight_Mode mode = elm_radio_state_value_get(event->obj);
   Eo *grid = data;

   if (mode != CUSTOM)
     efl_pack_layout_engine_set(grid, NULL, NULL);

   switch (mode)
     {
      case NONE:
        evas_object_size_hint_align_set(grid, 0.5, 0.5);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        break;
      case NONE_BUT_FILL:
        evas_object_size_hint_align_set(grid, -1, -1);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        break;
      case EQUAL:
        evas_object_size_hint_align_set(grid, 0.5, 0.5);
        for (int i = 0; i < 7; i++)
          evas_object_size_hint_weight_set(objects[i], 1, 1);
        break;
      case ONE:
        evas_object_size_hint_align_set(grid, 0.5, 0.5);
        for (int i = 0; i < 6; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        evas_object_size_hint_weight_set(objects[6], 1, 1);
        break;
      case TWO:
        evas_object_size_hint_align_set(grid, 0.5, 0.5);
        for (int i = 0; i < 5; i++)
          evas_object_size_hint_weight_set(objects[i], 0, 0);
        evas_object_size_hint_weight_set(objects[5], 1, 1);
        evas_object_size_hint_weight_set(objects[6], 1, 1);
        break;
      case CUSTOM:
        efl_pack_layout_engine_set(grid, CUSTOM_ENGINE_CLASS, NULL);
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

static Eina_Bool
layout_updated_cb(void *data, const Eo_Event *event)
{
   Elm_Label *o = data;
   char buf[64];
   int rows, cols, count;

   efl_pack_grid_size_get(event->obj, &cols, &rows);
   count = efl_content_count(event->obj);
   sprintf(buf, "%d items (%dx%d)", count, cols, rows);
   elm_object_text_set(o, buf);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
child_evt_cb(void *data, const Eo_Event *event)
{
   Elm_Label *o = data;
   Efl_Gfx *it = event->info;
   int col, row, colspan, rowspan;
   char buf[64];

   efl_pack_grid_position_get(event->obj, it, &col, &row, &colspan, &rowspan);
   if (event->desc == EFL_CONTAINER_EVENT_CONTENT_ADDED)
     sprintf(buf, "pack %d,%d %dx%d", col, row, colspan, rowspan);
   else
     sprintf(buf, "unpack %d,%d %dx%d", col, row, colspan, rowspan);
   elm_object_text_set(o, buf);

   return EO_CALLBACK_CONTINUE;
}

static void
_custom_engine_layout_do(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED,
                         Efl_Pack *pack, const void *data EINA_UNUSED)
{
   /* Example custom layout for grid:
    * divide space into regions of same size, place objects in center of their
    * cells using their min size
    * Note: This is a TERRIBLE layout function (disregards align, weight, ...)
    */

   int rows, cols, gw, gh, gx, gy, c, r, cs, rs, gmw = 0, gmh = 0;
   Eina_Iterator *it;
   Eo *item;

   efl_gfx_size_get(pack, &gw, &gh);
   efl_gfx_position_get(pack, &gx, &gy);

   efl_pack_grid_size_get(pack, &cols, &rows);
   if (!cols || !rows) goto end;

   it = efl_content_iterate(pack);
   EINA_ITERATOR_FOREACH(it, item)
     {
        if (efl_pack_grid_position_get(pack, item, &c, &r, &cs, &rs))
          {
             int x, y, mw, mh;

             evas_object_size_hint_min_get(item, &mw, &mh);
             x = gx + c * gw / cols + (cs * gw / cols - mw) / 2;
             y = gy + r * gh / rows + (rs * gh / rows - mh) / 2;
             efl_gfx_size_set(item, mw, mh);
             efl_gfx_position_set(item, x, y);

             gmw = MAX(gmw, mw);
             gmh = MAX(gmh, mh);
          }
     }
   eina_iterator_free(it);

end:
   evas_object_size_hint_min_set(pack, gmw * cols, gmh * rows);
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

   o = elm_radio_add(win);
   elm_object_text_set(o, "Custom layout");
   eo_event_callback_add(o, ELM_RADIO_EVENT_CHANGED, weights_cb, grid);
   evas_object_size_hint_align_set(o, 0, 0.5);
   elm_radio_state_value_set(o, CUSTOM);
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


   /* ro info */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "<b>Properties</>");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_label_add(win);
   eo_event_callback_add(grid, EFL_PACK_EVENT_LAYOUT_UPDATED, layout_updated_cb, o);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_label_add(win);
   eo_event_callback_add(grid, EFL_CONTAINER_EVENT_CONTENT_ADDED, child_evt_cb, o);
   eo_event_callback_add(grid, EFL_CONTAINER_EVENT_CONTENT_REMOVED, child_evt_cb, o);
   evas_object_size_hint_align_set(o, 0.5, 0);
   evas_object_size_hint_weight_set(o, 1, 1);
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

static const char *
btn_text(const char *str)
{
   static char buf[64];
   static int id = 0;
   sprintf(buf, "%s %d", str ?: "item", ++id);
   return buf;
}

static void
remove_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   //efl_pack_unpack(data, obj);
   eo_unref(obj);
}

static void
append_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *grid = data;
   Eo *o = elm_button_add(grid);
   elm_object_text_set(o, btn_text("appended"));
   evas_object_smart_callback_add(o, "clicked", remove_cb, grid);
   elm_object_tooltip_text_set(o, "Click to unpack");
   efl_pack_end(grid, o);
   efl_gfx_visible_set(o, 1);
}

static void
clear_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *grid = data;
   efl_pack_clear(grid);
}

void
test_ui_grid_linear(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Evas_Object *win, *o, *vbox, *f, *hbox, *grid, *ico, *bx;

   win = elm_win_util_standard_add("ui-grid-linear", "Efl.Ui.Grid Linear APIs");
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
   efl_pack_padding_set(hbox, 5, 0, EINA_TRUE);
   efl_gfx_visible_set(hbox, 1);

   ico = elm_icon_add(win);
   elm_icon_standard_set(ico, "list-add");
   o = elm_button_add(win);
   elm_object_content_set(o, ico);
   elm_object_text_set(o, "Append");
   evas_object_smart_callback_add(o, "clicked", append_cb, grid);
   efl_pack(hbox, o);
   efl_gfx_visible_set(o, 1);

   ico = elm_icon_add(win);
   elm_icon_standard_set(ico, "edit-clear-all");
   o = elm_button_add(win);
   elm_object_content_set(o, ico);
   elm_object_text_set(o, "Clear");
   evas_object_smart_callback_add(o, "clicked", clear_cb, grid);
   efl_pack(hbox, o);
   efl_gfx_visible_set(o, 1);


   /* ro info */
   bx = eo_add(EFL_UI_BOX_CLASS, win,
               efl_pack_direction_set(eo_self, EFL_ORIENT_DOWN));
   evas_object_size_hint_align_set(bx, 0, -1);
   evas_object_size_hint_weight_set(bx, 1, 1);
   efl_pack(hbox, bx);
   efl_gfx_visible_set(bx, 1);

   o = elm_label_add(win);
   elm_object_text_set(o, "<b>Properties</>");
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_label_add(win);
   eo_event_callback_add(grid, EFL_PACK_EVENT_LAYOUT_UPDATED, layout_updated_cb, o);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);

   o = elm_label_add(win);
   eo_event_callback_add(grid, EFL_CONTAINER_EVENT_CONTENT_ADDED, child_evt_cb, o);
   eo_event_callback_add(grid, EFL_CONTAINER_EVENT_CONTENT_REMOVED, child_evt_cb, o);
   evas_object_size_hint_align_set(o, 0.5, 0);
   evas_object_size_hint_weight_set(o, 1, 1);
   efl_pack(bx, o);
   efl_gfx_visible_set(o, 1);


   /* contents */
   f = elm_frame_add(win);
   elm_object_text_set(f, "Contents");
   evas_object_size_hint_align_set(f, -1, -1);
   evas_object_size_hint_weight_set(f, 1, 1);
   efl_pack(vbox, f);
   efl_gfx_visible_set(f, 1);

   efl_pack_grid_columns_set(grid, 4);
   efl_pack_grid_directions_set(grid, EFL_ORIENT_RIGHT, EFL_ORIENT_DOWN);
   evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(f, grid);
   evas_object_show(grid);

   o = elm_button_add(win);
   elm_object_text_set(o, btn_text(NULL));
   evas_object_smart_callback_add(o, "clicked", remove_cb, grid);
   efl_pack(grid, o);
   efl_gfx_visible_set(o, 1);

   o = elm_button_add(win);
   elm_object_text_set(o, btn_text(NULL));
   evas_object_smart_callback_add(o, "clicked", remove_cb, grid);
   efl_pack(grid, o);
   efl_gfx_visible_set(o, 1);

   o = elm_button_add(win);
   elm_object_text_set(o, btn_text(NULL));
   evas_object_smart_callback_add(o, "clicked", remove_cb, grid);
   efl_pack(grid, o);
   efl_gfx_visible_set(o, 1);

   evas_object_show(win);
}
