#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Ctxpopup_Item Elm_Ctxpopup_Item;

struct _Elm_Ctxpopup_Item
{
   ELM_WIDGET_ITEM;
   Elm_Object_Item *list_item;
};

struct _Widget_Data
{
   Evas_Object *parent;
   Evas_Object *base;
   Evas_Object *content;
   Evas_Object *list;
   Evas_Object *box;
   Evas_Object *arrow;
   Evas_Object *bg;
   Elm_Ctxpopup_Direction dir;
   Elm_Ctxpopup_Direction dir_priority[4];
   Eina_Bool horizontal:1;
   Eina_Bool visible:1;
   Eina_Bool list_visible:1;
   Eina_Bool finished:1;
};

static const char *widtype = NULL;

static void _freeze_on(void *data, Evas_Object *obj, void *event_info);
static void _freeze_off(void *data, Evas_Object *obj, void *event_info);
static void _hold_on(void *data, Evas_Object *obj, void *event_info);
static void _hold_off(void *data, Evas_Object *obj, void *event_info);
static void _on_focus_hook(void *data, Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj,
                             Evas_Object *src,
                             Evas_Callback_Type type,
                             void *event_info);
static void _parent_cut_off(Evas_Object *obj);
static void _parent_resize(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _parent_move(void *data,
                         Evas *e,
                         Evas_Object *obj,
                         void *event_info);
static void _parent_del(void *data,
                        Evas *e,
                        Evas_Object *obj,
                        void *event_info);
static void _adjust_pos_x(Evas_Coord_Point *pos,
                          Evas_Coord_Point *base_size,
                          Evas_Coord_Rectangle *hover_area);
static void _adjust_pos_y(Evas_Coord_Point *pos,
                          Evas_Coord_Point *base_size,
                          Evas_Coord_Rectangle *hover_area);
static Elm_Ctxpopup_Direction _calc_base_geometry(Evas_Object *obj,
                                                  Evas_Coord_Rectangle *rect);
static void _update_arrow(Evas_Object *obj,
                          Elm_Ctxpopup_Direction dir,
                          Evas_Coord_Rectangle rect);
static void _sizing_eval(Evas_Object *obj);
static void _hide_signal_emit(Evas_Object *obj,
                              Elm_Ctxpopup_Direction dir);
static void _show_signal_emit(Evas_Object *obj,
                              Elm_Ctxpopup_Direction dir);
static void _shift_base_by_arrow(Evas_Object *arrow,
                                 Elm_Ctxpopup_Direction dir,
                                 Evas_Coord_Rectangle *rect);
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _content_set_hook(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content);
static Evas_Object * _content_unset_hook(Evas_Object *obj,
                                         const char *part__);
static Evas_Object * _content_get_hook(const Evas_Object *obj,
                                       const char *part);
static void _item_text_set_hook(Elm_Object_Item *it,
                               const char *part,
                               const char *label);
static const char * _item_text_get_hook(const Elm_Object_Item *it,
                                        const char *part);
static void _item_content_set_hook(Elm_Object_Item *it,
                                   const char *part,
                                   Evas_Object *content);
static Evas_Object * _item_content_get_hook(const Elm_Object_Item *it,
                                            const char *part);
static void _item_disable_hook(Elm_Object_Item *it);
static void _item_signal_emit_hook(Elm_Object_Item *it,
                                   const char *emission,
                                   const char *source);
static void _bg_clicked_cb(void *data, Evas_Object *obj,
                           const char *emission,
                           const char *source);
static void _ctxpopup_show(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _hide_finished(void *data,
                           Evas_Object *obj,
                           const char *emission,
                           const char *source __UNUSED__);
static void _ctxpopup_hide(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _content_resize(void *data,
                            Evas *e,
                            Evas_Object *obj,
                            void *event_info);
static void _ctxpopup_move(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _restack(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _content_del(void *data,
                         Evas *e,
                         Evas_Object *obj,
                         void *event_info);
static void _list_del(Widget_Data *wd);
static void _disable_hook(Evas_Object *obj);
static void _signal_emit_hook(Evas_Object *obj, const char *emission, const char *source);
static void _signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data);
static void _signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data);

static const char SIG_DISMISSED[] = "dismissed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_DISMISSED, ""},
   {NULL, NULL}
};

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj,
           void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->list)) return;
   elm_widget_scroll_freeze_push(wd->list);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj,
            void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->list)) return;
   elm_widget_scroll_freeze_pop(wd->list);
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->list)) return;
   elm_widget_scroll_hold_push(wd->list);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->list)) return;
   elm_widget_scroll_hold_pop(wd->list);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;


   if (elm_widget_focus_get(obj))
     {
        //FIXME:
     }
   else
     {
        //FIXME:
     }
}

