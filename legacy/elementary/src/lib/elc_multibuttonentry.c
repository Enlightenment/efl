#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_multibuttonentry.h"

EAPI const char ELM_MULTIBUTTONENTRY_SMART_NAME[] = "elm_multibuttonentry";

#define MAX_STR     256
#define MIN_W_ENTRY 10

//widget signals
static const char SIG_ITEM_SELECTED[] = "item,selected";
static const char SIG_ITEM_ADDED[] = "item,added";
static const char SIG_ITEM_DELETED[] = "item,deleted";
static const char SIG_ITEM_CLICKED[] = "item,clicked";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_FOCUSED[] = "focused";
static const char SIG_UNFOCUSED[] = "unfocused";
static const char SIG_EXPANDED[] = "expanded";
static const char SIG_CONTRACTED[] = "contracted";
static const char SIG_EXPAND_STATE_CHANGED[] = "expand,state,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_ITEM_SELECTED, ""},
   {SIG_ITEM_ADDED, ""},
   {SIG_ITEM_DELETED, ""},
   {SIG_ITEM_CLICKED, ""},
   {SIG_CLICKED, ""},
   {SIG_FOCUSED, ""},
   {SIG_UNFOCUSED, ""},
   {SIG_EXPANDED, ""},
   {SIG_CONTRACTED, ""},
   {SIG_EXPAND_STATE_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_MULTIBUTTONENTRY_SMART_NAME, _elm_multibuttonentry,
  Elm_Multibuttonentry_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_multibuttonentry_smart_theme(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->theme(obj))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->items, l, item)
     {
        if (item->button)
          elm_widget_theme_object_set
            (obj, item->button, "multibuttonentry", "btn",
            elm_widget_style_get(obj));
        edje_object_scale_set
          (item->button, elm_widget_scale_get(obj) * elm_config_scale_get());
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_visual_guide_text_set(Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   elm_box_unpack(sd->box, sd->guide_text);
   elm_box_unpack(sd->box, sd->entry);
   if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK) return;

   if (!elm_widget_focus_get(obj))
     elm_object_focus_set(sd->entry, EINA_FALSE);

   if ((!eina_list_count(sd->items)) && sd->guide_text
       && (!elm_widget_focus_get(obj)) && (!sd->n_str))
     {
        evas_object_hide(sd->entry);
        elm_box_pack_end(sd->box, sd->guide_text);
        evas_object_show(sd->guide_text);
        sd->view_state = MULTIBUTTONENTRY_VIEW_GUIDETEXT;
     }
   else
     {
        evas_object_hide(sd->guide_text);

        if (sd->editable)
          {
             elm_box_pack_end(sd->box, sd->entry);
             evas_object_show(sd->entry);
             if (elm_widget_focus_get(obj))
               {
                  if (!sd->selected_it)
                    elm_object_focus_set(sd->entry, EINA_TRUE);
               }
          }
        sd->view_state = MULTIBUTTONENTRY_VIEW_ENTRY;
     }
}

