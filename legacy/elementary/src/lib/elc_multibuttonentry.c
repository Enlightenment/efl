#include <Elementary.h>
#include "elm_priv.h"

#define MAX_STR 256
#define MIN_W_ENTRY 10

typedef enum _Multibuttonentry_Pos
  {
     MULTIBUTTONENTRY_POS_START,
     MULTIBUTTONENTRY_POS_END,
     MULTIBUTTONENTRY_POS_BEFORE,
     MULTIBUTTONENTRY_POS_AFTER,
  } Multibuttonentry_Pos;

typedef enum _Multibuttonentry_Button_State
  {
     MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT,
     MULTIBUTTONENTRY_BUTTON_STATE_SELECTED,
  } Multibuttonentry_Button_State;


typedef enum _MultiButtonEntry_Closed_Button_Type
  {
     MULTIBUTTONENTRY_CLOSED_IMAGE,
     MULTIBUTTONENTRY_CLOSED_LABEL
  } MultiButtonEntry_Closed_Button_Type;

typedef enum _Multibuttonentry_View_State
  {
     MULTIBUTTONENTRY_VIEW_NONE,
     MULTIBUTTONENTRY_VIEW_GUIDETEXT,
     MULTIBUTTONENTRY_VIEW_ENTRY,
     MULTIBUTTONENTRY_VIEW_SHRINK
  } Multibuttonentry_View_State;

typedef struct _Widget_Data Widget_Data;
typedef struct _Multibuttonentry_Item Elm_Multibuttonentry_Item;

struct _Multibuttonentry_Item
  {
     ELM_WIDGET_ITEM;
     Evas_Object *button;
     Evas_Coord vw, rw; // vw: visual width, real width
     Eina_Bool  visible: 1;
     Evas_Smart_Cb func;
  };

typedef struct _Elm_Multibuttonentry_Item_Filter
  {
     Elm_Multibuttonentry_Item_Filter_Cb callback_func;
     void *data;
  } Elm_Multibuttonentry_Item_Filter;

