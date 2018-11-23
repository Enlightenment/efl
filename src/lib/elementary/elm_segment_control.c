#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define EFL_UI_TRANSLATABLE_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_segment_control.eo.h"
#include "elm_segment_control_item.eo.h"
#include "elm_widget_segment_control.h"

#define MY_CLASS ELM_SEGMENT_CONTROL_CLASS

#define MY_CLASS_NAME "Elm_Segment_Control"
#define MY_CLASS_NAME_LEGACY "elm_segment_control"

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

EOLIAN static void
_elm_segment_control_efl_ui_translatable_translation_update(Eo *obj EINA_UNUSED, Elm_Segment_Control_Data *sd)
{
   Elm_Object_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_wdg_item_translate(it);

   efl_ui_translatable_translation_update(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_segment_control_elm_layout_sizing_eval(Eo *obj, Elm_Segment_Control_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;
   int item_count;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   item_count = eina_list_count(sd->items);

   elm_coords_finger_size_adjust(item_count, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);

   evas_object_size_hint_combined_min_get(obj, &w, &h);

   if (w > minw) minw = w;
   if (h > minh) minh = h;

   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_item_free(Elm_Segment_Control_Item_Data *it)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (sd->selected_item == it) sd->selected_item = NULL;
   if (sd->items) sd->items = eina_list_remove(sd->items, EO_OBJ(it));

   evas_object_del(it->icon);
   eina_stringshare_del(it->label);
}

static void
_position_items(Elm_Segment_Control_Data *sd)
{
   Eina_List *l;
   Eina_Bool rtl;
   int item_count;
   Elm_Object_Item *eo_item;
   int bx, by, bw, bh, pos;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   item_count = eina_list_count(sd->items);
   if (item_count <= 0) return;

   evas_object_geometry_get
     (wd->resize_obj, &bx, &by, &bw, &bh);
   sd->item_width = bw / item_count;
   rtl = efl_ui_mirrored_get(sd->obj);

   if (rtl) pos = bx + bw - sd->item_width;
   else pos = bx;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_SEGMENT_ITEM_DATA_GET(eo_item, it);
        evas_object_geometry_set(VIEW(it), pos, by, sd->item_width, bh);
        if (rtl) pos -= sd->item_width;
        else pos += sd->item_width;
     }

   elm_layout_sizing_eval(sd->obj);
}

static void
_swallow_item_objects(Elm_Segment_Control_Item_Data *it)
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
_update_list(Elm_Segment_Control_Data *sd)
{
   int idx = 0;
   Eina_List *l;
   Eina_Bool rtl;
   int item_count;
   Elm_Object_Item *eo_it;

   efl_ui_focus_composition_elements_set(sd->obj, eina_list_clone(sd->items));
   _position_items(sd);

   item_count = eina_list_count(sd->items);

   if (item_count == 1)
     {
        eo_it = eina_list_nth(sd->items, 0);
        ELM_SEGMENT_ITEM_DATA_GET(eo_it, it);

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

   rtl = efl_ui_mirrored_get(sd->obj);
   EINA_LIST_FOREACH(sd->items, l, eo_it)
     {
        ELM_SEGMENT_ITEM_DATA_GET(eo_it, it);
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

        if (elm_widget_disabled_get(sd->obj)
            || elm_object_item_disabled_get(eo_it))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        idx++;
     }
}

EOLIAN static Efl_Ui_Theme_Apply_Result
_elm_segment_control_efl_ui_widget_theme_apply(Eo *obj, Elm_Segment_Control_Data *sd)
{
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Object_Item *eo_item;

   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   rtl = efl_ui_mirrored_get(obj);

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_SEGMENT_ITEM_DATA_GET(eo_item, it);
        elm_widget_theme_object_set
          (obj, VIEW(it), "segment_control", "item",
          elm_widget_style_get(obj));
        edje_object_scale_set(VIEW(it), efl_gfx_entity_scale_get(WIDGET(it)) *
                              elm_config_scale_get());
        edje_object_mirrored_set(VIEW(it), rtl);
     }

   _update_list(sd);

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_segment_control_efl_ui_widget_on_disabled_update(Eo *obj, Elm_Segment_Control_Data *sd, Eina_Bool disabled)
{
   if (!efl_ui_widget_on_disabled_update(efl_super(obj, MY_CLASS), disabled))
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
   Elm_Object_Item *eo_item = eina_list_data_get(list);

   if (eo_item) return NULL;

   ELM_SEGMENT_ITEM_DATA_GET(eo_item, it);
   edje_object_signal_emit(VIEW(it), "elm,state,segment,selected", "elm");

   return VIEW(it);
}

EOLIAN static Eina_Bool
_elm_segment_control_elm_widget_focus_next(Eo *obj, Elm_Segment_Control_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   static int count = 0;
   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   /* Focus chain */
   if ((items = efl_ui_widget_focus_custom_chain_get(obj)))
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
_segment_off(Elm_Segment_Control_Item_Data *it)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");

   if (sd->selected_item == it) sd->selected_item = NULL;
}

static void
_segment_on(Elm_Segment_Control_Item_Data *it)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (it == sd->selected_item) return;

   if (elm_object_item_disabled_get(EO_OBJ(it))) return;

   if (sd->selected_item) _segment_off(sd->selected_item);

   edje_object_signal_emit(VIEW(it), "elm,state,segment,selected", "elm");

   sd->selected_item = it;
   efl_event_callback_legacy_call(sd->obj, ELM_SEGMENT_CONTROL_EVENT_CHANGED, EO_OBJ(it));
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
   Elm_Segment_Control_Item_Data *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Evas_Coord x, y, w, h;

   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (elm_widget_disabled_get(sd->obj)) return;

   if (it == sd->selected_item) return;

   ev = event_info;
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);

   if ((ev->canvas.x >= x) && (ev->canvas.x <= (x + w)) && (ev->canvas.y >= y)
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
   Elm_Segment_Control_Item_Data *it = data;
   Evas_Event_Mouse_Down *ev = event_info;

   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;
   if (elm_widget_disabled_get(sd->obj)) return;

   if (it == sd->selected_item) return;

   edje_object_signal_emit(VIEW(it), "elm,state,segment,pressed", "elm");
}

static Elm_Segment_Control_Item_Data *
_item_find(const Evas_Object *obj,
           int idx)
{
   Elm_Object_Item *eo_it;

   ELM_SEGMENT_CONTROL_DATA_GET(obj, sd);

   eo_it = eina_list_nth(sd->items, idx);
   ELM_SEGMENT_ITEM_DATA_GET(eo_it, it);
   return it;
}

EOLIAN static void
_elm_segment_control_item_elm_widget_item_part_text_set(Eo *eo_item EINA_UNUSED,
                                Elm_Segment_Control_Item_Data *item,
                                const char *part,
                                const char *label)
{
   char buf[1024];

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, "elm.text")))
     {
        eina_stringshare_replace(&item->label, label);
        if (label)
          edje_object_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
        else
          edje_object_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");
        edje_object_part_text_escaped_set(VIEW(item), "elm.text", label);
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
        edje_object_part_text_escaped_set(VIEW(item), part, label);
     }

   edje_object_message_signal_process(VIEW(item));
}

