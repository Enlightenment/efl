#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_ctxpopup.h"

EAPI const char ELM_CTXPOPUP_SMART_NAME[] = "elm_ctxpopup";

static const char SIG_DISMISSED[] = "dismissed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DISMISSED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_CTXPOPUP_SMART_NAME, _elm_ctxpopup, Elm_Ctxpopup_Smart_Class,
   Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);
static Eina_Bool
_elm_ctxpopup_smart_focus_next(const Evas_Object *obj,
                               Elm_Focus_Direction dir,
                               Evas_Object **next)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd)
     return EINA_FALSE;

   if (!elm_widget_focus_next_get(sd->box, dir, next))
     {
        elm_widget_focused_object_clear(sd->box);
        elm_widget_focus_next_get(sd->box, dir, next);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_ctxpopup_smart_event(Evas_Object *obj,
                          Evas_Object *src __UNUSED__,
                          Evas_Callback_Type type,
                          void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!strcmp(ev->keyname, "Tab"))
     {
        if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          elm_widget_focus_cycle(sd->box, ELM_FOCUS_PREVIOUS);
        else
          elm_widget_focus_cycle(sd->box, ELM_FOCUS_NEXT);
        return EINA_TRUE;
     }

   if (strcmp(ev->keyname, "Escape")) return EINA_FALSE;

   evas_object_hide(obj);
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

   if (!rect) return ELM_CTXPOPUP_DIRECTION_DOWN;

   edje_object_part_geometry_get
     (sd->arrow, "ctxpopup_arrow", NULL, NULL, &arrow_size.x, &arrow_size.y);
   evas_object_resize(sd->arrow, arrow_size.x, arrow_size.y);

   //Initialize Area Rectangle.
   evas_object_geometry_get
     (sd->parent, &hover_area.x, &hover_area.y, &hover_area.w,
     &hover_area.h);

   evas_object_geometry_get(obj, &pos.x, &pos.y, NULL, NULL);

   //recalc the edje
   edje_object_size_min_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &base_size.x, &base_size.y);
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);

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

   //Check the Which direction is available.
   //If find a avaialble direction, it adjusts position and size.
   for (idx = 0; idx < 4; idx++)
     {
        switch (sd->dir_priority[idx])
          {
           case ELM_CTXPOPUP_DIRECTION_UNKNOWN:
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
             break;
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

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_geometry_get
     (sd->arrow, NULL, NULL, &arrow_size.w, &arrow_size.h);

   /* arrow is not being kept as sub-object on purpose, here. the
    * design of the widget does not help with the contrary */

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        edje_object_signal_emit(sd->arrow, "elm,state,left", "elm");
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_left",
          sd->arrow);
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
               (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_left", 1,
               drag);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        edje_object_signal_emit(sd->arrow, "elm,state,right", "elm");
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_right",
          sd->arrow);

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
               (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_right", 0,
               drag);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        edje_object_signal_emit(sd->arrow, "elm,state,top", "elm");
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_up",
          sd->arrow);

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
               (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_up", drag,
               1);
          }
        break;

      case ELM_CTXPOPUP_DIRECTION_UP:
        edje_object_signal_emit(sd->arrow, "elm,state,bottom", "elm");
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_down",
          sd->arrow);

        if (base_size.w > 0)
          {
             if (x < ((arrow_size.w * 0.5) + base_size.x))
               x = 0;
             else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
               x = base_size.w - arrow_size.w;
             else x = x - base_size.x - (arrow_size.w * 0.5);
             drag = (double)(x) / (double)(base_size.w - arrow_size.w);
             edje_object_part_drag_value_set
               (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.arrow_down",
               drag, 0);
          }
        break;

      default:
        break;
     }

   //should be here for getting accurate geometry value
   evas_object_smart_calculate(ELM_WIDGET_DATA(sd)->resize_obj);
}