static void
_shrink_mode_set(Evas_Object *obj,
                 Eina_Bool shrink)
{
   int count;
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;
   Evas_Coord button_min_width = 0;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (sd->view_state == MULTIBUTTONENTRY_VIEW_ENTRY)
     evas_object_hide(sd->entry);
   else if (sd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
     evas_object_hide(sd->guide_text);
   else if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
     {
        evas_object_hide(sd->rect_for_end);
        evas_object_hide(sd->end);
        sd->view_state = MULTIBUTTONENTRY_VIEW_NONE;
     }

   if (shrink == EINA_TRUE)
     {
        Evas_Coord w = 0, w_tmp = 0;
        Evas_Coord box_inner_item_width_padding = 0;

        elm_box_padding_get(sd->box, &box_inner_item_width_padding, NULL);
        // unpack all items and entry
        elm_box_unpack_all(sd->box);
        EINA_LIST_FOREACH (sd->items, l, item)
          {
             evas_object_hide(item->button);
             item->visible = EINA_FALSE;
          }
        // pack buttons only 1line
        w = sd->w_box;

        if (sd->label)
          {
             elm_box_pack_end(sd->box, sd->label);
             evas_object_size_hint_min_get(sd->label, &w_tmp, NULL);
             w -= w_tmp;
             w -= box_inner_item_width_padding;
          }

        item = NULL;
        count = eina_list_count(sd->items);
        button_min_width = 0;

        /* Evas_Coord button_min_height = 0; */
        if (sd->end_type == MULTIBUTTONENTRY_CLOSED_IMAGE)
          {
             const char *size_str;

             size_str = edje_object_data_get(sd->end, "closed_button_width");
             if (size_str) button_min_width = (Evas_Coord)atoi(size_str);
             /* it use for later
                size_str = edje_object_data_get(sd->end, "closed_button_height");
                if (size_str) button_min_width = (Evas_Coord)atoi(size_str);
              */
          }

        EINA_LIST_FOREACH (sd->items, l, item)
          {
             int w_label_count = 0;
             char buf[MAX_STR];

             elm_box_pack_end(sd->box, item->button);
             evas_object_show(item->button);
             item->visible = EINA_TRUE;

             w -= item->vw;
             w -= box_inner_item_width_padding;
             count--;

             if (sd->end_type == MULTIBUTTONENTRY_CLOSED_LABEL)
               {
                  if (count > 0)
                    {
                       snprintf(buf, sizeof(buf), "... + %d", count);
                       elm_object_text_set(sd->end, buf);
                       evas_object_size_hint_min_get
                         (sd->end, &w_label_count, NULL);
                    }

                  if (w < 0 || w < w_label_count)
                    {
                       elm_box_unpack(sd->box, item->button);
                       evas_object_hide(item->button);
                       item->visible = EINA_FALSE;

                       count++;
                       snprintf(buf, sizeof(buf), "... + %d", count);
                       elm_object_text_set(sd->end, buf);
                       evas_object_size_hint_min_get
                         (sd->end, &w_label_count, NULL);

                       elm_box_pack_end(sd->box, sd->end);
                       evas_object_show(sd->end);

                       sd->view_state = MULTIBUTTONENTRY_VIEW_SHRINK;
                       evas_object_smart_callback_call
                         (obj, SIG_EXPAND_STATE_CHANGED, (void *)1);
                       break;
                    }
               }
             else if (sd->end_type == MULTIBUTTONENTRY_CLOSED_IMAGE)
               {
                  if (w < button_min_width)
                    {
                       Evas_Coord rectSize;
                       Evas_Coord closed_height = 0;
                       const char *height_str =
                         edje_object_data_get
                           (ELM_WIDGET_DATA(sd)->resize_obj, "closed_height");

                       if (height_str)
                         closed_height =
                           (Evas_Coord)atoi(height_str);
                       elm_box_unpack(sd->box, item->button);
                       evas_object_hide(item->button);
                       item->visible = EINA_FALSE;

                       w += item->vw;
                       rectSize = w - button_min_width;
                       if (!sd->rect_for_end)
                         {
                            Evas *e = evas_object_evas_get(obj);

                            sd->rect_for_end = evas_object_rectangle_add(e);
                            evas_object_color_set(sd->rect_for_end, 0, 0, 0, 0);
                         }
                       evas_object_size_hint_min_set
                         (sd->rect_for_end, rectSize, closed_height *
                         elm_config_scale_get());
                       elm_box_pack_end(sd->box, sd->rect_for_end);
                       evas_object_show(sd->rect_for_end);

                       elm_box_pack_end(sd->box, sd->end);
                       evas_object_show(sd->end);

                       sd->view_state = MULTIBUTTONENTRY_VIEW_SHRINK;
                       evas_object_smart_callback_call
                         (obj, SIG_EXPAND_STATE_CHANGED, 0);
                       break;
                    }
               }
          }
     }
   else
     {
        // unpack all items and entry
        elm_box_unpack_all(sd->box);
        EINA_LIST_FOREACH (sd->items, l, item)
          {
             evas_object_hide(item->button);
             item->visible = EINA_FALSE;
          }
        evas_object_hide(sd->end);

        if (sd->rect_for_end) evas_object_hide(sd->rect_for_end);

        // pack buttons only 1line

        if (sd->label) elm_box_pack_end(sd->box, sd->label);

        // pack remain btns
        item = NULL;
        EINA_LIST_FOREACH (sd->items, l, item)
          {
             elm_box_pack_end(sd->box, item->button);
             evas_object_show(item->button);
             item->visible = EINA_TRUE;
          }

        sd->view_state = MULTIBUTTONENTRY_VIEW_NONE;
        evas_object_smart_callback_call
          (obj, SIG_EXPAND_STATE_CHANGED, (void *)(long)sd->shrink);
     }

   if (sd->view_state != MULTIBUTTONENTRY_VIEW_SHRINK)
     _visual_guide_text_set(obj);
}

static void
_view_update(Evas_Object *obj)
{
   Evas_Coord width = 1, height = 1;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (sd->w_box <= 0) return;

   // update label
   if (sd->label)
     {
        elm_box_unpack(sd->box, sd->label);
        elm_box_pack_start(sd->box, sd->label);
        evas_object_size_hint_min_get(sd->label, &width, &height);
     }

   if (sd->guide_text)
     {
        Evas_Coord guide_text_width = sd->w_box - width;

        evas_object_size_hint_min_set(sd->guide_text, guide_text_width, height);
     }

   // update buttons in shrink mode
   if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
     _shrink_mode_set(obj, EINA_TRUE);

   // update guidetext
   _visual_guide_text_set(obj);
}

static Eina_Bool
_elm_multibuttonentry_smart_on_focus(Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->on_focus(obj))
     return EINA_FALSE;

   if (elm_widget_focus_get(obj))
     {
        if (sd->editable)
          {
             if ((sd->selected_it))
               {
                  elm_entry_input_panel_show(sd->entry);
               }
             else if (((!sd->selected_it) || (!eina_list_count(sd->items))))
               {
                  elm_entry_cursor_end_set(sd->entry);
                  _view_update(obj);
                  elm_entry_input_panel_show(sd->entry);
               }
          }
        evas_object_smart_callback_call(obj, SIG_FOCUSED, NULL);
     }
   else
     {
        _view_update(obj);

        elm_entry_input_panel_hide(sd->entry);
        evas_object_smart_callback_call(obj, SIG_UNFOCUSED, NULL);
     }

   return EINA_TRUE;
}

static void
_button_item_del(Elm_Multibuttonentry_Item *item)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *it;
   Evas_Object *obj = WIDGET(item);

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (it == item)
          {
             sd->items = eina_list_remove(sd->items, it);
             elm_box_unpack(sd->box, it->button);

             evas_object_smart_callback_call(obj, SIG_ITEM_DELETED, it);

             evas_object_del(it->button);

             if (sd->selected_it == (Elm_Object_Item *)it)
               sd->selected_it = NULL;
             break;
          }
     }

   if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
     _shrink_mode_set(obj, EINA_TRUE);

   if (!eina_list_count(sd->items))
     _visual_guide_text_set(obj);
}

static void
_current_button_state_change(Evas_Object *obj,
                             Multibuttonentry_Button_State state)
{
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   item = (Elm_Multibuttonentry_Item *)sd->selected_it;

   if (!item || !item->button) return;

   switch (state)
     {
      case MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT:
        edje_object_signal_emit(item->button, "default", "");
        sd->selected_it = NULL;
        break;

      case MULTIBUTTONENTRY_BUTTON_STATE_SELECTED:
        edje_object_signal_emit(item->button, "focused", "");
        evas_object_smart_callback_call(obj, SIG_ITEM_SELECTED, item);
        break;

      default:
        edje_object_signal_emit(item->button, "default", "");
        sd->selected_it = NULL;
        break;
     }
}

