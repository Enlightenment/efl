#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED
#define EFL_UI_POPUP_PROTECTED
#define EFL_PART_PROTECTED
#define EFL_UI_WIDGET_SCROLLABLE_CONTENT_PROTECTED
#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_private.h"
#include "efl_ui_popup_part_backwall.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup"

static const char PART_NAME_BACKWALL[] = "backwall";

static void
_calc_align(Eo *obj)
{
   Efl_Ui_Popup_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

   Eina_Rect o_geom = efl_gfx_entity_geometry_get(obj);

   Evas_Coord pw, ph;
   pw = p_geom.w;
   ph = p_geom.h;

   Evas_Coord ow, oh;
   ow = o_geom.w;
   oh = o_geom.h;

   switch (pd->align)
     {
      case EFL_UI_POPUP_ALIGN_CENTER:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D((pw - ow ) / 2, (ph - oh) / 2));
         break;
      case EFL_UI_POPUP_ALIGN_LEFT:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D(0, (ph - oh) / 2));
         break;
      case EFL_UI_POPUP_ALIGN_RIGHT:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D(pw - ow, (ph - oh) / 2));
         break;
      case EFL_UI_POPUP_ALIGN_TOP:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D((pw - ow) / 2, 0));
         break;
      case EFL_UI_POPUP_ALIGN_BOTTOM:
         efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), EINA_POSITION2D((pw - ow) / 2, ph - oh));
         break;
      default:
         break;
     }
}

static void
_anchor_calc(Eo *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, pd);

   if (!pd->anchor) return;

   Eina_Position2D pos = {0, 0};

   Eina_Rect a_geom = efl_gfx_entity_geometry_get(pd->anchor);
   Eina_Rect o_geom = efl_gfx_entity_geometry_get(obj);
   Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

   pd->used_align = EFL_UI_POPUP_ALIGN_NONE;

   /* 1. Find align which display popup.
         It enables to shifting popup from exact position.
         LEFT, RIGHT - shift only y position within anchor object's height
         TOP, BOTTOM - shift only x position within anchor object's width
         CENTER - shift both x, y position within anchor object's area
    */

   for (int idx = 0; idx < 6; idx++)
     {
        Efl_Ui_Popup_Align cur_align;

        if (idx == 0)
          cur_align = pd->align;
        else
          cur_align = pd->priority[idx - 1];

        if (cur_align == EFL_UI_POPUP_ALIGN_NONE)
          continue;

        switch(cur_align)
          {
           case EFL_UI_POPUP_ALIGN_TOP:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = (a_geom.y - o_geom.h);

              if ((pos.y < 0) ||
                  ((pos.y + o_geom.h) > p_geom.h) ||
                  (o_geom.w > p_geom.w))
                continue;
              break;

           case EFL_UI_POPUP_ALIGN_LEFT:
              pos.x = (a_geom.x - o_geom.w);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);

              if ((pos.x < 0) ||
                  ((pos.x + o_geom.w) > p_geom.w) ||
                  (o_geom.h > p_geom.h))
                continue;
              break;

           case EFL_UI_POPUP_ALIGN_RIGHT:
              pos.x = (a_geom.x + a_geom.w);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);

              if ((pos.x < 0) ||
                  ((pos.x + o_geom.w) > p_geom.w) ||
                  (o_geom.h > p_geom.h))
                continue;
              break;

           case EFL_UI_POPUP_ALIGN_BOTTOM:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = (a_geom.y + a_geom.h);

              if ((pos.y < 0) ||
                  ((pos.y + o_geom.h) > p_geom.h) ||
                  (o_geom.w > p_geom.w))
                continue;
              break;

           case EFL_UI_POPUP_ALIGN_CENTER:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);

              if ((o_geom.w > p_geom.w) || (o_geom.h > p_geom.h))
                continue;
              break;

           default:
              continue;
          }

        if ((cur_align == EFL_UI_POPUP_ALIGN_TOP) ||
            (cur_align == EFL_UI_POPUP_ALIGN_BOTTOM) ||
            (cur_align == EFL_UI_POPUP_ALIGN_CENTER))
          {
             if (pos.x < 0)
               pos.x = 0;
             if ((pos.x + o_geom.w) > p_geom.w)
               pos.x = p_geom.w - o_geom.w;

             if ((pos.x > (a_geom.x + a_geom.w)) ||
                 ((pos.x + o_geom.w) < a_geom.x))
               continue;
          }

        if ((cur_align == EFL_UI_POPUP_ALIGN_LEFT) ||
            (cur_align == EFL_UI_POPUP_ALIGN_RIGHT) ||
            (cur_align == EFL_UI_POPUP_ALIGN_CENTER))
          {
             if (pos.y < 0)
               pos.y = 0;
             if ((pos.y + o_geom.h) > p_geom.h)
               pos.y = p_geom.h - o_geom.h;

             if ((pos.y > (a_geom.y + a_geom.h)) ||
                 ((pos.y + o_geom.h) < a_geom.y))
               continue;
          }

        pd->used_align = cur_align;
        goto end;
     }

   /* 2. Move popup to fit first valid align although entire popup can't display */

   for (int idx = 0; idx < 6; idx++)
     {
        Efl_Ui_Popup_Align cur_align;

        if (idx == 0)
          cur_align = pd->align;
        else
          cur_align = pd->priority[idx - 1];

        if (cur_align == EFL_UI_POPUP_ALIGN_NONE)
          continue;

        switch(cur_align)
          {
           case EFL_UI_POPUP_ALIGN_TOP:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = (a_geom.y - o_geom.h);
              pd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_LEFT:
              pos.x = (a_geom.x - o_geom.w);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);
              pd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_RIGHT:
              pos.x = (a_geom.x + a_geom.w);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);
              pd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_BOTTOM:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = (a_geom.y + a_geom.h);
              pd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_CENTER:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);
              pd->used_align = cur_align;
              goto end;
              break;

           default:
              break;
          }
     }