static void
_show_signals_emit(Evas_Object *obj,
                   Elm_Ctxpopup_Direction dir)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;
   if ((sd->list) && (!sd->list_visible)) return;

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_UP:
        elm_layout_signal_emit(obj, "elm,state,show,up", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_LEFT:
        elm_layout_signal_emit(obj, "elm,state,show,left", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        elm_layout_signal_emit(obj, "elm,state,show,right", "elm");
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
        elm_layout_signal_emit(obj, "elm,state,hide,left", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_RIGHT:
        elm_layout_signal_emit(obj, "elm,state,hide,right", "elm");
        break;

      case ELM_CTXPOPUP_DIRECTION_DOWN:
        elm_layout_signal_emit(obj, "elm,state,hide,down", "elm");
        break;

      default:
        break;
     }

   edje_object_signal_emit(sd->bg, "elm,state,hide", "elm");
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

static Eina_Bool
_elm_ctxpopup_smart_sub_object_add(Evas_Object *obj,
                                   Evas_Object *sobj)
{
   Elm_Widget_Smart_Class *parent_parent;

   parent_parent = (Elm_Widget_Smart_Class *)((Evas_Smart_Class *)
                                              _elm_ctxpopup_parent_sc)->parent;

   /* skipping layout's code, which registers size hint changing
    * callback on sub objects. a hack to make ctxpopup live, as it is,
    * on the new classing schema. this widget needs a total
    * rewrite. */
   if (!parent_parent->sub_object_add(obj, sobj))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_elm_ctxpopup_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord_Rectangle rect = { 0, 0, 1, 1 };
   Evas_Coord_Point list_size = { 0, 0 };

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->arrow) return;  /* simple way to flag "under deletion" */

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
             evas_object_size_hint_min_set(sd->box, rect.w, rect.h);
             evas_object_size_hint_min_set(obj, rect.w, rect.h);
          }
     }

   evas_object_move(ELM_WIDGET_DATA(sd)->resize_obj, rect.x, rect.y);
   evas_object_resize(ELM_WIDGET_DATA(sd)->resize_obj, rect.w, rect.h);

   _show_signals_emit(obj, sd->dir);
}

static void
_on_parent_del(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   evas_object_del(data);
}

static void
_on_parent_move(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(data);
}

static void
_on_parent_resize(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   evas_object_hide(data);
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
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   elm_box_recalculate(sd->box);
   elm_layout_sizing_eval(data);
}

//FIXME: lost the content size when theme hook is called.
static Eina_Bool
_elm_ctxpopup_smart_theme(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->theme(obj))
     return EINA_FALSE;

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
static Eina_Bool
_elm_ctxpopup_smart_content_set(Evas_Object *obj,
                                const char *part,
                                Evas_Object *content)
{
   Evas_Coord min_w = -1, min_h = -1;

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if ((part) && (strcmp(part, "default")))
     return ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_set
              (obj, part, content);

   if (!content) return EINA_FALSE;

   if (content == sd->content) return EINA_TRUE;

   if (sd->content) evas_object_del(sd->content);
   if (sd->content == sd->list) sd->list = NULL;

   evas_object_size_hint_weight_set
     (content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set
     (content, EVAS_HINT_FILL, EVAS_HINT_FILL);

   /* since it's going to be a box content, not a layout's... */
   evas_object_show(content);

   evas_object_size_hint_min_get(content, &min_w, &min_h);
   evas_object_size_hint_min_set(sd->box, min_w, min_h);
   elm_box_pack_end(sd->box, content);

   sd->content = content;
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_ctxpopup_smart_content_get(const Evas_Object *obj,
                                const char *part)
{
   if ((part) && (strcmp(part, "default"))) return NULL;
   return ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_get(obj, part);

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->content;
}

static Evas_Object *
_elm_ctxpopup_smart_content_unset(Evas_Object *obj,
                                  const char *part)
{
   Evas_Object *content;

   if ((part) && (strcmp(part, "default"))) return NULL;
   return ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_unset
            (obj, part);

   ELM_CTXPOPUP_DATA_GET(obj, sd);

   content = sd->content;
   if (!content) return NULL;

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
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   _hide_signals_emit(data, sd->dir);
}

static void
_on_show(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (sd->list)
     {
        elm_list_go(sd->list);
        sd->visible = EINA_TRUE;
        elm_object_focus_set(obj, EINA_TRUE);
        return;
     }

   if (!sd->content) return;

   sd->visible = EINA_TRUE;

   evas_object_show(sd->bg);
   evas_object_show(sd->arrow);

   edje_object_signal_emit(sd->bg, "elm,state,show", "elm");
   elm_layout_signal_emit(obj, "elm,state,show", "elm");

   elm_layout_sizing_eval(obj);

   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_on_hide(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!sd->visible) return;

   evas_object_hide(sd->bg);
   evas_object_hide(sd->arrow);

   sd->visible = EINA_FALSE;
   sd->list_visible = EINA_FALSE;
}

static void
_on_move(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (sd->visible) evas_object_show(sd->arrow);

   elm_layout_sizing_eval(obj);
}

static void
_hide_finished_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *emission __UNUSED__,
                  const char *source __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_list_resize_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(data, sd);

   if (!sd->visible) return;
   if (sd->list_visible) return;

   sd->list_visible = EINA_TRUE;

   evas_object_show(sd->bg);
   evas_object_show(sd->arrow);

   elm_layout_sizing_eval(data);
}

static void
_ctxpopup_restack_cb(void *data __UNUSED__,
                     Evas *e __UNUSED__,
                     Evas_Object *obj,
                     void *event_info __UNUSED__)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   evas_object_layer_set(sd->bg, evas_object_layer_get(obj));
}