static Eina_Bool
_focus_next_hook(const Evas_Object *obj,
                 Elm_Focus_Direction dir,
                 Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return EINA_FALSE;

   if (!elm_widget_focus_next_get(wd->box, dir, next))
     {
        elm_widget_focused_object_clear(wd->box);
        elm_widget_focus_next_get(wd->box, dir, next);
     }

   return EINA_TRUE;
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__,
            Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev;
   Widget_Data *wd;

   if (type != EVAS_CALLBACK_KEY_DOWN)
     return EINA_FALSE;
   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!strcmp(ev->keyname, "Tab"))
     {
        if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          elm_widget_focus_cycle(wd->box, ELM_FOCUS_PREVIOUS);
        else
          elm_widget_focus_cycle(wd->box, ELM_FOCUS_NEXT);
        return EINA_TRUE;
     }

   if (strcmp(ev->keyname, "Escape")) return EINA_FALSE;

   evas_object_hide(obj);
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_parent_cut_off(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !wd->parent) return;

   evas_object_event_callback_del_full(wd->parent,
                                       EVAS_CALLBACK_DEL,
                                       _parent_del,
                                       obj);
   evas_object_event_callback_del_full(wd->parent,
                                       EVAS_CALLBACK_MOVE,
                                       _parent_move,
                                       obj);
   evas_object_event_callback_del_full(wd->parent,
                                       EVAS_CALLBACK_RESIZE,
                                       _parent_resize,
                                       obj);
}

static void
_parent_resize(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   evas_object_hide(data);
}

static void
_parent_move(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible)
     {
        _sizing_eval(obj);
     }
}

static void
_parent_del(void *data,
            Evas *e __UNUSED__,
            Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   evas_object_del(data);
}