end:
   if (pd->used_align != EFL_UI_POPUP_ALIGN_NONE)
     efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
}

static void
_anchor_geom_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _anchor_calc(data);
}

static void
_anchor_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_POPUP_DATA_GET_OR_RETURN(data, pd);

   efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, data);

   pd->anchor = NULL;
   _anchor_calc(data);
}

static void
_anchor_detach(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   if (!pd->anchor) return;

   efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
   efl_event_callback_del(pd->anchor, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
   efl_event_callback_del(pd->anchor, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _anchor_geom_cb, obj);
   efl_event_callback_del(pd->anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
}

EOLIAN static void
_efl_ui_popup_anchor_set(Eo *obj, Efl_Ui_Popup_Data *pd, Eo *anchor)
{
   _anchor_detach(obj, pd);
   pd->anchor = anchor;

   if (anchor)
     {
        efl_event_callback_add(pd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
        _anchor_calc(obj);
     }
   else
     _calc_align(obj);
}

EOLIAN static Efl_Object *
_efl_ui_popup_anchor_get(const Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
   return pd->anchor;
}

EOLIAN static void
_efl_ui_popup_align_priority_set(Eo *obj EINA_UNUSED,
                                        Efl_Ui_Popup_Data *pd,
                                        Efl_Ui_Popup_Align first,
                                        Efl_Ui_Popup_Align second,
                                        Efl_Ui_Popup_Align third,
                                        Efl_Ui_Popup_Align fourth,
                                        Efl_Ui_Popup_Align fifth)
{
   pd->priority[0] = first;
   pd->priority[1] = second;
   pd->priority[2] = third;
   pd->priority[3] = fourth;
   pd->priority[4] = fifth;
}

EOLIAN static void
_efl_ui_popup_align_priority_get(const Eo *obj EINA_UNUSED,
                                        Efl_Ui_Popup_Data *pd,
                                        Efl_Ui_Popup_Align *first,
                                        Efl_Ui_Popup_Align *second,
                                        Efl_Ui_Popup_Align *third,
                                        Efl_Ui_Popup_Align *fourth,
                                        Efl_Ui_Popup_Align *fifth)
{
   if (first) *first = pd->priority[0];
   if (second) *second = pd->priority[1];
   if (third) *third = pd->priority[2];
   if (fourth) *fourth = pd->priority[3];
   if (fifth) *fifth = pd->priority[4];
}


static void
_backwall_clicked_cb(void *data,
                     Eo *o EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Eo *obj = data;
   efl_event_callback_call(obj, EFL_UI_POPUP_EVENT_BACKWALL_CLICKED, NULL);
}

EOLIAN static void
_efl_ui_popup_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Popup_Data *pd, Eina_Position2D pos)
{
   pd->align = EFL_UI_POPUP_ALIGN_NONE;
   _anchor_detach(obj, pd);

   pd->anchor = NULL;
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
}

EOLIAN static void
_efl_ui_popup_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Popup_Data *pd, Eina_Size2D size)
{
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), size);

   if (!pd->in_calc)
     efl_canvas_group_change(obj);
}

static void
_parent_geom_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *obj = data;

   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, pd);

   efl_canvas_group_change(obj);
}

static void
_hints_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Popup_Data *pd = data;

   if (!pd->in_calc)
     efl_canvas_group_change(ev->object);
}

