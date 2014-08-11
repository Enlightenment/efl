#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_ctxpopup.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#include "elm_interface_atspi_widget_action.h"
#include "elm_interface_atspi_widget_action.eo.h"

#define MY_CLASS ELM_CTXPOPUP_CLASS

#define MY_CLASS_NAME "Elm_Ctxpopup"
#define MY_CLASS_NAME_LEGACY "elm_ctxpopup"

EAPI const char ELM_CTXPOPUP_SMART_NAME[] = "elm_ctxpopup";

#define ELM_PRIV_CTXPOPUP_SIGNALS(cmd) \
   cmd(SIG_DISMISSED, "dismissed", "") \

ELM_PRIV_CTXPOPUP_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_CTXPOPUP_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};
#undef ELM_PRIV_CTXPOPUP_SIGNALS

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params);
static Elm_Ctxpopup_Item* _item_new(Eo *obj, 
                                    Elm_Ctxpopup_Data *sd,
                                    Evas_Smart_Cb func,
                                    const void *data);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"escape", _key_action_escape},
   {NULL, NULL}
};

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_translate(Eo *obj, Elm_Ctxpopup_Data *sd)
{
   Eina_List *l;
   Elm_Ctxpopup_Item *it;

   if (sd->auto_hide) evas_object_hide(obj);

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate());

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_focus_next(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   if (!sd)
     return EINA_FALSE;

   if (!elm_widget_focus_next_get(sd->box, dir, next))
     {
        elm_widget_focused_object_clear(sd->box);
        elm_widget_focus_next_get(sd->box, dir, next);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_focus_direction(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd, const Evas_Object *base, double degree, Evas_Object **direction, double *weight)
{
   Eina_Bool int_ret;

   Eina_List *l = NULL;
   void *(*list_data_get)(const Eina_List *list);

   if (!sd)
     return EINA_FALSE;

   list_data_get = eina_list_data_get;

   l = eina_list_append(l, sd->box);

   int_ret = elm_widget_focus_list_direction_get
            (obj, base, l, list_data_get, degree, direction, weight);
   eina_list_free(l);

   return int_ret;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);
   const char *dir = params;

   if (!sd->box) return EINA_FALSE;

   if (!strcmp(dir, "previous"))
     elm_widget_focus_cycle(sd->box, ELM_FOCUS_PREVIOUS);
   else if (!strcmp(dir, "next"))
     elm_widget_focus_cycle(sd->box, ELM_FOCUS_NEXT);
   else if (!strcmp(dir, "left"))
     elm_widget_focus_cycle(sd->box, ELM_FOCUS_LEFT);
   else if (!strcmp(dir, "right"))
     elm_widget_focus_cycle(sd->box, ELM_FOCUS_RIGHT);
   else if (!strcmp(dir, "up"))
     elm_widget_focus_cycle(sd->box, ELM_FOCUS_UP);
   else if (!strcmp(dir, "down"))
     elm_widget_focus_cycle(sd->box, ELM_FOCUS_DOWN);
   else return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   elm_ctxpopup_dismiss(obj);
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_event(Eo *obj, Elm_Ctxpopup_Data *sd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   (void)src;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_x_pos_adjust(Evas_Coord_Point *pos,
              Evas_Coord_Point *base_size,
              Evas_Coord_Rectangle *hover_area)
{
   pos->x -= (base_size->x / 2);

   if (pos->x < hover_area->x)
     pos->x = hover_area->x;
   else if ((pos->x + base_size->x) > (hover_area->x + hover_area->w))
     pos->x = (hover_area->x + hover_area->w) - base_size->x;

   if (base_size->x > hover_area->w)
     base_size->x -= (base_size->x - hover_area->w);

   if (pos->x < hover_area->x)
     pos->x = hover_area->x;
}

static void
_y_pos_adjust(Evas_Coord_Point *pos,
              Evas_Coord_Point *base_size,
              Evas_Coord_Rectangle *hover_area)
{
   pos->y -= (base_size->y / 2);

   if (pos->y < hover_area->y)
     pos->y = hover_area->y;
   else if ((pos->y + base_size->y) > (hover_area->y + hover_area->h))
     pos->y = hover_area->y + hover_area->h - base_size->y;

   if (base_size->y > hover_area->h)
     base_size->y -= (base_size->y - hover_area->h);

   if (pos->y < hover_area->y)
     pos->y = hover_area->y;
}

static Elm_Ctxpopup_Direction
_base_geometry_calc(Evas_Object *obj,
                    Evas_Coord_Rectangle *rect)
{
   Elm_Ctxpopup_Direction dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   Evas_Coord_Rectangle hover_area;
   Evas_Coord_Point pos = {0, 0};
   Evas_Coord_Point arrow_size;
   Evas_Coord_Point base_size;
   Evas_Coord_Point max_size;
   Evas_Coord_Point min_size;
   Evas_Coord_Point temp;
   int idx;

   ELM_CTXPOPUP_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_CTXPOPUP_DIRECTION_DOWN);

   if (!rect || !sd->parent) return ELM_CTXPOPUP_DIRECTION_DOWN;

   edje_object_part_geometry_get(sd->arrow, "base", NULL, NULL,
                                 &arrow_size.x, &arrow_size.y);
   evas_object_resize(sd->arrow, arrow_size.x, arrow_size.y);

   //Initialize Area Rectangle.
   evas_object_geometry_get
     (sd->parent, &hover_area.x, &hover_area.y, &hover_area.w,
     &hover_area.h);
   if (sd->parent && eo_isa(sd->parent, ELM_WIN_CLASS))
     hover_area.x = hover_area.y = 0;

   evas_object_geometry_get(obj, &pos.x, &pos.y, NULL, NULL);

   //recalc the edje
   edje_object_size_min_calc
     (wd->resize_obj, &base_size.x, &base_size.y);
   evas_object_smart_calculate(wd->resize_obj);

   //Limit to Max Size
   evas_object_size_hint_max_get(obj, &max_size.x, &max_size.y);

   if ((max_size.y > 0) && (base_size.y > max_size.y))
     base_size.y = max_size.y;

   if ((max_size.x > 0) && (base_size.x > max_size.x))
     base_size.x = max_size.x;
   //Limit to Min Size
   evas_object_size_hint_min_get(obj, &min_size.x, &min_size.y);

   if ((min_size.y > 0) && (base_size.y < min_size.y))
     base_size.y = min_size.y;

   if ((min_size.x > 0) && (base_size.x < min_size.x))
     base_size.x = min_size.x;

   //Check available directions. If there is one, it adjusts ctxpopup's position
   // and size.
   for (idx = 0; idx < 4; idx++)
     {
        switch (sd->dir_priority[idx])
          {
           case ELM_CTXPOPUP_DIRECTION_UP:
             temp.y = (pos.y - base_size.y);
             if ((temp.y - arrow_size.y) < hover_area.y)
               continue;

             _x_pos_adjust(&pos, &base_size, &hover_area);
             pos.y -= base_size.y;
             dir = ELM_CTXPOPUP_DIRECTION_UP;
             break;

           case ELM_CTXPOPUP_DIRECTION_LEFT:
             temp.x = (pos.x - base_size.x);
             if ((temp.x - arrow_size.x) < hover_area.x)
               continue;

             _y_pos_adjust(&pos, &base_size, &hover_area);
             pos.x -= base_size.x;
             dir = ELM_CTXPOPUP_DIRECTION_LEFT;
             break;

           case ELM_CTXPOPUP_DIRECTION_RIGHT:
             temp.x = (pos.x + base_size.x);
             if ((temp.x + arrow_size.x) >
                 (hover_area.x + hover_area.w))
               continue;

             _y_pos_adjust(&pos, &base_size, &hover_area);
             dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
             break;

           case ELM_CTXPOPUP_DIRECTION_DOWN:
             temp.y = (pos.y + base_size.y);
             if ((temp.y + arrow_size.y) >
                 (hover_area.y + hover_area.h))
               continue;

             _x_pos_adjust(&pos, &base_size, &hover_area);
             dir = ELM_CTXPOPUP_DIRECTION_DOWN;
             break;

           default:
             continue;
          }
        break;
     }

   //In this case, all directions are invalid because of lack of space.
   if (idx == 4)
     {
        Evas_Coord length[2];

        if (!sd->horizontal)
          {
             length[0] = pos.y - hover_area.y;
             length[1] = (hover_area.y + hover_area.h) - pos.y;

             // ELM_CTXPOPUP_DIRECTION_UP
             if (length[0] > length[1])
               {
                  _x_pos_adjust(&pos, &base_size, &hover_area);
                  pos.y -= base_size.y;
                  dir = ELM_CTXPOPUP_DIRECTION_UP;
                  if (pos.y < (hover_area.y + arrow_size.y))
                    {
                       base_size.y -= ((hover_area.y + arrow_size.y) - pos.y);
                       pos.y = hover_area.y + arrow_size.y;
                    }
               }
             //ELM_CTXPOPUP_DIRECTION_DOWN
             else
               {
                  _x_pos_adjust(&pos, &base_size, &hover_area);
                  dir = ELM_CTXPOPUP_DIRECTION_DOWN;
                  if ((pos.y + arrow_size.y + base_size.y) >
                      (hover_area.y + hover_area.h))
                    base_size.y -=
                      ((pos.y + arrow_size.y + base_size.y) -
                       (hover_area.y + hover_area.h));
               }
          }
        else
          {
             length[0] = pos.x - hover_area.x;
             length[1] = (hover_area.x + hover_area.w) - pos.x;

             //ELM_CTXPOPUP_DIRECTION_LEFT
             if (length[0] > length[1])
               {
                  _y_pos_adjust(&pos, &base_size, &hover_area);
                  pos.x -= base_size.x;
                  dir = ELM_CTXPOPUP_DIRECTION_LEFT;
                  if (pos.x < (hover_area.x + arrow_size.x))
                    {
                       base_size.x -= ((hover_area.x + arrow_size.x) - pos.x);
                       pos.x = hover_area.x + arrow_size.x;
                    }
               }
             //ELM_CTXPOPUP_DIRECTION_RIGHT
             else
               {
                  _y_pos_adjust(&pos, &base_size, &hover_area);
                  dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
                  if (pos.x + (arrow_size.x + base_size.x) >
                      hover_area.x + hover_area.w)
                    base_size.x -=
                      ((pos.x + arrow_size.x + base_size.x) -
                       (hover_area.x + hover_area.w));
               }
          }
     }

   //Final position and size.
   rect->x = pos.x;
   rect->y = pos.y;
   rect->w = base_size.x;
   rect->h = base_size.y;

   return dir;
}

static void
_arrow_update(Evas_Object *obj,
              Elm_Ctxpopup_Direction dir,
              Evas_Coord_Rectangle base_size)
{
   Evas_Coord_Rectangle arrow_size;
   Evas_Coord x, y;
   double drag;

   ELM_CTXPOPUP_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_geometry_get
     (sd->arrow, NULL, NULL, &arrow_size.w, &arrow_size.h);

   /* arrow is not being kept as sub-object on purpose, here. the
    * design of the widget does not help with the contrary */

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        edje_object_signal_emit(sd->arrow, "elm,state,left", "elm");
        if (!edje_object_part_swallow(wd->resize_obj,
            (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_right" :
             "elm.swallow.arrow_left"), sd->arrow))
          {
             evas_object_hide(sd->arrow);
             break;
          }

        if (base_size.h > 0)
          {
             if (y < ((arrow_size.h * 0.5) + base_size.y))
               y = 0;
             else if (y > base_size.y + base_size.h - (arrow_size.h * 0.5))
               y = base_size.h - arrow_size.h;
             else
               y = y - base_size.y - (arrow_size.h * 0.5);
             drag = (double)(y) / (double)(base_size.h - arrow_size.h);
             edje_object_part_drag_value_set
                (wd->resize_obj,
                 (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_right" :
                  "elm.swallow.arrow_left"), 1, drag);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        edje_object_signal_emit(sd->arrow, "elm,state,right", "elm");
        if (!edje_object_part_swallow(wd->resize_obj,
            (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_left" :
             "elm.swallow.arrow_right"), sd->arrow))
          {
             evas_object_hide(sd->arrow);
             break;
          }

        if (base_size.h > 0)
          {
             if (y < ((arrow_size.h * 0.5) + base_size.y))
               y = 0;
             else if (y > (base_size.y + base_size.h - (arrow_size.h * 0.5)))
               y = base_size.h - arrow_size.h;
             else
               y = y - base_size.y - (arrow_size.h * 0.5);
             drag = (double)(y) / (double)(base_size.h - arrow_size.h);
             edje_object_part_drag_value_set
                (wd->resize_obj,
                 (elm_widget_mirrored_get(obj) ? "elm.swallow.arrow_left" :
                  "elm.swallow.arrow_right"), 0, drag);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        edje_object_signal_emit(sd->arrow, "elm,state,top", "elm");
        if (!edje_object_part_swallow(wd->resize_obj, "elm.swallow.arrow_up", sd->arrow))
          {
             evas_object_hide(sd->arrow);
             break;
          }

        if (base_size.w > 0)
          {
             if (x < ((arrow_size.w * 0.5) + base_size.x))
               x = 0;
             else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
               x = base_size.w - arrow_size.w;
             else
               x = x - base_size.x - (arrow_size.w * 0.5);
             drag = (double)(x) / (double)(base_size.w - arrow_size.w);
             edje_object_part_drag_value_set
               (wd->resize_obj, "elm.swallow.arrow_up", drag,
               1);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_UP:
        edje_object_signal_emit(sd->arrow, "elm,state,bottom", "elm");
        if (!edje_object_part_swallow(wd->resize_obj, "elm.swallow.arrow_down", sd->arrow))
          {
             evas_object_hide(sd->arrow);
             break;
          }

        if (base_size.w > 0)
          {
             if (x < ((arrow_size.w * 0.5) + base_size.x))
               x = 0;
             else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
               x = base_size.w - arrow_size.w;
             else x = x - base_size.x - (arrow_size.w * 0.5);
             drag = (double)(x) / (double)(base_size.w - arrow_size.w);
             edje_object_part_drag_value_set
               (wd->resize_obj, "elm.swallow.arrow_down",
               drag, 0);
          }
        break;

      default:
        break;
     }

   //should be here for getting accurate geometry value
   evas_object_smart_calculate(wd->resize_obj);
}

static void
_show_signals_emit(Evas_Object *obj,
                   Elm_Ctxpopup_Direction dir)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;
   if ((sd->list) && (!sd->list_visible)) return;
   if (sd->emitted) return;

   sd->emitted = EINA_TRUE;
   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_UP:
        elm_layout_signal_emit(obj, "elm,state,show,up", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        elm_layout_signal_emit(obj,
              (elm_widget_mirrored_get(obj) ? "elm,state,show,right" :
               "elm,state,show,left"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        elm_layout_signal_emit(obj,
              (elm_widget_mirrored_get(obj) ? "elm,state,show,left" :
               "elm,state,show,right"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        elm_layout_signal_emit(obj, "elm,state,show,down", "elm");
        break;

      default:
        break;
     }

   edje_object_signal_emit(sd->bg, "elm,state,show", "elm");
   elm_layout_signal_emit(obj, "elm,state,show", "elm");
}

static void
_hide_signals_emit(Evas_Object *obj,
                   Elm_Ctxpopup_Direction dir)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_UP:
        elm_layout_signal_emit(obj, "elm,state,hide,up", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        elm_layout_signal_emit(obj,
              (elm_widget_mirrored_get(obj) ? "elm,state,hide,right" :
               "elm,state,hide,left"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        elm_layout_signal_emit(obj,
              (elm_widget_mirrored_get(obj) ? "elm,state,hide,left" :
               "elm,state,hide,right"), "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        elm_layout_signal_emit(obj, "elm,state,hide,down", "elm");
        break;

      default:
        break;
     }
   edje_object_signal_emit(sd->bg, "elm,state,hide", "elm");
   elm_layout_signal_emit(obj, "elm,state,hide", "elm");
}

static void
_base_shift_by_arrow(Evas_Object *arrow,
                     Elm_Ctxpopup_Direction dir,
                     Evas_Coord_Rectangle *rect)
{
   Evas_Coord arrow_w, arrow_h;

   evas_object_geometry_get(arrow, NULL, NULL, &arrow_w, &arrow_h);
   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        rect->x += arrow_w;
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        rect->x -= arrow_w;
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        rect->y += arrow_h;
        break;

      case ELM_CTXPOPUP_DIRECTION_UP:
        rect->y -= arrow_h;
        break;

      default:
        break;
     }
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_layout_sub_object_add_enable(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_sub_object_add(Eo *obj, Elm_Ctxpopup_Data *_pd EINA_UNUSED, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_add(sobj));

   return int_ret;
}

EOLIAN static void
_elm_ctxpopup_elm_layout_sizing_eval(Eo *obj, Elm_Ctxpopup_Data *sd)
{
   Evas_Coord_Rectangle rect = { 0, 0, 1, 1 };
   Evas_Coord_Point list_size = { 0, 0 }, parent_size = {0, 0};

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->arrow) return;  /* simple way to flag "under deletion" */
   if (!sd->parent) return; /* do not calculate sizes unless parent is set */

   //Base
   sd->dir = _base_geometry_calc(obj, &rect);

   _arrow_update(obj, sd->dir, rect);

   _base_shift_by_arrow(sd->arrow, sd->dir, &rect);

   if ((sd->list) && (sd->list_visible))
     {
        evas_object_geometry_get(sd->list, 0, 0, &list_size.x, &list_size.y);
        if ((list_size.x >= rect.w) || (list_size.y >= rect.h))
          {
             elm_list_mode_set(sd->list, ELM_LIST_COMPRESS);
             evas_object_size_hint_min_set(obj, list_size.x, list_size.y);
          }
        /*
            Decrease height of main object initiate recalc of size (sizing_eval).
            At each iteration of calculating size height of the object
            will be closer to the height of the list.
         */
        if (list_size.y < rect.h) rect.h--;
     }
   else if (sd->content)
     {
        evas_object_geometry_get(sd->content, 0, 0, &list_size.x, &list_size.y);
        if ((list_size.x >= rect.w) || (list_size.y >= rect.h))
          evas_object_size_hint_min_set(obj, list_size.x, list_size.y);
     }

   evas_object_geometry_get(sd->parent, NULL, NULL, &parent_size.x, &parent_size.y);
   evas_object_resize(sd->bg, parent_size.x, parent_size.y);

   evas_object_move(wd->resize_obj, rect.x, rect.y);
   evas_object_resize(wd->resize_obj, rect.w, rect.h);

   _show_signals_emit(obj, sd->dir);
}

static void
_on_parent_del(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_on_parent_move(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(data);
}

static void
_on_parent_resize(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   if (sd->auto_hide)
     {
        sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

        evas_object_hide(data);
        evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
     }
   else
     {
        if (sd->visible)
          elm_layout_sizing_eval(data);
     }
}

static void
_parent_detach(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->parent) return;

   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_MOVE, _on_parent_move, obj);
   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_RESIZE, _on_parent_resize, obj);
}

static void
_on_content_resized(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   elm_box_recalculate(sd->box);
   elm_layout_sizing_eval(data);
}

//FIXME: lost the content size when theme hook is called.
EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_theme_apply(Eo *obj, Elm_Ctxpopup_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   elm_widget_theme_object_set
     (obj, sd->bg, "ctxpopup", "bg", elm_widget_style_get(obj));
   elm_widget_theme_object_set
     (obj, sd->arrow, "ctxpopup", "arrow", elm_widget_style_get(obj));

   if (sd->list)
     {
        if (!strncmp(elm_object_style_get(obj), "default", strlen("default")))
          elm_object_style_set(sd->list, "ctxpopup");
        else
          elm_object_style_set(sd->list, elm_object_style_get(obj));
     }

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

/* kind of a big and tricky override here: an internal box will hold
 * the actual content. content aliases won't be of much help here */
EOLIAN static Eina_Bool
_elm_ctxpopup_elm_container_content_set(Eo *obj, Elm_Ctxpopup_Data *sd, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_TRUE;

   if ((part) && (strcmp(part, "default")))
     {
        eo_do_super(obj, MY_CLASS, int_ret = elm_obj_container_content_set(part, content));
        return int_ret;
     }

   if (!content) return EINA_FALSE;

   if (content == sd->content) return EINA_TRUE;

   evas_object_del(sd->content);
   if (sd->content == sd->list) sd->list = NULL;

   evas_object_size_hint_weight_set
     (content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set
     (content, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_box_pack_end(sd->box, content);
   evas_object_show(content);

   sd->content = content;
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

EOLIAN static Evas_Object*
_elm_ctxpopup_elm_container_content_get(Eo *obj, Elm_Ctxpopup_Data *sd, const char *part)
{

   if ((part) && (strcmp(part, "default")))
     {
        Evas_Object *ret = NULL;
        eo_do_super(obj, MY_CLASS, ret = elm_obj_container_content_get(part));
        return ret;
     }

   return sd->content;
}

EOLIAN static Evas_Object*
_elm_ctxpopup_elm_container_content_unset(Eo *obj, Elm_Ctxpopup_Data *sd, const char *part)
{
   Evas_Object *content = NULL;

   if ((part) && (strcmp(part, "default")))
     {
        eo_do_super(obj, MY_CLASS, content = elm_obj_container_content_unset(part));
        return content;
     }

   content = sd->content;
   if (!content) return content;

   elm_box_unpack(sd->box, content);
   sd->content = NULL;
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return content;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if ((part) && (strcmp(part, "default"))) return;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_DATA_GET(WIDGET(ctxpopup_it), sd);

   elm_object_item_part_text_set(ctxpopup_it->list_item, "default", label);
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(WIDGET(ctxpopup_it));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if (part && strcmp(part, "default")) return NULL;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;
   return elm_object_item_part_text_get(ctxpopup_it->list_item, "default");
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if ((part) && (strcmp(part, "icon"))
       && (strcmp(part, "start"))
       && (strcmp(part, "end"))) return;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_DATA_GET(WIDGET(ctxpopup_it), sd);

   if ((part) && (!strcmp(part, "end")))
     elm_object_item_part_content_set(ctxpopup_it->list_item, "end", content);
   else
     elm_object_item_part_content_set
       (ctxpopup_it->list_item, "start", content);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(WIDGET(ctxpopup_it));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if (part && strcmp(part, "icon") && strcmp(part, "start")
       && strcmp(part, "end")) return NULL;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   if (part && !strcmp(part, "end"))
     return elm_object_item_part_content_get(ctxpopup_it->list_item, "end");
   else
     return elm_object_item_part_content_get(ctxpopup_it->list_item, "start");
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   elm_object_item_disabled_set
     (ctxpopup_it->list_item, elm_widget_item_disabled_get(ctxpopup_it));
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   elm_object_item_signal_emit(ctxpopup_it->list_item, emission, source);
}

static void
_bg_clicked_cb(void *data,
               Evas_Object *obj EINA_UNUSED,
               const char *emission EINA_UNUSED,
               const char *source EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   _hide_signals_emit(data, sd->dir);
}

static void
_on_show(void *data EINA_UNUSED,
         Evas *e EINA_UNUSED,
         Evas_Object *obj,
         void *event_info EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (sd->list)
     {
        elm_list_go(sd->list);
        sd->visible = EINA_TRUE;
        /*
         * XXX: Giving focus to the list when it has nothing selected makes
         * it select the first of its items, which makes the popup in
         * Terminology never open and instead just trigger the first option.
         * I'll let as an excercise to the reader to figure out why that
         * is so fucking annoying. Extra points for noting why this is my
         * choice of a "fix" instead of fixing the actual focus/select issue
         * that seems to be spread all over Elementary.
         */
        //elm_object_focus_set(sd->list, EINA_TRUE);
        return;
     }

   if (!sd->content) return;

   sd->emitted = EINA_FALSE;
   sd->visible = EINA_TRUE;

   _show_signals_emit(obj, sd->dir);

   elm_layout_sizing_eval(obj);

   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_on_hide(void *data EINA_UNUSED,
         Evas *e EINA_UNUSED,
         Evas_Object *obj,
         void *event_info EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;

   sd->visible = EINA_FALSE;
   sd->list_visible = EINA_FALSE;
}

static void
_on_move(void *data EINA_UNUSED,
         Evas *e EINA_UNUSED,
         Evas_Object *obj,
         void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(obj);
}

static void
_hide_finished_cb(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_list_resize_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   if (!sd->visible) return;
   if (sd->list_visible) return;

   sd->list_visible = EINA_TRUE;

   elm_layout_sizing_eval(data);
}

static void
_list_del(Elm_Ctxpopup_Data *sd)
{
   ELM_SAFE_FREE(sd->list, evas_object_del);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Evas_Object *list;
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_DATA_GET(WIDGET(ctxpopup_it), sd);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   list = elm_object_item_widget_get(ctxpopup_it->list_item);

   if (eina_list_count(elm_list_items_get(list)) < 2)
     {
        if (ctxpopup_it->list_item)
          elm_widget_item_del(ctxpopup_it->list_item);
        sd->items = eina_list_remove(sd->items, ctxpopup_it);
        evas_object_hide(WIDGET(ctxpopup_it));

        return EINA_TRUE;
     }

   if (ctxpopup_it->list_item)
     elm_widget_item_del(ctxpopup_it->list_item);
   sd->items = eina_list_remove(sd->items, ctxpopup_it);
   if (sd->list_visible) elm_layout_sizing_eval(WIDGET(ctxpopup_it));

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_elm_widget_disable(Eo *obj, Elm_Ctxpopup_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_disable());
   if (!int_ret) return EINA_FALSE;
   if (sd->list)
     elm_object_disabled_set(sd->list, elm_widget_disabled_get(obj));
   else if (sd->content)
     elm_object_disabled_set(sd->content, elm_widget_disabled_get(obj));

   return EINA_TRUE;
}

EOLIAN static void
_elm_ctxpopup_evas_object_smart_add(Eo *obj, Elm_Ctxpopup_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "ctxpopup", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add
     (obj, "elm,action,hide,finished", "*", _hide_finished_cb, obj);

   //Background
   priv->bg = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, priv->bg, "ctxpopup", "bg", "default");
   edje_object_signal_callback_add
     (priv->bg, "elm,action,click", "*", _bg_clicked_cb, obj);
   evas_object_smart_member_add(priv->bg, obj);
   evas_object_stack_below(priv->bg, wd->resize_obj);

   //Arrow
   priv->arrow = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, priv->arrow, "ctxpopup", "arrow", "default");
   evas_object_smart_member_add(priv->arrow, obj);

   priv->dir_priority[0] = ELM_CTXPOPUP_DIRECTION_UP;
   priv->dir_priority[1] = ELM_CTXPOPUP_DIRECTION_LEFT;
   priv->dir_priority[2] = ELM_CTXPOPUP_DIRECTION_RIGHT;
   priv->dir_priority[3] = ELM_CTXPOPUP_DIRECTION_DOWN;
   priv->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   priv->auto_hide = EINA_TRUE;

   priv->box = elm_box_add(obj);
   evas_object_size_hint_weight_set
     (priv->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add
     (priv->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);

   /* box will be our content placeholder, thus the parent's version call */
   eo_do_super(obj, MY_CLASS, elm_obj_container_content_set("elm.swallow.content", priv->box));

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _on_show, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _on_hide, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _on_move, NULL);

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

EOLIAN static void
_elm_ctxpopup_evas_object_smart_del(Eo *obj, Elm_Ctxpopup_Data *sd)
{
   Elm_Ctxpopup_Item *it;

   evas_object_event_callback_del_full
     (sd->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);
   _parent_detach(obj);

   elm_ctxpopup_clear(obj);
   ELM_SAFE_FREE(sd->arrow, evas_object_del); /* stops _sizing_eval() from going on on deletion */
   ELM_SAFE_FREE(sd->bg, evas_object_del);

   EINA_LIST_FREE(sd->items, it)
     elm_widget_item_free(it);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_ctxpopup_elm_widget_parent_set(Eo *obj, Elm_Ctxpopup_Data *_pd EINA_UNUSED, Evas_Object *parent)
{
   //default parent is to be hover parent
   elm_ctxpopup_hover_parent_set(obj, parent);
}

EAPI Evas_Object *
elm_ctxpopup_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);

   /* access: parent could be any object such as elm_list which does
      not know elc_ctxpopup as its child object in the focus_next(); */
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   wd->highlight_root = EINA_TRUE;

   return obj;
}

EOLIAN static void
_elm_ctxpopup_eo_base_constructor(Eo *obj, Elm_Ctxpopup_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_POPUP_MENU));
}

EOLIAN static void
_elm_ctxpopup_hover_parent_set(Eo *obj, Elm_Ctxpopup_Data *sd, Evas_Object *parent)
{
   Evas_Coord x, y, w, h;

   if (!parent) return;

   _parent_detach(obj);

   evas_object_event_callback_add
     (parent, EVAS_CALLBACK_DEL, _on_parent_del, obj);
   evas_object_event_callback_add
     (parent, EVAS_CALLBACK_MOVE, _on_parent_move, obj);
   evas_object_event_callback_add
     (parent, EVAS_CALLBACK_RESIZE, _on_parent_resize, obj);

   sd->parent = parent;

   //Update Background
   evas_object_geometry_get(parent, &x, &y, &w, &h);
   if (parent && eo_isa(parent, ELM_WIN_CLASS))
     x = y = 0;
   evas_object_move(sd->bg, x, y);
   evas_object_resize(sd->bg, w, h);

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EOLIAN static Evas_Object*
_elm_ctxpopup_hover_parent_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   return sd->parent;
}

EOLIAN static void
_elm_ctxpopup_clear(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   _list_del(sd);
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
}

EOLIAN static void
_elm_ctxpopup_horizontal_set(Eo *obj, Elm_Ctxpopup_Data *sd, Eina_Bool horizontal)
{
   sd->horizontal = !!horizontal;

   if (!sd->list) return;

   elm_list_horizontal_set(sd->list, sd->horizontal);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EOLIAN static Eina_Bool
_elm_ctxpopup_horizontal_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   return sd->horizontal;
}

static void
_item_wrap_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Ctxpopup_Item *item = data;
   if (!item->wcb.org_func_cb) return;
   item->wcb.org_func_cb((void *)item->wcb.org_data, item->wcb.cobj, item);
}

EOLIAN static Elm_Object_Item*
_elm_ctxpopup_item_append(Eo *obj, Elm_Ctxpopup_Data *sd, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data)
{
   Elm_Ctxpopup_Item *item;

   item = _item_new(obj, sd, func, data);
   if (!item) return NULL;

   item->list_item =
     elm_list_item_append(sd->list, label, icon, NULL, _item_wrap_cb, item);
   sd->items = eina_list_append(sd->items, item);

   if (sd->visible) elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)item;
}

EOLIAN static void
_elm_ctxpopup_direction_priority_set(Eo *obj, Elm_Ctxpopup_Data *sd, Elm_Ctxpopup_Direction first, Elm_Ctxpopup_Direction second, Elm_Ctxpopup_Direction third, Elm_Ctxpopup_Direction fourth)
{
   sd->dir_priority[0] = first;
   sd->dir_priority[1] = second;
   sd->dir_priority[2] = third;
   sd->dir_priority[3] = fourth;

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_ctxpopup_direction_priority_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd, Elm_Ctxpopup_Direction *first, Elm_Ctxpopup_Direction *second, Elm_Ctxpopup_Direction *third, Elm_Ctxpopup_Direction *fourth)
{
   if (first) *first = sd->dir_priority[0];
   if (second) *second = sd->dir_priority[1];
   if (third) *third = sd->dir_priority[2];
   if (fourth) *fourth = sd->dir_priority[3];
}

EOLIAN static Elm_Ctxpopup_Direction
_elm_ctxpopup_direction_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   return sd->dir;
}

EOLIAN static void
_elm_ctxpopup_dismiss(Eo *obj, Elm_Ctxpopup_Data *sd)
{
   _hide_signals_emit(obj, sd->dir);
}

EOLIAN static void
_elm_ctxpopup_auto_hide_disabled_set(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd, Eina_Bool disabled)
{
   disabled = !!disabled;
   if (sd->auto_hide == !disabled) return;
   sd->auto_hide = !disabled;
}

EOLIAN static Eina_Bool
_elm_ctxpopup_auto_hide_disabled_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   return sd->auto_hide;
}

EOLIAN static void
_elm_ctxpopup_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static const Eina_List*
_elm_ctxpopup_items_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   return sd->items;
}

EOLIAN static Elm_Object_Item*
_elm_ctxpopup_first_item_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   if (!sd->items) return NULL;

   return eina_list_data_get(sd->items);
}

EOLIAN static Elm_Object_Item*
_elm_ctxpopup_last_item_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd)
{
   if (!sd->items) return NULL;

   return eina_list_data_get(eina_list_last(sd->items));
}

