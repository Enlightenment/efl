#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_interface_scrollable.h"

#define MY_PAN_CLASS ELM_OBJ_PAN_CLASS

#define MY_PAN_CLASS_NAME "Elm_Pan"
#define MY_PAN_CLASS_NAME_LEGACY "elm_pan"

#define ELM_PAN_DATA_GET_OR_RETURN(o, ptr)                      \
  Elm_Pan_Smart_Data *ptr = eo_data_scope_get(o, MY_PAN_CLASS);       \
  if (!ptr)                                                     \
    {                                                           \
       CRI("No smart data for object %p (%s)",             \
                o, evas_object_type_get(o));                    \
       return;                                                  \
    }

#define ELM_PAN_DATA_GET_OR_RETURN_VAL(o, ptr, val)             \
  Elm_Pan_Smart_Data *ptr = eo_data_scope_get(o, MY_PAN_CLASS);       \
  if (!ptr)                                                     \
    {                                                           \
       CRI("No smart data for object %p (%s)",             \
                o, evas_object_type_get(o));                    \
       return val;                                              \
    }

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
   if (!psd->gravity_x && !psd->gravity_y)
     {
        evas_object_move(psd->content, psd->x - psd->px, psd->y - psd->py);
        return;
     }

   if ((!psd->px) && (!psd->py))
     {
        psd->px = psd->delta_posx * psd->gravity_x;
        psd->py = psd->delta_posy * psd->gravity_y;
     }
   psd->delta_posx += psd->content_w - psd->prev_cw;
   psd->prev_cw = psd->content_w;
   psd->delta_posy += psd->content_h - psd->prev_ch;
   psd->prev_ch = psd->content_h;

   evas_object_move(psd->content, psd->x - psd->px, psd->y - psd->py);
   psd->px = psd->delta_posx * psd->gravity_x;
   psd->py = psd->delta_posy * psd->gravity_y;
}

EOLIAN static void
_elm_pan_evas_smart_add(Eo *obj, Elm_Pan_Smart_Data *priv)
{
   eo_do_super(obj, MY_PAN_CLASS, evas_obj_smart_add());

   priv->self = obj;
}

EOLIAN static void
_elm_pan_evas_smart_del(Eo *obj, Elm_Pan_Smart_Data *_pd EINA_UNUSED)
{
   _elm_pan_content_set(obj, NULL);

   eo_do_super(obj, MY_PAN_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_pan_evas_smart_move(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord x, Evas_Coord y)
{
   psd->x = x;
   psd->y = y;

   _elm_pan_update(psd);
}

EOLIAN static void
_elm_pan_evas_smart_resize(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord w, Evas_Coord h)
{
   psd->w = w;
   psd->h = h;

   _elm_pan_update(psd);
   evas_object_smart_callback_call(psd->self, SIG_CHANGED, NULL);
}

EOLIAN static void
_elm_pan_evas_smart_show(Eo *obj, Elm_Pan_Smart_Data *psd)
{
   eo_do_super(obj, MY_PAN_CLASS, evas_obj_smart_show());

   if (psd->content)
     evas_object_show(psd->content);
}

EOLIAN static void
_elm_pan_evas_smart_hide(Eo *obj, Elm_Pan_Smart_Data *psd)
{
   eo_do_super(obj, MY_PAN_CLASS, evas_obj_smart_hide());

   if (psd->content)
     evas_object_hide(psd->content);
}

EOLIAN static void
_elm_pan_pos_set(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord x, Evas_Coord y)
{
   if ((x == psd->px) && (y == psd->py)) return;
   psd->px = x;
   psd->py = y;

   _elm_pan_update(psd);
   evas_object_smart_callback_call(psd->self, SIG_CHANGED, NULL);
}

EOLIAN static void
_elm_pan_pos_get(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = psd->px;
   if (y) *y = psd->py;
}

EOLIAN static void
_elm_pan_pos_max_get(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord *x, Evas_Coord *y)
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
_elm_pan_pos_min_get(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y)
{
   if (x)
     *x = 0;
   if (y)
     *y = 0;
}

EOLIAN static void
_elm_pan_content_size_get(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = psd->content_w;
   if (h) *h = psd->content_h;
}

EOLIAN static void
_elm_pan_gravity_set(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, double x, double y)
{
   psd->gravity_x = x;
   psd->gravity_y = y;
   psd->prev_cw = psd->content_w;
   psd->prev_ch = psd->content_h;
   psd->delta_posx = 0;
   psd->delta_posy = 0;
}

EOLIAN static void
_elm_pan_gravity_get(Eo *obj EINA_UNUSED, Elm_Pan_Smart_Data *psd, double *x, double *y)
{
   if (x) *x = psd->gravity_x;
   if (y) *y = psd->gravity_y;
}

static Evas_Object *
_elm_pan_add(Evas *evas)
{
   Evas_Object *obj = eo_add(MY_PAN_CLASS, evas);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_pan_eo_base_constructor(Eo *obj, Elm_Pan_Smart_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_PAN_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_PAN_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
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
   psd->content_w = psd->content_h = psd->px = psd->py =
           psd->prev_cw = psd->prev_ch = psd->delta_posx = psd->delta_posy = 0;
   evas_object_smart_callback_call(psd->self, SIG_CHANGED, NULL);
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
   evas_object_smart_callback_call(psd->self, SIG_CHANGED, NULL);
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
   evas_object_smart_callback_call(psd->self, SIG_CHANGED, NULL);
}

EOLIAN static void
_elm_pan_class_constructor(Eo_Class *klass)
{
      evas_smart_legacy_type_register(MY_PAN_CLASS_NAME_LEGACY, klass);
}

/* pan smart object on top, scroller interface on bottom */
/* ============================================================ */

#define MY_SCROLLABLE_INTERFACE ELM_INTERFACE_SCROLLABLE_CLASS

#define MY_SCROLLABLE_INTERFACE_NAME "Elm_Interface_Scrollable"
#define MY_SCROLLABLE_INTERFACE_NAME_LEGACY "elm_interface_scrollable"

#define ELM_SCROLL_IFACE_DATA_GET_OR_RETURN(o, ptr)     \
  Elm_Scrollable_Smart_Interface_Data *ptr =            \
    (!eo_isa(o, MY_SCROLLABLE_INTERFACE) ? NULL :       \
     eo_data_scope_get(o, MY_SCROLLABLE_INTERFACE));    \
  if (!ptr)                                             \
    {                                                   \
       CRI("No interface data for object %p (%s)", \
                o, evas_object_type_get(o));            \
       return;                                          \
    }

#define ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  Elm_Scrollable_Smart_Interface_Data *ptr =                 \
    (!eo_isa(o, MY_SCROLLABLE_INTERFACE) ? NULL :            \
     eo_data_scope_get(o, MY_SCROLLABLE_INTERFACE));         \
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
#define EVTIME             1
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

   eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&x, &y));

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
}

