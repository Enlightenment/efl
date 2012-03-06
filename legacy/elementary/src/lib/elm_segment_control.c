#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Segment_Item Elm_Segment_Item;

struct _Widget_Data
{
   Evas_Object *obj;
   Evas_Object *base;
   Eina_List *seg_items;
   int item_count;
   Elm_Segment_Item *selected_item;
   int item_width;
};

struct _Elm_Segment_Item
{
   ELM_WIDGET_ITEM;
   Evas_Object *icon;
   const char *label;
   int seg_index;
};

static const char *widtype = NULL;
static void _sizing_eval(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _item_free(Elm_Segment_Item *it);
static void _segment_off(Elm_Segment_Item *it);
static void _segment_on(Elm_Segment_Item *it);
static void _position_items(Widget_Data *wd);
static void _on_move_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj
                            __UNUSED__, void *event_info __UNUSED__);
static void _mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj
                      __UNUSED__, void *event_info __UNUSED__);
static void _mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj
                        __UNUSED__, void *event_info __UNUSED__);
static void _swallow_item_objects(Elm_Segment_Item *it);
static void _update_list(Widget_Data *wd);
static Elm_Segment_Item * _item_find(const Evas_Object *obj, int index);
static Elm_Segment_Item* _item_new(Evas_Object *obj, Evas_Object *icon,
                                   const char *label);

static const char SIG_CHANGED[] = "changed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord w, h;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_coords_finger_size_adjust(wd->item_count, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(wd->item_count, &minw, 1, &minh);

   evas_object_size_hint_min_get(obj, &w, &h);
   if (w > minw) minw = w;
   if (h > minh) minh = h;
   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_del_hook(Evas_Object *obj)
{
   Elm_Segment_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_LIST_FREE(wd->seg_items, it)
     {
        _item_free(it);
        elm_widget_item_free(it);
     }

   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Segment_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   _elm_widget_mirrored_reload(obj);
   rtl = elm_widget_mirrored_get(obj);
   edje_object_mirrored_set(wd->base, rtl);

   _elm_theme_object_set(obj, wd->base, "segment_control", "base",
                         elm_widget_style_get(obj));
   edje_object_scale_set(wd->base, elm_widget_scale_get(wd->base)
                         *_elm_config->scale);

   EINA_LIST_FOREACH(wd->seg_items, l, it)
     {
        _elm_theme_object_set(obj, VIEW(it), "segment_control",
                              "item", elm_widget_style_get(obj));
        edje_object_scale_set(VIEW(it), elm_widget_scale_get(VIEW(it))
                              *_elm_config->scale);
        edje_object_mirrored_set(VIEW(it), rtl);
     }

   _update_list(wd);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;
   _update_list(wd);
}

// TODO:  Elm_widget elm_widget_focus_list_next_get  supports only Elm_widget list,
// Not the Elm_Widget_item. Focus switching with in widget not supported until
// it is supported in elm_widget
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
_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir,
                 Evas_Object **next)
{
   static int count=0;
   Widget_Data *;
   const Eina_List *items;
   void *(*list_data_get) (const Eina_List *list);

   wd = elm_widget_data_get(obj);
   if ((!wd)) return EINA_FALSE;

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     list_data_get = eina_list_data_get;
   else
     {
        items = wd->seg_items;
        list_data_get = _elm_list_data_get;
        if (!items) return EINA_FALSE;
     }
   return elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);
}
#endif

static void
_item_free(Elm_Segment_Item *it)
{
   Widget_Data *wd;

   if (!it) return;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   if (wd->selected_item == it) wd->selected_item = NULL;
   if (wd->seg_items) wd->seg_items = eina_list_remove(wd->seg_items, it);

   if (it->icon) evas_object_del(it->icon);
   if (it->label) eina_stringshare_del(it->label);
}

static void
_segment_off(Elm_Segment_Item *it)
{
   Widget_Data *wd;

   if (!it) return;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");

   if (wd->selected_item == it) wd->selected_item = NULL;
}

static void
_segment_on(Elm_Segment_Item *it)
{
   Widget_Data *wd;

   if (!it) return;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;
   if (it == wd->selected_item) return;

   if (wd->selected_item) _segment_off(wd->selected_item);

   edje_object_signal_emit(VIEW(it), "elm,state,segment,selected", "elm");

   wd->selected_item = it;
   evas_object_smart_callback_call(wd->obj, SIG_CHANGED, it);
}

static void
_position_items(Widget_Data *wd)
{
   Eina_List *l;
   Elm_Segment_Item *it;
   Eina_Bool rtl;
   int bx, by, bw, bh, pos;

   wd->item_count = eina_list_count(wd->seg_items);
   if (wd->item_count <= 0) return;

   evas_object_geometry_get(wd->base, &bx, &by, &bw, &bh);
   wd->item_width = bw / wd->item_count;
   rtl = elm_widget_mirrored_get(wd->obj);

   if (rtl)
     pos = bx + bw - wd->item_width;
   else
     pos = bx;

   EINA_LIST_FOREACH(wd->seg_items, l, it)
     {
        evas_object_move(VIEW(it), pos, by);
        evas_object_resize(VIEW(it), wd->item_width, bh);
        if (rtl)
          pos -= wd->item_width;
        else
          pos += wd->item_width;
     }
   _sizing_eval(wd->obj);
}