struct _Widget_Data
  {
     Evas_Object *base;
     Evas_Object *box;
     Evas_Object *entry;
     Evas_Object *label;
     Evas_Object *guidetext;
     Evas_Object *end;   // used to represent the total number of invisible buttons

     Evas_Object *rect_for_end;
     MultiButtonEntry_Closed_Button_Type end_type;

     Eina_List *items;
     Eina_List *filter_list;
     Elm_Object_Item *selected_it; /* selected item */

     const char *labeltxt, *guidetexttxt;

     int n_str;
     Multibuttonentry_View_State view_state;

     Evas_Coord w_box, h_box;
     int  shrink;
     Eina_Bool focused: 1;
     Eina_Bool last_btn_select: 1;
     Elm_Multibuttonentry_Item_Filter_Cb add_callback;
     void *add_callback_data;

     Eina_Bool editable;
  };

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _on_focus_hook(void *data __UNUSED__, Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hint_cb(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _resize_cb(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _event_init(Evas_Object *obj);
static void _shrink_mode_set(Evas_Object *obj, Eina_Bool shrink);
static void _view_update(Evas_Object *obj);
static void _set_label(Evas_Object *obj, const char *str);
static void _change_current_button_state(Evas_Object *obj, Multibuttonentry_Button_State state);
static void _change_current_button(Evas_Object *obj, Evas_Object *btn);
static void _button_clicked(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _del_button_obj(Evas_Object *obj, Evas_Object *btn);
static void _del_button_item(Elm_Multibuttonentry_Item *item);
static void _select_button(Evas_Object *obj, Evas_Object *btn);
static Elm_Object_Item *_add_button_item(Evas_Object *obj, const char *str, Multibuttonentry_Pos pos, const void *ref, Evas_Smart_Cb func, void *data);
static void _evas_mbe_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _entry_key_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _entry_key_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _entry_resized_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info);
static void _entry_focus_in_cb(void *data, Evas_Object *obj, void *event_info);
static void _entry_focus_out_cb(void *data, Evas_Object *obj, void *event_info);
static void _entry_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _view_init(Evas_Object *obj);
static void _set_vis_guidetext(Evas_Object *obj);
static void _calculate_box_min_size(Evas_Object *box, Evas_Object_Box_Data *priv);
static Evas_Coord _calculate_item_max_height(Evas_Object *box, Evas_Object_Box_Data *priv, int obj_index);
static void _box_layout_cb(Evas_Object *o, Evas_Object_Box_Data *priv, void *data);
static void _item_text_set_hook(Elm_Object_Item *it,
                                const char *part,
                                const char *label);
static const char *_item_text_get_hook(const Elm_Object_Item *it,
                                       const char *part);

static const char *widtype = NULL;

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

static const Evas_Smart_Cb_Description _signals[] = {
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

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->items)
     {
        Elm_Multibuttonentry_Item *item;
        EINA_LIST_FREE(wd->items, item)
          {
             _del_button_obj(obj, item->button);
             free(item);
          }
        wd->items = NULL;
     }
   wd->selected_it = NULL;

   if (wd->labeltxt) eina_stringshare_del(wd->labeltxt);
   if (wd->guidetexttxt) eina_stringshare_del(wd->guidetexttxt);
   if (wd->entry) evas_object_del(wd->entry);
   if (wd->label) evas_object_del(wd->label);
   if (wd->guidetext) evas_object_del(wd->guidetext);
   if (wd->end) evas_object_del(wd->end);
   if (wd->rect_for_end) evas_object_del(wd->rect_for_end);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   if (!wd) return;

   _elm_theme_object_set(obj, wd->base, "multibuttonentry", "base", elm_widget_style_get(obj));
   if (wd->box) edje_object_part_swallow(wd->base, "box.swallow", wd->box);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);

   EINA_LIST_FOREACH(wd->items, l, item)
     {
        if (item->button)
          _elm_theme_object_set(obj, item->button, "multibuttonentry", "btn", elm_widget_style_get(obj));
        edje_object_scale_set(item->button, elm_widget_scale_get(obj) * _elm_config->scale);
     }

   _sizing_eval(obj);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (elm_widget_focus_get(obj))
     {
        if (wd->editable)
          {
             if ((wd->selected_it))
               {
                  elm_entry_input_panel_show(wd->entry);
               }
             else if (((!wd->selected_it) || (!eina_list_count(wd->items))))
               {
                  if (wd->entry) elm_entry_cursor_end_set(wd->entry);
                  _view_update(obj);
                  elm_entry_input_panel_show(wd->entry);
               }
          }
        wd->focused = EINA_TRUE;
        evas_object_smart_callback_call(obj, SIG_FOCUSED, NULL);
     }
   else
     {
        wd->focused = EINA_FALSE;
        _view_update(obj);

        elm_entry_input_panel_hide(wd->entry);
        evas_object_smart_callback_call(obj, SIG_UNFOCUSED, NULL);
     }
}

static Eina_Bool
_event_hook(Evas_Object *obj __UNUSED__, Evas_Object *src __UNUSED__, Evas_Callback_Type type __UNUSED__, void *event_info __UNUSED__)
{
   return EINA_TRUE;
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   edje_object_signal_emit(wd->base, emission, source);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord left, right, top, bottom;

   if (!wd) return;
   evas_object_size_hint_min_get(wd->box, &minw, &minh);
   edje_object_part_geometry_get(wd->base, "top.left.pad", NULL, NULL, &left, &top);
   edje_object_part_geometry_get(wd->base, "bottom.right.pad", NULL, NULL, &right, &bottom);

   minw += (left + right);
   minh += (top + bottom);

   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_signal_mouse_clicked(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd || !wd->base) return;
   wd->focused = EINA_TRUE;
   _view_update(data);

   if (wd->editable)
     {
        elm_entry_input_panel_show(wd->entry);
        elm_object_focus_set(wd->entry, EINA_TRUE);
     }

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_changed_size_hint_cb(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   Evas_Object *eo = (Evas_Object *)data;
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   _sizing_eval(eo);
}

static void
_resize_cb(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord w, h;

   if (!wd) return;
   evas_object_geometry_get(wd->box, NULL, NULL, &w, &h);

   if (wd->h_box < h) evas_object_smart_callback_call(data, SIG_EXPANDED, NULL);
   else if (wd->h_box > h)
     evas_object_smart_callback_call(data, SIG_CONTRACTED, NULL);

   wd->w_box = w;
   wd->h_box = h;

   _view_update(data);
}

static void
_event_init(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->base) return;
   if (wd->base)
     {
        edje_object_signal_callback_add(wd->base, "mouse,clicked,1", "*", _signal_mouse_clicked, obj);
        evas_object_event_callback_add(wd->base, EVAS_CALLBACK_KEY_UP, _evas_mbe_key_up_cb, obj);
     }

   if (wd->box)
     {
        evas_object_event_callback_add(wd->box, EVAS_CALLBACK_RESIZE, _resize_cb, obj);
        evas_object_event_callback_add(wd->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hint_cb, obj);
     }

   if (wd->entry)
     {
        evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_KEY_UP, _entry_key_up_cb, obj);
        evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_KEY_DOWN, _entry_key_down_cb, obj);
        evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_RESIZE, _entry_resized_cb, obj);
        evas_object_smart_callback_add(wd->entry, "changed", _entry_changed_cb, obj);
        evas_object_smart_callback_add(wd->entry, "focused", _entry_focus_in_cb, obj);
        evas_object_smart_callback_add(wd->entry, "unfocused", _entry_focus_out_cb, obj);
        evas_object_smart_callback_add(wd->entry, "clicked", _entry_clicked_cb, obj);
     }
}

static void
_set_vis_guidetext(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   elm_box_unpack(wd->box, wd->guidetext);
   elm_box_unpack(wd->box, wd->entry);
   if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK) return;

   if (!wd->focused)
     elm_object_focus_set(wd->entry, EINA_FALSE);

   if (wd && (!eina_list_count(wd->items)) && wd->guidetext
       && (!elm_widget_focus_get(obj)) && (!wd->focused) && (!wd->n_str))
     {
        evas_object_hide(wd->entry);
        elm_box_pack_end(wd->box, wd->guidetext);
        evas_object_show(wd->guidetext);
        wd->view_state = MULTIBUTTONENTRY_VIEW_GUIDETEXT;
     }
   else
     {
        evas_object_hide(wd->guidetext);

        if (wd->editable)
          {
             elm_box_pack_end(wd->box, wd->entry);
             evas_object_show(wd->entry);
             if (elm_widget_focus_get(obj) || wd->focused)
               {
                  if (!wd->selected_it)
                    elm_object_focus_set(wd->entry, EINA_TRUE);
               }
          }
        wd->view_state = MULTIBUTTONENTRY_VIEW_ENTRY;
     }
}

