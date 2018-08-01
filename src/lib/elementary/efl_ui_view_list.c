#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_SELECTION_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_SCROLLBAR_BETA
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED
#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Elementary.h>
#include "efl_ui_view_list_private.h"
#include "efl_ui_view_list_precise_layouter.eo.h"

#include <assert.h>

#define MY_CLASS EFL_UI_VIEW_LIST_CLASS
#define MY_CLASS_NAME "Efl.Ui.View.List"

#define MY_PAN_CLASS EFL_UI_VIEW_LIST_PAN_CLASS

#define SIG_CHILD_ADDED "child,added"
#define SIG_CHILD_REMOVED "child,removed"
#define SELECTED_PROP "selected"

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};

void _efl_ui_view_list_custom_layout(Efl_Ui_View_List *);
void _efl_ui_view_list_item_select_set(Efl_Ui_View_List_LayoutItem*, Eina_Bool);
static void _layout(Efl_Ui_View_List_Data* pd);

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"select", _key_action_select},
   {"escape", _key_action_escape},
   {NULL, NULL}
};

EOLIAN static void
_efl_ui_view_list_pan_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Pan_Data *psd)
{
   evas_object_smart_changed(psd->wobj);
}


EOLIAN static void
_efl_ui_view_list_pan_efl_ui_pan_pan_position_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Pan_Data *psd, Eina_Position2D pos)
{
   if ((pos.x == psd->gmt.x) && (pos.y == psd->gmt.y)) return;

   psd->gmt.x = pos.x;
   psd->gmt.y = pos.y;

   efl_event_callback_call(obj, EFL_UI_PAN_EVENT_POSITION_CHANGED, NULL);
   evas_object_smart_changed(psd->wobj);
}

EOLIAN static Eina_Position2D
_efl_ui_view_list_pan_efl_ui_pan_pan_position_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Pan_Data *psd)
{
   return psd->gmt.pos;
}

EOLIAN static Eina_Position2D
_efl_ui_view_list_pan_efl_ui_pan_pan_position_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Pan_Data *psd)
{
   EFL_UI_VIEW_LIST_DATA_GET(psd->wobj, pd);
   Eina_Rect vgmt = {};
   Eina_Size2D min = {};

   vgmt = efl_ui_scrollable_viewport_geometry_get(pd->scrl_mgr);
   min = efl_ui_view_list_model_min_size_get(psd->wobj);

   min.w = min.w - vgmt.w;
   if (min.w < 0) min.w = 0;
   min.h = min.h - vgmt.h;
   if (min.h < 0) min.h = 0;

   return EINA_POSITION2D(min.w, min.h);
}

EOLIAN static Eina_Position2D
_efl_ui_view_list_pan_efl_ui_pan_pan_position_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Pan_Data *psd EINA_UNUSED)
{
   return EINA_POSITION2D(0, 0);
}

EOLIAN static Eina_Size2D
_efl_ui_view_list_pan_efl_ui_pan_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Pan_Data *psd)
{
   Eina_Size2D min = {};
   min = efl_ui_view_list_model_min_size_get(psd->wobj);

   return min;
}

EOLIAN static void
_efl_ui_view_list_pan_efl_object_destructor(Eo *obj, Efl_Ui_View_List_Pan_Data *psd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_PAN_CLASS));
}

#include "efl_ui_view_list_pan.eo.c"

EOLIAN static void
_efl_ui_view_list_efl_ui_scrollable_interactive_content_pos_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *psd, Eina_Position2D pos)
{
   efl_ui_scrollable_content_pos_set(psd->scrl_mgr, pos);
}

EOLIAN static Eina_Position2D
_efl_ui_view_list_efl_ui_scrollable_interactive_content_pos_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *psd)
{
   Eina_Position2D pos = efl_ui_scrollable_content_pos_get(psd->scrl_mgr);
   return pos;
}

EOLIAN static Eina_Size2D
_efl_ui_view_list_efl_ui_scrollable_interactive_content_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *psd)
{
   Eina_Size2D size = efl_ui_scrollable_content_size_get(psd->scrl_mgr);
   return size;
}

EOLIAN static Eina_Rect
_efl_ui_view_list_efl_ui_scrollable_interactive_viewport_geometry_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *psd)
{
   Eina_Rect gmt = efl_ui_scrollable_viewport_geometry_get(psd->scrl_mgr);
   return gmt;
}