static void
_on_move_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(data);
   if (!wd) return;

   _position_items(wd);

}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
          void *event_info)
{
   Widget_Data *wd;
   Elm_Segment_Item *it;
   Evas_Event_Mouse_Up *ev;
   Evas_Coord x, y, w, h;

   it = data;
   if (!it) return;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   if (elm_widget_disabled_get(wd->obj)) return;

   if (it == wd->selected_item) return;

   ev = event_info;
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);

   if ((ev->canvas.x >= x) && (ev->output.x <= (x + w)) && (ev->canvas.y >= y)
       && (ev->canvas.y <= (y + h)))
     _segment_on(it);
   else
     edje_object_signal_emit(VIEW(it), "elm,state,segment,normal", "elm");
}

static void
_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   Widget_Data *wd;
   Elm_Segment_Item *it;

   it = data;
   if (!it) return;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   if (elm_widget_disabled_get(wd->obj)) return;

   if (it == wd->selected_item) return;

   edje_object_signal_emit(VIEW(it), "elm,state,segment,pressed", "elm");
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
   else
     edje_object_signal_emit(VIEW(it), "elm,state,icon,hidden", "elm");

   if (it->label)
     {
        edje_object_part_text_set(VIEW(it), "elm.text", it->label);
        edje_object_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
     }
   else
     edje_object_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(it));
}

static void
_update_list(Widget_Data *wd)
{
   Eina_List *l;
   Elm_Segment_Item *it;
   Eina_Bool rtl;
   int idx = 0;

   _position_items(wd);

   if (wd->item_count == 1)
     {
        it = eina_list_nth(wd->seg_items, 0);
        it->seg_index = 0;

        //Set the segment type
        edje_object_signal_emit(VIEW(it),
                                "elm,type,segment,single", "elm");

        //Set the segment state
        if (wd->selected_item == it)
          edje_object_signal_emit(VIEW(it),
                                  "elm,state,segment,selected", "elm");
        else
          edje_object_signal_emit(VIEW(it),
                                  "elm,state,segment,normal", "elm");

        if (elm_widget_disabled_get(wd->obj))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        return;
     }

   rtl = elm_widget_mirrored_get(wd->obj);
   EINA_LIST_FOREACH(wd->seg_items, l, it)
     {
        it->seg_index = idx;

        //Set the segment type
        if (idx == 0)
          {
             if (rtl)
               edje_object_signal_emit(VIEW(it),
                                       "elm,type,segment,right", "elm");
             else
               edje_object_signal_emit(VIEW(it),
                                       "elm,type,segment,left", "elm");
          }
        else if (idx == (wd->item_count - 1))
          {
             if (rtl)
               edje_object_signal_emit(VIEW(it),
                                       "elm,type,segment,left", "elm");
             else
               edje_object_signal_emit(VIEW(it),
                                       "elm,type,segment,right", "elm");
          }
        else
          edje_object_signal_emit(VIEW(it),
                                  "elm,type,segment,middle", "elm");

        //Set the segment state
        if (wd->selected_item == it)
          edje_object_signal_emit(VIEW(it),
                                  "elm,state,segment,selected", "elm");
        else
          edje_object_signal_emit(VIEW(it),
                                  "elm,state,segment,normal", "elm");

        if (elm_widget_disabled_get(wd->obj))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");

        _swallow_item_objects(it);
        idx++;
     }
}

static Elm_Segment_Item *
_item_find(const Evas_Object *obj, int idx)
{
   Widget_Data *wd;
   Elm_Segment_Item *it;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = eina_list_nth(wd->seg_items, idx);
   return it;
}

static void
_item_text_set_hook(Elm_Object_Item *it, const char *part, const char *label)
{
   Widget_Data *wd;
   Elm_Segment_Item *item;

   if (part && strcmp(part, "default")) return;

   item = (Elm_Segment_Item *)it;
   wd = elm_widget_item_data_get(item);
   if (!wd) return;

   eina_stringshare_replace(&item->label, label);
   if (item->label)
     edje_object_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(item));
   //label can be NULL also.
   edje_object_part_text_set(VIEW(item), "elm.text", item->label);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it, const char *part)
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

   //Remove the existing icon
   if (item->icon)
     {
        edje_object_part_unswallow(VIEW(item), item->icon);
        evas_object_del(item->icon);
        item->icon = NULL;
     }
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
_item_content_get_hook(const Elm_Object_Item *it, const char *part)
{
   if (part && strcmp(part, "icon")) return NULL;
   return ((Elm_Segment_Item *)it)->icon;
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Widget_Data *wd;
   Elm_Segment_Item *item = (Elm_Segment_Item *)it;

   wd = elm_widget_item_data_get(item);
   if (!wd) return EINA_FALSE;

   _item_free(item);
   _update_list(wd);

   return EINA_TRUE;
}