static void
_adjust_pos_x(Evas_Coord_Point *pos, Evas_Coord_Point *base_size,
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
_adjust_pos_y(Evas_Coord_Point *pos, Evas_Coord_Point *base_size,
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
_calc_base_geometry(Evas_Object *obj, Evas_Coord_Rectangle *rect)
{
   Widget_Data *wd;
   Evas_Coord_Point pos = {0, 0};
   Evas_Coord_Point base_size;
   Evas_Coord_Point max_size;
   Evas_Coord_Point min_size;
   Evas_Coord_Rectangle hover_area;
   Evas_Coord_Point arrow_size;
   Elm_Ctxpopup_Direction dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   Evas_Coord_Point temp;
   int idx;

   wd = elm_widget_data_get(obj);

   if ((!wd) || (!rect))
     return ELM_CTXPOPUP_DIRECTION_DOWN;

   edje_object_part_geometry_get(wd->arrow, "ctxpopup_arrow", NULL, NULL,
                                 &arrow_size.x, &arrow_size.y);
   evas_object_resize(wd->arrow, arrow_size.x, arrow_size.y);

   //Initialize Area Rectangle.
   evas_object_geometry_get(wd->parent,
                            &hover_area.x,
                            &hover_area.y,
                            &hover_area.w,
                            &hover_area.h);

   evas_object_geometry_get(obj, &pos.x, &pos.y, NULL, NULL);

   //recalc the edje
   edje_object_size_min_calc(wd->base, &base_size.x, &base_size.y);
   evas_object_smart_calculate(wd->base);

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
        switch (wd->dir_priority[idx])
          {
           case ELM_CTXPOPUP_DIRECTION_UNKNOWN:
           case ELM_CTXPOPUP_DIRECTION_UP:
              temp.y = (pos.y - base_size.y);
              if ((temp.y - arrow_size.y) < hover_area.y)
                continue;
              _adjust_pos_x(&pos, &base_size, &hover_area);
              pos.y -= base_size.y;
              dir = ELM_CTXPOPUP_DIRECTION_UP;
              break;
           case ELM_CTXPOPUP_DIRECTION_LEFT:
              temp.x = (pos.x - base_size.x);
              if ((temp.x - arrow_size.x) < hover_area.x)
                continue;
              _adjust_pos_y(&pos, &base_size, &hover_area);
              pos.x -= base_size.x;
              dir = ELM_CTXPOPUP_DIRECTION_LEFT;
              break;
           case ELM_CTXPOPUP_DIRECTION_RIGHT:
              temp.x = (pos.x + base_size.x);
              if ((temp.x + arrow_size.x) >
                  (hover_area.x + hover_area.w))
                continue;
              _adjust_pos_y(&pos, &base_size, &hover_area);
              dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
              break;
           case ELM_CTXPOPUP_DIRECTION_DOWN:
              temp.y = (pos.y + base_size.y);
              if ((temp.y + arrow_size.y) >
                  (hover_area.y + hover_area.h))
                continue;
              _adjust_pos_x(&pos, &base_size, &hover_area);
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

        if (!wd->horizontal)
          {
             length[0] = pos.y - hover_area.y;
             length[1] = (hover_area.y + hover_area.h) - pos.y;

             // ELM_CTXPOPUP_DIRECTION_UP
             if (length[0] > length[1])
               {
                  _adjust_pos_x(&pos, &base_size, &hover_area);
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
                  _adjust_pos_x(&pos, &base_size, &hover_area);
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
                  _adjust_pos_y(&pos, &base_size, &hover_area);
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
                  _adjust_pos_y(&pos, &base_size, &hover_area);
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
_update_arrow(Evas_Object *obj, Elm_Ctxpopup_Direction dir,
              Evas_Coord_Rectangle base_size)
{
   Evas_Coord x, y;
   Evas_Coord_Rectangle arrow_size;
   Widget_Data *wd;
   double drag;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_geometry_get(wd->arrow, NULL, NULL, &arrow_size.w,
                            &arrow_size.h);

   //edje_object_part_unswallow(wd->base, wd->arrow);

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
         edje_object_signal_emit(wd->arrow, "elm,state,left", "elm");
         edje_object_part_swallow(wd->base,
                                  "elm.swallow.arrow_left",
                                  wd->arrow);
         if (base_size.h > 0)
           {
              if (y < ((arrow_size.h * 0.5) + base_size.y))
                y = 0;
              else if (y > base_size.y + base_size.h - (arrow_size.h * 0.5))
                y = base_size.h - arrow_size.h;
              else
                y = y - base_size.y - (arrow_size.h * 0.5);
              drag = (double) (y) / (double) (base_size.h - arrow_size.h);
              edje_object_part_drag_value_set(wd->base,
                                              "elm.swallow.arrow_left",
                                              1,
                                              drag);
           }
         break;
      case ELM_CTXPOPUP_DIRECTION_LEFT:
         edje_object_signal_emit(wd->arrow, "elm,state,right", "elm");
         edje_object_part_swallow(wd->base,
                                  "elm.swallow.arrow_right",
                                  wd->arrow);
         if (base_size.h > 0)
           {
              if (y < ((arrow_size.h * 0.5) + base_size.y))
                y = 0;
              else if (y > (base_size.y + base_size.h - (arrow_size.h * 0.5)))
                y = base_size.h - arrow_size.h;
              else
                y = y - base_size.y - (arrow_size.h * 0.5);
              drag = (double) (y) / (double) (base_size.h - arrow_size.h);
              edje_object_part_drag_value_set(wd->base,
                                              "elm.swallow.arrow_right",
                                              0,
                                              drag);
           }
         break;
      case ELM_CTXPOPUP_DIRECTION_DOWN:
         edje_object_signal_emit(wd->arrow, "elm,state,top", "elm");
         edje_object_part_swallow(wd->base, "elm.swallow.arrow_up", wd->arrow);
         if (base_size.w > 0)
           {
              if (x < ((arrow_size.w * 0.5) + base_size.x))
                x = 0;
              else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
                x = base_size.w - arrow_size.w;
              else
                x = x - base_size.x - (arrow_size.w * 0.5);
              drag = (double) (x) / (double) (base_size.w - arrow_size.w);
              edje_object_part_drag_value_set(wd->base,
                                              "elm.swallow.arrow_up",
                                              drag,
                                              1);
           }
         break;
      case ELM_CTXPOPUP_DIRECTION_UP:
         edje_object_signal_emit(wd->arrow, "elm,state,bottom", "elm");
         edje_object_part_swallow(wd->base,
                                  "elm.swallow.arrow_down",
                                  wd->arrow);
         if (base_size.w > 0)
           {
              if (x < ((arrow_size.w * 0.5) + base_size.x))
                x = 0;
              else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
                x = base_size.w - arrow_size.w;
              else x = x - base_size.x - (arrow_size.w * 0.5);
              drag = (double) (x) / (double) (base_size.w - arrow_size.w);
              edje_object_part_drag_value_set(wd->base,
                                              "elm.swallow.arrow_down",
                                              drag,
                                              0);
           }
         break;
      default:
         break;
     }

   //should be here for getting accurate geometry value
   evas_object_smart_calculate(wd->base);
}

static void
_hide_signal_emit(Evas_Object *obj, Elm_Ctxpopup_Direction dir)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd->visible) return;

   switch (dir)
     {
        case ELM_CTXPOPUP_DIRECTION_UP:
           edje_object_signal_emit(wd->base, "elm,state,hide,up", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_LEFT:
           edje_object_signal_emit(wd->base, "elm,state,hide,left", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_RIGHT:
           edje_object_signal_emit(wd->base, "elm,state,hide,right", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_DOWN:
           edje_object_signal_emit(wd->base, "elm,state,hide,down", "elm");
           break;
        default:
           break;
     }

   edje_object_signal_emit(wd->bg, "elm,state,hide", "elm");
}

static void
_show_signal_emit(Evas_Object *obj, Elm_Ctxpopup_Direction dir)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd->visible) return;
   if ((wd->list) && (!wd->list_visible)) return;

   switch (dir)
     {
        case ELM_CTXPOPUP_DIRECTION_UP:
           edje_object_signal_emit(wd->base, "elm,state,show,up", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_LEFT:
           edje_object_signal_emit(wd->base, "elm,state,show,left", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_RIGHT:
           edje_object_signal_emit(wd->base, "elm,state,show,right", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_DOWN:
           edje_object_signal_emit(wd->base, "elm,state,show,down", "elm");
           break;
        default:
           break;
     }
   edje_object_signal_emit(wd->bg, "elm,state,show", "elm");
   edje_object_signal_emit(wd->base, "elm,state,show", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Evas_Coord_Rectangle rect = { 0, 0, 1, 1 };
   Evas_Coord_Point list_size = { 0, 0 };

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   //Base
   wd->dir = _calc_base_geometry(obj, &rect);
   _update_arrow(obj, wd->dir, rect);
   _shift_base_by_arrow(wd->arrow, wd->dir, &rect);

   if ((wd->list) && (wd->list_visible))
     {
        evas_object_geometry_get(wd->list, 0, 0, &list_size.x, &list_size.y);
        if ((list_size.x >= rect.w) || (list_size.y >= rect.h))
          {
             elm_list_mode_set(wd->list, ELM_LIST_COMPRESS);
             evas_object_size_hint_min_set(wd->box, rect.w, rect.h);
             evas_object_size_hint_min_set(obj, rect.w, rect.h);
          }
   }

   evas_object_move(wd->base, rect.x, rect.y);
   evas_object_resize(wd->base, rect.w, rect.h);
   _show_signal_emit(obj, wd->dir);
}

static void
_shift_base_by_arrow(Evas_Object *arrow, Elm_Ctxpopup_Direction dir,
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

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_event_callback_del_full(wd->box, EVAS_CALLBACK_RESIZE,
                                       _content_resize, obj);
   _parent_cut_off(obj);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_ctxpopup_clear(obj);
   evas_object_del(wd->arrow);
   evas_object_del(wd->base);
   free(wd);
}

//FIXME: lost the content size when theme hook is called.
static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   _elm_widget_mirrored_reload(obj);

   _elm_theme_object_set(obj, wd->bg, "ctxpopup", "bg",
                         elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->base, "ctxpopup", "base",
                         elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->arrow, "ctxpopup", "arrow",
                         elm_widget_style_get(obj));

   if (wd->list)
     {
        if (!strncmp(elm_object_style_get(obj), "default", strlen("default")))
           elm_object_style_set(wd->list, "ctxpopup");
        else
           elm_object_style_set(wd->list, elm_object_style_get(obj));
     }

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible)
     {
        _sizing_eval(obj);
     }
}

static void
_content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Evas_Coord min_w = -1, min_h = -1;
   Widget_Data *wd;
   if ((part) && (strcmp(part, "default"))) return;
   wd = elm_widget_data_get(obj);
   if ((!wd) || (!content)) return;
   if (content == wd->content) return;

   //TODO: wd->list
   if (wd->content) evas_object_del(wd->content);

   //Use Box
   wd->box = elm_box_add(obj);
   evas_object_size_hint_weight_set(wd->box, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(content, EVAS_HINT_FILL,
                                  EVAS_HINT_FILL);
   evas_object_show(content);
   evas_object_size_hint_min_get(content, &min_w, &min_h);
   evas_object_size_hint_min_set(wd->box, min_w, min_h);
   elm_box_pack_end(wd->box, content);

   evas_object_event_callback_add(wd->box, EVAS_CALLBACK_RESIZE,
                                  _content_resize, obj);
   evas_object_event_callback_add(wd->box, EVAS_CALLBACK_DEL,
                                  _content_del, obj);

   elm_widget_sub_object_add(obj, wd->box);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->box);

   wd->content = content;
   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible)
     _sizing_eval(obj);
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;
   Evas_Object *content;
   if ((part) && (strcmp(part, "default"))) return NULL;
   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   content = wd->content;
   if ((!content) || (!wd->box)) return NULL;

   edje_object_part_unswallow(wd->base, wd->box);
   elm_widget_sub_object_del(obj, wd->box);
   evas_object_event_callback_del(wd->box, EVAS_CALLBACK_DEL, _content_del);
   edje_object_signal_emit(wd->base, "elm,state,content,disable", "elm");

   evas_object_del(wd->box);
   wd->box = NULL;
   wd->content = NULL;
   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   return content;
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;
   if ((part) && (strcmp(part, "default"))) return NULL;
   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

static void
_item_text_set_hook(Elm_Object_Item *it, const char *part, const char *label)
{
   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it;

   if ((part) && (strcmp(part, "default"))) return;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   wd = elm_widget_data_get(WIDGET(ctxpopup_it));
   if (!wd) return;

   elm_object_item_part_text_set(ctxpopup_it->list_item, "default", label);
   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible)
     {
        _sizing_eval(WIDGET(ctxpopup_it));
     }
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it, const char *part)
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
   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it;

   if ((part) && (strcmp(part, "icon"))
       && (strcmp(part, "start"))
       && (strcmp(part, "end"))) return;

   ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   wd = elm_widget_data_get(WIDGET(ctxpopup_it));
   if (!wd) return;

   if ((part) && (!strcmp(part, "end")))
      elm_object_item_part_content_set(ctxpopup_it->list_item, "end", content);
   else
      elm_object_item_part_content_set(ctxpopup_it->list_item, "start", content);

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible)
     {
        _sizing_eval(WIDGET(ctxpopup_it));
     }
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it, const char *part)
{
   Elm_Ctxpopup_Item *ctxpopup_it;

   if (part && strcmp(part, "icon") && strcmp(part, "start")
       && strcmp(part, "end")) return NULL;

   ctxpopup_it  = (Elm_Ctxpopup_Item *)it;

   if (part && !strcmp(part, "end"))
      return elm_object_item_part_content_get(ctxpopup_it->list_item, "end");
   else
      return elm_object_item_part_content_get(ctxpopup_it->list_item, "start");
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   wd = elm_widget_data_get(WIDGET(ctxpopup_it));
   if (!wd) return;

   elm_object_item_disabled_set(ctxpopup_it->list_item,
                                elm_widget_item_disabled_get(ctxpopup_it));
}

static void
_item_signal_emit_hook(Elm_Object_Item *it, const char *emission,
                       const char *source)
{
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;
   elm_object_item_signal_emit(ctxpopup_it->list_item, emission, source);
}

static void
_bg_clicked_cb(void *data, Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _hide_signal_emit(data, wd->dir);
}

static void
_ctxpopup_show(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if ((!wd->list) && (!wd->content)) return;

   if (wd->list)
     {
        elm_list_go(wd->list);
        wd->visible = EINA_TRUE;
        elm_object_focus_set(obj, EINA_TRUE);
        return;
     }

   wd->visible = EINA_TRUE;

   evas_object_show(wd->bg);
   evas_object_show(wd->base);
   evas_object_show(wd->arrow);

   edje_object_signal_emit(wd->bg, "elm,state,show", "elm");
   edje_object_signal_emit(wd->base, "elm,state,show", "elm");

   _sizing_eval(obj);
   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_hide_finished(void *data, Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_ctxpopup_hide(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->visible)) return;

   evas_object_hide(wd->bg);
   evas_object_hide(wd->arrow);
   evas_object_hide(wd->base);

   wd->visible = EINA_FALSE;
   wd->list_visible = EINA_FALSE;
}

static void
_content_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   elm_box_recalculate(wd->box);
   _sizing_eval(data);
}

static void
_list_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj,
                 void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (!wd->visible) return;
   if (wd->list_visible) return;

   wd->list_visible = EINA_TRUE;

   evas_object_show(wd->bg);
   evas_object_show(wd->base);
   evas_object_show(wd->arrow);
   _sizing_eval(obj);
}