static void
_current_button_change(Evas_Object *obj,
                       Evas_Object *btn)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   // change the state of previous button to "default"
   _current_button_state_change(obj, MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT);

   // change the current
   EINA_LIST_FOREACH (sd->items, l, item)
     {
        if (item->button == btn)
          {
             sd->selected_it = (Elm_Object_Item *)item;
             break;
          }
     }
   // change the state of current button to "focused"
   _current_button_state_change(obj, MULTIBUTTONENTRY_BUTTON_STATE_SELECTED);
}

static void
_button_select(Evas_Object *obj,
               Evas_Object *btn)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (btn)
     {
        _current_button_change(obj, btn);
        if (elm_widget_focus_get(obj))
          {
             elm_object_focus_set(sd->entry, EINA_FALSE);
             evas_object_focus_set(btn, EINA_TRUE);
          }
     }
   else
     {
        _current_button_state_change
          (obj, MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT);
        if (elm_widget_focus_get(obj) && sd->editable)
          elm_object_focus_set(sd->entry, EINA_TRUE);
     }
}

static void
_button_clicked_cb(void *data,
                   Evas_Object *obj,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Elm_Multibuttonentry_Item *item = NULL;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK) return;

   _current_button_change(data, obj);

   if (sd->selected_it)
     {
        if ((item = (Elm_Multibuttonentry_Item *)sd->selected_it) != NULL)
          {
             evas_object_smart_callback_call(data, SIG_ITEM_CLICKED, item);
             _button_select(data, item->button);
          }
     }
}

static void
_button_resize(Evas_Object *btn,
               Evas_Coord *realw,
               Evas_Coord *vieww)
{
   Evas_Coord rw, vw;
   const char *size_str;
   Evas_Coord w_btn = 0, button_max_width = 0;
   Evas_Coord w_text, h_btn, padding_outer = 0;

   size_str = edje_object_data_get(btn, "button_max_size");
   if (size_str) button_max_width = (Evas_Coord)atoi(size_str);

   // decide the size of button
   edje_object_part_geometry_get(btn, "elm.base", NULL, NULL, NULL, &h_btn);
   edje_object_part_geometry_get
     (btn, "elm.btn.text", NULL, NULL, &w_text, NULL);
   edje_object_part_geometry_get
     (btn, "right.padding", NULL, NULL, &padding_outer, NULL);
   w_btn = w_text + (2 * padding_outer);

   rw = w_btn;

   if (button_max_width < w_btn)
     vw = button_max_width;
   else
     vw = w_btn;

   //resize btn
   evas_object_resize(btn, vw, h_btn);
   evas_object_size_hint_min_set(btn, vw, h_btn);

   if (realw) *realw = rw;
   if (vieww) *vieww = vw;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Multibuttonentry_Item *item;

   if (part && strcmp(part, "default")) return;
   if (!label) return;

   item = (Elm_Multibuttonentry_Item *)it;
   edje_object_part_text_escaped_set(item->button, "elm.btn.text", label);
   _button_resize(item->button, &item->rw, &item->vw);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   Elm_Multibuttonentry_Item *item;

   if (part && strcmp(part, "default")) return NULL;
   item = (Elm_Multibuttonentry_Item *)it;
   return edje_object_part_text_get(item->button, "elm.btn.text");
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   _button_item_del((Elm_Multibuttonentry_Item *)it);

   return EINA_TRUE;
}

