#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include "elm_priv.h"
#include "elm_interface_scrollable.h"

#define MY_PAN_CLASS ELM_PAN_CLASS

#define MY_PAN_CLASS_NAME "Elm_Pan"
#define MY_PAN_CLASS_NAME_LEGACY "elm_pan"

#define ELM_PAN_DATA_GET_OR_RETURN(o, ptr)                      \
  Elm_Pan_Smart_Data *ptr = efl_data_scope_get(o, MY_PAN_CLASS);       \
  if (!ptr)                                                     \
    {                                                           \
       CRI("No smart data for object %p (%s)",             \
                o, evas_object_type_get(o));                    \
       return;                                                  \
    }

#define ELM_PAN_DATA_GET_OR_RETURN_VAL(o, ptr, val)             \
  Elm_Pan_Smart_Data *ptr = efl_data_scope_get(o, MY_PAN_CLASS);       \
  if (!ptr)                                                     \
    {                                                           \
       CRI("No smart data for object %p (%s)",             \
                o, evas_object_type_get(o));                    \
       return val;                                              \
    }

#define ELM_ANIMATOR_CONNECT(Obj, Bool, Callback, Data)                 \
  efl_event_callback_del(Obj, EFL_EVENT_ANIMATOR_TICK, Callback, Data); \
  efl_event_callback_add(Obj, EFL_EVENT_ANIMATOR_TICK, Callback, Data); \
  Bool = 1;

#define ELM_ANIMATOR_DISCONNECT(Obj, Bool, Callback, Data)              \
  efl_event_callback_del(Obj, EFL_EVENT_ANIMATOR_TICK, Callback, Data); \
  Bool = 0;

#ifndef CLAMP
# define CLAMP(x, min, \
               max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

static const char SIG_CHANGED[] = "changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

static void _elm_pan_content_set(Evas_Object *, Evas_Object *);
static void
_elm_scroll_scroll_to_x(Elm_Scrollable_Smart_Interface_Data *sid,
                        double t_in,
                        Evas_Coord pos_x);
static void
_elm_scroll_scroll_to_y(Elm_Scrollable_Smart_Interface_Data *sid,
                        double t_in,
                        Evas_Coord pos_y);
static void
_elm_scroll_wanted_coordinates_update(Elm_Scrollable_Smart_Interface_Data *sid,
                                      Evas_Coord x,
                                      Evas_Coord y);

static void _elm_scroll_hold_animator(void *data, const Efl_Event *event);
static void _elm_scroll_on_hold_animator(void *data, const Efl_Event *event);
static void _elm_scroll_scroll_to_y_animator(void *data, const Efl_Event *event);
static void _elm_scroll_scroll_to_x_animator(void *data, const Efl_Event *event);
static void _elm_scroll_bounce_y_animator(void *data, const Efl_Event *event);
static void _elm_scroll_bounce_x_animator(void *data, const Efl_Event *event);
static void _elm_scroll_momentum_animator(void *data, const Efl_Event *event);

static const char iface_scr_legacy_dragable_hbar[]  = "elm.dragable.hbar";
static const char iface_scr_legacy_dragable_vbar[]  = "elm.dragable.vbar";
static const char iface_scr_efl_ui_dragable_hbar[]  = "efl.dragable.hbar";
static const char iface_scr_efl_ui_dragable_vbar[]  = "efl.dragable.vbar";

static double
_round(double value, int pos)
{
   double temp;

   temp = value * pow( 10, pos );
   temp = floor( temp + 0.5 );
   temp *= pow( 10, -pos );

   return temp;
}

static void
_elm_pan_update(Elm_Pan_Smart_Data *psd)
{
   if (psd->content)
     {
        Efl_Ui_Focus_Manager *manager;

        manager = psd->interface_object;

        efl_ui_focus_manager_dirty_logic_freeze(manager);
        evas_object_move(psd->content, psd->x - psd->px, psd->y - psd->py);
        efl_ui_focus_manager_dirty_logic_unfreeze(manager);
        //XXX: hack, right now there is no api in efl_ui_focus_manager_sub.eo to mark it dirty
        // If we have moved the content, then emit this event, in order to ensure that the focus_manager_sub
        // logic tries to fetch the viewport again
        efl_event_callback_call(manager, EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, NULL);
     }
}

EOLIAN static void
_elm_pan_efl_canvas_group_group_add(Eo *obj, Elm_Pan_Smart_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_PAN_CLASS));

   priv->self = obj;
}

EOLIAN static void
_elm_pan_efl_canvas_group_group_del(Eo *obj, Elm_Pan_Smart_Data *_pd EINA_UNUSED)
{
   _elm_pan_content_set(obj, NULL);

   efl_canvas_group_del(efl_super(obj, MY_PAN_CLASS));
}

EOLIAN static void
_elm_pan_efl_gfx_entity_position_set(Eo *obj, Elm_Pan_Smart_Data *psd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_PAN_CLASS), pos);

   psd->x = pos.x;
   psd->y = pos.y;

   _elm_pan_update(psd);
}

EOLIAN static void
_elm_pan_efl_gfx_entity_size_set(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(obj, MY_PAN_CLASS), sz);

   psd->w = sz.w;
   psd->h = sz.h;

   _elm_pan_update(psd);
   efl_event_callback_legacy_call(psd->self, ELM_PAN_EVENT_CHANGED, NULL);
}

EOLIAN static void
_elm_pan_efl_gfx_entity_visible_set(Eo *obj, Elm_Pan_Smart_Data *psd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_PAN_CLASS), vis);
   if (psd->content) efl_gfx_entity_visible_set(psd->content, vis);
}

EOLIAN static void
_elm_pan_pos_set(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord x, Evas_Coord y)
{
   if ((x == psd->px) && (y == psd->py)) return;
   psd->px = x;
   psd->py = y;

   _elm_pan_update(psd);
   efl_event_callback_legacy_call(psd->self, ELM_PAN_EVENT_CHANGED, NULL);
}

EOLIAN static void
_elm_pan_pos_get(const Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = psd->px;
   if (y) *y = psd->py;
}

EOLIAN static void
_elm_pan_pos_max_get(const Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x)
     {
        if (psd->w < psd->content_w) *x = psd->content_w - psd->w;
        else *x = 0;
     }
   if (y)
     {
        if (psd->h < psd->content_h) *y = psd->content_h - psd->h;
        else *y = 0;
     }
}

EOLIAN static void
_elm_pan_pos_min_get(const Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   if (x)
     *x = 0;
   if (y)
     *y = 0;
}

EOLIAN static void
_elm_pan_content_size_get(const Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = psd->content_w;
   if (h) *h = psd->content_h;
}

static Evas_Object *
_elm_pan_add(Evas *evas)
{
   return elm_legacy_add(MY_PAN_CLASS, evas);
}

EOLIAN static Eo *
_elm_pan_efl_object_constructor(Eo *obj, Elm_Pan_Smart_Data *_pd EINA_UNUSED)
{
   efl_canvas_group_clipped_set(obj, EINA_TRUE);
   obj = efl_constructor(efl_super(obj, MY_PAN_CLASS));
   efl_canvas_object_type_set(obj, MY_PAN_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   return obj;
}

static void
_elm_pan_content_del_cb(void *data,
                        Evas *e EINA_UNUSED,
                        Evas_Object *obj EINA_UNUSED,
                        void *event_info EINA_UNUSED)
{
   Elm_Pan_Smart_Data *psd;

   psd = data;
   psd->content = NULL;
   psd->content_w = psd->content_h = psd->px = psd->py = 0;
   efl_event_callback_legacy_call(psd->self, ELM_PAN_EVENT_CHANGED, NULL);
}

static void
_elm_pan_content_resize_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Pan_Smart_Data *psd;
   Evas_Coord w, h;

   psd = data;
   evas_object_geometry_get(psd->content, NULL, NULL, &w, &h);
   if ((w != psd->content_w) || (h != psd->content_h))
     {
        psd->content_w = w;
        psd->content_h = h;
        _elm_pan_update(psd);
     }
   efl_event_callback_legacy_call(psd->self, ELM_PAN_EVENT_CHANGED, NULL);
}

static void
_elm_pan_content_set(Evas_Object *obj,
                     Evas_Object *content)
{
   Evas_Coord w, h;

   ELM_PAN_DATA_GET_OR_RETURN(obj, psd);

   if (content == psd->content) return;
   if (psd->content)
     {
        evas_object_smart_member_del(psd->content);
        evas_object_event_callback_del_full
          (psd->content, EVAS_CALLBACK_DEL, _elm_pan_content_del_cb, psd);
        evas_object_event_callback_del_full
          (psd->content, EVAS_CALLBACK_RESIZE, _elm_pan_content_resize_cb,
          psd);
        psd->content = NULL;
     }
   if (!content) goto end;

   psd->content = content;
   evas_object_smart_member_add(psd->content, psd->self);
   evas_object_geometry_get(psd->content, NULL, NULL, &w, &h);
   psd->content_w = w;
   psd->content_h = h;
   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _elm_pan_content_del_cb, psd);
   evas_object_event_callback_add
     (content, EVAS_CALLBACK_RESIZE, _elm_pan_content_resize_cb, psd);

   if (evas_object_visible_get(psd->self))
     evas_object_show(psd->content);
   else
     evas_object_hide(psd->content);

   _elm_pan_update(psd);

end:
   efl_event_callback_legacy_call(psd->self, ELM_PAN_EVENT_CHANGED, NULL);
}

EOLIAN static void
_elm_pan_class_constructor(Efl_Class *klass)
{
      evas_smart_legacy_type_register(MY_PAN_CLASS_NAME_LEGACY, klass);
}

/* pan smart object on top, scroller interface on bottom */
/* ============================================================ */

#define MY_SCROLLABLE_INTERFACE ELM_INTERFACE_SCROLLABLE_MIXIN

#define MY_SCROLLABLE_INTERFACE_NAME "Elm_Interface_Scrollable"
#define MY_SCROLLABLE_INTERFACE_NAME_LEGACY "elm_interface_scrollable"

#define ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(o, ptr)     \
  Elm_Scrollable_Smart_Interface_Data *ptr =            \
    (!efl_isa(o, MY_SCROLLABLE_INTERFACE) ? NULL :       \
     efl_data_scope_get(o, MY_SCROLLABLE_INTERFACE));    \
  if (!ptr)                                             \
    {                                                   \
       CRI("No interface data for object %p (%s)", \
                o, evas_object_type_get(o));            \
       return;                                          \
    }

#define ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  Elm_Scrollable_Smart_Interface_Data *ptr =                 \
    (!efl_isa(o, MY_SCROLLABLE_INTERFACE) ? NULL :            \
     efl_data_scope_get(o, MY_SCROLLABLE_INTERFACE));         \
  if (!ptr)                                                  \
    {                                                        \
       CRI("No interface data for object %p (%s)",      \
                o, evas_object_type_get(o));                 \
       return val;                                           \
    }

static void _elm_scroll_scroll_bar_size_adjust(
  Elm_Scrollable_Smart_Interface_Data *);
static void _elm_scroll_wanted_region_set(Evas_Object *);
static Eina_Bool _paging_is_enabled(Elm_Scrollable_Smart_Interface_Data *sid);
static Evas_Coord _elm_scroll_page_x_get(
   Elm_Scrollable_Smart_Interface_Data *sid, int offset, Eina_Bool limit);
static Evas_Coord _elm_scroll_page_y_get(
   Elm_Scrollable_Smart_Interface_Data *sid, int offset, Eina_Bool limit);

#define LEFT               0
#define RIGHT              1
#define UP                 2
#define DOWN               3
//#define SCROLLDBG 1
/* smoothness debug calls - for debugging how much smooth your app is */
#define SMOOTHDBG          1

#ifdef SMOOTHDBG
#define SMOOTH_DEBUG_COUNT 100

#define FPS                1 / 60
typedef struct _smooth_debug_info smooth_debug_info;
struct _smooth_debug_info
{
   double     t;
   double     dt;
   Evas_Coord pos;
   Evas_Coord dpos;
   double     vpos;
};

static smooth_debug_info smooth_x_history[SMOOTH_DEBUG_COUNT];
static smooth_debug_info smooth_y_history[SMOOTH_DEBUG_COUNT];
static int smooth_info_x_count = 0;
static int smooth_info_y_count = 0;
static double start_time = 0;
static int _elm_scroll_smooth_debug = 0;

void
_elm_scroll_smooth_debug_init(void)
{
   start_time = ecore_time_get();
   smooth_info_x_count = 0;
   smooth_info_y_count = 0;

   memset(&(smooth_x_history[0]), 0,
          sizeof(smooth_x_history[0]) * SMOOTH_DEBUG_COUNT);
   memset(&(smooth_y_history[0]), 0,
          sizeof(smooth_y_history[0]) * SMOOTH_DEBUG_COUNT);

   return;
}

void
_elm_scroll_smooth_debug_shutdown(void)
{
   int i = 0;
   int info_x_count = 0;
   int info_y_count = 0;
   double x_ave = 0, y_ave = 0;
   double x_sum = 0, y_sum = 0;
   double x_dev = 0, y_dev = 0;
   double x_dev_sum = 0, y_dev_sum = 0;

   if (smooth_info_x_count >= SMOOTH_DEBUG_COUNT)
     info_x_count = SMOOTH_DEBUG_COUNT;
   else
     info_x_count = smooth_info_x_count;

   if (smooth_info_y_count >= SMOOTH_DEBUG_COUNT)
     info_y_count = SMOOTH_DEBUG_COUNT;
   else
     info_y_count = smooth_info_y_count;

   DBG("\n\n<<< X-axis Smoothness >>>\n");
   DBG("| Num  | t(time)  | dt       | x    | dx   |vx(dx/1fps) |\n");

   for (i = info_x_count - 1; i >= 0; i--)
     {
        DBG("| %4d | %1.6f | %1.6f | %4d | %4d | %9.3f |\n", info_x_count - i,
            smooth_x_history[i].t,
            smooth_x_history[i].dt,
            smooth_x_history[i].pos,
            smooth_x_history[i].dpos,
            smooth_x_history[i].vpos);
        if (i == info_x_count - 1) continue;
        x_sum += smooth_x_history[i].vpos;
     }

   x_ave = x_sum / (info_x_count - 1);
   for (i = 0; i < info_x_count - 1; i++)
     {
        x_dev_sum += (smooth_x_history[i].vpos - x_ave) *
          (smooth_x_history[i].vpos - x_ave);
     }
   x_dev = x_dev_sum / (info_x_count - 1);
   DBG(" Standard deviation of X-axis velocity: %9.3f\n", sqrt(x_dev));

   DBG("\n\n<<< Y-axis Smoothness >>>\n");
   DBG("| Num  | t(time)  | dt       | y    |  dy  |vy(dy/1fps) |\n");
   for (i = info_y_count - 1; i >= 0; i--)
     {
        DBG("| %4d | %1.6f | %1.6f | %4d | %4d | %9.3f |\n", info_y_count - i,
            smooth_y_history[i].t,
            smooth_y_history[i].dt,
            smooth_y_history[i].pos,
            smooth_y_history[i].dpos,
            smooth_y_history[i].vpos);
        if (i == info_y_count - 1) continue;
        y_sum += smooth_y_history[i].vpos;
     }
   y_ave = y_sum / (info_y_count - 1);
   for (i = 0; i < info_y_count - 1; i++)
     {
        y_dev_sum += (smooth_y_history[i].vpos - y_ave) *
          (smooth_y_history[i].vpos - y_ave);
     }
   y_dev = y_dev_sum / (info_y_count - 1);

   DBG(" Standard deviation of Y-axis velocity: %9.3f\n", sqrt(y_dev));
}

static void
_elm_direction_arrows_eval(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Eina_Bool go_left = EINA_TRUE, go_right = EINA_TRUE;
   Eina_Bool go_up = EINA_TRUE, go_down = EINA_TRUE;
   Evas_Coord x = 0, y = 0, mx = 0, my = 0, minx = 0, miny = 0;

   if (!sid->edje_obj || !sid->pan_obj) return;

   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   elm_obj_pan_pos_get(sid->pan_obj, &x, &y);

   if (x == minx) go_left = EINA_FALSE;
   if (x == (mx + minx)) go_right = EINA_FALSE;
   if (y == miny) go_up = EINA_FALSE;
   if (y == (my + miny)) go_down = EINA_FALSE;
   if (go_left != sid->go_left)
     {
        if (go_left)
          edje_object_signal_emit(sid->edje_obj, "elm,action,show,left", "elm");
        else
          edje_object_signal_emit(sid->edje_obj, "elm,action,hide,left", "elm");
        sid->go_left = go_left;
     }
   if (go_right != sid->go_right)
     {
        if (go_right)
          edje_object_signal_emit(sid->edje_obj, "elm,action,show,right", "elm");
        else
          edje_object_signal_emit(sid->edje_obj, "elm,action,hide,right", "elm");
        sid->go_right= go_right;
     }
   if (go_up != sid->go_up)
     {
        if (go_up)
          edje_object_signal_emit(sid->edje_obj, "elm,action,show,up", "elm");
        else
          edje_object_signal_emit(sid->edje_obj, "elm,action,hide,up", "elm");
        sid->go_up = go_up;
     }
   if (go_down != sid->go_down)
     {
        if (go_down)
          edje_object_signal_emit(sid->edje_obj, "elm,action,show,down", "elm");
        else
          edje_object_signal_emit(sid->edje_obj, "elm,action,hide,down", "elm");
        sid->go_down= go_down;
     }
}

void
_elm_scroll_smooth_debug_movetime_add(int x,
                                      int y)
{
   double tim = 0;
   static int bx = 0;
   static int by = 0;

   tim = ecore_time_get();

   if (bx != x)
     {
        smooth_info_x_count++;
        memmove(&(smooth_x_history[1]), &(smooth_x_history[0]),
                sizeof(smooth_x_history[0]) * (SMOOTH_DEBUG_COUNT - 1));
        smooth_x_history[0].t = tim - start_time;
        smooth_x_history[0].dt = smooth_x_history[0].t - smooth_x_history[1].t;
        smooth_x_history[0].pos = x;
        smooth_x_history[0].dpos =
          smooth_x_history[0].pos - smooth_x_history[1].pos;

        if (smooth_x_history[0].dpos >= 0)
          smooth_x_history[0].vpos = (double)(smooth_x_history[0].dpos) /
            smooth_x_history[0].dt * FPS;
        else
          smooth_x_history[0].vpos = -((double)(smooth_x_history[0].dpos) /
                                       smooth_x_history[0].dt * FPS);
     }

   if (by != y)
     {
        smooth_info_y_count++;
        memmove(&(smooth_y_history[1]), &(smooth_y_history[0]),
                sizeof(smooth_y_history[0]) * (SMOOTH_DEBUG_COUNT - 1));
        smooth_y_history[0].t = tim - start_time;
        smooth_y_history[0].dt = smooth_y_history[0].t - smooth_y_history[1].t;
        smooth_y_history[0].pos = y;
        smooth_y_history[0].dpos = smooth_y_history[0].pos -
          smooth_y_history[1].pos;

        if (smooth_y_history[0].dpos >= 0)
          smooth_y_history[0].vpos = (double)(smooth_y_history[0].dpos) /
            smooth_y_history[0].dt * FPS;
        else
          smooth_y_history[0].vpos = -((double)(smooth_y_history[0].dpos) /
                                       smooth_y_history[0].dt * FPS);
     }

   bx = x;
   by = y;
}