static void
_elm_scroll_scroll_bar_size_adjust(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if (!sid->pan_obj || !sid->edje_obj) return;

   if (sid->size_adjust_recurse_abort) return;
   if (sid->size_adjust_recurse > 20)
     {
        sid->size_adjust_recurse_abort = EINA_TRUE;
        return;
     }
   sid->size_adjust_recurse++;
   if ((sid->content) || (sid->extern_pan))
     {
        Evas_Coord x, y, w, h, mx = 0, my = 0, vw = 0, vh = 0, px, py,
                   minx = 0, miny = 0;
        double vx, vy, size;

        edje_object_part_geometry_get
          (sid->edje_obj, "elm.swallow.content", NULL, NULL, &vw, &vh);
        w = sid->content_info.w;
        if (w < 1) w = 1;
        size = (double)vw / (double)w;

        if (size > 1.0)
          {
             size = 1.0;
             edje_object_part_drag_value_set
               (sid->edje_obj, "elm.dragable.hbar", 0.0, 0.0);
          }
        edje_object_part_drag_size_set
          (sid->edje_obj, "elm.dragable.hbar", size, 1.0);

        h = sid->content_info.h;
        if (h < 1) h = 1;
        size = (double)vh / (double)h;
        if (size > 1.0)
          {
             size = 1.0;
             edje_object_part_drag_value_set
               (sid->edje_obj, "elm.dragable.vbar", 0.0, 0.0);
          }
        edje_object_part_drag_size_set
          (sid->edje_obj, "elm.dragable.vbar", 1.0, size);

        edje_object_part_drag_value_get
          (sid->edje_obj, "elm.dragable.hbar", &vx, NULL);
        edje_object_part_drag_value_get
          (sid->edje_obj, "elm.dragable.vbar", NULL, &vy);

        eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
        eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
        x = vx * mx + minx;
        y = vy * my + miny;

        edje_object_part_drag_step_set
          (sid->edje_obj, "elm.dragable.hbar", (double)sid->step.x /
          (double)w, 0.0);
        edje_object_part_drag_step_set
          (sid->edje_obj, "elm.dragable.vbar", 0.0, (double)sid->step.y /
          (double)h);
        if (sid->page.x > 0)
          edje_object_part_drag_page_set
            (sid->edje_obj, "elm.dragable.hbar", (double)sid->page.x /
            (double)w, 0.0);
        else
          edje_object_part_drag_page_set
            (sid->edje_obj, "elm.dragable.hbar",
            -((double)sid->page.x * ((double)vw / (double)w)) / 100.0, 0.0);
        if (sid->page.y > 0)
          edje_object_part_drag_page_set
            (sid->edje_obj, "elm.dragable.vbar", 0.0,
            (double)sid->page.y / (double)h);
        else
          edje_object_part_drag_page_set
            (sid->edje_obj, "elm.dragable.vbar", 0.0,
            -((double)sid->page.y * ((double)vh / (double)h)) / 100.0);

        eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
        if (vx != mx) x = px;
        if (vy != my) y = py;
        eo_do(sid->pan_obj, elm_obj_pan_pos_set(x, y));
     }
   else
     {
        Evas_Coord px = 0, py = 0, minx = 0, miny = 0;

        edje_object_part_drag_size_set
          (sid->edje_obj, "elm.dragable.vbar", 1.0, 1.0);
        edje_object_part_drag_size_set
          (sid->edje_obj, "elm.dragable.hbar", 1.0, 1.0);
        eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
        eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
        eo_do(sid->pan_obj, elm_obj_pan_pos_set(minx, miny));
        if ((px != minx) || (py != miny))
          edje_object_signal_emit(sid->edje_obj, "elm,action,scroll", "elm");
     }
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
   edje_object_part_drag_value_get
     (sid->edje_obj, "elm.dragable.vbar", NULL, &vy);
   edje_object_part_drag_value_get
     (sid->edje_obj, "elm.dragable.hbar", &vx, NULL);
   eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
   x = _round(vx * (double)mx + minx, 1);
   y = _round(vy * (double)my + miny, 1);
   eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
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
     }

   if (sid->cb_func.drag_stop)
     sid->cb_func.drag_stop(sid->obj, NULL);
}

static void
_elm_scroll_anim_start(Elm_Scrollable_Smart_Interface_Data *sid)
{
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
   sid->freeze = EINA_FALSE;
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
   sid->freeze = EINA_FALSE;
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
_elm_interface_scrollable_content_size_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *w, Evas_Coord *h)
{
   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(w, h));
}

EOLIAN static void
_elm_interface_scrollable_content_viewport_size_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *w, Evas_Coord *h)
{
   if (!sid->pan_obj || !sid->edje_obj) return;

   edje_object_calc_force(sid->edje_obj);
   evas_object_geometry_get(sid->pan_obj, NULL, NULL, w, h);
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
   Evas_Coord cw, ch, w, ret;

   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(obj, sid, x);

   if (!sid->pan_obj) return 0;

   eo_do((Eo *)obj, elm_interface_scrollable_content_viewport_size_get(&w, NULL));
   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));
   ret = (cw - (x + w));

   return (ret >= 0) ? ret : 0;
}

/* Update the wanted coordinates according to the x, y passed
 * widget directionality, content size and etc. */
static void
_elm_scroll_wanted_coordinates_update(Elm_Scrollable_Smart_Interface_Data *sid,
                                      Evas_Coord x,
                                      Evas_Coord y)
{
   Evas_Coord cw, ch;

   if (!sid->pan_obj) return;

   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));

   /* Update wx/y/w/h - and if the requested positions aren't legal
    * adjust a bit. */
   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get
         (&sid->ww, &sid->wh));
   if (x < 0)
     sid->wx = 0;
   else if ((x + sid->ww) > cw)
     sid->wx = cw - sid->ww;
   else if (sid->is_mirrored)
     sid->wx = _elm_scroll_x_mirrored_get(sid->obj, x);
   else
     sid->wx = x;
   if (y < 0) sid->wy = 0;
   else if ((y + sid->wh) > ch)
     sid->wy = ch - sid->wh;
   else sid->wy = y;
}

static void
_elm_scroll_momentum_end(Elm_Scrollable_Smart_Interface_Data *sid)
{
   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator)) return;
   if (sid->down.momentum_animator)
     {
        Evas_Coord px, py;
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&px, &py));
        _elm_scroll_wanted_coordinates_update(sid, px, py);

        ELM_SAFE_FREE(sid->down.momentum_animator, ecore_animator_del);
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