static Eina_Bool
_efl_model_properties_has(Efl_Model *model, Eina_Stringshare *propfind)
{
   const Eina_Array *properties;
   Eina_Array_Iterator iter_prop;
   Eina_Stringshare *property;
   Eina_Bool ret = EINA_FALSE;
   unsigned i = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(model, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(propfind, EINA_FALSE);

   properties = efl_model_properties_get(model);

   EINA_ARRAY_ITER_NEXT(properties, i, property, iter_prop)
     {
        if (property == propfind)
          {
             ret = EINA_TRUE;
             break;
          }
     }
   return ret;
}

static void
_list_element_focused(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Rect geom;
   Eina_Position2D pos;
   Efl_Ui_Focus_Object *focused = efl_ui_focus_manager_focus_get(ev->object);

   if (!focused) return;

   EFL_UI_VIEW_LIST_DATA_GET(ev->object, pd);
   geom = efl_ui_focus_object_focus_geometry_get(focused);
   pos = efl_ui_scrollable_content_pos_get(pd->scrl_mgr);

   geom.x += pos.x;
   geom.y += pos.y;
   efl_ui_scrollable_scroll(pd->scrl_mgr, geom, EINA_TRUE);
}

static void
_on_item_mouse_up(void *data, Evas *evas EINA_UNUSED, Evas_Object *o EINA_UNUSED, void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Efl_Ui_View_List_LayoutItem *item = data;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   _efl_ui_view_list_item_select_set(item, EINA_TRUE);
}

EOLIAN static void
_efl_ui_view_list_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd, Elm_Object_Select_Mode mode)
{
   if (pd->select_mode == mode)
     return;

   pd->select_mode = mode;
}

EOLIAN static Elm_Object_Select_Mode
_efl_ui_view_list_select_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return pd->select_mode;
}

EOLIAN static void
_efl_ui_view_list_default_style_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd, Eina_Stringshare *style)
{
   eina_stringshare_replace(&pd->style, style);
}

EOLIAN static Eina_Stringshare *
_efl_ui_view_list_default_style_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return pd->style;
}

EOLIAN static void
_efl_ui_view_list_homogeneous_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd, Eina_Bool homogeneous)
{
   pd->homogeneous = homogeneous;
}

EOLIAN static Eina_Bool
_efl_ui_view_list_homogeneous_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return pd->homogeneous;
}

EOLIAN static void
_efl_ui_view_list_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_View_List_Data *pd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   evas_object_smart_changed(pd->obj);
}

EOLIAN static void
_efl_ui_view_list_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_View_List_Data *pd, Eina_Size2D size)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, size.w, size.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), size);

   evas_object_smart_changed(pd->obj);
}

EOLIAN static void
_efl_ui_view_list_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   _layout(pd);
}

EOLIAN static void
_efl_ui_view_list_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_View_List_Data *pd EINA_UNUSED, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);
}

//Scrollable Implement
static void
_efl_ui_view_list_bar_read_and_update(Eo *obj)
{
   EFL_UI_VIEW_LIST_DATA_GET(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   double vx, vy;

   edje_object_part_drag_value_get
      (wd->resize_obj, "efl.dragable.vbar", NULL, &vy);
   edje_object_part_drag_value_get
      (wd->resize_obj, "efl.dragable.hbar", &vx, NULL);

   efl_ui_scrollbar_bar_position_set(pd->scrl_mgr, vx, vy);

  efl_canvas_group_change(pd->pan_obj);
}

static void
_efl_ui_view_list_reload_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   EFL_UI_VIEW_LIST_DATA_GET(data, pd);

   efl_ui_scrollbar_bar_visibility_update(pd->scrl_mgr);
}

static void
_efl_ui_view_list_vbar_drag_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   _efl_ui_view_list_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_view_list_vbar_press_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_view_list_vbar_unpress_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_view_list_edje_drag_start_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   EFL_UI_VIEW_LIST_DATA_GET(data, pd);

   _efl_ui_view_list_bar_read_and_update(data);

   pd->scrl_freeze = efl_ui_scrollable_scroll_freeze_get(pd->scrl_mgr);
   efl_ui_scrollable_scroll_freeze_set(pd->scrl_mgr, EINA_TRUE);
   efl_event_callback_call(data, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
}