#endif

static void
_elm_scroll_scroll_bar_h_visibility_apply(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if (sid->hbar_flags != ELM_SCROLLER_POLICY_OFF)
     {
        if (sid->hbar_visible)
          edje_object_signal_emit
            (sid->edje_obj, "elm,action,show,hbar", "elm");
        else
          edje_object_signal_emit
            (sid->edje_obj, "elm,action,hide,hbar", "elm");
     }
   else
     edje_object_signal_emit(sid->edje_obj, "elm,action,hide,hbar", "elm");
   edje_object_message_signal_process(sid->edje_obj);
   _elm_scroll_scroll_bar_size_adjust(sid);
   if (sid->cb_func.content_min_limit)
     sid->cb_func.content_min_limit(sid->obj, sid->min_w, sid->min_h);
}

static void
_elm_scroll_scroll_bar_v_visibility_apply(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if (sid->vbar_flags != ELM_SCROLLER_POLICY_OFF)
     {
        if (sid->vbar_visible)
          edje_object_signal_emit
            (sid->edje_obj, "elm,action,show,vbar", "elm");
        else
          edje_object_signal_emit
            (sid->edje_obj, "elm,action,hide,vbar", "elm");
     }
   else
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,hide,vbar", "elm");
   edje_object_message_signal_process(sid->edje_obj);
   _elm_scroll_scroll_bar_size_adjust(sid);
   if (sid->cb_func.content_min_limit)
     sid->cb_func.content_min_limit(sid->obj, sid->min_w, sid->min_h);
}

static int
_elm_scroll_scroll_bar_h_visibility_adjust(
  Elm_Scrollable_Smart_Interface_Data *sid)
{
   int scroll_h_vis_change = 0;
   Evas_Coord w, vw = 0, vh = 0;

   if (!sid->edje_obj) return 0;

   w = sid->content_info.w;
   if (sid->pan_obj)
     evas_object_geometry_get(sid->pan_obj, NULL, NULL, &vw, &vh);
   if (sid->hbar_visible)
     {
        if (sid->min_w)
          {
             scroll_h_vis_change = 1;
             sid->hbar_visible = EINA_FALSE;
          }
        else
          {
             if (sid->hbar_flags == ELM_SCROLLER_POLICY_AUTO)
               {
                  if ((sid->content) || (sid->extern_pan))
                    {
                       if (w <= vw)
                         {
                            scroll_h_vis_change = 1;
                            sid->hbar_visible = EINA_FALSE;
                         }
                    }
                  else
                    {
                       scroll_h_vis_change = 1;
                       sid->hbar_visible = EINA_FALSE;
                    }
               }
             else if (sid->hbar_flags == ELM_SCROLLER_POLICY_OFF)
               {
                  scroll_h_vis_change = 1;
                  sid->hbar_visible = EINA_FALSE;
               }
          }
     }
   else
     {
        if (!sid->min_w)
          {
             if (sid->hbar_flags == ELM_SCROLLER_POLICY_AUTO)
               {
                  if ((sid->content) || (sid->extern_pan))
                    {
                       if (w > vw)
                         {
                            scroll_h_vis_change = 1;
                            sid->hbar_visible = EINA_TRUE;
                         }
                    }
               }
             else if (sid->hbar_flags == ELM_SCROLLER_POLICY_ON)
               {
                  scroll_h_vis_change = 1;
                  sid->hbar_visible = EINA_TRUE;
               }
          }
     }

   if (scroll_h_vis_change) _elm_scroll_scroll_bar_h_visibility_apply(sid);

   _elm_direction_arrows_eval(sid);
   return scroll_h_vis_change;
}

static int
_elm_scroll_scroll_bar_v_visibility_adjust(
  Elm_Scrollable_Smart_Interface_Data *sid)
{
   int scroll_v_vis_change = 0;
   Evas_Coord h, vw = 0, vh = 0;

   if (!sid->edje_obj) return 0;

   h = sid->content_info.h;
   if (sid->pan_obj)
     evas_object_geometry_get(sid->pan_obj, NULL, NULL, &vw, &vh);
   if (sid->vbar_visible)
     {
        if (sid->min_h)
          {
             scroll_v_vis_change = 1;
             sid->vbar_visible = EINA_FALSE;
          }
        else
          {
             if (sid->vbar_flags == ELM_SCROLLER_POLICY_AUTO)
               {
                  if ((sid->content) || (sid->extern_pan))
                    {
                       if (h <= vh)
                         {
                            scroll_v_vis_change = 1;
                            sid->vbar_visible = EINA_FALSE;
                         }
                    }
                  else
                    {
                       scroll_v_vis_change = 1;
                       sid->vbar_visible = EINA_FALSE;
                    }
               }
             else if (sid->vbar_flags == ELM_SCROLLER_POLICY_OFF)
               {
                  scroll_v_vis_change = 1;
                  sid->vbar_visible = EINA_FALSE;
               }
          }
     }
   else
     {
        if (!sid->min_h)
          {
             if (sid->vbar_flags == ELM_SCROLLER_POLICY_AUTO)
               {
                  if ((sid->content) || (sid->extern_pan))
                    {
                       if (h > vh)
                         {
                            scroll_v_vis_change = 1;
                            sid->vbar_visible = EINA_TRUE;
                         }
                    }
               }
             else if (sid->vbar_flags == ELM_SCROLLER_POLICY_ON)
               {
                  scroll_v_vis_change = 1;
                  sid->vbar_visible = EINA_TRUE;
               }
          }
     }
   if (scroll_v_vis_change) _elm_scroll_scroll_bar_v_visibility_apply(sid);

   _elm_direction_arrows_eval(sid);
   return scroll_v_vis_change;
}

static inline void
_elm_scroll_scroll_bar_auto_visibility_adjust(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Eina_Size2D sz;
   int w, h;

   if ((sid->vbar_flags != ELM_SCROLLER_POLICY_AUTO) ||
       (sid->hbar_flags != ELM_SCROLLER_POLICY_AUTO) ||
       !sid->hbar_visible || !sid->vbar_visible) return;

   if (!sid->content && !sid->extern_pan) return;

   w = sid->content_info.w;
   h = sid->content_info.h;
   sz = efl_gfx_entity_size_get(sid->edje_obj);

   // Adjust when the content may fit but the bars are visible. The if() test
   // does not guarantee that the content will fit (offsets & margins depend
   // on the theme).
   if ((w <= sz.w) && (h <= sz.h))
     {
        sid->hbar_visible = EINA_FALSE;
        sid->vbar_visible = EINA_FALSE;
        _elm_scroll_scroll_bar_h_visibility_apply(sid);
        _elm_scroll_scroll_bar_v_visibility_apply(sid);
        _elm_scroll_scroll_bar_h_visibility_adjust(sid);
        _elm_scroll_scroll_bar_v_visibility_adjust(sid);
     }
}

static void
_elm_scroll_scroll_bar_visibility_adjust(
  Elm_Scrollable_Smart_Interface_Data *sid)
{
   int changed = 0;

   changed |= _elm_scroll_scroll_bar_h_visibility_adjust(sid);
   changed |= _elm_scroll_scroll_bar_v_visibility_adjust(sid);

   if (changed)
     {
        _elm_scroll_scroll_bar_h_visibility_adjust(sid);
        _elm_scroll_scroll_bar_v_visibility_adjust(sid);
     }

   _elm_scroll_scroll_bar_auto_visibility_adjust(sid);
}

static inline EINA_PURE Eina_Bool
_elm_scroll_has_bars(const Elm_Scrollable_Smart_Interface_Data *sid)
{
   const char *iface_scr_dragable_hbar = NULL;
   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     {
        iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
     }
   else
     {
        iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
     }
   return edje_object_part_exists(sid->edje_obj, iface_scr_dragable_hbar) ||
         edje_object_part_exists(sid->edje_obj, iface_scr_dragable_vbar);
}

static void
_elm_scroll_scroll_bar_size_adjust(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if (!sid->pan_obj || !sid->edje_obj) return;
   if (efl_invalidated_get(sid->pan_obj) || efl_invalidated_get(sid->edje_obj)) return;

   if (sid->size_adjust_recurse_abort) return;
   if (sid->size_adjust_recurse > 20)
     {
        sid->size_adjust_recurse_abort = EINA_TRUE;
        return;
     }
   sid->size_adjust_recurse++;

   const char *iface_scr_dragable_hbar = NULL;
   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     {
        iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
     }
   else
     {
        iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
     }

   if ((sid->content) || (sid->extern_pan))
     {
        Evas_Coord x, y, w, h, mx = 0, my = 0, vw = 0, vh = 0,
                   px = 0, py = 0, minx = 0, miny = 0;
        double vx = 0.0, vy = 0.0, size;

        edje_object_calc_force(sid->edje_obj);
        if (elm_widget_is_legacy(sid->obj))
          {
             edje_object_part_geometry_get
               (sid->edje_obj, "elm.swallow.content", NULL, NULL, &vw, &vh);
          }
        else
          {
             edje_object_part_geometry_get
               (sid->edje_obj, "efl.content", NULL, NULL, &vw, &vh);
          }

        if (!_elm_scroll_has_bars(sid))
          goto skip_bars;

        w = sid->content_info.w;
        if (w < 1) w = 1;
        size = (double)vw / (double)w;

        if (size > 1.0)
          {
             size = 1.0;
             edje_object_part_drag_value_set
               (sid->edje_obj, iface_scr_dragable_hbar, 0.0, 0.0);
          }
        edje_object_part_drag_size_set
          (sid->edje_obj, iface_scr_dragable_hbar, size, 1.0);

        h = sid->content_info.h;
        if (h < 1) h = 1;
        size = (double)vh / (double)h;
        if (size > 1.0)
          {
             size = 1.0;
             edje_object_part_drag_value_set
               (sid->edje_obj, iface_scr_dragable_vbar, 0.0, 0.0);
          }
        edje_object_part_drag_size_set
          (sid->edje_obj, iface_scr_dragable_vbar, 1.0, size);

        edje_object_part_drag_value_get
          (sid->edje_obj, iface_scr_dragable_hbar, &vx, NULL);
        edje_object_part_drag_value_get
          (sid->edje_obj, iface_scr_dragable_vbar, NULL, &vy);

        elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
        elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
        x = vx * mx + minx;
        y = vy * my + miny;

        edje_object_part_drag_step_set
          (sid->edje_obj, iface_scr_dragable_hbar, (double)sid->step.x /
          (double)w, 0.0);
        edje_object_part_drag_step_set
          (sid->edje_obj, iface_scr_dragable_vbar, 0.0, (double)sid->step.y /
          (double)h);
        if (sid->page.x > 0)
          edje_object_part_drag_page_set
            (sid->edje_obj, iface_scr_dragable_hbar, (double)sid->page.x /
            (double)w, 0.0);
        else
          edje_object_part_drag_page_set
            (sid->edje_obj, iface_scr_dragable_hbar,
            -((double)sid->page.x * ((double)vw / (double)w)) / 100.0, 0.0);
        if (sid->page.y > 0)
          edje_object_part_drag_page_set
            (sid->edje_obj, iface_scr_dragable_vbar, 0.0,
            (double)sid->page.y / (double)h);
        else
          edje_object_part_drag_page_set
            (sid->edje_obj, iface_scr_dragable_vbar, 0.0,
            -((double)sid->page.y * ((double)vh / (double)h)) / 100.0);

        elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
        if (vx != mx) x = px;
        if (vy != my) y = py;
        elm_obj_pan_pos_set(sid->pan_obj, x, y);

        if (mx > 0) vx = (double)(x - minx) / (double)mx;
        else vx = 0.0;

        if (vx < 0.0) vx = 0.0;
        else if (vx > 1.0)
          vx = 1.0;

        if (my > 0) vy = (double)(y - miny) / (double)my;
        else vy = 0.0;

        if (vy < 0.0) vy = 0.0;
        else if (vy > 1.0)
          vy = 1.0;

        edje_object_part_drag_value_set
           (sid->edje_obj, iface_scr_dragable_vbar, 0.0, vy);
        edje_object_part_drag_value_set
           (sid->edje_obj, iface_scr_dragable_hbar, vx, 0.0);
     }
   else
     {
        Evas_Coord px = 0, py = 0, minx = 0, miny = 0;

        if (_elm_scroll_has_bars(sid))
          {
             edje_object_part_drag_size_set
                   (sid->edje_obj, iface_scr_dragable_vbar, 1.0, 1.0);
             edje_object_part_drag_size_set
                   (sid->edje_obj, iface_scr_dragable_hbar, 1.0, 1.0);
          }
        elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
        elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
        elm_obj_pan_pos_set(sid->pan_obj, minx, miny);
        if ((px != minx) || (py != miny))
          edje_object_signal_emit(sid->edje_obj, "elm,action,scroll", "elm");
     }

skip_bars:
   _elm_scroll_scroll_bar_visibility_adjust(sid);
   sid->size_adjust_recurse--;
   if (sid->size_adjust_recurse <= 0)
     {
        sid->size_adjust_recurse = 0;
        sid->size_adjust_recurse_abort = EINA_FALSE;
     }
}

static void
_elm_scroll_scroll_bar_read_and_update(
  Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord x, y, mx = 0, my = 0, minx = 0, miny = 0;
   double vx, vy;

   if (!sid->edje_obj || !sid->pan_obj) return;

   if ((sid->down.dragged) || (sid->down.bounce_x_animator)
       || (sid->down.bounce_y_animator) || (sid->down.momentum_animator)
       || (sid->scrollto.x.animator) || (sid->scrollto.y.animator))
     return;
   if (!_elm_scroll_has_bars(sid)) return;

   const char *iface_scr_dragable_hbar = NULL;
   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     {
        iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
     }
   else
     {
        iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
     }

   edje_object_part_drag_value_get
     (sid->edje_obj, iface_scr_dragable_vbar, NULL, &vy);
   edje_object_part_drag_value_get
     (sid->edje_obj, iface_scr_dragable_hbar, &vx, NULL);
   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   x = _round(vx * (double)mx + minx, 1);
   y = _round(vy * (double)my + miny, 1);
   elm_interface_scrollable_content_pos_set(sid->obj, x, y, EINA_TRUE);
   _elm_scroll_wanted_coordinates_update(sid, x, y);
}

static void
_elm_scroll_drag_start(Elm_Scrollable_Smart_Interface_Data *sid)
{
   sid->current_page.x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
   sid->current_page.y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);

   if (sid->cb_func.drag_start)
     sid->cb_func.drag_start(sid->obj, NULL);
}

static void
_elm_scroll_drag_stop(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord x, y;

   if (!(sid->down.bounce_x_animator) && !(sid->down.bounce_y_animator) &&
       !(sid->scrollto.x.animator) && !(sid->scrollto.y.animator))
     {
        x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
        y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);
        if (sid->cb_func.page_change &&
            ((x != sid->current_page.x) || (y != sid->current_page.y)))
          sid->cb_func.page_change(sid->obj, NULL);
        sid->current_page.x = x;
        sid->current_page.y = y;
     }

   if (sid->cb_func.drag_stop)
     sid->cb_func.drag_stop(sid->obj, NULL);
}

static void
_elm_scroll_anim_start(Elm_Scrollable_Smart_Interface_Data *sid)
{
   sid->current_page.x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
   sid->current_page.y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);

   if (sid->cb_func.animate_start)
     sid->cb_func.animate_start(sid->obj, NULL);
}

static void
_elm_scroll_anim_stop(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord x, y;

   if (sid->cb_func.page_change)
     {
        x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
        y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);
        if ((x != sid->current_page.x) || (y != sid->current_page.y))
           sid->cb_func.page_change(sid->obj, NULL);
        sid->current_page.x = x;
        sid->current_page.y = y;
     }

   if (sid->cb_func.animate_stop)
     sid->cb_func.animate_stop(sid->obj, NULL);
}

static void
_elm_scroll_policy_signal_emit(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if (sid->hbar_flags == ELM_SCROLLER_POLICY_ON)
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,show_always,hbar", "elm");
   else if (sid->hbar_flags == ELM_SCROLLER_POLICY_OFF)
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,hide,hbar", "elm");
   else
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,show_notalways,hbar", "elm");
   if (sid->vbar_flags == ELM_SCROLLER_POLICY_ON)
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,show_always,vbar", "elm");
   else if (sid->vbar_flags == ELM_SCROLLER_POLICY_OFF)
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,hide,vbar", "elm");
   else
     edje_object_signal_emit
       (sid->edje_obj, "elm,action,show_notalways,vbar", "elm");
   edje_object_message_signal_process(sid->edje_obj);
   _elm_scroll_scroll_bar_size_adjust(sid);
}

static void
_elm_scroll_reload_cb(void *data,
                      Evas_Object *obj EINA_UNUSED,
                      const char *emission EINA_UNUSED,
                      const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   _elm_scroll_policy_signal_emit(sid);
   _elm_scroll_scroll_bar_h_visibility_apply(sid);
   _elm_scroll_scroll_bar_v_visibility_apply(sid);
}

static void
_elm_scroll_vbar_drag_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.vbar_drag)
     sid->cb_func.vbar_drag(sid->obj, NULL);

   _elm_scroll_scroll_bar_read_and_update(sid);
}

static void
_elm_scroll_vbar_press_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.vbar_press)
     sid->cb_func.vbar_press(sid->obj, NULL);
}

static void
_elm_scroll_vbar_unpress_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.vbar_unpress)
     sid->cb_func.vbar_unpress(sid->obj, NULL);
}

static void
_elm_scroll_edje_drag_v_start_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   _elm_scroll_scroll_bar_read_and_update(sid);
   _elm_scroll_drag_start(sid);
   sid->freeze = EINA_TRUE;
}

static void
_elm_scroll_edje_drag_v_stop_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   _elm_scroll_scroll_bar_read_and_update(sid);
   _elm_scroll_drag_stop(sid);
   sid->freeze = sid->freeze_want;
}

static void
_elm_scroll_edje_drag_v_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   _elm_scroll_scroll_bar_read_and_update(sid);
}

static void
_elm_scroll_hbar_drag_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.hbar_drag)
     sid->cb_func.hbar_drag(sid->obj, NULL);

   _elm_scroll_scroll_bar_read_and_update(sid);
}

static void
_elm_scroll_hbar_press_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.hbar_press)
     sid->cb_func.hbar_press(sid->obj, NULL);
}

static void
_elm_scroll_hbar_unpress_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.hbar_unpress)
     sid->cb_func.hbar_unpress(sid->obj, NULL);
}