EOLIAN static const char *
_elm_segment_control_item_elm_widget_item_part_text_get(const Eo *eo_item EINA_UNUSED,
                                                Elm_Segment_Control_Item_Data *it,
                                                const char *part)
{
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return edje_object_part_text_get(VIEW(it), buf);
}

EOLIAN static void
_elm_segment_control_item_elm_widget_item_part_content_set(Eo *eo_item EINA_UNUSED,
                                                   Elm_Segment_Control_Item_Data *item,
                                                   const char *part,
                                                   Evas_Object *content)
{
   char buf[1024];

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
             edje_object_part_swallow(VIEW(item), part, content);
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

EOLIAN static Evas_Object *
_elm_segment_control_item_elm_widget_item_part_content_get(const Eo *eo_item EINA_UNUSED,
                                                   Elm_Segment_Control_Item_Data *item,
                                                   const char *part)
{
   if (part && !strcmp(part, "icon"))
     return item->icon;
   else
     return edje_object_part_swallow_get(VIEW(item), part);
}

EOLIAN static void
_elm_segment_control_item_efl_object_destructor(Eo *eo_item, Elm_Segment_Control_Item_Data *item)
{
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(item), sd);

   _item_free(item);
   _update_list(sd);

   efl_destructor(efl_super(eo_item, ELM_SEGMENT_CONTROL_ITEM_CLASS));
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   const char *txt = NULL;
   Elm_Segment_Control_Item_Data *it = (Elm_Segment_Control_Item_Data *)data;
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, NULL);

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Segment_Control_Item_Data *it = (Elm_Segment_Control_Item_Data *)data;
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