static Elm_Object_Item *
_button_item_add(Evas_Object *obj,
                 const char *str,
                 Multibuttonentry_Pos pos,
                 const void *ref,
                 Evas_Smart_Cb func,
                 void *data)
{
   Eina_List *l;
   Evas_Object *btn;
   Evas_Coord rw, vw;
   char *str_utf8 = NULL;
   Elm_Multibuttonentry_Item *item;
   Evas_Coord width = -1, height = -1;
   Elm_Multibuttonentry_Item_Filter *item_filter;
   Elm_Multibuttonentry_Item *reference = (Elm_Multibuttonentry_Item *)ref;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!str) return NULL;

   EINA_LIST_FOREACH (sd->filter_list, l, item_filter)
     {
        if (!(item_filter->callback_func(obj, str, data, item_filter->data)))
          return NULL;
     }
   // add button
   btn = edje_object_add(evas_object_evas_get(obj));
   str_utf8 = elm_entry_markup_to_utf8(str);

   //entry is cleared when text is made to button
   elm_object_text_set(sd->entry, "");

   elm_widget_theme_object_set
     (obj, btn, "multibuttonentry", "btn", elm_widget_style_get(obj));
   edje_object_part_text_escaped_set(btn, "elm.btn.text", str_utf8);
   edje_object_part_geometry_get
     (btn, "elm.btn.text", NULL, NULL, &width, &height);

   evas_object_size_hint_min_set(btn, width, height);

   edje_object_signal_callback_add
     (btn, "mouse,clicked,1", "*", _button_clicked_cb, obj);
   evas_object_size_hint_weight_set(btn, 0.0, 0.0);
   evas_object_show(btn);

   // append item list
   item = elm_widget_item_new(obj, Elm_Multibuttonentry_Item);
   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_data_set(item, data);
   _button_resize(btn, &rw, &vw);

   item->button = btn;
   item->rw = rw;
   item->vw = vw;
   item->visible = EINA_TRUE;

   if (func)
     {
        item->func = func;
     }

   switch (pos)
     {
      case MULTIBUTTONENTRY_POS_START:
        sd->items = eina_list_prepend(sd->items, item);
        if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
          {
             elm_widget_sub_object_add(obj, btn);
             _shrink_mode_set(obj, EINA_TRUE);
          }
        else
          {
             if (sd->label)
               elm_box_pack_after(sd->box, btn, sd->label);
             else
               elm_box_pack_start(sd->box, btn);
             if (sd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
               _visual_guide_text_set(obj);
          }
        break;

      case MULTIBUTTONENTRY_POS_END:
        sd->items = eina_list_append(sd->items, item);
        if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
          {
             elm_widget_sub_object_add(obj, btn);
             evas_object_hide(btn);
          }
        else
          {
             if (sd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
               _visual_guide_text_set(obj);
             if (sd->editable)
               {
                  elm_box_pack_before(sd->box, btn, sd->entry);
               }
             else
               elm_box_pack_end(sd->box, btn);
          }
        break;

      case MULTIBUTTONENTRY_POS_BEFORE:
        if (reference)
          sd->items = eina_list_prepend_relative(sd->items, item, reference);
        else
          sd->items = eina_list_append(sd->items, item);
        if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
          {
             elm_widget_sub_object_add(obj, btn);
             evas_object_hide(btn);
             _shrink_mode_set(obj, EINA_TRUE);
          }
        else
          {
             if (reference)
               elm_box_pack_before(sd->box, btn, reference->button);
             else
               {
                  if (sd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
                    _visual_guide_text_set(obj);
                  if (sd->editable)
                    {
                       elm_box_pack_before(sd->box, btn, sd->entry);
                    }
                  else
                    elm_box_pack_end(sd->box, btn);
               }
          }
        break;

      case MULTIBUTTONENTRY_POS_AFTER:
        if (reference)
          sd->items = eina_list_append_relative(sd->items, item, reference);
        else
          sd->items = eina_list_append(sd->items, item);
        if (sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
          {
             elm_widget_sub_object_add(obj, btn);
             _shrink_mode_set(obj, EINA_TRUE);
          }
        else
          {
             if (reference)
               elm_box_pack_after(sd->box, btn, reference->button);
             else
               {
                  if (sd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
                    _visual_guide_text_set(obj);
                  if (sd->editable)
                    {
                       elm_box_pack_before(sd->box, btn, sd->entry);
                    }
                  else
                    elm_box_pack_end(sd->box, btn);
               }
          }
        break;

      default:
        break;
     }
   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, item);

   free(str_utf8);

   return (Elm_Object_Item *)item;
}

//FIXME: having an empty event handling function and reacting on Evas
//events on specific objects is crazy, someone should fix that.
static Eina_Bool
_elm_multibuttonentry_smart_event(Evas_Object *obj,
                                  Evas_Object *src __UNUSED__,
                                  Evas_Callback_Type type __UNUSED__,
                                  void *event_info __UNUSED__)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   return EINA_TRUE;
}

static void
_elm_multibuttonentry_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord left, right, top, bottom;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   evas_object_size_hint_min_get(sd->box, &minw, &minh);
   edje_object_part_geometry_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "top.left.pad", NULL, NULL, &left, &top);
   edje_object_part_geometry_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "bottom.right.pad", NULL, NULL, &right,
     &bottom);

   minw += (left + right);
   minh += (top + bottom);

   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_mouse_clicked_signal_cb(void *data,
                         Evas_Object *obj __UNUSED__,
                         const char *emission __UNUSED__,
                         const char *source __UNUSED__)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   _view_update(data);

   if (sd->editable)
     {
        elm_entry_input_panel_show(sd->entry);
        elm_object_focus_set(sd->entry, EINA_TRUE);
     }

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_box_resize_cb(void *data,
               Evas *evas __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event __UNUSED__)
{
   Evas_Coord w, h;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   evas_object_geometry_get(sd->box, NULL, NULL, &w, &h);

   if (sd->h_box < h)
     evas_object_smart_callback_call(data, SIG_EXPANDED, NULL);
   else if (sd->h_box > h)
     evas_object_smart_callback_call(data, SIG_CONTRACTED, NULL);

   sd->w_box = w;
   sd->h_box = h;

   _view_update(data);
}

static void
_entry_resize_cb(void *data,
                 Evas *e __UNUSED__,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   Evas_Coord en_x, en_y, en_w, en_h;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   evas_object_geometry_get(sd->entry, &en_x, &en_y, &en_w, &en_h);

   if (elm_widget_focus_get(obj))
     elm_widget_show_region_set(sd->entry, en_x, en_y, en_w, en_h, EINA_TRUE);
}

static void
_entry_changed_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   const char *str;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   str = elm_object_text_get(sd->entry);
   sd->n_str = strlen(str);
}

static void
_entry_focus_in_cb(void *data,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Multibuttonentry_Item *item = NULL;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (sd->selected_it)
     {
        item = (Elm_Multibuttonentry_Item *)sd->selected_it;
        elm_object_focus_set(sd->entry, EINA_FALSE);
        evas_object_focus_set(item->button, EINA_TRUE);
     }
}

static void
_entry_focus_out_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   const char *str;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   str = elm_object_text_get(sd->entry);
   if (strlen(str))
     _button_item_add(data, str, MULTIBUTTONENTRY_POS_END, NULL, NULL, NULL);
}

static void
_entry_clicked_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   _current_button_state_change(data, MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT);
   elm_object_focus_set(sd->entry, EINA_TRUE);
}