static void
_efl_ui_view_list_edje_drag_stop_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   EFL_UI_VIEW_LIST_DATA_GET(data, pd);

   _efl_ui_view_list_bar_read_and_update(data);

   efl_ui_scrollable_scroll_freeze_set(pd->scrl_mgr, pd->scrl_freeze);
   efl_event_callback_call(data, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_efl_ui_view_list_edje_drag_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   _efl_ui_view_list_bar_read_and_update(data);
}

static void
_efl_ui_view_list_hbar_drag_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   _efl_ui_view_list_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_view_list_hbar_press_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_view_list_hbar_unpress_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_view_list_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_VIEW_LIST_DATA_GET(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double width = 0.0, height = 0.0;

   efl_ui_scrollbar_bar_size_get(pd->scrl_mgr, &width, &height);

   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.hbar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.vbar", 1.0, height);
}

static void
_efl_ui_view_list_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_VIEW_LIST_DATA_GET(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0;

   efl_ui_scrollbar_bar_position_get(pd->scrl_mgr, &posx, &posy);

   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.hbar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.vbar", 0.0, posy);
}

static void
_efl_ui_view_list_bar_show_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,vbar", "efl");
}

static void
_efl_ui_view_list_bar_hide_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,hide,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,hide,vbar", "efl");
}

EOLIAN static Eina_Bool
_efl_ui_view_list_efl_layout_signal_signal_callback_add(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *sd EINA_UNUSED, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ok = efl_layout_signal_callback_add(wd->resize_obj, emission, source, func_cb, data);

   return ok;
}

EOLIAN static Eina_Bool
_efl_ui_view_list_efl_layout_signal_signal_callback_del(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *sd EINA_UNUSED, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Eina_Bool ok;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   ok = efl_layout_signal_callback_del(wd->resize_obj, emission, source, func_cb, data);

   return ok;
}

static void
_efl_ui_view_list_edje_object_attach(Eo *obj)
{
   efl_layout_signal_callback_add
     (obj, "reload", "efl", _efl_ui_view_list_reload_cb, obj);
  //Vertical bar
   efl_layout_signal_callback_add
     (obj, "drag", "efl.dragable.vbar",
     _efl_ui_view_list_vbar_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,set", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,start", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_start_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,stop", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_stop_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,step", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,page", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "efl,vbar,press", "efl",
     _efl_ui_view_list_vbar_press_cb, obj);
   efl_layout_signal_callback_add
     (obj, "efl,vbar,unpress", "efl",
     _efl_ui_view_list_vbar_unpress_cb, obj);

  //Horizontal bar
   efl_layout_signal_callback_add
     (obj, "drag", "efl.dragable.hbar",
     _efl_ui_view_list_hbar_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,set", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,start", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_start_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,stop", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_stop_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,step", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,page", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "efl,hbar,press", "efl",
     _efl_ui_view_list_hbar_press_cb, obj);
   efl_layout_signal_callback_add
     (obj, "efl,hbar,unpress", "efl",
     _efl_ui_view_list_hbar_unpress_cb, obj);
}

static void
_efl_ui_view_list_edje_object_detach(Evas_Object *obj)
{
   efl_layout_signal_callback_del
     (obj, "reload", "efl", _efl_ui_view_list_reload_cb, obj);
  //Vertical bar
   efl_layout_signal_callback_del
     (obj, "drag", "efl.dragable.vbar", _efl_ui_view_list_vbar_drag_cb,
     obj);
   efl_layout_signal_callback_del
     (obj, "drag,set", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,start", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_start_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,stop", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_stop_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,step", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,page", "efl.dragable.vbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "efl,vbar,press", "efl",
     _efl_ui_view_list_vbar_press_cb, obj);
   efl_layout_signal_callback_del
     (obj, "efl,vbar,unpress", "efl",
   _efl_ui_view_list_vbar_unpress_cb, obj);

   //Horizontal bar
   efl_layout_signal_callback_del
       (obj, "drag", "efl.dragable.hbar",
       _efl_ui_view_list_hbar_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,set", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,start", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_start_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,stop", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_stop_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,step", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,page", "efl.dragable.hbar",
     _efl_ui_view_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "efl,hbar,press", "efl",
     _efl_ui_view_list_hbar_press_cb, obj);
   efl_layout_signal_callback_del
     (obj, "efl,hbar,unpress", "efl",
     _efl_ui_view_list_hbar_unpress_cb, obj);
}