static void
_shrink_mode_set(Evas_Object *obj, Eina_Bool shrink)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   if (!wd || !wd->box) return;
   if (wd->view_state == MULTIBUTTONENTRY_VIEW_ENTRY)
     evas_object_hide(wd->entry);
   else if (wd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
     evas_object_hide(wd->guidetext);
   else if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
     {
        evas_object_hide(wd->rect_for_end);
        evas_object_hide(wd->end);
        wd->view_state = MULTIBUTTONENTRY_VIEW_NONE;
     }

   if (shrink == EINA_TRUE)
     {
        Evas_Coord w = 0, w_tmp = 0;
        Evas_Coord box_inner_item_width_padding = 0;

        elm_box_padding_get(wd->box, &box_inner_item_width_padding, NULL);
        // unpack all items and entry
        elm_box_unpack_all(wd->box);
        EINA_LIST_FOREACH(wd->items, l, item)
          {
             if (item)
               {
                  evas_object_hide(item->button);
                  item->visible = EINA_FALSE;
               }
          }
        // pack buttons only 1line
        w = wd->w_box;

        if (wd->label)
          {
             elm_box_pack_end(wd->box, wd->label);
             evas_object_size_hint_min_get(wd->label, &w_tmp, NULL);
             w -= w_tmp;
             w -= box_inner_item_width_padding;
          }

        item = NULL;
        int count = eina_list_count(wd->items);
        Evas_Coord button_min_width = 0;
        /* Evas_Coord button_min_height = 0; */
        if (wd->end_type == MULTIBUTTONENTRY_CLOSED_IMAGE)
          {
             const char *size_str;
             size_str = edje_object_data_get(wd->end, "closed_button_width");
             if (size_str) button_min_width = (Evas_Coord)atoi(size_str);
             /* it use for later
             size_str = edje_object_data_get(wd->end, "closed_button_height");
             if (size_str) button_min_width = (Evas_Coord)atoi(size_str);
              */
          }

        EINA_LIST_FOREACH(wd->items, l, item)
          {
             if (item)
               {
                  int w_label_count = 0;
                  char buf[MAX_STR];

                  elm_box_pack_end(wd->box, item->button);
                  evas_object_show(item->button);
                  item->visible = EINA_TRUE;

                  w -= item->vw;
                  w -= box_inner_item_width_padding;
                  count--;

                  if (wd->end_type == MULTIBUTTONENTRY_CLOSED_LABEL)
                    {
                       if (count > 0)
                         {
                            snprintf(buf, sizeof(buf), "... + %d", count);
                            elm_object_text_set(wd->end, buf);
                            evas_object_size_hint_min_get(wd->end, &w_label_count, NULL);
                         }

                       if (w < 0 || w < w_label_count)
                         {
                            elm_box_unpack(wd->box, item->button);
                            evas_object_hide(item->button);
                            item->visible = EINA_FALSE;

                            count++;
                            snprintf(buf, sizeof(buf), "... + %d", count);
                            elm_object_text_set(wd->end, buf);
                            evas_object_size_hint_min_get(wd->end, &w_label_count, NULL);

                            elm_box_pack_end(wd->box, wd->end);
                            evas_object_show(wd->end);

                            wd->view_state = MULTIBUTTONENTRY_VIEW_SHRINK;
                            evas_object_smart_callback_call(obj, SIG_EXPAND_STATE_CHANGED, (void *)1);
                            break;
                         }
                    }
                  else if (wd->end_type == MULTIBUTTONENTRY_CLOSED_IMAGE)
                    {
                       if (w < button_min_width)
                         {
                            Evas_Coord rectSize;
                            Evas_Coord closed_height = 0;
                            const char *height_str = edje_object_data_get(wd->base, "closed_height");

                            if (height_str) closed_height = (Evas_Coord)atoi(height_str);
                            elm_box_unpack(wd->box, item->button);
                            evas_object_hide(item->button);
                            item->visible = EINA_FALSE;

                            w += item->vw;
                            rectSize = w - button_min_width;
                            if (!wd->rect_for_end)
                              {
                                 Evas *e = evas_object_evas_get(obj);
                                 wd->rect_for_end = evas_object_rectangle_add(e);
                                 evas_object_color_set(wd->rect_for_end, 0, 0, 0, 0);
                              }
                            evas_object_size_hint_min_set(wd->rect_for_end, rectSize, closed_height * _elm_config->scale);
                            elm_box_pack_end(wd->box, wd->rect_for_end);
                            evas_object_show(wd->rect_for_end);

                            elm_box_pack_end(wd->box, wd->end);
                            evas_object_show(wd->end);

                            wd->view_state = MULTIBUTTONENTRY_VIEW_SHRINK;
                            evas_object_smart_callback_call(obj, SIG_EXPAND_STATE_CHANGED, (void *)0);
                            break;
                         }
                    }
               }
          }
     }
   else
     {
        // unpack all items and entry
        elm_box_unpack_all(wd->box);
        EINA_LIST_FOREACH(wd->items, l, item)
          {
             if (item)
               {
                  evas_object_hide(item->button);
                  item->visible = EINA_FALSE;
               }
          }
        evas_object_hide(wd->end);

        if (wd->rect_for_end) evas_object_hide(wd->rect_for_end);

        // pack buttons only 1line

        if (wd->label) elm_box_pack_end(wd->box, wd->label);

        // pack remain btns
        item = NULL;
        EINA_LIST_FOREACH(wd->items, l, item)
          {
             if (item)
               {
                  elm_box_pack_end(wd->box, item->button);
                  evas_object_show(item->button);
                  item->visible = EINA_TRUE;
               }
          }

        wd->view_state = MULTIBUTTONENTRY_VIEW_NONE;
        evas_object_smart_callback_call(obj, SIG_EXPAND_STATE_CHANGED,
                                        (void *)(long)wd->shrink);
     }
   if (wd->view_state != MULTIBUTTONENTRY_VIEW_SHRINK)
     {
        _set_vis_guidetext(obj);
     }
}

static void
_view_update(Evas_Object *obj)
{
   Evas_Coord width = 1, height = 1;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->box || !wd->entry || !(wd->w_box > 0)) return;

   // update label
   if (wd->label)
     {
        elm_box_unpack(wd->box, wd->label);
        elm_box_pack_start(wd->box, wd->label);
        evas_object_size_hint_min_get(wd->label, &width, &height);
     }

   if (wd->guidetext)
     {
        Evas_Coord guide_text_width = wd->w_box - width;
        evas_object_size_hint_min_set(wd->guidetext, guide_text_width, height);
     }

   // update buttons in shrink mode
   if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
     _shrink_mode_set(obj, EINA_TRUE);

   // update guidetext
   _set_vis_guidetext(obj);
}