static void
_elm_scroll_edje_drag_h_start_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   _elm_scroll_scroll_bar_read_and_update(sid);
   _elm_scroll_drag_start(sid);
   sid->freeze = EINA_TRUE;
}

static void
_elm_scroll_edje_drag_h_stop_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   _elm_scroll_scroll_bar_read_and_update(sid);
   _elm_scroll_drag_stop(sid);
   sid->freeze = sid->freeze_want;
}

static void
_elm_scroll_edje_drag_h_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   _elm_scroll_scroll_bar_read_and_update(sid);
}

EOLIAN static void
_elm_interface_scrollable_content_size_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *w, Evas_Coord *h)
{
   elm_obj_pan_content_size_get(sid->pan_obj, w, h);
}

EOLIAN static void
_elm_interface_scrollable_content_viewport_geometry_get(const Eo *obj EINA_UNUSED,
                                                        Elm_Scrollable_Smart_Interface_Data *sid,
                                                        Evas_Coord *x,
                                                        Evas_Coord *y,
                                                        Evas_Coord *w,
                                                        Evas_Coord *h)
{
   if (!sid->pan_obj || !sid->edje_obj) return;

   edje_object_calc_force(sid->edje_obj);
   evas_object_geometry_get(sid->pan_obj, x, y, w, h);
}

EOLIAN static void
_elm_interface_scrollable_content_min_limit(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool w, Eina_Bool h)
{
   if (!sid->edje_obj) return;

   if (!sid->cb_func.content_min_limit)
     {
        ERR("Content minimim size limiting is unimplemented -- you "
            "must provide it yourself\n");
        return;
     }

   sid->min_w = !!w;
   sid->min_h = !!h;
   sid->cb_func.content_min_limit(sid->obj, w, h);
}

static Evas_Coord
_elm_scroll_x_mirrored_get(const Evas_Object *obj,
                           Evas_Coord x)
{
   Evas_Coord cw = 0, w = 0, min = 0, ret;

   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(obj, sid, x);

   if (!sid->pan_obj) return 0;

   elm_obj_pan_pos_min_get(sid->pan_obj, &min, NULL);
   elm_interface_scrollable_content_viewport_geometry_get
         ((Eo *)obj, NULL, NULL, &w, NULL);
   elm_obj_pan_content_size_get(sid->pan_obj, &cw, NULL);
   ret = cw - w - x + min + min;

   return (ret >= min) ? ret : min;
}

/* Update the wanted coordinates according to the x, y passed
 * widget directionality, content size and etc. */
static void
_elm_scroll_wanted_coordinates_update(Elm_Scrollable_Smart_Interface_Data *sid,
                                      Evas_Coord x,
                                      Evas_Coord y)
{
   Evas_Coord mx = 0, my = 0, minx = 0, miny = 0;

   if (!sid->pan_obj) return;

   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);

   /* Update wx/y/w/h - and if the requested positions aren't legal
    * adjust a bit. */
   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &sid->ww, &sid->wh);

   if (x < minx && !sid->is_mirrored)
     {
        if (!sid->loop_h) sid->wx = minx;
        else sid->wx = mx;
     }
   else if (sid->is_mirrored)
     sid->wx = _elm_scroll_x_mirrored_get(sid->obj, x);
   else if (!sid->loop_h && (x > mx)) sid->wx = mx;
   else if (sid->loop_h && x >= (sid->ww + mx)) sid->wx = minx;
   else sid->wx = x;

   if (y < miny)
     {
        if (!sid->loop_v) sid->wy = miny;
        else sid->wy = my;
     }
   else if (!sid->loop_v && (y > my)) sid->wy = my;
   else if (sid->loop_v && y >= (sid->wh + my)) sid->wy = miny;
   else sid->wy = y;
}

static void
_elm_scroll_momentum_end(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator)) return;
   if (sid->down.momentum_animator)
     {
        Evas_Coord px = 0, py = 0;
        elm_interface_scrollable_content_pos_get(sid->obj, &px, &py);
        _elm_scroll_wanted_coordinates_update(sid, px, py);

        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
        sid->down.bounce_x_hold = EINA_FALSE;
        sid->down.bounce_y_hold = EINA_FALSE;
        sid->down.ax = 0;
        sid->down.ay = 0;
        sid->down.dx = 0;
        sid->down.dy = 0;
        sid->down.pdx = 0;
        sid->down.pdy = 0;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
}

static void
_elm_scroll_bounce_x_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(data, sid);
   Evas_Coord x, y, dx, w, odx, ed, md;
   double t, p, dt, pd, r;

   t = ecore_loop_time_get();
   dt = t - sid->down.anim_start2;
   if (dt >= 0.0)
     {
        dt = dt / _elm_config->thumbscroll_bounce_friction;
        odx = sid->down.b2x - sid->down.bx;
        elm_interface_scrollable_content_viewport_geometry_get
              (sid->obj, NULL, NULL, &w, NULL);
        if (!sid->down.momentum_animator && (w > abs(odx)))
          {
             pd = (double)odx / (double)w;
             pd = (pd > 0) ? pd : -pd;
             pd = 1.0 - ((1.0 - pd) * (1.0 - pd));
             dt = dt / pd;
          }
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
        dx = (odx * p);
        r = 1.0;
        if (sid->down.momentum_animator)
          {
             ed = abs((int)(sid->down.dx * _elm_config->thumbscroll_momentum_friction - sid->down.b0x));
             md = abs((int)(_elm_config->thumbscroll_momentum_friction * 5 * w));
             if (ed > md) r = (double)(md) / (double)ed;
          }
        x = sid->down.b2x + (int)((double)(dx - odx) * r);
        if (!sid->down.cancelled)
          elm_interface_scrollable_content_pos_set(sid->obj, x, y, EINA_TRUE);
        if (dt >= 1.0)
          {
             if (sid->down.momentum_animator)
               sid->down.bounce_x_hold = EINA_TRUE;
             if ((!sid->down.bounce_y_animator) &&
                 (!sid->scrollto.y.animator))
               _elm_scroll_anim_stop(sid);
             sid->down.pdx = 0;
             sid->bouncemex = EINA_FALSE;
             _elm_scroll_momentum_end(sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
             ELM_ANIMATOR_CONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
          }
     }
}

static void
_elm_scroll_bounce_y_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(data, sid);
   Evas_Coord x, y, dy, h, ody, ed, md;
   double t, p, dt, pd, r;

   t = ecore_loop_time_get();
   dt = t - sid->down.anim_start3;
   if (dt >= 0.0)
     {
        dt = dt / _elm_config->thumbscroll_bounce_friction;
        ody = sid->down.b2y - sid->down.by;
        elm_interface_scrollable_content_viewport_geometry_get
              (sid->obj, NULL, NULL, NULL, &h);
        if (!sid->down.momentum_animator && (h > abs(ody)))
          {
             pd = (double)ody / (double)h;
             pd = (pd > 0) ? pd : -pd;
             pd = 1.0 - ((1.0 - pd) * (1.0 - pd));
             dt = dt / pd;
          }
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
        dy = (ody * p);
        r = 1.0;
        if (sid->down.momentum_animator)
          {
             ed = abs((int)(sid->down.dy * _elm_config->thumbscroll_momentum_friction - sid->down.b0y));
             md = abs((int)(_elm_config->thumbscroll_momentum_friction * 5 * h));
             if (ed > md) r = (double)(md) / (double)ed;
          }
        y = sid->down.b2y + (int)((double)(dy - ody) * r);
        if (!sid->down.cancelled)
          elm_interface_scrollable_content_pos_set(sid->obj, x, y, EINA_TRUE);
        if (dt >= 1.0)
          {
             if (sid->down.momentum_animator)
               sid->down.bounce_y_hold = EINA_TRUE;
             if ((!sid->down.bounce_x_animator) &&
                 (!sid->scrollto.y.animator))
               _elm_scroll_anim_stop(sid);
             sid->down.pdy = 0;
             sid->bouncemey = EINA_FALSE;
             _elm_scroll_momentum_end(sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
          }
     }
}

static void
_elm_scroll_bounce_eval(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord mx = 0, my = 0, px = 0, py = 0, bx, by, b2x, b2y, minx = 0, miny = 0;

   if (!sid->pan_obj) return;

   if (sid->freeze) return;
   if ((!sid->bouncemex) && (!sid->bouncemey)) return;
   if (sid->down.now) return;  // down bounce while still held down
   if (sid->down.onhold_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.onhold_animator, _elm_scroll_on_hold_animator, sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.hold_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.hold_animator, _elm_scroll_hold_animator, sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);

   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   bx = px;
   by = py;
   if (px < minx) px = minx;
   if ((px - minx) > mx) px = mx + minx;
   if (py < miny) py = miny;
   if ((py - miny) > my) py = my + miny;
   b2x = px;
   b2y = py;
   if ((!sid->obj) ||
       (!elm_widget_scroll_child_locked_x_get(sid->obj)))
     {
        if ((!sid->down.bounce_x_animator) && (!sid->bounce_animator_disabled))
          {
             if (sid->bouncemex)
               {
                  ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
                  ELM_ANIMATOR_CONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
                  sid->down.anim_start2 = ecore_loop_time_get();
                  sid->down.bx = bx;
                  sid->down.bx0 = bx;
                  sid->down.b2x = b2x;
                  if (sid->down.momentum_animator)
                    sid->down.b0x = sid->down.ax;
                  else sid->down.b0x = 0;
               }
          }
     }
   if ((!sid->obj) ||
       (!elm_widget_scroll_child_locked_y_get(sid->obj)))
     {
        if ((!sid->down.bounce_y_animator) && (!sid->bounce_animator_disabled))
          {
             if (sid->bouncemey)
               {
                  ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
                  ELM_ANIMATOR_CONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
                  sid->down.anim_start3 = ecore_loop_time_get();
                  sid->down.by = by;
                  sid->down.by0 = by;
                  sid->down.b2y = b2y;
                  if (sid->down.momentum_animator)
                    sid->down.b0y = sid->down.ay;
                  else sid->down.b0y = 0;
               }
          }
     }
}

EOLIAN static void
_elm_interface_scrollable_content_pos_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *x, Evas_Coord *y)
{
   if (!sid->pan_obj) return;

   elm_obj_pan_pos_get(sid->pan_obj, x, y);
}

EOLIAN static void
_elm_interface_scrollable_content_pos_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y, Eina_Bool sig)
{
   Evas_Coord mx = 0, my = 0, px = 0, py = 0, spx = 0, spy = 0, minx = 0, miny = 0;
   Evas_Coord cw = 0, ch = 0, pw = 0, ph = 0;
   double vx, vy;


   if (!sid->edje_obj || !sid->pan_obj) return;

   // FIXME: allow for bounce outside of range
   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   elm_obj_pan_content_size_get(sid->pan_obj, &cw, &ch);
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   evas_object_geometry_get(sid->pan_obj, NULL, NULL, &pw, &ph);

   if (_paging_is_enabled(sid))
     {
        if (sid->page_snap_horiz && !sid->loop_h)
          {
             //we passed one page to the right
             if (x > sid->current_page.x + sid->pagesize_h)
               x = sid->current_page.x + sid->pagesize_h;
             //we passed one page to the left
             if (x < sid->current_page.x - sid->pagesize_h)
               x = sid->current_page.x - sid->pagesize_h;
          }
        if (sid->page_snap_vert && !sid->loop_v)
          {
             //we passed one page to the bottom
             if (y > sid->current_page.y + sid->pagesize_v)
               y = sid->current_page.y + sid->pagesize_v;
             //we passed one page to the top
             if (y < sid->current_page.y - sid->pagesize_v)
               y = sid->current_page.y - sid->pagesize_v;
          }
     }

   if (sid->loop_h && cw > 0)
     {
        if (x < 0) x = cw + (x % cw);
        else if (x >= cw) x = (x % cw);
     }
   if (sid->loop_v && ch > 0)
     {
        if (y < 0) y = ch + (y % ch);
        else if (y >= ch) y = (y % ch);
     }

   if (!_elm_config->thumbscroll_bounce_enable)
     {

        if (x < minx) x = minx;
        if (!sid->loop_h && (x - minx) > mx) x = mx + minx;
        if (y < miny) y = miny;
        if (!sid->loop_v && (y - miny) > my) y = my + miny;
     }

   if (!sid->bounce_horiz)
     {
        if (x < minx) x = minx;
        if (!sid->loop_h && (x - minx) > mx) x = mx + minx;
     }
   if (!sid->bounce_vert)
     {
        if (y < miny) y = miny;
        if (!sid->loop_v && (y - miny) > my) y = my + miny;
     }

   elm_obj_pan_pos_set(sid->pan_obj, x, y);
   elm_obj_pan_pos_get(sid->pan_obj, &spx, &spy);

   if (mx > 0) vx = (double)(spx - minx) / (double)mx;
   else vx = 0.0;

   if (vx < 0.0) vx = 0.0;
   else if (vx > 1.0)
     vx = 1.0;

   if (my > 0) vy = (double)(spy - miny) / (double)my;
   else vy = 0.0;

   if (vy < 0.0) vy = 0.0;
   else if (vy > 1.0)
     vy = 1.0;

   const char *iface_scr_dragable_hbar = NULL;
   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     {
        iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
     }
   else
     {
        iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
     }

   if (_elm_scroll_has_bars(sid))
     {
        edje_object_part_drag_value_set
              (sid->edje_obj, iface_scr_dragable_vbar, 0.0, vy);
        edje_object_part_drag_value_set
              (sid->edje_obj, iface_scr_dragable_hbar, vx, 0.0);
     }

   if (!sid->loop_h && !sid->down.bounce_x_animator)
     {
        if (((x < minx) && (0 <= sid->down.dx)) ||
            ((x > (mx + minx)) && (0 >= sid->down.dx)))
          {
             sid->bouncemex = EINA_TRUE;
             _elm_scroll_bounce_eval(sid);
          }
        else
          sid->bouncemex = EINA_FALSE;
     }
   if (!sid->loop_v && !sid->down.bounce_y_animator)
     {
        if (((y < miny) && (0 <= sid->down.dy)) ||
            ((y > (my + miny)) && (0 >= sid->down.dy)))
          {
             sid->bouncemey = EINA_TRUE;
             _elm_scroll_bounce_eval(sid);
          }
        else
          sid->bouncemey = EINA_FALSE;
     }

   if (sig)
     {
        if ((x != px) || (y != py))
          {
             if (sid->cb_func.scroll)
               sid->cb_func.scroll(obj, NULL);
             edje_object_signal_emit(sid->edje_obj, "elm,action,scroll", "elm");
             if (x < px)
               {
                  if (sid->cb_func.scroll_left)
                    sid->cb_func.scroll_left(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,action,scroll,left", "elm");
               }
             if (x > px)
               {
                  if (sid->cb_func.scroll_right)
                    sid->cb_func.scroll_right(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,action,scroll,right", "elm");
               }
             if (y < py)
               {
                  if (sid->cb_func.scroll_up)
                    sid->cb_func.scroll_up(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,action,scroll,up", "elm");
               }
             if (y > py)
               {
                  if (sid->cb_func.scroll_down)
                    sid->cb_func.scroll_down(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,action,scroll,down", "elm");
               }
          }
        if (x != px)
          {
             if (x == minx)
               {
                  if (sid->cb_func.edge_left)
                    sid->cb_func.edge_left(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,edge,left", "elm");
               }
             if (x == (mx + minx))
               {
                  if (sid->cb_func.edge_right)
                    sid->cb_func.edge_right(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,edge,right", "elm");
               }
          }
        if (y != py)
          {
             if (y == miny)
               {
                  if (sid->cb_func.edge_top)
                    sid->cb_func.edge_top(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,edge,top", "elm");
               }
             if (y == my + miny)
               {
                  if (sid->cb_func.edge_bottom)
                    sid->cb_func.edge_bottom(obj, NULL);
                  edje_object_signal_emit(sid->edje_obj, "elm,edge,bottom", "elm");
               }
          }
     }

   _elm_direction_arrows_eval(sid);
}

EOLIAN static void
_elm_interface_scrollable_efl_ui_base_mirrored_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool mirrored)
{
   Evas_Coord wx;

   if (!sid->edje_obj) return;

   mirrored = !!mirrored;

   if (sid->is_mirrored == mirrored)
     return;

   sid->is_mirrored = mirrored;
   edje_object_mirrored_set(sid->edje_obj, mirrored);

   if (sid->is_mirrored)
     wx = _elm_scroll_x_mirrored_get(sid->obj, sid->wx);
   else
     wx = sid->wx;

   elm_interface_scrollable_content_pos_set(sid->obj, wx, sid->wy, EINA_FALSE);
   efl_ui_mirrored_set(efl_super(obj, ELM_INTERFACE_SCROLLABLE_MIXIN), mirrored);
}

/* returns TRUE when we need to move the scroller, FALSE otherwise.
 * Updates w and h either way, so save them if you need them. */
static Eina_Bool
_elm_scroll_content_region_show_internal(Evas_Object *obj,
                                         Evas_Coord *_x,
                                         Evas_Coord *_y,
                                         Evas_Coord w,
                                         Evas_Coord h)
{
   Evas_Coord cw = 0, ch = 0, px = 0, py = 0, nx, ny,
              minx = 0, miny = 0, pw = 0, ph = 0, x = *_x, y = *_y;

   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(obj, sid, EINA_FALSE);

   if (!sid->pan_obj) return EINA_FALSE;

   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   elm_obj_pan_content_size_get(sid->pan_obj, &cw, &ch);
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   evas_object_geometry_get(sid->pan_obj, NULL, NULL, &pw, &ph);

   nx = x;
   if ((x > px) && (w < pw))
     {
        if ((px + pw) < (x + w)) nx = x - pw + w;
        else nx = px;
     }
   ny = y;
   if ((y > py) && (h < ph))
     {
        if ((py + ph) < (y + h)) ny = y - ph + h;
        else ny = py;
     }

   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator) ||
       (sid->scrollto.x.animator) || (sid->scrollto.y.animator))
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
        if (sid->down.bounce_x_animator)
          {
             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
             sid->bouncemex = EINA_FALSE;
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
        if (sid->down.bounce_y_animator)
          {
             ELM_ANIMATOR_CONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
             sid->bouncemey = EINA_FALSE;
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }

        _elm_scroll_anim_stop(sid);
     }
   if (sid->down.hold_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.hold_animator, _elm_scroll_hold_animator, sid);
        _elm_scroll_drag_stop(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);
   if (sid->down.momentum_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
        sid->down.bounce_x_hold = EINA_FALSE;
        sid->down.bounce_y_hold = EINA_FALSE;
        sid->down.ax = 0;
        sid->down.ay = 0;
        sid->down.pdx = 0;
        sid->down.pdy = 0;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }

   if (_paging_is_enabled(sid))
     {
        x = _elm_scroll_page_x_get(sid, nx - px, EINA_FALSE);
        y = _elm_scroll_page_y_get(sid, ny - py, EINA_FALSE);
     }
   else
     {
        x = nx;
        y = ny;
     }
   if (!sid->loop_h)
     {
        if ((x + pw) > cw) x = cw - pw;
        if (x < minx) x = minx;
     }
   if (!sid->loop_v)
     {
        if ((y + ph) > ch) y = ch - ph;
        if (y < miny) y = miny;
     }

   if ((x == px) && (y == py)) return EINA_FALSE;
   *_x = x;
   *_y = y;
   return EINA_TRUE;
}

EOLIAN static void
_elm_interface_scrollable_content_region_get(const Eo *obj, Elm_Scrollable_Smart_Interface_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   elm_interface_scrollable_content_pos_get(obj, x, y);
   elm_interface_scrollable_content_viewport_geometry_get
         (obj, NULL, NULL, w, h);
}

/* Set should be used for calculated positions, for example, when we move
 * because of an animation or because this is the correct position after
 * constraints. */
EOLIAN static void
_elm_interface_scrollable_content_region_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        elm_interface_scrollable_content_pos_set(obj, x, y, EINA_FALSE);
        sid->down.sx = x;
        sid->down.sy = y;
        sid->down.x = sid->down.history[0].x;
        sid->down.y = sid->down.history[0].y;
     }
}

/* Set should be used for setting the wanted position, for example a
 * user scroll or moving the cursor in an entry. */
EOLIAN static void
_elm_interface_scrollable_content_region_show(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   sid->wx = (sid->is_mirrored ? _elm_scroll_x_mirrored_get(sid->obj, x) : x);
   sid->wy = y;
   sid->ww = w;
   sid->wh = h;
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        elm_interface_scrollable_content_pos_set(obj, x, y, EINA_TRUE);
        sid->down.sx = x;
        sid->down.sy = y;
        sid->down.x = sid->down.history[0].x;
        sid->down.y = sid->down.history[0].y;
     }
}

static void
_elm_scroll_wanted_region_set(Evas_Object *obj)
{
   Evas_Coord ww, wh, wx;
   Evas_Coord mx = 0, my = 0;

   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(obj, sid);

   wx = sid->wx;

   if (sid->down.now || sid->down.momentum_animator ||
       sid->down.bounce_x_animator || sid->down.bounce_y_animator ||
       sid->down.hold_animator || sid->down.onhold_animator ||
       sid->scrollto.x.animator || sid->scrollto.y.animator)
     return;

   sid->content_info.resized = EINA_FALSE;

   /* Flip to RTL cords only if init in RTL mode */
   if (sid->is_mirrored)
     wx = _elm_scroll_x_mirrored_get(obj, sid->wx);

   if (sid->ww == -1)
     {
        elm_interface_scrollable_content_viewport_geometry_get
              (obj, NULL, NULL, &ww, &wh);
     }
   else
     {
        ww = sid->ww;
        wh = sid->wh;
     }

   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);

   wx += (mx - sid->prev_cw) * sid->gravity_x;
   sid->wy += (my - sid->prev_ch) * sid->gravity_y;

   sid->prev_cw = mx;
   sid->prev_ch = my;

   elm_interface_scrollable_content_region_set(obj, wx, sid->wy, ww, wh);
}

