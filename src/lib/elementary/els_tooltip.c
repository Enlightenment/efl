#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"

#ifdef ISCOMFITOR
# define STR(X) #X
# define STUPID(X) STR(X)
# define TTDBG(x...) fprintf(stderr, STUPID(__LINE__)": " x)
#else
# define TTDBG(X...)
#endif

static const char _tooltip_key[] = "_elm_tooltip";

#define ELM_TOOLTIP_GET_OR_RETURN(tt, obj, ...)         \
  Elm_Tooltip *tt;                                      \
  do                                                    \
    {                                                   \
       if (!(obj))                                      \
         {                                              \
            CRI("Null pointer: " #obj);            \
            return __VA_ARGS__;                         \
         }                                              \
       tt = evas_object_data_get((obj), _tooltip_key);  \
       if (!tt)                                         \
         {                                              \
            ERR("Object does not have tooltip: " #obj); \
            return __VA_ARGS__;                         \
         }                                              \
    }                                                   \
  while (0)

#define ELM_TOOLTIP_GET_OR_CREATE(tt, obj, ...)         \
  Elm_Tooltip *tt;                                      \
  do                                                    \
    {                                                   \
       if (!(obj))                                      \
         {                                              \
            CRI("Null pointer: " #obj);            \
            return __VA_ARGS__;                         \
         }                                              \
       tt = evas_object_data_get((obj), _tooltip_key);  \
       if (!tt)                                         \
         {                                              \
            tt = _elm_tooltip_create((obj));     \
         }                                              \
    }                                                   \
  while (0)

struct _Elm_Tooltip
{
   Elm_Tooltip_Content_Cb   func;
   Evas_Smart_Cb            del_cb;
   const void              *data;
   const char              *style;
   Evas                    *evas, *tt_evas;
   Evas_Object             *eventarea, *owner;
   Evas_Object             *tooltip, *content;
   Evas_Object             *tt_win;
   Ecore_Timer             *show_timer;
   Ecore_Timer             *hide_timer;
   Ecore_Job               *reconfigure_job;
   Evas_Coord               mouse_x, mouse_y;
   struct
     {
        Evas_Coord            x, y, bx, by;
     } pad;
   struct
     {
        double                x, y;
     } rel_pos;
   Elm_Tooltip_Orient       orient; /** orientation for tooltip */
   int                      move_freeze;
   unsigned short           ref;

   double                   hide_timeout; /* from theme */
   Eina_Bool                visible_lock:1;
   Eina_Bool                changed_style:1;
   Eina_Bool                free_size : 1;
   Eina_Bool                unset_me : 1;
};

static void _elm_tooltip_reconfigure(Elm_Tooltip *tt);
static void _elm_tooltip_reconfigure_job_start(Elm_Tooltip *tt);
static void _elm_tooltip_reconfigure_job_stop(Elm_Tooltip *tt);
static void _elm_tooltip_hide_anim_start(Elm_Tooltip *tt);
static void _elm_tooltip_hide_anim_stop(Elm_Tooltip *tt);
static void _elm_tooltip_show_timer_stop(Elm_Tooltip *tt);
static void _elm_tooltip_hide(Elm_Tooltip *tt);
static void _elm_tooltip_data_clean(Elm_Tooltip *tt);
static void _elm_tooltip_unset(Elm_Tooltip *tt);

static void
_elm_tooltip_content_changed_hints_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _elm_tooltip_reconfigure_job_start(data);
   TTDBG("HINTS CHANGED\n");
}

static void
_elm_tooltip_content_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Tooltip *tt = data;
   tt->content = NULL;
   tt->visible_lock = EINA_FALSE;
   if (tt->tooltip) _elm_tooltip_hide(tt);
}

static void
_elm_tooltip_obj_move_cb(void *data, Evas *e  EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info  EINA_UNUSED)
{
   Elm_Tooltip *tt = data;
   _elm_tooltip_reconfigure_job_start(tt);
   TTDBG("TT MOVED\n");
}

static void
_elm_tooltip_obj_resize_cb(void *data, Evas *e  EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info  EINA_UNUSED)
{
   Elm_Tooltip *tt = data;
   _elm_tooltip_reconfigure_job_start(tt);
   TTDBG("TT RESIZE\n");
}

static void
_elm_tooltip_obj_mouse_move_cb(void *data, Evas *e  EINA_UNUSED,
                               Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Tooltip *tt = data;
   Evas_Event_Mouse_Move *ev = event_info;

   if (tt->mouse_x || tt->mouse_y)
     {
        if ((abs(ev->cur.output.x - tt->mouse_x) < 3) &&
            (abs(ev->cur.output.y - tt->mouse_y) < 3))
          {
             TTDBG("MOUSE MOVE REJECTED!\n");
             return;
          }
     }
   tt->mouse_x = ev->cur.output.x;
   tt->mouse_y = ev->cur.output.y;
   TTDBG("MOUSE MOVED\n");
   _elm_tooltip_reconfigure_job_start(tt);
}

static void
_elm_tooltip_show(Elm_Tooltip *tt)
{
   _elm_tooltip_show_timer_stop(tt);
   _elm_tooltip_hide_anim_stop(tt);

   TTDBG("TT SHOW\n");
   if (tt->tooltip)
     {
        _elm_tooltip_reconfigure_job_start(tt);
        TTDBG("RECURSIVE JOB\n");
        return;
     }
   if (tt->free_size)
     {
        tt->tt_win = elm_win_add(elm_win_get(tt->owner), "tooltip", ELM_WIN_TOOLTIP);
        elm_win_override_set(tt->tt_win, EINA_TRUE);
        tt->tt_evas = evas_object_evas_get(tt->tt_win);
        tt->tooltip = edje_object_add(tt->tt_evas);
        evas_object_size_hint_weight_set(tt->tooltip, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_win_resize_object_add(tt->tt_win, tt->tooltip);
     }
   else
      tt->tooltip = edje_object_add(tt->evas);
   if (!tt->tooltip) return;
   evas_object_pass_events_set(tt->tooltip, EINA_TRUE);

   if (tt->free_size)
     evas_object_layer_set(tt->tooltip, ELM_OBJECT_LAYER_TOOLTIP);

   evas_object_event_callback_add
     (tt->eventarea, EVAS_CALLBACK_MOVE, _elm_tooltip_obj_move_cb, tt);
   evas_object_event_callback_add
     (tt->eventarea, EVAS_CALLBACK_RESIZE, _elm_tooltip_obj_resize_cb, tt);

   if (tt->move_freeze == 0)
     {
      //No movement of tooltip upon mouse move if orientation set
      if ((tt->orient <= ELM_TOOLTIP_ORIENT_NONE) || (tt->orient >= ELM_TOOLTIP_ORIENT_LAST))
        {
           evas_object_event_callback_add(tt->eventarea,
                                          EVAS_CALLBACK_MOUSE_MOVE,
                                          _elm_tooltip_obj_mouse_move_cb, tt);
        }
     }
   tt->changed_style = EINA_TRUE;
   _elm_tooltip_reconfigure_job_start(tt);
}

static void
_elm_tooltip_content_del(Elm_Tooltip *tt)
{
   if (!tt->content) return;

   TTDBG("CONTENT DEL\n");
   evas_object_event_callback_del_full
     (tt->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _elm_tooltip_content_changed_hints_cb, tt);
   evas_object_event_callback_del_full
     (tt->content, EVAS_CALLBACK_DEL,
      _elm_tooltip_content_del_cb, tt);
   evas_object_hide(tt->content);
   ELM_SAFE_FREE(tt->content, evas_object_del);
}

static void
_elm_tooltip_hide(Elm_Tooltip *tt)
{
   Evas_Object *del;
   TTDBG("TT HIDE\n");
   _elm_tooltip_show_timer_stop(tt);
   _elm_tooltip_hide_anim_stop(tt);
   _elm_tooltip_reconfigure_job_stop(tt);

   if (!tt->tooltip) return;
   if (tt->visible_lock) return;

   _elm_tooltip_content_del(tt);

   evas_object_event_callback_del_full
     (tt->eventarea, EVAS_CALLBACK_MOVE, _elm_tooltip_obj_move_cb, tt);
   evas_object_event_callback_del_full
     (tt->eventarea, EVAS_CALLBACK_RESIZE, _elm_tooltip_obj_resize_cb, tt);
   evas_object_event_callback_del_full
     (tt->eventarea, EVAS_CALLBACK_MOUSE_MOVE, (Evas_Object_Event_Cb)_elm_tooltip_obj_mouse_move_cb, tt);

   del = tt->tt_win ? tt->tt_win : tt->tooltip;

   tt->tt_win = NULL;
   tt->tt_evas = NULL;
   tt->tooltip = NULL;
   evas_object_del(del);
}

static void
_elm_tooltip_reconfigure_job(void *data)
{
   Elm_Tooltip *tt = data;
   tt->reconfigure_job = NULL;
   _elm_tooltip_reconfigure(data);
}

static void
_elm_tooltip_reconfigure_job_stop(Elm_Tooltip *tt)
{
   ELM_SAFE_FREE(tt->reconfigure_job, ecore_job_del);
}

static void
_elm_tooltip_reconfigure_job_start(Elm_Tooltip *tt)
{
   ecore_job_del(tt->reconfigure_job);
   tt->reconfigure_job = ecore_job_add(_elm_tooltip_reconfigure_job, tt);
}

static Eina_Bool
_elm_tooltip_hide_anim_cb(void *data)
{
   Elm_Tooltip *tt = data;
   tt->hide_timer = NULL;
   _elm_tooltip_hide(tt);
   return EINA_FALSE;
}

static void
_elm_tooltip_hide_anim_start(Elm_Tooltip *tt)
{
   double extra = 0;
   if (tt->hide_timer) return;
   TTDBG("HIDE START\n");
   /* hide slightly faster when in window mode to look less stupid */
   if ((tt->hide_timeout > 0) && tt->tt_win) extra = 0.1;

   if (elm_widget_is_legacy(tt->owner))
     edje_object_signal_emit(tt->tooltip, "elm,action,hide", "elm");
   else
     edje_object_signal_emit(tt->tooltip, "efl,action,hide", "efl");
   tt->hide_timer = ecore_timer_add
     (tt->hide_timeout - extra, _elm_tooltip_hide_anim_cb, tt);
}

static void
_elm_tooltip_hide_anim_stop(Elm_Tooltip *tt)
{
   if (!tt->hide_timer) return;
   if (tt->tooltip)
     {
        if (elm_widget_is_legacy(tt->owner))
          edje_object_signal_emit(tt->tooltip, "elm,action,show", "elm");
        else
          edje_object_signal_emit(tt->tooltip, "efl,action,show", "efl");
     }

   ELM_SAFE_FREE(tt->hide_timer, ecore_timer_del);
}

static void
_elm_tooltip_reconfigure_orient(Elm_Tooltip *tt,
                                Evas_Coord ox, Evas_Coord oy, Evas_Coord ow, Evas_Coord oh,
                                Evas_Coord tw, Evas_Coord th, Evas_Coord cw, Evas_Coord ch)
{
   Evas_Coord mx, my;
   Evas_Coord dx, dy;
   Evas_Coord tcw, tch;
   Evas_Coord px, py;

   switch (tt->orient)
     {
      case ELM_TOOLTIP_ORIENT_TOP_LEFT:
         mx = ox - tw;
         my = oy - th;
         tt->rel_pos.x = 1.1;
         tt->rel_pos.y = 1.1;
         break;
      case ELM_TOOLTIP_ORIENT_TOP:
         mx = ox + ((ow - tw) / 2);
         my = oy - th;
         tt->rel_pos.x = 0.5;
         tt->rel_pos.y = 1.1;
         break;
      case ELM_TOOLTIP_ORIENT_TOP_RIGHT:
         mx = ox + ow;
         my = oy - th;
         tt->rel_pos.x = -1.1;
         tt->rel_pos.y = 1.1;
         break;
      case ELM_TOOLTIP_ORIENT_LEFT:
         mx = ox - tw;
         my = oy + ((oh - th) / 2);
         tt->rel_pos.x = 1.1;
         tt->rel_pos.y = 0.5;
         break;
      case ELM_TOOLTIP_ORIENT_CENTER:
         mx = ox + ((ow - tw) / 2);
         my = oy + ((oh - th) / 2);
         tt->rel_pos.x = 0.5;
         tt->rel_pos.y = 0.5;
         break;
      case ELM_TOOLTIP_ORIENT_RIGHT:
         mx = ox + ow;
         my = oy + ((oh - th) / 2);
         tt->rel_pos.x = -1.1;
         tt->rel_pos.y = 0.5;
         break;
      case ELM_TOOLTIP_ORIENT_BOTTOM_LEFT:
         mx = ox - tw;
         my = oy + oh;
         tt->rel_pos.x = 1.1;
         tt->rel_pos.y = -1.1;
         break;
      case ELM_TOOLTIP_ORIENT_BOTTOM:
         mx = ox + ((ow - tw) / 2);
         my = oy + oh;
         tt->rel_pos.x = 0.5;
         tt->rel_pos.y = -1.1;
         break;
      case ELM_TOOLTIP_ORIENT_BOTTOM_RIGHT:
         mx = ox + ow;
         my = oy + oh;
         tt->rel_pos.x = -1.1;
         tt->rel_pos.y = -1.1;
         break;
      default:
         return;
     }

   evas_object_geometry_get(tt->content, NULL, NULL, &tcw, &tch);
   if (tcw <= 0 || tcw > tw) tcw = tw;
   if (tch <= 0 || tch > th) tch = th;

   px = (tw - tcw) / 2;
   py = (th - tch) / 2;

   if (mx < 0)
     {
        dx = -mx;
        mx = -(px / 2);
        if (tt->rel_pos.x == 0.5)
          {
             tt->rel_pos.x = 0.5 - dx / (double)tcw;
             if (tt->rel_pos.x < 0.0) tt->rel_pos.x = 0.0;
          }
     }
   else if (mx + tw > cw)
     {
        dx = mx + tw - cw;
        mx = cw - tw + px / 2;
        if (tt->rel_pos.x == 0.5)
          {
             tt->rel_pos.x = 0.5 + dx / (double)tcw;
             if (tt->rel_pos.x > 1.0) tt->rel_pos.x = 1.0;
          }
     }

   if (my < 0)
     {
        dy = -my;
        my = -(py / 2);
        if (tt->rel_pos.y == 0.5)
          {
             tt->rel_pos.y = 0.5 - dy / (double)tch;
             if (tt->rel_pos.y < 0.0) tt->rel_pos.y = 0.0;
          }
     }
   else if (my + th > ch)
     {
        dy = my + th - ch;
        my = ch - th + py / 2;
        if (tt->rel_pos.y == 0.5)
          {
             tt->rel_pos.y = 0.5 + dy / (double)tch;
             if (tt->rel_pos.y > 1.0) tt->rel_pos.y = 1.0;
          }
     }

   evas_object_move(tt->tooltip, mx, my);
   evas_object_show(tt->tooltip);
}

static void
_elm_tooltip_reconfigure(Elm_Tooltip *tt)
{
   Evas_Coord ox, oy, ow, oh, px = 0, py = 0, tx, ty, tw, th;
   Evas_Coord cx = 0, cy = 0, cw = 0, ch = 0, basex = 0, basey = 0;;
   Evas_Coord eminw, eminh, ominw, ominh;
   double rel_x = 0.0, rel_y = 0.0;
   Eina_Bool inside_eventarea;
   Eina_Bool new_content = EINA_FALSE;

   _elm_tooltip_reconfigure_job_stop(tt);

   if (tt->hide_timer) return;
   if (!tt->tooltip) return;
   if (tt->changed_style)
     {
        const char *style = tt->style ? tt->style : "default";
        const char *str;
        if (!_elm_theme_object_set(tt->tt_win ? : tt->owner, tt->tooltip,
                                  "tooltip", NULL, style))
          {
             ERR("Could not apply the theme to the tooltip! style=%s", style);
             if (tt->tt_win) evas_object_del(tt->tt_win);
             else evas_object_del(tt->tooltip);
             tt->tt_win = NULL;
             tt->tt_evas = NULL;
             tt->tooltip = NULL;
             return;
          }

        tt->rel_pos.x = 0;
        tt->rel_pos.y = 0;

        tt->pad.x = 0;
        tt->pad.y = 0;
        tt->pad.bx = 0;
        tt->pad.by = 0;
        tt->hide_timeout = 0.0;

        str = edje_object_data_get(tt->tooltip, "transparent");
        if (tt->tt_win)
          {  /* FIXME: hardcoded here is bad */
             if (str && (!strcmp(str, "enabled")))
               {
                  evas_object_hide(tt->tt_win);
                  elm_win_alpha_set(tt->tt_win, EINA_TRUE);
               }
             else
               {
                  evas_object_hide(tt->tt_win);
                  elm_win_alpha_set(tt->tt_win, EINA_FALSE);
               }
          }

        str = edje_object_data_get(tt->tooltip, "pad_x");
        if (str) tt->pad.x = atoi(str);
        str = edje_object_data_get(tt->tooltip, "pad_y");
        if (str) tt->pad.y = atoi(str);

        str = edje_object_data_get(tt->tooltip, "pad_border_x");
        if (str) tt->pad.bx = atoi(str);
        str = edje_object_data_get(tt->tooltip, "pad_border_y");
        if (str) tt->pad.by = atoi(str);

        str = edje_object_data_get(tt->tooltip, "hide_timeout");
        if (str)
          {
             tt->hide_timeout = _elm_atof(str);
             if (tt->hide_timeout < 0.0) tt->hide_timeout = 0.0;
          }

        tt->changed_style = EINA_FALSE;
        if (tt->tooltip)
          {
             if (elm_widget_is_legacy(tt->owner))
               edje_object_part_swallow(tt->tooltip, "elm.swallow.content",
                                        tt->content);
             else
               edje_object_part_swallow(tt->tooltip, "efl.content",
                                        tt->content);
          }

        if (elm_widget_is_legacy(tt->owner))
          edje_object_signal_emit(tt->tooltip, "elm,action,show", "elm");
        else
          edje_object_signal_emit(tt->tooltip, "efl,action,show", "efl");
     }

   if (!tt->content)
     {
        tt->ref++;
        tt->content = tt->func((void *)tt->data, tt->owner, tt->tt_win ? : tt->owner);
        tt->ref--;
        if (tt->unset_me)
          {
             _elm_tooltip_unset(tt);
             return;
          }

        if (!tt->content)
          {
             WRN("could not create tooltip content!");
             if (tt->tt_win) evas_object_del(tt->tt_win);
             else evas_object_del(tt->tooltip);

             tt->tt_win = NULL;
             tt->tt_evas = NULL;
             tt->tooltip = NULL;
             return;
          }

        if (elm_widget_is_legacy(tt->owner))
          edje_object_part_swallow
             (tt->tooltip, "elm.swallow.content", tt->content);
        else
          edje_object_part_swallow
             (tt->tooltip, "efl.content", tt->content);
        new_content = EINA_TRUE;
        evas_object_event_callback_add(tt->content, EVAS_CALLBACK_DEL,
           _elm_tooltip_content_del_cb, tt);

        /* tooltip has to use layer tooltip */
        evas_object_layer_set(tt->tooltip, ELM_OBJECT_LAYER_TOOLTIP);
     }
   TTDBG("*******RECALC\n");
   evas_object_size_hint_combined_min_get(tt->content, &ominw, &ominh);
   /* force size hints to update */
   if ((!ominw) || (!ominh))
     {
        evas_object_smart_need_recalculate_set(tt->content, 1);
        evas_object_smart_calculate(tt->content);
        evas_object_size_hint_combined_min_get(tt->content, &ominw, &ominh);
     }
   if (new_content)
     evas_object_event_callback_add(tt->content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
           _elm_tooltip_content_changed_hints_cb, tt);
   edje_object_size_min_get(tt->tooltip, &eminw, &eminh);

   if (eminw && (ominw < eminw)) ominw = eminw;
   if (eminh && (ominh < eminh)) ominh = eminh;

   edje_object_size_min_restricted_calc(tt->tooltip, &tw, &th, ominw, ominh);
   TTDBG("TTSIZE:  tw=%d,th=%d,ominw=%d,ominh=%d\n", tw, th, ominw, ominh);

   if (tt->tt_win)
     {
        elm_win_screen_size_get(elm_widget_top_get(tt->owner),
                                &basex, &basey, &cw, &ch);
        elm_win_screen_position_get(elm_widget_top_get(tt->owner),
                                    &cx, &cy);
        evas_pointer_canvas_xy_get(tt->evas, &px, &py);
        cx -= basex;
        cy -= basey;
     }
   else
     {
        evas_output_size_get(tt->evas, &cw, &ch);
        evas_pointer_canvas_xy_get(tt->evas, &px, &py);
     }
   TTDBG("SCREEN:  cw=%d,ch=%d\n", cw, ch);

   evas_object_geometry_get(tt->eventarea, &ox, &oy, &ow, &oh);
   /* win reports its screen position for x/y;
    * reset to 0 since we expect canvas coords here
    */
   if (efl_isa(tt->eventarea, EFL_UI_WIN_CLASS))
     ox = oy = 0;
   TTDBG("EVENTAREA:  ox=%d,oy=%d,ow=%d,oh=%d\n", ox, oy, ow, oh);

   inside_eventarea = ((px >= ox) && (py >= oy) &&
                       (px <= (ox + ow)) && (py <= (oy + oh)));

   if (inside_eventarea)
     {
        /* try to position bottom right corner at pointer */
        tx = cx + px - tw - 1;
        ty = cy + py - th - 1;
        if (tx < 0)
          {
             tx = 0;
             if (ELM_RECTS_INTERSECT(tx, ty, tw, th, (cx + ox), (cy + oy),
                                     ow, oh))
               tx = cx + ox + ow;
          }
        if (ty < 0)
          {
             ty = 0;
             if (ELM_RECTS_INTERSECT(tx, ty, tw, th, (cx + ox), (cy + oy),
                                     ow, oh))
               ty = cy + oy + oh;
          }
        if ((tx + tw) > cw) tx = cw - tw;
        if ((ty + th) > ch) ty = ch - th;
        if (tx  < 0) tx = 0;
        if (ty  < 0) ty = 0;
     }
   else
     {
        /* try centered on middle of eventarea */
        tx = cx + ox + (ow / 2) - (tw / 2);
        if (py < oy)
          {
             ty = cx + oy - th;
             if (ty < cx) ty = cx + oy + oh;
             if ((ty + th) > (cx + ch)) ty = cy + ch - th;
          }
        else
          {
             ty = cy + oy + oh;
             if (ty < cy) ty = cy;
             if ((ty + th) > (cy + ch)) ty = cy + oy - th;
          }
        if (tx < cx) tx = cx;
        if ((tx + th) > (cx + cw)) tx = cy + cw - tw;

     }
   // jf tt is over the pointer even after positiong then screen
   // limiting, just use the poitner dumbly and choose to theleft or right
   // above or below depending which has more space/ we're in a mess
   // anyway
   if (ELM_RECTS_INTERSECT(tx, ty, tw, th, (cx + px), (cy + py), 1, 1))
     {
        if ((px + cx) > (cw / 2)) tx = cx + px - 1 - tw;
        else tx = cx + px + 1;
        if ((py + cy) > (ch / 2)) ty = cy + py - 1 - th;
        else ty = cy + py + 1;
     }

   if (inside_eventarea)
     {
        rel_x = (px - (tx - cx)) / (double)tw;
        rel_y = (py - (ty - cy)) / (double)th;
     }
   else
     {
        rel_x = (ox + (ow / 2) - (tx - cx)) / (double)tw;
        rel_y = (oy + (oh / 2) - (ty - cy)) / (double)th;
     }

   tx += basex;
   ty += basey;
   // XXX: if this is a window for toolkit this relies on abs positioning
   // and this is not portable to wayland so we need relative positioning
   // implemented lower down for this
   evas_object_geometry_set(tt->tt_win ? : tt->tooltip, tx, ty, tw, th);
   TTDBG("FINAL: tx=%d,ty=%d,tw=%d,th=%d\n", tx, ty, tw, th);
   evas_object_show(tt->tooltip);

#define FDIF(a, b) (fabs((a) - (b)) > 0.0001)
   if ((FDIF(rel_x, tt->rel_pos.x)) || (FDIF(rel_y, tt->rel_pos.y)))
     {
        Edje_Message_Float_Set *msg;

        msg = alloca(sizeof(Edje_Message_Float_Set) + sizeof(double));
        msg->count = 2;

        tt->rel_pos.x = rel_x;
        tt->rel_pos.y = rel_y;

        _elm_tooltip_reconfigure_orient(tt,
                                        cx + ox, cy + oy, ow, oh,
                                        tw, th, cw, ch);

        msg->val[0] = tt->rel_pos.x;
        msg->val[1] = tt->rel_pos.y;

        edje_object_message_send(tt->tooltip, EDJE_MESSAGE_FLOAT_SET, 1, msg);
     }
#undef FDIF
   if (tt->tt_win) evas_object_show(tt->tt_win);
}

static void
_elm_tooltip_show_timer_stop(Elm_Tooltip *tt)
{
   if (!tt->show_timer) return;
   ELM_SAFE_FREE(tt->show_timer, ecore_timer_del);
}

static Eina_Bool
_elm_tooltip_timer_show_cb(void *data)
{
   Elm_Tooltip *tt = data;
   tt->show_timer = NULL;
   _elm_tooltip_show(tt);
   return ECORE_CALLBACK_CANCEL;
}

static void
_elm_tooltip_obj_mouse_in_cb(void *data, Evas *e  EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info  EINA_UNUSED)
{
   Elm_Tooltip *tt = data;

   _elm_tooltip_hide_anim_stop(tt);

   if ((tt->show_timer) || (tt->tooltip)) return;

   tt->show_timer = ecore_timer_add(_elm_config->tooltip_delay, _elm_tooltip_timer_show_cb, tt);
   TTDBG("MOUSE IN\n");
}

static void
_elm_tooltip_obj_mouse_out_cb(Elm_Tooltip *tt, Evas *e  EINA_UNUSED, Evas_Object *obj EINA_UNUSED, Evas_Event_Mouse_Out *event EINA_UNUSED)
{
   if (tt->visible_lock) return;

   if (!tt->tooltip)
     {
        _elm_tooltip_show_timer_stop(tt);
        return;
     }
   _elm_tooltip_hide_anim_start(tt);
   TTDBG("MOUSE OUT\n");
}

static void _elm_tooltip_obj_free_cb(void *data, Evas *e  EINA_UNUSED, Evas_Object *obj, void *event_info  EINA_UNUSED);

static void
_elm_tooltip_unset(Elm_Tooltip *tt)
{
   if (tt->ref > 0)
     {
        tt->unset_me = EINA_TRUE;
        return;
     }
   tt->visible_lock = EINA_FALSE;
   _elm_tooltip_hide(tt);
   _elm_tooltip_data_clean(tt);

   if (tt->eventarea)
     {
        evas_object_event_callback_del_full
          (tt->eventarea, EVAS_CALLBACK_MOUSE_IN,
           _elm_tooltip_obj_mouse_in_cb, tt);
        evas_object_event_callback_del_full
          (tt->eventarea, EVAS_CALLBACK_MOUSE_OUT,
           (Evas_Object_Event_Cb)_elm_tooltip_obj_mouse_out_cb, tt);
        evas_object_event_callback_del_full
          (tt->eventarea, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

        evas_object_data_del(tt->eventarea, _tooltip_key);
     }
   if (tt->owner)
     {
        evas_object_event_callback_del_full
          (tt->owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);
        elm_widget_tooltip_del(tt->owner, tt);
     }

   eina_stringshare_del(tt->style);
   free(tt);
}

static void
_elm_tooltip_obj_free_cb(void *data, Evas *e  EINA_UNUSED, Evas_Object *obj, void *event_info  EINA_UNUSED)
{
   Elm_Tooltip *tt = data;
   if (tt->eventarea == obj) tt->eventarea = NULL;
   if (tt->owner == obj) tt->owner = NULL;
   _elm_tooltip_unset(tt);
}

static Elm_Tooltip *
_elm_tooltip_create(Evas_Object *eventarea)
{
   Elm_Tooltip *tt = NULL;

   tt = ELM_NEW(Elm_Tooltip);
   if (!tt) return NULL;

   tt->eventarea = eventarea;
   tt->evas = evas_object_evas_get(eventarea);
   evas_object_data_set(eventarea, _tooltip_key, tt);

   evas_object_event_callback_add(eventarea, EVAS_CALLBACK_MOUSE_IN,
      _elm_tooltip_obj_mouse_in_cb, tt);
   evas_object_event_callback_add(eventarea, EVAS_CALLBACK_MOUSE_OUT,
      (Evas_Object_Event_Cb)_elm_tooltip_obj_mouse_out_cb, tt);
   evas_object_event_callback_add(eventarea, EVAS_CALLBACK_FREE,
      _elm_tooltip_obj_free_cb, tt);

   return tt;
}

static void
_tooltip_label_style_set(Evas_Object *obj, Evas_Object *label)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   char buf[100] = {0};
   const char *style = tt->style ? tt->style : "default";

   sprintf(buf, "tooltip/%s", style);
   if (!elm_object_style_set(label, buf))
     {
        WRN("Failed to set tooltip label style: %s, reverting to old style",
            buf);
        elm_object_style_set(label, "tooltip"); //XXX: remove it in EFL 2.0
     }
}

static Evas_Object *
_elm_tooltip_label_create(void *data, Evas_Object *obj, Evas_Object *tooltip)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   _tooltip_label_style_set(obj, label);
   elm_object_text_set(label, data);
   return label;
}

static Evas_Object *
_elm_tooltip_trans_label_create(void *data, Evas_Object *obj, Evas_Object *tooltip)
{
   Evas_Object *label = elm_label_add(tooltip);
   const char **text = data;
   if (!label)
     return NULL;
   _tooltip_label_style_set(obj, label);
   elm_object_domain_translatable_text_set(label, text[0], text[1]);
   return label;
}

static void
_elm_tooltip_label_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   eina_stringshare_del(data);
}