static void
_layout_key_up_cb(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Elm_Multibuttonentry_Item *item = NULL;
   Evas_Event_Key_Up *ev = (Evas_Event_Key_Up *)event_info;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (!sd->box) return;

   if (sd->last_btn_select)
     {
        if (sd->selected_it &&
            ((strcmp(ev->keyname, "BackSpace") == 0) ||
             (strcmp(ev->keyname, "Delete") == 0)))
          {
             item = (Elm_Multibuttonentry_Item *)sd->selected_it;
             if (item && sd->editable)
               {
                  elm_widget_item_del(item);
                  elm_object_focus_set(sd->entry, EINA_TRUE);
               }
          }
        else if (((!sd->selected_it && (sd->n_str == 0) &&
                   (strcmp(ev->keyname, "BackSpace") == 0)) ||
                  (strcmp(ev->keyname, "Delete") == 0)))
          {
             item = eina_list_data_get(eina_list_last(sd->items));
             if (item)
               _button_select(data, item->button);
          }
     }
   else
     sd->last_btn_select = EINA_TRUE;
}

static void
_entry_key_down_cb(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info)
{
   Evas_Event_Key_Down *ev = (Evas_Event_Key_Down *)event_info;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (sd->n_str == 1 &&
       (!strcmp(ev->keyname, "BackSpace") || !strcmp(ev->keyname, "Delete")))
     sd->last_btn_select = EINA_FALSE;
}

static void
_entry_key_up_cb(void *data,
                 Evas *e __UNUSED__,
                 Evas_Object *obj __UNUSED__,
                 void *event_info)
{
   Evas_Event_Key_Up *ev = (Evas_Event_Key_Up *)event_info;
   const char *str;

   ELM_MULTIBUTTONENTRY_DATA_GET(data, sd);

   if (!sd->box) return;

   str = elm_object_text_get(sd->entry);

   if (strlen(str) &&
       (!strcmp(ev->keyname, "KP_Enter") || !strcmp(ev->keyname, "Return")))
     {
        _button_item_add
          (data, str, MULTIBUTTONENTRY_POS_END, NULL, NULL, NULL);
        sd->n_str = 0;
     }
}

static void
_callbacks_register(Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   elm_layout_signal_callback_add
     (obj, "mouse,clicked,1", "*", _mouse_clicked_signal_cb, obj);

   evas_object_event_callback_add
     (ELM_WIDGET_DATA(sd)->resize_obj, EVAS_CALLBACK_KEY_UP,
     _layout_key_up_cb, obj);

   evas_object_event_callback_add
     (sd->box, EVAS_CALLBACK_RESIZE, _box_resize_cb, obj);

   evas_object_event_callback_add
     (sd->entry, EVAS_CALLBACK_KEY_UP, _entry_key_up_cb, obj);
   evas_object_event_callback_add
     (sd->entry, EVAS_CALLBACK_KEY_DOWN, _entry_key_down_cb, obj);
   evas_object_event_callback_add
     (sd->entry, EVAS_CALLBACK_RESIZE, _entry_resize_cb, obj);
   evas_object_smart_callback_add
     (sd->entry, "changed", _entry_changed_cb, obj);
   evas_object_smart_callback_add
     (sd->entry, "focused", _entry_focus_in_cb, obj);
   evas_object_smart_callback_add
     (sd->entry, "unfocused", _entry_focus_out_cb, obj);
   evas_object_smart_callback_add
     (sd->entry, "clicked", _entry_clicked_cb, obj);
}

static void
_label_set(Evas_Object *obj,
           const char *str)
{
   Evas_Coord width, height, sum_width = 0;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!str) return;

   eina_stringshare_replace(&sd->label_str, str);

   evas_object_size_hint_min_set(sd->label, 0, 0);
   evas_object_resize(sd->label, 0, 0);
   edje_object_part_text_escaped_set(sd->label, "mbe.label", str);

   if (!strlen(str))
     {
        /* FIXME: not work yet */
        edje_object_signal_emit(sd->label, "elm,mbe,clear_text", "");
        edje_object_part_geometry_get
          (sd->label, "mbe.label", NULL, NULL, &width, &height);
        sum_width += width;
     }
   else
     {
        edje_object_signal_emit(sd->label, "elm,mbe,set_text", "");
        edje_object_part_geometry_get
          (sd->label, "mbe.label", NULL, NULL, &width, &height);

        sum_width += width;

        edje_object_part_geometry_get
          (sd->label, "mbe.label.left.padding", NULL, NULL, &width, NULL);
        sum_width += width;

        edje_object_part_geometry_get
          (sd->label, "mbe.label.right.padding", NULL, NULL, &width, NULL);
        sum_width += width;
     }

   evas_object_size_hint_min_set(sd->label, sum_width, height);
   evas_object_show(sd->label);
   _view_update(obj);
}

static void
_guide_text_set(Evas_Object *obj,
                const char *str)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!str) return;

   eina_stringshare_replace(&sd->guide_text_str, str);
   if (sd->guide_text == NULL)
     sd->guide_text = edje_object_add(evas_object_evas_get(obj));

   if (sd->guide_text)
     {
        elm_widget_theme_object_set(obj, sd->guide_text, "multibuttonentry",
                                    "guidetext", elm_widget_style_get(obj));
        evas_object_size_hint_weight_set
          (sd->guide_text, 0.0, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set
          (sd->guide_text, EVAS_HINT_FILL, EVAS_HINT_FILL);
        edje_object_part_text_escaped_set(sd->guide_text, "elm.text", str);
        _view_update(obj);
     }
}

static Elm_Multibuttonentry_Item_Filter *
_filter_new(Elm_Multibuttonentry_Item_Filter_Cb func,
            void *data)
{
   Elm_Multibuttonentry_Item_Filter *item_filter =
     ELM_NEW(Elm_Multibuttonentry_Item_Filter);
   if (!item_filter) return NULL;

   item_filter->callback_func = func;
   item_filter->data = data;

   return item_filter;
}

static void
_filter_free(Elm_Multibuttonentry_Item_Filter *item_filter)
{
   free(item_filter);
}