static Eina_Value
_scroll_wheel_post_event_job(void *data, const Eina_Value v,
                             const Eina_Future *dead EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   // Animations are disabled if we are here
   elm_interface_scrollable_content_pos_set(sid->obj, sid->wx, sid->wy, EINA_TRUE);
   if (_paging_is_enabled(sid))
     {
        sid->current_page.x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
        sid->current_page.y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);
     }
   return v;
}

static inline void
_scroll_wheel_post_event_go(Elm_Scrollable_Smart_Interface_Data *sid, int x, int y)
{
   if (sid->hold || sid->freeze) return;
   _elm_scroll_wanted_coordinates_update(sid, x, y);
   if (_elm_config->scroll_animation_disable)
     {
        Eina_Future *f;

        f = eina_future_then(efl_loop_job(efl_loop_get(sid->obj)),
                             _scroll_wheel_post_event_job, sid, NULL);
        efl_future_then(sid->obj, f);
     }
   else
     {
        _elm_scroll_scroll_to_x(sid, _elm_config->bring_in_scroll_friction, x);
        _elm_scroll_scroll_to_y(sid, _elm_config->bring_in_scroll_friction, y);
     }
}

static Eina_Bool
_scroll_wheel_post_event_cb(void *data, Evas *e EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Event_Mouse_Wheel *ev = sid->event_info;

   Evas_Coord x = 0, y = 0, vw = 0, vh = 0, cw = 0, ch = 0;
   int pagenumber_h = 0, pagenumber_v = 0;
   int mx = 0, my = 0, minx = 0, miny = 0;
   Eina_Bool hold = EINA_FALSE;
   Evas_Coord pwx, pwy;
   double t;
   int direction;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ev, EINA_TRUE);

   sid->event_info = NULL;
   direction = ev->direction;

   pwx = sid->wx;
   pwy = sid->wy;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
     direction = !direction;

   elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
   if (sid->scrollto.x.animator) x = sid->scrollto.x.end;
   if (sid->scrollto.y.animator) y = sid->scrollto.y.end;
   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   if (x < minx) x = minx;
   if (x > mx) x = mx;
   if (y < miny) y = miny;
   if (y > my) y = my;

   t = ecore_loop_time_get();

   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator) ||
       (sid->scrollto.x.animator) || (sid->scrollto.y.animator))
     {
        _elm_scroll_anim_stop(sid);
     }
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
   if (sid->down.bounce_x_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
        sid->bouncemex = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.bounce_y_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
        sid->bouncemey = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &vw, &vh);
   if (sid->pan_obj)
     elm_obj_pan_content_size_get(sid->pan_obj, &cw, &ch);
   if (!_paging_is_enabled(sid))
     {
        int d = ev->z;
        double delta_t = (double)(ev->timestamp - sid->last_wheel) / 1000.0;
        double mul;

        if (delta_t > 0.2) sid->last_wheel_mul = 0.0;
        if (delta_t > 0.2) delta_t = 0.2;
        mul = 1.0 + (_elm_config->scroll_accel_factor * ((0.2 - delta_t) / 0.2));
        mul = mul * (1.0 + (0.15 * sid->last_wheel_mul));
        d *= mul;
        sid->last_wheel = ev->timestamp;
        sid->last_wheel_mul = mul;

        if (!direction)
          {
             if ((ch > vh) || (cw <= vw))
               y += d * sid->step.y;
             else
               {
                  x += d * sid->step.x;
                  direction = 1;
               }
          }
        else
          {
             if ((cw > vw) || (ch <= vh))
               x += d * sid->step.x;
             else
               {
                  y += d * sid->step.y;
                  direction = 0;
               }
          }
        _scroll_wheel_post_event_go(sid, x, y);
     }
   else
     {
        int wx = x, wy = y;

        elm_interface_scrollable_current_page_get(sid->obj, &pagenumber_h, &pagenumber_v);
        if (!direction)
          {
             if ((ch > vh) || (cw <= vw))
               wy = (pagenumber_v + (1 * ev->z)) * sid->pagesize_v;
             else
               {
                  wx = (pagenumber_h + (1 * ev->z)) * sid->pagesize_h;
                  direction = 1;
               }
          }
        else
          {
             if ((cw > vw) || (ch <= vh))
               wx = (pagenumber_h + (1 * ev->z)) * sid->pagesize_h;
             else
               {
                  wy = (pagenumber_v + (1 * ev->z)) * sid->pagesize_v;
                  direction = 0;
               }
          }

        // Snap to first or last page before looping if not smooth
        if (_elm_config->scroll_animation_disable)
          {
             if (direction && sid->loop_h)
               {
                  if (sid->page_snap_horiz)
                    {
                       if ((x == mx) && (wx > mx)) wx = minx;
                       else if ((x == minx) && (wx < minx)) wx = mx;
                    }
                  else
                    {
                       if ((x < mx) && (wx > mx)) wx = mx;
                       else if ((x > minx) && (wx < minx)) wx = minx;
                    }
               }
             if (!direction && sid->loop_v)
               {
                  if (sid->page_snap_vert)
                    {
                       if ((y == my) && (wy > my)) wy = miny;
                       else if ((y == miny) && (wy < miny)) wy = my;
                    }
                  else
                    {
                       if ((y < my) && (wy > my)) wy = my;
                       else if ((y > miny) && (wy < miny)) wy = miny;
                    }
               }
          }

        _scroll_wheel_post_event_go(sid, wx, wy);
     }

   if (direction)
     {
        if ((pwx != sid->wx) ||
            (((t - sid->down.last_time_x_wheel) < 0.5) &&
             (sid->down.last_hold_x_wheel)))
          {
             sid->down.last_hold_x_wheel = EINA_TRUE;
             hold = EINA_TRUE;
          }
        else sid->down.last_hold_x_wheel = EINA_FALSE;
        sid->down.last_time_x_wheel = t;
     }
   else
     {
        if ((pwy != sid->wy) ||
            (((t - sid->down.last_time_y_wheel) < 0.5) &&
             (sid->down.last_hold_y_wheel)))
          {
             sid->down.last_hold_y_wheel = EINA_TRUE;
             hold = EINA_TRUE;
          }
        else sid->down.last_hold_y_wheel = EINA_FALSE;
        sid->down.last_time_y_wheel = t;
     }

   return !hold;
}

static void
_elm_scroll_wheel_event_cb(void *data,
                           Evas *e,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   Elm_Scrollable_Smart_Interface_Data *sid;
   Evas_Event_Mouse_Wheel *ev;
   int direction;

   sid = data;
   ev = event_info;
   sid->event_info = event_info;
   direction = ev->direction;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if ((evas_key_modifier_is_set(ev->modifiers, "Control")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Alt")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Meta")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Hyper")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Super")))
     return;
   if (direction)
     {
        if (sid->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL) return;
     }
   else
     {
        if (sid->block & EFL_UI_SCROLL_BLOCK_VERTICAL) return;
     }

   evas_post_event_callback_push(e, _scroll_wheel_post_event_cb, sid);
}

static Eina_Bool
_elm_scroll_post_event_up(void *data,
                          Evas *e EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->obj)
     {
        elm_widget_scroll_lock_set(sid->obj, EFL_UI_SCROLL_BLOCK_NONE);
     }
   return EINA_TRUE;
}

static Eina_Bool
_paging_is_enabled(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if ((sid->pagerel_h == 0.0) && (!sid->pagesize_h) &&
       (sid->pagerel_v == 0.0) && (!sid->pagesize_v))
     return EINA_FALSE;
   return EINA_TRUE;
}

static void
_elm_scroll_momentum_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   double t, dt, p;
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord x, y, dx, dy, px, py, maxx, maxy, minx, miny;
   Eina_Bool no_bounce_x_end = EINA_FALSE, no_bounce_y_end = EINA_FALSE;

   if (!sid->pan_obj)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
        return;
     }

   t = ecore_loop_time_get();

   if (sid->down.anim_dur == 0) dt = 1.0;
   else dt = (t - sid->down.anim_start) / sid->down.anim_dur;

   if (dt >= 0.0)
     {
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        dx = sid->down.dx * p;
        dy = sid->down.dy * p;
        sid->down.ax = dx;
        sid->down.ay = dy;
        x = sid->down.sx - dx;
        y = sid->down.sy - dy;
        elm_interface_scrollable_content_pos_get(sid->obj, &px, &py);
        if ((sid->down.bounce_x_animator) ||
            (sid->down.bounce_x_hold))
          {
             sid->down.bx = sid->down.bx0 - dx + sid->down.b0x;
             x = px;
          }
        if ((sid->down.bounce_y_animator) ||
            (sid->down.bounce_y_hold))
          {
             sid->down.by = sid->down.by0 - dy + sid->down.b0y;
             y = py;
          }
        elm_interface_scrollable_content_pos_set(sid->obj, x, y, EINA_TRUE);
        _elm_scroll_wanted_coordinates_update(sid, x, y);
        elm_obj_pan_pos_max_get(sid->pan_obj, &maxx, &maxy);
        elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);

        if (!_elm_config->thumbscroll_bounce_enable || !sid->bounce_horiz)
          {
             if (x <= minx) no_bounce_x_end = EINA_TRUE;
             if (!sid->loop_h && (x - minx) >= maxx) no_bounce_x_end = EINA_TRUE;
          }
        if (!_elm_config->thumbscroll_bounce_enable || !sid->bounce_vert)
          {
             if (y <= miny) no_bounce_y_end = EINA_TRUE;
             if (!sid->loop_v && (y - miny) >= maxy) no_bounce_y_end = EINA_TRUE;
          }
        if ((dt >= 1.0) ||
            ((sid->down.bounce_x_hold) && (sid->down.bounce_y_hold)) ||
            (no_bounce_x_end && no_bounce_y_end))
          {
             _elm_scroll_anim_stop(sid);

             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
             sid->down.bounce_x_hold = EINA_FALSE;
             sid->down.bounce_y_hold = EINA_FALSE;
             sid->down.ax = 0;
             sid->down.ay = 0;
             sid->down.pdx = 0;
             sid->down.pdy = 0;
             sid->down.anim_dur = 0;
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
     }
}

static Evas_Coord
_elm_scroll_page_x_get(Elm_Scrollable_Smart_Interface_Data *sid,
                       int offset, Eina_Bool limit)
{
   Evas_Coord x, y, w, h, dx, cw, ch, minx = 0;

   if (!sid->pan_obj) return 0;

   elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &w, &h);
   elm_obj_pan_content_size_get(sid->pan_obj, &cw, &ch);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, NULL);

   if (sid->pagerel_h > 0.0)
     sid->pagesize_h = w * sid->pagerel_h;

   if (!limit)
     x += offset;
   else
     {
        dx = (sid->pagesize_h * ((double)sid->page_limit_h - 0.5));

        if (offset > 0)
          x += (abs(offset) < dx ? offset : dx);
        else
          x += (abs(offset) < dx ? offset : -(dx + 1));
     }

   if (sid->is_mirrored) x += w;
   if (sid->pagesize_h > 0)
     {
        if (x >= 0)
          x = x + (sid->pagesize_h * 0.5);
        else if (x < 0 && sid->loop_h)
          x = x - (sid->pagesize_h * 0.5);
        x = x / (sid->pagesize_h);
        x = x * (sid->pagesize_h);
     }
   if (sid->is_mirrored) x -= w;
   if (!sid->loop_h)
     {
        if ((x + w) > cw) x = cw - w;
        if (x < minx) x = minx;
     }

   return x;
}

static Evas_Coord
_elm_scroll_page_y_get(Elm_Scrollable_Smart_Interface_Data *sid,
                       int offset, Eina_Bool limit)
{
   Evas_Coord x, y, w = 0, h = 0, dy, cw = 0, ch = 0, miny = 0;

   if (!sid->pan_obj) return 0;

   elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &w, &h);
   elm_obj_pan_content_size_get(sid->pan_obj, &cw, &ch);
   elm_obj_pan_pos_min_get(sid->pan_obj, NULL, &miny);

   if (sid->pagerel_v > 0.0)
     sid->pagesize_v = h * sid->pagerel_v;

   if (!limit)
     y += offset;
   else
     {
        dy = (sid->pagesize_v * ((double)sid->page_limit_v - 0.5));

        if (offset > 0)
          y += (abs(offset) < dy ? offset : dy);
        else
          y += (abs(offset) < dy ? offset : -(dy + 1));
     }

   if (sid->pagesize_v > 0)
     {
        if (y >= 0)
          y = y + (sid->pagesize_v * 0.5);
        else if (y < 0 && sid->loop_v)
          y = y - (sid->pagesize_v * 0.5);
        y = y / (sid->pagesize_v);
        y = y * (sid->pagesize_v);
     }
   if (!sid->loop_v)
     {
        if ((y + h) > ch) y = ch - h;
        if (y < miny) y = miny;
     }

   return y;
}

static void
_elm_scroll_scroll_to_x_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord px, py;
   double t, tt;

   if (!sid->pan_obj) goto on_end;

   t = ecore_loop_time_get();
   tt = (t - sid->scrollto.x.t_start) /
     (sid->scrollto.x.t_end - sid->scrollto.x.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   px = (sid->scrollto.x.start * (1.0 - tt)) +
     (sid->scrollto.x.end * tt);
   if (t >= sid->scrollto.x.t_end)
     {
        px = sid->scrollto.x.end;
        elm_interface_scrollable_content_pos_set(sid->obj, px, py, EINA_TRUE);
        sid->down.sx = px;
        sid->down.x = sid->down.history[0].x;
        sid->down.pdx = 0;
        _elm_scroll_wanted_coordinates_update(sid, px, py);
        if ((!sid->scrollto.y.animator) && (!sid->down.bounce_y_animator))
          _elm_scroll_anim_stop(sid);
        goto on_end;
     }
   elm_interface_scrollable_content_pos_set(sid->obj, px, py, EINA_TRUE);
   _elm_scroll_wanted_coordinates_update(sid, px, py);
   return;

 on_end:
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
}

static void
_elm_scroll_scroll_to_y_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord px, py;
   double t, tt;

   if (!sid->pan_obj) goto on_end;

   t = ecore_loop_time_get();
   tt = (t - sid->scrollto.y.t_start) /
     (sid->scrollto.y.t_end - sid->scrollto.y.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   py = (sid->scrollto.y.start * (1.0 - tt)) +
     (sid->scrollto.y.end * tt);
   if (t >= sid->scrollto.y.t_end)
     {
        py = sid->scrollto.y.end;
        elm_interface_scrollable_content_pos_set(sid->obj, px, py, EINA_TRUE);
        sid->down.sy = py;
        sid->down.y = sid->down.history[0].y;
        sid->down.pdy = 0;
        _elm_scroll_wanted_coordinates_update(sid, px, py);
        if ((!sid->scrollto.x.animator) && (!sid->down.bounce_x_animator))
          _elm_scroll_anim_stop(sid);
        goto on_end;
     }
   elm_interface_scrollable_content_pos_set(sid->obj, px, py, EINA_TRUE);
   _elm_scroll_wanted_coordinates_update(sid, px, py);

   return;

 on_end:
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
}

