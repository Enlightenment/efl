#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_segment_control.h"

EAPI Eo_Op ELM_OBJ_SEGMENT_CONTROL_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_SEGMENT_CONTROL_CLASS

#define MY_CLASS_NAME "Elm_Segment_Control"
#define MY_CLASS_NAME_LEGACY "elm_segment_control"

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void
_elm_segment_control_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Segment_Control_Smart_Data *sd = _pd;
   Elm_Segment_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate(NULL));

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_segment_control_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;
   int item_count;

   Elm_Segment_Control_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   item_count = eina_list_count(sd->items);

   elm_coords_finger_size_adjust(item_count, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
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

   evas_object_del(it->icon);
   eina_stringshare_del(it->label);
}

static void
_position_items(Elm_Segment_Control_Smart_Data *sd)
{
   Eina_List *l;
   Eina_Bool rtl;
   int item_count;
   Elm_Segment_Item *it;
   int bx, by, bw, bh, pos;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   item_count = eina_list_count(sd->items);
   if (item_count <= 0) return;

   evas_object_geometry_get
     (wd->resize_obj, &bx, &by, &bw, &bh);
   sd->item_width = bw / item_count;
   rtl = elm_widget_mirrored_get(sd->obj);

   if (rtl) pos = bx + bw - sd->item_width;
   else pos = bx;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        evas_object_move(VIEW(it), pos, by);
        evas_object_resize(VIEW(it), sd->item_width, bh);
        if (rtl) pos -= sd->item_width;
        else pos += sd->item_width;
     }

   elm_layout_sizing_eval(sd->obj);
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

        if (elm_widget_disabled_get(sd->obj))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        return;
     }

   rtl = elm_widget_mirrored_get(sd->obj);
   EINA_LIST_FOREACH(sd->items, l, it)
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

        if (elm_widget_disabled_get(sd->obj))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        idx++;
     }
}

static void
_elm_segment_control_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Segment_Item *it;

   Elm_Segment_Control_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   rtl = elm_widget_mirrored_get(obj);

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        elm_widget_theme_object_set
          (obj, VIEW(it), "segment_control", "item",
          elm_widget_style_get(obj));
        edje_object_scale_set(VIEW(it), elm_widget_scale_get(WIDGET(it)) *
                              elm_config_scale_get());
        edje_object_mirrored_set(VIEW(it), rtl);
     }

   _update_list(sd);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_segment_control_smart_disable(Eo *obj, void *_pd, va_list *list)
{
   Elm_Segment_Control_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_disable(&int_ret));
   if (!int_ret) return;

   _update_list(sd);

   if (ret) *ret = EINA_TRUE;
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

