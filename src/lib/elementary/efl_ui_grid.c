#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_grid_private.h"
#include "efl_ui_item_private.h"
#include "efl_ui_grid_item_private.h"

#define MY_CLASS      EFL_UI_GRID_CLASS
#define MY_CLASS_PFX  efl_ui_grid

#define MY_CLASS_NAME "Efl.Ui.Grid"

static void _grid_clear_internal(Eo *obj, Efl_Ui_Grid_Data *pd);
static void _grid_item_unpack_internal(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Ui_Grid_Item *item);

static void
_need_update(Efl_Ui_Grid_Data *pd)
{
   pd->need_update = EINA_TRUE;
   //group_calculate call
   efl_canvas_group_change(pd->obj);
}

static void
_relayout(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, Eina_Position2D pan)
{
   Eina_List *l;
   Efl_Ui_Grid_Item *item;
   Efl_Ui_Grid_Item_Data *prev;
   Eina_Position2D ipos;
   Eina_Position2D cur = {0, 0};
   int outer;
   int count = 0;
   Eina_Bool horiz = 0;
   Eina_Size2D min, max;

   if (pd->dir == EFL_UI_DIR_HORIZONTAL) horiz = 1;

   if (!pd->linemax)
     {
        pd->linemax =(horiz ?
                     ((pd->geo.h + pd->item.pad.h) / (pd->item.size.h + pd->item.pad.h)) :
                     ((pd->geo.w + pd->item.pad.w) / (pd->item.size.w + pd->item.pad.w)));
        if (!pd->linemax) pd->linemax = 1;
     }


   outer = (horiz ?
            (pd->geo.h + pd->item.pad.h) - (pd->linemax * (pd->item.size.h + pd->item.pad.h)) :
            (pd->geo.w + pd->item.pad.w) - (pd->linemax * (pd->item.size.w + pd->item.pad.w)));
  // outer left top
   outer = (horiz ? (outer * pd->item.align.h) : (outer * pd->item.align.w));
   outer = (horiz ? (outer + pd->geo.y) : (outer + pd->geo.x));

   // Block?
   EINA_LIST_FOREACH(pd->items, l, item)
     {
        EFL_UI_GRID_ITEM_DATA_GET(item, id);

        if (pd->need_update || (id && (id->update_me || id->update_begin)))
          {
             // Index begin with zero value :
             id->index = count;
             if (id->update_me) id->update_me = EINA_FALSE;
             if (id->update_begin)
               {
                  id->update_begin = EINA_FALSE;
                  pd->need_update = EINA_TRUE;
               }
             if (horiz)
               {
                  id->pos.row = (count % pd->linemax);
                  id->pos.col = (count / pd->linemax);
                  //Next Line
                  if (count == 0)
                    {
                       cur.x = pd->geo.x;
                       cur.y = outer;
                    }
                  else if (id->pos.row == 0)// ((cur.y + pd->item.pad.h + pd->item.size.h) > pd->geo.h)
                    {
                       cur.x = prev->geo.x + pd->item.size.w + pd->item.pad.w;
                       cur.y = outer;
                    }
                  else
                    {
                       //cur.x = cur.x;
                       cur.y = prev->geo.y + pd->item.size.h + pd->item.pad.h;
                    }
               }
             else
               {
                  id->pos.row = (count / pd->linemax);
                  id->pos.col = (count % pd->linemax);
                  //Next Line
                  if (count == 0)
                    {
                       cur.x = outer;
                       cur.y = pd->geo.y;
                    }
                  else if (id->pos.col == 0)//((cur.x + pd->item.pad.w + pd->item.size.w) > pd->geo.w)
                    {
                       cur.x = outer;
                       cur.y = prev->geo.y + pd->item.size.h + pd->item.pad.h;
                    }
                  else
                    {
                       cur.x = prev->geo.x + pd->item.size.w + pd->item.pad.w;
                       //cur.y = cur.y;
                    }
               }

             min = efl_gfx_hint_size_min_get(item);
             max = efl_gfx_hint_size_max_get(item);

             if (pd->item.size.w < min.w) pd->item.size.w = min.w;
             if (pd->item.size.h < min.h) pd->item.size.h = min.h;
             if ((max.w > 0) && pd->item.size.w > max.w) pd->item.size.w = max.w;
             if ((max.h > 0) && pd->item.size.h > max.h) pd->item.size.h = max.h;

             id->geo.x = cur.x;
             id->geo.y = cur.y;
             id->geo.w = pd->item.size.w;
             id->geo.h = pd->item.size.h;


          }

        ipos.x = id->geo.x - pan.x;
        ipos.y = id->geo.y - pan.y;

        //
        efl_gfx_entity_position_set(item, ipos);
        efl_gfx_entity_size_set(item, id->geo.size);
        //efl_gfx_hint_size_restricted_min_set(item, id->geo.size);

        prev = id;
        count++;
     }

   if (horiz)
     pd->geo.w = cur.x + pd->item.size.w + pd->item.pad.w - pd->geo.x;
   else
     pd->geo.h = cur.y + pd->item.size.h + pd->item.pad.h - pd->geo.y;

   //efl_gfx_hint_size_restricted_min_set(pd->content, pd->geo.size);
   efl_gfx_entity_size_set(pd->content, pd->geo.size);

   pd->need_update = EINA_FALSE;
}

