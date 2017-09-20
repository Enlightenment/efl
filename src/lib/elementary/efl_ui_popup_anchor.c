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
_anchor_calc(Evas_Object *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   EFL_UI_POPUP_ANCHOR_DATA_GET(obj, sd);

   Evas_Coord_Rectangle anchor_geom = {0, 0, 0, 0};
   Evas_Coord_Size popup_size = {0, 0};
   Evas_Coord_Size parent_size = {0, 0};
   Eina_Position2D pos = {0, 0};

   Evas_Object *parent = efl_ui_popup_parent_window_get(obj);

   evas_object_geometry_get(sd->anchor, &anchor_geom.x, &anchor_geom.y, &anchor_geom.w, &anchor_geom.h);
   evas_object_geometry_get(obj, NULL, NULL, &popup_size.w, &popup_size.h);
   evas_object_geometry_get(parent, NULL, NULL, &parent_size.w, &parent_size.h);

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
               pos.x = anchor_geom.x + ((anchor_geom.w - popup_size.w) / 2);
               pos.y = (anchor_geom.y - popup_size.h);

               if ((pos.y < 0) ||
                   ((pos.y + popup_size.h) > parent_size.h) ||
                   (popup_size.w > parent_size.w))
                 continue;
               break;

             case EFL_UI_POPUP_ALIGN_LEFT:
               pos.x = (anchor_geom.x - popup_size.w);
               pos.y = anchor_geom.y + ((anchor_geom.h - popup_size.h) / 2);

               if ((pos.x < 0) ||
                   ((pos.x + popup_size.w) > parent_size.w) ||
                   (popup_size.h > parent_size.h))
                 continue;
               break;

             case EFL_UI_POPUP_ALIGN_RIGHT:
               pos.x = (anchor_geom.x + anchor_geom.w);
               pos.y = anchor_geom.y + ((anchor_geom.h - popup_size.h) / 2);

               if ((pos.x < 0) ||
                   ((pos.x + popup_size.w) > parent_size.w) ||
                   (popup_size.h > parent_size.h))
                 continue;
               break;

             case EFL_UI_POPUP_ALIGN_BOTTOM:
               pos.x = anchor_geom.x + ((anchor_geom.w - popup_size.w) / 2);
               pos.y = (anchor_geom.y + anchor_geom.h);

               if ((pos.y < 0) ||
                   ((pos.y + popup_size.h) > parent_size.h) ||
                   (popup_size.w > parent_size.w))
                 continue;
               break;

             case EFL_UI_POPUP_ALIGN_CENTER:
               pos.x = anchor_geom.x + ((anchor_geom.w - popup_size.w) / 2);
               pos.y = anchor_geom.y + ((anchor_geom.h - popup_size.h) / 2);

               if (popup_size.w > parent_size.w || popup_size.h > parent_size.h)
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
             if ((pos.x + popup_size.w) > parent_size.w)
               pos.x = parent_size.w - popup_size.w;

             if ((pos.x > (anchor_geom.x + anchor_geom.w)) ||
                 ((pos.x + popup_size.w) < anchor_geom.x))
               continue;
          }

        if ((cur_align == EFL_UI_POPUP_ALIGN_LEFT) ||
            (cur_align == EFL_UI_POPUP_ALIGN_RIGHT) ||
            (cur_align == EFL_UI_POPUP_ALIGN_CENTER))
          {
             if (pos.y < 0)
               pos.y = 0;
             if ((pos.y + popup_size.h) > parent_size.h)
               pos.y = parent_size.h - popup_size.h;

             if ((pos.y > (anchor_geom.y + anchor_geom.h)) ||
                 ((pos.y + popup_size.h) < anchor_geom.y))
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
               pos.x = anchor_geom.x + ((anchor_geom.w - popup_size.w) / 2);
               pos.y = (anchor_geom.y - popup_size.h);
               sd->used_align = cur_align;
               goto end;
               break;

             case EFL_UI_POPUP_ALIGN_LEFT:
               pos.x = (anchor_geom.x - popup_size.w);
               pos.y = anchor_geom.y + ((anchor_geom.h - popup_size.h) / 2);
               sd->used_align = cur_align;
               goto end;
               break;

             case EFL_UI_POPUP_ALIGN_RIGHT:
               pos.x = (anchor_geom.x + anchor_geom.w);
               pos.y = anchor_geom.y + ((anchor_geom.h - popup_size.h) / 2);
               sd->used_align = cur_align;
               goto end;
               break;

             case EFL_UI_POPUP_ALIGN_BOTTOM:
               pos.x = anchor_geom.x + ((anchor_geom.w - popup_size.w) / 2);
               pos.y = (anchor_geom.y + anchor_geom.h);
               sd->used_align = cur_align;
               goto end;
               break;

             case EFL_UI_POPUP_ALIGN_CENTER:
               pos.x = anchor_geom.x + ((anchor_geom.w - popup_size.w) / 2);
               pos.y = anchor_geom.y + ((anchor_geom.h - popup_size.h) / 2);
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

   efl_event_callback_del(efl_ui_popup_parent_window_get(data), EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, data);

   sd->anchor = NULL;
   efl_ui_popup_align_set(efl_super(data, MY_CLASS), sd->align);
}

static void
_anchor_detach(Evas_Object *obj)
{
   EFL_UI_POPUP_ANCHOR_DATA_GET(obj, sd);

   if (sd->anchor == NULL) return;

   efl_event_callback_del(efl_ui_popup_parent_window_get(obj), EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
   efl_event_callback_del(sd->anchor, EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
   efl_event_callback_del(sd->anchor, EFL_GFX_EVENT_MOVE, _anchor_geom_cb, obj);
   efl_event_callback_del(sd->anchor, EFL_EVENT_DEL, _anchor_del_cb, obj);
}

EOLIAN static void
_efl_ui_popup_anchor_anchor_set(Eo *obj, Efl_Ui_Popup_Anchor_Data *pd, Evas_Object *anchor)
{
   _anchor_detach(obj);
   pd->anchor = anchor;

   if (anchor == NULL)
      efl_ui_popup_align_set(efl_super(obj, MY_CLASS), pd->align);
   else
     {
        efl_ui_popup_align_set(efl_super(obj, MY_CLASS), EFL_UI_POPUP_ALIGN_NONE);

        _anchor_calc(obj);

        efl_event_callback_add(efl_ui_popup_parent_window_get(obj), EFL_GFX_EVENT_RESIZE, _anchor_geom_cb, obj);
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

EOLIAN static void
_efl_ui_popup_anchor_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

#define EFL_UI_POPUP_ANCHOR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_popup_anchor), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_anchor)

#include "efl_ui_popup_anchor.eo.c"