static void
_ctxpopup_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (wd->visible)
     evas_object_show(wd->arrow);

   _sizing_eval(obj);
}

static void
_restack(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_layer_set(wd->bg,
                         evas_object_layer_get(obj));
   evas_object_layer_set(wd->base,
                         evas_object_layer_get(obj));
}

static void
_content_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   elm_object_content_unset(data);
}

static void
_list_del(Widget_Data *wd)
{
   if (!wd->list) return;

   edje_object_part_unswallow(wd->base, wd->box);
   elm_box_unpack(wd->box, wd->list);
   evas_object_del(wd->list);
   wd->list = NULL;
   wd->box = NULL;
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Evas_Object *list;
   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it = (Elm_Ctxpopup_Item *)it;

   wd = elm_widget_data_get(WIDGET(ctxpopup_it));
   if (!wd) return EINA_FALSE;

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   list = elm_object_item_widget_get(ctxpopup_it->list_item);

   if (eina_list_count(elm_list_items_get(list)) < 2)
     {
        elm_object_item_del(ctxpopup_it->list_item);
        evas_object_hide(WIDGET(ctxpopup_it));
        return EINA_TRUE;
     }

   elm_object_item_del(ctxpopup_it->list_item);
   if (wd->list_visible)
     _sizing_eval(WIDGET(ctxpopup_it));

   return EINA_TRUE;
}