static Eina_Bool
_elm_scroll_bounce_x_animator(void *data)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(data, sid, EINA_FALSE);
   Evas_Coord x, y, dx, w, odx, ed, md;
   double t, p, dt, pd, r;

   t = ecore_loop_time_get();
   dt = t - sid->down.anim_start2;
   if (dt >= 0.0)
     {
        dt = dt / _elm_config->thumbscroll_bounce_friction;
        odx = sid->down.b2x - sid->down.bx;
        eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get
              (&w, NULL));
        if (!sid->down.momentum_animator && (w > abs(odx)))
          {
             pd = (double)odx / (double)w;
             pd = (pd > 0) ? pd : -pd;
             pd = 1.0 - ((1.0 - pd) * (1.0 - pd));
             dt = dt / pd;
          }
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
        dx = (odx * p);
        r = 1.0;
        if (sid->down.momentum_animator)
          {
             ed = abs(sid->down.dx * (_elm_config->thumbscroll_friction +
                                      sid->down.extra_time) - sid->down.b0x);
             md = abs(_elm_config->thumbscroll_friction * 5 * w);
             if (ed > md) r = (double)(md) / (double)ed;
          }
        x = sid->down.b2x + (int)((double)(dx - odx) * r);
        if (!sid->down.cancelled)
          eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
        if (dt >= 1.0)
          {
             if (sid->down.momentum_animator)
               sid->down.bounce_x_hold = EINA_TRUE;
             if ((!sid->down.bounce_y_animator) &&
                 (!sid->scrollto.y.animator))
               _elm_scroll_anim_stop(sid);
             sid->down.bounce_x_animator = NULL;
             sid->down.pdx = 0;
             sid->bouncemex = EINA_FALSE;
             _elm_scroll_momentum_end(sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
             return ECORE_CALLBACK_CANCEL;
          }
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_elm_scroll_bounce_y_animator(void *data)
{
   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(data, sid, EINA_FALSE);
   Evas_Coord x, y, dy, h, ody, ed, md;
   double t, p, dt, pd, r;

   t = ecore_loop_time_get();
   dt = t - sid->down.anim_start3;
   if (dt >= 0.0)
     {
        dt = dt / _elm_config->thumbscroll_bounce_friction;
        ody = sid->down.b2y - sid->down.by;
        eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get
              (NULL, &h));
        if (!sid->down.momentum_animator && (h > abs(ody)))
          {
             pd = (double)ody / (double)h;
             pd = (pd > 0) ? pd : -pd;
             pd = 1.0 - ((1.0 - pd) * (1.0 - pd));
             dt = dt / pd;
          }
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
        dy = (ody * p);
        r = 1.0;
        if (sid->down.momentum_animator)
          {
             ed = abs(sid->down.dy * (_elm_config->thumbscroll_friction +
                                      sid->down.extra_time) - sid->down.b0y);
             md = abs(_elm_config->thumbscroll_friction * 5 * h);
             if (ed > md) r = (double)(md) / (double)ed;
          }
        y = sid->down.b2y + (int)((double)(dy - ody) * r);
        if (!sid->down.cancelled)
          eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
        if (dt >= 1.0)
          {
             if (sid->down.momentum_animator)
               sid->down.bounce_y_hold = EINA_TRUE;
             if ((!sid->down.bounce_x_animator) &&
                 (!sid->scrollto.y.animator))
               _elm_scroll_anim_stop(sid);
             sid->down.bounce_y_animator = NULL;
             sid->down.pdy = 0;
             sid->bouncemey = EINA_FALSE;
             _elm_scroll_momentum_end(sid);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
             return ECORE_CALLBACK_CANCEL;
          }
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_elm_scroll_bounce_eval(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord mx, my, px, py, bx, by, b2x, b2y, minx = 0, miny = 0;

   if (!sid->pan_obj) return;

   if (sid->freeze) return;
   if ((!sid->bouncemex) && (!sid->bouncemey)) return;
   if (sid->down.now) return;  // down bounce while still held down
   if (sid->down.onhold_animator)
     {
        ELM_SAFE_FREE(sid->down.onhold_animator, ecore_animator_del);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.hold_animator)
     {
        ELM_SAFE_FREE(sid->down.hold_animator, ecore_animator_del);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);

   eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
   bx = px;
   by = py;
   if (px < minx) px = minx;
   if ((px - minx) > mx) px = mx + minx;
   if (py < miny) py = miny;
   if ((py - miny) > my) py = my + miny;
   b2x = px;
   b2y = py;
   if ((!sid->obj) ||
       (!elm_widget_drag_child_locked_x_get(sid->obj)))
     {
        if ((!sid->down.bounce_x_animator) && (!sid->bounce_animator_disabled))
          {
             if (sid->bouncemex)
               {
                  ELM_SAFE_FREE(sid->scrollto.x.animator, ecore_animator_del);
                  sid->down.bounce_x_animator =
                    ecore_animator_add(_elm_scroll_bounce_x_animator, sid->obj);
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
       (!elm_widget_drag_child_locked_y_get(sid->obj)))
     {
        if ((!sid->down.bounce_y_animator) && (!sid->bounce_animator_disabled))
          {
             if (sid->bouncemey)
               {
                  ELM_SAFE_FREE(sid->scrollto.y.animator, ecore_animator_del);
                  sid->down.bounce_y_animator =
                    ecore_animator_add(_elm_scroll_bounce_y_animator, sid->obj);
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
_elm_interface_scrollable_content_pos_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *x, Evas_Coord *y)
{
   if (!sid->pan_obj) return;

   eo_do(sid->pan_obj, elm_obj_pan_pos_get(x, y));
}

EOLIAN static void
_elm_interface_scrollable_content_pos_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y, Eina_Bool sig)
{
   Evas_Coord mx = 0, my = 0, px = 0, py = 0, spx = 0, spy = 0, minx = 0, miny = 0;
   double vx, vy;


   if (!sid->edje_obj || !sid->pan_obj) return;

   // FIXME: allow for bounce outside of range
   eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));

   if (_paging_is_enabled(sid))
     {
        if (sid->page_snap_horiz)
          {
             //we passed one page to the right
             if (x > sid->current_page.x + sid->pagesize_h)
               x = sid->current_page.x + sid->pagesize_h;
             //we passed one page to the left
             if (x < sid->current_page.x - sid->pagesize_h)
               x = sid->current_page.x - sid->pagesize_h;
          }
        if (sid->page_snap_vert)
          {
             //we passed one page to the bottom
             if (y > sid->current_page.y + sid->pagesize_v)
               y = sid->current_page.y + sid->pagesize_v;
             //we passed one page to the top
             if (y < sid->current_page.y - sid->pagesize_v)
               y = sid->current_page.y - sid->pagesize_v;
          }
     }

   if (!_elm_config->thumbscroll_bounce_enable)
     {

        if (x < minx) x = minx;
        if ((x - minx) > mx) x = mx + minx;
        if (y < miny) y = miny;
        if ((y - miny) > my) y = my + miny;
     }

   if (!sid->bounce_horiz)
     {
        if (x < minx) x = minx;
        if ((x - minx) > mx) x = mx + minx;
     }
   if (!sid->bounce_vert)
     {
        if (y < miny) y = miny;
        if (y - miny > my) y = my + miny;
     }

   eo_do(sid->pan_obj, elm_obj_pan_pos_set(x, y));
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&spx, &spy));

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

   edje_object_part_drag_value_set
     (sid->edje_obj, "elm.dragable.vbar", 0.0, vy);
   edje_object_part_drag_value_set
     (sid->edje_obj, "elm.dragable.hbar", vx, 0.0);

   if (!sid->down.bounce_x_animator)
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
   if (!sid->down.bounce_y_animator)
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
_elm_interface_scrollable_mirrored_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool mirrored)
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

   eo_do(sid->obj, elm_interface_scrollable_content_pos_set(wx, sid->wy, EINA_FALSE));
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
   Evas_Coord mx = 0, my = 0, cw = 0, ch = 0, px = 0, py = 0, nx, ny,
              minx = 0, miny = 0, pw = 0, ph = 0, x = *_x, y = *_y;

   ELM_SCROLL_IFACE_DATA_GET_OR_RETURN_VAL(obj, sid, EINA_FALSE);

   if (!sid->pan_obj) return EINA_FALSE;

   eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
   evas_object_geometry_get(sid->pan_obj, NULL, NULL, &pw, &ph);

   nx = px;
   if ((x < px) && ((x + w) < (px + (cw - mx)))) nx = x;
   else if ((x > px) && ((x + w) > (px + (cw - mx))))
     nx = x + w - (cw - mx);
   ny = py;
   if ((y < py) && ((y + h) < (py + (ch - my)))) ny = y;
   else if ((y > py) && ((y + h) > (py + (ch - my))))
     ny = y + h - (ch - my);

   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator) ||
       (sid->scrollto.x.animator) || (sid->scrollto.y.animator))
     {
        ELM_SAFE_FREE(sid->scrollto.x.animator, ecore_animator_del);
        ELM_SAFE_FREE(sid->scrollto.y.animator, ecore_animator_del);
        if (sid->down.bounce_x_animator)
          {
             ELM_SAFE_FREE(sid->down.bounce_x_animator, ecore_animator_del);
             sid->bouncemex = EINA_FALSE;
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
        if (sid->down.bounce_y_animator)
          {
             ELM_SAFE_FREE(sid->down.bounce_y_animator, ecore_animator_del);
             sid->bouncemey = EINA_FALSE;
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }

        _elm_scroll_anim_stop(sid);
     }
   if (sid->down.hold_animator)
     {
        ELM_SAFE_FREE(sid->down.hold_animator, ecore_animator_del);
        _elm_scroll_drag_stop(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);
   if (sid->down.momentum_animator)
     {
        ELM_SAFE_FREE(sid->down.momentum_animator, ecore_animator_del);
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
   if ((x + pw) > cw) x = cw - pw;
   if (x < minx) x = minx;
   if ((y + ph) > ch) y = ch - ph;
   if (y < miny) y = miny;

   if ((x == px) && (y == py)) return EINA_FALSE;
   *_x = x;
   *_y = y;
   return EINA_TRUE;
}

EOLIAN static void
_elm_interface_scrollable_content_region_get(Eo *obj, Elm_Scrollable_Smart_Interface_Data *_pd EINA_UNUSED, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   eo_do(obj, elm_interface_scrollable_content_pos_get(x, y));
   eo_do(obj, elm_interface_scrollable_content_viewport_size_get(w, h));
}

/* Set should be used for calculated positions, for example, when we move
 * because of an animation or because this is the correct position after
 * constraints. */
EOLIAN static void
_elm_interface_scrollable_content_region_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        eo_do(obj, elm_interface_scrollable_content_pos_set(x, y, EINA_FALSE));
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
   sid->wx = x;
   sid->wy = y;
   sid->ww = w;
   sid->wh = h;
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        eo_do(obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
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
        eo_do(obj, elm_interface_scrollable_content_viewport_size_get
              (&ww, &wh));
     }
   else
     {
        ww = sid->ww;
        wh = sid->wh;
     }

   eo_do(obj, elm_interface_scrollable_content_region_set(wx, sid->wy, ww, wh));
}

static void
_elm_scroll_wheel_event_cb(void *data,
                           Evas *e EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   Elm_Scrollable_Smart_Interface_Data *sid;
   Evas_Event_Mouse_Wheel *ev;
   Evas_Coord x = 0, y = 0, vw = 0, vh = 0, cw = 0, ch = 0;
   int direction = 0;
   int pagenumber_h = 0, pagenumber_v = 0;

   sid = data;
   ev = event_info;
   direction = ev->direction;

   if (sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL)
     return;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if ((evas_key_modifier_is_set(ev->modifiers, "Control")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Alt")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Meta")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Hyper")) ||
       (evas_key_modifier_is_set(ev->modifiers, "Super")))
     return;
   else if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
     direction = !direction;
   eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
   if ((sid->down.bounce_x_animator) || (sid->down.bounce_y_animator) ||
       (sid->scrollto.x.animator) || (sid->scrollto.y.animator))
     {
        _elm_scroll_anim_stop(sid);
     }
   ELM_SAFE_FREE(sid->scrollto.x.animator, ecore_animator_del);
   ELM_SAFE_FREE(sid->scrollto.y.animator, ecore_animator_del);
   if (sid->down.bounce_x_animator)
     {
        ELM_SAFE_FREE(sid->down.bounce_x_animator, ecore_animator_del);
        sid->bouncemex = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.bounce_y_animator)
     {
        ELM_SAFE_FREE(sid->down.bounce_y_animator, ecore_animator_del);
        sid->bouncemey = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get(&vw, &vh));
   if (sid->pan_obj)
     eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));
   if (!_paging_is_enabled(sid))
     {
        if (!direction)
          {
             if (ch > vh || cw <= vw)
               y += ev->z * sid->step.y;
             else
               x += ev->z * sid->step.x;
          }
        else if (direction == 1)
          {
             if (cw > vw || ch <= vh)
               x += ev->z * sid->step.x;
             else
               y += ev->z * sid->step.y;
          }

        if ((!sid->hold) && (!sid->freeze))
          {
             _elm_scroll_wanted_coordinates_update(sid, x, y);
             eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
          }
     }
   else
     {
        eo_do(sid->obj, elm_interface_scrollable_current_page_get(&pagenumber_h, &pagenumber_v));
        if (!direction)
          {
             if (ch > vh || cw <= vw)
               y = (pagenumber_v + (1 * ev->z)) * sid->pagesize_v;
             else
               x = (pagenumber_h + (1 * ev->z)) * sid->pagesize_h;
          }
        else if (direction == 1)
          {
             if (cw > vw || ch <= vh)
               x = (pagenumber_h + (1 * ev->z)) * sid->pagesize_h;
             else
               y = (pagenumber_v + (1 * ev->z)) * sid->pagesize_v;
          }

        if ((!sid->hold) && (!sid->freeze))
          {
             _elm_scroll_scroll_to_x(sid, _elm_config->bring_in_scroll_friction, x);
             _elm_scroll_scroll_to_y(sid, _elm_config->bring_in_scroll_friction, y);
          }
     }
}