static void
_set_label(Evas_Object *obj, const char *str)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !str) return;
   eina_stringshare_replace(&wd->labeltxt, str);
   if (wd->label)
     {
        Evas_Coord width, height, sum_width = 0;
        evas_object_size_hint_min_set(wd->label, 0, 0);
        evas_object_resize(wd->label, 0, 0);
        edje_object_part_text_escaped_set(wd->label, "mbe.label", str);

        if (!strcmp(str, ""))
          {
             /* FIXME: not work yet */
             edje_object_signal_emit(wd->label, "elm,mbe,clear_text", "");
             edje_object_part_geometry_get(wd->label, "mbe.label", NULL, NULL, &width, &height);
             sum_width += width;
          }
        else
          {
             edje_object_signal_emit(wd->label, "elm,mbe,set_text", "");
             edje_object_part_geometry_get(wd->label, "mbe.label", NULL, NULL, &width, &height);

             sum_width += width;

             edje_object_part_geometry_get(wd->label, "mbe.label.left.padding", NULL, NULL, &width, NULL);
             sum_width += width;

             edje_object_part_geometry_get(wd->label, "mbe.label.right.padding", NULL, NULL, &width, NULL);
             sum_width += width;
          }
        evas_object_size_hint_min_set(wd->label, sum_width, height);
     }
   evas_object_show(wd->label);
   _view_update(obj);
}

static void
_set_guidetext(Evas_Object *obj, const char *str)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !str) return;

   eina_stringshare_replace(&wd->guidetexttxt, str);
   if (wd->guidetext == NULL)
     wd->guidetext = edje_object_add(evas_object_evas_get(obj));

   if (wd->guidetext)
     {
        _elm_theme_object_set(obj, wd->guidetext, "multibuttonentry",
                       "guidetext", elm_widget_style_get(obj));
        evas_object_size_hint_weight_set(wd->guidetext, 0.0, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(wd->guidetext, EVAS_HINT_FILL,
                                                           EVAS_HINT_FILL);
        edje_object_part_text_escaped_set(wd->guidetext, "elm.text", str);
        _view_update(obj);
     }
}

static void
_change_current_button_state(Evas_Object *obj, Multibuttonentry_Button_State state)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Multibuttonentry_Item *item = NULL;

   if (!wd) return;
   item = (Elm_Multibuttonentry_Item *)wd->selected_it;

   if (item && item->button)
     {
        switch (state)
          {
           case MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT:
              edje_object_signal_emit(item->button, "default", "");
              wd->selected_it = NULL;
              break;
           case MULTIBUTTONENTRY_BUTTON_STATE_SELECTED:
              edje_object_signal_emit(item->button, "focused", "");
              evas_object_smart_callback_call(obj, SIG_ITEM_SELECTED, item);
              break;
           default:
              edje_object_signal_emit(item->button, "default", "");
              wd->selected_it = NULL;
              break;
          }
     }
}

static void
_change_current_button(Evas_Object *obj, Evas_Object *btn)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Multibuttonentry_Item *item;

   if (!wd) return;

   // change the state of previous button to "default"
   _change_current_button_state(obj, MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT);

   // change the current
   EINA_LIST_FOREACH(wd->items, l, item)
     {
        if (item->button == btn)
          {
             wd->selected_it = (Elm_Object_Item *)item;
             break;
          }
     }
   // change the state of current button to "focused"
   _change_current_button_state(obj, MULTIBUTTONENTRY_BUTTON_STATE_SELECTED);
}

static void
_button_clicked(void *data, Evas_Object *obj, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   Elm_Multibuttonentry_Item *item = NULL;
   if (!wd || wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK) return;

   _change_current_button(data, obj);

   if (wd->selected_it)
     if ((item = (Elm_Multibuttonentry_Item *)wd->selected_it) != NULL)
       {
          evas_object_smart_callback_call(data, SIG_ITEM_CLICKED, item);
          _select_button(data, item->button);
       }
}

static void
_del_button_obj(Evas_Object *obj, Evas_Object *btn)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !btn) return;
   if (btn)
     evas_object_del(btn);
}

static void
_del_button_item(Elm_Multibuttonentry_Item *item)
{
   Eina_List *l;
   Elm_Multibuttonentry_Item *_item;
   if (!item) return;
   Widget_Data *wd;

   Evas_Object *obj = WIDGET(item);
   wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_LIST_FOREACH(wd->items, l, _item)
     {
        if (_item == item)
          {
             wd->items = eina_list_remove(wd->items, _item);
             elm_box_unpack(wd->box, _item->button);

             evas_object_smart_callback_call(obj, SIG_ITEM_DELETED, _item);

             _del_button_obj(obj, _item->button);

             if (wd->selected_it == (Elm_Object_Item *)_item)
               wd->selected_it = NULL;
             break;
          }
     }
   if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
     _shrink_mode_set(obj, EINA_TRUE);

   if (!eina_list_count(wd->items))
     _set_vis_guidetext(obj);
}

static void
_select_button(Evas_Object *obj, Evas_Object *btn)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (btn)
     {
        _change_current_button(obj, btn);
        if (elm_widget_focus_get(obj))
          {
             elm_object_focus_set(wd->entry, EINA_FALSE);
             evas_object_focus_set(btn, EINA_TRUE);
          }
     }
   else
     {
        _change_current_button_state(obj, MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT);
        if (elm_widget_focus_get(obj) && wd->editable)
          elm_object_focus_set(wd->entry, EINA_TRUE);
     }
}

