#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_SCROLLBAR_BETA

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_list_item_private.h"
#include "efl_ui_item_private.h"
#include "efl_ui_list_private.h"

#define MY_CLASS      EFL_UI_LIST_CLASS
#define MY_CLASS_PFX  efl_ui_list

#define MY_CLASS_NAME "Efl.Ui.List"

static void
_item_scroll_internal(Eo *obj,
                      Efl_Ui_List_Item *item,
                      double align,
                      Eina_Bool anim)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   Eina_Rect ipos, view;
   Eina_Position2D vpos;

   if (!pd->smanager) return;

   ipos = efl_gfx_entity_geometry_get(item);
   view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
   vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

   ipos.x = view.x;
   ipos.w = view.w;

   // FIXME: align case will not correctly show in the position because of
   //        bar size calculation. there are no certain way to know the scroll calcuation finished.
   if (EINA_DBL_EQ(align, -1.0))
     {
        ipos.y = ipos.y + vpos.y - view.y;
        ipos.h = view.h;
     }
   else if ((align > 0.0 || EINA_DBL_EQ(align, 0.0)) &&
            (align < 1.0 || EINA_DBL_EQ(align, 1.0)))
     {
        ipos.y = ipos.y + vpos.y - view.y - (int)((view.h - ipos.h) * align);
        ipos.h = view.h;
     }
   else ERR("align (%.2lf) is not proper value. it must be the value between [0.0 , 1.0]!", align);

   efl_ui_scrollable_scroll(pd->smanager, ipos, anim);
}

static void
_efl_ui_list_bar_read_and_update(Eo *obj)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   double vx = 0.0, vy = 0.0;

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.dragable.vbar", NULL, &vy);

   /* FIXME: Horizontal Scroll is not yet supported in the list.
      edje_object_part_drag_value_get
      (wd->resize_obj, "efl.dragable.hbar", &vx, NULL);
    */

   efl_ui_scrollbar_bar_position_set(pd->smanager, vx, vy);
}

static void
_efl_ui_list_reload_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   Eo *list = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(list, pd);

   efl_ui_scrollbar_bar_visibility_update(pd->smanager);
}

static void
_efl_ui_list_vbar_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   _efl_ui_list_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_list_vbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_list_vbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_list_edje_drag_start_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   Eo *list = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(list, pd);

   _efl_ui_list_bar_read_and_update(list);

   pd->freeze_want = efl_ui_scrollable_scroll_freeze_get(pd->smanager);
   efl_ui_scrollable_scroll_freeze_set(pd->smanager, EINA_TRUE);
   efl_event_callback_call(list, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
}

static void
_efl_ui_list_edje_drag_stop_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   Eo *list = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(list, pd);

   _efl_ui_list_bar_read_and_update(list);

   efl_ui_scrollable_scroll_freeze_set(pd->smanager, pd->freeze_want);
   efl_event_callback_call(list, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_efl_ui_list_edje_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   _efl_ui_list_bar_read_and_update(data);
}

/* FIXME: Horizontal Scroll is not yet supported in the list.
   static void
   _efl_ui_list_hbar_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
   {
   _efl_ui_list_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
   }

   static void
   _efl_ui_list_hbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
   {
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
   }

   static void
   _efl_ui_list_hbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
   {
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
   }
 */

static void
_efl_ui_list_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double width = 0.0, height = 0.0;

   edje_object_calc_force(wd->resize_obj);
   efl_ui_scrollbar_bar_size_get(pd->smanager, &width, &height);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.hbar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.vbar", 1.0, height);
}

static void
_efl_ui_list_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0;

   efl_ui_scrollbar_bar_position_get(pd->smanager, &posx, &posy);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.hbar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.vbar", 0.0, posy);
}

static void
_efl_ui_list_bar_show_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,vbar", "efl");
}

static void
_efl_ui_list_bar_hide_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,hide,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,hide,vbar", "efl");
}