static Eina_Bool
_elm_scroll_post_event_up(void *data,
                          Evas *e EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->obj)
     {
        if (sid->down.dragged)
          {
             elm_widget_drag_lock_x_set(sid->obj, EINA_FALSE);
             elm_widget_drag_lock_y_set(sid->obj, EINA_FALSE);
          }
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

static Eina_Bool
_elm_scroll_momentum_animator(void *data)
{
   double t, at, dt, p, r;
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord x, y, dx, dy, px, py, maxx, maxy, minx, miny;
   Eina_Bool no_bounce_x_end = EINA_FALSE, no_bounce_y_end = EINA_FALSE;

   if (!sid->pan_obj)
     {
        sid->down.momentum_animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   t = ecore_loop_time_get();
   dt = t - sid->down.anim_start;
   if (dt >= 0.0)
     {
        r = _elm_config->thumbscroll_min_friction / _elm_config->thumbscroll_friction;
        at = (double)sqrt(
           (sid->down.dx * sid->down.dx) + (sid->down.dy * sid->down.dy));
        at = at < ((1.0 - r) * _elm_config->thumbscroll_friction_standard) ?
           at : (1.0 - r) * _elm_config->thumbscroll_friction_standard;
        at = ((at / _elm_config->thumbscroll_friction_standard) + r) *
           (_elm_config->thumbscroll_friction + sid->down.extra_time);
        dt = dt / at;
        if (dt > 1.0) dt = 1.0;
        p = 1.0 - ((1.0 - dt) * (1.0 - dt));
        dx = (sid->down.dx * (_elm_config->thumbscroll_friction +
                              sid->down.extra_time) * p);
        dy = (sid->down.dy * (_elm_config->thumbscroll_friction +
                              sid->down.extra_time) * p);
        sid->down.ax = dx;
        sid->down.ay = dy;
        x = sid->down.sx - dx;
        y = sid->down.sy - dy;
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&px, &py));
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
        eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
        _elm_scroll_wanted_coordinates_update(sid, x, y);
        eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&maxx, &maxy));
        eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));

        if (!_elm_config->thumbscroll_bounce_enable || !sid->bounce_horiz)
          {
             if (x <= minx) no_bounce_x_end = EINA_TRUE;
             if ((x - minx) >= maxx) no_bounce_x_end = EINA_TRUE;
          }
        if (!_elm_config->thumbscroll_bounce_enable || !sid->bounce_vert)
          {
             if (y <= miny) no_bounce_y_end = EINA_TRUE;
             if ((y - miny) >= maxy) no_bounce_y_end = EINA_TRUE;
          }
        if ((dt >= 1.0) ||
            ((sid->down.bounce_x_hold) && (sid->down.bounce_y_hold)) ||
            (no_bounce_x_end && no_bounce_y_end))
          {
             _elm_scroll_anim_stop(sid);

             sid->down.momentum_animator = NULL;
             sid->down.bounce_x_hold = EINA_FALSE;
             sid->down.bounce_y_hold = EINA_FALSE;
             sid->down.ax = 0;
             sid->down.ay = 0;
             sid->down.pdx = 0;
             sid->down.pdy = 0;
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);

             return ECORE_CALLBACK_CANCEL;
          }
     }

   return ECORE_CALLBACK_RENEW;
}

static Evas_Coord
_elm_scroll_page_x_get(Elm_Scrollable_Smart_Interface_Data *sid,
                       int offset, Eina_Bool limit)
{
   Evas_Coord x, y, w, h, dx, cw, ch, minx = 0;

   if (!sid->pan_obj) return 0;

   eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get(&w, &h));
   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, NULL));

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

   if (sid->pagesize_h > 0)
     {
        x = x + (sid->pagesize_h * 0.5);
        x = x / (sid->pagesize_h);
        x = x * (sid->pagesize_h);
     }
   if ((x + w) > cw) x = cw - w;
   if (x < minx) x = minx;

   return x;
}

static Evas_Coord
_elm_scroll_page_y_get(Elm_Scrollable_Smart_Interface_Data *sid,
                       int offset, Eina_Bool limit)
{
   Evas_Coord x, y, w, h, dy, cw, ch, miny = 0;

   if (!sid->pan_obj) return 0;

   eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get(&w, &h));
   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(NULL, &miny));

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
        y = y + (sid->pagesize_v * 0.5);
        y = y / (sid->pagesize_v);
        y = y * (sid->pagesize_v);
     }
   if ((y + h) > ch) y = ch - h;
   if (y < miny) y = miny;

   return y;
}