static void
_disable_hook(Evas_Object *obj)
{
   //TODO: elm_object_disabled_set(); does not ignite this part
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_object_disabled_set(wd->list, elm_widget_disabled_get(obj));
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->base, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(wd->base, emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_del_full(wd->base, emission, source, func_cb, data);
}

EAPI Evas_Object *
elm_ctxpopup_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "ctxpopup");
   elm_widget_type_set(obj, "ctxpopup");
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_focus_next_hook_set(obj, _focus_next_hook);

   //Background
   wd->bg = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->bg);
   _elm_theme_object_set(obj, wd->bg, "ctxpopup", "bg", "default");
   edje_object_signal_callback_add(wd->bg,
                                   "elm,action,click",
                                   "",
                                   _bg_clicked_cb,
                                    obj);
   //Base
   wd->base = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->base);
   _elm_theme_object_set(obj, wd->base, "ctxpopup", "base", "default");
   edje_object_signal_callback_add(wd->base, "elm,action,hide,finished", "",
                                   _hide_finished, obj);

   //Arrow
   wd->arrow = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->arrow);
   _elm_theme_object_set(obj, wd->arrow, "ctxpopup", "arrow", "default");

   wd->dir_priority[0] = ELM_CTXPOPUP_DIRECTION_UP;
   wd->dir_priority[1] = ELM_CTXPOPUP_DIRECTION_LEFT;
   wd->dir_priority[2] = ELM_CTXPOPUP_DIRECTION_RIGHT;
   wd->dir_priority[3] = ELM_CTXPOPUP_DIRECTION_DOWN;
   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _ctxpopup_show,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _ctxpopup_hide,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _ctxpopup_move,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESTACK, _restack, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   //default parent is to be hover parent
   elm_ctxpopup_hover_parent_set(obj, parent);

   return obj;
}