EOLIAN static void
_efl_ui_view_list_efl_canvas_group_group_add(Eo *obj, Efl_Ui_View_List_Data *pd)
{
   Efl_Ui_View_List_Pan_Data *pan_data;
   Eina_Size2D min = {};
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;
   Evas_Object *o;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_layout_theme_set(obj, "view_list", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   pd->scrl_mgr = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj,
                            efl_ui_mirrored_set(efl_added, efl_ui_mirrored_get(obj)));
   pd->pan_obj = efl_add(MY_PAN_CLASS, obj);
   pan_data = efl_data_scope_get(pd->pan_obj, MY_PAN_CLASS);
   pan_data->wobj = obj;

   efl_ui_scroll_manager_pan_set(pd->scrl_mgr, pd->pan_obj);
   efl_ui_scrollable_bounce_enabled_set(pd->scrl_mgr, bounce, bounce);

   edje_object_part_swallow(wd->resize_obj, "efl.content", pd->pan_obj);
   edje_object_freeze(wd->resize_obj);
   o = (Evas_Object *)edje_object_part_object_get(wd->resize_obj, "efl.dragable.vbar");
   edje_object_thaw(wd->resize_obj);
   efl_gfx_stack_raise((Eo *)o);

   efl_gfx_entity_visible_set(pd->pan_obj, EINA_TRUE);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_DISABLED);

   edje_object_size_min_calc(wd->resize_obj, &min.w, &min.h);
   efl_gfx_size_hint_restricted_min_set(obj, min);

   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                         _efl_ui_view_list_bar_size_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                         _efl_ui_view_list_bar_pos_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                         _efl_ui_view_list_bar_show_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_view_list_bar_hide_cb, obj);

   _efl_ui_view_list_edje_object_attach(obj);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_view_list_efl_canvas_group_group_del(Eo *obj, Efl_Ui_View_List_Data *pd)
{
   ELM_SAFE_FREE(pd->pan_obj, evas_object_del);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_view_list_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   if (!pd->manager)
     pd->manager = efl_add(EFL_UI_FOCUS_MANAGER_CALC_CLASS, obj,
                          efl_ui_focus_manager_root_set(efl_added, root));

   return pd->manager;
}

EOLIAN static Eo *
_efl_ui_view_list_efl_object_finalize(Eo *obj, Efl_Ui_View_List_Data *pd)
{

   if (!pd->factory)
     pd->factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, obj);

   if(!pd->relayout)
     {
        pd->relayout = efl_add(EFL_UI_VIEW_LIST_PRECISE_LAYOUTER_CLASS, obj);
        if (pd->model)
          efl_ui_view_list_relayout_model_set(pd->relayout, pd->model);
     }
   return obj;
}

EOLIAN static Eo *
_efl_ui_view_list_efl_object_constructor(Eo *obj, Efl_Ui_View_List_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->obj = obj;
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_LIST);

   pd->segarray = efl_add(EFL_UI_VIEW_LIST_SEGARRAY_CLASS, obj, efl_ui_view_list_segarray_setup(efl_added, 32));

   efl_event_callback_add(obj, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED, _list_element_focused, NULL);

   efl_ui_focus_composition_custom_manager_set(obj, obj);
   efl_ui_focus_composition_logical_mode_set(obj, EINA_TRUE);

   pd->style = eina_stringshare_add(elm_widget_style_get(obj));

   pd->factory = NULL;
   pd->min.w = 0;
   pd->min.h = 0;

   return obj;
}

EOLIAN static void
_efl_ui_view_list_efl_object_destructor(Eo *obj, Efl_Ui_View_List_Data *pd)
{
   efl_ui_view_list_relayout_model_set(pd->relayout, NULL);

   efl_unref(pd->model);
   eina_stringshare_del(pd->style);

   _efl_ui_view_list_edje_object_detach(obj);

   ELM_SAFE_FREE(pd->pan_obj, evas_object_del);
   efl_canvas_group_del(efl_super(obj, MY_CLASS));

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_view_list_layout_factory_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd, Efl_Ui_Factory *factory)
{
   if (pd->factory)
     efl_unref(pd->factory);

   pd->factory = factory;
   efl_ref(pd->factory);
}