static void
_elm_tooltip_trans_label_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   const char **text = data;
   eina_stringshare_del(text[0]);
   eina_stringshare_del(text[1]);
   free(text);
}

static void
_elm_tooltip_data_clean(Elm_Tooltip *tt)
{
   if (tt->del_cb) tt->del_cb((void *)tt->data, tt->owner, NULL);
   tt->del_cb = NULL;
   tt->data = NULL;

   _elm_tooltip_content_del(tt);
}

EAPI void
elm_object_tooltip_move_freeze_push(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);

   tt->move_freeze++;
}

EAPI void
elm_object_tooltip_move_freeze_pop(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);

   tt->move_freeze--;
   if (tt->move_freeze < 0) tt->move_freeze = 0;
}

EAPI int
elm_object_tooltip_move_freeze_get(const Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj, 0);

   return tt->move_freeze;
}

EAPI void
elm_object_tooltip_orient_set(Evas_Object *obj, Elm_Tooltip_Orient orient)
{
   ELM_TOOLTIP_GET_OR_CREATE(tt, obj);

   if ((orient > ELM_TOOLTIP_ORIENT_NONE) && (orient < ELM_TOOLTIP_ORIENT_LAST))
     tt->orient = orient;
   else
     tt->orient = ELM_TOOLTIP_ORIENT_NONE;
}