static Eina_Bool
_elm_scroll_scroll_to_x_animator(void *data)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord px, py;
   double t, tt;

   if (!sid->pan_obj)
     {
        sid->scrollto.x.animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   t = ecore_loop_time_get();
   tt = (t - sid->scrollto.x.t_start) /
     (sid->scrollto.x.t_end - sid->scrollto.x.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
   px = (sid->scrollto.x.start * (1.0 - tt)) +
     (sid->scrollto.x.end * tt);
   if (t >= sid->scrollto.x.t_end)
     {
        px = sid->scrollto.x.end;
        eo_do(sid->obj, elm_interface_scrollable_content_pos_set(px, py, EINA_TRUE));
        sid->down.sx = px;
        sid->down.x = sid->down.history[0].x;
        sid->down.pdx = 0;
        _elm_scroll_wanted_coordinates_update(sid, px, py);
        sid->scrollto.x.animator = NULL;
        if ((!sid->scrollto.y.animator) && (!sid->down.bounce_y_animator))
          _elm_scroll_anim_stop(sid);
        return ECORE_CALLBACK_CANCEL;
     }
   eo_do(sid->obj, elm_interface_scrollable_content_pos_set(px, py, EINA_TRUE));
   _elm_scroll_wanted_coordinates_update(sid, px, py);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_elm_scroll_scroll_to_y_animator(void *data)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord px, py;
   double t, tt;

   if (!sid->pan_obj) return EINA_FALSE;

   t = ecore_loop_time_get();
   tt = (t - sid->scrollto.y.t_start) /
     (sid->scrollto.y.t_end - sid->scrollto.y.t_start);
   tt = 1.0 - tt;
   tt = 1.0 - (tt * tt);
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
   py = (sid->scrollto.y.start * (1.0 - tt)) +
     (sid->scrollto.y.end * tt);
   if (t >= sid->scrollto.y.t_end)
     {
        py = sid->scrollto.y.end;
        eo_do(sid->obj, elm_interface_scrollable_content_pos_set(px, py, EINA_TRUE));
        sid->down.sy = py;
        sid->down.y = sid->down.history[0].y;
        sid->down.pdy = 0;
        _elm_scroll_wanted_coordinates_update(sid, px, py);
        sid->scrollto.y.animator = NULL;
        if ((!sid->scrollto.x.animator) && (!sid->down.bounce_x_animator))
          _elm_scroll_anim_stop(sid);
        return ECORE_CALLBACK_CANCEL;
     }
   eo_do(sid->obj, elm_interface_scrollable_content_pos_set(px, py, EINA_TRUE));
   _elm_scroll_wanted_coordinates_update(sid, px, py);

   return ECORE_CALLBACK_RENEW;
}

static void
_elm_scroll_scroll_to_y(Elm_Scrollable_Smart_Interface_Data *sid,
                        double t_in,
                        Evas_Coord pos_y)
{
   Evas_Coord px, py, x, y, w, h;
   double t;

   if (!sid->pan_obj) return;

   if (sid->freeze) return;
   if (t_in <= 0.0)
     {
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
        eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get
              (&w, &h));
        y = pos_y;
        eo_do(sid->obj, elm_interface_scrollable_content_region_set(x, y, w, h));
        return;
     }
   t = ecore_loop_time_get();
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
   sid->scrollto.y.start = py;
   sid->scrollto.y.end = pos_y;
   sid->scrollto.y.t_start = t;
   sid->scrollto.y.t_end = t + t_in;
   if (!sid->scrollto.y.animator)
     {
        sid->scrollto.y.animator =
          ecore_animator_add(_elm_scroll_scroll_to_y_animator, sid);
        if (!sid->scrollto.x.animator)
          _elm_scroll_anim_start(sid);
     }
   if (sid->down.bounce_y_animator)
     {
        ELM_SAFE_FREE(sid->down.bounce_y_animator, ecore_animator_del);
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
   Evas_Coord px, py, x, y, w, h;
   double t;

   if (!sid->pan_obj) return;

   if (sid->freeze) return;
   if (t_in <= 0.0)
     {
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
        eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get
              (&w, &h));
        x = pos_x;
        eo_do(sid->obj, elm_interface_scrollable_content_region_set
              (x, y, w, h));
        return;
     }
   t = ecore_loop_time_get();
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
   sid->scrollto.x.start = px;
   sid->scrollto.x.end = pos_x;
   sid->scrollto.x.t_start = t;
   sid->scrollto.x.t_end = t + t_in;
   if (!sid->scrollto.x.animator)
     {
        sid->scrollto.x.animator =
          ecore_animator_add(_elm_scroll_scroll_to_x_animator, sid);
        if (!sid->scrollto.y.animator)
          _elm_scroll_anim_start(sid);
     }
   if (sid->down.bounce_x_animator)
     {
        ELM_SAFE_FREE(sid->down.bounce_x_animator, ecore_animator_del);
        _elm_scroll_momentum_end(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   sid->bouncemex = EINA_FALSE;
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

   if ((sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL) &&
       (sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
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
             ELM_SAFE_FREE(sid->down.onhold_animator, ecore_animator_del);
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

#ifdef EVTIME
                  t = ev->timestamp / 1000.0;
#else
                  t = ecore_loop_time_get();
#endif

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
                  at /= _elm_config->thumbscroll_sensitivity_friction;
                  dx = ev->canvas.x - ax;
                  dy = ev->canvas.y - ay;
                  if (at > 0)
                    {
                       vel = sqrt((dx * dx) + (dy * dy)) / at;
                       if ((_elm_config->thumbscroll_friction > 0.0) &&
                           (vel > _elm_config->thumbscroll_momentum_threshold))
                         {
                            Evas_Coord max_d;
                            int minx, miny, mx, my, px, py;
                            double tt = 0.0, dtt = 0.0;

                            eo_do(sid->pan_obj, elm_obj_pan_pos_min_get
                                  (&minx, &miny));
                            eo_do(sid->pan_obj, elm_obj_pan_pos_max_get
                                  (&mx, &my));
                            eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
                            max_d = _elm_config->thumbscroll_flick_distance_tolerance;
                            if (dx > 0)
                              {
                                 if (dx > max_d) dx = max_d;
                                 sid->down.dx = (sin((M_PI * (double)dx / max_d)
                                                     - (M_PI / 2)) + 1) * max_d / at;
                              }
                            else
                              {
                                 if (dx < -max_d) dx = -max_d;
                                 sid->down.dx = (sin((M_PI * (double)dx / max_d)
                                                     + (M_PI / 2)) - 1) * max_d / at;
                              }
                            if (dy > 0)
                              {
                                 if (dy > max_d) dy = max_d;
                                 sid->down.dy = (sin((M_PI * (double)dy / max_d)
                                                     - (M_PI / 2)) + 1) * max_d / at;
                              }
                            else
                              {
                                 if (dy < -max_d) dy = -max_d;
                                 sid->down.dy = (sin((M_PI * (double)dy / max_d)
                                                     + (M_PI / 2)) - 1) * max_d / at;
                              }
                            if (((sid->down.dx > 0) && (sid->down.pdx > 0)) ||
                                ((sid->down.dx < 0) && (sid->down.pdx < 0)) ||
                                ((sid->down.dy > 0) && (sid->down.pdy > 0)) ||
                                ((sid->down.dy < 0) && (sid->down.pdy < 0)))
                              {
                                 tt = ecore_loop_time_get();
                                 dtt = tt - sid->down.anim_start;

                                 if (dtt < 0.0) dtt = 0.0;
                                 else if (dtt >
                                          _elm_config->thumbscroll_friction)
                                   dtt = _elm_config->thumbscroll_friction;
                                 sid->down.extra_time =
                                   _elm_config->thumbscroll_friction - dtt;
                              }
                            else
                              sid->down.extra_time = 0.0;

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
                                     (!elm_widget_drag_child_locked_y_get
                                        (sid->obj)))
                                   {
                                      sid->down.momentum_animator =
                                        ecore_animator_add
                                          (_elm_scroll_momentum_animator, sid);
                                      ev->event_flags |=
                                        EVAS_EVENT_FLAG_ON_SCROLL;
                                      _elm_scroll_anim_start(sid);
                                   }
                                 sid->down.anim_start = ecore_loop_time_get();
                                 eo_do(sid->obj,
                                       elm_interface_scrollable_content_pos_get(&x, &y));
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

                  eo_do(sid->obj, elm_interface_scrollable_content_pos_get
                        (&x, &y));
                  if ((!sid->obj) ||
                      (!elm_widget_drag_child_locked_x_get
                         (sid->obj)))
                    {
                       pgx = _elm_scroll_page_x_get(sid, ox, EINA_TRUE);
                       if (pgx != x &&
                           !(sid->block &
                            ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
                         {
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_SCROLL;
                            _elm_scroll_scroll_to_x
                              (sid, _elm_config->page_scroll_friction, pgx);
                         }
                    }
                  if ((!sid->obj) ||
                      (!elm_widget_drag_child_locked_y_get
                         (sid->obj)))
                    {
                       pgy = _elm_scroll_page_y_get(sid, oy, EINA_TRUE);
                       if (pgy != y &&
                           !(sid->block &
                            ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL))
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

                  eo_do(sid->obj, elm_interface_scrollable_content_pos_get
                        (&x, &y));
                  if ((!sid->obj) ||
                      (!elm_widget_drag_child_locked_x_get
                         (sid->obj)))
                    {
                       pgx = _elm_scroll_page_x_get(sid, ox, EINA_TRUE);
                       if (pgx != x)
                         _elm_scroll_scroll_to_x
                           (sid, _elm_config->page_scroll_friction, pgx);
                    }
                  if ((!sid->obj) ||
                      (!elm_widget_drag_child_locked_y_get
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
             ELM_SAFE_FREE(sid->down.hold_animator, ecore_animator_del);
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
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
        eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
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

   if ((sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL) &&
       (sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
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
   ELM_SAFE_FREE(sid->scrollto.x.animator, ecore_animator_del);
   ELM_SAFE_FREE(sid->scrollto.y.animator, ecore_animator_del);
   if (sid->down.bounce_x_animator)
     {
        ELM_SAFE_FREE(sid->down.bounce_x_animator, ecore_animator_del);
        sid->bouncemex = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.bounce_y_animator)
     {
        ELM_SAFE_FREE(sid->down.bounce_y_animator, ecore_animator_del);
        sid->bouncemey = EINA_FALSE;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (sid->down.hold_animator)
     {
        ELM_SAFE_FREE(sid->down.hold_animator, ecore_animator_del);
        _elm_scroll_drag_stop(sid);
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   ELM_SAFE_FREE(sid->down.hold_enterer, ecore_idle_enterer_del);
   if (sid->down.momentum_animator)
     {
        ELM_SAFE_FREE(sid->down.momentum_animator, ecore_animator_del);
        sid->down.bounce_x_hold = EINA_FALSE;
        sid->down.bounce_y_hold = EINA_FALSE;
        sid->down.ax = 0;
        sid->down.ay = 0;
        if (sid->content_info.resized)
          _elm_scroll_wanted_region_set(sid->obj);
     }
   if (ev->button == 1)
     {
        sid->down.hist.est_timestamp_diff =
          ecore_loop_time_get() - ((double)ev->timestamp / 1000.0);
        sid->down.hist.tadd = 0.0;
        sid->down.hist.dxsum = 0.0;
        sid->down.hist.dysum = 0.0;
        sid->down.now = EINA_TRUE;
        sid->down.dragged = EINA_FALSE;
        sid->down.dir_x = EINA_FALSE;
        sid->down.dir_y = EINA_FALSE;
        sid->down.x = ev->canvas.x;
        sid->down.y = ev->canvas.y;
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
        sid->down.sx = x;
        sid->down.sy = y;
        sid->down.locked = EINA_FALSE;
        memset(&(sid->down.history[0]), 0,
               sizeof(sid->down.history[0]) * 60);
#ifdef EVTIME
        sid->down.history[0].timestamp = ev->timestamp / 1000.0;
        sid->down.history[0].localtimestamp = ecore_loop_time_get();
#else
        sid->down.history[0].timestamp = ecore_loop_time_get();
#endif
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

   eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
   eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
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

static Eina_Bool
_elm_scroll_post_event_move(void *data,
                            Evas *e EINA_UNUSED)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Eina_Bool horiz, vert;
   int start = 0;

   if (!sid->down.want_dragged) return EINA_TRUE;

   elm_widget_parents_bounce_get(sid->obj, &horiz, &vert);
   if (sid->down.hold_parent)
     {
        if ((sid->down.dir_x) && horiz &&
            !_elm_scroll_can_scroll(sid, sid->down.hdir))
          {
             sid->down.dir_x = EINA_FALSE;
          }
        if ((sid->down.dir_y) && vert &&
            !_elm_scroll_can_scroll(sid, sid->down.vdir))
          {
             sid->down.dir_y = EINA_FALSE;
          }
     }
   if (sid->down.dir_x)
     {
        if ((!sid->obj) ||
            (!elm_widget_drag_child_locked_x_get(sid->obj)))
          {
             sid->down.want_dragged = EINA_FALSE;
             sid->down.dragged = EINA_TRUE;
             if (sid->obj)
               {
                  elm_widget_drag_lock_x_set(sid->obj, 1);
               }
             start = 1;
          }
        else
          sid->down.dir_x = EINA_FALSE;
     }
   if (sid->down.dir_y)
     {
        if ((!sid->obj) ||
            (!elm_widget_drag_child_locked_y_get(sid->obj)))
          {
             sid->down.want_dragged = EINA_FALSE;
             sid->down.dragged = EINA_TRUE;
             if (sid->obj)
               {
                  elm_widget_drag_lock_y_set
                    (sid->obj, EINA_TRUE);
               }
             start = 1;
          }
        else
          sid->down.dir_y = EINA_FALSE;
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

   eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));

   if (*x < minx)
     *x += (minx - *x) * _elm_config->thumbscroll_border_friction;
   else if (sid->content_info.w <= sid->w)
     *x += (sid->down.sx - *x) * _elm_config->thumbscroll_border_friction;
   else if ((sid->content_info.w - sid->w + minx) < *x)
     *x += (sid->content_info.w - sid->w + minx - *x) *
       _elm_config->thumbscroll_border_friction;

   if (*y < miny)
     *y += (miny - *y) * _elm_config->thumbscroll_border_friction;
   else if (sid->content_info.h <= sid->h)
     *y += (sid->down.sy - *y) * _elm_config->thumbscroll_border_friction;
   else if ((sid->content_info.h - sid->h + miny) < *y)
     *y += (sid->content_info.h - sid->h + miny - *y) *
       _elm_config->thumbscroll_border_friction;
}

static Eina_Bool
_elm_scroll_hold_enterer(void *data)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;
   Evas_Coord ox = 0, oy = 0, fx = 0, fy = 0;

   sid->down.hold_enterer = NULL;

   fx = sid->down.hold_x;
   fy = sid->down.hold_y;

   if (_elm_config->scroll_smooth_amount > 0.0)
     {
        int i, count = 0;
        Evas_Coord basex = 0, basey = 0, x, y;
        double dt, t, tdiff, tnow, twin;
        struct
          {
             Evas_Coord x, y, dx, dy;
             double t, dt;
          } pos[60];

        tdiff = sid->down.hist.est_timestamp_diff;
        tnow = ecore_time_get() - tdiff;
        t = tnow;
        twin = _elm_config->scroll_smooth_time_window;
        for (i = 0; i < 60; i++)
          {
             if (sid->down.history[i].timestamp >
                 sid->down.dragged_began_timestamp)
               {
                  // oldest point is sd->down.history[i]
                  // newset is sd->down.history[0]
                  dt = t - sid->down.history[i].timestamp;
                  if (dt > twin)
                    {
                       i--;
                       count--;
                       break;
                    }
                  x = sid->down.history[i].x;
                  y = sid->down.history[i].y;
                  _elm_scroll_down_coord_eval(sid, &x, &y);
                  if (i == 0)
                    {
                       basex = x;
                       basey = y;
                    }
                  pos[i].x = x - basex;
                  pos[i].y = y - basey;
                  pos[i].t = sid->down.history[i].timestamp - sid->down.history[0].timestamp;
                  count++;
               }
          }
        if (count >= 2)
          {
             double dtsum = 0.0, tadd, maxdt;
             double dxsum = 0.0, dysum = 0.0, xsum = 0.0, ysum = 0.0;
             for (i = 0; i < (count - 1); i++)
               {
                  pos[i].dx = pos[i].x - pos[i + 1].x;
                  pos[i].dy = pos[i].y - pos[i + 1].y;
                  pos[i].dt = pos[i].t - pos[i + 1].t;
                  dxsum += pos[i].dx;
                  dysum += pos[i].dy;
                  dtsum += pos[i].dt;
                  xsum += pos[i].x;
                  ysum += pos[i].y;
               }
             maxdt = pos[i].t;
             dxsum /= (double)i;
             dysum /= (double)i;
             dtsum /= (double)i;
             if (dtsum > 0)
               {
                  xsum /= (double)i;
                  ysum /= (double)i;
                  tadd = tnow - sid->down.history[0].timestamp + _elm_config->scroll_smooth_future_time;
                  tadd = tadd - (maxdt / 2);
#define WEIGHT(n, o, v) n = (((double)o * (1.0 - v)) + ((double)n * v))
                  WEIGHT(tadd, sid->down.hist.tadd, _elm_config->scroll_smooth_history_weight);
                  WEIGHT(dxsum, sid->down.hist.dxsum, _elm_config->scroll_smooth_history_weight);
                  WEIGHT(dysum, sid->down.hist.dysum, _elm_config->scroll_smooth_history_weight);
                  fx = basex + xsum + ((dxsum * tadd) / dtsum);
                  fy = basey + ysum + ((dysum * tadd) / dtsum);
                  sid->down.hist.tadd = tadd;
                  sid->down.hist.dxsum = dxsum;
                  sid->down.hist.dysum = dysum;
                  WEIGHT(fx, sid->down.hold_x, _elm_config->scroll_smooth_amount);
                  WEIGHT(fy, sid->down.hold_y, _elm_config->scroll_smooth_amount);
               }
          }
     }

   eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&ox, &oy));
   if (sid->down.dir_x)
     {
        if ((!sid->obj) ||
            (!elm_widget_drag_child_locked_x_get(sid->obj)))
          ox = fx;
     }
   if (sid->down.dir_y)
     {
        if ((!sid->obj) ||
            (!elm_widget_drag_child_locked_y_get(sid->obj)))
          oy = fy;
     }

#ifdef SMOOTHDBG
   if (_elm_scroll_smooth_debug)
     _elm_scroll_smooth_debug_movetime_add(ox, oy);
#endif

   eo_do(sid->obj, elm_interface_scrollable_content_pos_set(ox, oy, EINA_TRUE));

   return EINA_FALSE;
}

static Eina_Bool
_elm_scroll_hold_animator(void *data)
{
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   ecore_idle_enterer_del(sid->down.hold_enterer);
   sid->down.hold_enterer =
     ecore_idle_enterer_before_add(_elm_scroll_hold_enterer, sid);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_elm_scroll_on_hold_animator(void *data)
{
   double t, td;
   double vx, vy;
   Evas_Coord x, y, ox, oy;
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
        eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&ox, &oy));
        x = ox;
        y = oy;

        if (sid->down.dir_x)
          {
             if ((!sid->obj) ||
                 (!elm_widget_drag_child_locked_x_get(sid->obj)))
               {
                  sid->down.onhold_vxe += vx;
                  x = ox + (int)sid->down.onhold_vxe;
                  sid->down.onhold_vxe -= (int)sid->down.onhold_vxe;
               }
          }

        if (sid->down.dir_y)
          {
             if ((!sid->obj) ||
                 (!elm_widget_drag_child_locked_y_get(sid->obj)))
               {
                  sid->down.onhold_vye += vy;
                  y = oy + (int)sid->down.onhold_vye;
                  sid->down.onhold_vye -= (int)sid->down.onhold_vye;
               }
          }

        eo_do(sid->obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
     }
   sid->down.onhold_tlast = t;

   return ECORE_CALLBACK_RENEW;
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

   if ((sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL) &&
       (sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
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
       !(sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
     {
        Evas_Coord px;
        ELM_SAFE_FREE(sid->scrollto.x.animator, ecore_animator_del);
        eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, NULL));
        sid->down.sx = px;
        sid->down.x = sid->down.history[0].x;
     }

   if ((sid->scrollto.y.animator) && (!sid->hold) && (!sid->freeze) &&
       !(sid->block & ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL))
     {
        Evas_Coord py;
        ELM_SAFE_FREE(sid->scrollto.y.animator, ecore_animator_del);
        eo_do(sid->pan_obj, elm_obj_pan_pos_get(NULL, &py));
        sid->down.sy = py;
        sid->down.y = sid->down.history[0].y;
     }

#ifdef SCROLLDBG
   DBG("::: %i %i\n", ev->cur.canvas.x, ev->cur.canvas.y);
#endif
   memmove(&(sid->down.history[1]), &(sid->down.history[0]),
           sizeof(sid->down.history[0]) * (60 - 1));
#ifdef EVTIME
   sid->down.history[0].timestamp = ev->timestamp / 1000.0;
   sid->down.history[0].localtimestamp = ecore_loop_time_get();
#else
   sid->down.history[0].timestamp = ecore_loop_time_get();
#endif
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
             if (!((sid->down.dir_x) || (sid->down.dir_y)) &&
                 (((x * x) + (y * y)) >
                  (_elm_config->thumbscroll_threshold *
                   _elm_config->thumbscroll_threshold)))
               {
                  if (sid->one_direction_at_a_time ==
                      ELM_SCROLLER_SINGLE_DIRECTION_SOFT)
                    {
                       int dodir = 0;
                       if (x > (y * 2))
                         {
                            if (!(sid->block &
                                  ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
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
                                  ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL))
                              {
                                 sid->down.dir_y = EINA_TRUE;
                              }
                            dodir++;
                         }
                       if (!dodir)
                         {
                            if (!(sid->block &
                                  ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
                              {
                                 sid->down.dir_x = EINA_TRUE;
                              }
                            if (!(sid->block &
                                  ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL))
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
                                  ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
                              {
                                 sid->down.dir_x = EINA_TRUE;
                              }
                            sid->down.dir_y = EINA_FALSE;
                         }
                       if (y > x)
                         {
                            sid->down.dir_x = EINA_FALSE;
                            if (!(sid->block &
                                  ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL))
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
                   ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL))
               {
                  sid->down.dir_x = EINA_TRUE;
               }
             if (!(sid->block &
                   ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL))
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
#ifdef EVTIME
                  sid->down.dragged_began_timestamp = ev->timestamp / 1000.0;
#else
                  sid->down.dragged_began_timestamp = ecore_loop_time_get();
#endif
               }
             sid->down.dragged_began = EINA_TRUE;
             if (!sid->down.dragged)
               {
                  sid->down.want_dragged = EINA_TRUE;
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
               }
             if (sid->down.dragged)
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
                Evas_Coord minx, miny, mx, my;

                eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
                eo_do(sid->pan_obj, elm_obj_pan_pos_max_get(&mx, &my));
                if (y < miny)
                  y += (miny - y) *
                    _elm_config->thumbscroll_border_friction;
                else if (my <= 0)
                  y += (sid->down.sy - y) *
                    _elm_config->thumbscroll_border_friction;
                else if ((my + miny) < y)
                  y += (my + miny - y) *
                    _elm_config->thumbscroll_border_friction;
                if (x < minx)
                  x += (minx - x) *
                    _elm_config->thumbscroll_border_friction;
                else if (mx <= 0)
                  x += (sid->down.sx - x) *
                    _elm_config->thumbscroll_border_friction;
                else if ((mx + minx) < x)
                  x += (mx + minx - x) *
                    _elm_config->thumbscroll_border_friction;
             }

             sid->down.hold_x = x;
             sid->down.hold_y = y;
             if (!sid->down.hold_animator)
               sid->down.hold_animator =
                 ecore_animator_add(_elm_scroll_hold_animator, sid);
          }
        else
          {
             if (sid->down.dragged_began)
               {
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
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
                  sid->down.onhold_animator = ecore_animator_add
                      (_elm_scroll_on_hold_animator, sid);
               }
          }
        else
          {
             if (sid->down.onhold_animator)
               {
                  ELM_SAFE_FREE(sid->down.onhold_animator, ecore_animator_del);
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

   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get(&w, &h));

   x = _elm_scroll_page_x_get(sid, 0, EINA_TRUE);
   y = _elm_scroll_page_y_get(sid, 0, EINA_TRUE);

   eo_do(sid->obj, elm_interface_scrollable_content_region_set(x, y, w, h));
}

static void
_elm_scroll_reconfigure(Elm_Scrollable_Smart_Interface_Data *sid)
{
   _elm_scroll_scroll_bar_size_adjust(sid);
   _elm_scroll_scroll_bar_h_visibility_apply(sid);
   _elm_scroll_scroll_bar_v_visibility_apply(sid);
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
   edje_object_signal_callback_add
     (sid->edje_obj, "drag", "elm.dragable.vbar", _elm_scroll_vbar_drag_cb,
     sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,set", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,start", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_start_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,stop", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_stop_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,step", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,page", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "elm,vbar,press", "elm",
     _elm_scroll_vbar_press_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "elm,vbar,unpress", "elm",
     _elm_scroll_vbar_unpress_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag", "elm.dragable.hbar", _elm_scroll_hbar_drag_cb,
     sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,set", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,start", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_start_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,stop", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_stop_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,step", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_add
     (sid->edje_obj, "drag,page", "elm.dragable.hbar",
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

   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag", "elm.dragable.vbar", _elm_scroll_vbar_drag_cb,
     sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,set", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,start", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_start_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,stop", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_stop_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,step", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,page", "elm.dragable.vbar",
     _elm_scroll_edje_drag_v_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "elm,vbar,press", "elm",
     _elm_scroll_vbar_press_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "elm,vbar,unpress", "elm",
     _elm_scroll_vbar_unpress_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag", "elm.dragable.hbar", _elm_scroll_hbar_drag_cb,
     sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,set", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,start", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_start_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,stop", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_stop_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,step", "elm.dragable.hbar",
     _elm_scroll_edje_drag_h_cb, sid);
   edje_object_signal_callback_del_full
     (sid->edje_obj, "drag,page", "elm.dragable.hbar",
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

   _elm_scroll_scroll_bar_visibility_adjust(sid);
}

static void
_elm_scroll_scroll_bar_reset(Elm_Scrollable_Smart_Interface_Data *sid)
{
   Evas_Coord px = 0, py = 0, minx = 0, miny = 0;

   if (!sid->edje_obj) return;

   edje_object_part_drag_value_set
     (sid->edje_obj, "elm.dragable.vbar", 0.0, 0.0);
   edje_object_part_drag_value_set
     (sid->edje_obj, "elm.dragable.hbar", 0.0, 0.0);
   if ((!sid->content) && (!sid->extern_pan))
     {
        edje_object_part_drag_size_set
          (sid->edje_obj, "elm.dragable.vbar", 1.0, 1.0);
        edje_object_part_drag_size_set
          (sid->edje_obj, "elm.dragable.hbar", 1.0, 1.0);
     }
   if (sid->pan_obj)
     {
        eo_do(sid->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
        eo_do(sid->pan_obj, elm_obj_pan_pos_get(&px, &py));
        eo_do(sid->pan_obj, elm_obj_pan_pos_set(minx, miny));
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
   Evas_Coord w, h;
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (sid->cb_func.content_viewport_resize)
     {
        eo_do(sid->obj,
              elm_interface_scrollable_content_viewport_size_get(&w, &h));
        sid->cb_func.content_viewport_resize(sid->obj, w, h);
     }
}

/* even external pan objects get this */
static void
_elm_scroll_pan_changed_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   Elm_Scrollable_Smart_Interface_Data *sid = data;

   if (!sid->pan_obj) return;

   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&w, &h));
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
_elm_interface_scrollable_content_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Object *content)
{
   Evas_Coord w, h;
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
        sid->pan_obj = o;
        evas_object_smart_callback_add
          (o, SIG_CHANGED, _elm_scroll_pan_changed_cb, sid);
        evas_object_event_callback_add(o, EVAS_CALLBACK_RESIZE,
                                       _elm_scroll_pan_resized_cb, sid);
        edje_object_part_swallow(sid->edje_obj, "elm.swallow.content", o);
     }

   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _elm_scroll_content_del_cb, sid);

   _elm_pan_content_set(sid->pan_obj, content);
   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&w, &h));
   sid->content_info.w = w;
   sid->content_info.h = h;

   _elm_scroll_scroll_bar_size_adjust(sid);
   _elm_scroll_scroll_bar_reset(sid);
}

EOLIAN static void
_elm_interface_scrollable_extern_pan_set(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Object *pan)
{
   if (!sid->edje_obj) return;

   eo_do(obj, elm_interface_scrollable_content_set(NULL));

   if (sid->pan_obj)
     {
        evas_object_smart_callback_del
          (sid->pan_obj, SIG_CHANGED, _elm_scroll_pan_changed_cb);
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
   evas_object_smart_callback_add
     (sid->pan_obj, SIG_CHANGED, _elm_scroll_pan_changed_cb, sid);
   evas_object_event_callback_add(sid->pan_obj, EVAS_CALLBACK_RESIZE,
                                  _elm_scroll_pan_resized_cb, sid);
   edje_object_part_swallow
     (sid->edje_obj, "elm.swallow.content", sid->pan_obj);
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
_elm_interface_scrollable_momentum_animator_disabled_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
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
             ELM_SAFE_FREE(sid->down.momentum_animator, ecore_animator_del);
             if (sid->content_info.resized)
               _elm_scroll_wanted_region_set(sid->obj);
          }
     }
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_bounce_animator_disabled_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->bounce_animator_disabled;
}

EOLIAN static void
_elm_interface_scrollable_bounce_animator_disabled_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool disabled)
{
   sid->bounce_animator_disabled = disabled;
   if (sid->bounce_animator_disabled)
     {
        ELM_SAFE_FREE(sid->scrollto.x.animator, ecore_animator_del);
        ELM_SAFE_FREE(sid->scrollto.y.animator, ecore_animator_del);
     }
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_wheel_disabled_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
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
_elm_interface_scrollable_step_size_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *x, Evas_Coord *y)
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
_elm_interface_scrollable_page_size_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Evas_Coord *x, Evas_Coord *y)
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
_elm_interface_scrollable_policy_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Scroller_Policy *hbar, Elm_Scroller_Policy *vbar)
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
_elm_interface_scrollable_single_direction_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->one_direction_at_a_time;
}

EOLIAN static void
_elm_interface_scrollable_repeat_events_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool repeat_events)
{
   if (sid->event_rect)
     evas_object_repeat_events_set(sid->event_rect, repeat_events);
}

EOLIAN static Eina_Bool
_elm_interface_scrollable_repeat_events_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
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
   if (sid->freeze)
     {
        if (sid->down.onhold_animator)
          {
             ELM_SAFE_FREE(sid->down.onhold_animator, ecore_animator_del);
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
_elm_interface_scrollable_page_snap_allow_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool *horiz, Eina_Bool *vert)
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
_elm_interface_scrollable_bounce_allow_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Eina_Bool *horiz, Eina_Bool *vert)
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
_elm_interface_scrollable_paging_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, double *pagerel_h, double *pagerel_v, Evas_Coord *pagesize_h, Evas_Coord *pagesize_v)
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

   eo_do(obj, elm_interface_scrollable_paging_get(NULL, NULL, &pagesize_h, &pagesize_v));

   eo_do(obj, elm_interface_scrollable_paging_set(h_pagerel, v_pagerel, pagesize_h, pagesize_v));
}

EOLIAN static void
_elm_interface_scrollable_page_scroll_limit_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int page_limit_h, int page_limit_v)
{
   sid->page_limit_h = page_limit_h;
   sid->page_limit_v = page_limit_v;
}

