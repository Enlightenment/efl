#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_segment_control.h"

EAPI const char ELM_SEGMENT_CONTROL_SMART_NAME[] = "elm_segment_control";

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_SEGMENT_CONTROL_SMART_NAME, _elm_segment_control,
  Elm_Segment_Control_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static void
_elm_segment_control_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;
   int item_count;

   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   item_count = eina_list_count(sd->items);

   elm_coords_finger_size_adjust(item_count, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(item_count, &minw, 1, &minh);

   evas_object_size_hint_min_get(obj, &w, &h);

   if (w > minw) minw = w;
   if (h > minh) minh = h;

   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_item_free(Elm_Segment_Item *it)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (sd->selected_item == it) sd->selected_item = NULL;
   if (sd->items) sd->items = eina_list_remove(sd->items, it);

   if (it->icon) evas_object_del(it->icon);
   if (it->label) eina_stringshare_del(it->label);
}

static void
_position_items(Elm_Segment_Control_Smart_Data *sd)
{
   Eina_List *l;
   Eina_Bool rtl;
   int item_count;
   Elm_Segment_Item *it;
   int bx, by, bw, bh, pos;

   item_count = eina_list_count(sd->items);
   if (item_count <= 0) return;

   evas_object_geometry_get
     (ELM_WIDGET_DATA(sd)->resize_obj, &bx, &by, &bw, &bh);
   sd->item_width = bw / item_count;
   rtl = elm_widget_mirrored_get(ELM_WIDGET_DATA(sd)->obj);

   if (rtl) pos = bx + bw - sd->item_width;
   else pos = bx;

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        evas_object_move(VIEW(it), pos, by);
        evas_object_resize(VIEW(it), sd->item_width, bh);
        if (rtl) pos -= sd->item_width;
        else pos += sd->item_width;
     }

   elm_layout_sizing_eval(ELM_WIDGET_DATA(sd)->obj);
}

static void
_swallow_item_objects(Elm_Segment_Item *it)
{
   if (!it) return;

   if (it->icon)
     {
        edje_object_part_swallow(VIEW(it), "elm.swallow.icon", it->icon);
        edje_object_signal_emit(VIEW(it), "elm,state,icon,visible", "elm");
     }
   else edje_object_signal_emit(VIEW(it), "elm,state,icon,hidden", "elm");

   if (it->label)
     {
        edje_object_part_text_escaped_set(VIEW(it), "elm.text", it->label);
        edje_object_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
     }
   else
     edje_object_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");

   edje_object_message_signal_process(VIEW(it));
}

static void
_update_list(Elm_Segment_Control_Smart_Data *sd)
{
   int idx = 0;
   Eina_List *l;
   Eina_Bool rtl;
   int item_count;
   Elm_Segment_Item *it;

   _position_items(sd);

   item_count = eina_list_count(sd->items);

   if (item_count == 1)
     {
        it = eina_list_nth(sd->items, 0);
        it->seg_index = 0;

        //Set the segment type
        edje_object_signal_emit(VIEW(it), "elm,type,segment,single", "elm");

        //Set the segment state
        if (sd->selected_item == it)
          edje_object_signal_emit
            (VIEW(it), "elm,state,segment,selected", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");

        if (elm_widget_disabled_get(ELM_WIDGET_DATA(sd)->obj))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        return;
     }

   rtl = elm_widget_mirrored_get(ELM_WIDGET_DATA(sd)->obj);
   EINA_LIST_FOREACH (sd->items, l, it)
     {
        it->seg_index = idx;

        //Set the segment type
        if (idx == 0)
          {
             if (rtl)
               edje_object_signal_emit
                 (VIEW(it), "elm,type,segment,right", "elm");
             else
               edje_object_signal_emit
                 (VIEW(it), "elm,type,segment,left", "elm");
          }
        else if (idx == (item_count - 1))
          {
             if (rtl)
               edje_object_signal_emit
                 (VIEW(it), "elm,type,segment,left", "elm");
             else
               edje_object_signal_emit
                 (VIEW(it), "elm,type,segment,right", "elm");
          }
        else
          edje_object_signal_emit(VIEW(it), "elm,type,segment,middle", "elm");

        //Set the segment state
        if (sd->selected_item == it)
          edje_object_signal_emit
            (VIEW(it), "elm,state,segment,selected", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");

        if (elm_widget_disabled_get(ELM_WIDGET_DATA(sd)->obj))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        idx++;
     }
}

static Eina_Bool
_elm_segment_control_smart_theme(Evas_Object *obj)
{
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_segment_control_parent_sc)->theme(obj))
     return EINA_FALSE;

   rtl = elm_widget_mirrored_get(obj);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        elm_widget_theme_object_set
          (obj, VIEW(it), "segment_control", "item",
          elm_widget_style_get(obj));
        edje_object_scale_set(VIEW(it), elm_widget_scale_get(VIEW(it)) *
                              elm_config_scale_get());
        edje_object_mirrored_set(VIEW(it), rtl);
     }

   _update_list(sd);

   return EINA_TRUE;
}