static void
_reposition(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, Eina_Position2D pan)
{
   Efl_Ui_Grid_Item *item;
   Eina_List *l;
   Eina_Position2D ipos;

   // Block?
   EINA_LIST_FOREACH(pd->items, l, item)
     {
        EFL_UI_GRID_ITEM_DATA_GET(item, id);
        if (!id) continue;

        ipos.x = id->geo.x - pan.x;
        ipos.y = id->geo.y - pan.y;
        efl_gfx_entity_position_set(item, ipos);
        efl_gfx_entity_size_set(item, id->geo.size);
        //efl_gfx_hint_size_min_set(item, id->geo.size);
     }

}

//Need to reimplements for grid
static void
_item_scroll_internal(Eo *obj,
                      Efl_Ui_Grid_Item *item,
                      double align,
                      Eina_Bool anim)
{
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   Eina_Rect ipos, view;
   Eina_Position2D vpos;

   if (!pd->smanager) return;

   ipos = efl_gfx_entity_geometry_get(item);
   view = efl_ui_scrollable_viewport_geometry_get(pd->smanager);
   vpos = efl_ui_scrollable_content_pos_get(pd->smanager);

   if (pd->dir == EFL_UI_DIR_HORIZONTAL)
     {
       ipos.y = view.y;
       //ipos.h = ipos.h;

       // FIXME: align case will not correctly show in the position because of
       //        bar size calculation. there are no certain way to know the scroll calcuation finished.
       if (EINA_DBL_EQ(align, -1.0)) //Internal Prefix
         {
            ipos.x = ipos.x + vpos.x - view.x;
            //ipos.w = ipos.w;
         }
       else if ((align > 0.0 || EINA_DBL_EQ(align, 0.0)) &&
                (align < 1.0 || EINA_DBL_EQ(align, 1.0)))
         {
            ipos.x = ipos.x + vpos.x - view.x - (int)((view.w - ipos.w) * align);
            ipos.w = view.w;
         }
       else ERR("align (%.2lf) is not proper value. it must be the value between [0.0 , 1.0]!", align);

     }
  else //VERTICAL
    {
       ipos.x = view.x;
       //ipos.w = ipos.w;

       // FIXME: align case will not correctly show in the position because of
       //        bar size calculation. there are no certain way to know the scroll calcuation finished.
       if (EINA_DBL_EQ(align, -1.0)) //Internal Prefix
         {
            ipos.y = ipos.y + vpos.y - view.y;
            //ipos.h = ipos.h;
         }
       else if ((align > 0.0 || EINA_DBL_EQ(align, 0.0)) &&
                (align < 1.0 || EINA_DBL_EQ(align, 1.0)))
         {
            ipos.y = ipos.y + vpos.y - view.y - (int)((view.h - ipos.h) * align);
            ipos.h = view.h;
         }
       else ERR("align (%.2lf) is not proper value. it must be the value between [0.0 , 1.0]!", align);
    }

   efl_ui_scrollable_scroll(pd->smanager, ipos, anim);
}