EOLIAN static void
_efl_ui_popup_efl_ui_widget_widget_parent_set(Eo *obj, Efl_Ui_Popup_Data *pd, Eo *parent)
{
   if (!parent)
     {
        /* unsetting parent, probably before deletion */
        if (pd->win_parent)
          {
             efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _parent_geom_cb, obj);
             efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _parent_geom_cb, obj);
          }
        goto end;
     }
   pd->win_parent = efl_provider_find(obj, EFL_UI_WIN_CLASS);
   if (!pd->win_parent)
     {
        ERR("Cannot find window parent");
        return;
     }

   Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

   efl_gfx_entity_position_set(pd->backwall, EINA_POSITION2D(p_geom.x, p_geom.y));
   efl_gfx_entity_size_set(pd->backwall, EINA_SIZE2D(p_geom.w, p_geom.h));

   efl_event_callback_add(pd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _parent_geom_cb, obj);
   efl_event_callback_add(pd->win_parent, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _parent_geom_cb, obj);
end:
   efl_ui_widget_parent_set(efl_super(obj, MY_CLASS), parent);
}

EOLIAN static void
_efl_ui_popup_align_set(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd, Efl_Ui_Popup_Align type)
{
   pd->align = type;

   efl_canvas_group_change(obj);
}

EOLIAN static Efl_Ui_Popup_Align
_efl_ui_popup_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
   return pd->align;
}

static Eina_Bool
_timer_cb(void *data)
{
   Eo *popup = data;
   efl_event_callback_call(popup, EFL_UI_POPUP_EVENT_TIMEOUT, NULL);
   efl_del(popup);

   return ECORE_CALLBACK_CANCEL;
}

static void
_timer_del(Efl_Ui_Popup_Data *pd)
{
   if (pd->timer)
     {
        ecore_timer_del(pd->timer);
        pd->timer = NULL;
     }
}

static void
_timer_init(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   if (pd->timeout > 0.0)
     pd->timer = ecore_timer_add(pd->timeout, _timer_cb, obj);
}

EOLIAN static void
_efl_ui_popup_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Popup_Data *pd, Eina_Bool v)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, v))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), v);

   if (v)
     {
        _timer_del(pd);
        _timer_init(obj, pd);
     }
}

EOLIAN static void
_efl_ui_popup_closing_timeout_set(Eo *obj, Efl_Ui_Popup_Data *pd, double time)
{
   if (time < 0.0)
     time = 0.0;

   pd->timeout = time;

   _timer_del(pd);

   if (efl_gfx_entity_visible_get(obj))
     _timer_init(obj, pd);
}

EOLIAN static double
_efl_ui_popup_closing_timeout_get(const Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
   return pd->timeout;
}

/* this will ONLY be called during _sizing_eval() */
static void
_scrollable_content_size_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;

   efl_gfx_entity_size_set(ev->object, *size);
   /* finish group calc chain */
   efl_canvas_group_calculate(efl_super(ev->object, EFL_UI_WIDGET_SCROLLABLE_CONTENT_MIXIN));
}

static void
_sizing_eval(Eo *obj)
{
   Eina_Size2D min;

   /* trigger layout calc */
   efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
   if (efl_ui_widget_scrollable_content_did_group_calc_get(obj)) return;
   min = efl_gfx_hint_size_combined_min_get(obj);

   Eina_Size2D size = efl_gfx_entity_size_get(obj);

   Eina_Size2D new_size;
   new_size.w = (min.w > size.w ? min.w : size.w);
   new_size.h = (min.h > size.h ? min.h : size.h);
   efl_gfx_entity_size_set(obj, new_size);
}

EOLIAN static void
_efl_ui_popup_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   /* When efl_canvas_group_change() is called, just flag is set instead of size
    * calculation.
    * The actual size calculation is done here when the object is rendered to
    * avoid duplicate size calculations. */
   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
   pd->in_calc = EINA_TRUE;
   _sizing_eval(obj);
   pd->in_calc = EINA_FALSE;
   if (pd->anchor)
     _anchor_calc(obj);
   else
     _calc_align(obj);

   Eina_Rect p_geom = efl_gfx_entity_geometry_get(pd->win_parent);

   efl_gfx_entity_position_set(pd->backwall, EINA_POSITION2D(0, 0));
   efl_gfx_entity_size_set(pd->backwall, EINA_SIZE2D(p_geom.w, p_geom.h));
}

