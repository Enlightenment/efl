#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_POPUP_PROTECTED
#define EFL_UI_POPUP_ANCHOR_BETA

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_anchor_private.h"

#define MY_CLASS EFL_UI_POPUP_ANCHOR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup.Anchor"

static void
_anchor_calc(Eo *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   EFL_UI_POPUP_ANCHOR_DATA_GET(obj, sd);

   Eina_Position2D pos = {0, 0};

   Eo *parent = efl_provider_find(obj, EFL_UI_WIN_CLASS);
   if (!parent)
     {
        ERR("Cannot find window parent");
        return;
     }

   Eina_Rect a_geom = efl_gfx_geometry_get(sd->anchor);
   Eina_Rect o_geom = efl_gfx_geometry_get(obj);
   Eina_Rect p_geom = efl_gfx_geometry_get(parent);

   sd->used_align = EFL_UI_POPUP_ALIGN_NONE;

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
          cur_align = sd->align;
        else
          cur_align = sd->priority[idx - 1];

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

        sd->used_align = cur_align;
        goto end;
     }

   /* 2. Move popup to fit first valid align although entire popup can't display */

   for (int idx = 0; idx < 6; idx++)
     {
        Efl_Ui_Popup_Align cur_align;

        if (idx == 0)
          cur_align = sd->align;
        else
          cur_align = sd->priority[idx - 1];

        if (cur_align == EFL_UI_POPUP_ALIGN_NONE)
          continue;

        switch(cur_align)
          {
           case EFL_UI_POPUP_ALIGN_TOP:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = (a_geom.y - o_geom.h);
              sd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_LEFT:
              pos.x = (a_geom.x - o_geom.w);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);
              sd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_RIGHT:
              pos.x = (a_geom.x + a_geom.w);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);
              sd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_BOTTOM:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = (a_geom.y + a_geom.h);
              sd->used_align = cur_align;
              goto end;
              break;

           case EFL_UI_POPUP_ALIGN_CENTER:
              pos.x = a_geom.x + ((a_geom.w - o_geom.w) / 2);
              pos.y = a_geom.y + ((a_geom.h - o_geom.h) / 2);
              sd->used_align = cur_align;
              goto end;
              break;

           default:
              break;
          }
     }

end:
   if (sd->used_align != EFL_UI_POPUP_ALIGN_NONE)
     efl_gfx_position_set(efl_super(obj, EFL_UI_POPUP_CLASS), pos);
}

static void
_anchor_geom_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   _anchor_calc(data);
}

static void
_anchor_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   EFL_UI_POPUP_ANCHOR_DATA_GET(data, sd);

   Eo *parent = efl_provider_find(data, EFL_UI_WIN_CLASS);
   if (!parent)
     {
        ERR("Cannot find window parent");
        return;
     }

   efl_event_callback_del(parent, EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, data);

   sd->anchor = NULL;
   efl_ui_popup_align_set(efl_super(data, MY_CLASS), sd->align);
}

static void
_anchor_detach(Eo *obj)
{
   EFL_UI_POPUP_ANCHOR_DATA_GET(obj, sd);

   if (sd->anchor == NULL) return;

   Eo *parent = efl_provider_find(obj, EFL_UI_WIN_CLASS);
   if (!parent)
     {
        ERR("Cannot find window parent");
        return;
     }

   efl_event_callback_del(parent, EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
   efl_event_callback_del(sd->anchor, EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
   efl_event_callback_del(sd->anchor, EFL_GFX_EVENT_MOVE, _anchor_geom_cb, obj);
   efl_event_callback_del(sd->anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
}

EOLIAN static void
_efl_ui_popup_anchor_anchor_set(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd, Eo *anchor)
{
   _anchor_detach(obj);
   pd->anchor = anchor;

   if (anchor == NULL)
     efl_ui_popup_align_set(efl_super(obj, MY_CLASS), pd->align);
   else
     {
        efl_ui_popup_align_set(efl_super(obj, MY_CLASS), EFL_UI_POPUP_ALIGN_NONE);

        _anchor_calc(obj);

        Eo *parent = efl_provider_find(obj, EFL_UI_WIN_CLASS);
        if (!parent)
          {
              ERR("Cannot find window parent");
              return;
          }

        efl_event_callback_add(parent, EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_GFX_EVENT_MOVE, _anchor_geom_cb, obj);
        efl_event_callback_add(anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
     }
}

EOLIAN static Efl_Object *
_efl_ui_popup_anchor_anchor_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Anchor_Data *pd)
{
   return pd->anchor;
}

EOLIAN static void
_efl_ui_popup_anchor_align_priority_set(Eo *obj EINA_UNUSED,
                                        Efl_Ui_Popup_Anchor_Data *pd,
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
_efl_ui_popup_anchor_align_priority_get(Eo *obj EINA_UNUSED,
                                        Efl_Ui_Popup_Anchor_Data *pd,
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
_efl_ui_popup_anchor_efl_gfx_position_set(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd EINA_UNUSED, Eina_Position2D pos)
{
   _anchor_detach(obj);

   pd->anchor = NULL;
   pd->align = EFL_UI_POPUP_ALIGN_NONE;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), pos);
}

EOLIAN static void
_efl_ui_popup_anchor_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd EINA_UNUSED)
{
   elm_layout_sizing_eval(efl_super(obj, MY_CLASS));

   if (pd->anchor != NULL)
     _anchor_calc(obj);
}

EOLIAN static void
_efl_ui_popup_anchor_efl_ui_popup_align_set(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd, Efl_Ui_Popup_Align type)
{
   pd->align = type;
   if (pd->anchor == NULL)
     efl_ui_popup_align_set(efl_super(obj, MY_CLASS), type);
   else
     _anchor_calc(obj);
}

EOLIAN static Efl_Ui_Popup_Align
_efl_ui_popup_anchor_efl_ui_popup_align_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Anchor_Data *pd)
{
   return pd->align;
}

EOLIAN static void
_efl_ui_popup_anchor_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->priority[0] = EFL_UI_POPUP_ALIGN_TOP;
   pd->priority[1] = EFL_UI_POPUP_ALIGN_LEFT;
   pd->priority[2] = EFL_UI_POPUP_ALIGN_RIGHT;
   pd->priority[3] = EFL_UI_POPUP_ALIGN_BOTTOM;
   pd->priority[4] = EFL_UI_POPUP_ALIGN_CENTER;
}

EOLIAN static void
_efl_ui_popup_anchor_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

#define EFL_UI_POPUP_ANCHOR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_popup_anchor), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_anchor)

#include "efl_ui_popup_anchor.eo.c"