static Eina_Bool
_elm_segment_control_smart_disable(Evas_Object *obj)
{
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_segment_control_parent_sc)->disable(obj))
     return EINA_FALSE;

   _update_list(sd);

   return EINA_TRUE;
}

// TODO: elm_widget_focus_list_next_get supports only Elm_widget list,
// not the Elm_Widget_item. Focus switching within widget not
// supported until it is supported in elm_widget
#if 0
static void *
_elm_list_data_get(const Eina_List *list)
{
   Elm_Segment_Item *it = eina_list_data_get(list);

   if (it) return NULL;

   edje_object_signal_emit(VIEW(it), "elm,state,segment,selected", "elm");

   return VIEW(it);
}

static Eina_Bool
_elm_segment_control_smart_focus_next(const Evas_Object *obj,
                                      Elm_Focus_Direction dir,
                                      Evas_Object **next)
{
   static int count = 0;
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   /* Focus chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->items;
        list_data_get = _elm_list_data_get;
        if (!items) return EINA_FALSE;
     }

   return elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
}

#endif

static void
_segment_off(Elm_Segment_Item *it)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");

   if (sd->selected_item == it) sd->selected_item = NULL;
}

static void
_segment_on(Elm_Segment_Item *it)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (it == sd->selected_item) return;

   if (sd->selected_item) _segment_off(sd->selected_item);

   edje_object_signal_emit(VIEW(it), "elm,state,segment,selected", "elm");

   sd->selected_item = it;
   evas_object_smart_callback_call(ELM_WIDGET_DATA(sd)->obj, SIG_CHANGED, it);
}

static void
_on_move_resize(void *data, Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   ELM_SEGMENT_CONTROL_DATA_GET(data, sd);

   _position_items(sd);
}

static void
_on_mouse_up(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info)
{
   Elm_Segment_Item *it;
   Evas_Event_Mouse_Up *ev;
   Evas_Coord x, y, w, h;

   it = data;
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (elm_widget_disabled_get(ELM_WIDGET_DATA(sd)->obj)) return;

   if (it == sd->selected_item) return;

   ev = event_info;
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);

   if ((ev->canvas.x >= x) && (ev->output.x <= (x + w)) && (ev->canvas.y >= y)
       && (ev->canvas.y <= (y + h)))
     _segment_on(it);
   else
     edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");
}

static void
_on_mouse_down(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   Elm_Segment_Item *it;

   it = data;
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (elm_widget_disabled_get(ELM_WIDGET_DATA(sd)->obj)) return;

   if (it == sd->selected_item) return;

   edje_object_signal_emit(VIEW(it), "elm,state,segment,pressed", "elm");
}

static Elm_Segment_Item *
_item_find(const Evas_Object *obj,
           int idx)
{
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   it = eina_list_nth(sd->items, idx);
   return it;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Segment_Item *item;

   if (part && strcmp(part, "default")) return;

   item = (Elm_Segment_Item *)it;
   eina_stringshare_replace(&item->label, label);
   if (item->label)
     edje_object_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(item));

   //label can be NULL also.
   edje_object_part_text_escaped_set(VIEW(item), "elm.text", item->label);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;
   return ((Elm_Segment_Item *)it)->label;
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Segment_Item *item;

   if (part && strcmp(part, "icon")) return;

   item = (Elm_Segment_Item *)it;
   if (content == item->icon) return;

   //Remove the existing icon
   if (item->icon) evas_object_del(item->icon);
   item->icon = content;
   if (item->icon)
     {
        elm_widget_sub_object_add(VIEW(item), item->icon);
        edje_object_part_swallow(VIEW(item), "elm.swallow.icon", item->icon);
        edje_object_signal_emit(VIEW(item), "elm,state,icon,visible", "elm");
     }
   else
     edje_object_signal_emit(VIEW(item), "elm,state,icon,hidden", "elm");
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   if (part && strcmp(part, "icon")) return NULL;

   return ((Elm_Segment_Item *)it)->icon;
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Segment_Item *item = (Elm_Segment_Item *)it;
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   _item_free(item);
   _update_list(sd);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data,
                Evas_Object *obj __UNUSED__,
                Elm_Widget_Item *item __UNUSED__)
{
   const char *txt = NULL;
   Elm_Segment_Item *it = (Elm_Segment_Item *)data;
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, NULL);

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 Elm_Widget_Item *item __UNUSED__)
{
   Elm_Segment_Item *it = (Elm_Segment_Item *)data;
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (ELM_WIDGET_DATA(sd)->disabled)
     return strdup(E_("State: Disabled"));

   if (it == sd->selected_item)
     return strdup(E_("State: Selected"));
   else
     return strdup(E_("State: Unselected"));
}

static void
_access_widget_item_register(Elm_Segment_Item *it)
{
   Elm_Access_Info *ai;

   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   ai = _elm_access_object_get(it->base.access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Segment Control Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
}

static Elm_Segment_Item *
_item_new(Evas_Object *obj,
          Evas_Object *icon,
          const char *label)
{
   Elm_Segment_Item *it;

   it = elm_widget_item_new(obj, Elm_Segment_Item);
   if (!it) return NULL;

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);

   VIEW(it) = edje_object_add(evas_object_evas_get(obj));
   edje_object_scale_set
     (VIEW(it), elm_widget_scale_get(VIEW(it)) * elm_config_scale_get());
   evas_object_smart_member_add(VIEW(it), obj);

   elm_widget_sub_object_add(obj, VIEW(it));
   elm_widget_theme_object_set
     (obj, VIEW(it), "segment_control", "item", elm_object_style_get(obj));
   edje_object_mirrored_set(VIEW(it), elm_widget_mirrored_get(WIDGET(it)));

   if (label) eina_stringshare_replace(&it->label, label);
   if (it->label)
     edje_object_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(it));
   edje_object_part_text_escaped_set(VIEW(it), "elm.text", label);

   it->icon = icon;
   if (it->icon) elm_widget_sub_object_add(VIEW(it), it->icon);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, it);
   evas_object_show(VIEW(it));

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_widget_item_register(it);

   return it;
}

static void
_elm_segment_control_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Segment_Control_Smart_Data);

   ELM_WIDGET_CLASS(_elm_segment_control_parent_sc)->base.add(obj);

   elm_layout_theme_set
     (obj, "segment_control", "base", elm_widget_style_get(obj));

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _on_move_resize, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _on_move_resize, obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_segment_control_smart_del(Evas_Object *obj)
{
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   EINA_LIST_FREE (sd->items, it)
     {
        _item_free(it);
        elm_widget_item_free(it);
     }

   ELM_WIDGET_CLASS(_elm_segment_control_parent_sc)->base.del(obj);
}

static Eina_Bool
_elm_segment_control_smart_focus_next(const Evas_Object *obj,
                           Elm_Focus_Direction dir,
                           Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *l;
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_CHECK(obj) EINA_FALSE;
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, it)
     items = eina_list_append(items, it->base.access_obj);

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_access_obj_process(Elm_Segment_Control_Smart_Data * sd, Eina_Bool is_access)
{
   Eina_List *l;
   Elm_Segment_Item *it;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (is_access) _access_widget_item_register(it);
        else
          _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
     }
}

static void
_access_hook(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_SEGMENT_CONTROL_CHECK(obj);
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);
   
   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
     _elm_segment_control_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
   _access_obj_process(sd, is_access);
}

static void
_elm_segment_control_smart_set_user(Elm_Segment_Control_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_segment_control_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_segment_control_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_segment_control_smart_theme;
   ELM_WIDGET_CLASS(sc)->disable = _elm_segment_control_smart_disable;

#if 0
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_segment_control_smart_focus_next;
#else
   ELM_WIDGET_CLASS(sc)->focus_next = NULL; /* not 'focus chain manager' */
#endif

   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_segment_control_smart_sizing_eval;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_segment_control_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _access_hook;
}