EOLIAN static Elm_Object_Item*
_elm_ctxpopup_item_prepend(Eo *obj, Elm_Ctxpopup_Data *sd, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data)
{
   Elm_Ctxpopup_Item *item;

   item = _item_new(obj, sd, func, data);
   if (!item) return NULL;

   item->list_item =
     elm_list_item_prepend(sd->list, label, icon, NULL, _item_wrap_cb, item);
   sd->items = eina_list_prepend(sd->items, item);

   if (sd->visible) elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)item;
}

EAPI Elm_Object_Item *
elm_ctxpopup_item_prev_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Ctxpopup_Item *item = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_CTXPOPUP_DATA_GET(WIDGET(item), sd);

   if (!sd->items) return NULL;

   l = eina_list_data_find_list(sd->items, it);
   if (l && l->prev) return eina_list_data_get(l->prev);

   return NULL;
}

EAPI Elm_Object_Item *
elm_ctxpopup_item_next_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Ctxpopup_Item *item = (Elm_Ctxpopup_Item *)it;

   ELM_CTXPOPUP_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_CTXPOPUP_DATA_GET(WIDGET(item), sd);

   if (!sd->items) return NULL;

   l = eina_list_data_find_list(sd->items, it);
   if (l && l->next) return eina_list_data_get(l->next);

   return NULL;
}

