#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup SegmentControl SegmentControl
 *
 * SegmentControl Widget is a horizontal control made of multiple segment items,
 * each segment item functioning similar to discrete two state button. A segment
 * control groups the the items together and provides compact single button with
 * multiple equal size segments. Segment item size is determined by base widget
 * size and the number of items added.
 * Only one Segment item can be at selected state. A segment item can display
 * combination of Text and any Evas_Object like Images or other widget.
 *
 * Signals that you can add callbacks for are:
 *
 * "changed" -when the user clicks on a segment item which is not previously
 *            selected and get selected. The event_info parameter is the
 *            segment item index.
 */
typedef struct _Widget_Data Widget_Data;

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
   Elm_Widget_Item base;
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

   EINA_LIST_FREE(wd->seg_items, it) _item_free(it);

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
        _elm_theme_object_set(obj, it->base.view, "segment_control",
                              "item", elm_widget_style_get(obj));
        edje_object_scale_set(it->base.view, elm_widget_scale_get(it->base.view)
                              *_elm_config->scale);
        edje_object_mirrored_set(it->base.view, rtl);
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

   edje_object_signal_emit(it->base.view, "elm,state,segment,selected", "elm");
   return it->base.view;
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

   elm_widget_item_pre_notify_del(it);

   if (it->icon) evas_object_del(it->icon);
   if (it->label) eina_stringshare_del(it->label);

   elm_widget_item_del(it);
}

static void
_segment_off(Elm_Segment_Item *it)
{
   Widget_Data *wd;

   if (!it) return;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   edje_object_signal_emit(it->base.view, "elm,state,segment,normal", "elm");

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

   edje_object_signal_emit(it->base.view, "elm,state,segment,selected", "elm");

   wd->selected_item = it;
   evas_object_smart_callback_call(wd->obj, SIG_CHANGED, (void *)(unsigned long)it->seg_index);
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
        evas_object_move(it->base.view, pos, by);
        evas_object_resize(it->base.view, wd->item_width, bh);
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
   evas_object_geometry_get(it->base.view, &x, &y, &w, &h);

   if ((ev->canvas.x >= x) && (ev->output.x <= (x + w)) && (ev->canvas.y >= y)
       && (ev->canvas.y <= (y + h)))
     _segment_on(it);
   else
     edje_object_signal_emit(it->base.view, "elm,state,segment,normal", "elm");
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

   edje_object_signal_emit(it->base.view, "elm,state,segment,pressed", "elm");
}