EAPI void
elm_ctxpopup_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;
   Evas_Coord x, y, w, h;

   wd = elm_widget_data_get(obj);
   if ((!wd) || (!parent)) return;

   _parent_cut_off(obj);

   evas_object_event_callback_add(parent,
                                  EVAS_CALLBACK_DEL,
                                  _parent_del,
                                  obj);
   evas_object_event_callback_add(parent,
                                  EVAS_CALLBACK_MOVE,
                                  _parent_move,
                                  obj);
   evas_object_event_callback_add(parent,
                                  EVAS_CALLBACK_RESIZE,
                                  _parent_resize,
                                  obj);

   elm_widget_sub_object_add(parent, obj);
   wd->parent = parent;

   //Update Background
   evas_object_geometry_get(parent, &x, &y, &w, &h);
   evas_object_move(wd->bg, x, y);
   evas_object_resize(wd->bg, w, h);

   if (wd->visible) _sizing_eval(obj);
}

EAPI Evas_Object *
elm_ctxpopup_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->parent;
}

EAPI void
elm_ctxpopup_clear(Evas_Object * obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   _list_del(wd);
   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;
}

EAPI void
elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->horizontal = !!horizontal;

   if (!wd->list) return;

   elm_list_horizontal_set(wd->list, wd->horizontal);

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible) _sizing_eval(obj);
}