static void
_scroll_edje_object_attach(Eo *obj)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);

   efl_layout_signal_callback_add
     (obj, "reload", "efl",
     _efl_ui_list_reload_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag", "efl.dragable.vbar",
     _efl_ui_list_vbar_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,set", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,start", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_start_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,stop", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_stop_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,step", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "drag,page", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_cb, obj);
   efl_layout_signal_callback_add
     (obj, "efl,vbar,press", "efl",
     _efl_ui_list_vbar_press_cb, obj);
   efl_layout_signal_callback_add
     (obj, "efl,vbar,unpress", "efl",
     _efl_ui_list_vbar_unpress_cb, obj);

   /* FIXME: Horizontal Scroll is not yet supported in the list.
      efl_layout_signal_callback_add
      (obj, "drag", "efl.dragable.hbar",
       _efl_ui_list_hbar_drag_cb, obj);
      efl_layout_signal_callback_add
      (obj, "drag,set", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_cb, obj);
      efl_layout_signal_callback_add
      (obj, "drag,start", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_start_cb, obj);
      efl_layout_signal_callback_add
      (obj, "drag,stop", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_stop_cb, obj);
      efl_layout_signal_callback_add
      (obj, "drag,step", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_cb, obj);
      efl_layout_signal_callback_add
      (obj, "drag,page", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_cb, obj);
      efl_layout_signal_callback_add
      (obj, "efl,hbar,press", "efl",
       _efl_ui_list_hbar_press_cb, obj);
      efl_layout_signal_callback_add
      (obj, "efl,hbar,unpress", "efl",
       _efl_ui_list_hbar_unpress_cb, obj);
    */
}

static void
_scroll_edje_object_detach(Eo *obj)
{
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);

   efl_layout_signal_callback_del
     (obj, "reload", "efl",
     _efl_ui_list_reload_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag", "efl.dragable.vbar",
     _efl_ui_list_vbar_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,set", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,start", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_start_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,stop", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_stop_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,step", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "drag,page", "efl.dragable.vbar",
     _efl_ui_list_edje_drag_cb, obj);
   efl_layout_signal_callback_del
     (obj, "efl,vbar,press", "efl",
     _efl_ui_list_vbar_press_cb, obj);
   efl_layout_signal_callback_del
     (obj, "efl,vbar,unpress", "efl",
     _efl_ui_list_vbar_unpress_cb, obj);

   /* FIXME: Horizontal Scroll is not yet supported in the list.
      efl_layout_signal_callback_del
      (obj, "drag", "efl.dragable.hbar",
       _efl_ui_list_hbar_drag_cb, obj);
      efl_layout_signal_callback_del
      (obj, "drag,set", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_cb, obj);
      efl_layout_signal_callback_del
      (obj, "drag,start", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_start_cb, obj);
      efl_layout_signal_callback_del
      (obj, "drag,stop", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_stop_cb, obj);
      efl_layout_signal_callback_del
      (obj, "drag,step", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_cb, obj);
      efl_layout_signal_callback_del
      (obj, "drag,page", "efl.dragable.hbar",
       _efl_ui_list_edje_drag_cb, obj);
      efl_layout_signal_callback_del
      (obj, "efl,hbar,press", "efl",
       _efl_ui_list_hbar_press_cb, obj);
      efl_layout_signal_callback_del
      (obj, "efl,hbar,unpress", "efl",
       _efl_ui_list_hbar_unpress_cb, obj);
    */
}

static void
_efl_ui_list_pan_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);
   if (!pd->smanager) return;

   elm_layout_sizing_eval(obj);

   if (!pd->pan_resized)
     {
        Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
        // FIXME: Below code is workaround size check
        if (view.h > 1)
          {
             pd->pan_resized = EINA_TRUE;
             if (pd->scroll.item)
               {
                  _item_scroll_internal(obj, pd->scroll.item, pd->scroll.align, pd->scroll.anim);

                  pd->scroll.item = NULL;
                  pd->scroll.align = 0.0;
                  pd->scroll.anim = EINA_FALSE;
                  return;
               }
          }
     }
}

static void
_efl_ui_list_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_efl_ui_list_size_hint_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