EOLIAN static Eo *
_efl_ui_popup_efl_object_constructor(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "popup");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _hints_changed_cb, pd);
   efl_event_callback_add(obj, EFL_UI_WIDGET_SCROLLABLE_CONTENT_EVENT_OPTIMAL_SIZE_CALC, _scrollable_content_size_cb, pd);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   pd->backwall = edje_object_add(evas_object_evas_get(obj));
   elm_widget_element_update(obj, pd->backwall, PART_NAME_BACKWALL);
   evas_object_smart_member_add(pd->backwall, obj);
   evas_object_stack_below(pd->backwall, wd->resize_obj);

   edje_object_signal_callback_add(pd->backwall, "efl,action,click", "*",
                                   _backwall_clicked_cb, obj);

   pd->align = EFL_UI_POPUP_ALIGN_CENTER;

   pd->priority[0] = EFL_UI_POPUP_ALIGN_TOP;
   pd->priority[1] = EFL_UI_POPUP_ALIGN_LEFT;
   pd->priority[2] = EFL_UI_POPUP_ALIGN_RIGHT;
   pd->priority[3] = EFL_UI_POPUP_ALIGN_BOTTOM;
   pd->priority[4] = EFL_UI_POPUP_ALIGN_CENTER;
   return obj;
}

EOLIAN static void
_efl_ui_popup_efl_object_invalidate(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   ELM_SAFE_DEL(pd->backwall);
   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_popup_efl_object_destructor(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   _anchor_detach(obj, pd);

   efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _parent_geom_cb,
                          obj);
   efl_event_callback_del(pd->win_parent, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _parent_geom_cb,
                          obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

/* Standard widget overrides */

ELM_PART_CONTENT_DEFAULT_IMPLEMENT(efl_ui_popup, Efl_Ui_Popup_Data)

EOLIAN static Eo *
_efl_ui_popup_efl_part_part_get(const Eo *obj, Efl_Ui_Popup_Data *_pd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (eina_streq(part, PART_NAME_BACKWALL))
     return ELM_PART_IMPLEMENT(EFL_UI_POPUP_PART_BACKWALL_CLASS, obj, part);

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

EOLIAN static void
_efl_ui_popup_part_backwall_repeat_events_set(Eo *obj, void *_pd EINA_UNUSED, Eina_Bool repeat)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   efl_canvas_object_repeat_events_set(sd->backwall, repeat);
}

EOLIAN static Eina_Bool
_efl_ui_popup_part_backwall_repeat_events_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   return efl_canvas_object_repeat_events_get(sd->backwall);
}

EOLIAN static Eina_Stringshare *
_efl_ui_popup_part_backwall_efl_file_file_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   Eo *content = edje_object_part_swallow_get(sd->backwall, "efl.content");
   return content ? efl_file_get(content) : NULL;
}

EOLIAN static Eina_Stringshare *
_efl_ui_popup_part_backwall_efl_file_key_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   Eo *content = edje_object_part_swallow_get(sd->backwall, "efl.content");
   return content ? efl_file_key_get(content) : NULL;
}

EOLIAN static void
_efl_ui_popup_part_backwall_efl_file_unload(Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   efl_file_unload(efl_super(obj, EFL_UI_POPUP_PART_BACKWALL_CLASS));
   Eo *prev_obj = edje_object_part_swallow_get(sd->backwall, "efl.content");
   if (prev_obj)
     {
        edje_object_signal_emit(sd->backwall, "efl,state,content,unset", "efl");
        edje_object_part_unswallow(sd->backwall, prev_obj);
        efl_del(prev_obj);
     }
}

EOLIAN static Eina_Error
_efl_ui_popup_part_backwall_efl_file_load(Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Efl_Ui_Popup_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_POPUP_CLASS);

   Eina_Error err;

   if (efl_file_loaded_get(obj)) return 0;

   err = efl_file_load(efl_super(obj, EFL_UI_POPUP_PART_BACKWALL_CLASS));
   if (err) return err;

   Eo *prev_obj = edje_object_part_swallow_get(sd->backwall, "efl.content");
   if (prev_obj)
     {
        edje_object_signal_emit(sd->backwall, "efl,state,content,unset", "efl");
        edje_object_part_unswallow(sd->backwall, prev_obj);
        efl_del(prev_obj);
     }

   Eo *image = efl_add(EFL_UI_IMAGE_CLASS, pd->obj);
   Eina_Bool ret;
   const Eina_File *f;

   f = efl_file_mmap_get(obj);
   if (f)
     ret = efl_file_simple_mmap_load(image, f, efl_file_key_get(obj));
   else
     ret = efl_file_simple_load(image, efl_file_get(obj), efl_file_key_get(obj));
   if (!ret)
     {
        efl_del(image);
        return EFL_GFX_IMAGE_LOAD_ERROR_GENERIC;
     }
   edje_object_part_swallow(sd->backwall, "efl.content", image);
   edje_object_signal_emit(sd->backwall, "efl,state,content,set", "efl");

   return 0;
}

#include "efl_ui_popup_part_backwall.eo.c"

/* Efl.Part end */

#include "efl_ui_popup.eo.c"