static void
_swallow_item_objects(Elm_Segment_Item *it)
{
   if (!it) return;

   if (it->icon)
     {
        edje_object_part_swallow(it->base.view, "elm.swallow.icon", it->icon);
        edje_object_signal_emit(it->base.view, "elm,state,icon,visible", "elm");
     }
   else
     edje_object_signal_emit(it->base.view, "elm,state,icon,hidden", "elm");

   if (it->label)
     edje_object_signal_emit(it->base.view, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(it->base.view, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(it->base.view);
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
        edje_object_signal_emit(it->base.view,
                                "elm,type,segment,single", "elm");

        //Set the segment state
        if (wd->selected_item == it)
          edje_object_signal_emit(it->base.view,
                                  "elm,state,segment,selected", "elm");
        else
          edje_object_signal_emit(it->base.view,
                                  "elm,state,segment,normal", "elm");

        if (elm_widget_disabled_get(wd->obj))
          edje_object_signal_emit(it->base.view, "elm,state,disabled", "elm");

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
               edje_object_signal_emit(it->base.view,
                                       "elm,type,segment,right", "elm");
             else
               edje_object_signal_emit(it->base.view,
                                       "elm,type,segment,left", "elm");
          }
        else if (idx == (wd->item_count - 1))
          {
             if (rtl)
               edje_object_signal_emit(it->base.view,
                                       "elm,type,segment,left", "elm");
             else
               edje_object_signal_emit(it->base.view,
                                       "elm,type,segment,right", "elm");
          }
        else
          edje_object_signal_emit(it->base.view,
                                  "elm,type,segment,middle", "elm");

        //Set the segment state
        if (wd->selected_item == it)
          edje_object_signal_emit(it->base.view,
                                  "elm,state,segment,selected", "elm");
        else
          edje_object_signal_emit(it->base.view,
                                  "elm,state,segment,normal", "elm");

        if (elm_widget_disabled_get(wd->obj))
          edje_object_signal_emit(it->base.view, "elm,state,disabled", "elm");

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

   it->base.view = edje_object_add(evas_object_evas_get(obj));
   edje_object_scale_set(it->base.view, elm_widget_scale_get(it->base.view)
                         *_elm_config->scale);
   evas_object_smart_member_add(it->base.view, obj);
   elm_widget_sub_object_add(obj, it->base.view);
   _elm_theme_object_set(obj, it->base.view, "segment_control", "item",
                         elm_object_style_get(obj));
   edje_object_mirrored_set(it->base.view,
                            elm_widget_mirrored_get(it->base.widget));

   if (label)
     eina_stringshare_replace(&it->label, label);
   if (it->label)
     edje_object_signal_emit(it->base.view, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(it->base.view, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(it->base.view);
   edje_object_part_text_set(it->base.view, "elm.text", label);

   it->icon = icon;
   if (it->icon) elm_widget_sub_object_add(it->base.view, it->icon);
   evas_object_event_callback_add(it->base.view, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, it);
   evas_object_event_callback_add(it->base.view, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, it);
   evas_object_show(it->base.view);

   return it;
}

/**
 * Create new SegmentControl.
 * @param [in] parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup SegmentControl
 */
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

/**
 * Add new segment item to SegmentControl.
 * @param [in] obj The SegmentControl object
 * @param [in] icon Any Objects like icon, Label, layout etc
 * @param [in] label The label for added segment item.
 *             Note that, NULL is different from empty string "".
 * @return The new segment item or NULL if it cannot be created
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Segment_Item *
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

   return it;
}

/**
 * Insert a new segment item to SegmentControl.
 * @param [in] obj The SegmentControl object
 * @param [in] icon Any Objects like icon, Label, layout etc
 * @param [in] label The label for added segment item.
 *        Note that, NULL is different from empty string "".
 * @param [in] index Segment item location. Value should be between 0 and
 *        Existing total item count( @see elm_segment_control_item_count_get() )
 * @return The new segment item or NULL if it cannot be created
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Segment_Item *
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
   return it;
}

/**
 * Delete a segment item from SegmentControl
 * @param [in] obj The SegmentControl object
 * @param [in] it The segment item to be deleted
 *
 * @ingroup SegmentControl
 */
EAPI void
elm_segment_control_item_del(Elm_Segment_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   Widget_Data *wd;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   _item_free(it);
   _update_list(wd);
}

/**
 * Delete a segment item of given index from SegmentControl
 * @param [in] obj The SegmentControl object
 * @param [in] index The position at which segment item to be deleted.
 *
 * @ingroup SegmentControl
 */
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
   _item_free(it);
   _update_list(wd);
}

/**
 * Get the label of a segment item.
 * @param [in] obj The SegmentControl object
 * @param [in] index The index of the segment item
 * @return The label of the segment item
 *
 * @ingroup SegmentControl
 */
EAPI const char*
elm_segment_control_item_label_get(const Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it;

   it = _item_find(obj, idx);
   if (it) return it->label;

   return NULL;
}

/**
 * Set the label of a segment item.
 * @param [in] it The SegmentControl Item
 * @param [in] label New label text.
 *
 * @ingroup SegmentControl
 */
EAPI void
elm_segment_control_item_label_set(Elm_Segment_Item* it, const char* label)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   Widget_Data *wd;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   eina_stringshare_replace(&it->label, label);
   if (it->label)
     edje_object_signal_emit(it->base.view, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(it->base.view, "elm,state,text,hidden", "elm");
   edje_object_message_signal_process(it->base.view);
   //label can be NULL also.
   edje_object_part_text_set(it->base.view, "elm.text", it->label);
}

/**
 * Get the icon of a segment item of SegmentControl
 * @param [in] obj The SegmentControl object
 * @param [in] index The index of the segment item
 * @return The icon object.
 *
 * @ingroup SegmentControl
 */
EAPI Evas_Object *
elm_segment_control_item_icon_get(const Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it;

   it = _item_find(obj, idx);
   if (it) return it->icon;

   return NULL;
}

/**
 * Set the Icon to the segment item
 * @param [in] it The SegmentControl Item
 * @param [in] icon Objects like Layout, Icon, Label etc...
 *
 * @ingroup SegmentControl
 */
EAPI void
elm_segment_control_item_icon_set(Elm_Segment_Item *it, Evas_Object *icon)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);

   //Remove the existing icon
   if (it->icon)
     {
        edje_object_part_unswallow(it->base.view, it->icon);
        evas_object_del(it->icon);
        it->icon = NULL;
     }

   it->icon = icon;
   if (it->icon)
     {
        elm_widget_sub_object_add(it->base.view, it->icon);
        edje_object_part_swallow(it->base.view, "elm.swallow.icon", it->icon);
        edje_object_signal_emit(it->base.view, "elm,state,icon,visible", "elm");
     }
   else
     edje_object_signal_emit(it->base.view, "elm,state,icon,hidden", "elm");
}