static void
_efl_ui_grid_bar_read_and_update(Eo *obj)
{
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   double vx = 0.0, vy = 0.0;

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.dragable.vbar", NULL, &vy);

   edje_object_part_drag_value_get
     (wd->resize_obj, "efl.dragable.hbar", &vx, NULL);

   efl_ui_scrollbar_bar_position_set(pd->smanager, vx, vy);
}

static void
_efl_ui_grid_reload_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   Eo *list = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(list, pd);

   efl_ui_scrollbar_bar_visibility_update(pd->smanager);
}

static void
_efl_ui_grid_vbar_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   _efl_ui_grid_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_grid_vbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_grid_vbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_grid_edje_drag_start_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   Eo *list = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(list, pd);

   _efl_ui_grid_bar_read_and_update(list);

   pd->freeze_want = efl_ui_scrollable_scroll_freeze_get(pd->smanager);
   efl_ui_scrollable_scroll_freeze_set(pd->smanager, EINA_TRUE);
   efl_event_callback_call(list, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
}

static void
_efl_ui_grid_edje_drag_stop_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   Eo *list = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(list, pd);

   _efl_ui_grid_bar_read_and_update(list);

   efl_ui_scrollable_scroll_freeze_set(pd->smanager, pd->freeze_want);
   efl_event_callback_call(list, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_efl_ui_grid_edje_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   _efl_ui_grid_bar_read_and_update(data);
}

static void
_efl_ui_grid_hbar_drag_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   _efl_ui_grid_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_grid_hbar_press_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_grid_hbar_unpress_cb(void *data,
                             Evas_Object *obj EINA_UNUSED,
                             const char *emission EINA_UNUSED,
                             const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_grid_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double width = 0.0, height = 0.0;

   edje_object_calc_force(wd->resize_obj);
   efl_ui_scrollbar_bar_size_get(pd->smanager, &width, &height);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.hbar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.vbar", 1.0, height);
}

static void
_efl_ui_grid_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0;

   efl_ui_scrollbar_bar_position_get(pd->smanager, &posx, &posy);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.hbar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.vbar", 0.0, posy);
}

static void
_efl_ui_grid_bar_show_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,vbar", "efl");
}

static void
_efl_ui_grid_bar_hide_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
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
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);

   efl_layout_signal_callback_add(obj, "reload", "efl",
                                  obj, _efl_ui_grid_reload_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_vbar_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,set", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,vbar,press", "efl",
                                  obj, _efl_ui_grid_vbar_press_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,vbar,unpress", "efl",
                                  obj, _efl_ui_grid_vbar_unpress_cb, NULL);

   efl_layout_signal_callback_add(obj, "drag", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_hbar_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,set", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,hbar,press", "efl",
                                  obj, _efl_ui_grid_hbar_press_cb, NULL);
   efl_layout_signal_callback_add(obj, "efl,hbar,unpress", "efl",
                                  obj, _efl_ui_grid_hbar_unpress_cb, NULL);
}

static void
_scroll_edje_object_detach(Eo *obj)
{
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);

   efl_layout_signal_callback_del(obj, "reload", "efl",
                                  obj, _efl_ui_grid_reload_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_vbar_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,set", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,start", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,stop", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,step", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,page", "efl.dragable.vbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,vbar,press", "efl",
                                  obj, _efl_ui_grid_vbar_press_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,vbar,unpress", "efl",
                                  obj, _efl_ui_grid_vbar_unpress_cb, NULL);

   efl_layout_signal_callback_del(obj, "drag", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_hbar_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,set", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,start", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,stop", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,step", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "drag,page", "efl.dragable.hbar",
                                  obj, _efl_ui_grid_edje_drag_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,hbar,press", "efl",
                                  obj, _efl_ui_grid_hbar_press_cb, NULL);
   efl_layout_signal_callback_del(obj, "efl,hbar,unpress", "efl",
                                  obj, _efl_ui_grid_hbar_unpress_cb, NULL);
}