EAPI Eina_Bool
elm_ctxpopup_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return wd->horizontal;
}

EAPI Elm_Object_Item *
elm_ctxpopup_item_append(Evas_Object *obj, const char *label,
                         Evas_Object *icon, Evas_Smart_Cb func,
                         const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;
   Elm_Ctxpopup_Item *item;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = elm_widget_item_new(obj, Elm_Ctxpopup_Item);
   if (!item) return NULL;

   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(item, _item_disable_hook);
   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(item, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(item, _item_content_get_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);

   if (!wd->list)
     {
        //The first item is appended.
        wd->list = elm_list_add(obj);
        elm_list_mode_set(wd->list, ELM_LIST_EXPAND);
        elm_list_horizontal_set(wd->list, wd->horizontal);
        evas_object_event_callback_add(wd->list, EVAS_CALLBACK_RESIZE,
                                      _list_resize, obj);
        _content_set_hook(obj, "default", wd->list);
     }

   item->list_item = elm_list_item_append(wd->list, label, icon, NULL, func, data);

   wd->dir = ELM_CTXPOPUP_DIRECTION_UNKNOWN;

   if (wd->visible) _sizing_eval(obj);

   return (Elm_Object_Item *)item;
}

EAPI void
elm_ctxpopup_direction_priority_set(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction first,
                                    Elm_Ctxpopup_Direction second,
                                    Elm_Ctxpopup_Direction third,
                                    Elm_Ctxpopup_Direction fourth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->dir_priority[0] = first;
   wd->dir_priority[1] = second;
   wd->dir_priority[2] = third;
   wd->dir_priority[3] = fourth;

   if (wd->visible)
     _sizing_eval(obj);
}

EAPI void
elm_ctxpopup_direction_priority_get(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction *first,
                                    Elm_Ctxpopup_Direction *second,
                                    Elm_Ctxpopup_Direction *third,
                                    Elm_Ctxpopup_Direction *fourth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (first) *first = wd->dir_priority[0];
   if (second) *second = wd->dir_priority[1];
   if (third) *third = wd->dir_priority[2];
   if (fourth) *fourth = wd->dir_priority[3];
}

EAPI Elm_Ctxpopup_Direction
elm_ctxpopup_direction_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return ELM_CTXPOPUP_DIRECTION_UNKNOWN;
   return wd->dir;
}

EAPI void
elm_ctxpopup_dismiss(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _hide_signal_emit(obj, wd->dir);
}