static void
_list_del(Elm_Ctxpopup_Smart_Data *sd)
{
   if (!sd->list) return;

   evas_object_del(sd->list);
   sd->list = NULL;
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
        elm_object_item_del(ctxpopup_it->list_item);
        evas_object_hide(WIDGET(ctxpopup_it));

        return EINA_TRUE;
     }

   elm_object_item_del(ctxpopup_it->list_item);
   if (sd->list_visible) elm_layout_sizing_eval(WIDGET(ctxpopup_it));

   return EINA_TRUE;
}

static Eina_Bool
_elm_ctxpopup_smart_disable(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->disable(obj))
     return EINA_FALSE;

   elm_object_disabled_set(sd->list, elm_widget_disabled_get(obj));

   return EINA_TRUE;
}

static void
_elm_ctxpopup_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Ctxpopup_Smart_Data);

   ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "ctxpopup", "base", elm_widget_style_get(obj));
   elm_layout_signal_callback_add
     (obj, "elm,action,hide,finished", "", _hide_finished_cb, obj);

   //Background
   priv->bg = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, priv->bg, "ctxpopup", "bg", "default");
   edje_object_signal_callback_add
     (priv->bg, "elm,action,click", "", _bg_clicked_cb, obj);

   evas_object_stack_below(priv->bg, obj);

   //Arrow
   priv->arrow = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, priv->arrow, "ctxpopup", "arrow", "default");

   priv->dir_priority[0] = ELM_CTXPOPUP_DIRECTION_UP;
   priv->dir_priority[1] = ELM_CTXPOPUP_DIRECTION_LEFT;
   priv->dir_priority[2] = ELM_CTXPOPUP_DIRECTION_RIGHT;
   priv->dir_priority[3] = ELM_CTXPOPUP_DIRECTION_DOWN;
   priv->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESTACK, _ctxpopup_restack_cb, obj);

   priv->box = elm_box_add(obj);
   evas_object_size_hint_weight_set
     (priv->box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add
     (priv->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);

   /* box will be our content placeholder, thus the parent's version call */
   ELM_CONTAINER_CLASS(_elm_ctxpopup_parent_sc)->content_set
     (obj, "elm.swallow.content", priv->box);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _on_show, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _on_hide, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _on_move, NULL);

   elm_widget_can_focus_set(obj, EINA_TRUE);
}

static void
_elm_ctxpopup_smart_del(Evas_Object *obj)
{
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (sd->box, EVAS_CALLBACK_RESIZE, _on_content_resized, obj);
   _parent_detach(obj);

   elm_ctxpopup_clear(obj);
   evas_object_del(sd->arrow);
   sd->arrow = NULL; /* stops _sizing_eval() from going on on deletion */

   evas_object_del(sd->bg);
   sd->bg = NULL;

   ELM_WIDGET_CLASS(_elm_ctxpopup_parent_sc)->base.del(obj);
}

static void
_elm_ctxpopup_smart_parent_set(Evas_Object *obj,
                               Evas_Object *parent)
{
   //default parent is to be hover parent
   elm_ctxpopup_hover_parent_set(obj, parent);
}