static void
_box_min_size_calculate(Evas_Object *box,
                        Evas_Object_Box_Data *priv)
{
   Evas_Coord minw, minh, mnw, mnh, ww;
   Evas_Coord w, cw = 0, cmaxh = 0;
   Evas_Object_Box_Option *opt;
   const Eina_List *l;
   double wx;

   /* FIXME: need to calc max */
   minw = 0;
   minh = 0;

   evas_object_geometry_get(box, NULL, NULL, &w, NULL);
   evas_object_size_hint_min_get(box, &minw, NULL);

   EINA_LIST_FOREACH (priv->children, l, opt)
     {
        evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);
        evas_object_size_hint_weight_get(opt->obj, &wx, NULL);

        if (wx)
          {
             if (mnw != -1 && (w - cw) >= mnw)
               ww = w - cw;
             else
               ww = w;
          }
        else
          ww = mnw;

        if ((cw + mnw) > w)
          {
             minh += cmaxh;
             cw = 0;
          }
        cw += ww;
        if (cmaxh < mnh) cmaxh = mnh;
     }

   minh += cmaxh;

   evas_object_size_hint_min_set(box, minw, minh);
}

static Evas_Coord
_item_max_height_calculate(Evas_Object *box,
                           Evas_Object_Box_Data *priv,
                           int obj_index)
{
   Evas_Coord mnw, mnh, cw = 0, cmaxh = 0, w, ww;
   Evas_Object_Box_Option *opt;
   int local_index = 0;
   const Eina_List *l;
   double wx;

   evas_object_geometry_get(box, NULL, NULL, &w, NULL);

   EINA_LIST_FOREACH (priv->children, l, opt)
     {
        evas_object_size_hint_min_get(opt->obj, &mnw, &mnh);
        evas_object_size_hint_weight_get(opt->obj, &wx, NULL);

        if (wx)
          {
             if (mnw != -1 && (w - cw) >= mnw)
               ww = w - cw;
             else
               ww = w;
          }
        else
          ww = mnw;

        if ((cw + ww) > w)
          {
             if (local_index > obj_index) return cmaxh;
             cw = 0;
          }

        cw += ww;
        if (cmaxh < mnh) cmaxh = mnh;

        local_index++;
     }

   return cmaxh;
}

static void
_box_layout_cb(Evas_Object *o,
               Evas_Object_Box_Data *priv,
               void *data __UNUSED__)
{
   Evas_Coord cw = 0, ch = 0, cmaxh = 0, obj_index = 0;
   Evas_Coord x, y, w, h, xx, yy;
   Evas_Object_Box_Option *opt;
   Evas_Coord minw, minh;
   const Eina_List *l;
   Evas_Object *obj;
   double ax, ay;

   _box_min_size_calculate(o, priv);

   evas_object_geometry_get(o, &x, &y, &w, &h);

   evas_object_size_hint_min_get(o, &minw, &minh);
   evas_object_size_hint_align_get(o, &ax, &ay);
   if (w < minw)
     {
        x = x + ((w - minw) * (1.0 - ax));
        w = minw;
     }
   if (h < minh)
     {
        y = y + ((h - minh) * (1.0 - ay));
        h = minh;
     }

   xx = x;
   yy = y;

   EINA_LIST_FOREACH (priv->children, l, opt)
     {
        Evas_Coord mnw, mnh, mxw, mxh;
        Evas_Coord ww, hh, ow, oh;
        double wx, wy;
        int fw, fh;

        obj = opt->obj;
        evas_object_size_hint_align_get(obj, &ax, &ay);
        evas_object_size_hint_weight_get(obj, &wx, &wy);
        evas_object_size_hint_min_get(obj, &mnw, &mnh);
        evas_object_size_hint_max_get(obj, &mxw, &mxh);
        fw = fh = 0;
        if (ax == -1.0) {fw = 1; ax = 0.5; }
        if (ay == -1.0) {fh = 1; ay = 0.5; }

        if (wx)
          {
             if (mnw != -1 && (w - cw) >= mnw)
               ww = w - cw;
             else
               ww = w;
          }
        else
          ww = mnw;
        hh = _item_max_height_calculate(o, priv, obj_index);

        ow = mnw;
        if (fw) ow = ww;
        if ((mxw >= 0) && (mxw < ow)) ow = mxw;
        oh = mnh;
        if (fh) oh = hh;
        if ((mxh >= 0) && (mxh < oh)) oh = mxh;

        if ((cw + ww) > w)
          {
             ch += cmaxh;
             cw = 0;
          }

        evas_object_move
          (obj, xx + cw + (Evas_Coord)(((double)(ww - ow)) * ax),
          yy + ch + (Evas_Coord)(((double)(hh - oh)) * ay));
        evas_object_resize(obj, ow, oh);

        cw += ww;
        if (cmaxh < hh) cmaxh = hh;

        obj_index++;
     }
}