EOLIAN static void
_elm_interface_scrollable_page_scroll_limit_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int *page_limit_h, int *page_limit_v)
{
   if (page_limit_h) *page_limit_h = sid->page_limit_h;
   if (page_limit_v) *page_limit_v = sid->page_limit_v;
}

EOLIAN static void
_elm_interface_scrollable_current_page_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int *pagenumber_h, int *pagenumber_v)
{
   Evas_Coord x, y;

   eo_do(sid->obj, elm_interface_scrollable_content_pos_get(&x, &y));
   if (pagenumber_h)
     {
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
_elm_interface_scrollable_last_page_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, int *pagenumber_h, int *pagenumber_v)
{
   Evas_Coord cw, ch;

   if (!sid->pan_obj) return;

   eo_do(sid->pan_obj, elm_obj_pan_content_size_get(&cw, &ch));
   if (pagenumber_h)
     {
        if (sid->pagesize_h > 0)
          *pagenumber_h = cw / sid->pagesize_h + 1;
        else
          *pagenumber_h = 0;
     }
   if (pagenumber_v)
     {
        if (sid->pagesize_v > 0)
          *pagenumber_v = ch / sid->pagesize_v + 1;
        else
          *pagenumber_v = 0;
     }
}

EOLIAN static void
_elm_interface_scrollable_page_show(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, int pagenumber_h, int pagenumber_v)
{
   Evas_Coord w, h;
   Evas_Coord x = 0;
   Evas_Coord y = 0;

   sid->current_page.x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
   sid->current_page.y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);

   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get(&w, &h));
   if (pagenumber_h >= 0) x = sid->pagesize_h * pagenumber_h;
   if (pagenumber_v >= 0) y = sid->pagesize_v * pagenumber_v;

   sid->wx = x;
   sid->wy = y;
   sid->ww = w;
   sid->wh = h;

   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     eo_do(obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));

   if ((sid->current_page.x != x) || (sid->current_page.y != y))
     {
        if (sid->cb_func.page_change)
          sid->cb_func.page_change(sid->obj, NULL);
     }
}

