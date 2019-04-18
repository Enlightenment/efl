#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_POPUP_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_private.h"
#include "efl_ui_anchor_popup_private.h"

#define MY_CLASS EFL_UI_ANCHOR_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Anchor_Popup"

static void
_anchor_calc(Eo *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, ppd);
   EFL_UI_ANCHOR_POPUP_DATA_GET(obj, pd);

   Eina_Position2D pos = {0, 0};

   Eina_Rect a_geom = efl_gfx_entity_geometry_get(pd->anchor);
   Eina_Rect o_geom = efl_gfx_entity_geometry_get(obj);
   Eina_Rect p_geom = efl_gfx_entity_geometry_get(ppd->win_parent);

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
          cur_align = ppd->align;
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
          cur_align = ppd->align;
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
     efl_gfx_entity_position_set(efl_super(obj, EFL_UI_POPUP_CLASS), pos);
}

static void
_anchor_geom_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _anchor_calc(data);
}

static void
_anchor_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_POPUP_DATA_GET_OR_RETURN(data, ppd);
   EFL_UI_ANCHOR_POPUP_DATA_GET(data, pd);

   efl_event_callback_del(ppd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, data);

   pd->anchor = NULL;
   //Add align calc only
   Eina_Bool needs_size_calc = ppd->needs_size_calc;
   elm_layout_sizing_eval(data);
   ppd->needs_size_calc = needs_size_calc;
}

static void
_anchor_detach(Eo *obj)
{
   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, ppd);
   EFL_UI_ANCHOR_POPUP_DATA_GET(obj, pd);

   if (!pd->anchor) return;

   efl_event_callback_del(ppd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
   efl_event_callback_del(pd->anchor, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
   efl_event_callback_del(pd->anchor, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _anchor_geom_cb, obj);
   efl_event_callback_del(pd->anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
}

EOLIAN static void
_efl_ui_anchor_popup_anchor_set(Eo *obj, Efl_Ui_Anchor_Popup_Data *pd, Eo *anchor)
{
   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, ppd);

   _anchor_detach(obj);
   pd->anchor = anchor;

   if (anchor)
     {
        efl_event_callback_add(ppd->win_parent, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
     }

   //Add align/anchor calc only
   Eina_Bool needs_size_calc = ppd->needs_size_calc;
   elm_layout_sizing_eval(obj);
   ppd->needs_size_calc = needs_size_calc;
}

EOLIAN static Efl_Object *
_efl_ui_anchor_popup_anchor_get(const Eo *obj EINA_UNUSED, Efl_Ui_Anchor_Popup_Data *pd)
{
   return pd->anchor;
}

EOLIAN static void
_efl_ui_anchor_popup_align_priority_set(Eo *obj EINA_UNUSED,
                                        Efl_Ui_Anchor_Popup_Data *pd,
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
_efl_ui_anchor_popup_align_priority_get(const Eo *obj EINA_UNUSED,
                                        Efl_Ui_Anchor_Popup_Data *pd,
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

EOLIAN static void
_efl_ui_anchor_popup_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Anchor_Popup_Data *pd, Eina_Position2D pos)
{
   _anchor_detach(obj);

   pd->anchor = NULL;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
}

EOLIAN static void
_efl_ui_anchor_popup_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Anchor_Popup_Data *pd)
{
   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, ppd);
   /* When elm_layout_sizing_eval() is called, just flag is set instead of size
    * calculation.
    * The actual size calculation is done here when the object is rendered to
    * avoid duplicate size calculations. */
   if (ppd->needs_group_calc)
     {
        if (pd->anchor)
          ppd->needs_align_calc = EINA_FALSE;

        efl_canvas_group_calculate(efl_super(obj, MY_CLASS));

        if (pd->anchor)
          _anchor_calc(obj);
     }
}

EOLIAN static Eo *
_efl_ui_anchor_popup_efl_object_constructor(Eo *obj,
                                            Efl_Ui_Anchor_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "anchor_popup");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   pd->priority[0] = EFL_UI_POPUP_ALIGN_TOP;
   pd->priority[1] = EFL_UI_POPUP_ALIGN_LEFT;
   pd->priority[2] = EFL_UI_POPUP_ALIGN_RIGHT;
   pd->priority[3] = EFL_UI_POPUP_ALIGN_BOTTOM;
   pd->priority[4] = EFL_UI_POPUP_ALIGN_CENTER;

   return obj;
}

EOLIAN static void
_efl_ui_anchor_popup_efl_object_destructor(Eo *obj,
                                           Efl_Ui_Anchor_Popup_Data *pd EINA_UNUSED)
{
   _anchor_detach(obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "efl_ui_anchor_popup.eo.c"