EAPI Elm_Tooltip_Orient
elm_object_tooltip_orient_get(const Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj, ELM_TOOLTIP_ORIENT_NONE);

   Elm_Tooltip_Orient orient = ELM_TOOLTIP_ORIENT_NONE;

   orient = tt->orient;
   return orient;
}

/**
 * Notify tooltip should recalculate its theme.
 * @internal
 */
void
elm_tooltip_theme(Elm_Tooltip *tt)
{
   if (!tt->tooltip) return;
   tt->changed_style = EINA_TRUE;
   _elm_tooltip_reconfigure_job_start(tt);
}

/**
 * Set the content to be shown in the tooltip object for specific event area.
 *
 * Setup the tooltip to object. The object @a eventarea can have only
 * one tooltip, so any previous tooltip data is removed. @p func(with
 * @p data) will be called every time that need show the tooltip and
 * it should return a valid Evas_Object. This object is then managed
 * fully by tooltip system and is deleted when the tooltip is gone.
 *
 * This is an internal function that is used by objects with sub-items
 * that want to provide different tooltips for each of them. The @a
 * owner object should be an elm_widget and will be used to track
 * theme changes and to feed @a func and @a del_cb. The @a eventarea
 * may be any object and is the one that should be used later on with
 * elm_object_tooltip apis, such as elm_object_tooltip_hide(),
 * elm_object_tooltip_show() or elm_object_tooltip_unset().
 *
 * @param eventarea the object being attached a tooltip.
 * @param owner the elm_widget that owns this object, will be used to
 *        track theme changes and to be used in @a func or @a del_cb.
 * @param func the function used to create the tooltip contents. The
 *        @a Evas_Object parameters will receive @a owner as value.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @p func, the tooltip is unset with
 *        elm_object_tooltip_unset() or the owner object @a obj
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is NULL.
 *
 * @internal
 * @ingroup Elm_Tooltips
 */