static void
_resize_button(Evas_Object *btn, Evas_Coord *realw, Evas_Coord *vieww)
{
   Evas_Coord rw, vw;
   Evas_Coord w_text, h_btn, padding_outer = 0;
   Evas_Coord w_btn = 0, button_max_width = 0;
   const char *size_str;

   size_str = edje_object_data_get(btn, "button_max_size");
   if (size_str) button_max_width = (Evas_Coord)atoi(size_str);

   // decide the size of button
   edje_object_part_geometry_get(btn, "elm.base", NULL, NULL, NULL, &h_btn);
   edje_object_part_geometry_get(btn, "elm.btn.text", NULL, NULL, &w_text, NULL);
   edje_object_part_geometry_get(btn, "right.padding", NULL, NULL, &padding_outer, NULL);
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

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   _del_button_item((Elm_Multibuttonentry_Item *)it);
   return EINA_TRUE;
}

static Elm_Object_Item*
_add_button_item(Evas_Object *obj, const char *str, Multibuttonentry_Pos pos, const void *ref, Evas_Smart_Cb func, void *data)
{
   Elm_Multibuttonentry_Item *item;
   Elm_Multibuttonentry_Item_Filter *item_filter;
   Elm_Multibuttonentry_Item *reference = (Elm_Multibuttonentry_Item *)ref;
   Eina_List *l;
   Evas_Object *btn;
   Evas_Coord width = -1, height = -1;
   char *str_utf8 = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->box || !wd->entry) return NULL;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   if (!str) return NULL;

   EINA_LIST_FOREACH(wd->filter_list, l, item_filter)
     {
        if (!(item_filter->callback_func(obj, str, data, item_filter->data)))
          return NULL;
     }
   // add button
   btn = edje_object_add(evas_object_evas_get(obj));
   str_utf8 = elm_entry_markup_to_utf8(str);

   //entry is cleared when text is made to button
   elm_object_text_set(wd->entry, "");

   _elm_theme_object_set(obj, btn, "multibuttonentry", "btn", elm_widget_style_get(obj));
   edje_object_part_text_escaped_set(btn, "elm.btn.text", str_utf8);
   edje_object_part_geometry_get(btn, "elm.btn.text", NULL, NULL, &width, &height);

   evas_object_size_hint_min_set(btn, width, height);

   edje_object_signal_callback_add(btn, "mouse,clicked,1", "*", _button_clicked, obj);
   evas_object_size_hint_weight_set(btn, 0.0, 0.0);
   evas_object_show(btn);

   // append item list
   item = elm_widget_item_new(obj, Elm_Multibuttonentry_Item);
   if (item)
     {
        elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
        elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
        elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
        elm_widget_item_data_set(item, data);
        Evas_Coord rw, vw;
        _resize_button(btn, &rw, &vw);
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
              wd->items = eina_list_prepend(wd->items, item);
              if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
                {
                   elm_widget_sub_object_add(obj, btn);
                   _shrink_mode_set(obj, EINA_TRUE);
                }
              else
                {
                   if (wd->label)
                     elm_box_pack_after(wd->box, btn, wd->label);
                   else
                     elm_box_pack_start(wd->box, btn);
                   if (wd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
                     _set_vis_guidetext(obj);
                }
              break;
           case MULTIBUTTONENTRY_POS_END:
              wd->items = eina_list_append(wd->items, item);
              if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
                {
                   elm_widget_sub_object_add(obj, btn);
                   evas_object_hide(btn);
                }
              else
                {
                   if (wd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
                     _set_vis_guidetext(obj);
                   if (wd->entry && wd->editable)
                     elm_box_pack_before(wd->box, btn, wd->entry);
                   else
                     elm_box_pack_end(wd->box, btn);
                }
              break;
           case MULTIBUTTONENTRY_POS_BEFORE:
              if (reference)
                wd->items = eina_list_prepend_relative(wd->items, item, reference);
              else
                wd->items = eina_list_append(wd->items, item);
              if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
                {
                   elm_widget_sub_object_add(obj, btn);
                   evas_object_hide(btn);
                   _shrink_mode_set(obj, EINA_TRUE);
                }
              else
                {
                   if (reference)
                     elm_box_pack_before(wd->box, btn, reference->button);
                   else
                     {
                        if (wd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
                          _set_vis_guidetext(obj);
                        if (wd->entry && wd->editable)
                          elm_box_pack_before(wd->box, btn, wd->entry);
                        else
                          elm_box_pack_end(wd->box, btn);
                     }
                }
              break;
           case MULTIBUTTONENTRY_POS_AFTER:
              if (reference)
                wd->items = eina_list_append_relative(wd->items, item, reference);
              else
                wd->items = eina_list_append(wd->items, item);
              if (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK)
                {
                   elm_widget_sub_object_add(obj, btn);
                   _shrink_mode_set(obj, EINA_TRUE);
                }
              else
                {
                   if (reference)
                     elm_box_pack_after(wd->box, btn, reference->button);
                   else
                     {
                        if (wd->view_state == MULTIBUTTONENTRY_VIEW_GUIDETEXT)
                          _set_vis_guidetext(obj);
                        if (wd->entry && wd->editable)
                          elm_box_pack_before(wd->box, btn, wd->entry);
                        else
                          elm_box_pack_end(wd->box, btn);
                     }
                }
              break;
           default:
              break;
          }
     }
   evas_object_smart_callback_call(obj, SIG_ITEM_ADDED, item);

   free(str_utf8);

   return (Elm_Object_Item *)item;
}

static Elm_Multibuttonentry_Item_Filter*
_filter_new(Elm_Multibuttonentry_Item_Filter_Cb func, void *data)
{
   Elm_Multibuttonentry_Item_Filter *item_filter = ELM_NEW(Elm_Multibuttonentry_Item_Filter);
   if (!item_filter) return NULL;

   item_filter->callback_func= func;
   item_filter->data = data;

   return item_filter;
}

static void
_filter_free(Elm_Multibuttonentry_Item_Filter *item_filter)
{
   free(item_filter);
}

static void
_evas_mbe_key_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Multibuttonentry_Item *item = NULL;

   if (!wd || !wd->base || !wd->box) return;

   Evas_Event_Key_Up *ev = (Evas_Event_Key_Up*)event_info;

   if (wd->last_btn_select)
     {
        if (wd->selected_it &&
            ((strcmp(ev->keyname, "BackSpace") == 0) ||
             (strcmp(ev->keyname, "Delete") == 0)))
          {
             item = (Elm_Multibuttonentry_Item *)wd->selected_it;
             if (item && wd->editable)
               {
                  _del_button_item(item);
                  elm_widget_item_free(item);
                  elm_object_focus_set(wd->entry, EINA_TRUE);
               }
          }
        else if (((!wd->selected_it && (wd->n_str == 0) &&
                   (strcmp(ev->keyname, "BackSpace") == 0)) ||
                   (strcmp(ev->keyname, "Delete") == 0)))
          {
             item = eina_list_data_get(eina_list_last(wd->items));
             if (item)
               _select_button(data, item->button);
          }
     }
   else
     wd->last_btn_select = EINA_TRUE;
}