static void
_efl_ui_grid_content_moved_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   if (!pd->smanager) return;

   efl_canvas_group_change(pd->obj);
}

static void
_efl_ui_grid_pan_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   if (!pd->smanager) return;

   //reset linemax for recalculate layout.
   pd->linemax = 0;

   _need_update(pd);
   elm_layout_sizing_eval(obj);

   pd->pan_resized = EINA_TRUE;
}

static void
_efl_ui_grid_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_efl_ui_grid_size_hint_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

EOLIAN static Eo *
_efl_ui_grid_efl_object_constructor(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Eo *
_efl_ui_grid_efl_object_finalize(Eo *obj,
                                 Efl_Ui_Grid_Data *pd)
{
   pd->obj = obj = efl_finalize(efl_super(obj, MY_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   efl_ui_layout_theme_set(obj, "grid", "base", efl_ui_widget_style_get(obj));

   pd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_composite_attach(obj, pd->smanager);
   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));

   pd->pan = efl_add(EFL_UI_PAN_CLASS, obj);

   efl_ui_scroll_manager_pan_set(pd->smanager, pd->pan);
   edje_object_part_swallow(wd->resize_obj, "efl.content", pd->pan);

   pd->select_mode = EFL_UI_SELECT_MODE_SINGLE;

   if ((pd->item.size.w == 0) && (pd->item.size.h == 0))
     {
        pd->item.size.w = 1;
        pd->item.size.h = 1;
     }

   _scroll_edje_object_attach(obj);

   //FIXME: Workaround code! fake Content for pan resize.
  // to remove this code, we need to customize pan class.
   pd->content = efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj));
   efl_gfx_color_set(pd->content, 0, 0, 0, 0);
   efl_content_set(pd->pan, pd->content);

   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_grid_bar_size_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_grid_bar_pos_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_grid_bar_show_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_grid_bar_hide_cb, obj);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_grid_resized_cb, obj);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_grid_size_hint_changed_cb, obj);
   efl_event_callback_add(pd->pan, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_grid_pan_resized_cb, obj);
   efl_event_callback_add(pd->content, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED,
                          _efl_ui_grid_content_moved_cb, obj);

   elm_layout_sizing_eval(obj);

   return obj;
}