static void
_elm_scroll_scroll_to_y(Elm_Scrollable_Smart_Interface_Data *sid,
                        double t_in,
                        Evas_Coord pos_y)
{
   Evas_Coord px = 0, py = 0, x, y, w, h;
   double t;

   if (!sid->pan_obj) return;

   if (sid->freeze) return;
   if (t_in <= 0.0)
     {
        elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
        elm_interface_scrollable_content_viewport_geometry_get
              (sid->obj, NULL, NULL, &w, &h);
        y = pos_y;
        elm_interface_scrollable_content_region_set(sid->obj, x, y, w, h);
        return;
     }
   t = ecore_loop_time_get();
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   if (py == pos_y) return;
   sid->scrollto.y.start = py;
   sid->scrollto.y.end = pos_y;
   sid->scrollto.y.t_start = t;
   sid->scrollto.y.t_end = t + t_in;
   if (!sid->scrollto.y.animator)
     {
        ELM_ANIMATOR_CONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
        if (!sid->scrollto.x.animator)
          _elm_scroll_anim_start(sid);
     }
   if (sid->down.bounce_y_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
        _elm_scroll_momentum_end(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   sid->bouncemey = EINA_FALSE;
}

static void
_elm_scroll_scroll_to_x(Elm_Scrollable_Smart_Interface_Data *sid,
                        double t_in,
                        Evas_Coord pos_x)
{
   Evas_Coord px = 0, py = 0, x, y, w, h;
   double t;

   if (!sid->pan_obj) return;

   if (sid->freeze) return;
   if (t_in <= 0.0)
     {
        elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
        elm_interface_scrollable_content_viewport_geometry_get
              (sid->obj, NULL, NULL, &w, &h);
        x = pos_x;
        elm_interface_scrollable_content_region_set
              (sid->obj, x, y, w, h);
        return;
     }
   t = ecore_loop_time_get();
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   if (px == pos_x) return;
   sid->scrollto.x.start = px;
   sid->scrollto.x.end = pos_x;
   sid->scrollto.x.t_start = t;
   sid->scrollto.x.t_end = t + t_in;
   if (!sid->scrollto.x.animator)
     {
        ELM_ANIMATOR_CONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
        if (!sid->scrollto.y.animator)
          _elm_scroll_anim_start(sid);
     }
   if (sid->down.bounce_x_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
        _elm_scroll_momentum_end(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   sid->bouncemex = EINA_FALSE;
}

static Eina_Bool
_elm_scroll_running_momentum_speed_get(Elm_Scrollable_Smart_Interface_Data *sid, double *vx, double *vy)
{
   double p = 0;
   int remain_x = 0, remain_y = 0;
   double remain_dur = 0;

   p = (ecore_loop_time_get() -  sid->down.anim_start) / sid->down.anim_dur;

   // if momentum animation is not running now
   if ( p > 1.0 || p < 0)
     {
        if (vx) *vx = 0;
        if (vy) *vy = 0;
        return EINA_FALSE;
     }

   // calculate current velecity from remain distance and time
   remain_x = sid->down.dx -  sid->down.dx * p;
   remain_y = sid->down.dy -  sid->down.dy * p;
   remain_dur = sid->down.anim_dur -  sid->down.anim_dur * p;

   if (vx) *vx = remain_x / remain_dur;
   if (vy) *vy = remain_y / remain_dur;

   return EINA_TRUE;
}

static Eina_Bool
_elm_scroll_momentum_calc(int dx, int dy, double dt, double *vx, double *vy, int *dist_x, int *dist_y, double *dur)
{
   int n = 0;
   double vel = 0, vel_x = 0, vel_y = 0;
   double fvel = 0, fvel_x = 0, fvel_y = 0;
   int distance_x = 0 , distance_y = 0;
   int sign_dx = 0, sign_dy = 0;
   int sign_vx = 0, sign_vy = 0;


   double r = _elm_config->thumbscroll_momentum_friction;
   const int min_px = 3;

   if ( dt == 0 ) return EINA_FALSE;

   // store sign value of distance
   sign_dx = (dx > 0) - (dx < 0);
   sign_dy = (dy > 0) - (dy < 0);

   if (vx) sign_vx = (*vx > 0) - (*vx < 0);
   if (vy) sign_vy = (*vy > 0) - (*vy < 0);

   // scale factor must be below 1.0
   if ( r >=  1 ) r = 0.99;

   if (vx && (sign_dx == sign_vx)) vel_x = *vx;
   if (vy && (sign_dy == sign_vy)) vel_y = *vy;

   // calculate time based velecity (unit : px/second)
   vel_x += dx / dt;
   vel_y += dy / dt;

  vel_x *= _elm_config->thumbscroll_sensitivity_friction;
  vel_y *= _elm_config->thumbscroll_sensitivity_friction;

   vel = sqrt((vel_x * vel_x) + (vel_y * vel_y));

   // calculate frame based velecity (unit : px/frame)
   fvel_x = vel_x * (1/60.0);
   fvel_y = vel_y * (1/60.0);
   fvel = vel * (1/60.0);

   if (abs(fvel) < _elm_config->thumbscroll_threshold ) return EINA_FALSE;

   // calculate a number of frames to reach min_px when it follows a geometric sequence with scale factor r
   n = log(min_px/fvel) / log(r);

   distance_x = fvel_x * (( 1 - pow(r, n)) / (1 - r));
   distance_y = fvel_y * (( 1 - pow(r, n)) / (1 - r));

   // remove sign of distance
   distance_x = abs(distance_x);
   distance_y = abs(distance_y);

   // clamp distance by thumbscroll_momentum_distance_max
   distance_x = CLAMP(distance_x, 0, _elm_config->thumbscroll_momentum_distance_max);
   distance_y = CLAMP(distance_y, 0, _elm_config->thumbscroll_momentum_distance_max);

   // restore sign
   distance_x *= sign_dx;
   distance_y *= sign_dy;

   if (dist_x) *dist_x = distance_x;
   if (dist_y) *dist_y = distance_y;

   if (vx) *vx = vel_x;
   if (vy) *vy = vel_y;

   // convert to time based animation duration
   if (dur) *dur = CLAMP((n / 60.0), _elm_config->thumbscroll_momentum_animation_duration_min_limit, _elm_config->thumbscroll_momentum_animation_duration_max_limit);

   return EINA_TRUE;
}

static void
_elm_scroll_mouse_up_event_cb(void *data,
                              Evas *e,
                              Evas_Object *obj EINA_UNUSED,
                              void *event_info)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord x = 0, y = 0, ox = 0, oy = 0;
   Evas_Event_Mouse_Up *ev;

   if (!sid->pan_obj) return;

   if ((sid->block & EFL_UI_SCROLL_BLOCK_VERTICAL) &&
       (sid->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL))
     return;

#ifdef SMOOTHDBG
   if (_elm_scroll_smooth_debug) _elm_scroll_smooth_debug_shutdown();
#endif

   ev = event_info;
   sid->down.hold_parent = EINA_FALSE;
   sid->down.dx = 0;
   sid->down.dy = 0;
   evas_post_event_callback_push(e, _elm_scroll_post_event_up, sid);

   // FIXME: respect elm_widget_scroll_hold_get of parent container
   if (!_elm_config->thumbscroll_enable) return;

   if (ev->button == 1)
     {
        if (sid->down.onhold_animator)
          {
             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.onhold_animator, _elm_scroll_on_hold_animator, sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
        x = ev->canvas.x - sid->down.x;
        y = ev->canvas.y - sid->down.y;
        if (sid->down.dragged)
          {
             _elm_scroll_drag_stop(sid);
             if ((!sid->hold) && (!sid->freeze))
               {
                  int i;
                  double t, at, dt;
                  Evas_Coord ax, ay, dx, dy, vel;

                  double vel_x, vel_y, dur;
                  Evas_Coord dist_x, dist_y;

                  t = ev->timestamp / 1000.0;

                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                  ax = ev->canvas.x;
                  ay = ev->canvas.y;
                  at = 0.0;
#ifdef SCROLLDBG
                  DBG("------ %i %i\n", ev->canvas.x, ev->canvas.y);
#endif
                  for (i = 0; i < 60; i++)
                    {
                       dt = t - sid->down.history[i].timestamp;
                       if (dt > 0.2) break;
#ifdef SCROLLDBG
                       DBG("H: %i %i @ %1.3f\n",
                           sid->down.history[i].x,
                           sid->down.history[i].y, dt);
#endif
                       at += dt;
                       ax += sid->down.history[i].x;
                       ay += sid->down.history[i].y;
                    }
                  ax /= (i + 1);
                  ay /= (i + 1);
                  at /= (i + 1);

                  dx = ev->canvas.x - ax;
                  dy = ev->canvas.y - ay;

                  _elm_scroll_running_momentum_speed_get(sid, &vel_x, &vel_y);

                  if (at > 0 && _elm_scroll_momentum_calc(dx, dy, at, &vel_x, &vel_y, &dist_x, &dist_y, &dur))
                    {
                       vel = sqrt((vel_x * vel_x) + (vel_y * vel_y));

                       if ((_elm_config->thumbscroll_momentum_friction > 0.0) &&
                           (vel > _elm_config->thumbscroll_momentum_threshold))
                         {
                            int minx, miny, mx, my, px, py;
                            double dtt = 0.0;

                            elm_obj_pan_pos_min_get
                                  (sid->pan_obj, &minx, &miny);
                            elm_obj_pan_pos_max_get
                                  (sid->pan_obj, &mx, &my);
                            elm_obj_pan_pos_get(sid->pan_obj, &px, &py);

                            sid->down.dx = dist_x;
                            sid->down.dy = dist_y;
                            sid->down.anim_dur = dur;

                            if (abs(sid->down.dx) > _elm_config->thumbscroll_acceleration_threshold &&
                                (dtt < _elm_config->thumbscroll_acceleration_time_limit) &&
                                (((sid->down.dx > 0) && (sid->down.pdx > 0)) ||
                                ((sid->down.dx < 0) && (sid->down.pdx < 0))))
                              if (px > minx && px < mx)
                                sid->down.dx += (double)sid->down.pdx * _elm_config->thumbscroll_acceleration_weight;
                            if (abs(sid->down.dy) > _elm_config->thumbscroll_acceleration_threshold &&
                                (dtt < _elm_config->thumbscroll_acceleration_time_limit) &&
                                (((sid->down.dy > 0) && (sid->down.pdy > 0)) ||
                                ((sid->down.dy < 0) && (sid->down.pdy < 0))))
                              if (py > miny && py < my)
                              {
                                sid->down.dy += (double)sid->down.pdy * _elm_config->thumbscroll_acceleration_weight;
                              }
                            sid->down.pdx = sid->down.dx;
                            sid->down.pdy = sid->down.dy;
                            ox = -sid->down.dx;
                            oy = -sid->down.dy;
                            if (!_paging_is_enabled(sid))
                              {
                                 if ((!sid->down.momentum_animator) &&
                                     (!sid->momentum_animator_disabled) &&
                                     (sid->obj) &&
                                     (!elm_widget_scroll_child_locked_y_get
                                        (sid->obj)))
                                   {
                                      ELM_ANIMATOR_CONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
                                      ev->event_flags |=
                                        EVAS_EVENT_FLAG_ON_SCROLL;
                                      _elm_scroll_anim_start(sid);
                                   }
                                 sid->down.anim_start = ecore_loop_time_get();
                                 elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
                                 sid->down.sx = x;
                                 sid->down.sy = y;
                                 sid->down.b0x = 0;
                                 sid->down.b0y = 0;
                              }
                         }
                    }
               }
             else
               {
                  sid->down.pdx = 0;
                  sid->down.pdy = 0;
               }
             evas_event_feed_hold(e, 0, ev->timestamp, ev->data);
             if (_paging_is_enabled(sid))
               {
                  Evas_Coord pgx, pgy;

                  elm_interface_scrollable_content_pos_get
                        (sid->obj, &x, &y);
                  if ((!sid->obj) ||
                      (!elm_widget_scroll_child_locked_x_get
                         (sid->obj)))
                    {
                       pgx = _elm_scroll_page_x_get(sid, ox, EINA_TRUE);
                       if (pgx != x &&
                           !(sid->block &
                            EFL_UI_SCROLL_BLOCK_HORIZONTAL))
                         {
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                            _elm_scroll_scroll_to_x
                              (sid, _elm_config->page_scroll_friction, pgx);
                         }
                    }
                  if ((!sid->obj) ||
                      (!elm_widget_scroll_child_locked_y_get
                         (sid->obj)))
                    {
                       pgy = _elm_scroll_page_y_get(sid, oy, EINA_TRUE);
                       if (pgy != y &&
                           !(sid->block &
                            EFL_UI_SCROLL_BLOCK_VERTICAL))
                         {
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                            _elm_scroll_scroll_to_y
                              (sid, _elm_config->page_scroll_friction, pgy);
                         }
                    }
               }
          }
        else
          {
             sid->down.pdx = 0;
             sid->down.pdy = 0;
             if (_paging_is_enabled(sid))
               {
                  Evas_Coord pgx, pgy;

                  elm_interface_scrollable_content_pos_get
                        (sid->obj, &x, &y);
                  if ((!sid->obj) ||
                      (!elm_widget_scroll_child_locked_x_get
                         (sid->obj)))
                    {
                       pgx = _elm_scroll_page_x_get(sid, ox, EINA_TRUE);
                       if (pgx != x)
                         _elm_scroll_scroll_to_x
                           (sid, _elm_config->page_scroll_friction, pgx);
                    }
                  if ((!sid->obj) ||
                      (!elm_widget_scroll_child_locked_y_get
                         (sid->obj)))
                    {
                       pgy = _elm_scroll_page_y_get(sid, oy, EINA_TRUE);
                       if (pgy != y)
                         _elm_scroll_scroll_to_y
                           (sid, _elm_config->page_scroll_friction, pgy);
                    }
               }
          }
        if (sid->down.hold_animator)
          {
             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.hold_animator, _elm_scroll_hold_animator, sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
        ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);
        if (sid->down.scroll)
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
             sid->down.scroll = EINA_FALSE;
          }
        if (sid->down.hold)
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             sid->down.hold = EINA_FALSE;
          }
        sid->down.dragged_began = EINA_FALSE;
        sid->down.dir_x = EINA_FALSE;
        sid->down.dir_y = EINA_FALSE;
        sid->down.want_dragged = EINA_FALSE;
        sid->down.dragged = EINA_FALSE;
        sid->down.now = EINA_FALSE;
        elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
        elm_interface_scrollable_content_pos_set(sid->obj, x, y, EINA_TRUE);
        _elm_scroll_wanted_coordinates_update(sid, x, y);

        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);

        if (!_paging_is_enabled(sid))
          _elm_scroll_bounce_eval(sid);
     }
}

static void
_elm_scroll_mouse_down_event_cb(void *data,
                                Evas *e EINA_UNUSED,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info)
{
   Elm_Scrollable_Smart_Interface_Data *sid;
   Evas_Event_Mouse_Down *ev;
   Evas_Coord x = 0, y = 0;

   sid = data;
   ev = event_info;

   if ((sid->block & EFL_UI_SCROLL_BLOCK_VERTICAL) &&
       (sid->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL))
     return;

#ifdef SMOOTHDBG
   if (getenv("ELS_SCROLLER_SMOOTH_DEBUG")) _elm_scroll_smooth_debug = 1;
   if (_elm_scroll_smooth_debug) _elm_scroll_smooth_debug_init();
#endif

   if (!_elm_config->thumbscroll_enable) return;

   sid->down.hold = EINA_FALSE;
   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator) ||
       (sid->down.momentum_animator) || (sid->scrollto.x.animator) ||
       (sid->scrollto.y.animator))
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL |
          EVAS_EVENT_FLAG_ON_HOLD;
        sid->down.scroll = EINA_TRUE;
        sid->down.hold = EINA_TRUE;
        _elm_scroll_anim_stop(sid);
     }
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
   if (sid->down.bounce_x_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
        sid->bouncemex = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.bounce_y_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
        sid->bouncemey = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.hold_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.hold_animator, _elm_scroll_hold_animator, sid);
        _elm_scroll_drag_stop(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);
   if (sid->down.momentum_animator)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
        sid->down.bounce_x_hold = EINA_FALSE;
        sid->down.bounce_y_hold = EINA_FALSE;
        sid->down.ax = 0;
        sid->down.ay = 0;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (ev->button == 1)
     {
        sid->down.est_timestamp_diff =
          ecore_loop_time_get() - ((double)ev->timestamp / 1000.0);
        sid->down.now = EINA_TRUE;
        sid->down.dragged = EINA_FALSE;
        sid->down.dir_x = EINA_FALSE;
        sid->down.dir_y = EINA_FALSE;
        sid->down.x = ev->canvas.x;
        sid->down.y = ev->canvas.y;
        elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
        sid->down.sx = x;
        sid->down.sy = y;
        sid->down.locked = EINA_FALSE;
        memset(&(sid->down.history[0]), 0,
               sizeof(sid->down.history[0]) * 60);
        sid->down.history[0].timestamp = ev->timestamp / 1000.0;
        sid->down.dragged_began_timestamp = sid->down.history[0].timestamp;
        sid->down.history[0].x = ev->canvas.x;
        sid->down.history[0].y = ev->canvas.y;
     }
   sid->down.dragged_began = EINA_FALSE;
   sid->down.hold_parent = EINA_FALSE;
   sid->down.cancelled = EINA_FALSE;
   if (sid->hold || sid->freeze)
     sid->down.want_reset = EINA_TRUE;
   else
     sid->down.want_reset = EINA_FALSE;
}

static Eina_Bool
_elm_scroll_can_scroll(Elm_Scrollable_Smart_Interface_Data *sid,
                       int dir)
{
   Evas_Coord mx = 0, my = 0, px = 0, py = 0, minx = 0, miny = 0;

   if (!sid->pan_obj) return EINA_FALSE;

   elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
   elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
   switch (dir)
     {
      case LEFT:
        if (px > minx) return EINA_TRUE;
        break;

      case RIGHT:
        if ((px - minx) < mx) return EINA_TRUE;
        break;

      case UP:
        if (py > miny) return EINA_TRUE;
        break;

      case DOWN:
        if ((py - miny) < my) return EINA_TRUE;
        break;

      default:
        break;
     }
   return EINA_FALSE;
}

static inline void
_elm_widget_parents_bounce_get(Eo *obj, Eina_Bool *horiz, Eina_Bool *vert)
{
   Evas_Object *parent_obj = obj;
   Eina_Bool h = EINA_FALSE, v = EINA_FALSE;

   *horiz = EINA_FALSE;
   *vert = EINA_FALSE;

   do
     {
        parent_obj = elm_widget_parent_get(parent_obj);
        if ((!parent_obj) || (!efl_isa(parent_obj, EFL_UI_WIDGET_CLASS))) break;

        if (efl_isa(parent_obj, ELM_INTERFACE_SCROLLABLE_MIXIN))
          {
             elm_interface_scrollable_bounce_allow_get(parent_obj, &h, &v);
             if (h) *horiz = EINA_TRUE;
             if (v) *vert = EINA_TRUE;
          }
     }
   while (parent_obj);
}