static void
_entry_key_down_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Key_Down *ev = (Evas_Event_Key_Down *)event_info;

   if (!wd) return;

   if ((wd->n_str == 1) && (strcmp(ev->keyname, "BackSpace") == 0 || (strcmp(ev->keyname, "Delete") == 0 )))
     wd->last_btn_select = EINA_FALSE;
}

static void
_entry_key_up_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Key_Up *ev = (Evas_Event_Key_Up *) event_info;
   const char *str;

   if (!wd || !wd->base || !wd->box) return;

   str = elm_object_text_get(wd->entry);

   if ((strcmp(str, "") != 0) && (strcmp(ev->keyname, "KP_Enter") == 0 || strcmp(ev->keyname, "Return") == 0 ))
     {
        _add_button_item(data, str, MULTIBUTTONENTRY_POS_END, NULL, NULL, NULL);
        wd->n_str = 0;
     }
}

static void
_entry_clicked_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   _change_current_button_state(data, MULTIBUTTONENTRY_BUTTON_STATE_DEFAULT);
   elm_object_focus_set(wd->entry, EINA_TRUE);
}

static void
_entry_focus_in_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Multibuttonentry_Item *item = NULL;

   if (!wd) return;

   if (wd->selected_it)
     {
        item = (Elm_Multibuttonentry_Item *)wd->selected_it;
        elm_object_focus_set(wd->entry, EINA_FALSE);
        evas_object_focus_set(item->button, EINA_TRUE);
     }
}

static void
_entry_focus_out_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const char *str;

   if (!wd) return;

   str = elm_object_text_get(wd->entry);
   if (strlen(str))
     _add_button_item(data, str, MULTIBUTTONENTRY_POS_END, NULL, NULL, NULL);
}

static void
_entry_changed_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   const char *str;

   if (!wd) return;

   str = elm_object_text_get(wd->entry);
   wd->n_str = strlen(str);
}

static void
_entry_resized_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Coord en_x, en_y, en_w, en_h;

   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   evas_object_geometry_get(wd->entry, &en_x, &en_y, &en_w, &en_h);

   if (wd->focused)
     elm_widget_show_region_set(wd->entry, en_x, en_y, en_w, en_h, EINA_TRUE);
}

static void
_view_init(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (!wd->box)
     {
        wd->box = elm_box_add(obj);
        if (!wd->box) return;
        elm_widget_sub_object_add(obj, wd->box);
        elm_box_layout_set(wd->box, _box_layout_cb, NULL, NULL);
        elm_box_homogeneous_set(wd->box, EINA_FALSE);
        edje_object_part_swallow(wd->base, "box.swallow", wd->box);
     }
   if (!wd->label)
     {
        wd->label = edje_object_add(evas_object_evas_get(obj));
        if (!wd->label) return;
        _elm_theme_object_set(obj, wd->label, "multibuttonentry", "label", elm_widget_style_get(obj));
        _set_label(obj, "");
        elm_widget_sub_object_add(obj, wd->label);
     }

   if (!wd->entry)
     {
        wd->entry = elm_entry_add(obj);
        if (!wd->entry) return;
        elm_entry_single_line_set(wd->entry, EINA_TRUE);
        elm_object_text_set(wd->entry, "");
        elm_entry_input_panel_enabled_set(wd->entry, EINA_FALSE);
        evas_object_size_hint_min_set(wd->entry, MIN_W_ENTRY, 0);
        evas_object_size_hint_weight_set(wd->entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(wd->entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
        if (wd->box) elm_box_pack_end(wd->box, wd->entry);
        evas_object_show(wd->entry);
        wd->view_state = MULTIBUTTONENTRY_VIEW_ENTRY;
     }

   if (!wd->end)
     {
        const char *end_type;

        end_type = edje_object_data_get(wd->base, "closed_button_type");
        if (!end_type || !strcmp(end_type, "label"))
          {
             wd->end = elm_label_add(obj);
             if (!wd->end) return;
             elm_object_style_set(wd->end, "extended/multibuttonentry_default");
             wd->end_type = MULTIBUTTONENTRY_CLOSED_LABEL;
          }
        else
          {
             const char *size_str;
             wd->end = edje_object_add(evas_object_evas_get(obj));
             if (!wd->end) return;
             _elm_theme_object_set(obj, wd->end, "multibuttonentry", "closedbutton", elm_widget_style_get(obj));
             Evas_Coord button_min_width = 0;
             Evas_Coord button_min_height = 0;

             size_str = edje_object_data_get(wd->end, "closed_button_width");
             if (size_str) button_min_width = (Evas_Coord)atoi(size_str);
             size_str = edje_object_data_get(wd->end, "closed_button_height");
             if (size_str) button_min_height = (Evas_Coord)atoi(size_str);

             wd->end_type = MULTIBUTTONENTRY_CLOSED_IMAGE;
             evas_object_size_hint_min_set(wd->end,
                                           button_min_width * _elm_config->scale,
                                           button_min_height * _elm_config->scale);
             elm_widget_sub_object_add(obj, wd->end);
          }
     }
}

static void
_calculate_box_min_size(Evas_Object *box, Evas_Object_Box_Data *priv)
{
   Evas_Coord minw, minh, mnw, mnh, ww;
   Evas_Coord w, cw = 0, cmaxh = 0;
   const Eina_List *l;
   Evas_Object_Box_Option *opt;
   double wx;

   /* FIXME: need to calc max */
   minw = 0;
   minh = 0;

   evas_object_geometry_get(box, NULL, NULL, &w, NULL);
   evas_object_size_hint_min_get(box, &minw, NULL);

   EINA_LIST_FOREACH(priv->children, l, opt)
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
             cmaxh = 0;
          }
        cw += ww;
        if (cmaxh < mnh) cmaxh = mnh;
     }

   minh += cmaxh;

   evas_object_size_hint_min_set(box, minw, minh);
}