EOLIAN static Eo *
_efl_ui_list_efl_object_constructor(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Eo *
_efl_ui_list_efl_object_finalize(Eo *obj,
                                 Efl_Ui_List_Data *pd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   Efl_Ui_Theme_Apply_Result theme_apply_ret = efl_ui_layout_theme_set(obj, "list", "base", efl_ui_widget_style_get(obj));
   if (theme_apply_ret == EFL_UI_THEME_APPLY_RESULT_FAIL)
     CRI("list(%p) failed to set theme [efl/list:%s]!", obj, efl_ui_widget_style_get(obj) ?: "NULL");

   pd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));
   efl_composite_attach(obj, pd->smanager);

   pd->pan = efl_add(EFL_UI_PAN_CLASS, obj);

   efl_ui_scroll_manager_pan_set(pd->smanager, pd->pan);
   edje_object_part_swallow(wd->resize_obj, "efl.content", pd->pan);

   pd->box = efl_add(EFL_UI_BOX_CLASS, obj,
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL));
   efl_ui_mirrored_set(pd->box, efl_ui_mirrored_get(obj));
   efl_content_set(pd->pan, pd->box);

   pd->select_mode = EFL_UI_SELECT_SINGLE;

   _scroll_edje_object_attach(obj);

   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_list_bar_size_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_list_bar_pos_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_list_bar_show_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_list_bar_hide_cb, obj);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_RESIZE,
                          _efl_ui_list_resized_cb, obj);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_CHANGE_SIZE_HINTS,
                          _efl_ui_list_size_hint_changed_cb, obj);
   efl_event_callback_add(pd->pan, EFL_GFX_ENTITY_EVENT_RESIZE,
                          _efl_ui_list_pan_resized_cb, obj);

   elm_layout_sizing_eval(obj);

   return obj;
}

EOLIAN static void
_efl_ui_list_efl_object_destructor(Eo *obj, Efl_Ui_List_Data *pd)
{
   _scroll_edje_object_detach(obj);

   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_list_bar_size_changed_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_list_bar_pos_changed_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_list_bar_show_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_list_bar_hide_cb, obj);
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_RESIZE,
                          _efl_ui_list_resized_cb, obj);
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_CHANGE_SIZE_HINTS,
                          _efl_ui_list_size_hint_changed_cb, obj);
   efl_event_callback_del(pd->pan, EFL_GFX_ENTITY_EVENT_RESIZE,
                          _efl_ui_list_pan_resized_cb, obj);

   efl_del(pd->box);
   pd->box = NULL;
   efl_del(pd->pan);
   pd->pan = NULL;
   efl_del(pd->smanager);
   pd->smanager = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_list_efl_canvas_group_group_calculate(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd EINA_UNUSED)
{
   // Need to be implemented
   efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_list_elm_layout_sizing_eval(Eo *obj, Efl_Ui_List_Data *pd)
{
   Eina_Size2D min = {0, 0}, max = {0, 0}, size = {-1, -1};
   Eina_Rect view = {};
   Evas_Coord vmw = 0, vmh = 0;
   double xw = 0.0, yw = 0.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (pd->box)
     {
        min = efl_gfx_size_hint_combined_min_get(pd->box);
        max = efl_gfx_size_hint_max_get(pd->box);
        efl_gfx_size_hint_weight_get(pd->box, &xw, &yw);
     }

   if (pd->smanager)
     view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);

   if (xw > 0.0)
     {
        if ((min.w > 0) && (view.w < min.w))
          view.w = min.w;
        else if ((max.w > 0) && (view.w > max.w))
          view.w = max.w;
     }
   else if (min.w > 0)
     view.w = min.w;

   if (yw > 0.0)
     {
        if ((min.h > 0) && (view.h < min.h))
          view.h = min.h;
        else if ((max.h > 0) && (view.h > max.h))
          view.h = max.h;
     }
   else if (min.h > 0)
     view.h = min.h;

   if (pd->box) efl_gfx_entity_size_set(pd->box, EINA_SIZE2D(view.w, view.h));

   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   if (pd->match_content_w) size.w = vmw + min.w;
   if (pd->match_content_h) size.h = vmh + min.h;

   max = efl_gfx_size_hint_max_get(obj);
   if ((max.w > 0) && (size.w > max.w)) size.w = max.w;
   if ((max.h > 0) && (size.h > max.h)) size.h = max.h;

   efl_gfx_size_hint_min_set(obj, size);
}

//FIXME: is this box related API could be improved more?
EOLIAN static int
_efl_ui_list_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->box ? efl_content_count(pd->box) : 0;
}

EOLIAN static Eina_Iterator *
_efl_ui_list_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->box ? efl_content_iterate(pd->box) : NULL;
}

/* FIXME: Direction is not supported yet in the list
   EOLIAN static void
   _efl_ui_list_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_List_Data *pd, Efl_Ui_Dir dir)
   {
   switch (dir)
     {
      case EFL_UI_DIR_RTL:
        // FIXME: Should be inverted!
      case EFL_UI_DIR_HORIZONTAL:
      case EFL_UI_DIR_LTR:
        pd->dir = EFL_UI_DIR_HORIZONTAL;
        break;

      case EFL_UI_DIR_UP:
        // FIXME: Should be inverted!
      case EFL_UI_DIR_DOWN:
      case EFL_UI_DIR_VERTICAL:
      case EFL_UI_DIR_DEFAULT:
      default:
        pd->dir = EFL_UI_DIR_VERTICAL;
        break;
     }

   efl_pack_layout_request(obj);
   }
 */