static Evas_Object*
_elm_segment_control_item_elm_widget_item_access_register(Eo *eo_it EINA_UNUSED,
                                                  Elm_Segment_Control_Item_Data *it)
{
   Elm_Access_Info *ai;
   Evas_Object *ret;

   ret = elm_wdg_item_access_register(efl_super(eo_it, ELM_SEGMENT_CONTROL_ITEM_CLASS));

   ai = _elm_access_info_get(it->base->access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Segment Control Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);

   return ret;
}

static Elm_Object_Item *
_item_new(Evas_Object *obj,
          Evas_Object *icon,
          const char *label)
{
   Eo *eo_item;

   eo_item = efl_add(ELM_SEGMENT_CONTROL_ITEM_CLASS, obj);
   if (!eo_item) return NULL;

   ELM_SEGMENT_ITEM_DATA_GET(eo_item, it);

   if (label) eina_stringshare_replace(&it->label, label);
   if (it->label)
     edje_object_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(it));
   edje_object_part_text_escaped_set(VIEW(it), "elm.text", label);

   it->icon = icon;
   if (it->icon) elm_widget_sub_object_add(obj, it->icon);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, it);
   evas_object_show(VIEW(it));

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_wdg_item_access_register(eo_item);

   return eo_item;
}

EOLIAN static Eo *
_elm_segment_control_item_efl_object_constructor(Eo *obj, Elm_Segment_Control_Item_Data *it)
{
   obj = efl_constructor(efl_super(obj, ELM_SEGMENT_CONTROL_ITEM_CLASS));
   it->base = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);

   Evas_Object *parent;
   parent = efl_parent_get(obj);

   VIEW_SET(it, edje_object_add(evas_object_evas_get(parent)));
   _efl_ui_focus_event_redirector(VIEW(it), obj);
   edje_object_scale_set(VIEW(it),efl_gfx_entity_scale_get(WIDGET(it)) *
                         elm_config_scale_get());
   evas_object_smart_member_add(VIEW(it), parent);

   elm_widget_sub_object_add(parent, VIEW(it));
   elm_widget_theme_object_set
     (parent, VIEW(it), "segment_control", "item", elm_object_style_get(parent));
   edje_object_mirrored_set(VIEW(it), efl_ui_mirrored_get(WIDGET(it)));

   return obj;
}

EOLIAN static void
_elm_segment_control_efl_canvas_group_group_add(Eo *obj, Elm_Segment_Control_Data *sd)
{
   sd->obj = obj;
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
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

EOLIAN static void
_elm_segment_control_efl_canvas_group_group_del(Eo *obj, Elm_Segment_Control_Data *sd)
{
   Elm_Object_Item *eo_it;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_RESIZE,
                                       _on_move_resize, obj);
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_MOVE,
                                       _on_move_resize, obj);
   EINA_LIST_FREE(sd->items, eo_it)
     {
        efl_del(eo_it);
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

static Eina_Bool _elm_segment_control_smart_focus_next_enable = EINA_FALSE;

static void
_access_obj_process(Elm_Segment_Control_Data *sd, Eina_Bool is_access)
{
   Eina_List *l;
   Eo *eo_item;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        if (is_access) elm_wdg_item_access_register(eo_item);
        else
          elm_wdg_item_access_unregister(eo_item);
     }
}