static Elm_Ctxpopup_Item*
_item_new(Eo *obj,
          Elm_Ctxpopup_Data *sd,
          Evas_Smart_Cb func,
          const void *data)
{
   Elm_Ctxpopup_Item *item;

   item = elm_widget_item_new(obj, Elm_Ctxpopup_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(item, _item_disable_hook);
   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(item, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(item, _item_content_get_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);

   if (!sd->list)
     {
        sd->list = elm_list_add(obj);
        if (!strncmp(elm_object_style_get(obj), "default", strlen("default")))
          elm_object_style_set(sd->list, "ctxpopup");
        else elm_object_style_set(sd->list, elm_object_style_get(obj));
        elm_list_mode_set(sd->list, ELM_LIST_EXPAND);
        elm_list_horizontal_set(sd->list, sd->horizontal);
        evas_object_event_callback_add
          (sd->list, EVAS_CALLBACK_RESIZE, _list_resize_cb, obj);
        elm_layout_content_set(obj, "default", sd->list);
     }

   item->wcb.org_func_cb = func;
   item->wcb.org_data = data;
   item->wcb.cobj = obj;

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   return item;
}

EOLIAN static const Elm_Atspi_Action*
_elm_ctxpopup_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Elm_Ctxpopup_Data *sd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
          { "move,previous", "move", "previous", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,left", "move", "left", _key_action_move},
          { "move,right", "move", "right", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

#include "elm_ctxpopup.eo.c"