static Evas_Coord
_calculate_item_max_height(Evas_Object *box, Evas_Object_Box_Data *priv, int obj_index)
{
   Evas_Coord mnw, mnh, cw = 0, cmaxh = 0, w, ww;
   const Eina_List *l;
   Evas_Object_Box_Option *opt;
   int local_index = 0;
   double wx;

   evas_object_geometry_get(box, NULL, NULL, &w, NULL);

   EINA_LIST_FOREACH(priv->children, l, opt)
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
             cmaxh = 0;
          }

        cw += ww;
        if (cmaxh < mnh) cmaxh = mnh;

        local_index++;
     }

   return cmaxh;
}

static void
_box_layout_cb(Evas_Object *o, Evas_Object_Box_Data *priv, void *data __UNUSED__)
{
   Evas_Coord x, y, w, h, xx, yy;
   const Eina_List *l;
   Evas_Object *obj;
   Evas_Coord minw, minh;
   double ax, ay;
   Evas_Object_Box_Option *opt;

   _calculate_box_min_size(o, priv);

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

   Evas_Coord cw = 0, ch = 0, cmaxh = 0, obj_index = 0;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Coord mnw, mnh, mxw, mxh;
        double wx, wy;
        int fw, fh;

        obj = opt->obj;
        evas_object_size_hint_align_get(obj, &ax, &ay);
        evas_object_size_hint_weight_get(obj, &wx, &wy);
        evas_object_size_hint_min_get(obj, &mnw, &mnh);
        evas_object_size_hint_max_get(obj, &mxw, &mxh);
        fw = fh = 0;
        if (ax == -1.0) {fw = 1; ax = 0.5;}
        if (ay == -1.0) {fh = 1; ay = 0.5;}
        Evas_Coord ww, hh, ow, oh;

        if (wx)
          {
             if (mnw != -1 && (w - cw) >= mnw)
                ww = w - cw;
             else
                ww = w;
          }
        else
           ww = mnw;
        hh = _calculate_item_max_height(o, priv, obj_index);

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
             cmaxh = 0;
          }

        evas_object_move(obj,
                         xx + cw + (Evas_Coord)(((double)(ww - ow)) * ax),
                         yy + ch + (Evas_Coord)(((double)(hh - oh)) * ay));
        evas_object_resize(obj, ow, oh);

        cw += ww;
        if (cmaxh < hh) cmaxh = hh;

        obj_index++;
     }
}

static void
_item_text_set_hook(Elm_Object_Item *it, const char *part, const char *label)
{
   Elm_Multibuttonentry_Item *item;
   if (part && strcmp(part, "default")) return;
   if (!label) return;
   item = (Elm_Multibuttonentry_Item *)it;
   edje_object_part_text_escaped_set(item->button, "elm.btn.text", label);
   _resize_button(item->button, &item->rw, &item->vw);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it, const char *part)
{
   Elm_Multibuttonentry_Item *item;
   if (part && strcmp(part, "default")) return NULL;
   item = (Elm_Multibuttonentry_Item *)it;
   return edje_object_part_text_get(item->button, "elm.btn.text");
}

static void
_text_set_hook(Evas_Object *obj, const char *part, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   if (!part || !strcmp(part, "default"))
     {
        if (label) _set_label(obj, label);
        else _set_label(obj, "");
     }
   else if (!strcmp(part, "guide"))
     {
        if (label) _set_guidetext(obj, label);
        else _set_guidetext(obj, "");
     }
}

static const char *
_text_get_hook(const Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if (!part || !strcmp(part, "default"))
     {
        return wd->labeltxt;
     }
   else if (!strcmp(part, "guide"))
     {
        return wd->guidetexttxt;
     }
   return NULL;
}

EAPI Evas_Object *
elm_multibuttonentry_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "multibuttonentry");
   elm_widget_type_set(obj, "multibuttonentry");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);

   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_text_set_hook_set(obj, _text_set_hook);
   elm_widget_text_get_hook_set(obj, _text_get_hook);

   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "multibuttonentry", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   wd->view_state = MULTIBUTTONENTRY_VIEW_NONE;
   wd->focused = EINA_FALSE;
   wd->last_btn_select = EINA_TRUE;
   wd->n_str = 0;
   wd->rect_for_end = NULL;
   wd->add_callback = NULL;
   wd->add_callback_data = NULL;
   wd->editable = EINA_TRUE;

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _view_init(obj);
   _event_init(obj);

   return obj;
}

EAPI Evas_Object *
elm_multibuttonentry_entry_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;

   return wd->entry;
}

EAPI Eina_Bool
elm_multibuttonentry_expanded_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return -1;
     return (wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK ? EINA_FALSE : EINA_TRUE);
}