EOLIAN static void
_efl_ui_grid_efl_object_invalidate(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   _scroll_edje_object_detach(obj);

   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_grid_bar_size_changed_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_grid_bar_pos_changed_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_grid_bar_show_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_grid_bar_hide_cb, obj);
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_grid_resized_cb, obj);
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_grid_size_hint_changed_cb, obj);
   efl_event_callback_del(pd->pan, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_grid_pan_resized_cb, obj);
   efl_event_callback_del(pd->content, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED,
                          _efl_ui_grid_content_moved_cb, obj);
   _grid_clear_internal(obj, pd);

   if (pd->smanager) efl_del(pd->smanager);
   pd->smanager = NULL;
   if (pd->content) efl_del(pd->content);
   pd->content = NULL;
   if (pd->pan) efl_del(pd->pan);
   pd->pan = NULL;

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_grid_efl_object_destructor(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_grid_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Grid_Data *pd)
{

   Eina_Position2D pos = efl_ui_scrollable_content_pos_get(pd->smanager);
   Eina_Rect geo = efl_ui_scrollable_viewport_geometry_get(pd->smanager);

   //ERR("pos[%d,%d], geo[%d,%d,%d,%d]", pos.x, pos.y, geo.x, geo.y, geo.w, geo.h);

   if (geo.w <= 1 || geo.h <= 1) return;
   // Need to be implemented
   if (pd->need_update)
     {
        _relayout(obj, pd, pos);
     }
   else
     {
        if (pd->pre_pos.x != pos.x || pd->pre_pos.y != pos.y)
          {
             _reposition(obj, pd, pos);
          }
     }

   pd->pre_pos = pos;

   if (pd->pan_resized)
     {
        pd->pan_resized = EINA_FALSE;
        // FIXME: Below code is workaround size check
        if ((geo.h > 1) && (pd->scroll.item))
          {
             _item_scroll_internal(obj, pd->scroll.item, pd->scroll.align, pd->scroll.anim);

             pd->scroll.item = NULL;
             pd->scroll.align = 0.0;
             pd->scroll.anim = EINA_FALSE;
             return;
          }
     }

   efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_grid_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   Eina_Size2D min = {0, 0}, max = {0, 0}, size = {-1, -1};
   Eina_Rect view = {};
   Evas_Coord vmw = 0, vmh = 0;
   double xw = 0.0, yw = 0.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);


   min = efl_gfx_hint_size_combined_min_get(obj);
   max = efl_gfx_hint_size_max_get(obj);
   efl_gfx_hint_weight_get(obj, &xw, &yw);

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

   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   if (pd->match_content_w) size.w = vmw + min.w;
   if (pd->match_content_h) size.h = vmh + min.h;

   max = efl_gfx_hint_size_max_get(obj);
   if ((max.w > 0) && (size.w > max.w)) size.w = max.w;
   if ((max.h > 0) && (size.h > max.h)) size.h = max.h;
   pd->geo = view;
   efl_gfx_hint_size_min_set(obj, size);

   _need_update(pd);
   return;
}

EOLIAN static int
_efl_ui_grid_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return eina_list_count(pd->items);
}

static Eina_Bool
_grid_item_iterator_next(Item_Iterator *it, void **data)
{
   Efl_Ui_Grid_Item *item;

   if (!eina_iterator_next(it->real_iterator, (void **)&item))
     return EINA_FALSE;

   if (data) *data = item;
   return EINA_TRUE;
}

static Eo *
_grid_item_iterator_get_container(Item_Iterator *it)
{
   return it->object;
}

static void
_grid_item_iterator_free(Item_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   eina_list_free(it->list);
   free(it);
}

static Eina_Iterator *
_grid_item_iterator_new(Eo *obj, Eina_List *list)
{
  // NEED-TO-IMPLEMENTS
   Item_Iterator *item;

   item = calloc(1, sizeof(*item));
   if (!item) return NULL;

   EINA_MAGIC_SET(&item->iterator, EINA_MAGIC_ITERATOR);

   item->list = eina_list_clone(list);
   item->real_iterator = eina_list_iterator_new(item->list);
   item->iterator.version = EINA_ITERATOR_VERSION;
   item->iterator.next = FUNC_ITERATOR_NEXT(_grid_item_iterator_next);
   item->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_grid_item_iterator_get_container);
   item->iterator.free = FUNC_ITERATOR_FREE(_grid_item_iterator_free);
   item->object = obj;

   return &item->iterator;
}

EOLIAN static Eina_Iterator *
_efl_ui_grid_efl_container_content_iterate(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   return _grid_item_iterator_new(obj, pd->items);
}

EOLIAN static void
_efl_ui_grid_efl_ui_direction_direction_set(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, Efl_Ui_Dir dir)
{
   //FIXME: Currently only support horizontal and vertical mode.
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

   pd->linemax = 0;
   efl_pack_layout_request(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_grid_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->dir;
}


EOLIAN static Eina_Error
_efl_ui_grid_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   efl_ui_mirrored_set(pd->smanager, efl_ui_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   return int_ret;
}

static void
_grid_item_pressed(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Grid_Item *item = event->object;
   efl_event_callback_call(obj, EFL_UI_EVENT_PRESSED, item);
}

static void
_grid_item_unpressed(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Grid_Item *item = event->object;
   efl_event_callback_call(obj, EFL_UI_EVENT_UNPRESSED, item);
}

static void
_grid_item_longpressed(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Grid_Item *item = event->object;
   efl_event_callback_call(obj, EFL_UI_EVENT_LONGPRESSED, item);
}

static void
_grid_item_selected(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Grid_Item *item = event->object;
   Efl_Ui_Grid_Item *selected;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);

   /* Single Select */
   if (pd->select_mode != EFL_UI_SELECT_MODE_MULTI)
     {
        EINA_LIST_FREE(pd->selected, selected)
          {
             if (selected != item)
               efl_ui_item_selected_set(selected, EINA_FALSE);
          }
     }
   pd->selected = eina_list_append(pd->selected, item);
   pd->last_selected = item;

   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTABLE_SELECTED, item);
}