EOLIAN static void
_elm_segment_control_efl_ui_widget_on_access_update(Eo *obj EINA_UNUSED, Elm_Segment_Control_Data *sd, Eina_Bool acs)
{
   _elm_segment_control_smart_focus_next_enable = acs;
   _access_obj_process(sd, _elm_segment_control_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_segment_control_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_segment_control_efl_object_constructor(Eo *obj, Elm_Segment_Control_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_LIST);

   return obj;
}

EOLIAN static Elm_Object_Item*
_elm_segment_control_item_add(Eo *obj, Elm_Segment_Control_Data *sd, Evas_Object *icon, const char *label)
{
   Elm_Object_Item *eo_item;

   eo_item = _item_new(obj, icon, label);
   if (!eo_item) return NULL;

   sd->items = eina_list_append(sd->items, eo_item);
   _update_list(sd);

   return eo_item;
}

EOLIAN static Elm_Object_Item*
_elm_segment_control_item_insert_at(Eo *obj, Elm_Segment_Control_Data *sd, Evas_Object *icon, const char *label, int idx)
{
   Elm_Object_Item *eo_item;
   Elm_Segment_Control_Item_Data *it_rel;


   if (idx < 0) idx = 0;

   eo_item = _item_new(obj, icon, label);
   if (!eo_item) return NULL;

   it_rel = _item_find(obj, idx);
   if (it_rel) sd->items = eina_list_prepend_relative(sd->items, eo_item, EO_OBJ(it_rel));
   else sd->items = eina_list_append(sd->items, eo_item);

   _update_list(sd);

   return eo_item;
}

EOLIAN static void
_elm_segment_control_item_del_at(Eo *obj, Elm_Segment_Control_Data *_pd EINA_UNUSED, int idx)
{
   Elm_Segment_Control_Item_Data *it;

   it = _item_find(obj, idx);
   if (!it) return;

   efl_del(EO_OBJ(it));
}

EOLIAN static const char*
_elm_segment_control_item_label_get(const Eo *obj, Elm_Segment_Control_Data *_pd EINA_UNUSED, int idx)
{
   Elm_Segment_Control_Item_Data *it;

   it = _item_find(obj, idx);
   if (it) return it->label;

   return NULL;
}

EOLIAN static Evas_Object*
_elm_segment_control_item_icon_get(const Eo *obj, Elm_Segment_Control_Data *_pd EINA_UNUSED, int idx)
{
   Elm_Segment_Control_Item_Data *it = _item_find(obj, idx);
   if (it) return it->icon;
   return NULL;
}

EOLIAN static int
_elm_segment_control_item_count_get(const Eo *obj EINA_UNUSED, Elm_Segment_Control_Data *sd)
{
   return eina_list_count(sd->items);
}

EOLIAN static Evas_Object *
_elm_segment_control_item_object_get(const Eo *eo_it EINA_UNUSED, Elm_Segment_Control_Item_Data *it)
{
   return VIEW(it);
}

EOLIAN static Elm_Object_Item*
_elm_segment_control_item_selected_get(const Eo *obj EINA_UNUSED, Elm_Segment_Control_Data *sd)
{
   return EO_OBJ(sd->selected_item);
}

EOLIAN static void
_elm_segment_control_item_selected_set(Eo *eo_item EINA_UNUSED,
                               Elm_Segment_Control_Item_Data *item,
                               Eina_Bool selected)
{
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(item);
   ELM_SEGMENT_CONTROL_DATA_GET(WIDGET(item), sd);

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

EOLIAN static Elm_Object_Item*
_elm_segment_control_item_get(const Eo *obj, Elm_Segment_Control_Data *_pd EINA_UNUSED, int idx)
{
   Elm_Segment_Control_Item_Data *it = _item_find(obj, idx);
   return EO_OBJ(it);
}

EOLIAN static int
_elm_segment_control_item_index_get(const Eo *eo_it EINA_UNUSED, Elm_Segment_Control_Item_Data *it)
{
   ELM_SEGMENT_CONTROL_ITEM_CHECK_OR_RETURN(it, -1);

   return it->seg_index;
}

EOLIAN static void
_elm_segment_control_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      _elm_segment_control_smart_focus_next_enable = EINA_TRUE;
}

EOLIAN static Eina_Rect
_elm_segment_control_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Elm_Segment_Control_Item_Data *pd)
{
   return efl_gfx_entity_geometry_get(VIEW(pd));
}

EOLIAN static Efl_Ui_Focus_Object*
_elm_segment_control_item_efl_ui_focus_object_focus_parent_get(const Eo *obj EINA_UNUSED, Elm_Segment_Control_Item_Data *pd)
{
   return WIDGET(pd);
}

/* Internal EO APIs and hidden overrides */

#define ELM_SEGMENT_CONTROL_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_segment_control), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_segment_control)

#include "elm_segment_control_item.eo.c"
#include "elm_segment_control.eo.c"