static void
_view_init(Evas_Object *obj)
{
   const char *end_type;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   sd->box = elm_box_add(obj);

   if (!sd->box) return;
   elm_box_layout_set(sd->box, _box_layout_cb, NULL, NULL);
   elm_box_homogeneous_set(sd->box, EINA_FALSE);
   elm_layout_content_set(obj, "box.swallow", sd->box);

   sd->label = edje_object_add(evas_object_evas_get(obj));
   if (!sd->label) return;
   elm_widget_theme_object_set
     (obj, sd->label, "multibuttonentry", "label",
     elm_widget_style_get(obj));
   _label_set(obj, "");
   elm_widget_sub_object_add(obj, sd->label);

   sd->entry = elm_entry_add(obj);
   if (!sd->entry) return;
   elm_entry_single_line_set(sd->entry, EINA_TRUE);
   elm_object_text_set(sd->entry, "");
   elm_entry_input_panel_enabled_set(sd->entry, EINA_FALSE);
   evas_object_size_hint_min_set(sd->entry, MIN_W_ENTRY, 0);
   evas_object_size_hint_weight_set
     (sd->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(sd->box, sd->entry);
   evas_object_show(sd->entry);

   sd->view_state = MULTIBUTTONENTRY_VIEW_ENTRY;

   end_type = edje_object_data_get
       (ELM_WIDGET_DATA(sd)->resize_obj, "closed_button_type");
   if (!end_type || !strcmp(end_type, "label"))
     {
        sd->end = elm_label_add(obj);
        if (!sd->end) return;
        elm_object_style_set(sd->end, "extended/multibuttonentry_default");
        sd->end_type = MULTIBUTTONENTRY_CLOSED_LABEL;
     }
   else
     {
        const char *size_str;
        Evas_Coord button_min_width = 0;
        Evas_Coord button_min_height = 0;

        sd->end = edje_object_add(evas_object_evas_get(obj));
        if (!sd->end) return;
        elm_widget_theme_object_set
          (obj, sd->end, "multibuttonentry", "closedbutton",
          elm_widget_style_get(obj));

        size_str = edje_object_data_get(sd->end, "closed_button_width");
        if (size_str) button_min_width = (Evas_Coord)atoi(size_str);
        size_str = edje_object_data_get(sd->end, "closed_button_height");
        if (size_str) button_min_height = (Evas_Coord)atoi(size_str);

        sd->end_type = MULTIBUTTONENTRY_CLOSED_IMAGE;
        evas_object_size_hint_min_set
          (sd->end, button_min_width * elm_config_scale_get(),
          button_min_height * elm_config_scale_get());
        elm_widget_sub_object_add(obj, sd->end);
     }
}

static Eina_Bool
_elm_multibuttonentry_smart_text_set(Evas_Object *obj,
                                     const char *part,
                                     const char *label)
{
   if (!part || !strcmp(part, "default"))
     {
        if (label) _label_set(obj, label);
        return EINA_TRUE;
     }
   else if (!strcmp(part, "guide"))
     {
        if (label) _guide_text_set(obj, label);
        return EINA_TRUE;
     }
   else return _elm_multibuttonentry_parent_sc->text_set(obj, part, label);
}

static const char *
_elm_multibuttonentry_smart_text_get(const Evas_Object *obj,
                                     const char *part)
{
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (!part || !strcmp(part, "default"))
     {
        return sd->label_str;
     }
   else if (!strcmp(part, "guide"))
     {
        return sd->guide_text_str;
     }
   else return _elm_multibuttonentry_parent_sc->text_get(obj, part);
}

static void
_elm_multibuttonentry_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Multibuttonentry_Smart_Data);

   ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->base.add(obj);

   elm_layout_theme_set
     (obj, "multibuttonentry", "base", elm_widget_style_get(obj));

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->view_state = MULTIBUTTONENTRY_VIEW_NONE;
   priv->last_btn_select = EINA_TRUE;
   priv->n_str = 0;
   priv->rect_for_end = NULL;
   priv->add_callback = NULL;
   priv->add_callback_data = NULL;
   priv->editable = EINA_TRUE;

   _view_init(obj);
   _callbacks_register(obj);
}

static void
_elm_multibuttonentry_smart_del(Evas_Object *obj)
{
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->items, item)
     {
        evas_object_del(item->button);
        free(item);
     }
   sd->items = NULL;

   sd->selected_it = NULL;

   if (sd->label_str) eina_stringshare_del(sd->label_str);
   if (sd->guide_text_str) eina_stringshare_del(sd->guide_text_str);
   evas_object_del(sd->entry);
   if (sd->label) evas_object_del(sd->label);
   if (sd->guide_text) evas_object_del(sd->guide_text);
   if (sd->end) evas_object_del(sd->end);
   if (sd->rect_for_end) evas_object_del(sd->rect_for_end);

   ELM_WIDGET_CLASS(_elm_multibuttonentry_parent_sc)->base.del(obj);
}

static void
_elm_multibuttonentry_smart_set_user(Elm_Multibuttonentry_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_multibuttonentry_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_multibuttonentry_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_multibuttonentry_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_multibuttonentry_smart_event;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_multibuttonentry_smart_on_focus;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->text_set = _elm_multibuttonentry_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_multibuttonentry_smart_text_get;
   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_multibuttonentry_smart_sizing_eval;
}

EAPI const Elm_Multibuttonentry_Smart_Class *
elm_multibuttonentry_smart_class_get(void)
{
   static Elm_Multibuttonentry_Smart_Class _sc =
     ELM_MULTIBUTTONENTRY_SMART_CLASS_INIT_NAME_VERSION
       (ELM_MULTIBUTTONENTRY_SMART_NAME);
   static const Elm_Multibuttonentry_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_multibuttonentry_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_multibuttonentry_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_multibuttonentry_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Evas_Object *
elm_multibuttonentry_entry_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->entry;
}

EAPI Eina_Bool
elm_multibuttonentry_expanded_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) - 1;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK ?
          EINA_FALSE : EINA_TRUE;
}

EAPI void
elm_multibuttonentry_expanded_set(Evas_Object *obj,
                                  Eina_Bool expanded)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (((sd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK) ?
        EINA_FALSE : EINA_TRUE) == expanded) return;

   if (expanded)
     _shrink_mode_set(obj, EINA_FALSE);
   else
     _shrink_mode_set(obj, EINA_TRUE);
}

EAPI void
elm_multibuttonentry_editable_set(Evas_Object *obj,
                                  Eina_Bool editable)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   editable = !!editable;
   if (sd->editable == editable) return;
   sd->editable = editable;

   if (sd->editable)
     evas_object_show(sd->entry);
   else
     {
        elm_entry_input_panel_hide(sd->entry);
        evas_object_hide(sd->entry);
     }
}