/**
 * Get the Segment items count from SegmentControl
 * @param [in] obj The SegmentControl object
 * @return Segment items count.
 *
 * @ingroup SegmentControl
 */
EAPI int
elm_segment_control_item_count_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return 0;

   return eina_list_count(wd->seg_items);
}

/**
 * Get the base object of segment item.
 * @param [in] it The Segment item
 * @return obj The base object of the segment item.
 *
 * @ingroup SegmentControl
 */
EAPI Evas_Object*
elm_segment_control_item_object_get(const Elm_Segment_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);

   return it->base.view;
}

/**
 * Get the selected segment item in the SegmentControl
 * @param [in] obj The SegmentControl object
 * @return Selected Segment Item. NULL if none of segment item is selected.
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Segment_Item*
elm_segment_control_item_selected_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->selected_item;
}

/**
 * Select/unselect a particular segment item of SegmentControl
 * @param [in] it The Segment item that is to be selected or unselected.
 * @param [in] select Passing EINA_TRUE will select the segment item and
 *             EINA_FALSE will unselect.
 *
 * @ingroup SegmentControl
 */
EAPI void
elm_segment_control_item_selected_set(Elm_Segment_Item *it, Eina_Bool selected)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   Widget_Data *wd;

   wd = elm_widget_item_data_get(it);
   if (!wd) return;

   if (it == wd->selected_item)
     {
        //already in selected state.
        if (selected) return;

        //unselect case
        _segment_off(it);
     }
   else if (selected)
     _segment_on(it);

   return;
}

/**
 * Get the Segment Item from the specified Index.
 * @param [in] obj The Segment Control object.
 * @param [in] index The index of the segment item.
 * @return The Segment item.
 *
 * @ingroup SegmentControl
 */
EAPI Elm_Segment_Item *
elm_segment_control_item_get(const Evas_Object *obj, int idx)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Segment_Item *it;

   it = _item_find(obj, idx);

   return it;
}

/**
 * Get the index of a Segment item in the SegmentControl
 * @param [in] it The Segment Item.
 * @return Segment Item index.
 *
 * @ingroup SegmentControl
 */
EAPI int
elm_segment_control_item_index_get(const Elm_Segment_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, -1);

   return it->seg_index;
}