EAPI const Elm_Segment_Control_Smart_Class *
elm_segment_control_smart_class_get(void)
{
   static Elm_Segment_Control_Smart_Class _sc =
     ELM_SEGMENT_CONTROL_SMART_CLASS_INIT_NAME_VERSION
       (ELM_SEGMENT_CONTROL_SMART_NAME);
   static const Elm_Segment_Control_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_segment_control_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_segment_control_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_segment_control_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Elm_Object_Item *
elm_segment_control_item_add(Evas_Object *obj,
                             Evas_Object *icon,
                             const char *label)
{
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   it = _item_new(obj, icon, label);
   if (!it) return NULL;

   sd->items = eina_list_append(sd->items, it);
   _update_list(sd);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_segment_control_item_insert_at(Evas_Object *obj,
                                   Evas_Object *icon,
                                   const char *label,
                                   int idx)
{
   Elm_Segment_Item *it, *it_rel;

   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   if (idx < 0) idx = 0;

   it = _item_new(obj, icon, label);
   if (!it) return NULL;

   it_rel = _item_find(obj, idx);
   if (it_rel) sd->items = eina_list_prepend_relative(sd->items, it, it_rel);
   else sd->items = eina_list_append(sd->items, it);

   _update_list(sd);

   return (Elm_Object_Item *)it;
}

EAPI void
elm_segment_control_item_del_at(Evas_Object *obj,
                                int idx)
{
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_CHECK(obj);

   it = _item_find(obj, idx);
   if (!it) return;

   elm_object_item_del((Elm_Object_Item *)it);
}

EAPI const char *
elm_segment_control_item_label_get(const Evas_Object *obj,
                                   int idx)
{
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;

   it = _item_find(obj, idx);
   if (it) return it->label;

   return NULL;
}

EAPI Evas_Object *
elm_segment_control_item_icon_get(const Evas_Object *obj,
                                  int idx)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;

   Elm_Segment_Item *it = _item_find(obj, idx);
   if (it) return it->icon;

   return NULL;
}

EAPI int
elm_segment_control_item_count_get(const Evas_Object *obj)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) 0;
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   return eina_list_count(sd->items);
}

EAPI Evas_Object *
elm_segment_control_item_object_get(const Elm_Object_Item *it)
{
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, NULL);

   return VIEW(it);
}

EAPI Elm_Object_Item *
elm_segment_control_item_selected_get(const Evas_Object *obj)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   return (Elm_Object_Item *)sd->selected_item;
}

EAPI void
elm_segment_control_item_selected_set(Elm_Object_Item *it,
                                      Eina_Bool selected)
{
   Elm_Segment_Item *item = (Elm_Segment_Item *)it;

   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it);
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (item == sd->selected_item)
     {
        //already in selected state.
        if (selected) return;

        //unselect case
        _segment_off(item);
     }
   else if (selected)
     _segment_on(item);
}

EAPI Elm_Object_Item *
elm_segment_control_item_get(const Evas_Object *obj,
                             int idx)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;

   return (Elm_Object_Item *)_item_find(obj, idx);
}

EAPI int
elm_segment_control_item_index_get(const Elm_Object_Item *it)
{
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, -1);

   return ((Elm_Segment_Item *)it)->seg_index;
}