EOLIAN static Efl_Ui_Theme_Apply_Result
_efl_ui_list_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_List_Data *pd)
{
   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   return int_ret;
}

static void
_list_item_pressed(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_List_Item *item = event->object;
   efl_event_callback_call(obj, EFL_UI_EVENT_PRESSED, item);
}

static void
_list_item_unpressed(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_List_Item *item = event->object;
   efl_event_callback_call(obj, EFL_UI_EVENT_UNPRESSED, item);
}

static void
_list_item_longpressed(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_List_Item *item = event->object;
   efl_event_callback_call(obj, EFL_UI_EVENT_LONGPRESSED, item);
}

static void
_list_item_selected(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_List_Item *item = event->object;
   Efl_Ui_List_Item *selected;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);

   /* Single Select */
   if (pd->select_mode != EFL_UI_SELECT_MULTI)
     {
        EINA_LIST_FREE(pd->selected, selected)
          {
             if (selected != item)
               efl_ui_item_selected_set(selected, EINA_FALSE);
          }
     }
   pd->selected = eina_list_append(pd->selected, item);
   pd->last_selected = item;

   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTED, item);
}

static void
_list_item_unselected(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_List_Item *item = event->object;
   EFL_UI_LIST_DATA_GET_OR_RETURN(obj, pd);

   pd->selected = eina_list_remove(pd->selected, item);
   if (pd->last_selected == item) pd->last_selected = NULL;

   efl_event_callback_call(obj, EFL_UI_EVENT_UNSELECTED, item);
}

static Eina_Bool
_list_item_process(Eo *obj, Efl_Ui_List_Data *pd, EINA_UNUSED Efl_Ui_List_Item *it)
{
   EFL_UI_LIST_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   //FIXME: This is tricky workaround for set select mode and parent value.
   EFL_UI_LIST_ITEM_DATA_GET_OR_RETURN(it, ld, EINA_FALSE);
   EFL_UI_ITEM_DATA_GET_OR_RETURN(it, id, EINA_FALSE);
   id->select_mode = &(pd->select_mode);
   id->parent = obj;
   ld->parent = obj;
   efl_ui_mirrored_set(it, efl_ui_mirrored_get(obj));

   efl_event_callback_add(it, EFL_UI_EVENT_PRESSED, _list_item_pressed, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_UNPRESSED, _list_item_unpressed, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_LONGPRESSED, _list_item_longpressed, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_SELECTED, _list_item_selected, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_UNSELECTED, _list_item_unselected, obj);

   return EINA_TRUE;
}

static void
_list_item_clear(Eo *obj, Efl_Ui_List_Data *pd EINA_UNUSED, EINA_UNUSED Efl_Ui_List_Item *it)
{
   EFL_UI_LIST_ITEM_CHECK_OR_RETURN(it);
   EFL_UI_LIST_ITEM_DATA_GET_OR_RETURN(it, ld);
   EFL_UI_ITEM_DATA_GET_OR_RETURN(it, id);
   id->select_mode = NULL;
   id->parent = NULL;
   ld->parent = NULL;

   efl_event_callback_del(it, EFL_UI_EVENT_PRESSED, _list_item_pressed, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_UNPRESSED, _list_item_unpressed, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_LONGPRESSED, _list_item_longpressed, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_SELECTED, _list_item_selected, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_UNSELECTED, _list_item_unselected, obj);
}