static Elm_Segment_Item*
_item_new(Evas_Object *obj, Evas_Object *icon, const char *label)
{
   Elm_Segment_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = elm_widget_item_new(obj, Elm_Segment_Item);
   if (!it) return NULL;
   elm_widget_item_data_set(it, wd);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);

   VIEW(it) = edje_object_add(evas_object_evas_get(obj));
   edje_object_scale_set(VIEW(it), elm_widget_scale_get(VIEW(it))
                         *_elm_config->scale);
   evas_object_smart_member_add(VIEW(it), obj);
   elm_widget_sub_object_add(obj, VIEW(it));
   evas_object_clip_set(VIEW(it), evas_object_clip_get(obj));

   _elm_theme_object_set(obj, VIEW(it), "segment_control", "item",
                         elm_object_style_get(obj));
   edje_object_mirrored_set(VIEW(it),
                            elm_widget_mirrored_get(WIDGET(it)));

   if (label)
     eina_stringshare_replace(&it->label, label);
   if (it->label)
     edje_object_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(it));
   edje_object_part_text_set(VIEW(it), "elm.text", label);

   it->icon = icon;
   if (it->icon) elm_widget_sub_object_add(VIEW(it), it->icon);
   evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, it);
   evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, it);
   evas_object_show(VIEW(it));

   return it;
}

EAPI Evas_Object *
elm_segment_control_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "segment_control");
   elm_widget_type_set(obj, "segment_control");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);

   // TODO: Focus switch support to Elm_widget_Item not supported yet.
#if 0
   elm_widget_focus_next_hook_set(obj, _focus_next_hook);
#endif

   wd->obj = obj;

   wd->base = edje_object_add(e);
   edje_object_scale_set(wd->base, elm_widget_scale_get(wd->base)
                         *_elm_config->scale);
   _elm_theme_object_set(obj, wd->base, "segment_control", "base", "default");
   elm_widget_resize_object_set(obj, wd->base);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _on_move_resize, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE,
                                  _on_move_resize, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   return obj;
}

EAPI Elm_Object_Item *
elm_segment_control_item_add(Evas_Object *obj, Evas_Object *icon,
                             const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_new(obj, icon, label);
   if (!it) return NULL;

   wd->seg_items = eina_list_append(wd->seg_items, it);
   _update_list(wd);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_segment_control_item_insert_at(Evas_Object *obj, Evas_Object *icon,
                                   const char *label, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it, *it_rel;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (idx < 0) idx = 0;

   it = _item_new(obj, icon, label);
   if (!it) return NULL;

   it_rel = _item_find(obj, idx);
   if (it_rel)
     wd->seg_items = eina_list_prepend_relative(wd->seg_items, it, it_rel);
   else
     wd->seg_items = eina_list_append(wd->seg_items, it);

   _update_list(wd);
   return (Elm_Object_Item *)it;
}

EAPI void
elm_segment_control_item_del(Elm_Object_Item *it)
{
   elm_object_item_del(it);
}

EAPI void
elm_segment_control_item_del_at(Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Elm_Segment_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   it = _item_find(obj, idx);
   if (!it) return;
   elm_object_item_del((Elm_Object_Item *)it);
}

EAPI const char*
elm_segment_control_item_label_get(const Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it = _item_find(obj, idx);
   if (it) return it->label;
   return NULL;
}

EAPI void
elm_segment_control_item_label_set(Elm_Object_Item* it, const char* label)
{
   _item_text_set_hook(it, NULL, label);
}

EAPI Evas_Object *
elm_segment_control_item_icon_get(const Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it = _item_find(obj, idx);
   if (it) return it->icon;
   return NULL;
}

EAPI void
elm_segment_control_item_icon_set(Elm_Object_Item *it, Evas_Object *icon)
{
   _item_content_set_hook(it, NULL, icon);
}

EAPI int
elm_segment_control_item_count_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return 0;

   return eina_list_count(wd->seg_items);
}

EAPI Evas_Object*
elm_segment_control_item_object_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return VIEW(it);
}

EAPI Elm_Object_Item*
elm_segment_control_item_selected_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return (Elm_Object_Item *) wd->selected_item;
}

EAPI void
elm_segment_control_item_selected_set(Elm_Object_Item *it, Eina_Bool selected)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Widget_Data *wd;
   Elm_Segment_Item *item = (Elm_Segment_Item *)it;

   wd = elm_widget_item_data_get(item);
   if (!wd) return;

   if (item == wd->selected_item)
     {
        //already in selected state.
        if (selected) return;

        //unselect case
        _segment_off(item);
     }
   else if (selected)
     _segment_on(item);

   return;
}

EAPI Elm_Object_Item *
elm_segment_control_item_get(const Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   return (Elm_Object_Item *) _item_find(obj, idx);
}

EAPI int
elm_segment_control_item_index_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, -1);
   return ((Elm_Segment_Item *)it)->seg_index;
}