EAPI Eina_Bool
elm_multibuttonentry_editable_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) - 1;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->editable;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_prepend(Evas_Object *obj,
                                  const char *label,
                                  Evas_Smart_Cb func,
                                  void *data)
{
   return _button_item_add
            (obj, label, MULTIBUTTONENTRY_POS_START, NULL, func, data);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_append(Evas_Object *obj,
                                 const char *label,
                                 Evas_Smart_Cb func,
                                 void *data)
{
   return _button_item_add
            (obj, label, MULTIBUTTONENTRY_POS_END, NULL, func, data);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_insert_before(Evas_Object *obj,
                                        Elm_Object_Item *before,
                                        const char *label,
                                        Evas_Smart_Cb func,
                                        void *data)
{
   return _button_item_add
            (obj, label, MULTIBUTTONENTRY_POS_BEFORE, before, func, data);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_insert_after(Evas_Object *obj,
                                       Elm_Object_Item *after,
                                       const char *label,
                                       Evas_Smart_Cb func,
                                       void *data)
{
   return _button_item_add
            (obj, label, MULTIBUTTONENTRY_POS_AFTER, after, func, data);
}

EAPI const Eina_List *
elm_multibuttonentry_items_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_first_item_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return eina_list_data_get(sd->items);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_last_item_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return eina_list_data_get(eina_list_last(sd->items));
}

EAPI Elm_Object_Item *
elm_multibuttonentry_selected_item_get(const Evas_Object *obj)
{
   ELM_MULTIBUTTONENTRY_CHECK(obj) NULL;
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   return sd->selected_it;
}

EAPI void
elm_multibuttonentry_item_selected_set(Elm_Object_Item *it,
                                       Eina_Bool selected)
{
   Elm_Multibuttonentry_Item *item = (Elm_Multibuttonentry_Item *)it;

   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it);

   if (selected) _button_select(WIDGET(item), item->button);
   else _button_select(WIDGET(item), NULL);
}

EAPI Eina_Bool
elm_multibuttonentry_item_selected_get(const Elm_Object_Item *it)
{
   //TODO : To be implemented.
   if (!it) return EINA_FALSE;

   return EINA_TRUE;
}

EAPI void
elm_multibuttonentry_clear(Evas_Object *obj)
{
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   if (sd->items)
     {
        EINA_LIST_FREE (sd->items, item)
          {
             elm_box_unpack(sd->box, item->button);
             evas_object_del(item->button);
             free(item);
          }
        sd->items = NULL;
     }
   sd->selected_it = NULL;
   _view_update(obj);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_prev_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   EINA_LIST_FOREACH (sd->items, l, item)
     {
        if (item == (Elm_Multibuttonentry_Item *)it)
          {
             l = eina_list_prev(l);
             if (!l) return NULL;
             return eina_list_data_get(l);
          }
     }
   return NULL;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_next_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   ELM_MULTIBUTTONENTRY_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_MULTIBUTTONENTRY_DATA_GET(WIDGET(it), sd);

   EINA_LIST_FOREACH (sd->items, l, item)
     {
        if (item == (Elm_Multibuttonentry_Item *)it)
          {
             l = eina_list_next(l);
             if (!l) return NULL;
             return eina_list_data_get(l);
          }
     }
   return NULL;
}

EINA_DEPRECATED EAPI void *
elm_multibuttonentry_item_data_get(const Elm_Object_Item *it)
{
   return elm_widget_item_data_get(it);
}

EINA_DEPRECATED EAPI void
elm_multibuttonentry_item_data_set(Elm_Object_Item *it,
                                   void *data)
{
   return elm_widget_item_data_set(it, data);
}

EAPI void
elm_multibuttonentry_item_filter_append(Evas_Object *obj,
                                        Elm_Multibuttonentry_Item_Filter_Cb func,
                                        void *data)
{
   Elm_Multibuttonentry_Item_Filter *new_item_filter = NULL;
   Elm_Multibuttonentry_Item_Filter *_item_filter = NULL;
   Eina_List *l;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);

   new_item_filter = _filter_new(func, data);
   if (!new_item_filter) return;

   EINA_LIST_FOREACH (sd->filter_list, l, _item_filter)
     {
        if (_item_filter && ((_item_filter->callback_func == func)
                             && (_item_filter->data == data)))
          {
             INF("Already Registered this item filter!!!!\n");
             return;
          }
     }
   sd->filter_list = eina_list_append(sd->filter_list, new_item_filter);
}

EAPI void
elm_multibuttonentry_item_filter_prepend(Evas_Object *obj,
                                         Elm_Multibuttonentry_Item_Filter_Cb func,
                                         void *data)
{
   Elm_Multibuttonentry_Item_Filter *new_item_filter = NULL;
   Elm_Multibuttonentry_Item_Filter *_item_filter = NULL;
   Eina_List *l;

   ELM_MULTIBUTTONENTRY_CHECK(obj);
   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);
   EINA_SAFETY_ON_NULL_RETURN(func);

   new_item_filter = _filter_new(func, data);
   if (!new_item_filter) return;

   EINA_LIST_FOREACH (sd->filter_list, l, _item_filter)
     {
        if (_item_filter && ((_item_filter->callback_func == func)
                             && (_item_filter->data == data)))
          {
             INF("Already Registered this item filter!!!!\n");
             return;
          }
     }
   sd->filter_list = eina_list_prepend(sd->filter_list, new_item_filter);
}

EAPI void
elm_multibuttonentry_item_filter_remove(Evas_Object *obj,
                                        Elm_Multibuttonentry_Item_Filter_Cb func,
                                        void *data)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item_Filter *item_filter;

   ELM_MULTIBUTTONENTRY_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH (sd->filter_list, l, item_filter)
     {
        if ((item_filter->callback_func == func)
            && ((!data) || (item_filter->data == data)))
          {
             sd->filter_list = eina_list_remove_list(sd->filter_list, l);
             _filter_free(item_filter);
             return;
          }
     }
}