EOLIAN static void
_efl_ui_view_list_efl_ui_view_model_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd, Efl_Model *model)
{
   if (pd->model == model)
     return;

   if (pd->model)
     {
        if (pd->relayout)
          efl_ui_view_list_relayout_model_set(pd->relayout, NULL);
        efl_ui_view_list_segarray_flush(pd->segarray);
     }

   efl_replace(&pd->model, model);

   if (pd->model && pd->relayout)
     efl_ui_view_list_relayout_model_set(pd->relayout, pd->model);

   evas_object_smart_changed(pd->obj);
}

EOLIAN static Efl_Model *
_efl_ui_view_list_efl_ui_view_model_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return pd->model;
}

EOLIAN int
_efl_ui_view_list_efl_access_selection_selected_children_count_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return eina_list_count(pd->selected_items);
}

EOLIAN Eo*
_efl_ui_view_list_efl_access_selection_selected_child_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd, int child_index)
{
   if(child_index <  (int) eina_list_count(pd->selected_items))
     {
        Efl_Ui_View_List_LayoutItem* items = eina_list_nth(pd->selected_items, child_index);
        return items[child_index].layout;
     }
   else
     return NULL;
}

EOLIAN Eina_Bool
_efl_ui_view_list_efl_access_selection_child_select(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_view_list_efl_access_selection_selected_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_view_list_efl_access_selection_is_child_selected(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_view_list_efl_access_selection_all_children_select(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_view_list_efl_access_selection_access_selection_clear(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_view_list_efl_access_selection_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED, int child_index EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_key_action_move(Evas_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_key_action_select(Evas_Object *obj EINA_UNUSED, const char *params EINA_UNUSED)
{
   return EINA_FALSE;
}

static Eina_Bool
_key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_focus_manager_reset_history(obj);
   return EINA_TRUE;
}

void
_efl_ui_view_list_item_select_set(Efl_Ui_View_List_LayoutItem *item, Eina_Bool selected)
{
   Eina_Stringshare *sprop;
   assert(item != NULL);
   assert(item->children != NULL);

   selected = !!selected;

   sprop = eina_stringshare_add(SELECTED_PROP);

   if (_efl_model_properties_has(item->children, sprop))
     {
        Eina_Value v;
        eina_value_setup(&v, EINA_VALUE_TYPE_UCHAR);
        eina_value_set(&v, selected);
        efl_model_property_set(item->children, sprop, &v);
        eina_value_flush(&v);
     }
   eina_stringshare_del(sprop);
}

static void
_efl_ui_view_list_relayout_set(Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED, Efl_Ui_View_List_Relayout *object)
{
   efl_replace(&pd->relayout, object);
   if (pd->model && pd->relayout)
     efl_ui_view_list_relayout_model_set(pd->relayout, pd->model);
}

static Efl_Ui_View_List_Relayout *
_efl_ui_view_list_relayout_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd EINA_UNUSED)
{
   return pd->relayout;
}

static void
_layout(Efl_Ui_View_List_Data *pd)
{
   if (!pd->model)
     return;

   efl_ui_view_list_relayout_layout_do(pd->relayout, pd->obj, pd->segarray_first, pd->segarray);
}

static Eina_Value
_children_slice_then(void * data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_View_List_Data *pd = data;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   efl_ui_view_list_segarray_insert_value(pd->segarray, pd->slice.start, v);

   pd->segarray_first = pd->slice.start;
   pd->slice.start = pd->slice.count = 0;
   pd->slice.future = NULL;

   efl_ui_view_list_relayout_layout_do(pd->relayout, pd->obj, pd->segarray_first, pd->segarray);
 on_error:
   return v;
}

/* EFL UI LIST MODEL INTERFACE */
EOLIAN static Eina_Size2D
_efl_ui_view_list_efl_ui_view_list_model_min_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return pd->min;
}

EOLIAN static void
_efl_ui_view_list_efl_ui_view_list_model_min_size_set(Eo *obj, Efl_Ui_View_List_Data *pd, Eina_Size2D min)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   pd->min.w = min.w;
   pd->min.h = min.h;

   evas_object_size_hint_min_set(wd->resize_obj, pd->min.w, pd->min.h);
   efl_event_callback_call(pd->pan_obj, EFL_UI_PAN_EVENT_CONTENT_CHANGED, NULL);
}

EOLIAN static void
_efl_ui_view_list_efl_ui_focus_composition_prepare(Eo *obj, Efl_Ui_View_List_Data *pd)
{
   Eina_List *order = efl_ui_view_list_relayout_elements_get(pd->relayout);
   efl_ui_focus_composition_elements_set(obj, order);
}

EOLIAN Eina_List*
_efl_ui_view_list_efl_access_object_access_children_get(const Eo *obj, Efl_Ui_View_List_Data *pd)
{
   Eina_List *ret = NULL, *ret2 = NULL;

   ret = efl_ui_view_list_relayout_elements_get(pd->relayout);
   ret2 = efl_access_object_access_children_get(efl_super(obj, EFL_UI_VIEW_LIST_CLASS));

   return eina_list_merge(ret, ret2);
}

EOLIAN static Eina_Bool
_efl_ui_view_list_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_View_List_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect EINA_UNUSED)
{
   return efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, obj);
}