static Eina_Bool
_elm_scroll_post_event_move(void *data,
                            Evas *e EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Efl_Ui_Scroll_Block block;
   Eina_Bool horiz, vert;
   int start = 0;

   if (!sid->down.want_dragged) return EINA_TRUE;

   block = elm_widget_scroll_lock_get(sid->obj);
   _elm_widget_parents_bounce_get(sid->obj, &horiz, &vert);
   if (sid->down.hold_parent)
     {
        if ((sid->down.dir_x) && (horiz || !sid->bounce_horiz) &&
            !_elm_scroll_can_scroll(sid, sid->down.hdir))
          {
             sid->down.dir_x = EINA_FALSE;
          }
        if ((sid->down.dir_y) && (vert || !sid->bounce_vert) &&
            !_elm_scroll_can_scroll(sid, sid->down.vdir))
          {
             sid->down.dir_y = EINA_FALSE;
          }
        sid->down.dragged_began = EINA_TRUE;
     }
   if (sid->down.dir_x)
     {
        if ((!sid->obj) ||
            (!elm_widget_scroll_child_locked_x_get(sid->obj)))
          {
             if (sid->down.dragged_began)
               {
                  sid->down.want_dragged = EINA_FALSE;
                  sid->down.dragged = EINA_TRUE;
                  if (sid->obj)
                    {
                       block |= EFL_UI_SCROLL_BLOCK_HORIZONTAL;
                       elm_widget_scroll_lock_set(sid->obj, block);
                    }
                  start = 1;
               }
          }
        else
          {
             sid->down.dragged_began = EINA_TRUE;
             sid->down.dir_x = EINA_FALSE;
          }
     }
   if (sid->down.dir_y)
     {
        if ((!sid->obj) ||
            (!elm_widget_scroll_child_locked_y_get(sid->obj)))
          {
             if (sid->down.dragged_began)
               {
                  sid->down.want_dragged = EINA_FALSE;
                  sid->down.dragged = EINA_TRUE;
                  if (sid->obj)
                    {
                       block |= EFL_UI_SCROLL_BLOCK_VERTICAL;
                       elm_widget_scroll_lock_set(sid->obj, block);
                    }
                  start = 1;
               }
          }
        else
          {
             sid->down.dragged_began = EINA_TRUE;
             sid->down.dir_y = EINA_FALSE;
          }
     }
   if ((!sid->down.dir_x) && (!sid->down.dir_y))
     {
        sid->down.cancelled = EINA_TRUE;
     }
   if (start) _elm_scroll_drag_start(sid);

   return EINA_TRUE;
}

static void
_elm_scroll_down_coord_eval(Elm_Scrollable_Smart_Interface_Data *sid,
                            Evas_Coord *x,
                            Evas_Coord *y)
{
   Evas_Coord minx, miny;

   if (!sid->pan_obj) return;

   if (sid->down.dir_x) *x = sid->down.sx - (*x - sid->down.x);
   else *x = sid->down.sx;
   if (sid->down.dir_y) *y = sid->down.sy - (*y - sid->down.y);
   else *y = sid->down.sy;

   if ((sid->down.dir_x) || (sid->down.dir_y))
     {
        if (!((sid->down.dir_x) && (sid->down.dir_y)))
          {
             if (sid->down.dir_x) *y = sid->down.locked_y;
             else *x = sid->down.locked_x;
          }
     }

   elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);

   if (!sid->loop_h && *x < minx)
     *x += (minx - *x) * _elm_config->thumbscroll_border_friction;
   else if (!sid->loop_h && sid->content_info.w <= sid->w)
     *x += (sid->down.sx - *x) * _elm_config->thumbscroll_border_friction;
   else if (!sid->loop_h && (sid->content_info.w - sid->w + minx) < *x)
     *x += (sid->content_info.w - sid->w + minx - *x) *
       _elm_config->thumbscroll_border_friction;

   if (!sid->loop_v && *y < miny)
     *y += (miny - *y) * _elm_config->thumbscroll_border_friction;
   else if (!sid->loop_v && sid->content_info.h <= sid->h)
     *y += (sid->down.sy - *y) * _elm_config->thumbscroll_border_friction;
   else if (!sid->loop_v && (sid->content_info.h - sid->h + miny) < *y)
     *y += (sid->content_info.h - sid->h + miny - *y) *
       _elm_config->thumbscroll_border_friction;
}

static Eina_Bool
_elm_scroll_hold_enterer(void *data)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord ox = 0, oy = 0, fx = 0, fy = 0;
//   Evas_Coord fy2;

   sid->down.hold_enterer = NULL;

   fx = sid->down.hold_x;
   fy = sid->down.hold_y;
//   fy2 = fy;
   if ((_elm_config->scroll_smooth_amount > 0.0) &&
       (_elm_config->scroll_smooth_time_window > 0.0))
     {
        int i, count = 0;
        Evas_Coord basex = 0, basey = 0, x, y;
        double dt, tdiff, tnow, twin, ttot;
        double xx, yy, tot;
        struct
          {
             Evas_Coord x, y;
             double t;
          } pos[100];

        tdiff = sid->down.est_timestamp_diff;
        tnow = ecore_loop_time_get();
        twin = _elm_config->scroll_smooth_time_window;
        for (i = 0; i < 60; i++)
          {
             if ((sid->down.history[i].timestamp - tdiff) > tnow)
               continue;
             if ((sid->down.history[i].timestamp >
                 sid->down.dragged_began_timestamp) || (count == 0))
               {
                  x = sid->down.history[i].x;
                  y = sid->down.history[i].y;
                  _elm_scroll_down_coord_eval(sid, &x, &y);
                  if (count == 0)
                    {
                       basex = x;
                       basey = y;
                    }
                  dt = (tnow + tdiff) - sid->down.history[i].timestamp;
                  if ((dt > twin) && (count > 0)) break;
                  if ((dt > 0.0) && (count == 0))
                    {
                       pos[count].x = x - basex;
                       pos[count].y = y - basey;
                       pos[count].t = 0.0;
                       count++;
                    }
                  pos[count].x = x - basex;
                  pos[count].y = y - basey;
                  pos[count].t = dt;
                  count++;
               }
          }
        if (count > 0)
          {
             xx = 0.0;
             yy = 0.0;
             tot = 0.0;
             ttot = pos[count - 1].t;
             for (i = 0; i < count; i++)
               {
                  double wt;

                  if (ttot > 0.0)
                    {
                       if (i < (count - 1))
                         wt = (ttot - pos[i].t) * (pos[i + 1].t - pos[i].t);
                       else
                         wt = 0.0;
                    }
                  else wt = 1.0;

                  xx += ((double)(pos[i].x)) * wt;
                  yy += ((double)(pos[i].y)) * wt;
                  tot += wt;
               }
             if (tot > 0.0)
               {
                  xx = basex + (xx / tot);
                  yy = basey + (yy / tot);
                  fx =
                    (_elm_config->scroll_smooth_amount * xx) +
                    ((1.0 - _elm_config->scroll_smooth_amount) * fx);
                  fy =
                    (_elm_config->scroll_smooth_amount * yy) +
                    ((1.0 - _elm_config->scroll_smooth_amount) * fy);
               }
          }
     }
//   printf("%1.5f %i %i\n",
//          ecore_loop_time_get() - sid->down.dragged_began_timestamp,
//          fy, fy2);

   elm_interface_scrollable_content_pos_get(sid->obj, &ox, &oy);
   if (sid->down.dir_x)
     {
        if ((!sid->obj) ||
            (!elm_widget_scroll_child_locked_x_get(sid->obj)))
          ox = fx;
     }
   if (sid->down.dir_y)
     {
        if ((!sid->obj) ||
            (!elm_widget_scroll_child_locked_y_get(sid->obj)))
          oy = fy;
     }

#ifdef SMOOTHDBG
   if (_elm_scroll_smooth_debug)
     _elm_scroll_smooth_debug_movetime_add(ox, oy);
#endif

   elm_interface_scrollable_content_pos_set(sid->obj, ox, oy, EINA_TRUE);

   return EINA_FALSE;
}

static void
_elm_scroll_hold_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   ecore_idle_enterer_del(sid->down.hold_enterer);
   sid->down.hold_enterer =
     ecore_idle_enterer_before_add(_elm_scroll_hold_enterer, sid);
}

static void
_elm_scroll_on_hold_animator(void *data, const Efl_Event *event EINA_UNUSED)
{
   double t, td;
   double vx, vy;
   Evas_Coord x, y, ox = 0, oy = 0;
   Elm_Scrollable_Smart_Interface_Data *sid;

   sid = data;
   t = ecore_loop_time_get();
   if (sid->down.onhold_tlast > 0.0)
     {
        td = t - sid->down.onhold_tlast;
        vx = sid->down.onhold_vx * td *
          (double)_elm_config->thumbscroll_hold_threshold * 2.0;
        vy = sid->down.onhold_vy * td *
          (double)_elm_config->thumbscroll_hold_threshold * 2.0;
        elm_interface_scrollable_content_pos_get(sid->obj, &ox, &oy);
        x = ox;
        y = oy;

        if (sid->down.dir_x)
          {
             if ((!sid->obj) ||
                 (!elm_widget_scroll_child_locked_x_get(sid->obj)))
               {
                  sid->down.onhold_vxe += vx;
                  x = ox + (int)sid->down.onhold_vxe;
                  sid->down.onhold_vxe -= (int)sid->down.onhold_vxe;
               }
          }

        if (sid->down.dir_y)
          {
             if ((!sid->obj) ||
                 (!elm_widget_scroll_child_locked_y_get(sid->obj)))
               {
                  sid->down.onhold_vye += vy;
                  y = oy + (int)sid->down.onhold_vye;
                  sid->down.onhold_vye -= (int)sid->down.onhold_vye;
               }
          }

        elm_interface_scrollable_content_pos_set(sid->obj, x, y, EINA_TRUE);
     }
   sid->down.onhold_tlast = t;
}

static void
_elm_scroll_mouse_move_event_cb(void *data,
                                Evas *e,
                                Evas_Object *obj EINA_UNUSED,
                                void *event_info)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Event_Mouse_Move *ev;
   Evas_Coord x = 0, y = 0;

   if (!sid->pan_obj) return;

   if ((sid->block & EFL_UI_SCROLL_BLOCK_VERTICAL) &&
       (sid->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL))
     return;

   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     sid->down.hold_parent = EINA_TRUE;
   evas_post_event_callback_push(e, _elm_scroll_post_event_move, sid);

   // FIXME: respect elm_widget_scroll_hold_get of parent container
   if (!_elm_config->thumbscroll_enable)
     return;

   if (!sid->down.now) return;

   if ((sid->scrollto.x.animator) && (!sid->hold) && (!sid->freeze) &&
       !(sid->block & EFL_UI_SCROLL_BLOCK_HORIZONTAL))
     {
        Evas_Coord px = 0;
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
        elm_obj_pan_pos_get(sid->pan_obj, &px, NULL);
        sid->down.sx = px;
        sid->down.x = sid->down.history[0].x;
     }

   if ((sid->scrollto.y.animator) && (!sid->hold) && (!sid->freeze) &&
       !(sid->block & EFL_UI_SCROLL_BLOCK_VERTICAL))
     {
        Evas_Coord py = 0;
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
        elm_obj_pan_pos_get(sid->pan_obj, NULL, &py);
        sid->down.sy = py;
        sid->down.y = sid->down.history[0].y;
     }

#ifdef SCROLLDBG
   DBG("::: %i %i\n", ev->cur.canvas.x, ev->cur.canvas.y);
#endif
   memmove(&(sid->down.history[1]), &(sid->down.history[0]),
           sizeof(sid->down.history[0]) * (60 - 1));
   sid->down.history[0].timestamp = ev->timestamp / 1000.0;
   sid->down.history[0].x = ev->cur.canvas.x;
   sid->down.history[0].y = ev->cur.canvas.y;

   if (!sid->down.dragged_began)
     {
        x = ev->cur.canvas.x - sid->down.x;
        y = ev->cur.canvas.y - sid->down.y;

        sid->down.hdir = -1;
        sid->down.vdir = -1;

        if (x > 0) sid->down.hdir = LEFT;
        else if (x < 0)
          sid->down.hdir = RIGHT;
        if (y > 0) sid->down.vdir = UP;
        else if (y < 0)
          sid->down.vdir = DOWN;

        if (x < 0) x = -x;
        if (y < 0) y = -y;

        if (sid->one_direction_at_a_time)
          {
             if (((x * x) + (y * y)) >
                 (_elm_config->thumbscroll_threshold *
                  _elm_config->thumbscroll_threshold))
               {
                  if (sid->one_direction_at_a_time ==
                      ELM_SCROLLER_SINGLE_DIRECTION_SOFT)
                    {
                       int dodir = 0;
                       if (x > (y * 2))
                         {
                            if (!(sid->block &
                                  EFL_UI_SCROLL_BLOCK_HORIZONTAL))
                              {
                                 sid->down.dir_x = EINA_TRUE;
                              }
                            sid->down.dir_y = EINA_FALSE;
                            dodir++;
                         }
                       if (y > (x * 2))
                         {
                            sid->down.dir_x = EINA_FALSE;
                            if (!(sid->block &
                                  EFL_UI_SCROLL_BLOCK_VERTICAL))
                              {
                                 sid->down.dir_y = EINA_TRUE;
                              }
                            dodir++;
                         }
                       if (!dodir)
                         {
                            if (!(sid->block &
                                  EFL_UI_SCROLL_BLOCK_HORIZONTAL))
                              {
                                 sid->down.dir_x = EINA_TRUE;
                              }
                            if (!(sid->block &
                                  EFL_UI_SCROLL_BLOCK_VERTICAL))
                              {
                                 sid->down.dir_y = EINA_TRUE;
                              }
                         }
                    }
                  else if (sid->one_direction_at_a_time ==
                           ELM_SCROLLER_SINGLE_DIRECTION_HARD)
                    {
                       if (x > y)
                         {
                            if (!(sid->block &
                                  EFL_UI_SCROLL_BLOCK_HORIZONTAL))
                              {
                                 sid->down.dir_x = EINA_TRUE;
                              }
                            sid->down.dir_y = EINA_FALSE;
                         }
                       if (y > x)
                         {
                            sid->down.dir_x = EINA_FALSE;
                            if (!(sid->block &
                                  EFL_UI_SCROLL_BLOCK_VERTICAL))
                              {
                                 sid->down.dir_y = EINA_TRUE;
                              }
                         }
                    }
               }
          }
        else
          {
             if (!(sid->block &
                   EFL_UI_SCROLL_BLOCK_HORIZONTAL))
               {
                  sid->down.dir_x = EINA_TRUE;
               }
             if (!(sid->block &
                   EFL_UI_SCROLL_BLOCK_VERTICAL))
               {
                  sid->down.dir_y = EINA_TRUE;
               }
          }
     }
   if ((!sid->hold) && (!sid->freeze))
     {
        if ((sid->down.dragged) ||
            (((x * x) + (y * y)) >
             (_elm_config->thumbscroll_threshold *
              _elm_config->thumbscroll_threshold)))
          {
             if (!sid->down.dragged_began &&
                 _elm_config->scroll_smooth_start_enable)
               {
                  sid->down.x = ev->cur.canvas.x;
                  sid->down.y = ev->cur.canvas.y;
                  sid->down.dragged_began_timestamp = ev->timestamp / 1000.0;
               }

             if (!sid->down.dragged)
               {
                  sid->down.want_dragged = EINA_TRUE;
               }
             if ((((_elm_scroll_can_scroll(sid, LEFT) || _elm_scroll_can_scroll(sid, RIGHT)) && sid->down.dir_x) ||
                  ((_elm_scroll_can_scroll(sid, UP) || _elm_scroll_can_scroll(sid, DOWN)) && sid->down.dir_y)) &&
                 !sid->down.dragged_began)
               {
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                  sid->down.dragged_began = EINA_TRUE;
               }
             else if (sid->down.dragged_began)
               {
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
               }
             if (sid->down.dir_x)
               x = sid->down.sx - (ev->cur.canvas.x - sid->down.x);
             else
               x = sid->down.sx;
             if (sid->down.dir_y)
               y = sid->down.sy - (ev->cur.canvas.y - sid->down.y);
             else
               y = sid->down.sy;
             if (sid->down.want_reset)
               {
                  sid->down.x = ev->cur.canvas.x;
                  sid->down.y = ev->cur.canvas.y;
                  sid->down.want_reset = EINA_FALSE;
               }
             if ((sid->down.dir_x) || (sid->down.dir_y))
               {
                  if (!sid->down.locked)
                    {
                       sid->down.locked_x = x;
                       sid->down.locked_y = y;
                       sid->down.locked = EINA_TRUE;
                    }
                  if (!((sid->down.dir_x) && (sid->down.dir_y)))
                    {
                       if (sid->down.dir_x) y = sid->down.locked_y;
                       else x = sid->down.locked_x;
                    }
               }
             {
                Evas_Coord minx = 0, miny = 0, mx, my;

                elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
                elm_obj_pan_pos_max_get(sid->pan_obj, &mx, &my);
                if (!sid->loop_v && y < miny)
                  y += (miny - y) *
                    _elm_config->thumbscroll_border_friction;
                else if (!sid->loop_v && my <= 0)
                  y += (sid->down.sy - y) *
                    _elm_config->thumbscroll_border_friction;
                else if (!sid->loop_v && (my + miny) < y)
                  y += (my + miny - y) *
                    _elm_config->thumbscroll_border_friction;
                if (!sid->loop_h && x < minx)
                  x += (minx - x) *
                    _elm_config->thumbscroll_border_friction;
                else if (!sid->loop_h && mx <= 0)
                  x += (sid->down.sx - x) *
                    _elm_config->thumbscroll_border_friction;
                else if (!sid->loop_h && (mx + minx) < x)
                  x += (mx + minx - x) *
                    _elm_config->thumbscroll_border_friction;
             }

             sid->down.hold_x = x;
             sid->down.hold_y = y;
             ELM_ANIMATOR_CONNECT(sid->obj, sid->down.hold_animator, _elm_scroll_hold_animator, sid);
          }
        else
          {
             if (sid->down.dragged_began)
               {
                  if ((_elm_scroll_can_scroll(sid, sid->down.hdir) && sid->down.dir_x) ||
                      (_elm_scroll_can_scroll(sid, sid->down.vdir) && sid->down.dir_y))
                    {
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                    }
                  if (!sid->down.hold)
                    {
                       sid->down.hold = EINA_TRUE;
                       evas_event_feed_hold
                         (e, 1, ev->timestamp, ev->data);
                    }
               }
          }
     }
   else if (!sid->freeze)
     {
        double vx = 0.0, vy = 0.0;

        x = ev->cur.canvas.x - sid->x;
        y = ev->cur.canvas.y - sid->y;
        if (x < _elm_config->thumbscroll_hold_threshold)
          {
             if (_elm_config->thumbscroll_hold_threshold > 0.0)
               vx = -(double)(_elm_config->thumbscroll_hold_threshold - x)
                 / _elm_config->thumbscroll_hold_threshold;
             else
               vx = -1.0;
          }
        else if (x > (sid->w - _elm_config->thumbscroll_hold_threshold))
          {
             if (_elm_config->thumbscroll_hold_threshold > 0.0)
               vx = (double)(_elm_config->thumbscroll_hold_threshold -
                             (sid->w - x)) /
                 _elm_config->thumbscroll_hold_threshold;
             else
               vx = 1.0;
          }
        if (y < _elm_config->thumbscroll_hold_threshold)
          {
             if (_elm_config->thumbscroll_hold_threshold > 0.0)
               vy = -(double)(_elm_config->thumbscroll_hold_threshold - y)
                 / _elm_config->thumbscroll_hold_threshold;
             else
               vy = -1.0;
          }
        else if (y > (sid->h - _elm_config->thumbscroll_hold_threshold))
          {
             if (_elm_config->thumbscroll_hold_threshold > 0.0)
               vy = (double)(_elm_config->thumbscroll_hold_threshold -
                             (sid->h - y)) /
                 _elm_config->thumbscroll_hold_threshold;
             else
               vy = 1.0;
          }
        if ((vx != 0.0) || (vy != 0.0))
          {
             sid->down.onhold_vx = vx;
             sid->down.onhold_vy = vy;
             if (!sid->down.onhold_animator)
               {
                  sid->down.onhold_vxe = 0.0;
                  sid->down.onhold_vye = 0.0;
                  sid->down.onhold_tlast = 0.0;

                  ELM_ANIMATOR_CONNECT(sid->obj, sid->down.onhold_animator, _elm_scroll_on_hold_animator, sid);
               }
          }
        else
          {
             if (sid->down.onhold_animator)
               {
                  ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.onhold_animator, _elm_scroll_on_hold_animator, sid);
                  if (sid->content_info.resized)
                    _elm_scroll_wanted_region_set(sid->obj);
               }
          }
     }
}