EOLIAN static void
_elm_interface_scrollable_page_bring_in(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid, int pagenumber_h, int pagenumber_v)
{
   Evas_Coord w, h;
   Evas_Coord x = 0;
   Evas_Coord y = 0;

   sid->current_page.x = _elm_scroll_page_x_get(sid, 0, EINA_FALSE);
   sid->current_page.y = _elm_scroll_page_y_get(sid, 0, EINA_FALSE);

   eo_do(sid->obj, elm_interface_scrollable_content_viewport_size_get(&w, &h));
   if (pagenumber_h >= 0) x = sid->pagesize_h * pagenumber_h;
   if (pagenumber_v >= 0) y = sid->pagesize_v * pagenumber_v;
   if (_elm_scroll_content_region_show_internal(obj, &x, &y, w, h))
     {
        _elm_scroll_scroll_to_x(sid, _elm_config->bring_in_scroll_friction, x);
        _elm_scroll_scroll_to_y(sid, _elm_config->bring_in_scroll_friction, y);
     }

   if ((sid->current_page.x != x) || (sid->current_page.y != y))
     {
        if (sid->cb_func.page_change)
          sid->cb_func.page_change(sid->obj, NULL);
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
   if (!sid->pan_obj) return;

   eo_do(sid->pan_obj, elm_obj_pan_gravity_set(x, y));
}

EOLIAN static void
_elm_interface_scrollable_gravity_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, double *x, double *y)
{
   if (!sid->pan_obj) return;

   eo_do(sid->pan_obj, elm_obj_pan_gravity_get(x, y));
}