void
elm_object_sub_tooltip_content_cb_set(Evas_Object *eventarea, Evas_Object *owner, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   Elm_Tooltip *tt = NULL;
   Eina_Bool just_created = EINA_TRUE;

   EINA_SAFETY_ON_NULL_GOTO(owner, error);
   EINA_SAFETY_ON_NULL_GOTO(eventarea, error);

   if (!func)
     {
        elm_object_tooltip_unset(eventarea);
        return;
     }

   tt = evas_object_data_get(eventarea, _tooltip_key);
   if (tt && tt->owner)
     {
        if (tt->owner != owner)
          {
             if (tt->owner != eventarea)
               evas_object_event_callback_del_full
                 (tt->owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

             elm_widget_tooltip_del(tt->owner, tt);

             if (owner != eventarea)
               evas_object_event_callback_add
                 (owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

             elm_widget_tooltip_add(tt->owner, tt);
          }

        if ((tt->func == func) && (tt->data == data) &&
            (tt->del_cb == del_cb))
          return;
        _elm_tooltip_data_clean(tt);
        just_created = EINA_FALSE;
     }
   else
     {
        if (!tt)
          {
             tt = _elm_tooltip_create(eventarea);
             if (!tt) goto error;
          }

        tt->owner = owner;
        if (owner != eventarea)
          evas_object_event_callback_add
            (owner, EVAS_CALLBACK_FREE, _elm_tooltip_obj_free_cb, tt);

        elm_widget_tooltip_add(tt->owner, tt);
     }

   tt->func = func;
   tt->data = data;
   tt->del_cb = del_cb;

   if (!just_created) _elm_tooltip_reconfigure_job_start(tt);
   else if (efl_canvas_pointer_inside_get(eventarea, NULL) && (!tt->tooltip))
     _elm_tooltip_show(tt);
   return;

 error:
   if (del_cb) del_cb((void *)data, owner, NULL);
}

EAPI void
elm_object_tooltip_show(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   tt->visible_lock = EINA_TRUE;
   _elm_tooltip_show(tt);
}

EAPI void
elm_object_tooltip_hide(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   tt->visible_lock = EINA_FALSE;
   _elm_tooltip_hide_anim_start(tt);
}

EAPI void
elm_object_tooltip_text_set(Evas_Object *obj, const char *text)
{
   Elm_Tooltip *tt;
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if (!text)
     {
        tt = evas_object_data_get((obj), _tooltip_key);
        if (tt)
          elm_object_tooltip_unset(obj);
	return;
     }

   text = eina_stringshare_add(text);
   elm_object_tooltip_content_cb_set
     (obj, _elm_tooltip_label_create, text, _elm_tooltip_label_del_cb);
}

EAPI void
elm_object_tooltip_domain_translatable_text_set(Evas_Object *obj, const char *domain, const char *text)
{
   const char **data;
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(text);

   data = malloc(2 * sizeof(char *));
   if (!data) return;
   data[0] = eina_stringshare_add(domain);
   data[1] = eina_stringshare_add(text);
   elm_object_tooltip_content_cb_set
     (obj, _elm_tooltip_trans_label_create, data,
      _elm_tooltip_trans_label_del_cb);
}

EAPI void
elm_object_tooltip_content_cb_set(Evas_Object *obj, Elm_Tooltip_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   elm_object_sub_tooltip_content_cb_set(obj, obj, func, data, del_cb);
}

EAPI void
elm_object_tooltip_unset(Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj);
   _elm_tooltip_unset(tt);
}

EAPI void
elm_object_tooltip_style_set(Evas_Object *obj, const char *style)
{
   ELM_TOOLTIP_GET_OR_CREATE(tt, obj);
   if (!eina_stringshare_replace(&tt->style, style)) return;
   elm_tooltip_theme(tt);
}

EAPI const char *
elm_object_tooltip_style_get(const Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj, NULL);
   return tt->style ? tt->style : "default";
}

EAPI Eina_Bool
elm_object_tooltip_window_mode_set(Evas_Object *obj, Eina_Bool disable)
{
   ELM_TOOLTIP_GET_OR_CREATE(tt, obj, EINA_FALSE);
   return tt->free_size = disable;
}

EAPI Eina_Bool
elm_object_tooltip_window_mode_get(const Evas_Object *obj)
{
   ELM_TOOLTIP_GET_OR_RETURN(tt, obj, EINA_FALSE);
   return tt->free_size;
}