static void
_grid_item_unselected(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Grid_Item *item = event->object;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);

   pd->selected = eina_list_remove(pd->selected, item);
   if (pd->last_selected == item) pd->last_selected = NULL;

   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTABLE_UNSELECTED, item);
}

static void
_grid_item_deleted(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   Efl_Ui_Grid_Item *it = event->object;
   EFL_UI_GRID_DATA_GET_OR_RETURN(obj, pd);
   _grid_item_unpack_internal(obj, pd, it);
}

static Eina_Bool
_grid_item_process(Eo *obj, Efl_Ui_Grid_Data *pd, EINA_UNUSED Efl_Ui_Grid_Item *it)
{
   EFL_UI_GRID_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   //FIXME: This is tricky workaround for set select mode and parent value.
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(it, gd, EINA_FALSE);
   EFL_UI_ITEM_DATA_GET_OR_RETURN(it, id, EINA_FALSE);
   id->select_mode = &(pd->select_mode);
   id->parent = obj;
   gd->parent = obj;
   efl_canvas_group_member_add(pd->pan, it);
   efl_ui_mirrored_set(it, efl_ui_mirrored_get(obj));

   efl_event_callback_add(it, EFL_UI_EVENT_PRESSED, _grid_item_pressed, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_UNPRESSED, _grid_item_unpressed, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_LONGPRESSED, _grid_item_longpressed, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_SELECTABLE_SELECTED, _grid_item_selected, obj);
   efl_event_callback_add(it, EFL_UI_EVENT_SELECTABLE_UNSELECTED, _grid_item_unselected, obj);
   efl_event_callback_add(it, EFL_EVENT_DEL, _grid_item_deleted, obj);

   return EINA_TRUE;
}

static void
_grid_item_unpack_internal(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Ui_Grid_Item *it)
{
   EFL_UI_GRID_ITEM_CHECK_OR_RETURN(it);
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(it, gd);
   EFL_UI_ITEM_DATA_GET_OR_RETURN(it, id);
   id->select_mode = NULL;
   id->parent = NULL;
   gd->parent = NULL;

   pd->items = eina_list_remove(pd->items, it);
   if (efl_ui_item_selected_get(it))
     {
        pd->selected = eina_list_remove(pd->selected, it);
     }

   efl_event_callback_del(it, EFL_UI_EVENT_PRESSED, _grid_item_pressed, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_UNPRESSED, _grid_item_unpressed, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_LONGPRESSED, _grid_item_longpressed, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_SELECTABLE_SELECTED, _grid_item_selected, obj);
   efl_event_callback_del(it, EFL_UI_EVENT_SELECTABLE_UNSELECTED, _grid_item_unselected, obj);
   efl_event_callback_del(it, EFL_EVENT_DEL, _grid_item_deleted, obj);
}


static void
_grid_clear_internal(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   Efl_Ui_Grid_Item *it = NULL;
   Eina_List *l, *ll;
   EINA_LIST_FOREACH_SAFE(pd->items, l, ll, it)
     {
        efl_del(it);
     }
   eina_list_free(pd->selected);
   pd->items = NULL;
   pd->selected = NULL;
}