EOLIAN static void
_elm_interface_scrollable_movement_block_set(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid, Elm_Scroller_Movement_Block block)
{
   sid->block = block;
}

EOLIAN static Elm_Scroller_Movement_Block
_elm_interface_scrollable_movement_block_get(Eo *obj EINA_UNUSED, Elm_Scrollable_Smart_Interface_Data *sid)
{
   return sid->block;
}

EOLIAN static void
_elm_interface_scrollable_evas_smart_add(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid)
{
   memset(sid, 0, sizeof(*sid));

   sid->obj = obj;

   sid->x = 0;
   sid->y = 0;
   sid->w = 0;
   sid->h = 0;
   sid->step.x = 32;
   sid->step.y = 32;
   sid->page.x = -50;
   sid->page.y = -50;
   sid->page_limit_h = 9999;
   sid->page_limit_v = 9999;
   sid->hbar_flags = ELM_SCROLLER_POLICY_AUTO;
   sid->vbar_flags = ELM_SCROLLER_POLICY_AUTO;
   sid->hbar_visible = EINA_TRUE;
   sid->vbar_visible = EINA_TRUE;

   sid->bounce_horiz = EINA_TRUE;
   sid->bounce_vert = EINA_TRUE;

   sid->one_direction_at_a_time = ELM_SCROLLER_SINGLE_DIRECTION_SOFT;
   sid->momentum_animator_disabled = EINA_FALSE;
   sid->bounce_animator_disabled = EINA_FALSE;
   sid->block = ELM_SCROLLER_MOVEMENT_NO_BLOCK;

   _elm_scroll_scroll_bar_reset(sid);

   eo_do_super(obj, MY_SCROLLABLE_INTERFACE, evas_obj_smart_add());
}

EOLIAN static void
_elm_interface_scrollable_evas_smart_del(Eo *obj, Elm_Scrollable_Smart_Interface_Data *sid)
{

   eo_do_super(obj, MY_SCROLLABLE_INTERFACE, evas_obj_smart_del());

   eo_do(obj, elm_interface_scrollable_content_set(NULL));
   if (!sid->extern_pan) evas_object_del(sid->pan_obj);

   ecore_idle_enterer_del(sid->down.hold_enterer);
   ecore_animator_del(sid->down.hold_animator);
   ecore_animator_del(sid->down.onhold_animator);
   ecore_animator_del(sid->down.momentum_animator);
   ecore_animator_del(sid->down.bounce_x_animator);
   ecore_animator_del(sid->down.bounce_y_animator);
   ecore_animator_del(sid->scrollto.x.animator);
   ecore_animator_del(sid->scrollto.y.animator);
}

EOLIAN static void
_elm_interface_scrollable_eo_base_constructor(Eo *obj, Elm_Scrollable_Smart_Interface_Data *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_SCROLLABLE_INTERFACE, eo_constructor());
}

EOLIAN static void
_elm_interface_scrollable_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_SCROLLABLE_INTERFACE_NAME_LEGACY, klass);
}

#include "elm_interface_scrollable.eo.c"
#include "elm_pan.eo.c"