static void
_elm_segment_control_smart_focus_next(Eo *obj, void *_pd, va_list *list)
{
   static int count = 0;
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Elm_Segment_Control_Smart_Data *sd = _pd;

   /* Focus chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = sd->items;
        list_data_get = _elm_list_data_get;
        if (!items) return;
     }

   int_ret = elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
   if (ret) *ret = int_ret;
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
   evas_object_smart_callback_call(sd->obj, SIG_CHANGED, it);
}

static void
_on_move_resize(void *data, Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   ELM_SEGMENT_CONTROL_DATA_GET(data, sd);

   _position_items(sd);
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Elm_Segment_Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord x, y, w, h;

   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (elm_widget_disabled_get(sd->obj)) return;

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
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Elm_Segment_Item *it = data;
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;
   if (elm_widget_disabled_get(sd->obj)) return;

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
   char buf[1024];
   item = (Elm_Segment_Item *)it;

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, "elm.text")))
     {
        eina_stringshare_replace(&item->label, label);
        if (label)
          edje_object_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
        else
          edje_object_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");
     }
   else
     {
        if (label)
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,visible", part);
             edje_object_signal_emit(VIEW(item), buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,hidden", part);
             edje_object_signal_emit(VIEW(item), buf, "elm");
          }
     }

   edje_object_message_signal_process(VIEW(item));
   //label can be NULL also.
   edje_object_part_text_escaped_set(VIEW(item), part, label);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return edje_object_part_text_get(VIEW(it), buf);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Segment_Item *item;
   char buf[1024];
   item = (Elm_Segment_Item *)it;

   if (!part || !strcmp("icon", part))
     {
        if (content == item->icon) return;

        evas_object_del(item->icon);
        item->icon = content;
        if (!item->icon)
          {
             elm_widget_sub_object_add(VIEW(item), item->icon);
             edje_object_part_swallow(VIEW(item), "elm.swallow.icon", item->icon);
             edje_object_signal_emit(VIEW(item), "elm,state,icon,visible", "elm");
          }
        else
          edje_object_signal_emit(VIEW(item), "elm,state,icon,hidden", "elm");
     }
   else
     {
        if (content)
          {
             edje_object_part_swallow(VIEW(it), part, content);
             snprintf(buf, sizeof(buf), "elm,state,%s,visible", part);
             edje_object_signal_emit(VIEW(item), buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,hidden", part);
             edje_object_signal_emit(VIEW(item), buf, "elm");
          }
     }
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Segment_Item *item;
   item = (Elm_Segment_Item *)it;

   if (part && !strcmp(part, "icon"))
     return item->icon;
   else
     return edje_object_part_swallow_get(VIEW(item), part);
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
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   const char *txt = NULL;
   Elm_Segment_Item *it = (Elm_Segment_Item *)data;
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, NULL);

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Segment_Item *it = (Elm_Segment_Item *)data;
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(WIDGET(it), wd, NULL);

   if (wd->disabled)
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

   ai = _elm_access_info_get(it->base.access_obj);

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
   edje_object_scale_set(VIEW(it),elm_widget_scale_get(WIDGET(it)) *
                         elm_config_scale_get());
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
_elm_segment_control_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "segment_control", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _on_move_resize, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _on_move_resize, obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_segment_control_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Segment_Item *it;

   Elm_Segment_Control_Smart_Data *sd = _pd;

   EINA_LIST_FREE(sd->items, it)
     {
        _item_free(it);
        elm_widget_item_free(it);
     }

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static Eina_Bool _elm_segment_control_smart_focus_next_enable = EINA_FALSE;

static void
_elm_segment_control_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = _elm_segment_control_smart_focus_next_enable;
}

static void
_elm_segment_control_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;

   Eina_List *items = NULL;
   Eina_List *l;
   Elm_Segment_Item *it;

   ELM_SEGMENT_CONTROL_CHECK(obj);
   Elm_Segment_Control_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->items, l, it)
     items = eina_list_append(items, it->base.access_obj);

   *ret = elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_elm_segment_control_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
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
_elm_segment_control_smart_access(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Segment_Control_Smart_Data *sd = _pd;
   _elm_segment_control_smart_focus_next_enable = va_arg(*list, int);
   _access_obj_process(sd, _elm_segment_control_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_segment_control_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Elm_Segment_Control_Smart_Data *sd = _pd;
   sd->obj = obj;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Elm_Object_Item *
elm_segment_control_item_add(Evas_Object *obj,
                             Evas_Object *icon,
                             const char *label)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do(obj, elm_obj_segment_control_item_add(icon, label, &ret));
   return ret;
}

static void
_item_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *icon = va_arg(*list, Evas_Object *);
   const char *label = va_arg(*list, const char *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Segment_Item *it;

   Elm_Segment_Control_Smart_Data *sd = _pd;

   it = _item_new(obj, icon, label);
   if (!it) return;

   sd->items = eina_list_append(sd->items, it);
   _update_list(sd);

   *ret =(Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_segment_control_item_insert_at(Evas_Object *obj,
                                   Evas_Object *icon,
                                   const char *label,
                                   int idx)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do(obj, elm_obj_segment_control_item_insert_at(icon, label, idx, &ret));
   return ret;
}

static void
_item_insert_at(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *icon = va_arg(*list, Evas_Object *);
   const char *label = va_arg(*list, const char *);
   int idx = va_arg(*list, int);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Segment_Item *it, *it_rel;

   Elm_Segment_Control_Smart_Data *sd = _pd;

   if (idx < 0) idx = 0;

   it = _item_new(obj, icon, label);
   if (!it) return;

   it_rel = _item_find(obj, idx);
   if (it_rel) sd->items = eina_list_prepend_relative(sd->items, it, it_rel);
   else sd->items = eina_list_append(sd->items, it);

   _update_list(sd);

   *ret = (Elm_Object_Item *)it;
}

EAPI void
elm_segment_control_item_del_at(Evas_Object *obj,
                                int idx)
{
   ELM_SEGMENT_CONTROL_CHECK(obj);
   eo_do(obj, elm_obj_segment_control_item_del_at(idx));
}

static void
_item_del_at(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int idx = va_arg(*list, int);
   Elm_Segment_Item *it;

   it = _item_find(obj, idx);
   if (!it) return;

   elm_object_item_del((Elm_Object_Item *)it);
}

EAPI const char *
elm_segment_control_item_label_get(const Evas_Object *obj,
                                   int idx)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   const char *ret;
   eo_do((Eo *) obj, elm_obj_segment_control_item_label_get(idx, &ret));
   return ret;
}

static void
_item_label_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int idx = va_arg(*list, int);
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;
   Elm_Segment_Item *it;

   it = _item_find(obj, idx);
   if (it) *ret = it->label;
}

EAPI Evas_Object *
elm_segment_control_item_icon_get(const Evas_Object *obj,
                                  int idx)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) NULL;
   Evas_Object *ret;
   eo_do((Eo *) obj, elm_obj_segment_control_item_icon_get(idx, &ret));
   return ret;
}

static void
_item_icon_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int idx = va_arg(*list, int);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   *ret = NULL;

   Elm_Segment_Item *it = _item_find(obj, idx);
   if (it) *ret = it->icon;
}

EAPI int
elm_segment_control_item_count_get(const Evas_Object *obj)
{
   ELM_SEGMENT_CONTROL_CHECK(obj) 0;
   int ret;
   eo_do((Eo *) obj, elm_obj_segment_control_item_count_get(&ret));
   return ret;
}

static void
_item_count_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);

   Elm_Segment_Control_Smart_Data *sd = _pd;
   *ret = eina_list_count(sd->items);
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
   Elm_Object_Item *ret;
   eo_do((Eo *) obj, elm_obj_segment_control_item_selected_get(&ret));
   return ret;
}

static void
_item_selected_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Segment_Control_Smart_Data *sd = _pd;
   *ret = (Elm_Object_Item *)sd->selected_item;
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
   Elm_Object_Item *ret;
   eo_do((Eo *) obj, elm_obj_segment_control_item_get(idx, &ret));
   return ret;
}

static void
_item_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int idx = va_arg(*list, int);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = (Elm_Object_Item *)_item_find(obj, idx);
}

EAPI int
elm_segment_control_item_index_get(const Elm_Object_Item *it)
{
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, -1);

   return ((Elm_Segment_Item *)it)->seg_index;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_segment_control_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_segment_control_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_segment_control_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_TRANSLATE), _elm_segment_control_smart_translate),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_DISABLE), _elm_segment_control_smart_disable),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_segment_control_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_segment_control_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_segment_control_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACCESS), _elm_segment_control_smart_access),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_segment_control_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ADD), _item_add),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_INSERT_AT), _item_insert_at),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_DEL_AT), _item_del_at),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_LABEL_GET), _item_label_get),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ICON_GET), _item_icon_get),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_COUNT_GET), _item_count_get),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_SELECTED_GET), _item_selected_get),
        EO_OP_FUNC(ELM_OBJ_SEGMENT_CONTROL_ID(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_GET), _item_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      _elm_segment_control_smart_focus_next_enable = EINA_TRUE;
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ADD, "Append a new item to the segment control object."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_INSERT_AT, "Insert a new item to the segment control object at specified position."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_DEL_AT, "Remove a segment control item at given index from its parent,."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_LABEL_GET, "Get the label of item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_ICON_GET, "Get the icon associated to the item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_COUNT_GET, "Get the Segment items count from segment control."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_SELECTED_GET, "Get the selected item."),
     EO_OP_DESCRIPTION(ELM_OBJ_SEGMENT_CONTROL_SUB_ID_ITEM_GET, "Get the item placed at specified index."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_SEGMENT_CONTROL_BASE_ID, op_desc, ELM_OBJ_SEGMENT_CONTROL_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Segment_Control_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_segment_control_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