static void
_elm_ctxpopup_smart_set_user(Elm_Ctxpopup_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_ctxpopup_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_ctxpopup_smart_del;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_ctxpopup_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->disable = _elm_ctxpopup_smart_disable;
   ELM_WIDGET_CLASS(sc)->event = _elm_ctxpopup_smart_event;
   ELM_WIDGET_CLASS(sc)->theme = _elm_ctxpopup_smart_theme;
   ELM_WIDGET_CLASS(sc)->sub_object_add = _elm_ctxpopup_smart_sub_object_add;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_ctxpopup_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_get = _elm_ctxpopup_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_set = _elm_ctxpopup_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_ctxpopup_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_ctxpopup_smart_sizing_eval;
}

EAPI const Elm_Ctxpopup_Smart_Class *
elm_ctxpopup_smart_class_get(void)
{
   static Elm_Ctxpopup_Smart_Class _sc =
     ELM_CTXPOPUP_SMART_CLASS_INIT_NAME_VERSION(ELM_CTXPOPUP_SMART_NAME);
   static const Elm_Ctxpopup_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_ctxpopup_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_ctxpopup_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_ctxpopup_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_ctxpopup_hover_parent_set(Evas_Object *obj,
                              Evas_Object *parent)
{
   Evas_Coord x, y, w, h;

   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

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
   evas_object_move(sd->bg, x, y);
   evas_object_resize(sd->bg, w, h);

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_ctxpopup_hover_parent_get(const Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj) NULL;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->parent;
}

EAPI void
elm_ctxpopup_clear(Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   _list_del(sd);
   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
}

EAPI void
elm_ctxpopup_horizontal_set(Evas_Object *obj,
                            Eina_Bool horizontal)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   sd->horizontal = !!horizontal;

   if (!sd->list) return;

   elm_list_horizontal_set(sd->list, sd->horizontal);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EAPI Eina_Bool
elm_ctxpopup_horizontal_get(const Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj) EINA_FALSE;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI Elm_Object_Item *
elm_ctxpopup_item_append(Evas_Object *obj,
                         const char *label,
                         Evas_Object *icon,
                         Evas_Smart_Cb func,
                         const void *data)
{
   Elm_Ctxpopup_Item *item;

   ELM_CTXPOPUP_CHECK(obj) NULL;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

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
        //The first item is appended.
        sd->list = elm_list_add(obj);
        elm_list_mode_set(sd->list, ELM_LIST_EXPAND);
        elm_list_horizontal_set(sd->list, sd->horizontal);
        evas_object_event_callback_add
          (sd->list, EVAS_CALLBACK_RESIZE, _list_resize_cb, obj);
        elm_layout_content_set(obj, "default", sd->list);
     }

   item->list_item =
     elm_list_item_append(sd->list, label, icon, NULL, func, data);

   sd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (sd->visible) elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)item;
}

EAPI void
elm_ctxpopup_direction_priority_set(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction first,
                                    Elm_Ctxpopup_Direction second,
                                    Elm_Ctxpopup_Direction third,
                                    Elm_Ctxpopup_Direction fourth)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   sd->dir_priority[0] = first;
   sd->dir_priority[1] = second;
   sd->dir_priority[2] = third;
   sd->dir_priority[3] = fourth;

   if (sd->visible) elm_layout_sizing_eval(obj);
}

EAPI void
elm_ctxpopup_direction_priority_get(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction *first,
                                    Elm_Ctxpopup_Direction *second,
                                    Elm_Ctxpopup_Direction *third,
                                    Elm_Ctxpopup_Direction *fourth)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   if (first) *first = sd->dir_priority[0];
   if (second) *second = sd->dir_priority[1];
   if (third) *third = sd->dir_priority[2];
   if (fourth) *fourth = sd->dir_priority[3];
}

EAPI Elm_Ctxpopup_Direction
elm_ctxpopup_direction_get(const Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj) ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   return sd->dir;
}

EAPI void
elm_ctxpopup_dismiss(Evas_Object *obj)
{
   ELM_CTXPOPUP_CHECK(obj);
   ELM_CTXPOPUP_DATA_GET(obj, sd);

   _hide_signals_emit(obj, sd->dir);
}