static void
_elm_scroll_page_adjust(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord x, y, w, h;

   if (!_paging_is_enabled(sid)) return;

   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &w, &h);

   x = _elm_scroll_page_x_get(sid, 0, EINA_TRUE);
   y = _elm_scroll_page_y_get(sid, 0, EINA_TRUE);

   elm_interface_scrollable_content_region_set(sid->obj, x, y, w, h);
}

static void
_elm_scroll_reconfigure(Elm_Scrollable_Smart_Interface_Data *sid)
{
   _elm_scroll_scroll_bar_size_adjust(sid);
   _elm_scroll_page_adjust(sid);
}

static void
_on_edje_move(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *edje_obj,
              void *event_info EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   int x, y;

   evas_object_geometry_get(edje_obj, &x, &y, NULL, NULL);

   sid->x = x;
   sid->y = y;

   _elm_scroll_reconfigure(sid);
}

static void
_on_edje_resize(void *data,
                Evas *e,
                Evas_Object *edje_obj,
                void *event_info EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord w, h;
   int current_calc;
   Eina_Bool reconf_ok = EINA_TRUE;

   evas_object_geometry_get(edje_obj, NULL, NULL, &w, &h);

   sid->w = w;
   sid->h = h;

   current_calc = evas_smart_objects_calculate_count_get(e);
   if (current_calc == sid->current_calc)
     {
        sid->size_count++;
        if (sid->size_count > 3) reconf_ok = EINA_FALSE;
     }
   else
     {
        sid->current_calc = current_calc;
        sid->size_count = 0;
     }
   if (reconf_ok) _elm_scroll_reconfigure(sid);
   _elm_scroll_wanted_region_set(sid->obj);
}

static void
_scroll_edje_object_attach(Evas_Object *obj)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(obj, sid);

   evas_object_event_callback_add
     (sid->edje_obj, EVAS_CALLBACK_RESIZE, _on_edje_resize, sid);
   evas_object_event_callback_add
     (sid->edje_obj, EVAS_CALLBACK_MOVE, _on_edje_move, sid);

   edje_object_signal_callback_add
     (sid->edje_obj, "reload", "elm", _elm_scroll_reload_cb, sid);

   if (!_elm_scroll_has_bars(sid))
     return;

   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
   else
     iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
   edje_object_signal_callback_add
     (sid->edje_obj, "drag", iface_scr_dragable_vbar, _elm_scroll_vbar_drag_cb,
     sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,set", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,start", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_start_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,stop", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_stop_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,step", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,page", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_cb, sid);

   edje_object_signal_callback_add
     (sid->edje_obj, "elm,vbar,press", "elm",
     _elm_scroll_vbar_press_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "elm,vbar,unpress", "elm",
     _elm_scroll_vbar_unpress_cb, sid);

   const char *iface_scr_dragable_hbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
   else
     iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
   edje_object_signal_callback_add
     (sid->edje_obj, "drag", iface_scr_dragable_hbar, _elm_scroll_hbar_drag_cb,
     sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,set", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,start", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_start_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,stop", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_stop_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,step", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,page", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_cb, sid);

   edje_object_signal_callback_add
     (sid->edje_obj, "elm,hbar,press", "elm",
     _elm_scroll_hbar_press_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "elm,hbar,unpress", "elm",
     _elm_scroll_hbar_unpress_cb, sid);
}

static void
_scroll_event_object_attach(Evas_Object *obj)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(obj, sid);

   evas_object_event_callback_add
     (sid->event_rect, EVAS_CALLBACK_MOUSE_WHEEL, _elm_scroll_wheel_event_cb,
     sid);
   evas_object_event_callback_add
     (sid->event_rect, EVAS_CALLBACK_MOUSE_DOWN,
     _elm_scroll_mouse_down_event_cb, sid);
   evas_object_event_callback_add
     (sid->event_rect, EVAS_CALLBACK_MOUSE_UP,
     _elm_scroll_mouse_up_event_cb, sid);
   evas_object_event_callback_add
     (sid->event_rect, EVAS_CALLBACK_MOUSE_MOVE,
     _elm_scroll_mouse_move_event_cb, sid);
}

static void
_scroll_edje_object_detach(Evas_Object *obj)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(obj, sid);

   evas_object_event_callback_del_full
     (sid->edje_obj, EVAS_CALLBACK_RESIZE, _on_edje_resize, sid);
   evas_object_event_callback_del_full
     (sid->edje_obj, EVAS_CALLBACK_MOVE, _on_edje_move, sid);

   if (!_elm_scroll_has_bars(sid))
     return;

   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
   else
     iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag", iface_scr_dragable_vbar, _elm_scroll_vbar_drag_cb,
     sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,set", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,start", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_start_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,stop", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_stop_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,step", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,page", iface_scr_dragable_vbar,
     _elm_scroll_edje_drag_v_cb, sid);

   edje_object_signal_callback_del_full
     (sid->edje_obj, "elm,vbar,press", "elm",
     _elm_scroll_vbar_press_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "elm,vbar,unpress", "elm",
     _elm_scroll_vbar_unpress_cb, sid);

   const char *iface_scr_dragable_hbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
   else
     iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag", iface_scr_dragable_hbar, _elm_scroll_hbar_drag_cb,
     sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,set", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,start", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_start_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,stop", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_stop_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,step", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,page", iface_scr_dragable_hbar,
     _elm_scroll_edje_drag_h_cb, sid);

     edje_object_signal_callback_del_full
     (sid->edje_obj, "elm,hbar,press", "elm",
     _elm_scroll_hbar_press_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "elm,hbar,unpress", "elm",
     _elm_scroll_hbar_unpress_cb, sid);
}

static void
_scroll_event_object_detach(Evas_Object *obj)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(obj, sid);

   evas_object_event_callback_del_full
     (sid->event_rect, EVAS_CALLBACK_MOUSE_WHEEL, _elm_scroll_wheel_event_cb,
     sid);
   evas_object_event_callback_del_full
     (sid->event_rect, EVAS_CALLBACK_MOUSE_DOWN,
     _elm_scroll_mouse_down_event_cb, sid);
   evas_object_event_callback_del_full
     (sid->event_rect, EVAS_CALLBACK_MOUSE_UP,
     _elm_scroll_mouse_up_event_cb, sid);
   evas_object_event_callback_del_full
     (sid->event_rect, EVAS_CALLBACK_MOUSE_MOVE,
     _elm_scroll_mouse_move_event_cb, sid);
}

EOLIAN static void
_elm_interface_scrollable_objects_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Object *edje_object, Evas_Object *hit_rectangle)
{
   Evas_Coord mw, mh;

   if (!edje_object || !hit_rectangle) return;

   if (sid->edje_obj)
     _scroll_edje_object_detach(obj);

   sid->edje_obj = edje_object;

   if (sid->event_rect)
     _scroll_event_object_detach(obj);

   sid->event_rect = hit_rectangle;
   evas_object_repeat_events_set(hit_rectangle, EINA_TRUE);

   _scroll_edje_object_attach(obj);
   _scroll_event_object_attach(obj);

   mw = mh = -1;
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);

   if (elm_widget_is_legacy(sid->obj))
     {
        if (edje_object_part_exists(sid->edje_obj, "elm.scrollbar.base"))
          {
            Evas_Object *base;

            base = edje_object_part_swallow_get
              (sid->edje_obj, "elm.scrollbar.base");
            if (!base)
              {
                base = evas_object_rectangle_add
                    (evas_object_evas_get(sid->edje_obj));
                evas_object_color_set(base, 0, 0, 0, 0);
                edje_object_part_swallow
                  (sid->edje_obj, "elm.scrollbar.base", base);
              }
            if (!_elm_config->thumbscroll_enable)
              evas_object_size_hint_min_set(base, mw, mh);
          }
     }
   else
     {
        if (edje_object_part_exists(sid->edje_obj, "efl.scrollbar.base"))
          {
            Evas_Object *base;

            base = edje_object_part_swallow_get
              (sid->edje_obj, "efl.scrollbar.base");
            if (!base)
              {
                base = evas_object_rectangle_add
                    (evas_object_evas_get(sid->edje_obj));
                evas_object_color_set(base, 0, 0, 0, 0);
                edje_object_part_swallow
                  (sid->edje_obj, "efl.scrollbar.base", base);
              }
            if (!_elm_config->thumbscroll_enable)
              evas_object_size_hint_min_set(base, mw, mh);
          }
     }

   _elm_scroll_scroll_bar_visibility_adjust(sid);
}

static void
_elm_scroll_scroll_bar_reset(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord px = 0, py = 0, minx = 0, miny = 0;

   if (!sid->edje_obj || efl_invalidated_get(sid->edje_obj)) return;

   const char *iface_scr_dragable_hbar = NULL;
   const char *iface_scr_dragable_vbar = NULL;
   if (elm_widget_is_legacy(sid->obj))
     {
        iface_scr_dragable_hbar = iface_scr_legacy_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_legacy_dragable_vbar;
     }
   else
     {
        iface_scr_dragable_hbar = iface_scr_efl_ui_dragable_hbar;
        iface_scr_dragable_vbar = iface_scr_efl_ui_dragable_vbar;
     }

   if (_elm_scroll_has_bars(sid))
     {
        edje_object_part_drag_value_set
              (sid->edje_obj, iface_scr_dragable_vbar, 0.0, 0.0);
        edje_object_part_drag_value_set
              (sid->edje_obj, iface_scr_dragable_hbar, 0.0, 0.0);
        if ((!sid->content) && (!sid->extern_pan))
          {
             edje_object_part_drag_size_set
                   (sid->edje_obj, iface_scr_dragable_vbar, 1.0, 1.0);
             edje_object_part_drag_size_set
                   (sid->edje_obj, iface_scr_dragable_hbar, 1.0, 1.0);
          }
     }
   if (sid->pan_obj)
     {
        elm_obj_pan_pos_min_get(sid->pan_obj, &minx, &miny);
        elm_obj_pan_pos_get(sid->pan_obj, &px, &py);
        elm_obj_pan_pos_set(sid->pan_obj, minx, miny);
     }
   if ((px != minx) || (py != miny))
     edje_object_signal_emit(sid->edje_obj, "elm,action,scroll", "elm");
   _elm_direction_arrows_eval(sid);
}

static void
_elm_scroll_pan_resized_cb(void *data,
                          Evas *e EINA_UNUSED,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   Evas_Coord w = 0, h = 0;
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.content_viewport_resize)
     {
        elm_interface_scrollable_content_viewport_geometry_get
              (sid->obj, NULL, NULL, &w, &h);
        sid->cb_func.content_viewport_resize(sid->obj, w, h);
     }
}

/* even external pan objects get this */
static void
_elm_scroll_pan_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Coord w = 0, h = 0;
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (!sid->pan_obj) return;

   elm_obj_pan_content_size_get(sid->pan_obj, &w, &h);
   if ((w != sid->content_info.w) || (h != sid->content_info.h))
     {
        sid->content_info.w = w;
        sid->content_info.h = h;
        _elm_scroll_scroll_bar_size_adjust(sid);

        evas_object_size_hint_min_set
          (sid->edje_obj, sid->content_info.w, sid->content_info.h);
        sid->content_info.resized = EINA_TRUE;
        _elm_scroll_wanted_region_set(sid->obj);
     }
}

static void
_elm_scroll_content_del_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   sid->content = NULL;
   _elm_scroll_scroll_bar_size_adjust(sid);
   _elm_scroll_scroll_bar_reset(sid);
}

EOLIAN static void
_elm_interface_scrollable_scrollable_content_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Object *content)
{
   Evas_Coord w = 0, h = 0;
   Evas_Object *o;

   if (!sid->edje_obj) return;

   if (sid->content)
     {
        /* if we had content, for sure we had a pan object */
        _elm_pan_content_set(sid->pan_obj, NULL);
        evas_object_event_callback_del_full
          (sid->content, EVAS_CALLBACK_DEL, _elm_scroll_content_del_cb, sid);
     }

   sid->content = content;
   sid->wx = sid->wy = 0;
   /* (-1) means want viewports size */
   sid->ww = sid->wh = -1;
   if (!content) return;

   if (!sid->pan_obj)
     {
        o = _elm_pan_add(evas_object_evas_get(obj));
        ELM_PAN_DATA_GET_OR_RETURN(o, pd);
        pd->interface_object = obj;
        sid->pan_obj = o;
        efl_event_callback_add
          (o, ELM_PAN_EVENT_CHANGED, _elm_scroll_pan_changed_cb, sid);
        evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE,
                                       _elm_scroll_pan_resized_cb, sid);

        if (elm_widget_is_legacy(obj))
          edje_object_part_swallow(sid->edje_obj, "elm.swallow.content", o);
        else
          edje_object_part_swallow(sid->edje_obj, "efl.content", o);
     }

   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _elm_scroll_content_del_cb, sid);

   _elm_pan_content_set(sid->pan_obj, content);
   elm_obj_pan_content_size_get(sid->pan_obj, &w, &h);
   sid->content_info.w = w;
   sid->content_info.h = h;

   _elm_scroll_scroll_bar_size_adjust(sid);
   _elm_scroll_scroll_bar_reset(sid);
}

EOLIAN static void
_elm_interface_scrollable_extern_pan_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Object *pan)
{
   if (!sid->edje_obj) return;

   elm_interface_scrollable_content_set(obj, NULL);

   if (sid->pan_obj)
     {
        efl_event_callback_del(sid->pan_obj, ELM_PAN_EVENT_CHANGED, _elm_scroll_pan_changed_cb, sid);
        evas_object_event_callback_del(sid->pan_obj, EVAS_CALLBACK_RESIZE,
                                       _elm_scroll_pan_resized_cb);
     }

   if (sid->extern_pan)
     {
        if (sid->pan_obj)
          {
             /* not owned by scroller, just leave (was external already) */
             edje_object_part_unswallow(sid->edje_obj, sid->pan_obj);
             sid->pan_obj = NULL;
          }
     }
   else
     {
        ELM_SAFE_FREE(sid->pan_obj, evas_object_del);
     }
   if (!pan)
     {
        sid->extern_pan = EINA_FALSE;
        return;
     }

   sid->pan_obj = pan;

   sid->extern_pan = EINA_TRUE;
   efl_event_callback_add
     (sid->pan_obj, ELM_PAN_EVENT_CHANGED, _elm_scroll_pan_changed_cb, sid);
   evas_object_event_callback_add(sid->pan_obj, EVAS_CALLBACK_RESIZE,
                                  _elm_scroll_pan_resized_cb, sid);

   if (elm_widget_is_legacy(obj))
     {
        edje_object_part_swallow
        (sid->edje_obj, "elm.swallow.content", sid->pan_obj);
     }
   else
     {
        edje_object_part_swallow
        (sid->edje_obj, "efl.content", sid->pan_obj);
     }
}

EOLIAN static void
_elm_interface_scrollable_drag_start_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb drag_start_cb)
{
   sid->cb_func.drag_start = drag_start_cb;
}

EOLIAN static void
_elm_interface_scrollable_drag_stop_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb drag_stop_cb)
{
   sid->cb_func.drag_stop = drag_stop_cb;
}

EOLIAN static void
_elm_interface_scrollable_animate_start_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb animate_start_cb)
{
   sid->cb_func.animate_start = animate_start_cb;
}

EOLIAN static void
_elm_interface_scrollable_animate_stop_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb animate_stop_cb)
{
   sid->cb_func.animate_stop = animate_stop_cb;
}

EOLIAN static void
_elm_interface_scrollable_page_change_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb page_change_cb EINA_UNUSED)
{
   sid->cb_func.page_change = page_change_cb;
}

EOLIAN static void
_elm_interface_scrollable_scroll_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb scroll_cb)
{
   sid->cb_func.scroll = scroll_cb;
}

EOLIAN static void
_elm_interface_scrollable_scroll_left_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb scroll_left_cb)
{
   sid->cb_func.scroll_left = scroll_left_cb;
}

EOLIAN static void
_elm_interface_scrollable_scroll_right_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb scroll_right_cb)
{
   sid->cb_func.scroll_right = scroll_right_cb;
}

EOLIAN static void
_elm_interface_scrollable_scroll_up_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb scroll_up_cb)
{
   sid->cb_func.scroll_up = scroll_up_cb;
}

EOLIAN static void
_elm_interface_scrollable_scroll_down_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb scroll_down_cb)
{
   sid->cb_func.scroll_down = scroll_down_cb;
}

EOLIAN static void
_elm_interface_scrollable_edge_left_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb edje_left_cb)
{
   sid->cb_func.edge_left = edje_left_cb;
}

EOLIAN static void
_elm_interface_scrollable_edge_right_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb edje_right_cb)
{
   sid->cb_func.edge_right = edje_right_cb;
}

EOLIAN static void
_elm_interface_scrollable_edge_top_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb edje_top_cb)
{
   sid->cb_func.edge_top = edje_top_cb;
}

EOLIAN static void
_elm_interface_scrollable_edge_bottom_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb edje_bottom_cb)
{
   sid->cb_func.edge_bottom = edje_bottom_cb;
}

EOLIAN static void
_elm_interface_scrollable_vbar_drag_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb vbar_drag_cb)
{
   sid->cb_func.vbar_drag = vbar_drag_cb;
}

EOLIAN static void
_elm_interface_scrollable_vbar_press_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb vbar_press_cb)
{
   sid->cb_func.vbar_press = vbar_press_cb;
}