/* Pack APIs */
EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_pack_clear(Eo *obj, Efl_Ui_Grid_Data *pd)
{
  _grid_clear_internal(obj, pd);

   elm_layout_sizing_eval(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_unpack_all(Eo *obj, Efl_Ui_Grid_Data *pd)
{

   Efl_Ui_Grid_Item *it = NULL;
   Eina_List *l, *ll;
   EINA_LIST_FOREACH_SAFE(pd->items, l, ll, it)
     {
        _grid_item_unpack_internal(obj, pd, it);
     }
   eina_list_free(pd->selected);
   pd->items = NULL;
   pd->selected = NULL;

   elm_layout_sizing_eval(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_unpack(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx_Entity *subobj)
{
   Efl_Ui_Grid_Item *item = (Efl_Ui_Grid_Item *)subobj;

   _grid_item_unpack_internal(obj, pd, item);

   elm_layout_sizing_eval(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_pack(Eo *obj, Efl_Ui_Grid_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_grid_item_process(obj, pd, subobj)) return EINA_FALSE;
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(subobj, pid, EINA_FALSE);
   pd->items = eina_list_append(pd->items, subobj);

   pid->update_me = EINA_TRUE;

   efl_canvas_group_change(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Grid_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_grid_item_process(obj, pd, subobj)) return EINA_FALSE;
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(subobj, pid, EINA_FALSE);
   pd->items = eina_list_prepend(pd->items, subobj);
   // Defered item's placing in group calculation
   pid->update_me = EINA_TRUE;
   _need_update(pd);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_linear_pack_before(Eo *obj,
                                         Efl_Ui_Grid_Data *pd,
                                         Efl_Gfx_Entity *subobj,
                                         const Efl_Gfx_Entity *existing)
{
   if (!_grid_item_process(obj, pd, subobj)) return EINA_FALSE;
   EFL_UI_GRID_ITEM_CHECK_OR_RETURN(existing, EINA_FALSE);
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(subobj, pid, EINA_FALSE);

   pd->items = eina_list_prepend_relative(pd->items, subobj, existing);
   // Defered item's placing in group calculation
   pid->update_begin = EINA_TRUE;
   _need_update(pd);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_linear_pack_after(Eo *obj,
                                        Efl_Ui_Grid_Data *pd,
                                        Efl_Gfx_Entity *subobj,
                                        const Efl_Gfx_Entity *existing)
{
   if (!_grid_item_process(obj, pd, subobj)) return EINA_FALSE;
   EFL_UI_GRID_ITEM_CHECK_OR_RETURN(existing, EINA_FALSE);
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(subobj, pid, EINA_FALSE);

   pd->items = eina_list_append_relative(pd->items, subobj, existing);
   // Defered item's placing in group calculation
   pid->update_begin = EINA_TRUE;
   _need_update(pd);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_grid_efl_pack_linear_pack_at(Eo *obj,
                                     Efl_Ui_Grid_Data *pd,
                                     Efl_Gfx_Entity *subobj,
                                     int index)
{
   if (!_grid_item_process(obj, pd, subobj)) return EINA_FALSE;
   Efl_Ui_Grid_Item *existing = eina_list_nth(pd->items, index);
   EFL_UI_GRID_ITEM_DATA_GET_OR_RETURN(subobj, pid, EINA_FALSE);

   pd->items = eina_list_prepend_relative(pd->items, subobj, existing);
   // Defered item's placing in group calculation
   pid->update_begin = EINA_TRUE;
   _need_update(pd);
   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_grid_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, int index)
{
   return eina_list_nth(pd->items, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_grid_efl_pack_linear_pack_unpack_at(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, int index)
{
   Efl_Gfx_Entity *target = eina_list_nth(pd->items, index);
   pd->items = eina_list_remove(pd->items, target);
   /*
   if (after)
     {
     }
   else
   */
   _need_update(pd);
   return target;
}

EOLIAN static int
_efl_ui_grid_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Grid_Data *pd,
                                            const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->items, (void *)subobj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   _need_update(pd);
   elm_layout_sizing_eval(obj);

   efl_event_callback_legacy_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

EOLIAN static void
_efl_ui_grid_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   _need_update(pd);
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_grid_efl_pack_pack_padding_set(Eo *obj EINA_UNUSED,
                                       Efl_Ui_Grid_Data *pd,
                                       double h,
                                       double v,
                                       Eina_Bool scalable)
{
   pd->item.pad.w = (int )h;
   pd->item.pad.h = (int) v;

   pd->pad_scalable = !!scalable;

   //reset linemax for recalculate layout
   pd->linemax = 0;
   _need_update(pd);
}

EOLIAN static void
_efl_ui_grid_efl_pack_pack_padding_get(const Eo *obj EINA_UNUSED,
                                       Efl_Ui_Grid_Data *pd,
                                       double *h,
                                       double *v,
                                       Eina_Bool *scalable)
{
   //??
   *h = (double) pd->item.pad.w;
   *v = (double) pd->item.pad.h;
   *scalable = !!pd->pad_scalable;
}

EOLIAN static void
_efl_ui_grid_efl_pack_pack_align_set(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, double h, double v)
{
   pd->item.align.w = h;
   pd->item.align.h = v;
   _need_update(pd);
}

EOLIAN static void
_efl_ui_grid_efl_pack_pack_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, double *h, double *v)
{
   *h = pd->item.align.w;
   *v = pd->item.align.h;
}

EOLIAN static void
_efl_ui_grid_efl_ui_scrollable_interactive_match_content_set(Eo *obj EINA_UNUSED,
                                                             Efl_Ui_Grid_Data *pd,
                                                             Eina_Bool match_content_w,
                                                             Eina_Bool match_content_h)
{
   pd->match_content_w = !!match_content_w;
   pd->match_content_h = !!match_content_h;

   efl_ui_scrollable_match_content_set(pd->smanager, match_content_w, match_content_h);

   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_grid_efl_ui_multi_selectable_select_mode_set(Eo *obj EINA_UNUSED,
                                                     Efl_Ui_Grid_Data *pd,
                                                     Efl_Ui_Select_Mode mode)
{
   Efl_Ui_Grid_Item *selected;

   if ((pd->select_mode == EFL_UI_SELECT_MODE_MULTI &&
        mode != EFL_UI_SELECT_MODE_MULTI) ||
       mode == EFL_UI_SELECT_MODE_NONE)
     {
        Eina_List *clone = eina_list_clone(pd->selected);
        EINA_LIST_FREE(clone, selected)
          efl_ui_item_selected_set(selected, EINA_FALSE);
     }

   pd->select_mode = mode;
}

EOLIAN static Efl_Ui_Select_Mode
_efl_ui_grid_efl_ui_multi_selectable_select_mode_get(const Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->select_mode;
}

/* List APIs */
EOLIAN static void
_efl_ui_grid_item_scroll(Eo *obj,
                         Efl_Ui_Grid_Data *pd,
                         Efl_Ui_Grid_Item *item,
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
_efl_ui_grid_item_scroll_align(Eo *obj,
                               Efl_Ui_Grid_Data *pd,
                               Efl_Ui_Grid_Item *item,
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

EOLIAN static Efl_Ui_Grid_Item *
_efl_ui_grid_last_selected_item_get(const Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->last_selected;
}

EOLIAN static Eina_Iterator *
_efl_ui_grid_selected_items_get(Eo *obj, Efl_Ui_Grid_Data *pd)
{
   return _grid_item_iterator_new(obj, pd->selected);
}

EOLIAN static void
_efl_ui_grid_item_size_set(Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd, Eina_Size2D size)
{
   pd->item.size = size;
   //reset linemax for recalculate layout.
   pd->linemax = 0;

   _need_update(pd);
}

EOLIAN static Eina_Size2D
_efl_ui_grid_item_size_get(const Eo *obj EINA_UNUSED, Efl_Ui_Grid_Data *pd)
{
   return pd->item.size;
}

/* Internal EO APIs and hidden overrides */
#define EFL_UI_GRID_EXTRA_OPS \
  ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_grid)

#include "efl_ui_grid.eo.c"