/* Pack APIs */
EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_pack_clear(Eo *obj, Efl_Ui_List_Data *pd)
{
   Eina_Bool ret = efl_pack_clear(pd->box);

   if (ret)
     {
        eina_list_free(pd->items);
        eina_list_free(pd->selected);
        pd->items = NULL;
        pd->selected = NULL;
     }

   elm_layout_sizing_eval(obj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_unpack_all(Eo *obj, Efl_Ui_List_Data *pd)
{
   Eina_Bool ret = efl_pack_unpack_all(pd->box);

   if (ret)
     {
        Efl_Ui_List_Item *it = NULL;
        EINA_LIST_FREE(pd->items, it)
          {
             _list_item_clear(obj, pd, it);
          }
        eina_list_free(pd->selected);
        pd->items = NULL;
        pd->selected = NULL;
     }

   elm_layout_sizing_eval(obj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_unpack(Eo *obj, Efl_Ui_List_Data *pd, Efl_Gfx_Entity *subobj)
{
   Eina_Bool ret = efl_pack_unpack(pd->box, subobj);
   Efl_Ui_List_Item *item = (Efl_Ui_List_Item *)subobj;

   if (ret)
     {
        pd->items = eina_list_remove(pd->items, item);
        if (efl_ui_item_selected_get(item))
          {
             pd->selected = eina_list_remove(pd->selected, item);
          }
        _list_item_clear(obj, pd, item);
     }

   elm_layout_sizing_eval(obj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_pack(Eo *obj, Efl_Ui_List_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_list_item_process(obj, pd, subobj)) return EINA_FALSE;

   Eina_Bool ret = efl_pack_end(pd->box, subobj);
   pd->items = eina_list_append(pd->items, subobj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_List_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_list_item_process(obj, pd, subobj)) return EINA_FALSE;

   Eina_Bool ret = efl_pack_end(pd->box, subobj);
   pd->items = eina_list_append(pd->items, subobj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_List_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_list_item_process(obj, pd, subobj)) return EINA_FALSE;

   Eina_Bool ret = efl_pack_begin(pd->box, subobj);
   pd->items = eina_list_prepend(pd->items, subobj);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_linear_pack_before(Eo *obj,
                                         Efl_Ui_List_Data *pd,
                                         Efl_Gfx_Entity *subobj,
                                         const Efl_Gfx_Entity *existing)
{
   if (!_list_item_process(obj, pd, subobj)) return EINA_FALSE;
   EFL_UI_LIST_ITEM_CHECK_OR_RETURN(existing, EINA_FALSE);

   Eina_Bool ret = efl_pack_before(pd->box, subobj, existing);
   pd->items = eina_list_prepend_relative(pd->items, subobj, existing);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_linear_pack_after(Eo *obj,
                                        Efl_Ui_List_Data *pd,
                                        Efl_Gfx_Entity *subobj,
                                        const Efl_Gfx_Entity *existing)
{
   if (!_list_item_process(obj, pd, subobj)) return EINA_FALSE;
   EFL_UI_LIST_ITEM_CHECK_OR_RETURN(existing, EINA_FALSE);

   Eina_Bool ret = efl_pack_after(pd->box, subobj, existing);
   pd->items = eina_list_append_relative(pd->items, subobj, existing);
   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_list_efl_pack_linear_pack_at(Eo *obj,
                                     Efl_Ui_List_Data *pd,
                                     Efl_Gfx_Entity *subobj,
                                     int index)
{
   if (!_list_item_process(obj, pd, subobj)) return EINA_FALSE;
   Efl_Ui_List_Item *existing = efl_pack_content_get(pd->box, index);

   Eina_Bool ret = efl_pack_at(pd->box, subobj, index);
   pd->items = eina_list_append_relative(pd->items, subobj, existing);
   return ret;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_list_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int index)
{
   return efl_pack_content_get(pd->box, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_list_efl_pack_linear_pack_unpack_at(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, int index)
{
   return efl_pack_unpack_at(pd->box, index);
}

EOLIAN static int
_efl_ui_list_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                            Efl_Ui_List_Data *pd,
                                            const Efl_Gfx_Entity *subobj)
{
   return efl_pack_index_get(pd->box, subobj);
}

EOLIAN static void
_efl_ui_list_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_List_Data *pd)
{
   efl_pack_layout_update(pd->box);
   elm_layout_sizing_eval(obj);

   efl_event_callback_legacy_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

EOLIAN static void
_efl_ui_list_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_List_Data *pd)
{
   efl_pack_layout_request(pd->box);
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_list_efl_pack_pack_padding_set(Eo *obj EINA_UNUSED,
                                       Efl_Ui_List_Data *pd,
                                       double h,
                                       double v,
                                       Eina_Bool scalable)
{
   efl_pack_padding_set(pd->box, h, v, scalable);
}

EOLIAN static void
_efl_ui_list_efl_pack_pack_padding_get(const Eo *obj EINA_UNUSED,
                                       Efl_Ui_List_Data *pd,
                                       double *h,
                                       double *v,
                                       Eina_Bool *scalable)
{
   efl_pack_padding_get(pd->box, h, v, scalable);
}

/* FIXME: align could not work properly on the list
   EOLIAN static void
   _efl_ui_list_efl_pack_pack_align_set(Eo *obj, Efl_Ui_List_Data *pd, double h, double v)
   {
   efl_pack_align_set(pd->box, h, v);
   }

   EOLIAN static void
   _efl_ui_list_efl_pack_pack_align_get(Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd, double *h, double *v)
   {
   efl_pack_align_get(pd->box, &h, &v);
   }
 */

/* Scroll APIs */
EOLIAN static void
_efl_ui_list_efl_ui_scrollable_interactive_match_content_set(Eo *obj EINA_UNUSED,
                                                             Efl_Ui_List_Data *pd,
                                                             Eina_Bool match_content_w,
                                                             Eina_Bool match_content_h)
{
   pd->match_content_w = !!match_content_w;
   pd->match_content_h = !!match_content_h;

   efl_ui_scrollable_match_content_set(pd->smanager, match_content_w, match_content_h);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_list_efl_ui_multi_selectable_select_mode_set(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_List_Data *pd,
                                                     Efl_Ui_Select_Mode mode)
{
   Efl_Ui_List_Item *selected;

   if ((pd->select_mode == EFL_UI_SELECT_MULTI &&
        mode != EFL_UI_SELECT_MULTI) ||
       mode == EFL_UI_SELECT_NONE)
     {
        Eina_List *clone = eina_list_clone(pd->selected);
        EINA_LIST_FREE(clone, selected)
          efl_ui_item_selected_set(selected, EINA_FALSE);
     }

   pd->select_mode = mode;
}

EOLIAN static Efl_Ui_Select_Mode
_efl_ui_list_efl_ui_multi_selectable_select_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->select_mode;
}

/* List APIs */
EOLIAN static void
_efl_ui_list_item_scroll(Eo *obj,
                         Efl_Ui_List_Data *pd,
                         Efl_Ui_List_Item *item,
                         Eina_Bool animation)
{
   // Need to be implemented here.
   if (pd->pan_resized)
     {
        _item_scroll_internal(obj, item, -1.0, animation);
     }
   else
     {
        pd->scroll.item = item;
        pd->scroll.align = -1.0;
        pd->scroll.anim = animation;
     }
}

EOLIAN static void
_efl_ui_list_item_scroll_align(Eo *obj,
                               Efl_Ui_List_Data *pd,
                               Efl_Ui_List_Item *item,
                               double align,
                               Eina_Bool animation)
{
   // Need to be implemented here.
   if (pd->pan_resized)
     {
        _item_scroll_internal(obj, item, align, animation);
     }
   else
     {
        pd->scroll.item = item;
        pd->scroll.align = align;
        pd->scroll.anim = animation;
     }
}

EOLIAN static Efl_Ui_List_Item *
_efl_ui_list_last_selected_item_get(const Eo *obj EINA_UNUSED, Efl_Ui_List_Data *pd)
{
   return pd->last_selected;
}

static Eina_Bool
_list_item_iterator_next(Item_Iterator *it, void **data)
{
   Efl_Ui_List_Item *item;

   if (!eina_iterator_next(it->real_iterator, (void **)&item))
     return EINA_FALSE;

   if (data) *data = item;
   return EINA_TRUE;
}

static Eo *
_list_item_iterator_get_container(Item_Iterator *it)
{
   return it->object;
}

static void
_list_item_iterator_free(Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   free(it);
}

EOLIAN static Eina_Iterator *
_efl_ui_list_selected_items_get(Eo *obj, Efl_Ui_List_Data *pd)
{
   Item_Iterator *item;

   item = calloc(1, sizeof(*item));
   if (!item) return NULL;

   EINA_MAGIC_SET(&item->iterator, EINA_MAGIC_ITERATOR);

   item->list = eina_list_clone(pd->selected);
   item->real_iterator = eina_list_iterator_new(item->list);
   item->iterator.version = EINA_ITERATOR_VERSION;
   item->iterator.next = FUNC_ITERATOR_NEXT(_list_item_iterator_next);
   item->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_list_item_iterator_get_container);
   item->iterator.free = FUNC_ITERATOR_FREE(_list_item_iterator_free);
   item->object = obj;

   return &item->iterator;
}

/* Internal EO APIs and hidden overrides */
#define EFL_UI_LIST_EXTRA_OPS \
  ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_list)

#include "efl_ui_list.eo.c"