EOLIAN static void
_elm_interface_scrollable_vbar_unpress_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb vbar_unpress_cb)
{
   sid->cb_func.vbar_unpress = vbar_unpress_cb;
}

EOLIAN static void
_elm_interface_scrollable_hbar_drag_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb hbar_drag_cb)
{
   sid->cb_func.hbar_drag = hbar_drag_cb;
}

EOLIAN static void
_elm_interface_scrollable_hbar_press_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb hbar_press_cb)
{
   sid->cb_func.hbar_press = hbar_press_cb;
}

EOLIAN static void
_elm_interface_scrollable_hbar_unpress_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Cb hbar_unpress_cb)
{
   sid->cb_func.hbar_unpress = hbar_unpress_cb;
}

EOLIAN static void
_elm_interface_scrollable_content_min_limit_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Min_Limit_Cb min_limit_cb)
{
   sid->cb_func.content_min_limit = min_limit_cb;
}

EOLIAN static void
_elm_interface_scrollable_content_viewport_resize_cb_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Interface_Scrollable_Resize_Cb viewport_resize_cb)
{
   sid->cb_func.content_viewport_resize = viewport_resize_cb;
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_momentum_animator_disabled_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->momentum_animator_disabled;
}

EOLIAN static void
_elm_interface_scrollable_momentum_animator_disabled_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool disabled)
{
   sid->momentum_animator_disabled = disabled;
   if (sid->momentum_animator_disabled)
     {
        if (sid->down.momentum_animator)
          {
             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
     }
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_bounce_animator_disabled_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->bounce_animator_disabled;
}

EOLIAN static void
_elm_interface_scrollable_bounce_animator_disabled_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool disabled)
{
   sid->bounce_animator_disabled = disabled;
   if (sid->bounce_animator_disabled)
     {
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
        ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
     }
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_wheel_disabled_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->wheel_disabled;
}

EOLIAN static void
_elm_interface_scrollable_wheel_disabled_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool disabled)
{
   if (!sid->event_rect) return;

   if ((!sid->wheel_disabled) && (disabled))
     evas_object_event_callback_del_full
       (sid->event_rect, EVAS_CALLBACK_MOUSE_WHEEL,
       _elm_scroll_wheel_event_cb, sid);
   else if ((sid->wheel_disabled) && (!disabled))
     evas_object_event_callback_add
       (sid->event_rect, EVAS_CALLBACK_MOUSE_WHEEL,
       _elm_scroll_wheel_event_cb, sid);
   sid->wheel_disabled = disabled;
}

EOLIAN static void
_elm_interface_scrollable_step_size_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y)
{
   if (x < 1) x = 1;
   if (y < 1) y = 1;
   sid->step.x = x;
   sid->step.y = y;

   _elm_scroll_scroll_bar_size_adjust(sid);
}

EOLIAN static void
_elm_interface_scrollable_step_size_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = sid->step.x;
   if (y) *y = sid->step.y;
}

EOLIAN static void
_elm_interface_scrollable_page_size_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y)
{
   sid->page.x = x;
   sid->page.y = y;

   _elm_scroll_scroll_bar_size_adjust(sid);
}

EOLIAN static void
_elm_interface_scrollable_page_size_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = sid->page.x;
   if (y) *y = sid->page.y;
}

EOLIAN static void
_elm_interface_scrollable_policy_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar)
{
   if (!sid->edje_obj) return;

   if ((sid->hbar_flags == hbar) && (sid->vbar_flags == vbar)) return;

   sid->hbar_flags = hbar;
   sid->vbar_flags = vbar;
   _elm_scroll_policy_signal_emit(sid);
   if (sid->cb_func.content_min_limit)
     sid->cb_func.content_min_limit(sid->obj, sid->min_w, sid->min_h);
   _elm_direction_arrows_eval(sid);
}

EOLIAN static void
_elm_interface_scrollable_policy_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Scroller_Policy *hbar, Elm_Scroller_Policy *vbar)
{
   if (hbar) *hbar = sid->hbar_flags;
   if (vbar) *vbar = sid->vbar_flags;
}

EOLIAN static void
_elm_interface_scrollable_single_direction_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Scroller_Single_Direction single_dir)
{
   sid->one_direction_at_a_time = single_dir;
}

EOLIAN static Elm_Scroller_Single_Direction
_elm_interface_scrollable_single_direction_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->one_direction_at_a_time;
}

EOLIAN static void
_elm_interface_scrollable_content_events_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool repeat_events)
{
   if (sid->event_rect)
     evas_object_repeat_events_set(sid->event_rect, repeat_events);
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_content_events_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   if (sid->event_rect)
     return evas_object_repeat_events_get(sid->event_rect);
   else
     return EINA_TRUE;
}

EOLIAN static void
_elm_interface_scrollable_hold_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool hold)
{
   sid->hold = hold;
}

EOLIAN static void
_elm_interface_scrollable_freeze_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool freeze)
{
   sid->freeze = freeze;
   sid->freeze_want = freeze;
   if (sid->freeze)
     {
        if (sid->down.onhold_animator)
          {
             ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.onhold_animator, _elm_scroll_on_hold_animator, sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
     }
   else
     _elm_scroll_bounce_eval(sid);
}

EOLIAN static void
_elm_interface_scrollable_page_snap_allow_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool horiz, Eina_Bool vert)
{
   sid->page_snap_horiz = !!horiz;
   sid->page_snap_vert = !!vert;
}

EOLIAN static void
_elm_interface_scrollable_page_snap_allow_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool *horiz, Eina_Bool *vert)
{
   if (horiz) *horiz = sid->page_snap_horiz;
   if (vert) *vert = sid->page_snap_vert;
}

EOLIAN static void
_elm_interface_scrollable_bounce_allow_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool horiz, Eina_Bool vert)
{
   sid->bounce_horiz = !!horiz;
   sid->bounce_vert = !!vert;
}

EOLIAN static void
_elm_interface_scrollable_bounce_allow_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool *horiz, Eina_Bool *vert)
{
   if (horiz) *horiz = sid->bounce_horiz;
   if (vert) *vert = sid->bounce_vert;
}

EOLIAN static void
_elm_interface_scrollable_paging_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, double pagerel_h, double pagerel_v, Evas_Coord pagesize_h, Evas_Coord pagesize_v)
{
   sid->pagerel_h = pagerel_h;
   sid->pagerel_v = pagerel_v;
   sid->pagesize_h = pagesize_h;
   sid->pagesize_v = pagesize_v;

   _elm_scroll_page_adjust(sid);
}

EOLIAN static void
_elm_interface_scrollable_paging_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, double *pagerel_h, double *pagerel_v, Evas_Coord *pagesize_h, Evas_Coord *pagesize_v)
{
   if (pagerel_h) *pagerel_h = sid->pagerel_h;
   if (pagerel_v) *pagerel_v = sid->pagerel_v;
   if (pagesize_h) *pagesize_h = sid->pagesize_h;
   if (pagesize_v) *pagesize_v = sid->pagesize_v;
}

EOLIAN static void
_elm_interface_scrollable_page_relative_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *_pd EINA_UNUSED, double h_pagerel, double v_pagerel)
{
   Evas_Coord pagesize_h, pagesize_v;

   elm_interface_scrollable_paging_get(obj, NULL, NULL, &pagesize_h, &pagesize_v);

   elm_interface_scrollable_paging_set(obj, h_pagerel, v_pagerel, pagesize_h, pagesize_v);
}

EOLIAN static void
_elm_interface_scrollable_page_scroll_limit_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int page_limit_h, int page_limit_v)
{
   sid->page_limit_h = page_limit_h;
   sid->page_limit_v = page_limit_v;
}

EOLIAN static void
_elm_interface_scrollable_page_scroll_limit_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int *page_limit_h, int *page_limit_v)
{
   if (page_limit_h) *page_limit_h = sid->page_limit_h;
   if (page_limit_v) *page_limit_v = sid->page_limit_v;
}

EOLIAN static void
_elm_interface_scrollable_current_page_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int *pagenumber_h, int *pagenumber_v)
{
   Evas_Coord x, y;

   elm_interface_scrollable_content_pos_get(sid->obj, &x, &y);
   if (pagenumber_h)
     {
        if (sid->is_mirrored)
          x = _elm_scroll_x_mirrored_get(sid->obj, x);

        if (sid->pagesize_h > 0)
          {
             double result = (double)x / (double)sid->pagesize_h;
             double rest = result - (x / sid->pagesize_h);
             if (rest >= 0.5)
               *pagenumber_h = result + 1;
             else
               *pagenumber_h = result;
          }
        else
          *pagenumber_h = 0;
     }
   if (pagenumber_v)
     {
        if (sid->pagesize_v > 0)
          {
             double result = (double)y / (double)sid->pagesize_v;
             double rest = result - (y / sid->pagesize_v);
             if (rest >= 0.5)
               *pagenumber_v = result + 1;
             else
               *pagenumber_v = result;
          }
        else
          *pagenumber_v = 0;
     }
}

EOLIAN static void
_elm_interface_scrollable_last_page_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int *pagenumber_h, int *pagenumber_v)
{
   Evas_Coord cw, ch;

   if (!sid->pan_obj) return;

   elm_obj_pan_content_size_get(sid->pan_obj, &cw, &ch);
   if (pagenumber_h)
     {
        if ((sid->pagesize_h > 0) && (cw > sid->pagesize_h))
          *pagenumber_h = ceil((double)cw / (double)sid->pagesize_h) - 1;
        else
          *pagenumber_h = 0;
     }
   if (pagenumber_v)
     {
        if ((sid->pagesize_v > 0) && (ch > sid->pagesize_v))
          *pagenumber_v = ceil((double)ch / (double)sid->pagesize_v) - 1;
        else
          *pagenumber_v = 0;
     }
}

EOLIAN static void
_elm_interface_scrollable_page_show(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, int pagenumber_h, int pagenumber_v)
{
   Evas_Coord w = 0, h = 0;
   Evas_Coord x = 0;
   Evas_Coord y = 0;

   sid->current_page.x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
   sid->current_page.y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);

   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &w, &h);
   x = sid->pagesize_h * pagenumber_h;
   x = (sid->is_mirrored ? _elm_scroll_x_mirrored_get(sid->obj, x) : x);
   y = sid->pagesize_v * pagenumber_v;

   sid->wx = x;
   sid->wy = y;
   sid->ww = w;
   sid->wh = h;

   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     elm_interface_scrollable_content_pos_set(obj, x, y, EINA_TRUE);

   if ((sid->current_page.x != x) || (sid->current_page.y != y))
     {
        if (sid->cb_func.page_change)
          sid->cb_func.page_change(sid->obj, NULL);
     }
}

EOLIAN static void
_elm_interface_scrollable_page_bring_in(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, int pagenumber_h, int pagenumber_v)
{
   Evas_Coord w = 0, h = 0;
   Evas_Coord x = 0;
   Evas_Coord y = 0;

   elm_interface_scrollable_content_viewport_geometry_get
         (sid->obj, NULL, NULL, &w, &h);
   x = sid->pagesize_h * pagenumber_h;
   x = (sid->is_mirrored ? _elm_scroll_x_mirrored_get(sid->obj, x) : x);
   y = sid->pagesize_v * pagenumber_v;
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        _elm_scroll_scroll_to_x(sid, _elm_config->bring_in_scroll_friction, x);
        _elm_scroll_scroll_to_y(sid, _elm_config->bring_in_scroll_friction, y);
     }
}

EOLIAN static void
_elm_interface_scrollable_region_bring_in(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        _elm_scroll_scroll_to_x(sid, _elm_config->bring_in_scroll_friction, x);
        _elm_scroll_scroll_to_y(sid, _elm_config->bring_in_scroll_friction, y);
     }
}

EOLIAN static void
_elm_interface_scrollable_gravity_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, double x, double y)
{
   sid->gravity_x = x;
   sid->gravity_y = y;
   elm_obj_pan_pos_max_get(sid->pan_obj, &sid->prev_cw, &sid->prev_ch);
}

EOLIAN static void
_elm_interface_scrollable_gravity_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, double *x, double *y)
{
   if (x) *x = sid->gravity_x;
   if (y) *y = sid->gravity_y;
}

EOLIAN static void
_elm_interface_scrollable_movement_block_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Efl_Ui_Scroll_Block block)
{
   sid->block = block;
}

EOLIAN static Efl_Ui_Scroll_Block
_elm_interface_scrollable_movement_block_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->block;
}

EOLIAN static void
_elm_interface_scrollable_content_loop_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool loop_h, Eina_Bool loop_v)
{
   if (sid->loop_h == loop_h && sid->loop_v == loop_v) return;

   sid->loop_h = loop_h;
   sid->loop_v = loop_v;

   if (sid->loop_h)
     edje_object_signal_emit(sid->edje_obj, "elm,loop_x,set", "elm");
   else
     edje_object_signal_emit(sid->edje_obj, "elm,loop_x,unset", "elm");

   if (sid->loop_v)
     edje_object_signal_emit(sid->edje_obj, "elm,loop_y,set", "elm");
   else
     edje_object_signal_emit(sid->edje_obj, "elm,loop_y,unset", "elm");
}

EOLIAN static void
_elm_interface_scrollable_content_loop_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool *loop_h, Eina_Bool *loop_v)
{
   *loop_h = sid->loop_h;
   *loop_v = sid->loop_v;
}

EOLIAN static void
_elm_interface_scrollable_efl_canvas_group_group_add(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid)
{
   memset(sid, 0, sizeof(*sid));

   sid->obj = obj;

   sid->x = 0;
   sid->y = 0;
   sid->w = 0;
   sid->h = 0;
   sid->step.x = 32 * elm_config_scale_get();
   sid->step.y = 32 * elm_config_scale_get();
   sid->page.x = -50;
   sid->page.y = -50;
   sid->page_limit_h = 9999;
   sid->page_limit_v = 9999;
   sid->hbar_flags = ELM_SCROLLER_POLICY_AUTO;
   sid->vbar_flags = ELM_SCROLLER_POLICY_AUTO;
   sid->hbar_visible = EINA_TRUE;
   sid->vbar_visible = EINA_TRUE;
   sid->loop_h = EINA_FALSE;
   sid->loop_v = EINA_FALSE;

   sid->bounce_horiz = EINA_TRUE;
   sid->bounce_vert = EINA_TRUE;

   sid->one_direction_at_a_time = ELM_SCROLLER_SINGLE_DIRECTION_SOFT;
   sid->momentum_animator_disabled = EINA_FALSE;
   sid->bounce_animator_disabled = EINA_FALSE;
   sid->block = EFL_UI_SCROLL_BLOCK_NONE;

   _elm_scroll_scroll_bar_reset(sid);

   efl_canvas_group_add(efl_super(obj, MY_SCROLLABLE_INTERFACE));
}

EOLIAN static void
_elm_interface_scrollable_efl_canvas_group_group_del(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid)
{

   efl_canvas_group_del(efl_super(obj, MY_SCROLLABLE_INTERFACE));

   elm_interface_scrollable_content_set(obj, NULL);
   if (!sid->extern_pan) evas_object_del(sid->pan_obj);

   ecore_idle_enterer_del(sid->down.hold_enterer);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.hold_animator, _elm_scroll_hold_animator, sid);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.onhold_animator, _elm_scroll_on_hold_animator, sid);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.momentum_animator, _elm_scroll_momentum_animator, sid);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_x_animator, _elm_scroll_bounce_x_animator, sid->obj);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->down.bounce_y_animator, _elm_scroll_bounce_y_animator, sid->obj);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.x.animator, _elm_scroll_scroll_to_x_animator, sid);
   ELM_ANIMATOR_DISCONNECT(sid->obj, sid->scrollto.y.animator, _elm_scroll_scroll_to_y_animator, sid);
}

EOLIAN static void
_elm_interface_scrollable_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_SCROLLABLE_INTERFACE_NAME_LEGACY, klass);
}


/* Legacy ABI compatibility - APIs never worked and were hidden behind
 * EFL_EO_API_SUPPORT (from elm_interface.h) or inside internal headers.
 * Removed between 1.18 and 1.19. The symbols are kept purely for ABI
 * compatibility reasons.
 */
EAPI void elm_pan_gravity_set(Elm_Pan *obj EINA_UNUSED, double x EINA_UNUSED, double y EINA_UNUSED) {}
EAPI void elm_pan_gravity_get(const Elm_Pan *obj EINA_UNUSED, double *x EINA_UNUSED, double *y EINA_UNUSED) {}

EOLIAN static Efl_Ui_Focus_Manager*
_elm_interface_scrollable_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   Efl_Ui_Focus_Manager *manager;

   manager = efl_add(EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS, obj,
     efl_ui_focus_manager_root_set(efl_added, root)
   );

   return manager;
}

EOLIAN static Efl_Object*
_elm_interface_scrollable_efl_object_constructor(Eo *obj, Elm_Scrollable_Smart_Interface_Data *pd EINA_UNUSED)
{
   Eo *o = efl_constructor(efl_super(obj, MY_SCROLLABLE_INTERFACE));

   return o;
}

static Eina_Bool
_filter_cb(const void *iterator EINA_UNUSED, void *data, void *fdata)
{
   Eina_Rect geom;
   int min_x, max_x, min_y, max_y;

   geom = efl_ui_focus_object_focus_geometry_get(data);

   min_x = geom.rect.x;
   min_y = geom.rect.y;
   max_x = eina_rectangle_max_x(&geom.rect);
   max_y = eina_rectangle_max_y(&geom.rect);

   Eina_Bool inside = eina_rectangle_coords_inside(fdata, min_x, min_y) ||
                      eina_rectangle_coords_inside(fdata, min_x, max_y) ||
                      eina_rectangle_coords_inside(fdata, max_x, min_y) ||
                      eina_rectangle_coords_inside(fdata, max_x, max_y);

   return inside;
}

EOLIAN static Eina_Iterator*
_elm_interface_scrollable_efl_ui_focus_manager_border_elements_get(const Eo *obj, Elm_Scrollable_Smart_Interface_Data *pd EINA_UNUSED)
{
   Eina_Iterator *border_elements;
   Eina_Rectangle *rect = calloc(1, sizeof(Eina_Rectangle));

   border_elements = efl_ui_focus_manager_border_elements_get(efl_super(obj, MY_SCROLLABLE_INTERFACE));
   elm_interface_scrollable_content_viewport_geometry_get(obj, &rect->x, &rect->y, &rect->w, &rect->h);

   return eina_iterator_filter_new(border_elements, _filter_cb, free, rect);
}

EOLIAN static void
_elm_interface_scrollable_item_loop_enabled_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *pd EINA_UNUSED, Eina_Bool enable EINA_UNUSED)
{
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_item_loop_enabled_get(const Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}


/* Internal EO APIs and hidden overrides */

#define ELM_PAN_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_pan)

#define ELM_INTERFACE_SCROLLABLE_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_interface_scrollable)

#include "elm_interface_scrollable.eo.c"
#include "elm_pan.eo.c"