EOLIAN static Efl_Ui_View_List_LayoutItem *
_efl_ui_view_list_efl_ui_view_list_model_realize(Eo *obj, Efl_Ui_View_List_Data *pd, Efl_Ui_View_List_LayoutItem *item)
{
   Efl_Ui_View_List_Item_Event evt;
   EINA_SAFETY_ON_NULL_RETURN_VAL(item->children, item);

   item->layout = efl_ui_factory_create(pd->factory, item->children, obj);
   evas_object_smart_member_add(item->layout, pd->pan_obj);
   evas_object_event_callback_add(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item);

   if (_elm_config->atspi_mode)
     {
        efl_access_added(item->layout);
        efl_access_children_changed_added_signal_emit(obj, item->layout);
     }

   evt.child = item->children;
   evt.layout = item->layout;
   evt.index = efl_ui_view_list_item_index_get(item);
   efl_event_callback_call(obj, EFL_UI_VIEW_LIST_EVENT_ITEM_REALIZED, &evt);
   efl_ui_focus_composition_dirty(obj);

   evas_object_show(item->layout);
   return item;
}

EOLIAN static void
_efl_ui_view_list_efl_ui_view_list_model_unrealize(Eo *obj, Efl_Ui_View_List_Data *pd, Efl_Ui_View_List_LayoutItem *item)
{
   Efl_Ui_View_List_Item_Event evt;
   EINA_SAFETY_ON_NULL_RETURN(item->layout);

   evas_object_event_callback_del_full(item->layout, EVAS_CALLBACK_MOUSE_UP, _on_item_mouse_up, item);
   if (elm_object_focus_allow_get(item->layout))
     {
        if (elm_object_focus_get(item->layout))
          elm_object_focus_set(item->layout, EINA_FALSE);
        efl_ui_focus_manager_calc_unregister(obj, item->layout);
     }
   evas_object_hide(item->layout);
   evas_object_move(item->layout, -9999, -9999);

   evt.child = item->children;
   evt.layout = item->layout;
   evt.index = efl_ui_view_list_item_index_get(item);
   efl_event_callback_call(obj, EFL_UI_VIEW_LIST_EVENT_ITEM_UNREALIZED, &evt);

   evas_object_smart_member_del(item->layout);
   efl_ui_factory_release(pd->factory, item->layout);
   item->layout = NULL;
}

EOLIAN static void
_efl_ui_view_list_efl_ui_view_list_model_load_range_set(Eo* obj, Efl_Ui_View_List_Data* pd, int first, int count)
{
   if (pd->slice.future) return ;

   pd->slice.start = first;
   pd->slice.count = count;

   if (efl_model_children_count_get(pd->model))
     {
        Eina_Future *f = efl_model_children_slice_get(pd->model, first, count);
        f = eina_future_then(f, _children_slice_then, pd);
        pd->slice.future = efl_future_Eina_FutureXXX_then(obj, f);
     }
}

EOLIAN static int
_efl_ui_view_list_efl_ui_view_list_model_model_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_View_List_Data *pd)
{
   return efl_model_children_count_get(pd->model);
}

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_view_list, Efl_Ui_View_List_Data)

/* Internal EO APIs and hidden overrides */

#define EFL_UI_VIEW_LIST_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_view_list)

#include "efl_ui_view_list.eo.c"
#include "efl_ui_view_list_relayout.eo.c"
#include "efl_ui_view_list_model.eo.c"