EAPI void
elm_multibuttonentry_expanded_set(Evas_Object *obj, Eina_Bool expanded)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->box ||
       ((wd->view_state == MULTIBUTTONENTRY_VIEW_SHRINK) ? EINA_FALSE : EINA_TRUE) == expanded) return;

   if (expanded)
     _shrink_mode_set(obj, EINA_FALSE);
   else
     _shrink_mode_set(obj, EINA_TRUE);

}

EAPI void
elm_multibuttonentry_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->editable == editable) return;
   wd->editable = editable;

   if (wd->entry)
     {
        if (wd->editable)
          evas_object_show(wd->entry);
        else
          {
             elm_entry_input_panel_hide(wd->entry);
             evas_object_hide(wd->entry);
          }
     }
}

EAPI Eina_Bool
elm_multibuttonentry_editable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return -1;
   return wd->editable;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_prepend(Evas_Object *obj, const char *label, Evas_Smart_Cb func, void *data)
{
   return _add_button_item(obj, label, MULTIBUTTONENTRY_POS_START, NULL, func, data);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_append(Evas_Object *obj, const char *label, Evas_Smart_Cb func, void *data)
{
   return _add_button_item(obj, label, MULTIBUTTONENTRY_POS_END, NULL, func, data);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *label, Evas_Smart_Cb func, void *data)
{
   return _add_button_item(obj, label, MULTIBUTTONENTRY_POS_BEFORE, before, func, data);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *label, Evas_Smart_Cb func, void *data)
{
   return _add_button_item(obj, label, MULTIBUTTONENTRY_POS_AFTER, after, func, data);
}

EAPI const Eina_List *
elm_multibuttonentry_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->items;
}

EAPI Elm_Object_Item *
elm_multibuttonentry_first_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return eina_list_data_get(wd->items);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return eina_list_data_get(eina_list_last(wd->items));
}

EAPI Elm_Object_Item *
elm_multibuttonentry_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected_it;
}

EAPI void
elm_multibuttonentry_item_selected_set(Elm_Object_Item *it, Eina_Bool selected)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Multibuttonentry_Item *item = (Elm_Multibuttonentry_Item *)it;
   if (selected) _select_button(WIDGET(item), item->button);
   else _select_button(WIDGET(item), NULL);
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
   ELM_CHECK_WIDTYPE(obj, widtype);
   Elm_Multibuttonentry_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->items)
     {
        EINA_LIST_FREE(wd->items, item)
          {
             elm_box_unpack(wd->box, item->button);
             _del_button_obj(obj, item->button);
             free(item);
          }
        wd->items = NULL;
     }
   wd->selected_it = NULL;
   _view_update(obj);
}

EAPI Elm_Object_Item *
elm_multibuttonentry_item_prev_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Widget_Data *wd;
   Eina_List *l;
   Elm_Multibuttonentry_Item *_item;

   wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return NULL;

   EINA_LIST_FOREACH(wd->items, l, _item)
     {
        if (_item == (Elm_Multibuttonentry_Item *)it)
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
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Widget_Data *wd;
   Eina_List *l;
   Elm_Multibuttonentry_Item *_item;
   wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return NULL;

   EINA_LIST_FOREACH(wd->items, l, _item)
     {
        if (_item == (Elm_Multibuttonentry_Item *)it)
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
elm_multibuttonentry_item_data_set(Elm_Object_Item *it, void *data)
{
   return elm_widget_item_data_set(it, data);
}

EAPI void
elm_multibuttonentry_item_filter_append(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data)
{
   Elm_Multibuttonentry_Item_Filter *new_item_filter = NULL;
   Elm_Multibuttonentry_Item_Filter *_item_filter = NULL;
   Eina_List *l;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   ELM_CHECK_WIDTYPE(obj, widtype);
   EINA_SAFETY_ON_NULL_RETURN(func);

   new_item_filter= _filter_new(func, data);
   if (!new_item_filter) return;

   EINA_LIST_FOREACH(wd->filter_list, l, _item_filter)
     {
        if (_item_filter && ((_item_filter->callback_func == func) && (_item_filter->data == data)))
          {
             printf("Already Registered this item filter!!!!\n");
             return;
          }
     }
   wd->filter_list = eina_list_append(wd->filter_list, new_item_filter);
}

EAPI void
elm_multibuttonentry_item_filter_prepend(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data)
{
   Elm_Multibuttonentry_Item_Filter *new_item_filter = NULL;
   Elm_Multibuttonentry_Item_Filter *_item_filter = NULL;
   Eina_List *l;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   ELM_CHECK_WIDTYPE(obj, widtype);
   EINA_SAFETY_ON_NULL_RETURN(func);

   new_item_filter = _filter_new(func, data);
   if (!new_item_filter) return;

   EINA_LIST_FOREACH(wd->filter_list, l, _item_filter)
     {
        if (_item_filter && ((_item_filter->callback_func == func) && (_item_filter->data == data)))
          {
             printf("Already Registered this item filter!!!!\n");
             return;
          }
     }
   wd->filter_list = eina_list_prepend(wd->filter_list, new_item_filter);
}

EAPI void
elm_multibuttonentry_item_filter_remove(Evas_Object *obj, Elm_Multibuttonentry_Item_Filter_Cb func, void *data)
{
   Widget_Data *wd;
   Eina_List *l;
   Elm_Multibuttonentry_Item_Filter *item_filter;

   wd = elm_widget_data_get(obj);

   EINA_SAFETY_ON_NULL_RETURN(func);

   EINA_LIST_FOREACH(wd->filter_list, l, item_filter)
     {
        if ((item_filter->callback_func == func) && ((!data) || (item_filter->data == data)))
          {
             wd->filter_list = eina_list_remove_list(wd->filter_list, l);
             _filter_free(item_filter);
             return;
          }
     }
}
